import re
import matplotlib.pyplot as plt
import numpy as np

def parse_log_data(log_content):
    """
    Log dosyasının içeriğini ayrıştırarak zaman içindeki performans ve enerji metriklerini çıkarır.

    Args:
        log_content (str): Log dosyasının içeriği.

    Returns:
        dict: Zaman, paket, energest vb. verileri içeren bir sözlük.
    """

    times = []
    packets_sent = []
    packets_missed = []
    cpu_times = []
    lpm_times = []
    deep_lpm_times = []
    radio_listen_times = []
    radio_transmit_times = []

    # Daha basit ve net bir regex deseni kullanıyoruz
    pattern = re.compile(
        r"===== Performance Metrics =====\s*\n"
        r"Time: (\d+) seconds\s*\n"
        r"Packets Sent: (\d+)\s*\n"
        r"Packets Missed: (\d+)\s*\n"
        r"===== Energest Metrics =====\s*\n"
        r" CPU Time: (\d+)s\s*\n"
        r" LPM Time: (\d+)s\s*\n"
        r" DEEP LPM Time: (\d+)s\s*\n"
        r" Radio LISTEN Time: (\d+)s\s*\n"
        r" Radio TRANSMIT Time: (\d+)s\s*\n"
        r"================================\s*\n"
    )
    
    matches = pattern.findall(log_content)

    for match in matches:
        try:
            time, sent, missed, cpu, lpm, deep_lpm, listen, transmit = map(int, match)
            times.append(time)
            packets_sent.append(sent)
            packets_missed.append(missed)
            cpu_times.append(cpu)
            lpm_times.append(lpm)
            deep_lpm_times.append(deep_lpm)
            radio_listen_times.append(listen)
            radio_transmit_times.append(transmit)

        except ValueError as ve:
             print(f"ValueError: {ve}, match: {match}")
        except Exception as e:
             print(f"Error processing match: {match}, Error: {e}")
    
    if not matches:
        print("Regex eşleşmesi bulunamadı. Satır satır kontrol ediliyor:")
        for line in log_content.splitlines():
            print(f"Satır: {line}")
            time_match = re.search(r"Time: (\d+) seconds", line)
            if time_match:
                print(f"Time eşleşti: {time_match.group(1)}")
    

    return {
        "time": times,
        "packets_sent": packets_sent,
        "packets_missed": packets_missed,
        "cpu_time": cpu_times,
        "lpm_time": lpm_times,
        "deep_lpm_time": deep_lpm_times,
        "radio_listen_time": radio_listen_times,
        "radio_transmit_time": radio_transmit_times,
    }


def plot_data(data):
    """
    Ayrıştırılan verileri kullanarak grafikler çizer.

    Args:
        data (dict): Ayrıştırılmış log verileri.
    """
    times = data["time"]
    packets_sent = data["packets_sent"]
    packets_missed = data["packets_missed"]
    cpu_times = data["cpu_time"]
    lpm_times = data["lpm_time"]
    deep_lpm_times = data["deep_lpm_time"]
    radio_listen_times = data["radio_listen_time"]
    radio_transmit_times = data["radio_transmit_time"]

    plt.figure(figsize=(15, 10))

    # Paket Gönderim ve Kayıp Grafiği
    plt.subplot(3, 2, 1)
    plt.plot(times, packets_sent, label="Packets Sent", marker="o", linestyle='-')
    plt.plot(times, packets_missed, label="Packets Missed", marker="x", linestyle='-')
    plt.xlabel("Time (seconds)")
    plt.ylabel("Number of Packets")
    plt.title("Packets Sent and Missed Over Time")
    plt.legend()
    plt.grid(True)

    # CPU, LPM ve Deep LPM Zaman Grafiği
    plt.subplot(3, 2, 2)
    plt.plot(times, cpu_times, label="CPU Time", marker="o", linestyle='-')
    plt.plot(times, lpm_times, label="LPM Time", marker="x", linestyle='-')
    plt.plot(times, deep_lpm_times, label="Deep LPM Time", marker="^", linestyle='-')
    plt.xlabel("Time (seconds)")
    plt.ylabel("Time (seconds)")
    plt.title("CPU, LPM, and Deep LPM Time Over Time")
    plt.legend()
    plt.grid(True)

    # Radyo Dinleme ve İletim Zaman Grafiği
    plt.subplot(3, 2, 3)
    plt.plot(times, radio_listen_times, label="Radio Listen Time", marker="o", linestyle='-')
    plt.plot(times, radio_transmit_times, label="Radio Transmit Time", marker="x", linestyle='-')
    plt.xlabel("Time (seconds)")
    plt.ylabel("Time (seconds)")
    plt.title("Radio Listen and Transmit Time Over Time")
    plt.legend()
    plt.grid(True)

    # Enerji Tüketimi (Tahmini)
    plt.subplot(3, 2, 4)
    # Enerji tüketimi hesaplamaları (gerçek değerler için daha detaylı analiz gerekebilir)
    cpu_energy = np.array(cpu_times) * 0.01  # Örnek katsayılar
    lpm_energy = np.array(lpm_times) * 0.001
    listen_energy = np.array(radio_listen_times) * 0.005
    transmit_energy = np.array(radio_transmit_times) * 0.02  # Örnek katsayı

    total_energy = cpu_energy + lpm_energy + listen_energy + transmit_energy

    plt.plot(times, total_energy, label="Total Energy Consumption", marker="o", linestyle='-')
    plt.xlabel("Time (seconds)")
    plt.ylabel("Energy (Arbitrary Units)")
    plt.title("Estimated Total Energy Consumption")
    plt.legend()
    plt.grid(True)

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    file_path = "energest.txt"  # Metin dosyasının adını buraya girin

    try:
        with open(file_path, "r") as file:
            log_content = file.read()
        parsed_data = parse_log_data(log_content)
        # Ayrıştırılan veriyi kontrol için ekrana yazdır
        print("Ayrıştırılan Veriler:", parsed_data)
        plot_data(parsed_data)
    except FileNotFoundError:
        print(f"Hata: '{file_path}' dosyası bulunamadı.")
    except Exception as e:
        print(f"Bir hata oluştu: {e}")
