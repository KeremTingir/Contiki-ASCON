# Contiki-NG Üzerinde Ascon Şifreleme Uygulaması Sunumu

## Giriş

Bu sunumda, Contiki-NG işletim sistemi üzerinde geliştirdiğimiz, düşük güç tüketimli IoT cihazlarında güvenli veri iletişimi sağlamayı amaçlayan Ascon şifreleme uygulamasını tanıtacağız. Projemiz, bir UDP istemci ve bir UDP sunucu arasındaki iletişimi şifreleyerek veri gizliliğini korumayı hedefler. Bu sunumda projenin amacı, kullanılan teknolojiler, kod yapısı, performans değerlendirmesi ve elde ettiğimiz sonuçlar detaylı olarak ele alınacaktır.

## Proje Amacı

Projenin temel amacı, düşük kaynaklara sahip IoT cihazlarında güvenli veri iletişimi sağlamaktır. Bu amaç doğrultusunda, hafif ve etkili bir şifreleme algoritması olan Ascon algoritmasını kullanarak UDP üzerinden veri transferini şifreledik. Böylece, cihazlar arası haberleşme sırasında verilerin gizliliğini ve bütünlüğünü korumayı hedefledik.

## Kullanılan Teknolojiler

1.  **Contiki-NG:** Düşük güç tüketimli ve kaynak kısıtlı IoT cihazları için geliştirilmiş, açık kaynak kodlu bir işletim sistemidir. Projemizin temelini oluşturur ve ağ yığını ile süreç yönetimini sağlar.

2.  **UDP (User Datagram Protocol):** Ağ üzerinden veri göndermek için kullanılan bağlantısız bir protokoldür. Hızlı ve basit iletişime olanak tanır, ancak güvenilirlik konusunda TCP kadar güçlü değildir.

3.  **Ascon:** Hafif ve güvenli bir şifreleme algoritmasıdır. Özellikle düşük kaynaklı cihazlar için tasarlanmıştır ve hem şifreleme hem de kimlik doğrulama özelliklerini bir arada sunar. Projemizde, veri gizliliğini sağlamak için Ascon şifreleme algortiması kullandık.

## Proje Mimarisi

Projemiz iki ana parçadan oluşmaktadır:

1.  **UDP İstemci (client.c):**
    *   Belirli aralıklarla (10 saniyede bir) bir mesaj oluşturur ("hi [sayı]" formatında).
    *   Bu mesajı Ascon algoritması ile şifreler.
    *   Şifrelenmiş mesajı UDP üzerinden sunucuya gönderir.
    *   Gönderilen paket sayısı, kaçırılan paket sayısı ve enerji tüketimi gibi bilgileri kaydeder ve loglar.
    *   Sunucudan aldığı cevabı şifresini çözüp loglar.
    
2.  **UDP Sunucu (server.c):**
    *   UDP üzerinden istemciden gelen şifrelenmiş paketleri dinler.
    *   Alınan paketlerin şifresini Ascon algoritması ile çözer.
    *   Çözülen mesajı loglar.
    *   Orijinal mesajı tekrar şifreleyerek cevap olarak istemciye geri gönderir.
    *   Alınan ve gönderilen paket sayısı, enerji tüketimi gibi bilgileri kaydeder ve loglar.

## Kod Yapısı

Projemizdeki temel dosyalar şunlardır:

1.  `client.c`: UDP istemci uygulamasının kaynak kodunu içerir.
2.  `server.c`: UDP sunucu uygulamasının kaynak kodunu içerir.
3. `ascon.c`: Ascon şifreleme algoritmasının C kaynak kodunu içerir.
4. `ascon.h`: Ascon şifreleme algoritmasının C başlık (header) dosyasıdır.
5. `Makefile`: Projenin derlenmesi için gerekli ayarları içerir.

### client.c Kodunun İşlevi:

*   **`PROCESS(udp_client_process, "UDP client")`:** İstemci uygulamasının ana sürecini tanımlar.
*   **`simple_udp_register(...)`:** UDP bağlantısını başlatır ve geri çağırma (callback) fonksiyonunu kaydeder.
*   **`etimer_set(...)`:** Belirli aralıklarla veri göndermek için zamanlayıcı ayarlar.
*   **`snprintf(str, sizeof(str), "hi %" PRIu32 "", tx_count)`:** Gönderilecek mesajı oluşturur.
*   **`ascon_encrypt(...)`:** Mesajı Ascon algoritması ile şifreler.
*   **`simple_udp_sendto(...)`:** Şifrelenmiş mesajı UDP ile sunucuya gönderir.
*   **`udp_rx_callback(...)`:** Sunucudan gelen cevabı alır, şifresini çözer ve loglar.
*   **Energest Logları:** CPU, LPM, Radyo gibi enerji tüketimlerini ölçer ve loglar.

### server.c Kodunun İşlevi:

*   **`PROCESS(udp_server_process, "UDP server")`:** Sunucu uygulamasının ana sürecini tanımlar.
*   **`NETSTACK_ROUTING.root_start()`:** Sunucu cihazını ağın kök düğümü yapar.
*   **`simple_udp_register(...)`:** UDP bağlantısını başlatır ve geri çağırma (callback) fonksiyonunu kaydeder.
*   **`udp_rx_callback(...)`:** İstemciden gelen şifrelenmiş mesajı alır, şifresini çözer ve loglar.
*   **`ascon_decrypt(...)`:** Mesajı Ascon algoritması ile şifresini çözer.
*   **`ascon_encrypt(...)`:** Cevap mesajını Ascon algoritması ile şifreler.
*   **`simple_udp_sendto(...)`:** Şifrelenmiş cevap mesajını UDP ile istemciye gönderir.
*   **Energest Logları:** CPU, LPM, Radyo gibi enerji tüketimlerini ölçer ve loglar.

### ascon.c ve ascon.h:
*   **ascon.c:** Ascon algoritmasının gerçeklenmiş kodunu içerir, şifreleme ve şifre çözme fonksiyonları burada bulunur.
*   **ascon.h:** Ascon kütüphanesine ait fonksiyonların prototiplerini ve gerekli tanımlamaları içerir.

### Makefile Dosyası:

*   **`CONTIKI_PROJECT = udp-client udp-server`:** Derlenecek projelerin isimlerini tanımlar.
*   **`CFLAGS += -I$(CONTIKI_NG)/ascon`:** Derleyicinin Ascon başlık dosyasını bulmasını sağlar.
*   **`PROJECT_SOURCEFILES += ascon.c`:** Ascon kaynak dosyasını derleme işlemine dahil eder.

## Performans Değerlendirmesi

Projemiz boyunca, hem şifreleme işleminin performansı hem de enerji tüketimi detaylı bir şekilde loglanmıştır. Loglanan bilgiler, iletilen paket sayıları, kaçırılan paket sayıları, CPU kullanımı, düşük güç modu (LPM) kullanımı ve radyo iletişimi için harcanan enerji gibi çeşitli metrikleri içerir. Bu veriler, projenin verimliliği ve gerçek dünya uygulamalarına uygunluğu hakkında fikir verir.

## Elde Edilen Sonuçlar

*   Veri iletişimi Ascon algoritması ile başarıyla şifrelenmiştir.
*   İstemci ve sunucu arasındaki haberleşme stabil bir şekilde gerçekleşmektedir.
*   Loglanan veriler ile enerji tüketimi ve performans hakkında bilgi elde edilmiştir.

## Sonuç

Bu proje, Contiki-NG işletim sistemi üzerinde Ascon algoritması kullanılarak güvenli veri iletişiminin mümkün olduğunu göstermiştir. Düşük güçlü cihazlarda bile verimli bir şekilde çalışan şifreleme yöntemi ile IoT cihazlarının daha güvenli hale getirilmesi sağlanmıştır. Gelecekteki çalışmalarda, farklı şifreleme yöntemleri denenerek, daha optimize edilmiş çözümler geliştirilebilir.

Bu sunum, projenin temel işlevlerini, mimarisini ve elde edilen sonuçları özetlemektedir. Daha fazla detay için kaynak kodlarına ve log çıktılarına başvurabilirsiniz.
