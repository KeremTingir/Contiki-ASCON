# Contiki-NG Ascon Şifreleme Uygulaması

Bu proje, Contiki-NG işletim sistemi üzerinde çalışan basit bir UDP istemci-sunucu uygulamasında Ascon şifreleme algoritmasının kullanımını göstermektedir. Proje, enerji tüketimi ve performans metrikleri ile birlikte veri şifreleme ve şifre çözme işlemlerini içerir.

## Proje Yapısı

Bu proje iki ana bölümden oluşmaktadır:

1.  **UDP İstemci (client.c):** Belirli bir periyotta sunucuya şifrelenmiş UDP paketleri gönderir. Ayrıca gönderilen paketler, kaçırılan paketler ve enerji tüketimi gibi metrikleri kaydeder ve görüntüler.
2.  **UDP Sunucu (server.c):** İstemciden gelen şifrelenmiş UDP paketlerini alır, şifresini çözer, işlem yapar ve ardından istemciye şifrelenmiş bir yanıt gönderir. Ayrıca alınan ve gönderilen paketler ve enerji tüketimi gibi metrikleri kaydeder ve görüntüler.

## Gerekli Kütüphaneler

*   **Contiki-NG:** İşletim sistemi ve ağ yığını.
*   **Ascon:** Ascon şifreleme algoritmasının C kütüphanesi. (Projenize eklenmelidir.)

## Kurulum

1.  **Contiki-NG Kurulumu:** Contiki-NG işletim sistemini geliştirme ortamınıza kurun.
2.  **Ascon Kütüphanesi:** Ascon kütüphanesini indirin ve Contiki-NG projenize entegre edin. Ascon kütüphanesinin header dosyalarını (örneğin, `ascon.h`) projenizin include dizininde bulundurmanız gerekebilir.
3.  **Proje Dosyaları:** `client.c` ve `server.c` dosyalarını Contiki-NG projenizin uygulama dizinine (genellikle `examples/`) ekleyin.
4.  **Makefile:** `Makefile` dosyanızı, uygulama dosyalarını ve gerekli Contiki-NG modüllerini içerecek şekilde güncelleyin. Örnek bir `Makefile` şuna benzer olabilir:

    ```makefile
    CONTIKI_PROJECT = udp-ascon
    all: $(CONTIKI_PROJECT)

    CONTIKI = ../../..
    include $(CONTIKI)/Makefile.include
    
    PROJECT_SOURCEFILES += client.c
    PROJECT_SOURCEFILES += server.c
    
    MODULES += os/services/shell
    MODULES += tools/shell-commands
    
    MODULES += net/routing
    MODULES += net/ipv6/simple-udp
    MODULES += sys/log
    MODULES += sys/energest
    MODULES += os/lib/random
    
    ```

## Çalıştırma

1.  **Derleme:** Projeyi Contiki-NG derleme ortamınızda derleyin.
2.  **Yükleme:** Derlenen kodu iki ayrı node'a yükleyin; birini istemci, diğerini sunucu olarak kullanın.
3.  **İletişim:** Sunucu düğümünü çalıştırarak ağda root olmasını sağlayın. İstemci düğümü, sunucuya ulaşılabilir hale geldiğinde veri göndermeye başlayacaktır.
4.  **Log Takibi:** İstemci ve sunucu düğümlerinin log çıktılarını seri porttan veya ilgili arayüzden takip ederek iletişimi ve performansı gözlemleyebilirsiniz.

## Detaylar

### İstemci (client.c)

*   **Veri Gönderme:** İstemci, belirli aralıklarla sunucuya şifrelenmiş "hi [sayı]" mesajları gönderir.
*   **Şifreleme:** Veri göndermeden önce `ascon_encrypt` fonksiyonu kullanılarak şifrelenir.
*   **Metrikler:** Gönderilen paket sayısı, kaçırılan paket sayısı ve enerji tüketimi (CPU, LPM, Radyo) gibi metrikler loglanır.
*   **Zamanlama:** Periyodik veri gönderimi için bir zamanlayıcı kullanılır.
*   **Ağ Kontrolü:** Sadece ağa bağlı ve root düğüme ulaşılabilir durumda ise veri gönderimi yapılır.

### Sunucu (server.c)

*   **Veri Alma:** Sunucu, istemciden gelen şifrelenmiş UDP paketlerini dinler.
*   **Şifre Çözme:** Alınan veri, `ascon_decrypt` fonksiyonu kullanılarak şifresi çözülür.
*   **Yanıt:** Sunucu, alınan veriyi bir cevap olarak tekrar şifreleyerek istemciye geri gönderir.
*   **Metrikler:** Alınan ve gönderilen paket sayısı ve enerji tüketimi gibi metrikler loglanır.

### Ascon Şifreleme

*   **Anahtar ve Nonce:** Her iki düğümde de aynı 128-bit şifreleme anahtarı (`key`) ve nonce (`nonce`) değerleri kullanılır.
*   **Şifreleme ve Şifre Çözme:** `ascon_encrypt` ve `ascon_decrypt` fonksiyonları, belirtilen anahtar ve nonce ile veri şifreleme ve şifre çözme işlemleri için kullanılır.
*   **Güvenlik Notu:** Bu örnekte kullanılan sabit anahtar ve nonce değerleri sadece test amaçlıdır. Gerçek uygulamalarda rastgele oluşturulmuş ve güvenli bir şekilde paylaşılmış anahtarlar ve nonce değerleri kullanılmalıdır.

## Sonuç

Bu proje, Contiki-NG ortamında Ascon şifreleme algoritmasının basit bir uygulamasını göstermektedir. Enerji tüketimi ve performans takibi, düşük güçlü IoT cihazları için önemli bir gereksinimdir ve bu proje, bu konularda temel bir anlayış sağlar. Gerçek dünya uygulamalarında, bu temel yapıyı kullanarak daha karmaşık ve güvenli iletişim protokolleri oluşturabilirsiniz.
