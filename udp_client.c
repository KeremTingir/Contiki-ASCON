#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include "sys/energest.h"
#include "ascon.h"  // Ascon kütüphanesini dahil et

#include <stdint.h>
#include <inttypes.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (10 * CLOCK_SECOND)
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;
static uint32_t tx_count = 0;
static uint32_t missed_tx_count = 0;

static const uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}; // 128-bit key
static const uint8_t nonce[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}; // 128-bit nonce

/* Convert energest time to seconds */
static unsigned long to_seconds(uint64_t time) {
  return (unsigned long)(time / ENERGEST_SECOND);
}

PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);

static void udp_rx_callback(struct simple_udp_connection *c,
                            const uip_ipaddr_t *sender_addr,
                            uint16_t sender_port,
                            const uip_ipaddr_t *receiver_addr,
                            uint16_t receiver_port,
                            const uint8_t *data,
                            uint16_t datalen) {
  uint8_t decrypted_data[sizeof(data)];

  // Ascon ile gelen şifreli veriyi çözüyoruz
  ascon_decrypt(data, decrypted_data, datalen, key, nonce);

  LOG_INFO("Received response from ");
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
  LOG_INFO("Response Data: '%.*s'\n", datalen, (char *)decrypted_data);
}

PROCESS_THREAD(udp_client_process, ev, data) {
  static struct etimer periodic_timer;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (3 * CLOCK_SECOND)));

  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if (NETSTACK_ROUTING.node_is_reachable() &&
        NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      snprintf(str, sizeof(str), "hi %" PRIu32 "", tx_count);

      // Veriyi Ascon ile şifreliyoruz
      uint8_t encrypted_data[sizeof(str)];
      ascon_encrypt(str, encrypted_data, strlen(str), key, nonce);

      // Şifreli veriyi gönderiyoruz
      simple_udp_sendto(&udp_conn, encrypted_data, sizeof(encrypted_data), &dest_ipaddr);
      LOG_INFO("Sending encrypted request %" PRIu32 " to ", tx_count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      missed_tx_count++;
    }

    // Log performance metrics
    LOG_INFO("SEND_INTERVAL: %lus, Packets Sent: %lu, Packets Missed: %lu\n", 
         SEND_INTERVAL / CLOCK_SECOND, tx_count, missed_tx_count);
    LOG_INFO("===== Performance Metrics =====\n");
    LOG_INFO("Time: %lu seconds\n", clock_seconds());
    LOG_INFO("Packets Sent: %lu\n", tx_count);
    LOG_INFO("Packets Missed: %lu\n", missed_tx_count);

    LOG_INFO("===== Energest Metrics =====\n");
    LOG_INFO(" CPU Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_CPU)));
    LOG_INFO(" LPM Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_LPM)));
    LOG_INFO(" DEEP LPM Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)));
    LOG_INFO(" Radio LISTEN Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)));
    LOG_INFO(" Radio TRANSMIT Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)));
    LOG_INFO("================================\n");

    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}