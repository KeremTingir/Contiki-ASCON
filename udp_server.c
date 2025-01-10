#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include "sys/energest.h"
#include "ascon.h"  // Ascon kütüphanesini dahil et

#include <stdint.h>

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;
static const uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}; // 128-bit key
static const uint8_t nonce[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}; // 128-bit nonce
static uint32_t rx_count = 0;
static uint32_t tx_count = 0;

/* Convert energest time to seconds */
static unsigned long to_seconds(uint64_t time) {
  return (unsigned long)(time / ENERGEST_SECOND);
}

static void udp_rx_callback(struct simple_udp_connection *c,
                            const uip_ipaddr_t *sender_addr,
                            uint16_t sender_port,
                            const uip_ipaddr_t *receiver_addr,
                            uint16_t receiver_port,
                            const uint8_t *data,
                            uint16_t datalen) {
  uint8_t decrypted_data[sizeof(data)];

  // Ascon ile gelen veriyi şifre çözüyoruz
  ascon_decrypt(data, decrypted_data, datalen, key, nonce);

  rx_count++;
  LOG_INFO("Received request '%.*s' from ", datalen, (char *)decrypted_data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

  // Yanıt olarak veriyi şifreliyoruz
  uint8_t encrypted_data[sizeof(decrypted_data)];
  ascon_encrypt(decrypted_data, encrypted_data, sizeof(decrypted_data), key, nonce);

  LOG_INFO("Sending encrypted response to client.\n");
  simple_udp_sendto(&udp_conn, encrypted_data, sizeof(encrypted_data), sender_addr);
  tx_count++;

  // Log performance metrics
  LOG_INFO("===== Performance Metrics =====\n");
  LOG_INFO("Time: %lu seconds\n", clock_seconds());
  LOG_INFO("Packets Received: %lu\n", rx_count);
  LOG_INFO("Packets Sent: %lu\n", tx_count);

  LOG_INFO("===== Energest Metrics =====\n");
  LOG_INFO(" CPU Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_CPU)));
  LOG_INFO(" LPM Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_LPM)));
  LOG_INFO(" DEEP LPM Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)));
  LOG_INFO(" Radio LISTEN Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)));
  LOG_INFO(" Radio TRANSMIT Time: %lus\n", to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)));
  LOG_INFO("================================\n");
}

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

PROCESS_THREAD(udp_server_process, ev, data) {
  PROCESS_BEGIN();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}