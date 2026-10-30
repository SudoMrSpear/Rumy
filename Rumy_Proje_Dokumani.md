# Rumy — Proje Dokümanı

[English](Rumy_Project_Document.md) · **Türkçe**

**Kapsam:** Projenin başından bugüne (27 Eylül 2026) kadar yapılan her şey: ne yapıldı, hangi dille, hangi
kütüphane ve araçlarla, nasıl ve neden öyle yapıldı. Kartta çalışan sürüm: **1.2.0** (1.1.0'dan sonra: Spotify
ekranı, kulaklıklı Rumy, ritim takibi, hareketli uyku "z"leri, yeni ana ekran düzeni, ana ekranda dönen plaklı
"şimdi çalıyor" şeridi, Creality K2 yazıcı ekranı ve kamerası, iyileştirme 1–2).

Bu doküman README'yi tekrarlamaz; README kullanım kılavuzudur (kurulum, komutlar, sorun giderme). Bu doküman ise
projenin **iç yapısını ve geçmişini** anlatır. Kod satır numaraları değişebileceği için dosya ve işlev adlarıyla
referans verilmiştir.

---

## İçindekiler

1. [Proje özeti](#1-proje-özeti)
2. [Donanım](#2-donanım)
3. [Kullanılan diller, kütüphaneler ve araçlar](#3-kullanılan-diller-kütüphaneler-ve-araçlar)
4. [Klasör yapısı](#4-klasör-yapısı)
5. [Genel mimari](#5-genel-mimari)
6. [Bellek ve flash yerleşimi](#6-bellek-ve-flash-yerleşimi)
7. [Ekran, dokunmatik ve çizim altyapısı](#7-ekran-dokunmatik-ve-çizim-altyapısı)
8. [Ekranlar](#8-ekranlar)
9. [Rumy'nin yüzü ve animasyonları](#9-rumynin-yüzü-ve-animasyonları)
10. [Ağ, veri kaynakları ve önbellek](#10-ağ-veri-kaynakları-ve-önbellek)
11. [Önemli günler, deprem, saat](#11-önemli-günler-deprem-saat)
12. [WiZ akıllı ampul](#12-wiz-akıllı-ampul)
13. [Ayarlar, ilk kurulum ve telefon uygulaması](#13-ayarlar-ilk-kurulum-ve-telefon-uygulaması)
14. [SD kart](#14-sd-kart)
15. [Spotify ekranı ve bilgisayar köprüsü](#15-spotify-ekranı-ve-bilgisayar-köprüsü)
16. [Kulaklıklı Rumy ve ritim takibi](#16-kulaklıklı-rumy-ve-ritim-takibi)
17. [Tanılama, seri komutlar, ölçüm araçları](#17-tanılama-seri-komutlar-ölçüm-araçları)
18. [Performans çalışmaları](#18-performans-çalışmaları)
19. [Test ve doğrulama](#19-test-ve-doğrulama)
20. [Derleme, yükleme ve dağıtım](#20-derleme-yükleme-ve-dağıtım)
21. [Kod kuralları ve kısıtlar](#21-kod-kuralları-ve-kısıtlar)
22. [Sürüm geçmişi](#22-sürüm-geçmişi)
23. [Karşılaşılan sorunlar ve çözümleri](#23-karşılaşılan-sorunlar-ve-çözümleri)
24. [Bilinen sınırlar ve açık işler](#24-bilinen-sınırlar-ve-açık-işler)
25. [Creality K2 yazıcı ekranı ve kamera](#25-creality-k2-yazıcı-ekranı-ve-kamera)
26. [Maç takibi (ESPN)](#26-maç-takibi-espn)
27. [Spotify çalma listeleri](#27-spotify-çalma-listeleri)
28. [Kod ajanı ışığı: Claude Code ve Codex (ClawLight uyarlaması)](#28-kod-ajanı-ışığı-claude-code-ve-codex-clawlight-uyarlaması)
29. [Kablosuz güncelleme (WiFi)](#29-kablosuz-güncelleme-wifi)
30. [Telefon uygulaması (iPhone)](#30-telefon-uygulaması-iphone)

---

## 1. Proje özeti

Rumy, **ESP32-2432S028R** ("Cheap Yellow Display", kısaca CYD) kartı için yazılmış bir **masaüstü arkadaşıdır**.
Siyah ekranda parlayan iki camgöbeği gözden oluşan bir karakterdir. Göz ifadeleriyle duruma tepki verir; aynı ekranda:

- **Saat ve tarih** (NTP ile eşitlenir, Türkiye saati),
- **Hava durumu** (Open-Meteo: anlık, saatlik, 7 günlük, hava kalitesi, deniz suyu sıcaklığı),
- **Marmara depremleri** (AFAD; köprü ve bellek yokken Kandilli),
- **Bayramlar ve anma günleri** (Nager.Date + Diyanet tablosu) ve kişisel önemli günler,
- **WiZ akıllı ampul kontrolü** (UDP, yerel ağ),
- **Bilgisayarda çalan müzik** (Spotify ekranı: kapak, şarkı adı, kontroller) ve **müziği Rumy'nin de
  "dinlemesi"** (kulaklık takıp ritme göre kafa sallaması)

gösterilir. Tüm arayüz Türkçedir. Ayarlar cihazın dokunmatik ekranından, telefondan (ana ekrana eklenen telefon uygulaması, bölüm 13.3) ya da USB seri
porttan yapılabilir. WiFi şifresi ve kişisel ayarlar **kodda değil**, cihazın NVS belleğinde durur.

---

## 2. Donanım

| Parça | Özellik | Rumy'de kullanımı |
|---|---|---|
| İşlemci | ESP32-D0WD-V3, 2 çekirdek Xtensa LX6, 240 MHz | Çekirdek 1: arayüz; çekirdek 0: ağ ve ampul görevleri |
| RAM | 520 KB SRAM (heap olarak ~240 KB kullanılabilir), PSRAM yok | Sabit bufferlar açılışta bir kez ayrılır |
| Flash | 4 MB, QIO 80 MHz | Uygulama (1,81 MB), NVS, kapak önbelleği (960 KB) |
| Ekran | 2,8" 320×240 TFT, **ILI9341** (bazı kartlarda ST7789), SPI | 40 MHz SPI, DMA, donanım kaydırması |
| Dokunmatik | XPT2046 dirençli, ayrı SPI | Yazılımsal SPI 1 MHz, kalibrasyon + yön düzeltmesi |
| microSD | SPI (VSPI) | İsteğe bağlı: kişisel günler, bayram tablosu, veri önbelleği |
| RGB LED | GPIO 4/16/17 | Açılışta kapatılır |
| USB-seri | CH340 | Yükleme (otomatik reset) ve 460 800 baud seri hat (komutlar + Spotify köprüsü) |
| Arka ışık | GPIO 21, PWM | Parlaklık ve gece kısılması |

**Pin tablosu** (`src/app/config.h`):

| Sinyal | Pin | Sinyal | Pin |
|---|---|---|---|
| TFT SCLK / MOSI / MISO | 14 / 13 / 12 | TFT DC / CS / BL | 2 / 15 / 21 |
| Dokunmatik SCLK / MOSI / MISO | 25 / 32 / 39 | Dokunmatik CS / IRQ | 33 / 36 |
| SD SCK / MISO / MOSI / CS | 18 / 19 / 23 / 5 | LED R / G / B | 4 / 16 / 17 |

Dokunmatik ham değer aralığı: X 300–3900, Y 200–3700. ILI9341'li kartta dokunmatik yönü (`offset_rotation`) **6**'dır
(kartta X ekseninin ters olduğu bulunup düzeltildi; ST7789 için 0).

---

## 3. Kullanılan diller, kütüphaneler ve araçlar

### 3.1 Diller

| Dil | Nerede | Neden |
|---|---|---|
| **C++17** (`-std=gnu++17`) | Kartın yazılımı (`src/`, `lib/rumy_core/`), simülatör (`sim/`), birim testleri (`test/`) | ESP32'de Arduino çerçevesi C++ ile çalışır; `constexpr`, isim alanları, şablonlar (`store::putT`) kullanıldı. İstisna ve RTTI yok, dinamik bellek yalnızca açılışta. |
| **C** | LVGL kütüphanesi, üretilmiş font dosyaları (`src/ui/fonts/*.c`) | LVGL C ile yazılmıştır; `lv_conf.h` ile yapılandırıldı. |
| **Python 3.10** | `tools/` altındaki tüm araçlar: derleme betikleri, ikon/font üreticileri, simülatör çalıştırıcı, dayanıklılık kaydedici, **Spotify köprüsü** | Hızlı yazılır, Pillow/numpy gibi kütüphaneleri var, PlatformIO betikleri de Python'dur. |
| **HTML + CSS + JavaScript** | Telefon uygulaması (`web/index.html`, `web/icon.png`) | Tek dosya; derlemede gzip'lenip flash'a gömülür. |
| **PowerShell / Bash** | Geliştirme sırasında derleme, yükleme ve ölçüm komutları | Windows geliştirme ortamı. |

### 3.2 Kart tarafı kütüphaneler (PlatformIO `lib_deps`)

| Kütüphane | Sürüm | Görevi |
|---|---|---|
| **Arduino-ESP32** (espressif32 platformu `6.13.0`) | 2.0.17 (ESP-IDF 4.4 tabanlı) | FreeRTOS, WiFi, NVS (`Preferences`), SD, UART, esp_partition |
| **LVGL** | 9.5.0 | Arayüz çizimi: nesneler, stiller, animasyon zamanlayıcıları, yazılımsal çizim motoru |
| **LovyanGFX** | 1.2.30 | ILI9341/ST7789 sürücüsü (SPI+DMA), XPT2046 dokunmatik, ilk kurulum ekranı |
| **ArduinoJson** | 7.4.3 | Hava/deprem/bayram JSON'larının akış hâlinde, filtreli ve sabit arenada ayrıştırılması |
| **WiFiManager** (tzapu) | 2.0.17 | İlk kurulumda "Rumy-Kurulum" erişim noktası ve WiFi seçim sayfası |
| mbedTLS, lwIP | ESP-IDF içinde | HTTPS (AFAD, Nager; bellek yetmezse köprü/Kandilli, bölüm 10.6) ve TCP/UDP |

### 3.3 Bilgisayar tarafı araçlar ve kütüphaneler

| Araç / kütüphane | Kullanıldığı yer |
|---|---|
| **PlatformIO** (penv içindeki `platformio.exe`) | Derleme, yükleme, birim testleri, ortamlar (`cyd`, `cyd_st7789`, `cyd_endurance`, `cyd_prof`, `native`) |
| **esptool** 4.x | Flash yazma/okuma (yedek alma, sadece uygulama güncelleme) |
| Xtensa GCC (PlatformIO toolchain) | Kart için derleyici |
| **MinGW-w64 g++** | Kartsız önizleme (simülatör) derleyicisi |
| **Unity** | `pio test -e native` birim test çerçevesi |
| **Pillow** | İkon dönüştürme, önizleme PNG'leri, kapak resmi işleme |
| **resvg-py, pypng, lz4** | Meteocons SVG → PNG → LVGL ikon hattı |
| **Node.js + lv_font_conv 1.5.3** | Inter ve Font Awesome'dan glif alt kümesi fontları üretme |
| **pyserial** | Seri port (dayanıklılık kaydı, Spotify köprüsü, testler) |
| **winrt** (`winrt-Windows.Media.Control` vb.) | Windows'un medya oturumundan (SMTC) çalan şarkıyı ve kapağı okuma |
| **numpy + pyaudiowpatch** | Ses çıkışını (WASAPI loopback) dinleyip ritim/tempo bulma |
| **pycaw** | Yalnızca geliştirme sırasında ses düğmelerinin Windows sesini değiştirdiğini doğrulamak için |

### 3.4 Veri ve varlık kaynakları

| Kaynak | Lisans | Kullanımı |
|---|---|---|
| Meteocons (Bas Milius) | MIT | Hava ikonları (64/48/28/24 px) |
| Inter (Rasmus Andersson) | SIL OFL 1.1 | Tüm yazılar (12/16/22 px ve 56 px rakamlar) |
| Font Awesome 5 Free | OFL (font), CC BY 4.0 (simgeler) | Simgeler (ampul, WiFi, oynat, Spotify logosu...) |
| Open-Meteo | CC BY 4.0 | Hava, hava kalitesi, deniz |
| AFAD | Kamuya açık servis | Deprem listesi |
| Kandilli Rasathanesi (Boğaziçi Ünv.) | Kamuya açık sayfa, şifresiz | Köprü ve HTTPS belleği yokken deprem listesi |
| Nager.Date | MIT | Resmî tatiller |
| Mozilla CA deposu (certifi) + GlobalSign | – | Kırpılmış kök sertifika paketi |

---

## 4. Klasör yapısı

```
Rumy/
├── platformio.ini          Ortamlar, derleme bayrakları, sürüm (1.3.0), panel seçimi
├── partitions/rumy_4mb.csv Flash bölüm tablosu (nvs, otadata, app0, updater, coredump, art)
├── updater/                Kablosuz güncellemenin güncelleyicisi: ota_1'de duran küçük ayrı uygulama
│                           (platformio.ini, src/main.cpp, copy_bin.py; bölüm 29)
├── include/
│   ├── lv_conf.h           LVGL yapılandırması
│   └── rumy_prof.h         Profil ölçüm makroları (yalnızca cyd_prof)
├── src/                    Kart yazılımı (C++)
│   ├── main.cpp            Açılış sırası ve ana döngü
│   ├── app/                config.h (tüm sabitler), mem (sabit bufferlar), store (görevler arası veri),
│   │                       shared.h (ortak veri yapıları), portal (ilk kurulum), app (yeniden başlatma istekleri)
│   ├── board/              display (LovyanGFX + LVGL köprüsü, donanım kaydırma), lgfx_cyd.h (panel ayarı), sdcard
│   ├── ui/                 ui.cpp (ekran yöneticisi), scr_*.cpp (her ekran), theme, draw_util, slide (geçiş),
│   │                       icons + icons_embedded (flash'taki ikonlar; 480×320 için icons_big), record_frames (+_big),
│   │                       bench, lvmem, rows, fonts/
│   ├── rumy/               rumy.cpp (yüz, ifadeler, animasyonlar, kulaklık), accessories (+_big; bayrak/şapka/şemsiye)
│   ├── media/              media.cpp (Spotify köprüsü protokolü, kapak önbelleği, tempo), camera.cpp (yazıcı
│   │                       kamerası karelerini seri porttan akış hâlinde çözüp panele çizer), fwupdate.cpp
│   │                       (kablosuz güncellemeyi alır, bölüm 29)
│   ├── printer/            printer.cpp (Creality K2'ye yerel WebSocket istemcisi, ağ görevinde çalışır)
│   ├── net/                net.cpp (ağ görevi ve zamanlayıcı), http_min (küçük HTTP/HTTPS istemcisi),
│   │                       ca_bundle.h (kök sertifikalar), cache (SD önbelleği)
│   ├── weather/ quake/ events/ clock/ settings/ wiz/ webui/ diag/
├── lib/rumy_core/src/      Arduino'ya bağımlı olmayan, bilgisayarda test edilen mantık (ayrıştırıcılar,
│                           önemli gün hesabı, WiZ protokolü, önbellek biçimi, Türkçe metinler, köprü
│                           protokolü media_proto, hava türü wx_mood, takım paleti color)
├── test/                   19 test takımı (168 test) + fixtures/ (API ve yazıcı cevaplarından örnekler)
├── sim/                    Kartsız önizleme: sim_main.cpp (senaryo), sim_variants.cpp (uç durumlar),
│                           sim_audit.cpp (otomatik ekran denetimi), sim_stubs.cpp (donanım taklitleri),
│                           denetim_izin.txt (bilerek böyle olan denetim bulguları)
├── pi/                     Raspberry Pi sürümü (deneysel): Makefile, lv_conf.h, include/ (Arduino/ESP-IDF
│                           çağrılarının Linux karşılıkları), src/ (ekran/DRM, dokunmatik, ağ, dosya, saat, TCP köprü,
│                           sağlık kaydı, servis bekçisi), kurulum.sh, kurulum2.sh ve kurulum3.sh (yönetici
│                           adımları), spi_hiz.sh (ekran hızı),
│                           rumy.service (kullanıcı servisi)
├── tools/                  Python araçları (bkz. bölüm 3 ve 20); tools/go2rtc/ (go2rtc.exe, .gitignore'da);
│                           pi_deploy.py (Pi'ye gönder, orada derle, yeniden başlat)
├── web/index.html, icon.png Telefon uygulaması ve ana ekran simgesi
├── sd_card/                SD karta kopyalanabilecek içerik ve ikon kaynakları (img/)
├── docs/                   Önizleme görüntüleri, ikon RAM tablosu
├── dist/                   Derleme çıktıları (.bin) ve boyut raporları (.gitignore'da)
├── certs/                  Kök sertifika kaynakları
├── LICENSE, LICENSES/       MIT lisansı ve üçüncü taraf lisans metinleri (THIRD_PARTY_NOTICES.md)
├── .github/                GitHub Actions derlemesi, hata ve öneri şablonları
├── README.md               Kullanım kılavuzu
├── CHANGELOG.md            Sürüm notları
├── CONTRIBUTING.md, SECURITY.md
└── Rumy_Proje_Dokumani.md  Bu doküman
```

**Kod miktarı** (üretilmiş font/ikon/web dosyaları hariç): yaklaşık **18 000 satır**. En büyük dosyalar:
`src/rumy/rumy.cpp` (yüz ve animasyonlar), `tools/spotify_bridge.py`, `src/ui/scr_home.cpp`,
`src/ui/scr_weather.cpp`, `src/ui/ui.cpp`, `src/ui/scr_light.cpp`, `src/media/media.cpp`, `src/wiz/wiz.cpp`.
Üretilmiş dosyalar: `src/ui/icons_embedded.cpp` (148 KB ikon verisi), `src/ui/record_frames.*` (24 plak karesi,
30 KB; `tools/make_record.py`), `src/ui/fonts/*.c` (4 font), `src/webui/web_assets.h` (gzip'li web sayfası),
`src/app/portal_font.h` (kurulum ekranı fontu), `src/ui/match_art.*` (lig amblemleri ve dönen top karesi;
`tools/make_match_art.py`).

---

## 5. Genel mimari

### 5.1 Açılış sırası (`src/main.cpp`, `setup()`)

1. `Serial.setRxBufferSize(2304)` ve `Serial.begin(460800)`: seri hat, Spotify köprüsünün 2 KB'lık kapak
   parçalarını tamponlayabilecek büyüklükte açılır.
2. RGB LED kapatılır, `store::init()` (görevler arası veri alanları), `settings::load()` (NVS'ten ayarlar),
   açılış sayacı artırılır, `diag::begin()` (sıfırlanma nedeni kaydı).
3. WiFi hiç ayarlanmamışsa `portal::run()`: LVGL başlatılmadan, LovyanGFX ile sade bir kurulum ekranı ve
   WiFiManager erişim noktası açılır (bkz. 13.2).
4. `mem::init()`: bütün büyük bufferlar **bir kez** ayrılır (LVGL havuzu 36 KB, iki çizim bufferı 2×20 KB,
   JSON arenaları). Başarısızsa ekrana hata yazılıp 10 sn sonra yeniden başlanır.
5. `display::begin()` (panel + dokunmatik), `lv_init()`, `display::startLvgl()` (LVGL ekranı, dokunmatik
   girdisi, donanım kaydırma ayarı).
6. `sdcard::begin()` (kart yoksa uyarı kaydı, çalışma devam eder), önbelleklerden son hava ve deprem verisi,
   `events::begin()`.
7. `media::begin()`: flash'taki kapak bölümü eşlenir, dolu yuvalar sayılır.
8. `ui::begin()`: tema, ana ekran oluşturulur ve yüklenir.
9. `net::begin()`: ağ görevi (çekirdek 0) başlar; WiFi bağlantısı, NTP, veri çekme zamanlayıcısı.
10. `wiz::begin()`: ampul görevi (çekirdek 0).
11. Her adımdan sonra kalan boş heap ölçülüp tek satırda seri porta yazılır (`bos bellek: basla ... wiz ...`).

`loop()` her turda: `ui::loop()` (LVGL zamanlayıcıları, çizim, ekran geçişleri, veri yoklama) →
`diag::loop()` (seri komutlar + köprü satırları) → `media::loop()` (bağlantı ve aktarım zaman aşımları) →
`app::poll()` (bekleyen yeniden başlatma istekleri).

### 5.2 Görevler ve çekirdekler

| Görev | Çekirdek | Öncelik | Yığın | İş |
|---|---|---|---|---|
| Arduino `loopTask` (UI) | 1 | 1 | 8 KB | LVGL, bütün ekranlar, seri hat, Spotify köprüsü, kapak yazma |
| `net` | 0 | 3 | 10 KB | WiFi, NTP, HTTP/HTTPS istekleri, JSON ayrıştırma, SD önbellek |
| `wiz` | 0 | 4 | 4 KB | UDP ile ampul keşfi, durum okuma, komut gönderme |

**Kural:** LVGL çağrıları **yalnızca UI görevinde** yapılır. Diğer görevler sonuçlarını `store`'a yazar; UI
görevi 250 ms'de bir yoklar.

### 5.3 Veri akışı: `store` (`src/app/store.*`)

`store`, görevler arası paylaşılan verinin tek adresidir. Her **alan** (domain) için bir kopya ve bir sürüm
numarası tutar: `FORECAST, DAILY, AIR, MARINE, QUAKE, EVENTS, BULB, NET, LVGL, SETTINGS`.

- Üretici (ör. ağ görevi) `store::putT(store::FORECAST, veri)` ile kilit altında kopyalar, sürüm artar.
- UI görevi `pollData()` içinde `store::getT(alan, kopya, görülenSürüm)` çağırır; yalnızca sürüm değiştiyse
  kopyalar ve ilgili bit maskesini (`M_FORECAST`, `M_BULB`...) açar.
- `dispatch(maske)`: ana ekrana ve o an açık ekrana `update(maske)` çağrısı gider. Her ekran yalnızca kendisini
  ilgilendiren maskelere göre ilgili nesneleri geçersiz kılar (yeniden çizdirir).
- Saniyelik olaylar (`M_SECOND`), dakika (`M_MINUTE`), gece modu (`M_NIGHT`) ve medya (`M_MEDIA`, Spotify köprüsü)
  da aynı maske sistemiyle dağıtılır.

Bu yapı sayesinde görevler birbirini beklemez, LVGL tek görevde kalır ve yeniden çizim yalnızca gerektiğinde olur.

### 5.4 Ekran yöneticisi (`src/ui/ui.cpp`)

- Ekran kimlikleri: `SCR_HOME, SCR_WEATHER, SCR_LIGHT, SCR_SETTINGS` (yatay kaydırmayla gezilen ana dizi) ve alt
  ekranlar `SCR_DIAG, SCR_PAIR, SCR_LOCATION, SCR_NIGHT, SCR_INFO, SCR_EVENT, SCR_DISPLAY_CFG, SCR_NIGHT_CFG,
  SCR_MEDIA`.
- **Ana ekran bellekte kalıcıdır**; diğer ekranlar açılırken oluşturulur, kapanınca silinir (LVGL havuzunu
  korumak için).
- `go(ekran, yön)` bir sonraki turda uygulanacak geçişi kaydeder; `doGo()` sırasıyla: **eski ekranı siler** (ana
  ekran değilse), havuzda yeni ekrana yetecek yer olup olmadığına bakar (`roomFor`), ekranı oluşturur, günceller ve
  donanım kaydırmalı geçişi (`slide::run`) başlatır. Eski ekranın görüntüsü kaydırma boyunca panelin kendi belleğinde
  durduğu için LVGL nesnelerine gerek yoktur; bu sıra sayesinde iki ekran hiçbir zaman aynı anda bellekte olmaz
  (havuz tepesi 22,8 KB → 16,2 KB).
- **Bellek yetmezse:** her ekranın ölçülmüş ihtiyacı (%25 paylı, ör. Ayarlar 13,5 KB, Hava 10 KB, Spotify 6,5 KB)
  bir tabloda durur. Havuzdaki boş alan bundan azsa ya da en büyük boş blok 2 KB'tan küçükse ekran açılmaz:
  "Bellek yetersiz, ekran açılamadı" uyarısı çıkar, Rumy ana ekranda kalır, seri porta neden yazılır (en fazla
  5 sn'de bir) ve `rapor`'daki "reddedilen ekran" sayacı artar. Eskiden bu durumda LVGL `abort()` ile kartı yeniden
  başlatıyordu.
- Hareketler: sola/sağa kaydırma ana dizide gezinir; alt ekranlarda sağa kaydırma geri döner; ana ekranda sağa
  kaydırma (köprü bağlıysa) Spotify ekranını açar; Spotify ekranında sola kaydırma ana ekrana döner.
- Otomatik kurallar (`perSecond()`):
  - Gece modu saatindeyse ve dokunulmamışsa gece ekranına geçilir, çıkınca ana ekrana dönülür.
  - Alt ekranlarda belirlenen süre (varsayılan 60 sn) dokunulmazsa ana ekrana dönülür (Spotify ekranı hariç).
  - Spotify ekranında müzik 1 dk duraklatılmış ve 1 dk dokunulmamışsa ana ekrana dönülür.
  - Önemli günün ilk açılışında tam ekran kutlama kartı gösterilir.
- Müzik **başladığında** ana ekrandaysak Spotify ekranına geçilir (şarkı değişimlerinde geçilmez); köprü
  bağlantısı koparsa Spotify ekranından çıkılır.

---

## 6. Bellek ve flash yerleşimi

### 6.1 RAM bütçesi

Büyük bufferlar açılışta, WiFi başlamadan önce ayrılır ve hiç serbest bırakılmaz (parçalanmayı önler):

| Kalem | Boyut |
|---|---|
| LVGL havuzu (TLSF) | 24 KB (27 Eylül'e kadar 36 KB) |
| Çizim bufferları (DMA, 2 × 32 satır × 320 × 2 B) | 2 × 20 KB |
| JSON arenası / WiZ JSON arenası | 8 KB / 1,5 KB |
| Ağ görevi yığını / WiZ yığını | 10 KB / 4 KB |
| Seri alma tamponu (Spotify köprüsü için) | 2,3 KB |
| WiFi sürücüsü + ağ görevi | ~60 KB |
| SD / FATFS (`max_files = 1`) | ~11 KB |

Ölçülen değerler (kartta, `rapor` komutu): WiFi bağlıyken **boş heap ~80,8 KB** (LVGL havuzu küçültülmeden önce
68,6 KB, Spotify eklenmeden önce 72 KB). HTTPS isteği ancak boş heap ≥ 64 KB ve en büyük blok ≥ 34 KB iken başlatılır
(el sıkışmanın anlık tepesi ~60 KB ölçüldü); değilse ertelenir ve sayaç artar. Eşiğin üstündeki pay 4,6 KB'tan
**~16,8 KB**'a çıktı.

**LVGL havuzunun ekran bazında kullanımı** (kartta `bench bellek` turu; her adımın başında tepe sıfırlanır; müzik
çalarken, Spotify ekranında kapak ve iki satırlık uzun şarkı adıyla ölçüldü):

| Ekran / adım | Kullanım | Tepe |
|---|---|---|
| Ana ekran (kulaklıklı Rumy, notalar) | 5,4 KB | 6,8–7,6 KB |
| Hava (kaydırma, saatlik şerit) | 10,9 KB | 13,2 KB |
| Işık | 10,3 KB | 12,0 KB |
| Ayarlar (kaydırma) | 14,8 KB | **16,2 KB** (en yüksek) |
| Ekran / Gece modu alt sayfası | 12,3 / 11,9 KB | 13,7 KB / – |
| Konum / Ampul / Tanılama / Hakkında | 8,2 / 7,5 / 9,0 / 7,4 KB | – / – / 10,0 KB / – |
| Spotify ekranı | 9,1 KB | 10,5 KB |
| Gece ekranı / Kutlama ekranı | 6,5 / 6,5 KB | 8,0 / 7,8 KB |

Havuz boyutu bu ölçümle belirlendi: en yüksek tepe 16 196 B, üstüne %25 pay (20 245 B), üstüne havuzun kendi
yönetim yükü (TLSF kontrol yapısı + blok başlıkları, tepe anında ~3,2 KB) → 23,5 KB → **24 KB**. Küçültmeden önce
geçişlerde eski ve yeni ekran aynı anda bellekte olduğu için tepe 22,8 KB idi; eski ekranın önce silinmesi
(bölüm 5.4) bunu 16,2 KB'a indirdi.

### 6.2 Flash bölümleri (`partitions/rumy_4mb.csv`)

| Bölüm | Tür | Adres | Boyut | İçerik |
|---|---|---|---|---|
| `nvs` | data/nvs | 0x9000 | 20 KB | Ayarlar (WiFi, ad, konum, parlaklık, gece modu...) |
| `otadata` | data/ota | 0xE000 | 8 KB | Hangi uygulamanın açılacağı (ana yazılım ya da güncelleyici) |
| `app0` | app/ota_0 | 0x10000 | 2,5 MB | Uygulama (25 Ekim: 2 154 976 B = 2,06 MB, bölümün %82'si) |
| `updater` | app/ota_1 | 0x290000 | 448 KB | **Güncelleyici** (421 968 B, bölüm 29) |
| `coredump` | data/coredump | 0x300000 | 64 KB | Çökme dökümü |
| `art` | data (0x40) | 0x310000 | 960 KB | **Albüm kapağı önbelleği** (21 yuva × 44 KB) |

`art` bölümü bu oturumda eklendi; daha önce flash'ın bu son 960 KB'ı boş duruyordu. Tablo değiştiğinde NVS aynı
adreste kaldığı için ayarlar korundu. 25 Ekim'de kablosuz güncelleme için `app0` 2,94 MB'lık `factory` bölümünden
2,5 MB'lık `ota_0`'a indirildi ve boşalan 448 KB'a güncelleyici (`ota_1`) kondu; `nvs`, `coredump` ve `art` aynı
adreste kaldığı için ayarlar ve kapak önbelleği yine korundu. İki tam uygulama bölümü (klasik çift OTA) sığmazdı:
2 × 2,15 MB > 3,01 MB.

### 6.3 Statik bellek

Statik DRAM (.data + .bss, çekirdek dahil) ~60 KB; IRAM ~123 KB / ~128 KB (LVGL'nin sık çağrılan piksel işlevleri
hız için IRAM'e alındı, sınıra yakın).

---

## 7. Ekran, dokunmatik ve çizim altyapısı

### 7.1 Panel sürücüsü (`src/board/lgfx_cyd.h`, `display.cpp`)

- LovyanGFX ile HSPI üzerinde ILI9341 (ya da ST7789), yazma 40 MHz, okuma 16 MHz, DMA.
- Dönüş (rotation) 1: yatay 320×240. Bu dönüşte ILI9341'in MADCTL'i `MV|MX|MH` olur.
- Arka ışık PWM ile yüzde olarak ayarlanır; gece modunda ayrı parlaklık.
- Dokunmatik: XPT2046, yazılımsal SPI 1 MHz (önce 250 kHz idi), kalibrasyon aralığı + `offset_rotation` 6.
- LVGL "flush" geri çağrısı her parçayı DMA ile panele yollar; LVGL renk biçimi `RGB565_SWAPPED` (panelin bayt
  sırası).
- `dokunma ac` seri komutu açıkken her dokunuşun koordinatı seri porta yazılır (kalibrasyon kontrolü; bu oturumda
  eklendi ve ses düğmesi sorununu teşhis etmekte kullanıldı).

### 7.2 LVGL yapılandırması (`include/lv_conf.h`)

- Kısmi çizim (partial rendering), 32 satırlık iki buffer: biri panele giderken diğerine çizilir.
- `LV_MEM_SIZE` 24 KB (önce 36 KB; bkz. 6.1), TLSF ayırıcı, önbellekler kapalı (`LV_CACHE_DEF_SIZE 0`). Havuz
  heap'ten bir kez ayrılır (`LV_MEM_POOL_ALLOC`); `config.h`'deki `LVGL_POOL_BYTES` ile aynı olmalıdır
  (`static_assert`).
- `LV_ATTRIBUTE_FAST_MEM = IRAM_ATTR`: piksel doldurma/karıştırma işlevleri IRAM'de.
- `LV_DRAW_SW_CIRCLE_CACHE_SIZE 8`: yuvarlak köşe maskeleri önbelleği.
- `LV_BIN_DECODER_RAM_LOAD 0`: ikonlar flash'tan satır satır çözülür, RAM'e kopyalanmaz.
- SD için `LV_USE_FS_ARDUINO_SD`; profil sürümünde LVGL ölçüm noktaları `rumy_prof_begin/end`'e bağlanır.

### 7.3 Özel çizim nesneleri (`src/ui/draw_util.*`)

RAM'i korumak için ekranların çoğu, onlarca etiket nesnesi yerine **tek bir nesnenin `DRAW_MAIN` olayında**
doğrudan çizilir. Yardımcılar: `du::text` (tek satır metin), `du::box` (yuvarlak köşeli dolgu), `du::hgrad`
(yatay gradyan), `du::hfade` (tek renkten saydama yatay geçiş; kayan yazının kenarlarını yumuşatır),
`du::image` (ikon), `du::Clip` (geçici kırpma bölgesi), `du::visible` (satır görünür mü).
Örnek: ana ekrandaki saat, hava bloğu, tarih satırı, "şimdi çalıyor" şeridi ve çubuklar beş özel çizim nesnesidir;
ana ekran böylece LVGL havuzunda ~5,5 KB yer tutar.

### 7.4 Donanım kaydırmalı sayfa geçişi (`src/ui/slide.cpp`)

İlk sürümde sayfa geçişi her karede tüm ekranı yeniden çizdiği için 3 kare / 5–7 fps kalıyordu. Çözüm, ILI9341'in
**dikey kaydırma** özelliğidir (yatay kullanımda yatay kaydırmaya dönüşür):

1. Açılışta `VSCRDEF (0x33)` ile tüm ekran kaydırma alanı tanımlanır.
2. Geçiş başlarken LVGL'nin ekran yenileme zamanlayıcısı durdurulur, yeni ekran yüklenir ve yerleşimi hesaplanır.
3. Her karede, yumuşak yavaşlayan (cubic ease-out) eğriyle hedef konum bulunur; **yalnızca yeni açılan sütun
   şeridi** çizilir (LVGL'nin geçersiz alan listesi elle ayarlanıp `lv_display_refr_timer` çağrılır), sonra
   `VSCRSADD (0x37)` ile panelin başlangıç satırı kaydırılır: panel eski görüntüyü kendi belleğinde kaydırır.
4. Geçiş bitince zamanlayıcı devam eder, parmağın kalkması beklenir (yanlış dokunma olmasın).

Sonuç: 220 ms'lik geçişte 18–30 kare, en uzun kare 20–35 ms. Farklı panellerde yön ters olursa `kayma ters`
komutuyla çevrilir.

### 7.5 Yazı tipleri (`tools/make_fonts.py`)

- **Inter** (12 px normal, 16 px orta, 22 px yarı kalın) ve saat için **56 px** rakamlar (sabit genişlikli rakam
  düzeltmesiyle, saat değişirken yazı kaymasın diye).
- **Font Awesome** simgeleri aynı fontların içine gömülür (`src/ui/fonts/symbols.h` içinde `RS_BULB`, `RS_WIFI`,
  `RS_PLAY`, `RS_PAUSE`, `RS_NEXT`, `RS_PREV`, `RS_VOL_UP`, `RS_VOL_DOWN`, `RS_MUSIC`, `RS_SPOTIFY`...).
- Yalnızca gereken glifler üretilir (lv_font_conv, 4 bit gri tonlama). Bu oturumda şarkı adları için karakter
  kümesi genişletildi: Latin-1 (é, ñ, ü, ø...) ve Latin Genişletilmiş-A (ő, ł, č, ś...) eklendi; Türkçe harfler
  zaten vardı. Genişletilmiş fontlar ve Spotify kodu birlikte uygulamayı ~74 KB büyüttü (1,74 → 1,81 MB).

### 7.6 İkonlar (`tools/convert_assets.py`, `tools/embed_icons.py`)

Meteocons SVG → resvg ile hedef boyutta PNG → Pillow ile paletli PNG → LVGL'nin `LVGLImage.py` betiğiyle LVGL
ikili biçimi → `embed_icons.py` ile C dizisi (`src/ui/icons_embedded.cpp`, 148 096 B, flash'ta).

| Boyut | Biçim | Kullanım |
|---|---|---|
| 64 px | I8 (256 renk paletli) | Hava ekranı ana ikonu |
| 48 px | I8 | Ana ekran hava bloğu |
| 28 px | I4 (16 renk) | Saatlik şerit, gün doğumu/batımı |
| 24 px | I4 | 7 günlük liste |

v1.1.0'dan önce ikonlar SD karttan okunuyor ve 20 KB'lık RAM önbelleğinde tutuluyordu; hava ekranı açılışı
~300 ms sürüyordu. Flash'a gömülünce 10 ms'ye indi ve 20 KB RAM çizim bufferlarına verildi.

### 7.7 Tema (`src/ui/theme.h`)

Siyah zemin (`C_BG`), göz rengi camgöbeği `0x19D3F0`, neon palet: yeşil `0x2BFF6A`, limon `0xB6FF3B`, camgöbeği
`0x00E0FF`, sarı `0xFFD60A`, turuncu `0xFF9F1C`, kırmızı `0xFF4040`, pembe `0xFF4FA3` (kulaklık), mor
`0xB84DFF`, mavi `0x1E6BFF`; çubuk zemini `0x1B222B`. Sıcaklık değere göre renklenir (`theme::tempColor`).

---

## 8. Ekranlar

### 8.1 Ana ekran (`src/ui/scr_home.cpp`)

Yerleşim (320×240, piksel koordinatları):

| Bölge | Konum | İçerik |
|---|---|---|
| Yüz | 0,0 – 320×88 | Rumy'nin gözleri (bkz. bölüm 9); müzik çalarken kulaklık ve notalar |
| Konum | 8,8 | Yeşil konum simgesi + şehir |
| Ampul düğmesi | 272,4 – 42×42 daire | Çerçeve: sarı açık, gri kapalı, kırmızı ulaşılamıyor; dokun = aç/kapa, uzun bas = Işık ekranı |
| Saat | 6,88 – 182×66 | Saat neon yeşil, iki nokta camgöbeği ve saniyede bir yanıp söner, dakika beyaz (56 px) |
| Hava sütunu | sol kenar x = 184 (`COL_X`), üst y = 95 (`COL_Y`); nesne 174,95 – 146×96 | Saatin sağında sola hizalı tek sütun: en üstte selam ("İyi geceler Deniz", camgöbeği), altında hava ikonu (CYD 64 px, Pi 85 px) + sıcaklığa göre renkli anlık derece (CYD 29 px, Pi 39 px `fontTemp`), onun altında ↑en yüksek (turuncu) ↓en düşük (camgöbeği) (CYD 12 px, Pi 21 px; CYD'de derece ikona 4 px yakın, Pi'de 12 px). Sütunun en üstünde her zaman şarkı satırı vardır, selam ve hava hep `NP_SHIFT` (plak + 5 px) aşağıdadır (3 Ekim'den beri müzik çalarkenki düzen varsayılan; şarkı başlayıp bitince bir şey kaymaz); ikon tarih satırının sağına uzanır. Dokununca Hava ekranı. |
| Tarih | 6,154 – 156×29 (`DATE_W`, sütunun 14 px soluna kadar) | Gün adı sarı, tarih yeşil (22 px), saatin altında ortalı (saat genişliği `clockW()`, sığmazsa sola ya da sağa dayanır). Sığdırma sırası: tam gün adı → kısa gün adı (Pazartesi → Pzt) → kısa ay (`rc::monthShortTr`, "Cum 28 Tem") → gün adı yok. 20 senaryo `ui_preview.py --tarih` ile çizilir |
| Şimdi çalıyor | 184,95 – 124×17 (Pi 245,127 – 219×22) | Sütunun en üstünde: müzik varken solda dönen plak (ortası mor; CYD 17 px, Pi 22 px), sağında küçük yazıyla şarkı adı (CYD 12 px, Pi 16 px), sığmayan ad kayar; müzik yokken %40 saydam duran plak ve gri "Çalan yok". Dokununca Spotify ekranı (ayrıntı aşağıda) |
| Çubuklar | 6,184 – 308×26 | Termometre + sıcaklık çubuğu (günün min–maks aralığında), damla + nem, yaprak + hava kalitesi (AQI'ye göre yeşilden mora) |
| Bilgi kartı | 6,212 – 308×26 | 5 sn'de bir değişir: rüzgâr adı (poyraz, lodos...), 6 saatlik yağış / "Şemsiye al", gün doğumu–batımı ve gün uzunluğu, deniz suyu, hava kalitesi, önemli gün, son deprem; uyarılar (bağlantı yok, SD yok) kırmızı çerçeveli |

Rumy'nin ruh hâli `pickMood()` ile öncelik sırasıyla seçilir: gece uykusu → deprem şaşkınlığı → **müzik dinliyor**
→ göz uykusu (açıksa, ayarlanan süre dokunulmazsa; varsayılan kapalı) → uyanma mahmurluğu (1,2 sn) → bağlantı yoksa üzgün → önemli gün (kutlama/sakin
+ aksesuar) → yağmur yaklaşıyorsa (ve şu an yağmıyorsa) yukarı bakma + şemsiye → **şu anki havaya göre ifade**
(`rc::wxKind` + `rumy::weatherLook`, bölüm 9.6) → normal. Işık açıkken ifade değişmez (30 Eylül'de
kaldırıldı; alttan kesik gözler "kırpık" görünüyordu); gözler yalnızca ampulün rengini hafifçe
alır.

**"Şimdi çalıyor" şeridi** (`drawNowPlaying`, `npMeasure`, `npDrawSpan`, `updateNowPlaying`, `npTick`): saatin sağındaki sütunun en üstünde. 2 Ekim'e kadar tarih satırının sağındaydı; sonra
sütunun başına taşındı.
- **Ne zaman görünür:** köprü bağlı, oturum aktif ve şarkı adı varken. Çalarken plak döner, yazı beyazdır.
  Duraklatılınca plak durur, yazı griye döner, kayan yazı başa döner. 1 dk duraklatılmış kalırsa
  (`MEDIA_PAUSED_HOME_MS`) ya da köprü koparsa şerit soluk plak ve "Çalan yok" hâline geçer (3 Ekim'e kadar
  gizleniyor ve selam/hava yukarı kayıyordu).
- **Plak:** CYD'de 17 px (2 Ekim'e kadar 25 px), Pi'de 22 px çapında; çizim ölçüleri 25 px'e göre tanımlı ve
  `DESIGN_PX` ile boya göre ölçeklenir: 1 px gri kenar, koyu gövde, iki ince oluk halkası, karşılıklı iki açık gri parlama yayı,
  mor (`C_NEON_PURPLE`) etiket, etiket üzerinde karşılıklı iki açık mor yay ve ortada siyah delik. Parlama ve
  etiket yayları döndükçe hareket belli olur. Hız gerçek plak gibi 33⅓ devir/dk (200°/sn); açı 0,1° hassasiyetle,
  geçen süreyle ilerler. Plak her karede çizilmez: `tools/make_record.py` onu bilgisayarda 8 kat büyük çizip
  küçültür (yumuşak kenar) ve 24 dönüş karesini RGB565 olarak `src/ui/record_frames.cpp`'ye gömer (desen 180°'de
  kendini tekrarladığı için yarım tur 24 kare = 7,5°; CYD 13,9 KB, Pi 23,2 KB flash). Kartta her adımda tek bir resim çizilir;
  kare numarası değişmediyse plak alanı geçersiz kılınmaz. İlk sürümde plak 10 ayrı LVGL şekliyle (daire, halka,
  yay) çiziliyordu ve kare başına ~3,5 ms tutuyordu; resimle ~1 ms'ye indi.
- **Kayan yazı:** küçük yazı (CYD 12 px, Pi 16 px), pencere plağın 6 px sağından ekranın sağ kenarının 12 px soluna
  kadar (CYD ~93 px, Pi ~189 px; eski yerinde ~85 px idi). Ad sığıyorsa
  sabit durur. Sığmıyorsa 1,5 sn bekler, saniyede 30 px sola akar; adın arkasından 32 px boşlukla aynı ad gelir
  (dairesel), başa gelince yine 1,5 sn bekler. Sağ kenarda 10 px yumuşak kaybolma, yazı kayarken sol kenarda da.
  Yeni şarkıda baştan başlar. LVGL'nin hazır kayan etiketi kullanılmadı: tek çizim nesnesi havuzdan çok az yer
  tutar ve bekleme süreleri istenildiği gibi ayarlanabilir. Şarkı değişince adın bir kopyası alınır ve her harfin
  bayt konumu ile x konumu bir kez hesaplanır (`npMeasure`); her karede LVGL'ye adın tamamı (76 harfe kadar) değil,
  yalnızca pencereye düşen parça (~12 harf) verilir (`npDrawSpan`), çünkü LVGL her metin çiziminde bütün harfleri
  birkaç kez tarar.
- **Yerleşim:** şerit sütunun başında, x = 184'te (`NP_X = COL_X`), sabit durur. Selam ve hava bloğu her zaman
  `NP_SHIFT` kadar aşağıdadır ve hava ikonu tarih satırının sağına uzanır. Bu yüzden tarih x = 170'te bitmeli:
  önce gün adı kısalır (`rc::dayShortTr`), sonra ay (`rc::monthShortTr`, "Cum 28 Tem").
- **Maliyet:** tek nesne + tek zamanlayıcı (33 ms). Zamanlayıcı yalnızca ana ekran görünürken, şerit açıkken ve
  müzik çalarken çalışır; her karede yalnızca plak alanı ve (kayıyorsa) yazı penceresi geçersiz kılınır. Şerit
  görünürken tarih nesnesi sütunun soluna kadar daraltılır. Aşağıdaki süreler şeridin eski yerinde (tarih
  satırında) CYD'de ölçüldü. Kartta ölçülen kare başına çizim süresi (müzik çalarken ana ekran, 30 fps): yalnız yüz
  10,8 ms, + dönen plak 12,7 ms, + kayan uzun ad 14,5 ms (ilk sürümde 16,4 ms). LVGL havuzunda ~180 B, heap'te
  ~580 B (ad kopyası ve harf konum tabloları). Dokununca Spotify ekranı açılır.

### 8.2 Hava ekranı (`scr_weather.cpp`)

Hava durumuna göre renk değiştiren gradyan zemin, 64 px ikon, sonraki 1 saatin yağış grafiği, yatay kaydırılan
24 saatlik şerit (gün doğumu/batımı dahil), 7 günlük min–maks çubukları ve 10 kutuluk detay ızgarası (hissedilen,
nem, rüzgâr, basınç, UV, görüş, bulut, çiy noktası, hava kalitesi, deniz). Şerit, 7 gün ve ızgara tek nesneli özel
çizimdir.

### 8.3 Işık ekranı (`scr_light.cpp`)

Renk tonu, parlaklık ve beyaz ışık sıcaklığı kaydırıcıları, 8 hazır sahne; komutlar ampul görevine kuyrukla gider,
80 ms'den sık gönderilmez. 3 Ekim revizyonu: başlıkta lambanın renginde ampul (yarı saydam daire
içinde), kaydırıcıların solunda ikon (palet, güneş, termometre), sağında değer; renk satırının sağında lambanın şu
anki rengi. Topuzlar beyaz halkalı ve içi renkli; o an kullanılmayan kipin topuzu içi boş halka (eskiden %50 saydam
çiziliyordu). Parlaklık dolgusu lambanın rengindedir (düz renk; geçiş denendi, LVGL'de çizim tepesini 11 KB
artırdığı için bırakıldı). Sahneler 40 px yüksekliğinde; üstte lambanın o sahnedeki renginde ikon (`fontM`), altta
ad (`fontS`); seçili sahne kendi renginde çerçeve ve %18'lik zeminle gösterilir.

| Sahne | İkon | Ampul komutu |
|---|---|---|
| Sabah | güneş | 6500 K, %100 |
| Çalışma | dizüstü | 5000 K, %100 |
| Okuma | kitap | 4000 K, %80 |
| Yemek | çatal bıçak | 3000 K, %80 |
| Akşam | kanepe | 2700 K, %50 |
| Film | film şeridi | WiZ sahnesi 18 (TV), %40 |
| Mum | alev | WiZ sahnesi 29 (mum ışığı), %50 |
| Gece | ay | WiZ sahnesi 14 (gece lambası), %10 |

Seçili sahne ampulden okunan kip, parlaklık ve sıcaklık/sahne numarası birebir tutunca işaretlenir.

### 8.4 Ayarlar ve alt sayfalar

`scr_settings` (konum, ampul, WiFi sıfırlama, Telefon (`scr_phone`: QR + eşleştirme kodu), deprem eşiği, bakım yeniden başlatması, tanılama,
hakkında, fabrika ayarları), `scr_display` (parlaklık, ana ekrana dönüş süresi, kart süresi, **göz uykusu**,
dokunmatik yönü; yön 10 sn içinde onaylanmazsa geri alınır), `scr_nightcfg` (gece modu: kapalı / güneşe göre /
saatlere göre, gece parlaklığı, başlangıç–bitiş), `scr_location` (81 il), `scr_pair` (ampul eşleştirme),
`scr_diag` (tanılama), `scr_info` (hakkında).

### 8.5 Gece ekranı (`scr_night.cpp`)

Kısık ışık, uyuyan Rumy (hareketli "z"ler), küçük saat ve tarih. Dokununca 30 sn uyanır. Müzik çalıyorsa Rumy
kulaklığı takılı uyur (kulaklık da kısık renkte). Varsayılan gece saatleri **02:00–07:00**.

### 8.6 Kutlama ekranı (`scr_event.cpp`)

Önemli günün ilk açılışında tam ekran: büyük Rumy (kutlama ya da 10 Kasım'da sakin ifade + bayrak), günün adı.

### 8.7 Spotify ekranı (`scr_media.cpp`) ve çalma listeleri (`scr_playlist.cpp`) — bkz. bölüm 15 ve 27.

### 8.8 Yazıcı ekranı (`scr_printer.cpp`) — bkz. bölüm 25.

### 8.9 Maç ekranı (`scr_match.cpp`) ve maç bildirimleri (`match_ui.cpp`) — bkz. bölüm 26.

---

## 9. Rumy'nin yüzü ve animasyonları

Kaynak: `src/rumy/rumy.cpp` ve `rumy.h`. Yüz, tek bir LVGL nesnesidir; her şey `DRAW_MAIN` olayında çizilir. Ana
ekranda 320×88, göz 52 px; gece ve kutlama ekranlarında daha büyük. Tüm ölçüler göz boyutuna oranlanır, bu yüzden
aynı kod her boyutta çalışır.

### 9.1 Göz geometrisi ve ifadeler

Her ifade (`Mood`) birkaç parametreyle tanımlanır (`Params`): genişlik `w`, yükseklik `h`, kaydırma `dx/dy`,
yorgun (`tired`, üst köşeden üçgen kesik), kızgın (`angry`), mutlu (`happy`, alttan yuvarlak kesik: "^ ^"),
düz (`flat`, üstten düz kesik), baş konumu (`head`: gözleri, kulaklığı ve notaları birlikte aşağı indirir; yalnızca
MUSIC'te 0,12e ≈ 6 px). Parametreler dörtte bir piksel hassasiyetinde (`Q = 4`) tutulur ve her karede
hedefe doğru üçte bir oranla yumuşakça yaklaşır (`ease`).

| İfade | Görünüm | Ne zaman |
|---|---|---|
| NORMAL | Kare yuvarlak gözler | Varsayılan, parçalı bulutlu |
| HAPPY | Alttan derin kesik "^ ^" + pembe yanaklar, ara sıra zıplama | Güneşli, karlı |
| SLEEPY | Ağır kapaklı; 5,6 sn'de bir gözler yavaşça kapanıp birden açılır (uyuklama) | Uyanırken 1,2 sn; bunaltıcı sıcak |
| ASLEEP | İnce çizgi, aşağıda | Gece, göz uykusu |
| SAD | Düşük kapaklı, aşağıda; 4,2 sn'de bir gözyaşı | Bağlantı yok, yağmurlu |
| SURPRISED | Çok büyük (1,15 × 1,36), girişte bir an daha da büyüyüp oturur, titreme | M4+ deprem (30 sn) |
| LOOKUP | Yukarı bakan | Yağmur yaklaşıyor ya da çisenti (+ şemsiye) |
| CELEBRATE | Mutlu, yanaklar, sık zıplama, renkli konfeti | Önemli gün (+ bayrak/parti şapkası) |
| CALM | Düz üst kesik | 10 Kasım (+ bayrak), kapalı, açık gece |
| **MUSIC** | Keyifle kısılmış "^ ^", göz kırpmaz, etrafa bakmaz | Bilgisayarda müzik çalarken |
| SQUINT | Kısık, iç köşeler aşağı (`angry`) | Sisli, rüzgarlı |
| SCARED | İri, endişeli, gözler birbirine yaklaşık (`pinch`) | Fırtınalı |
| SQUEEZE | `>  <` sıkılı gözler (çizgiyle çizilir) | Soğuk |

Her gözün etrafında iki yarı saydam katmanla **parıltı** (glow) çizilir.

### 9.2 Hareketler

- **Göz kırpma:** 3–7 sn aralıkla, 150 ms; bazen art arda iki kez.
- **Bakınma:** 2,5–7,5 sn'de bir rastgele bakış yönü (üzgün, uykulu, sakin ve müzikte bakınmaz).
- **Dokunma:** Rumy'ye dokununca dokunulan noktaya bakar ve göz kırpar (`poke`).
- **Zıplama:** mutlu ve kutlama ifadelerinde.
- **Titreme:** şaşkınlıkta 900 ms.
- **Kare hızı:** normalde 30 fps (33 ms), uyurken 20 fps (50 ms). Yeniden çizim yalnızca görünüm değiştiğinde
  yapılır: her karede görünümün özeti (`signature`) hesaplanır, değişmediyse hiçbir şey çizilmez; değiştiyse
  yalnızca yüzün kapladığı dikdörtgen (`faceBox`) geçersiz kılınır.

### 9.3 Hareketli uyku "z"leri (bu oturumda eklendi)

Eskiden uyurken sağ gözün yanında sabit "Z z" yazısı vardı. Şimdi `drawZs()`:
- 3 adet "z", 3,6 sn'lik döngüde 1,2 sn arayla doğar.
- Her biri **çizgiyle** çizilir (üst çizgi, çapraz, alt çizgi) ki boyutu kesintisiz büyüyebilsin (5→14 px).
- Yolu: sağ gözün üst köşesinden çapraz yukarı-sağa; sinüs ile ±3 px salınır; ilk %15'te belirir, son %38'de söner.
- Yumuşak parıltı için önce kalın ve saydam, sonra ince ve parlak çizilir.
- Yalnızca z'lerin küçük alanı her karede geçersiz kılınır; ekranın geri kalanı çizilmez.
- Kulaklık takılıysa z'ler kulaklık kapağının yanından çıkar.

### 9.4 Kulaklık, notalar ve ritim hareketi — bkz. bölüm 16.

### 9.5 Aksesuarlar (`src/rumy/accessories.*`)

Bayrak, parti şapkası ve şemsiye, 32 px I4 paletli resimler (convert_assets ile üretilir). Müzik çalarken
aksesuarlar gösterilmez (kulaklıkla çakışmasınlar diye).

### 9.6 Hava durumu ifadeleri ve efektler (1 Ekim 2026)

Taslaklar simülatörde denendi; kaş denemesi beğenilmeyip kaldırıldı, güçlendirilmiş hâl seçildi.

- **Sınıflandırma (`rumy_core/wx_mood`, testli):** `wxKind(WxNow)` öncelik sırası: fırtına (95–99) → kar (71–77,
  85–86) → yağmur (61–67, 80–82) → çisenti (51–57) → sis (45, 48) → rüzgar (hamle ≥50 ya da rüzgar ≥35 km/sa) →
  soğuk (hissedilen ≤2 °C) → bunaltıcı (≥33 °C) → açık gündüz (≥27 °C ya da UV ≥6 ise "güneşli ve sıcak") → açık
  gece → kapalı (3) → yok. Hissedilen yoksa sıcaklık kullanılır.
- **Görünüm (`rumy::weatherLook`):** her tür için ruh hâli + aksesuar + efekt; tablo README'de.
- **Efektler (`rumy::Effect`, `rumy.cpp`):** parçacıklar zamanın saf işlevi (`part(t)`); çizim ve geçersiz kılma
  aynı işlevi kullanır. Her karede parçacığın yalnızca eski ve yeni konumunu kapsayan küçük dikdörtgen geçersiz
  kılınır (en fazla ~10 alan; LVGL sınırı 32). Ruh hâline bağlı ek efekt: üzgünde gözyaşı, kutlamada konfeti.
- **Yüz hareketleri:** uyuklama (`doze`), fırtınada irkilme (`wide/doze`), rüzgarda yaslanma ve soğukta titreme
  (`swayX`), kapalıda buluta bakma ve siste bakınma (bakış yönü), soğukta buz mavisi göz rengi.
- **Fırtına parlaması** yüz şeridinin tamamını 4,8 sn'de iki kez yeniler; diğer efektler küçük alanlar.
- **Deneme:** seri komut `hava <tür>` (2 dk), `hava kapat`.
- **Simülatör:** `hava_*` (12 tür, ana ekran) ve `rumy_ifadeler_tum_1..4` (13 ruh hâli, gerçek boyut).

---

## 10. Ağ, veri kaynakları ve önbellek

### 10.1 Ağ görevi (`src/net/net.cpp`)

WiFi bağlantısını yönetir (kopunca 20 sn'de bir yeniden dener), NTP'yi başlatır ve bir iş zamanlayıcısıyla
kaynakları sırayla çeker. Her kaynağın kendi periyodu ve hata sonrası **üstel geri çekilmesi** vardır (30 sn'den
15 dk'ya). Saat eşitlenmeden çekilen veri "eşitlenmemiş" işaretlenir ve saat gelince yeniden çekilir (eski bir
hatada 1970 tarihli zaman damgası çıkıyordu).

| Kaynak (`rapor` adı) | Sunucu | Protokol | Periyot |
|---|---|---|---|
| Anlık + saatlik + 7 günlük hava (`hava+7gun`) | api.open-meteo.com | HTTP | 15 dk, tek istek |
| Deniz suyu (`deniz`) | marine-api.open-meteo.com | HTTP | 60 dk |
| Hava kalitesi (`havakalitesi`) | air-quality-api.open-meteo.com | HTTP | 60 dk |
| Deprem (`afad`) | servisnet.afad.gov.tr | HTTPS | 10 dk |
| Resmî tatiller (`nager`) | date.nager.at | HTTPS | 24 saat |
| Maç (`mac`) | site.api.espn.com, sports.core.api.espn.com | HTTP | 6 saat; maç sırasında 20 sn (bölüm 26) |

(Dayanıklılık sürümünde periyotlar 30–60 sn'ye iner.) Anlık/saatlik ve 7 günlük veri iyileştirme 2'den beri tek istekte gelir (`current` + `minutely_15` + `hourly` + `daily` aynı adreste); store'daki `FORECAST` ve
`DAILY` alanları ayrı kaldı, arayüz değişmedi. 7 günlük veri artık 15 dk'da bir geldiği hâlde SD'ye her seferinde
yazılmaz: önbellek katmanı (`cache::save`) zaman damgası dışındaki içerik değişmediyse yazmaz, değişmese de saatte
bir tazeler (`REWRITE_AFTER_S`). İlk çekmeler WiFi bağlandıktan sonra 3 / 30 / 45 / 60 / 90. sn'de
başlar; hata olursa her kaynak 30 sn'den 15 dk'ya kadar üstel geri çekilir.

### 10.2 Küçük HTTP istemcisi (`src/net/http_min.*`)

Arduino'nun `HTTPClient`'ı yerine yazılmış, bellek dostu istemci: bağlanma 6 sn, okuma 10 sn zaman aşımı,
`chunked` aktarım çözme, cevap gövdesini akış (`ByteSource`) olarak sunar; JSON doğrudan akıştan ayrıştırılır,
tam gövde RAM'e alınmaz. Hata kodları: bağlantı, zaman aşımı, protokol, TLS belleği, ayrıştırma vb.

### 10.3 HTTPS ve sertifikalar

`tools/gen_ca_bundle.py`, yalnızca AFAD ve Nager'ın ihtiyaç duyduğu kök sertifikaları Mozilla deposundan (certifi)
seçip `src/net/ca_bundle.h`'ye yazar. "GlobalSign Root CA" artık o depoda olmadığı için GlobalSign'ın resmî
adresinden indirilip SHA-256 parmak iziyle doğrulanır. TLS için mbedTLS 16 KB giriş/çıkış bufferları kullanır;
el sıkışma ~60 KB anlık bellek ister, bu yüzden bellek eşiği kontrolü vardır (bölüm 6.1). Telefon uygulaması kullanılırken
(son istekten 20 sn sonrasına kadar, `webui::busy()`) HTTPS istekleri ertelenir.

### 10.4 JSON ayrıştırma (`lib/rumy_core/src/openmeteo.cpp`, `afad.cpp`, `nager.cpp`)

ArduinoJson 7 ile, sabit 8 KB'lık bir arena (`JsonArena`) üzerinde, **filtreli** ayrıştırma: yalnızca gereken
alanlar belleğe alınır. Sonuçlar küçük sabit boyutlu yapılara (`rc::Forecast`, `rc::Daily`, `rc::QuakeInfo`...)
dönüştürülür. Bu kod Arduino'ya bağımlı değildir ve bilgisayarda gerçek API cevaplarıyla test edilir.

Birleşik hava cevabı `omParseWeather` ile tek belgede ayrıştırılır. Filtre ve okuma kodu parça ayrıştırıcılarıyla
(`omParseForecast`, `omParseDaily`) ortaktır; bir test aynı cevaptan ikisinin de bayt bayt aynı yapıyı ürettiğini
doğrular. Filtre anahtarları statik dizgi olarak verilir (`JsonString(k, true)`), böylece arenaya kopyalanmaz.
Arenanın açılıştan beri en yüksek kullanımı `rapor`'un `[ag]` satırında (`json tepe`) görünür; ağ görevindeki bütün
ayrıştırmalar (hava, AFAD, Nager, yazıcı) aynı 8 KB'ı sırayla kullanır.

### 10.5 Önbellek (`src/net/cache.*`, `lib/rumy_core/src/cache_fmt.*`)

Son başarılı hava ve deprem verisi SD karttaki `/rumy/cache` altına sürümlü, sağlama toplamlı bir ikili biçimde
yazılır; açılışta buradan okunur ki WiFi gelmeden ekran dolu olsun. Saat eşitlenmemişken önbelleğe yazılmaz.

---

## 11. Önemli günler, deprem, saat

- **Saat** (`src/clock/clock.cpp`): `configTzTime("<+03>-3", "pool.ntp.org", "time.google.com",
  "tr.pool.ntp.org")`; Türkiye UTC+3, yaz saati yok.
- **Önemli günler** (`lib/rumy_core/src/events_calc.cpp`, `src/events/events.cpp`):
  - Milli bayramlar ve anma günleri sabit tablodan.
  - Ramazan ve Kurban bayramı: Nager.Date'ten; Nager'ın "geçici" işaretli tarihleri varsa gömülü Diyanet
    tablosu tercih edilir.
  - Kişisel günler (doğum günü, yıl dönümü) telefon uygulamasından eklenir, SD'de `/rumy/events.json`'da tutulur; yıl
    girilirse "34. yaş", "10. yıl" gibi gösterilir.
  - Bugünün olayları için Rumy kutlar (kutlama ifadesi + aksesuar), ilk açılışta tam ekran kart çıkar.
- **Deprem** (`src/quake/quake.cpp`, `afad.cpp`, `kandilli.cpp`): AFAD'dan son 2 günün Marmara bölgesi depremleri; eşik
  ayarlanabilir (varsayılan M2,0). M4 ve üstü yeni bir deprem 1 saat boyunca kartta sabitlenir, Rumy 30 sn şaşırır
  ve uyuyorsa uyanır. Kaynak sırası (27 Ekim, bölüm 10.6): köprü bağlıysa AFAD'ı bilgisayar çeker; değilse bellek
  yetiyorsa kart HTTPS ile çeker; o da olmazsa Kandilli Rasathanesi'nin şifresiz listesi kullanılır.

### 10.6 Köprü üzerinden HTTPS ve Kandilli yedeği (`src/net/relay.*`, `tools/veri_koprusu.py`)

**Sorun (27 Ekim ölçümü):** statik RAM Eylül'den bu yana 60,8 → 89,2 KB büyüdü (maç, lig, Spotify listeleri, ajan
ışığı, kablosuz güncelleme, telefon uygulaması); boş heap açılıştan sonra ~36–40 KB, uzun çalışmada en fazla ~46 KB.
HTTPS ise ≥ 64 KB boş + ≥ 34 KB blok istiyor (el sıkışma tepesi 47–58 KB ölçülmüştü). Sonuç: 38 saatlik bir
açılışta AFAD ve Nager 4 598'er kez ertelendi, hiç çekilmedi. Ölçülen paylar (12'şer örnek): web sunucusu + mDNS
~4,5 KB, boştaki yazıcı bağlantısı ölçülemeyecek kadar küçük.

**Çözüm (köprü + Kandilli):**
- Rumy `@veri afad <yol>` (kartın kurduğu AFAD yolunun aynısı) ya da `@veri nager <yıl>` der. Köprü
  (`tools/veri_koprusu.py`) bilgisayarda HTTPS ile çeker, kartın ayrıştırıcılarıyla aynı kurallarla özetler ve tek
  satır döndürür: `@deprem <24 sa sayısı> <son: zaman büyüklük×10 derinlik×10 enlem×1e4 boylam×1e4> <en büyük: ...>
  \t<yer>\t<yer>` ya da `@bayram <yıl> <ramazan ay gün> <kurban ay gün> <kesin değil 0|1>`. Kart 12 sn bekler
  (`cfg::RELAY_TIMEOUT_MS`); isteği ağ görevi hazırlar, döngü görevi yollar (`relay::service`), cevap
  `media::lineEnd` → `relay::line` ile gelir. Çözümleme `rc::relayQuakeLine` / `rc::relayHolidayLine`.
- Köprü yoksa ya da cevap vermezse: bellek yetiyorsa eskisi gibi doğrudan HTTPS; yetmiyorsa deprem için
  `http://www.koeri.boun.edu.tr/scripts/lst0.asp` (windows-1254, sabit sütunlu son 500 deprem, Türkiye saati).
  `rc::kandilliParse` satır satır okur, AFAD'la aynı kutu, eşik ve 2 günlük pencereyi uygular, pencere dışına çıkınca
  durur (sayfanın ~25 KB'ı okunur). Büyüklük Mw, yoksa ML, yoksa MD. Yer adı "KÖY-İLÇE (İL)" → "İlçe (İl)".
  `QuakeInfo.source` verinin AFAD mı Kandilli mi olduğunu tutar. Nager'ın yedeği gömülü Diyanet tablosudur.
- HTTPS'e giden işlerde yazıcı bağlantısını kapatma (`printer::holdForTls`) artık `net::get` içinde, yalnızca HTTPS
  gerçekten denenince yapılır. İş tablosundaki `tls` sütunu kalktı; her kaynağın son yolu `rapor`'da görünür
  (`http`, `https`, `kopru`, `kandilli`), `[veri]` satırı köprü isteklerini sayar.
- Ölçüm (kartta, açılış #170): köprüyle AFAD ve Nager 200 (köprü cevabı 275 ms); köprü kapalıyken AFAD işi Kandilli'den
  200, 129 ms, 25 024 B. Gerçek Kandilli sayfası bilgisayarda Python ile çapraz denetlendi (aynı son ve en büyük
  deprem).

---

## 12. WiZ akıllı ampul

`src/wiz/wiz.cpp` + `lib/rumy_core/src/wiz_proto.cpp`, `pairing.cpp`:

- WiZ ampuller yerel ağda **UDP 38899** portunda JSON komutlarıyla konuşur (`getPilot`, `setPilot`,
  `getSystemConfig`...). Rumy bulut hizmetine hiç bağlanmaz.
- **Keşif:** yayın (broadcast) ile ampuller bulunur, adaylar listelenir; eşleştirmede seçilen ampul bir kez yanıp
  söner. Eşleşme **MAC adresiyle** tutulur; IP değişse bile 15 sn'de bir yeniden keşifle bulunur.
- 5 sn'de bir durum okunur (açık/kapalı, renk, parlaklık); komutlar en az 80 ms arayla, son komut 1 sn sessizlik
  sonrası doğrulanır.
- Gözlerin rengi ampul açıkken ampulün rengine hafifçe karışır (`rc::pilotColor`, `lv_color_mix`).
- `tools/fake_bulb.py`: gerçek ampul olmadan test için sahte WiZ ampul (aynı UDP protokolü).
- Başlangıç noktası, WiZ ampulü CYD'den yöneten küçük bir deneme çizimiydi.

---

## 13. Ayarlar, ilk kurulum ve telefon uygulaması

### 13.1 Ayarlar ve NVS (`src/settings/settings.*`)

`Settings` yapısı NVS'te (`Preferences`) tek tek anahtarlarla saklanır:

| Anahtar | Alan | Anlam |
|---|---|---|
| `name`, `city` | ad, şehir | Selamda ve konumda görünür |
| `lat`, `lon`, `prov` | koordinat, il kodu | Hava ve bölge için |
| `bri`, `nbri` | parlaklık, gece parlaklığı | % |
| `nmode`, `nstart`, `nend` | gece modu, başlangıç, bitiş (dakika) | Varsayılan saatlere göre 02:00–07:00 |
| `home`, `card` | ana ekrana dönüş (sn), kart süresi (sn) | 60 / 5 |
| `maint` | 04:00 bakım yeniden başlatması | Açık/kapalı |
| `trot` | dokunmatik yönü | 0–7, ILI9341'de 6 |
| `wifi` | WiFi ayarlandı mı | İlk kurulum |
| `qmag` | deprem eşiği ×10 | 20 = M2,0 |
| `esleep` | göz uykusu (dk) | 0 = kapalı (3 Ekim'den beri varsayılan) |
| `bmac`, `bname` | ampul MAC ve adı | Eşleştirme |
| `team` | takım (ESPN no) | 0 = takım yok (varsayılan); ör. Fenerbahçe 436 |
| `fut` | maç takibi açık mı | Varsayılan açık; kapalıyken hiçbir maç sorgulanmaz |
| `tc1`, `tc2` | takımın iki rengi (0xRRGGBB) | 0 = varsayılan lacivert-sarı palet |
| `wagent`, `wgoal`, `wonly` | ampul: ajan uyarısı, gol kutlaması, yalnızca ampul açıkken | Varsayılan üçü de açık |
| `gsl`, `gucl` | Süper Lig / Şampiyonlar Ligi gol bildirimleri | Varsayılan açık |
| `uyku` | "Şimdi uyut" bitişi (UTC sn) | 0 = yok; yeniden başlatmada korunur |
| `boots` | açılış sayacı | Tanılama |

Eşli telefonların belirteçleri ayrı `telefon` ad alanında (`t0`–`t3`, `next`) tutulur.

Okunan değerler `sane()` ile sınırlara çekilir (bozuk NVS'e karşı). Değişiklikler `store::SETTINGS` sürümünü
artırır ve UI görevine `M_SETTINGS` olarak ulaşır.

### 13.2 İlk kurulum (`src/app/portal.cpp`)

WiFi ayarlı değilse LVGL başlatılmaz; LovyanGFX ile sade bir ekran (VLW biçimli gömülü font, QR kod) ve WiFiManager
ile **Rumy-Kurulum** erişim noktası açılır. Sayfada WiFi seçimi, **ad** ve **il** alanları vardır (il adı ya da
plaka kodu kabul edilir). Kaydedince Rumy yeniden başlar. Bağlantı başarısızsa neden (yanlış şifre, ağ yok...)
yakalanıp kullanıcıya gösterilir.

### 13.3 Telefon uygulaması (`src/webui/webui.cpp`, `src/ui/scr_phone.cpp`, `web/index.html`)

Rumy 80 portunda küçük bir HTTP sunucusu çalıştırır (WiFi bağlıyken her zaman açık, `rumy.local` mDNS adıyla).
Sunduğu tek sayfa iPhone'da **Ana Ekrana Ekle** ile tam ekran bir uygulamaya dönüşür (`apple-mobile-web-app-capable`,
`/icon.png` 180 px simge). Kullanım ve ekranlar README bölüm 7'de; tasarım ve ölçümler bölüm 30'da.

- **Eşleştirme:** Rumy'de Ayarlar → Telefon açıkken 4 haneli bir kod üretilir (`webui::setPairing`). Telefon
  `POST /api/pair {"code"}` ile bu kodu yollar; doğruysa 128 bit rastgele bir belirteç (32 onaltılık) üretilip
  NVS'e yazılır ve telefona döner (en fazla 4 telefon, beşinci en eskinin yerine). Telefon belirteci kendi
  saklama alanında tutar ve her istekte `X-Token` başlığıyla yollar. Ekran kapanınca kod geçersizleşir; her
  başarılı eşleşmeden sonra kod yenilenir. Hatalı kod ya da belirteçte 5 denemede 1 dk kilit.
- **API:** `GET /`, `GET /icon.png`, `GET /api/provinces`, `POST /api/pair` (belirteçsiz); `GET /api/state`,
  `POST /api/settings` (yalnızca gönderilen alanlar değişir), `POST /api/events`, `POST /api/sleep {"on"}`,
  `POST /api/refresh`, `POST /api/reboot`, `POST /api/ping`. Gövde en fazla 3 KB.
- **Şimdi uyut:** `settings::setSleepUntil()` gece bitiş saatinin bir sonraki geldiği ana kadar uyku yazar
  (`rc::sleepUntilUtc`, `lib/rumy_core/src/phone_app.h`); UI görevinin `computeNight()` bu süre içinde geceyi
  zorlar. Dokunma ve yazıcı hatası gece modundaki gibi kısa süre uyandırır; `{"on":false}` bitirir.
- Sayfa tek HTML dosyasıdır; `tools/build_web.py` derleme öncesinde onu gzip'leyip `web_assets.h`'ye, simgeyi
  `web_icon.h`'ye gömer.

---

## 14. SD kart

`src/board/sdcard.cpp`: VSPI üzerinde 20 MHz (başarısızsa 4 MHz). FATFS her açık dosya için 4 KB tampon ayırdığı
için `max_files = 1` (SD erişimi tek kilitle sıralanır; bellek darboğazı çözümünün parçası). Kullanım:
`/rumy/cache` (son veri), `/rumy/holidays` (bayram tabloları), `/rumy/events.json` (kişisel günler).
`sdtest` komutu 50 kez yazıp okuyarak kartı sınar. Kart yoksa her şey çalışır; yalnızca kişisel günler kaydedilmez
ve bilgi kartında bir uyarı görünür.

---

## 15. Spotify ekranı ve bilgisayar köprüsü

### 15.1 Araştırma ve seçilen yol

Açık kaynak projeler incelendi:
- [Spotify-Diy-Thing](https://github.com/witnessmenow/Spotify-Diy-Thing) (MIT), [llamafi](https://github.com/bvarbanov90/llamafi),
  [CarDisplay-OS](https://github.com/ServetAKC/CarDisplay-OS): kart **Spotify Web API'ye doğrudan** bağlanır
  (OAuth, geliştirici hesabı, HTTPS, JPEG çözme).
- [NowPLayingDisplay](https://github.com/RodgerE1/NowPLayingDisplay): bilgisayar Windows'un medya oturumundan
  çalan şarkıyı okuyup ESP32'ye gönderir.

Rumy'de HTTPS için gereken ~60 KB bellek zaten hava/deprem için sınırda olduğundan ve kart bilgisayara bağlı
kullanıldığından **ikinci yol** seçildi: bilgisayardaki bir Python köprüsü şarkı bilgisini ve kapağı **USB seri
hat** üzerinden gönderir. Spotify hesabı ya da API anahtarı gerekmez; Windows'a bilgi veren her oynatıcı
(`--any` ile tarayıcı, VLC...) çalışır.

### 15.2 Bilgisayar köprüsü (`tools/spotify_bridge.py`)

- **Şarkı bilgisi:** `winrt` ile `GlobalSystemMediaTransportControlsSessionManager` açılır; Spotify oturumu
  (yoksa `--any` ile geçerli oturum) seçilir. Başlık, sanatçı, albüm, çalma durumu, konum ve süre okunur. Konum,
  Spotify'ın son güncelleme zamanından bu yana geçen süre eklenerek düzeltilir.
- **Kapak:** oturumun küçük resmi (Spotify'da 300×300 PNG) okunur → ortadan kare kırpılır → 144×144'e LANCZOS ile
  küçültülür → köşeleri 12 px yuvarlatılıp siyaha maskelenir → **RGB565 little-endian** ham veriye (41 472 B)
  çevrilir. Kimliği ham resmin CRC32'sidir.
- **Vurgu rengi:** kapak 24×24'e küçültülür, renkler kabaca gruplanır, doygunluk × parlaklık × sıklığa göre en iyi
  renk seçilir ve neon görünsün diye doygunluğu ≥ 0,65, parlaklığı ≥ 0,9'a çekilir. Spotify ekranındaki ilerleme
  çubuğu, oynat düğmesi, sanatçı adı ve kulaklıklı Rumy'nin notaları bu renge boyanır.
- **Yazı temizliği:** karttaki fontta olmayan karakterler Unicode NFKD ile aksanlarından arındırılır ya da atılır;
  sekme/satır sonları boşluğa çevrilir; alanlar bayt sınırına göre "…" ile kısaltılır.
- **Kontroller:** Rumy'den gelen `@cmd play/next/prev` komutları SMTC'nin `try_toggle_play_pause_async`,
  `try_skip_next_async`, `try_skip_previous_async` çağrılarına; `volup/voldown` Windows'un ses tuşlarına
  (`keybd_event`, basış başına 2 adım) çevrilir.
- **Bağlantı:** CH340 portu kendisi bulunur (ya da `--port`); port açılırken DTR/RTS kapalı tutulur ki kart
  sıfırlanmasın. Kopunca 2 sn'de bir yeniden bağlanır. `--log` Rumy'nin günlüğünü de yazar, `--send "komut"` bağlanınca
  karta seri komut yollar, `--autostart ac/kapat` Windows başlangıç klasörüne konsolsuz (pythonw) başlatıcı ekler/kaldırır.

### 15.3 Seri protokol

Rumy'nin günlük satırları da aynı hattan geçer; protokol satırları `@` ile başlar ve günlükten ayrılır.

| Yön | Mesaj | Anlamı |
|---|---|---|
| PC → Rumy | `@np⇥çalıyor⇥konum_ms⇥süre_ms⇥kapak_hash⇥renk⇥uygulama⇥ad⇥sanatçı⇥albüm` | Saniyede bir (ya da değişince) çalma durumu; sekme ayrımlı |
| Rumy → PC | `@need <hash>` | Bu kapak önbellekte yok (5 sn'de bir tekrarlanabilir) |
| PC → Rumy | `@art <hash> <gen> <yük> <bayt> <crc32>` | Kapak aktarımı başlıyor |
| Rumy → PC | `@go <hash>` | İlk sektör silindi, veri gönderilebilir (kimlik 3 Ekim'den beri; köprü kimliği tutmayan `@go`'yu atar) |
| PC → Rumy | 2048 baytlık ham parça | Kapak verisi |
| Rumy → PC | `@ok <alınan>` | Parça yazıldı, sonrakini gönder (köprü yalnızca beklediği `<alınan>`'ı kabul eder) |
| Rumy → PC | `@art ok <hash>` / `@art err <neden>` | Aktarım bitti / hata (crc, zaman aşımı, yer...) |
| Rumy → PC | `@cmd play\|next\|prev\|volup\|voldown` | Dokunmatik düğmeler |
| Rumy → PC | `@cmd seek <ms>` | İlerleme çubuğu sürüklendi; köprü SMTC `try_change_playback_position_async` ile atlar, 3 sn hedef konumu esas alır |
| PC → Rumy | `@tempo <vuruş_ms> <sonraki_vuruşa_ms>` | Ritim bilgisi (bölüm 16); `0 0` = bilinmiyor |
| PC → Rumy | `@vol <0-100>` / `@vol -` | Spotify'ın ses seviyesi (değişince ve saniyede bir); `-` = Spotify'ın ses oturumu yok |
| Rumy ↔ PC | `@cmd pls`, `@pls`, `@pl`, `@plnow`, `@cmd plplay`, `@plst`, `@cmd plmix` | Çalma listeleri (bölüm 27.3) |
| PC → Rumy | `@ajanlar <sürüm> <satır> <çalışan> <bekleyen> <boşta>` + satır başına `@ajan⇥sıra⇥kimlik⇥c\|b\|i⇥durum_sn⇥iş_sn⇥proje⇥ayrıntı` | Claude Code oturumları (bölüm 28) |
| Rumy → PC | `@foto <hash> <px> <oyuncu adı>` | Gol ekranı için oyuncu fotoğrafı; köprü `@art` ile px×px kare yollar ya da `@foto yok <hash>` der |
| PC → Rumy / Rumy → PC | `@ping` / `@pong` | Test |

Rumy 6 sn boyunca `@np` almazsa köprünün koptuğunu kabul eder. Kapak ya da kamera aktarımı hatayla biterse Rumy, hat
300 ms susana kadar (en çok 2 sn) gelen baytları yok sayar (`rapor` → `[medya] … atilan N B`); böylece yarım kalan
verinin parçaları komut satırı gibi okunmaz.

### 15.4 Kartta alma ve kapak önbelleği (`src/media/media.cpp`)

- Seri okuma `diag::readSerial()` içindedir: satır başında `@` gelirse satır medya modülüne yönlendirilir (320
  bayta kadar); kapak aktarımı sırasındaysa gelen baytlar doğrudan medya modülüne gider.
- **Kapaklar RAM'de değil, flash'ta tutulur:** `art` bölümü 21 yuvaya (her biri 0xB000 = 44 KB) bölünür. Her yuvanın
  başında 32 baytlık başlık vardır (sihirli sayı `RART`, kimlik, sıra numarası, en, boy, CRC32, boyut), ardından
  RGB565 piksel verisi.
- Açılışta tüm başlıklar okunur; geçerli olanlar bir tabloya alınır. Bölümün tamamı `esp_partition_mmap` ile
  **belleğe eşlenir**; LVGL kapak resmini doğrudan flash adresinden çizer (tıpkı gömülü ikonlar gibi), RAM
  harcanmaz.
- Yeni kapak için **en eski yuva** (en küçük sıra numarası, ekranda gösterilen hariç) seçilir; bu, flash yıpranmasını
  yuvalara yayar.
- Veri önce yazılır, **başlık en son** yazılır: aktarım yarıda kalırsa yuva geçersiz sayılır. CRC32 karttaki
  `esp_rom_crc32_le` ile hesaplanır ve Python'daki `zlib.crc32` ile birebir aynıdır (doğrulandı).
- **Önemli ders:** ilk denemede kapaklar hiç gelmedi. Sebep: flash silme/yazma sırasında işlemci önbelleği kapanır
  ve IRAM'de olmayan UART kesmesi çalışamaz; 460 800 baud'da donanım FIFO'su (128 bayt) ~3 ms'de dolar, veri kaybolur.
  Çözüm, **akış denetimi**: kart 2 KB'lık parçanın tamamı seri tamponda birikmeden okumaya başlamaz; parçayı flash'a
  yazar, bir sonraki parçanın sektörünü siler ve **ancak sonra** `@ok` gönderir. Bilgisayar `@ok` gelmeden yeni veri
  göndermediği için flash işlemleri hat boşken yapılır.
- Seri hız 115 200'den **460 800 baud**'a çıkarıldı, alma tamponu 2 304 bayta büyütüldü. Kapak aktarımı ölçümü:
  kartta parça başına ~77 ms, toplam **1,2–1,8 sn**; aynı albüm tekrar gelince anında. 22+ kapak hatasız aktarıldı.
- İkinci ders: köprünün okuma döngüsü ilk sürümde `read(256)` + 0,2 sn zaman aşımıyla çalıştığı için her `@ok`
  0,2 sn gecikiyordu (5,8 sn); `read(in_waiting or 1)` ile 1,2 sn'ye indi.

### 15.5 Spotify ekranı (`src/ui/scr_media.cpp`)

| Bölge | İçerik |
|---|---|
| 10,10 – 144×144 | Kapak (yoksa koyu kutu + vurgu renginde nota simgesi) |
| Sağ sütun, üst | Spotify logosu (yeşil) + "Spotify" (duraklatılmışsa "· duraklatıldı") |
| Sağ sütun | Şarkı adı 22 px, en fazla 2 satır, sığmazsa "…"; sanatçı 16 px vurgu renginde; albüm 12 px, 2 satır |
| 10,164 | İlerleme çubuğu (vurgu rengi), altında geçen süre ve toplam süre; konum saniyede bir kartta tahmin edilir |
| Alt sıra | 5 yuvarlak düğme: ses −, önceki, **oynat/duraklat** (vurgu renginde dolu, 46 px), sonraki, ses + |
| Köprü yoksa | "Bilgisayar bağlı değil / spotify_bridge.py" |
| Müzik yoksa | "Müzik çalmıyor / Spotify'da bir şarkı aç" |

Oynat/duraklata basınca ekran **hemen** güncellenir (iyimser güncelleme); gerçek durum bir sonraki `@np` ile gelir.

**Ses düğmeleri sorunu (bu oturumda düzeltildi):** oynat/sonraki çalışırken ekranın kenarındaki ses düğmeleri
çalışmıyordu. `dokunma ac` ile toplanan koordinatlar dokunuşların düğmelerin çevresine düştüğünü gösterdi; düğmelerin
dokunma alanı 4 px'ten 10 px'e genişletildi ve sorun çözüldü (ses + için 275–297, ses − için 31–47 x
koordinatlarındaki dokunuşlar komuta dönüştü).

---

## 16. Kulaklıklı Rumy ve ritim takibi

### 16.1 Kartta çizim (`rumy.cpp`: `phones`, `drawPhones`, `band`, `cup`)

Müzik çalarken (köprü bağlı + oturum aktif + çalıyor) ana ekrandaki Rumy **MUSIC** ifadesine geçer ve kulaklık
takar. Tüm ölçüler göz boyutuna (`e`) oranlıdır; ana ekranda e = 52 px:

- **Kapaklar:** gözlerin dış kenarından 3 px ötede, genişlik 0,22e (11 px), yükseklik 0,58e (30 px), kapsül biçimi.
  Göze bakan tarafta koyu **yastık** şeridi, dış tarafta ince beyaz **parlama**, çevresinde iki katmanlı parıltı.
- **Bant:** iki çeyrek daire yayı (`lv_draw_arc`) + düz bir çizgi; gözlerin parıltısının üstünden 5+ px boşlukla
  geçer. Önce kalın ve saydam (parıltı, uçları düz), sonra ince ve opak (uçları yuvarlak) çizilir. Uçları düz
  parıltı, yay ile çizginin birleştiği yerlerde saydamlığın üst üste binip parlak nokta yapmasını önler (4× büyütmede
  fark edilip düzeltildi).
- **Renk:** neon pembe `0xFF4FA3` (camgöbeği gözlerle zıt, neon tema ile uyumlu). Gece ekranında kısık tonu.
- **Notalar (♪):** sağ kapağın dışından yukarı doğru yükselir; baş (elips), sap ve bayrak çizgiyle çizilir, boyut
  5→8 px büyür, ±2 px salınır, belirip söner; 2,4 sn'lik döngüde 2 nota. Renk albüm kapağının vurgu rengidir.
  Yeri bilerek seçildi: ekranın sağ üstünde ampul düğmesi, sol üstünde şehir yazısı var; tek boş sütun sağ kapak
  ile ampul düğmesi arası.
- **Baş aşağı iner:** MUSIC ifadesinde bütün baş (gözler, kulaklık, notalar) ~6 px aşağı
  iner (`Params::head`); kulaklığın bandı ekranın üst kenarına yapışık durmaz. Geçiş diğer parametreler gibi
  yumuşaktır (~0,3 sn). Gece ekranında (uyuyan Rumy) değişiklik yok.
- Müzik çalarken **göz uykusu çalışmaz** (Rumy dinliyor); gece modunda kulaklığı takılı uyur, "z"ler kapağın
  yanından çıkar. Aksesuarlar gizlenir.

### 16.2 Ritim hareketi (`groove`)

Bir vuruş süresi P ve bir vuruş anı (çapa) bilinir. Her karede vuruş içindeki evre p (0–1) ve vuruş sayısı k
hesaplanır:
- **Sağa sola sallanma:** `swayX = A_s · sin(π · (k + p))` → iki vuruşta bir tam salınım (A_s ≈ 0,06e = 3 px).
- **Kafa sallama:** vuruştan biraz önce başlayan hızlı iniş (smoothstep, periyodun %20'si) ve yavaş dönüş
  (karesel sönüm): `bobY = A_b · g(p)` (A_b ≈ 0,08e = 4 px). Tepe noktası vuruştan ~%10 periyot sonra.
- Gözler **ve** kulaklık birlikte hareket eder (ortak "kafa" konumu).
- Tempo bilgisi yoksa 1,1 sn'lik yavaş, genel bir sallanma kullanılır.

**Kartta faz kilitleme (`setTempo`):** köprüden saniyede bir tempo gelir. Yeni tempo eskisine %4'ten yakınsa ve
vuruş anı farkı 90 ms'den azsa, çapa farkın üçte biri kadar kaydırılır ve periyot 3:1 ağırlıkla ortalanır (yumuşak
PLL); değilse doğrudan yeni değere geçilir. Tüm hesaplar tamsayıyla ve `lv_trigo_sin` ile yapılır.

### 16.3 Ritim takibi (köprü: `BeatTracker`, `Loopback`, `start_beat_tracker`)

1. **Ses kaynağı:** `pyaudiowpatch` ile Windows'taki **bütün** ses çıkışlarının WASAPI loopback akışı açılır. Her
   saniye son 0,5 sn'de veri gelen ve seviyesi eşiğin üstünde olan cihazlar arasından en yüksek seviyelisi seçilir;
   çalan cihaz 2 sn sessiz kalmadıkça kaynak değişmez (bir sistem sesi kaynağı kaçırmasın diye). Bu, sesin
   varsayılan çıkış (monitör) yerine **Bluetooth kulaklıktan** dinlendiği bir kurulumda eklendi; ilk
   sürüm yalnızca varsayılan cihazı dinliyor ve hiç veri alamıyordu (Windows sessiz cihazın loopback'ine veri
   göndermez).
2. **Başlangıç kuvveti (onset):** 48 kHz'de 1024 örneklik adımlarla (21,3 ms), 2048 örneklik Hann pencereli FFT;
   log sıkıştırılmış genliklerin pozitif farkı (spectral flux). Yalnızca **3 kHz altı** kullanılır ve 200 Hz altı
   (davul/bas) 4 kat ağırlıklandırılır. İlk sürüm tüm bantları kullanınca vuruşun yarısında çalan hi-hat'lere
   kilitleniyordu (faz 240 ms kayık); bant sınırlamasıyla düzeldi.
3. **Tempo:** son 8 sn'lik kuvvet zarfı yerel ortalamadan arındırılır, normalize edilir, **özilinti**
   (autocorrelation) hesaplanır. 60–200 BPM gecikmeleri arasında, 120 BPM merkezli log-Gauss ön bilgisiyle
   ağırlıklı tepe seçilir, parabolik ara değerlemeyle alt-örnek hassasiyeti bulunur. Özilinti ölçümden ölçüme
   üstel ortalamayla (0,75/0,25) biriktirilir; tek ölçüm tempoyu oynatamaz.
4. **Oktav düzeltmesi:** bulunan tempo 80 BPM altındaysa ve iki katı da güçlüyse (≥ %50) iki katı; 170 üstündeyse
   ve yarısı güçlüyse yarısı seçilir (gerçek şarkıda 65 ↔ 129 BPM karışması böyle giderildi).
5. **Faz:** son 4 sn üzerinde tarak süzgeci (comb filter): her olası kaymada vuruş noktalarındaki kuvvetler toplanır,
   en büyüğü son vuruş anını verir. Son 4 sn kullanılması küçük tempo hatalarının birikmesini önler.
6. **Kararlılık:** güven (özilinti tepesi) 0,12'nin altındaysa ölçüm sayılmaz; 8 ardışık başarısız ölçüme kadar
   son tempo korunur; mevcut tempodan %6'dan fazla farklı bir değer ancak 3 ölçüm üst üste gelirse kabul edilir.
7. **Gecikme payı:** ses kulağa hoparlörden ~50 ms, **Bluetooth kulaklıktan ~150–200 ms** geç ulaşır. Köprü
   açılışta PowerShell ile (`Get-PnpDevice -Class Bluetooth`) Bluetooth cihaz adlarını alır; dinlenen çıkışın adı
   bir Bluetooth cihazını içeriyorsa gecikme payı 200 ms, değilse 50 ms olur (`--beat-offset` ile elle ayarlanır).
   PowerShell çıktısı UTF-8'e zorlanıp hataya dayanıklı çözülür (Türkçe karakterli bir cihaz adı ilk sürümü
   çökertmişti).
8. **Gönderim:** köprü her kalp atışında (saniyede bir) `@tempo <periyot_ms> <sonraki_vuruşa_ms>` gönderir;
   müzik durunca `@tempo 0 0`.

**Doğrulama:**
- Bilinen tempolarda üretilmiş sentetik şarkılar (davul + ara hi-hat + gürültü + ton): 124/96/140 BPM →
  123,0/96,7/140,4 BPM tahmin, vuruş zamanı hatası +9/−16/−17 ms.
- Gerçek müzikte (Spotify, Bluetooth kulaklık): önceki sürümde 86/129/65 arasında gidip gelen tahmin, son sürümde
  34 sn boyunca sabit 129 BPM; başka bir şarkıda 105 BPM.

**Önemli ders (COM):** ilk sürümde köprünün ana döngüsü tamamen takıldı: `PyAudio()` ana iş parçacığında Windows
COM'u tek iş parçacıklı (STA) başlatıyor, bu da `winrt`'nin eşzamansız çağrılarının geri dönüşünü engelliyordu. Ses
kütüphanesinin başlatılması tamamen kendi iş parçacığına taşınarak çözüldü.

---

## 17. Tanılama, seri komutlar, ölçüm araçları

Seri hat 460 800 baud. Komutlar (`src/diag/diag.cpp`):

| Komut | İş |
|---|---|
| `rapor` / `mem` | Sürüm, çalışma süresi, açılış nedeni, bellek (boş, en düşük, en büyük blok, eğilim), LVGL havuzu (tepe ve bellek yetmediği için açılmayan ekran sayısı dahil), görev yığınları, WiFi/RSSI/saat, TLS tepesi, ertelenen HTTPS sayısı, JSON arenasının tepesi (`json tepe`), her veri kaynağının (`hava+7gun`, `deniz`, `havakalitesi`, `afad`, `nager`) son kodu/süresi/baytı/hataları ve bir sonraki çekmeye kalan süre, SD, **medya** (köprü, kapak yuvaları, aktarım/hata sayıları), **kamera** (kare, hata, kare süresi), ampul, **yazıcı** (bağlantı, ad, durum, ilerleme, kalan süre, hata kodu, mesaj/bağlantı/ayrıştırma sayaçları) |
| `yenile` | Tüm veriyi hemen çek |
| `web ac` / `web kapat` | Telefon uygulamasının sunucusu (açılışta açık; `kapat` yeniden başlatana kadar) |
| `telefon unut` | Eşli bütün telefonları siler |
| `sdtest` | SD'ye 50 kez yaz/oku |
| `dokunmatik 0-7` | Dokunmatik yönünü değiştir ve kaydet |
| `dokunma ac` / `dokunma kapat` | Her dokunuşun koordinatını yaz |
| `bench`, `bench gecis`, `bench kaydir` | Kart kendi kendine geçiş ve kaydırma yapar, her adımın karelerini, en uzun karesini ve fps'ini ölçer |
| `bench bellek` | Bütün ekranları ve alt sayfaları gezer; her adımda LVGL havuzunun tepesini sıfırlayıp kullanımı, tepeyi, en büyük boş bloğu ve parçalanmayı yazar; sonda en yüksek tepeyi özetler. Tur sırasında otomatik ekran geçişleri (gece, ana ekrana dönüş, Spotify) durur |
| `lvgl yuk <KB>` | LVGL havuzundan 1 KB'lık parçalarla bellek tutar (`lvgl yuk 0` bırakır); "bellek yetersiz" geri dönüşünü sınamak için |
| `perf ac` / `perf kapat` | Gezinirken saniyede bir çizim ölçümü |
| `profil ac` / `profil` | (cyd_prof) LVGL iç işlevlerinin süre tablosu |
| `spitest <MHz>` | Ekrana desen yazıp geri okuyarak SPI hızını sınar |
| `kayma ters` / `kayma duz` | Donanım kaydırma yönü |
| `gece 02:00 07:00` | Gece modunu saatlere göre ayarla ve kaydet |
| `yazici <ip>` / `yazici kapat` | Creality K2 yazıcının IP adresini NVS'e kaydet (bölüm 25) ya da özelliği kapat |
| `ekran ana` / `ekran spotify` / `ekran yazici` | O sayfaya geç (karta dokunmadan deneme için) |
| `ekran goruntusu` | Paneli satır satır geri okuyup `@shot` / `@px <y> <x> <base64>` / `@shotend` satırlarıyla gönderir; köprü bunu `tools/.cache/rumy_ekran.png` olarak kaydeder |
| `wiz durum`, `wiz tara`, `wiz esle <mac>`, `wiz unut` | Ampulün eşleşme/çevrimiçi/IP/ışık durumu ve komut izni; ağda tarama ve adayların listesi; eşleştirme; eşleşmeyi silme |
| `wiz ac`, `wiz kapat`, `wiz parlak <10-100>`, `wiz sicak <K>`, `wiz renk <0-359>`, `wiz sahne <no>` | Ampulü dokunmadan kumanda etme (Pi'de uzaktan deneme için) |
| `reboot`, `wifi-sifirla` | Yeniden başlat, WiFi'yi sıfırla |

Köprü çalışırken seri port meşgul olduğundan, köprü `tools/.cache/rumy_komut.txt` dosyasına yazılan satırları
(ör. `rapor`, `ekran goruntusu`) karta iletir ve dosyayı siler; `--log` ile kartın cevabı köprünün çıktısına düşer.

Ayrıca her 5 dk'da bir tek satırlık özet (`diag up=... heap=... lvgl=... fps=... tls=...`) ve açılışta adım adım
boş heap satırı yazılır. Tanılama ekranı aynı bilgileri cihazda gösterir.

---

## 18. Performans çalışmaları

v1.0.2 kartta ölçüldüğünde sayfa geçişleri ve hava ekranında kaydırma takılıyordu. `bench` ölçümleriyle:

| Adım | v1.0.2 | v1.1.0 |
|---|---|---|
| Sayfa geçişi (220 ms) | 3 kare, 5–7 fps, kare 100–190 ms | 18–30 kare, en uzun kare 20–35 ms |
| Hava ekranını açma | 293–304 ms | 9–10 ms |
| Hava ekranında kaydırma | 5–10 fps | 24–25 fps |
| Saatlik şerit | 15 fps | 27–30 fps |
| Ayarlarda kaydırma | 13 fps | 30–32 fps (SPI sınırı) |

| Değişiklik | Etki |
|---|---|
| Donanım kaydırmalı geçiş (bölüm 7.4) | Kareler 5–8 kat ucuzladı |
| Flash QIO 80 MHz (önce DIO 40) | Font/ikon okuma hızlandı, kaydırma ~2× |
| İkonlar flash'a gömüldü | 300 ms → 10 ms açılış, 20 KB RAM boşaldı |
| `-O2` (önce `-Os`), `build_unflags = -Os` | Çizim %20–25 hızlandı (+137 KB flash) |
| Çizim bufferları 16 → 32 satır | Kare başına parça sayısı yarıya indi |
| LVGL piksel işlevleri IRAM'de, köşe maskesi önbelleği | Birkaç ms |
| Dokunmatik SPI 250 kHz → 1 MHz | Kaydırmada okuma gecikmesi kısaldı |
| SPI 80 MHz denendi | `spitest 80`: piksellerin %3–5'i bozuk → 40 MHz'de kalındı |

Bu oturumdaki animasyonlar da performansa göre tasarlandı: uyku "z"leri 20 fps ve yalnızca küçük alan; kulaklık
modunda yüz 30 fps ama yalnızca yüzün ve nota sütununun alanı yeniden çizilir.

**"Şimdi çalıyor" şeridinin ölçülerek hafifletilmesi** (`perf ac`, müzik çalarken ana ekran, gündüz; gece modu
ölçüm için geçici olarak kaydırıldı ve geri alındı). Şeridin payını ayırmak için şerit animasyonu kapalı geçici bir
sürüm de ölçüldü:

| Durum | Kare başına çizim (ort.) |
|---|---|
| Yalnız yüz (kulaklık, kafa sallama, notalar) | 10,8 ms |
| + plak, 10 LVGL şekliyle çizilen ilk sürüm | 14,3 ms |
| + plak ve kayan uzun ad, ilk sürüm | 16,4 ms |
| + plak, önceden hazırlanmış 24 kareden tek resim | 11,9–12,7 ms |
| + plak ve kayan uzun ad (resimli plak, görünen parça çizimi, daraltılmış tarih nesnesi) | 14,5 ms |

Kare hızı her durumda 30 fps'de kaldı. Plağın resimle çizilmesi en büyük kazancı sağladı (~2,5 ms/kare). Kayan
yazıda yalnızca görünen harflerin çizilmesinin normal çalışmadaki etkisi ölçüm gürültüsü içinde kaldı (±1 ms);
müzik çalarken ana ekrana kayışlarda ise en uzun kare 23–32 ms'den 22 ms'ye indi (kayış, ekranı dikey sütun
şeritleri hâlinde çizdiği için şerit her geçişte birkaç kez çizilir). Müzik çalarken ana ekrana kayış 20 kare,
müziksiz 22–23 kare sürer (kulaklıklı yüz ve şerit kareyi biraz ağırlaştırıyor; 200 ms'lik süre değişmiyor).

---

## 19. Test ve doğrulama

### 19.1 Birim testleri (`test/`, `pio test -e native`)

Arduino'ya bağımlı olmayan mantık (`lib/rumy_core`) bilgisayarda Unity ile test edilir: **19 takım, 168 test** (27 Ekim),
hepsi geçiyor. Aşağıdaki tabloya sonradan eklenenler: `test_espn` (fikstür, maç olayları, lig günü, puan durumu),
`test_livescore` (hızlı gol), `test_agents` (Claude oturumları, bölüm 28.5), `test_fwimage` (kablosuz güncelleme
teklifi, imaj başlığı, SHA dosyası, imaj gövdesi/özeti ayrımı; 9 test, bölüm 29). `test_phone` (telefon uygulaması: uyku bitişi, eşleştirme belirteci ve kodu; 7 test, bölüm 30), `test_kandilli`
(Kandilli satırı ve sayfası, yer adı, köprü satırları; 9 test, bölüm 10.6).

| Takım | Konu |
|---|---|
| test_openmeteo | Birleşik hava isteğinin adresi (uç koordinatlarda da tampona sığması), gerçek birleşik cevabın ayrıştırılması, birleşik ve parça ayrıştırıcıların aynı yapıyı üretmesi, eksik/yarım cevap, küçük arena; anlık/saatlik/7 gün/hava kalitesi/deniz parça ayrıştırma (gerçek cevap örnekleri; 10 test). Test çıktısı arena tepesini yazar |
| test_afad | Deprem listesi, bölge ve eşik süzgeci |
| test_nager | Tatil ayrıştırma, geçici tarihlerde Diyanet tablosu tercihi |
| test_events | Önemli gün hesabı, yaş/yıl dönümü |
| test_cache | Önbellek biçimi, sürüm ve sağlama |
| test_color | Renk dönüşümleri, ampul rengi |
| test_pairing | Ampul eşleştirme mantığı |
| test_text | Türkçe metinler (gün/ay adları, süre, selam, AQI seviyeleri) |
| test_wind | Rüzgâr adı (poyraz, lodos...) |
| test_wiz | WiZ protokol mesajları |
| test_wxmood | Hava türü sınıflandırması: yağış kodları, yağışın rüzgar/sıcaklıktan önceliği, rüzgar ve hamle eşikleri, hissedilen yoksa sıcaklık, açık gündüz/gece, sıcak gündüz (hissedilen ya da UV), kapalı ve bilinmeyen kod, adlar (7 test) |
| test_media | Köprü protokolü (`media_proto`): `np` satırı (tam, kısa, boş alanlı, Türkçe harfli), şarkı özeti, `art` kapak teklifi ve yuva başlığı doğrulaması (144 ve 192 px), konum tahmini ve sarma tespiti, `tempo` satırı, vuruş kilidi (ilk kilit, küçük hatada yumuşatma, büyük değişimde atlama, sayaç taşması; 12 test) |
| test_k2 | Creality K2 durum mesajı: yazıcıdan alınmış gerçek tam mesaj, kısmi güncelleme, baskı/hata/filament bitti/kalp atışı örnekleri, iş adı temizleme, istek mesajı, yazıcı adresi doğrulaması (yalnızca IPv4; 11 test) |

Örnek cevaplar `test/fixtures/` altında (`make_fixtures.py` ile C dizilerine çevrilir).

### 19.2 Kartsız önizleme (simülatör, `tools/ui_preview.py`)

Gerçek LVGL 9.5, gerçek ekran kodu (`src/ui`, `src/rumy`), gerçek gömülü ikonlar ve test verileri MinGW-w64 g++ ile
bilgisayarda derlenir; yalnızca WiFi, WiZ, SD, saat, ekran sürücüsü ve medya köprüsü `sim/sim_stubs.cpp` ile taklit
edilir. `sim/sim_main.cpp` bir senaryo oynatır (gündüz, yağmur, ışık açık, hava ekranı kaydırmaları, ayarlar, gece,
deprem, bayram, göz uykusu, Spotify, kulaklıklı Rumy, "şimdi çalıyor" şeridinin kısa ad / uzun ad / kayan /
duraklatılmış / gizlenmiş / kısa gün adı / uzun tarih hâlleri, yazıcı ekranı, tanılama ekranının kaynak satırları...)
ve **59 ekran görüntüsü** üretir (`docs/preview/`,
`docs/ui_preview.png`; `--buyuk` ile 480×320: `docs/preview_buyuk/`), her ekran için LVGL havuzu kullanımını yazar. Önizleme 64 bit çalıştığı için işaretçiler
iki kat yer kaplar; bu yüzden önizleme havuzu kartınkinin 1,5 katıdır (kartta 24 KB → önizlemede 36 KB,
`RUMY_SIM_POOL_KB`). Önizlemede zaman kartın ~2 katı hızlı ilerler
(taklit edilen `vTaskDelay` zamanı ileri sarar). Bu oturumdaki her görsel değişiklik (z'ler, Spotify ekranı,
kulaklık, hava bloğu) önce burada görülüp düzeltildi, sonra karta yüklendi.

**Uç durumlar (`--cesit`, 4 Ekim):** `sim/sim_variants.cpp` 100 ekranlık ayrı bir senaryo oynatır →
`docs/preview_cesit/`, `docs/ui_cesit.png` (`--buyuk` ile 480×320). Kapsam: ana ekranda 14 hava ikonu (gündüz/gece),
−25 °C ile 45 °C arası sıcaklıklar ve eksik en yüksek/en düşük, çubukların en uç değerleri (nem %0/%100, AQI 0/500),
uzun ad / ad yok / uzun şehir / şehir yok, saat eşitlenmemiş, hiç veri yok, 14 kart döngüsü (uzun deprem yeri, uzun
özel gün adı, 123 km/sa rüzgar, 99 saatlik baskı), M7,8 deprem; hava ekranında kış (−18 °C, yoğun kar) ve eksik
veriyle tüm kaydırmalar; ışık ekranında renkli/kısık/çevrimdışı ve uzun ampul adı; 8 adaylı ve taramadaki eşleştirme;
Spotify'da çok uzun / emojili / Japonca / süresiz / sanatçısız / süreyi aşmış şarkı ve ses %0–%100; çalma
listelerinde emojili, çok uzun, tek harfli, Japonca adlar ve 0–10 000 şarkı, 30 liste; maçta uzun takım ve oyuncu
adları, 22 olay, 90+12', devre arası; yazıcıda uzun dosya adı, uzun hata, bilinmeyen durum; tanılamada bütün
kaynakların hatalı olduğu hâl; maç + baskı + müzik + özel gün + deprem aynı anda; uzun bildirim.

**Animasyon filmleri (`--film`, 10 Ekim):** `sim/sim_film.cpp` gol ekranını, bildirimleri ve lig kartlarını oynatır.
Film sırasında LVGL'nin her yenilemesi bir kare olarak kaydedilir (`film_<ad>/NNNN_<ms>.ppm`); `ui_preview.py`
bunları GIF'e ve 12 karelik inceleme şeridine çevirir. Her film için kare sayısı, kare başına yeniden çizilen ekran
payı (ortalama / en çok) ve tam ekran kare sayısı yazdırılır. Film süresi taklit edilen saate göre ölçülür
(`ui::loop()`'un kendi `vTaskDelay`'i zamanı ilerletir), yani kartla aynı zamanlama.

**Otomatik ekran denetimi (`sim/sim_audit.cpp`, her modda):** önizleme LVGL'ye yalnızca dinleyen bir çizim birimi
ekler (havuzdan bellek almaz; ölçümler ve görüntüler değişmez). Her ekran görüntüsü tek parça tamponla çizilirken
bütün yazı ve resim çizimleri toplanır: yazının gerçek mürekkep kutusu (harf harf), resmin saydam olmayan
pikselleri, kırpma alanı ve çizen nesne. Üç tür bulgu yazılır: **üst üste** (yazı-yazı ya da yazı-resim; arada
opak bir dolgu ile örtülenler sayılmaz), **kesik** (yazı kırpma alanından taşıyor; kaydırılabilir listelerin
görünür alanı ve kayan yazılar sayılmaz), **eksik harf** (fontta olmayan karakter). Bulgular
`<çıktı>/denetim.txt`'ye yazılır; bilerek böyle olanlar (kayan şarkı adı, kapak üstündeki liste düğmesi...)
`sim/denetim_izin.txt` ile susturulur. İlk koşuda bulunanlar CHANGELOG.md'da.

### 19.3 Kart üzerinde doğrulama

Her yüklemeden sonra seri porttan açılış kaydı ve `rapor` okundu (boş heap, LVGL, yığınlar, medya). Performans
`bench` ile ölçüldü. Kapak aktarımı test betikleriyle (bilinen verili sahte kapak, CRC doğrulaması, parça başına
süre) sınandı. Ses düğmelerinin Windows sesini gerçekten değiştirdiği `pycaw` ile ölçüldü (84 → 88 → geri 84).

### 19.4 Dayanıklılık testi

`cyd_endurance` ortamı veriyi 30 sn'de bir çeker, ekranları 5 sn'de bir değiştirir; `tools/endurance_log.py` seri
kaydı toplar ve en düşük heap, heap eğilimi (B/saat, sızıntı eşiği −300), parçalanma, TLS tepesi, yığın en düşükleri,
hatalar ve beklenmeyen yeniden başlatmaları özetler. (24 saatlik tam koşu henüz yapılmadı.)

---

## 20. Derleme, yükleme ve dağıtım

### 20.1 Ortamlar (`platformio.ini`)

| Ortam | Açıklama |
|---|---|
| `cyd` | ILI9341, normal sürüm |
| `cyd_st7789` | ST7789 panelli kartlar |
| `cyd_endurance` | Dayanıklılık testi |
| `cyd_prof` | LVGL profil ölçümü açık |
| `native` | Bilgisayarda birim testleri |

Ortak bayraklar: `-O2`, `RUMY_VERSION`, ArduinoJson ayarları, `CORE_DEBUG_LEVEL=1`; kaynak dosyalar `-Wall -Wextra`
ile derlenir ve **uyarısız** tutulur. Flash QIO 80 MHz, CPU 240 MHz.

### 20.2 Derleme betikleri

- `tools/build_web.py` (öncesi): `web/index.html` → gzip → `src/webui/web_assets.h`.
- `tools/post_build.py` (sonrası): `dist/Rumy.bin` (yalnızca uygulama), `dist/Rumy-full-0x0.bin` (önyükleyici +
  bölüm tablosu + uygulama + güncelleyici, tek dosya), ST7789/dayanıklılık/profil sürümleri ve
  `dist/size_report_<ortam>.txt`; uygulama 2 MB hedefini aşarsa uyarır, uygulama bölümüne (2,5 MB) ve güncelleyici
  bölümüne (448 KB) sığmayı ayrıca yazar. Güncelleyici (`dist/Rumy-guncelleyici[-st7789].bin`) yoksa ya da
  kaynaklarından eskiyse önce `pio run -d updater` ile derlenir; USB yüklemesi onu da 0x290000'e yazar.

### 20.3 Yükleme

- Kaynak koddan: `pio run -e cyd -t upload` (port `secrets.ini`'den ya da `--upload-port COM7`) (CH340 otomatik reset, BOOT tuşu gerekmez).
- Yalnızca uygulama (ayarlar korunur): `esptool write_flash 0x10000 dist/Rumy.bin`.
- Tam imaj: `esptool write_flash --erase-all 0x0 dist/Rumy-full-0x0.bin`.
- Önce yedek: `esptool read_flash 0 0x400000 yedek/...bin` (bu projede karttaki Bruce yazılımının yedeği alındı).
- Launcher ile: `Rumy.bin` SD kartın köküne, Launcher menüsünden kurulum (Launcher kendi bölüm tablosunu
  kullandığı için bu kurulumda kablosuz güncelleme yoktur).
- **Kablosuz (25 Ekim'den sonra):** `python tools/spotify_bridge.py --guncelle` (bölüm 29). Yeni bölüm tablosunun
  karta bir kez USB ile yüklenmiş olması gerekir.
- **Not:** Spotify köprüsü seri portu tutar; yüklemeden önce kapatılmalıdır.

Geliştirme bilgisayarında kart ön paneldeki USB hub'a takılıyken bağlantı kopmaları (Windows "Kod 43") görüldü;
arka porta takılınca sorun kalmadı.

---

## 21. Kod kuralları ve kısıtlar

Projenin başından beri uyulan kurallar (katkı adımları için bkz. [CONTRIBUTING.md](CONTRIBUTING.md)):

- **C++ kodunda hiç yorum satırı yok.** Açıklamalar README'de ve bu dokümanda; kod, adlandırmayla okunur tutuldu.
  (Python araçlarının başında kullanım açıklaması vardır.)
- **WiFi ve kişisel ayarlar kodda sabit değil:** NVS'te durur; `secrets*.ini` ve `yedek/` `.gitignore`'dadır.
- LVGL çağrıları yalnızca UI görevinde.
- Döngülerde `String` birleştirme yok (heap parçalanması), bloklayan `delay()` yok, her ağ işleminde zaman aşımı var.
- Büyük bellek yalnızca açılışta ayrılır; çalışma sırasında büyük `malloc` yok.
- Derleme uyarısız (`-Wall -Wextra`).
- Arayüz metinleri Türkçe; seri günlük satırları ASCII'ye yakın Türkçe.

---

## 22. Sürüm geçmişi

Sürüm notları [CHANGELOG.md](CHANGELOG.md) dosyasındadır. Tasarım kararları ve ölçümler ilgili bölümlerde anlatılır.

---

## 23. Karşılaşılan sorunlar ve çözümleri

| Sorun | Kök neden | Çözüm |
|---|---|---|
| USB bağlantısı kopuyor, "Kod 43" | Ön paneldeki USB hub | Kart bilgisayarın arka USB portuna takıldı |
| HTTPS istekleri hiç başlamıyor | WiFi bağlıyken yalnızca 52,9 KB boş heap | SD tamponları, LVGL havuzu ve ağ yığını küçültüldü; eşik 64 KB |
| Dokunmatik ayna gibi | Kartta X ekseni ters | `offset_rotation` 6 |
| Sayfa geçişi 5–7 fps | Her karede tam ekran çizimi | ILI9341 donanım kaydırması |
| Hava ekranı 300 ms'de açılıyor | İkonlar SD'den | İkonlar flash'a gömüldü |
| SPI 80 MHz | Panel bu hızda güvenilmez | `spitest` ile ölçülüp 40 MHz'de kalındı |
| Kapaklar gelmiyor | Flash silinirken UART kesmesi çalışamıyor | Akış denetimi: flash işlemi yalnızca hat boşken |
| Kapak 5,8 sn'de geliyor | Köprüde 0,2 sn'lik okuma zaman aşımı | `read(in_waiting or 1)` → 1,2 sn |
| Ses düğmeleri çalışmıyor | Kenardaki düğmelerin dokunma alanı dar | Dokunma alanı 10 px genişletildi |
| Gözler varsayılan kısık | Gece öncesi "uykulu" kuralı | Kural kaldırıldı |
| "y" kuyruğu kesik | Nesne yüksekliği font satırından kısa | 29 px |
| Ritim hi-hat'e kilitleniyor | Tiz bantlar baskın | 3 kHz alt sınır, bas 4× |
| Köprü ana döngüsü takılıyor | PyAudio COM'u STA başlatıyor, winrt geri dönüşleri gelmiyor | Ses kütüphanesi kendi iş parçacığında |
| Ses verisi gelmiyor | Müzik varsayılan olmayan Bluetooth kulaklıkta | Tüm çıkışlar dinlenip aktif olan seçiliyor |
| Köprü açılışta çöküyor | PowerShell çıktısında Türkçe karakter | UTF-8'e zorlanıp hataya dayanıklı çözme |
| Tempo 86/129/65 arasında gidip geliyor | Tek ölçümlük özilinti, oktav belirsizliği | Birikimli özilinti, oktav düzeltmesi, oylama, bekletme |
| Bluetooth'ta Rumy ritmin önünde | Kablosuz gecikme | Bluetooth tespit edilince 200 ms gecikme payı |
| Pi'de ekran boş | `fbcon=map:9` ile ekran hattını açan yok | Rumy hattı kendisi açıyor (`SETCRTC`) |
| Pi'de renkler bozuk | Paketli RGB565 karışımında kırmızının kesirli bitleri maviye taşıyor | Kanallar 32 bite ayrıldı |
| Pi'de alta basınca üst algılanıyor | Bu kartta dokunmatik Y ters | `dokunmatik 2`, ince ayar ekranı |
| WiZ açılıp kapanmayı gösteriyor ama renk değişmiyor | Ampul yazılımı 1.38.0 imzasız komutu reddediyor (-32602) | WiZ uygulamasında "Tüm kontrollere izin ver" |
| Pi'de hava ekranı kaydırması ~10 fps | SPI 22 MHz'de tam ekran kare 108 ms | `spi_hiz.sh 40` denenecek |
| WiFi'den ekran görüntüsü alırken köprü kopuyor (3 Ekim) | Boş heap hız eşiğinin (60 KB) altına indi, her parçada beklendi; aynı anda HTTPS 47–52 KB tuttu; gönderim 1 sn'de zaman aşımına düştü | Görüntü sırasında TLS başlamaz, eşikler 40/24 KB, zaman aşımı 4 sn |
| AFAD/Nager günlerce çekilmiyor (27 Ekim) | Statik RAM 28 KB büyüdü, boş heap 36–46 KB'ta kaldı, HTTPS ≥ 64 KB istiyor | Köprü HTTPS'i bilgisayarda yapıyor; köprü yoksa Kandilli (şifresiz) |
| CYD'de plak kare içinde dönüyor | Plak 25 → 17 px yapılırken çizim ölçüleri küçülmedi (12,4 px yarıçap 17 px karede), kenar halkası kareyi dolduruyordu | Ölçüler `DESIGN_PX = 25`'e göre ölçekleniyor; Pi kareleri aynı kaldı |

---

## 24. Bilinen sınırlar ve açık işler

- **Veri kaynakları resmî değil:** maç verisi ESPN'in ve LiveScore'un herkese açık ama belgelenmemiş servislerinden
  gelir; biçim değişirse ayrıştırıcıların güncellenmesi gerekir. Bir deneme makinesinde LiveScore'un sertifikası
  doğrulanamadı; bu durumda hızlı gol bildirimi çalışmaz ve goller ESPN'den (50–100 sn geç) gelir. Oyuncu
  fotoğraflarında TheSportsDB'nin ücretsiz deneme anahtarı istek reddedebilir (403); araç o zaman Wikimedia Commons ve
  Vikipedi'ye geçer.
- **Deprem bölgesi Marmara'ya sabit:** AFAD sorgusu ve Kandilli süzgeci Marmara kutusunu kullanır
  (`rc::marmaraBox`, 39,5–41,8 K, 26,0–30,5 D). Başka bir ildeki kullanıcı için kutunun konuma göre seçilmesi açık iş.
- **Bellek payı dar:** statik RAM ~89 KB; WiFi bağlıyken boş heap 36–46 KB'ta kaldığı için kart HTTPS'i kendisi
  yapamıyor (eşik 64 KB). AFAD ve Nager köprü üzerinden ya da Kandilli'den gelir (bölüm 10.6). Heap'in en büyük boş
  bloğu uzun çalışmadan sonra ~37–45 KB'a iner; 24 saatlik dayanıklılık testi (`cyd_endurance`) henüz koşulmadı.
- **Köprü yalnızca Windows'ta:** Spotify ekranı, kulaklıklı Rumy, kod ajanı ışığı, yazıcı kamerası, hızlı gol ve oyuncu
  fotoğrafları bilgisayardaki köprüyü ister (SMTC ve WASAPI Windows'a özgü). Köprünün açılışta başlaması için bir kez
  `--autostart ac` gerekir.
- **Ritim takibi**, vuruşu belirsiz şarkılarda (6/8, yavaş balad) yarım ya da bir buçuk tempoya kayabilir; Bluetooth
  gecikmesi cihaza göre değişir (`--beat-offset`).
- **Yazı tipleri** yalnızca Latin, Latin Genişletilmiş-A ve gereken simgeleri içerir; Japonca, Arapça ve emoji şarkı
  adlarında görüntülenmez (köprü atar ya da aksansız karşılığına çevirir).
- **Flash ve IRAM:** uygulama 2,5 MB'lık `app0` bölümünün ~%85'ini kullanıyor; IRAM ~123/128 KB. Küçültmek gerekirse
  ilk adaylar büyük yazı tipleri ve gömülü resimlerdir.
- **Kablosuz güncelleme** SD kart ister (imaj + 256 KB boş yer); Launcher kurulumunda ve Pi'de yoktur. Kartın panel
  türü denetlenmez: ST7789'lu karta ST7789 imajı elle seçilmeli.
- **Yazıcı:** Creality'nin hata kodu tablosu yok; ekranda yazıcının kendi kodu gösterilir. Kamera görüntüsü köprü,
  go2rtc ve `av` ister; kare alınırken arayüz görevi bekler (kabin ışığı açıkken kare başına ~200 ms).
- **SD kart bir sıfırlamadan sonra takılamayabilir:** esptool kartı bir SD işlemi ortasında sıfırlarsa kart cevap
  vermez hâlde kalır ve yalnızca güç kesilince düzelir. Öneri: `sdcard::begin` başarısız olursa CS düşükken ≥ 600 bayt
  0xFF gönderip yeniden denemek.
- **Baskı sürerken HTTPS:** yazıcı soketinin tamponları birikince en düşük boş heap ~7 KB'a inebilir (yalnızca kart
  HTTPS yaparken; köprü varken olmaz). Öneri: TLS isteği süresince yazıcı soketini kapatmak ya da `SO_RCVBUF`'u
  küçültmek.
- **Raspberry Pi sürümü deneyseldir:** CYD kadar sınanmadı. 480×320 kod ajanı sayfasında proje adının "y" kuyruğu alt
  satıra 2 px değiyor (simülatör denetimindeki tek bulgu).

---

## 25. Creality K2 yazıcı ekranı ve kamera

Amaç: Spotify ekranı gibi bir yazıcı sayfası (Spotify'ın solunda); baskının durumu, hata varsa belirtilmesi ve hata
anında o sayfanın kendiliğinden açılması (gece de), mümkünse yazıcının kamerası ve bitişi gösteren bir çubuk.

### 25.1 Araştırma ve seçilen yol

Geliştirmede kullanılan yazıcı ağda tarandı (yalnızca port yoklaması); model kodu F021. Yazıcıya yalnızca okuma istekleri gönderildi.

| Port | Servis | Kullanım |
|---|---|---|
| 9999 | Creality'nin yerel WebSocket'i (Creality Print'in kullandığı) | **Durum, ilerleme, kalan süre, hata kodu** |
| 7125 | Stok Moonraker (Klipper) | Denendi: şifresiz çalışıyor ama dilimleyici "Unknown" olduğu için tahmini süre ve kalan süre yok, Creality'ye özgü hatalar yok |
| 8000 | `webrtc_local` (kamera) | Yalnızca WebRTC (H.264, 1280×720, ~17 fps, anahtar kare 1 sn'de bir); MJPEG ya da tek kare adresi yok |

Protokol, açık kaynak Home Assistant entegrasyonu **ha_creality_ws**'in kodundan çıkarıldı (bağlanınca
`{"method":"get","params":{"ReqPrinterPara":1}}`, yazıcının `{"ModeCode":"heart_beat"}` mesajına `ok` cevabı,
mesajların kısmi güncelleme olup birleştirilmesi). Kamera için **go2rtc**'nin K2'ye özel `#format=creality` desteği
kullanıldı (K2 RTP'yi SDP cevabında bildirmediği bir payload tipiyle gönderir; genel WebRTC istemcileri bağlanır ama
görüntü alamaz). Kaynaklar: github.com/3dg1luk43/ha_creality_ws, helixscreen.org (K2 sayfası), go2rtc #1600,
sw3defy.github.io (K2 Plus kamera sayfası), github.com/skilly00/K2-Camera-Fix, github.com/pdromnt/k2dash.

### 25.2 Ayrıştırıcı (`lib/rumy_core/src/k2.*`, bilgisayarda test edilir)

- `k2Parse`: ArduinoJson filtresiyle yalnızca gereken alanları okur: `state`, `printProgress`, `printLeftTime`,
  `printJobTime`, `printStartTime`, `printFileName`, `layer`, `TotalLayer`, `nozzleTemp`, `targetNozzleTemp`,
  `bedTemp0`, `targetBedTemp0`, `boxTemp`, `err{errcode,key,value}`, `withSelfTest`, `materialStatus`, `hostname`,
  `model`, `curFeedratePct`, `ModeCode`. Sıcaklıklar kimi zaman metin ("23.330000") kimi zaman sayı gelir; ikisi de
  okunur. Kısmi mesajlar önceki durumun üstüne yazılır.
- `k2Phase`: hata kodu ≠ 0 → Hata; `withSelfTest` 1–99 → Kalibrasyon; `state` 1 Yazdırılıyor, 2 Tamamlandı,
  4 Durduruldu, 5 Duraklatıldı; diğerleri Hazır. `k2Runout`: `materialStatus` 1 ve baskı sürüyor/duraklatılmış.
- `k2JobName`: "Telefon_Standi_V2.stl_PLA_54m36s.gcode" → "Telefon Standi V2" + malzeme "PLA".
- Test sırasında bir hata yakalandı: tam sayılar float üzerinden okunduğu için `printStartTime` (1790423916)
  1790423936 oluyordu; tam sayılar artık doğrudan okunuyor.

### 25.3 Kartta bağlantı (`src/printer/printer.cpp`)

- Ayrı görev açılmadı: istemci **ağ görevinin döngüsünde** (`printer::service`, 40 ms'de bir) çalışır ve ağ görevinin
  8 KB'lık JSON alanını ortak kullanır (ağ görevi işleri sırayla yaptığı için çakışma yok). Böylece ek yığın ve ek
  JSON belleği gerekmedi.
- lwIP soketiyle **bloklamayan** bağlantı (3 sn zaman aşımı), HTTP Upgrade el sıkışması (anahtar `mbedtls_base64`),
  maskeli istemci çerçeveleri, ping/pong, kapanma. Mesajlar **ara bellek olmadan, soketten akış hâlinde**
  ayrıştırılır (`WsSource`, 128 B'lık okuma tamponu); mesaj boyu sınırı yok (tam durum mesajı 1,9 KB).
- 30 sn'de bir durum isteği; 45 sn hiç veri gelmezse yeniden bağlanma; bağlanamazsa 5 sn'den 60 sn'ye kadar artan
  bekleme. Her okuma/yazma 400 ms zaman aşımlı.
- Sonuç `store::PRINTER` ile arayüze gider (saniyede en fazla bir; evre ya da hata değişince hemen).
- Adres NVS'te (`phost`): telefon uygulamasındaki "3D yazıcı" bölümü ya da seri `yazici 192.168.1.19`. Yalnızca IPv4
  kabul edilir (`rc::k2HostValid`, 4 parça, 0–255, baştaki sıfır yok; `settings::sane` ve web sayfası aynı kuralı
  uygular). Önceden ad da (ör. "K2-YAZICI") kabul ediliyor ama bağlanılamıyordu.

### 25.4 Ekran ve davranış

- **Yer:** Ana ekrandan sağa kaydırınca Spotify (köprü bağlıysa), bir kez daha kaydırınca yazıcı; köprü yoksa
  doğrudan yazıcı. Sola kaydırınca geri.
- **Düzen** (üç özel çizim nesnesi, LVGL havuzunda ana ekranın üstüne ~900 B): solda 160×120 kamera kutusu; sağda
  yazıcı adı, renkli durum rozeti, iş adı, malzeme ve katman, nozul/tabla/kabin sıcaklıkları ya da kırmızı hata
  kutusu; altta yüzde, bitiş saati ("Bitiş 15:12" = şimdi + `printLeftTime`), bitişi gösteren çubuk, kalan ve geçen
  süre. Tamamlanınca yeşil çubuk ve "Süre 56 dk"; bağlantı yoksa IP ve "Açık ve aynı ağda mı?"; ayarlı değilse
  kurulum yönergesi.
- **Hata anında** (`ui::printerChanged`): yeni bir hata kodu ya da filament bitmesi görülünce hangi ekranda olursa
  olsun yazıcı ekranı açılır, çerçeve kırmızı olur. Gecenin içindeyse ekran **60 sn uyanır**,
  sonra gece ekranına dönülür; gece ekranında kırmızı "Yazıcı hatası · kod …" satırı kalır. Aynı hata tekrar
  tetiklemez.
- Baskı sürerken ya da hata varken yazıcı ekranı "ana ekrana dönüş" süresiyle kapanmaz.
- **Ana ekran:** bilgi kartı dönüşüne "Baskı %45 · 1 sa 12 dk kaldı" kartı eklenir (dokununca yazıcı ekranı); hata
  varsa kart kırmızı ve öncelikli; baskı bittikten sonra 30 dk "Baskı bitti: …". **Gece ekranı:** baskı sürerken
  sönük "Baskı %45 · bitiş 04:22".

### 25.5 Kamera

- **Bilgisayar (köprü, `PrinterCamera`):** kart yazıcı ekranını açınca 5 sn'de bir `@cam on <ip>` gönderir. Köprü
  `tools/go2rtc/go2rtc.exe`'yi (v1.9.14) yalnızca 127.0.0.1'den erişilebilir şekilde, penceresiz başlatır ve bir
  Windows iş nesnesine bağlar (köprü nasıl kapanırsa kapansın go2rtc de kapanır). Görüntü RTSP'den PyAV ile çözülür;
  saniyede bir son kare 4:3 kırpılıp 160×120'ye küçültülür ve ≤ 6 KB olana kadar kalite düşürülerek JPEG yapılır.
  İstek 12 sn gelmezse gönderim durur, go2rtc 60 sn sonra kapanır. Tüm kareleri çözmek bilgisayarın işlemcisinin
  ~%1'i.
- **Aktarım:** `@cam <bayt> <crc32>` → kart `@go <crc32>` → 2048 baytlık parçalar, her birinden sonra `@ok <n>` → `@cam ok`
  ya da `@cam err …` (kapak aktarımıyla aynı akış denetimi; seri tampon taşmaz).
- **Kart (`src/media/camera.cpp`):** JPEG RAM'de biriktirilmez; seri porttan **gelirken** TJpgDec (LovyanGFX'in
  `lgfx_jd_*` işlevleri) ile çözülüp kamera kutusuna doğrudan panele basılır (`display::drawJpeg`), CRC sonda
  denetlenir. Yalnızca 4 KB'lık çözücü alanı, yazıcı ekranı açıkken ayrılır. İlk tasarımda 10 KB'lık kare tamponu
  vardı; kamera açıkken HTTPS istekleri sırasında en düşük boş heap 9 952 B'a indi, akışlı çözmeyle 16 972 B'a çıktı.
  LovyanGFX'in hazır `drawJpg`'si her karede 3,9 KB `malloc` ettiği için kullanılmadı.
- Kamera kareleri gelirken LVGL o bölgeyi çizmez (yer tutucu yalnızca kare yokken çizilir); ekrandaki kırmızı
  çerçeve yalnızca hata durumu değişince yeniden ayarlanır (her güncellemede ayarlamak bütün ekranı yeniden çizdirip
  kamera görüntüsünü siliyordu).

### 25.6 Doğrulama ve ölçümler (kartta)

- Gerçek yazıcıya bağlandı: "K2-YAZICI · Tamamlandı · %100 · Telefon Standi V2"; 51 mesajda 0 ayrıştırma hatası; yanlış
  IP'de "Bağlantı yok", doğru IP'ye dönünce yeniden bağlandı.
- Kamera: 120 kare, 0 hata; karanlık karede (~960 B) alma + çözme + çizme 33 ms. Renkli bir deneme resmi karta
  gönderilip `ekran goruntusu` ile geri okundu: renk, konum ve boyut birebir doğru.
- Bellek: yazıcı ekranı LVGL havuzunda +~900 B (ekran ihtiyaç tablosunda 3 KB), en yüksek tepe değişmedi
  (16 440 B). Heap: açılışta 79,3 KB (önce 80,3 KB; yazıcı durumu kopyaları ve bağlantı), bağlantı açıkken ~78,5 KB,
  kamera açıkken 74,3 KB. Ağ görevi yığınında en az 4 652 B boş.
- 4 ortam uyarısız derlendi, 79/79 test geçti, simülatörde 9 yeni ekran görüntüsü.
- **Canlı baskı (27 Eylül 20:38, `ekran goruntusu`):** "Yazdırılıyor", Telefon Standi V2, PLA, katman 14/198, nozul
  222/220°, tabla 50°, kabin 28°, %8, bitiş 21:25, kalan 47 dk, geçen 6 dk. 20:38 + 47 dk = 21:25, yani bitiş saati
  tutarlı. Kamera kabin ışığı açıkken baskıyı net gösterdi. Aynı baskıda HTTPS sırasındaki bellek düşüşü bulundu
  (bölüm 24).

---

## 26. Maç takibi (ESPN)

Amaç: takımın maçında Rumy'nin gol, oyuncu değişikliği ve kart kararlarını aktarması, gece modunda hiç uyandırmaması.
İlk sürüm yalnızca Fenerbahçe içindi; 1.3.0'dan beri takım Süper Lig ve beş büyük ligden seçilir (bölüm 26.7).

### 26.1 Kaynak araştırması

| Kaynak | Sonuç |
|---|---|
| ESPN (belgelenmemiş, herkese açık) | Seçildi. Hesap/anahtar yok, **şifresiz HTTP** çalışıyor (CYD'de TLS belleği gerekmez), Süper Lig + Avrupa kupaları + kupa, gol/kart/değişiklik dakikasıyla. Risk: resmî değil, biçim değişebilir |
| API-Football (resmî) | Ücretsiz plan günde 100 istek, hesap + anahtar; 30 sn'lik sorguyla ~50 dk'da biter |
| football-data.org | Ücretsiz planda Süper Lig yok |
| SofaScore, FotMob | Resmî değil, tarayıcı dışı istemcileri engelliyor / imza istiyor |

Ölçülen ESPN uçları (Beşiktaş–Fenerbahçe 5 Eylül maçıyla):

| Uç | Boyut | Kullanım |
|---|---|---|
| `site…/soccer/all/teams/436/schedule?fixture=true` | 276 KB (35 maç) | 6 saatte bir sıradaki maç; akıştan maç maç ayrıştırılır |
| `sports.core…/leagues/all/events/<id>/competitions/<id>/status` | 341 B | Dakika, devre, durum (`pre`/`in`/`post`, `STATUS_HALFTIME`) |
| `…/plays?limit=10&page=<n>` | ≤ 19 KB | Gol (penaltı, kendi kalesine), sarı/kırmızı kart, değişiklik; her olayda o anki skor |
| `sports.core…/soccer/athletes/<id>` | 4,8 KB | Değişiklikte çıkan oyuncunun adı (olayda yalnızca bağlantısı var) |

Değişiklik olayındaki ad **giren** oyuncudur (kadro verisinde `subbedIn`), ikinci katılımcı çıkandır.

### 26.2 Kod

- `lib/rumy_core/src/espn.*` (bilgisayarda testli, `test/test_espn`, 8 test): yol üreticileri, akış ayrıştırıcıları
  (üst düzey anahtarı bulup diziyi eleman eleman filtreyle okur, 8 KB JSON alanı yeter), `matchApply` (yalnızca yeni
  sıradaki olayları ekler, golün kime yazıldığını skordaki artıştan bulur, en son 20 olayı tutar), Süper Lig takım
  adlarının Türkçesi ("Besiktas" → "Beşiktaş", "BJK"), kupa adları.
- `src/match/match.*` (ağ görevinde, `mac` işi): fikstür 6 saatte bir; başlamaya 15 dk kala 60 sn'de bir durum,
  maç sırasında 20 sn'de bir durum + yeni olay sayfaları (en çok 3 sayfa, 2 oyuncu adı). İş periyodu artık işe göre
  değişebiliyor (`Job::periodFn`). Rumy açıldığında maç sürüyorsa ilk çekilen olaylar "tohum" sayılır (`seedSeq`),
  bildirim çıkmaz; ESPN olay silerse (VAR) liste yeniden kurulur, yine bildirim çıkmaz. Bitince sonuç 3 saat kalır.
  Takım NVS'te (`team`, `takim` komutu; 0 = kapalı).
- `src/ui/match_ui.*`: üst katmanda bildirimler. Fenerbahçe golü (`ours`) tam ekran animasyonlu gol ekranı (`goal_screen.*`, 11 sn); diğerleri üstte kayarak gelen 6 sn kutu.
  Sırayla gösterilir; gece modunda ya da bench sırasında atlanır; maç ekranı açıkken yalnızca bizim golümüz çıkar.
- `src/ui/scr_match.cpp`: maç ekranı (kaydırılabilir olay listesi, tek çizim nesnesi).
- Ana ekran: `CARD_MATCH` (maç günü; başlamaya 3 saat kala sabit; canlıyken sabit lacivert kart; bitince 3 saat).
- Yeni yazı tipi `font_rumy_goal` (40 px, yalnızca "GOL!"; Pi 54 px), ikonlar `FUTBOL`, `EXCHANGE`.
- Pi: `net_pi.cpp` iş tablosu, gövde sınırı 256 → 512 KB (fikstür 276 KB).
- Deneme: `mac test …` ağ görevinde sahte bir maç kurar (10 dk), `mac test kapat` gerçek veriye döner.

### 26.3 Bellek

`rc::Match` 1 496 B; ağ görevinde, store'da ve arayüzde birer kopya (~4,5 KB statik RAM). Olay sayfası 480 B.
Simülatörde LVGL tepesi değişmedi (24 760 B); gol ekranı açıkken havuzda +550 B.
Şampiyonlar Ligi listesi (26.4) ayrı yapıda: `rc::Board` 1 760 B, üç kopya.

### 26.4 Lig günü: Süper Lig ve Şampiyonlar Ligi'nin bütün maçları (13 Ekim 2026)

Takım takibinden ayrı, `cfg::BOARD_LEAGUES` (`tur.1`, `uefa.champions`) liglerinin o günkü bütün maçlarını izleyen
ikinci bir ağ işi (`lig`). Her turda ligler sırayla çekilir; biri hata verirse diğerininki yine yayınlanır. Her maçta
`league` alanı var; ilk çekiş "tohum" sayımı lig başına (`Board::seeded` bit maskesi). Süper Lig puan günü 15–50 KB
(10 Ekim: 4 maç 42,6 KB), maç olmayan gün 1,8 KB. Süper Lig listesinde bütün satırlar Türk takımı olduğundan gri
vurgu yalnızca Avrupa liglerinde; seçili takımın satırı her ligde takım renklerinde (takım numarasıyla).

- **Kaynak:** `site.api.espn.com/apis/site/v2/sports/soccer/uefa.champions/scoreboard?dates=YYYYMMDD` (şifresiz
  HTTP). Tek istekte o günün bütün maçları: durum, dakika, skor ve `details` içinde her gol (dakika, takım, golcü,
  penaltı / kendi kalesine). Ölçülen: maç olmayan gün 1,8 KB, 9 maçlık gece başlamadan 115 KB, 6 maçı bitmiş gece
  71 KB. Daha hafif uçlar denendi: `personalized/v2/scoreboard/header` 137–716 KB (daha büyük), core `events` yalnızca
  bağlantı veriyor (skor için maç başına ayrı istek).
- **Kod:** `lib/rumy_core/src/espn_board.h` + `espn.cpp` (`espnParseBoard`, testli: gerçek 10 Eylül verisinden
  iki aşamalı fikstür, `test/test_espn`): olaylar dizisi maç maç filtreyle okunur; her maçta gol sayısı tutulur,
  yeni goller sıra numarasıyla `goals` halkasına (8) eklenir. İlk çekişte (gün başı ya da açılış) var olan goller
  tohum sayılır, bildirim çıkmaz; VAR ile gol silinirse sayı düşer, bildirim çıkmaz. Kendi kalesine gollerde takım
  tarafı skorla tutmazsa çevrilir. Fenerbahçe'nin kendi maçı listede vurgulu durur ama golleri bu işten bildirilmez
  (onları `mac` işi tam ekran gösterir). Türk takımlarının adları Türkçe (`espnTeamTurkish`), diğerleri ESPN'in kısa
  adı ("Man City", "S Bratislava").
- **Zamanlama** (`src/match/board.*`): maç varken 60 sn'de bir; ilk maçtan 10 dk önce başlar; o günün maçları
  bitince ya da maç yoksa gece yarısından 2 dk sonraya (en çok 6 saat) kadar bekler. Gün değişince liste silinir.
  Seri komut: `mac` raporunun altında `[sl]` satırı ve maç listesi; `sl yenile` hemen çeker; her yeni gol tanılama
  günlüğüne yazılır.
- **Arayüz:** `match_ui` önce takımın olaylarını, sonra ŞL gollerini sırayla gösterir: üstte 6 sn kutu, solda
  lig rozeti, "Galatasaray 1–1 Barcelona" (golü atan taraf parlak), altında top ikonu + golcü + dakika (penaltı
  "pen.", kendi kalesine "k.k."). Gece modunda ve bench sırasında çıkmaz. Maç ekranında olay listesinin altında
  "Şampiyonlar Ligi · bugün" bölümü: saat/dakika (canlıysa kırmızı), ev sahibi, skor, deplasman; Türk takımlı satır
  gri, takımın satırı takım renklerinde. Taslaklar `docs/preview_film/*sl_*`.
- **Bellek:** `rc::Board` 1 760 B (18 maç, 8 gol); ağ görevinde, store'da ve arayüzde birer kopya (5,3 KB statik RAM).
  JSON alanı: 64 bit bilgisayarda bu fikstürün tepesi 9 KB (yuva 16 B); kartta yuva 8 B olduğundan aynı veri ~5 KB,
  8 KB alana sığar. Bilgisayar testi bu yüzden 16 KB alanla koşar. Çok kartlı bir maçta (≈30 olay) alan yetmeyebilir;
  o zaman çekiş `E_PARSE` döner ve bir sonraki dakikada yeniden denenir.
- **Kartta ölçüldü (13 Ekim 21:57, yalnız ŞL):** 9 maçlık gece 116 524 B, 613 ms; JSON alanı tepe 4 376 / 8 192 B; boş bellek 69 →
  63 KB, ağ yığını 5,9 KB boş.
- **İlk yüklemede yakalanan hata:** maç öncesi olaylarda `competitors[].leaders[].leaders[].athlete.links[].rel[]` 13 kat
  iç içe; ArduinoJson'ın varsayılan derinlik sınırı 10 olduğundan üçüncü maçta `E_PARSE` (-6) alındı (bitmiş maçlarda
  bu alan yok, testte görünmedi). `eachElement` artık `NestingLimit(24)` ile okuyor; maç öncesi gerçek veriyle test
  eklendi (`espn_board_pre.json`).
- **Canlı doğrulama (13 Ekim 22:10):** 14 istek, 0 hata, 7 canlı maçta 3 gol yakalandı (Atlético 1-0, Viking 1-0,
  Villarreal 0-1). İki ligli sürümle (22:16): tur 2 istek, 108 KB, 2,2 sn; boş bellek 63,6 KB, ağ yığını 5,8 KB boş.

### 26.5 Futbol sayfası (13 Ekim 2026)

- **Gezinme:** ana ekrandan sağa kaydırınca Futbol (`SCR_FOOTBALL`); oradan sağa Spotify / yazıcı, sola ana ekran.
  Lige dokununca `SCR_LEAGUE`; sağa kaydırınca ya da başlıktaki ‹ ile Futbol'a döner. Seri: `ekran futbol`,
  `ligsayfa <tur.1|uefa.champions|eng.1|esp.1|ita.1|ger.1|fra.1> [puan]`.
- **Futbol sayfası:** 7 lig (Süper Lig, Şampiyonlar Ligi, Premier Lig, LaLiga, Serie A, Bundesliga, Ligue 1),
  amblem + ad; Süper Lig ve ŞL'de bugünkü durum lig günü işinden (`3 canlı`, `Bugün 4 maç`).
- **Lig sayfası, maç günü:** ESPN `scoreboard` tarih verilmeden "güncel gün"ü döndürür (bitmiş son gün ya da sıradaki
  maç günü); ‹ › bir gün geri/ileri gider, boş günleri en çok 14 gün atlar (her boş gün 1,8 KB). Tarih aralığı
  (`dates=A-B`) bu uçta 400 döndüğü için kullanılmadı. Satırlar lig günü listesiyle aynı çizim (`match_ui::drawGameRow`).
- **Puan durumu** (sağ üst tuş, tekrar basınca maçlar): `apis/v2/sports/soccer/<lig>/standings`, şifresiz HTTP,
  61–69 KB (ŞL 36 takım 143 KB). Akışta `entries` anahtarı aranır (`seekKey`), her takım filtreyle okunur:
  sıra, O, G, B, M, averaj, puan; ESPN'in bölge notu (renk + açıklama) Türkçeye çevrilip soldaki renkli şerit ve
  tablonun altındaki açıklamada kullanılır (Şampiyonlar Ligi, … ön eleme, Küme düşme, Son 16, Play-off, Elendi).
  Takımın satırı takım renklerinde.
- **Kod:** `rc::Standings` (36 satır, 6 bölge), `rc::LeagueView` (maç günü `Board` ya da tablo, birleşik/union,
  1,8 KB), `espnParseStandings`, `espnLeagueSlug`; `src/match/leagues.*` ağ işi `ligsayfa` yalnızca sayfa açıkken
  çalışır (istek sırası UI'dan `volatile` alanlarla gelir, `net::refresh`): tablo 10 dk, maç günü 5 dk, canlı maç
  varken 60 sn; sayfa kapanınca durur. `src/ui/scr_football.cpp` iki ekran (tek çizim nesnesi + kaydırma kabı).
- **Rozetler:** depoda hiçbir kulüp ya da turnuva logosu yoktur. `tools/make_match_art.py` her turnuva için
  `tools/assets/lig_logolari/<ad>.png` varsa onu kullanır (git dışı, kullanıcının kendi koyduğu logolar), yoksa
  kısaltmalı sade bir rozet çizer (SL, ŞL, AL, KL, TK, SK, PL, LL, SA, BL, L1; yazı LVGL paketindeki Montserrat
  yazı tipiyle, OFL 1.1).
- **Kartta ölçüldü:** Süper Lig tablosu 55 KB, Premier Lig tablosu 61 KB / 362 ms, ŞL maç günü 112 KB; boş
  bellek 63 → 58 KB (en az 44 KB), LVGL tepe 8,0 / 22,5 KB, reddedilen ekran 0, ağ yığını 4,7 KB boş.

### 26.6 Hızlı gol bildirimi (17 Ekim 2026)

**Ölçüm (17 Ekim, canlı maçlar).** ESPN her gol için olayın gerçek saatini (`wallclock`) tutuyor; ilk görüldüğü an
bununla karşılaştırıldı (6 sn'de bir skor tablosu ve olay listesi, ayrıca ESPN'in kendi sitesinin anlık kanalı
FastCast `gp-soccer-<lig>-<maç>` JSON-yama akışı):

| Gol | ESPN skor tablosu | ESPN anlık kanal | LiveScore |
|---|---|---|---|
| Inter 44' | +57 sn | – | – |
| PSG 77' | +92 sn | – | – |
| Bayern 26' | +100 sn | – | – |
| PSG 90+3' | +49 sn | +48 sn | – |
| Aston Villa 36' | +69 sn | +73 sn | – |
| Bayern 34' | +69 sn | +81 sn | – |
| Fenerbahçe 47' (İrfan Can) | +93 sn (olay listesi +77) | +69 sn | ölçüm başlarken vardı |
| Barcelona 41' (Rodri) | +82 sn'den fazla | +69 sn | **+22 sn** |
| Bayern 52' (köprü modülüyle) | – | – | **+44 sn** |

ESPN'in gecikmesi kendi veri girişinden (50–100 sn); önbellek 8–10 sn, anlık kanal da hızlı değil. Kartın ESPN'e
soruş aralığı (Fenerbahçe 20 sn, lig günü 60 sn) bunun üstüne ekleniyordu. LiveScore ESPN'den 45–60 sn önde;
VAR iptalini de (Fenerbahçe 47') ESPN'in anlık kanalından 32, skor tablosundan 93 sn önce gösterdi. SofaScore ve
FotMob'un maç ayrıntısı korumalı (403 / imza); korumalar aşılmadı. LiveScore ESPN gibi resmî olmayan bir kaynak.

**LiveScore uçları** (`prod-public-api.livescore.com`, yalnızca HTTPS, CDN önbelleği 10 sn):
`/v1/api/app/live/soccer/3?MD=1` bütün canlı maçlar (~58 KB), `/v1/api/app/incidents/soccer/<Eid>?locale=en` tek
maçın olayları (0,4–4 KB: skor `Tr1/Tr2`, devre devre olaylar). Olay kodları ESPN kayıtlarıyla karşılaştırıldı:
36 gol, 37 penaltı golü, 39 kendi kalesine (`Nm` golü kazanan taraf), 38 iptal edilen gol (`IR: VAR`), 62 kaçan
penaltı, 43 sarı, 44 ikinci sarı, 45 kırmızı, 63 asist. Skor olay listesinden birkaç saniye önce güncelleniyor.

**Köprü** (`tools/fast_goals.py`, köprü açılınca başlar): ESPN'den bugünün Süper Lig ve Avrupa kupası maçlarını ve
takımın fikstürünü alır; maç saatinde LiveScore canlı listesine 5 sn'de bir bakar, maçları takım adlarından
eşler (aksansız, kelime önekleri, ters ev/deplasman), skor artınca olay listesinden golcüyü alıp
`@gol <espn-no> <ev> <dep> <h|a> <g|p|o> <dakika> <oyuncu>` yollar; ad henüz yoksa adsız yollar, 90 sn içinde ad
gelince aynı golü adla bir daha yollar. Skor düşerse `@skor`. 30 sn'de bir canlı maçların `@skor`u (canlılık).
Deneme: `python tools/fast_goals.py --deneme esp.1 ita.1` (17 Ekim'de 5 canlı maçta eşleme doğru, Inter 84' ve
Bayern 52' satırları üretildi).

**Kart:** `src/match/fast.*` köprü satırlarını posta kutusuna (8) koyar ve `mac` işini uyandırır; ağ görevinde
takımın maçına aitse maça (`rc::matchAddFast`: kimliği `MATCH_FAST_ID`li gol olayı, skor), değilse lig günü
listesine (`board::fastMsg`) işlenir. Köprüden 45 sn haber gelmezse ve takımın maçı canlıysa kart kendisi sorar:
maç başına bir kez canlı listeden eşleme (TLS, 58 KB, akışla; olmazsa 2 dk sonra yeniden), sonra 10 sn'de bir
olay listesi (TLS, küçük). ESPN sonra aynı golü getirince skor imzasıyla (ev-dep) eşlenir: hızlı olayın kimliği
ESPN'inkiyle değişir, yeni bildirim çıkmaz (`matchApply`); lig günü listesinde imza bit kümesiyle aynı gol ikinci
kez bildirilmez. Hızlı kaynak taze olduğu sürece (90 sn) skor ondan alınır; ESPN'in geride kalan skoru yanlış
"gol iptal" uyarısı vermez, LiveScore'un düşürdüğü skor ise VAR iptali olarak 25 sn onaydan sonra bildirilir.
ESPN aralıkları da kısaldı: takımın maçı canlıyken 10 sn (önbellek 8–10 sn), lig günü 20 sn.
Seri: `mac` raporunda `[hizli] kaynak kopru|kart|yok · livescore <Eid> · istek · hata · hizli gol · kopru mesaji`,
`[lig] hizli gol N`. Bellek: statik RAM +1,7 KB (LiveScore maçı, imza kümeleri).

### 26.7 Takım seçimi ve takım renkleri (1.3.0)

- **Takım listesi:** `tools/make_teams.py` ESPN'in `soccer/<lig>/teams` uç noktasından altı ligin (Süper Lig, Premier
  Lig, LaLiga, Serie A, Bundesliga, Ligue 1) takımlarını alır ve `web/index.html`'deki `LEAGUES` ve `TEAMS` satırlarına
  yazar: ESPN numarası, ad, kısaltma, iki renk, lig sırası (114 takım, ~5 KB, gzip'te ~2 KB). Süper Lig takımlarının
  Türkçe adları ve kısaltmaları kartın `espn.cpp` tablosundan okunur; ESPN Süper Lig'de çoğu takıma renk vermediği
  (000000) için kulüp renkleri aracın `RENK` tablosundan gelir (birkaç büyük Avrupa kulübünün rengi de düzeltilir).
  Sezon başında yeniden çalıştırılır.
- **Uygulama:** Futbol sayfasında en üstte "Maç takibi" anahtarı (`fut`), altında lig sekmeleri ve renkli rozetli
  takım satırları; rozet yazısı takımın ikinci rengi, zeminle karşıtlığı 3:1'in altındaysa beyaz ya da siyahtır.
  Takım seçilince `{"team", "tc1", "tc2"}` yollanır; "Takımım yok" `team` 0 yapar.
- **Kart:** `settings` NVS'e `fut`, `tc1`, `tc2` ekler. `fut` 0 iken maç ve lig günü işleri sorgu yapmaz (`match.cpp`,
  `board.cpp`), diğer maçların gol bildirimleri gelmez (`match_ui::leagueWanted`) ve köprüye `@futbol 0` gider.
  Seri komutlar: `futbol ac|kapat`, `takim <no> [renk1 renk2]`.
- **Palet:** `rc::teamPalette(c1, c2)` (`lib/rumy_core/src/color.*`) iki renkten koyu zemin, derin ton, şerit, vurgu,
  açık tonlar ve iki ampul rengi üretir. Zemin açık olmayan renkten (HSV değeri 80/255'e indirilerek) gelir; ikisi de
  açık ya da renksizse nötr koyu gri kullanılır. Vurgu, öteki renk zeminle en az 3:1 karşıtlık veriyorsa o renk, değilse
  beyazdır. Ampul renkleri takımın iki renginin doygun hâlidir (renksizse beyaz; ikisi aynıysa biri sıcak beyaz).
  Renk verilmemişse (0, 0) palet bugünkü lacivert-sarı sabitleri verir. `theme::team*()` bu paleti UI görevinde
  döndürür; ayarlar değişince `theme::setTeamColors` yeniden hesaplar. Testler `test/test_color`.
- **Köprü:** Rumy köprüye bağlanınca, ayar değişince ve dakikada bir `@futbol <0|1> <takım> <renk1> <renk2>` yollar
  (`rc::footballLine`). `tools/fast_goals.py` takımın fikstürünü bu numarayla çeker (kapalıysa hiç izlemez);
  `tools/player_photo.py` takımın adını ve ligini ESPN'den bir kez alıp önbelleğe yazar, forma denetimini takımın iki
  rengiyle, Commons aramasını takımın adıyla yapar.
- **Simülatör:** `RUMY_SIM_ONLY=takim python tools/ui_preview.py --film` altı örnek takımla canlı kartı, iki gol
  ekranını ve maç ekranını çizer.

## 27. Spotify çalma listeleri

Amaç: Spotify çalma listelerini Rumy'den açabilmek. Tasarım taslakları
gösterildi (2 giriş, 3 düzen); seçilen: A (liste: kapak + ad + şarkı sayısı), "Karışık" düğmesi, Spotify oynatma
ekranında ayrı bir düğme.

### 27.1 Araştırma

- Şarkı bilgisi Windows'un medya oturumundan (SMTC) geliyor; SMTC çalma listesi bilmez ve liste başlatamaz.
  Listeler ve başlatma için **Spotify Web API** gerekiyor.
- Şubat–Mart 2026 değişiklikleri ([geçiş rehberi](https://developer.spotify.com/documentation/web-api/tutorials/february-2026-migration-guide)):
  geliştirme modundaki uygulamalar için **hesap sahibinin Premium olması şart**, uygulama başına 5 kullanıcı;
  `GET /me/playlists` duruyor, liste içeriği yalnızca sahip olunan listeler için; listenin şarkı sayısı artık
  `items.total` (`tracks` eskidi, ikisi de okunur).
- `PUT /me/player/play` yalnızca Premium; `context_uri` ile liste, `offset.position` ile başlangıç şarkısı.
- Yönlendirme adresi `localhost` olamaz, `http://127.0.0.1:<port>` olmalı. Masaüstü araç olduğu için gizli anahtar
  yerine **PKCE** kullanıldı (client secret gerekmez).
- Beğenilen Şarkılar `spotify:user:<id>:collection` bağlamıyla başlıyor (4 Ekim, gerçek hesap). Spotify'ın kendi
  listeleri (Daily Mix vb.) henüz denenmedi (bölüm 24).

### 27.2 Köprü (`tools/spotify_api.py`, `tools/spotify_bridge.py`)

- `spotify_api.py`: PKCE girişi (`--spotify-giris <client_id>`: 127.0.0.1:8888'de tek seferlik HTTP sunucusu,
  tarayıcıda izin, kod → anahtar), anahtar yenileme (401'de bir kez yeniden dener; yenileme reddedilirse "giriş yok"
  durumuna düşer), `playlists()` (önce Beğenilen Şarkılar sayısı, sonra en çok 30 liste), `context()` (çalan liste),
  `device()` (bu bilgisayarın adıyla eşleşen "Computer" cihazı; yoksa etkin cihaz), `play()` (cihaz yoksa
  `spotify:` ile Spotify'ı açar, 15 sn bekler; "Karışık" açıksa rastgele bir şarkıdan başlar ve karıştırmayı açar).
  Her istekte 8 sn zaman aşımı. Hatalar Türkçe kısa mesaja çevrilir ("Spotify Premium gerekli", "Bilgisayarda Spotify
  açılamadı"...). Anahtarlar `tools/.cache/spotify_api.json` (git dışı), ekrana basılmaz.
- Köprüde `Playlists`: bağlantılar arasında yaşar; listeler 5 dk önbellekte. Ekran açılınca (`@cmd pls`) önbellek
  hemen gönderilir, eskiyse ayrı iş parçacığında tazelenir; her açılışta çalan liste yeniden sorulur. Kapaklar
  128 px olarak indirilip (6 paralel) saklanır.
- **Paftalar:** kart RAM'i dar olduğu için liste kapakları ayrı ayrı gönderilmez; 9'ar kapak ART×ART boyunda tek bir
  "kapak" görüntüsüne dizilir (hücre ART/3 = 48 px, kapak ART×5/18 = 40 px, hücrenin sol üstünde, yuvarlatılmış).
  Kart paftayı normal kapak gibi `@need` ile ister ve aynı flash önbelleğine yazar; liste ekranı paftayı kırparak
  çizer (`du::Clip`). 30 liste = 4 pafta = 4 yuva (21 yuvadan); paftalar yuva seçiminde korunur.

### 27.3 Protokol

| Yön | Mesaj | Anlamı |
|---|---|---|
| Rumy → PC | `@cmd pls` | Liste ekranı açıldı, listeleri gönder |
| PC → Rumy | `@pls <sürüm> <n> hazir\|yukleniyor\|giris\|hata <karışık 0\|1> [mesaj]` | Liste başlığı; sürüm içerik değişmedikçe aynı kalır |
| PC → Rumy | `@pl⇥sıra⇥şarkı_sayısı⇥pafta_hash⇥hücre⇥ad` | Her liste (ad en çok 39 bayt) |
| PC → Rumy | `@plnow <sıra\|-1>` | Çalan liste |
| Rumy → PC | `@cmd plplay <sürüm> <sıra>` | Listeyi başlat; sürüm tutmazsa köprü "Liste değişti" der ve listeyi yeniden yollar |
| PC → Rumy | `@plst <sıra> bas\|tamam\|hata [mesaj]` | Başlatma durumu |
| Rumy → PC | `@cmd plmix 0\|1` | "Karışık" tercihi (köprüde saklanır) |

Ayrıştırıcılar `lib/rumy_core/src/media_proto.*` içinde, bilgisayarda testli (`test/test_media`).

### 27.4 Kart (`src/media/media.cpp`, `src/ui/scr_playlist.cpp`)

- `media::Playlists` (statik ~1,4 KB: 30 × 44 B satır + durum): başlık sürüm değişince listeyi sıfırlar, satırlar
  sırayla eklenir (araya kayıp satır girerse kabul edilmez). Başlatma 25 sn içinde yanıtlanmazsa "Bilgisayardan
  yanıt gelmedi". Köprü koparsa durum sıfırlanır, liste ekranı ana ekrana döner.
- Liste ekranı: sabit başlık (geri, "Çalma listelerim", "Karışık"), altında dikey kayan tek çizim nesnesi (yalnızca
  görünen satırlar çizilir). Çalan liste yeşil, vurgulu ve hoparlör ikonlu; dokununca satır yeşil çerçeve, "Bilgisayarda
  başlatılıyor…" ve dönen halka (yalnızca halkanın alanı 60 ms'de bir yeniden çizilir); başarıda Spotify ekranına
  döner, hatada satır kırmızı çerçeveli ve mesajlı kalır. Uzun adlar "…" ile kısaltılır. Açılınca çalan liste görünür
  olacak şekilde kaydırılır. Boş durumlar: yükleniyor, hesap bağlı değil (giriş komutu), ağ hatası, liste yok.
- Spotify ekranı: kapağın sağ alt köşesinde siyah, vurgu renginde çerçeveli liste düğmesi (32 px, dokunma alanı
  +8 px).
- Yeni ikonlar `LIST`, `RANDOM`; seri port: `ekran liste`, `rapor` → `[medya] listeler …`.

## 28. Kod ajanı ışığı: Claude Code ve Codex (ClawLight uyarlaması)

Önce Claude Code için yapıldı (28.1–28.6), ardından Codex eklendi (28.7).

### 28.1 Durumlar ve görünüm

| Durum | Ne zaman | Ana ekrandaki yıldız |
|---|---|---|
| Çalışıyor (`c`) | İstek gönderildi, araç kullanılıyor | Yeşil, saniyede 60° döner, boyu %84–100 arasında nefes alır |
| Seni bekliyor (`b`) | İzin isteği, soru (`AskUserQuestion`), plan onayı (`ExitPlanMode`), MCP bilgi isteği, API hatası | Kırmızı, parıltısı 1,1 sn'de bir nabız atar |
| Bitti / boşta (`i`) | İş bitti ya da oturum yeni açıldı | Sarı, sabit; 30 dk sonra listeden düşer |
| Yok | Açık oturum yok, köprü kapalı ya da 30 sn haber yok | Gizli |

Toplam renk en acil durumdan gelir (bekleyen > çalışan > biten). Yıldız sekiz ışınlı (uzun/kısa), çizgiyle çizilir
(`agents_ui::drawBeacon`), iki saydam daire parıltısı vardır; ana ekranda şehir adının altında (x 4, y 26, 58×28 alan,
yarıçap 9 px), Claude Code'un çalışırken dönen yıldızına gönderme. Aynı durumda birden çok oturum varsa yanında
sayısı yazar. Animasyon yalnızca ana ekran görünürken ve durum çalışıyor/bekliyor iken 20 fps'te, yalnızca yıldızın
~28×28 px'lik alanı yeniden çizilerek yapılır (simülatör filmi: kare başına ekranın ort. %6–7'si).

### 28.2 Bildirimler ve tepkiler (`src/ui/agents_ui.*`)

- **Durum farkı** `rc::agentsNotes` ile bulunur (önceki ve yeni liste, oturum kimliğiyle): bir oturum beklemeye
  geçince `AN_WAIT`, çalışmadan bitire geçip iş süresi ≥ 120 sn ise `AN_DONE`. Köprü yeniden bağlanınca ilk liste
  tohum sayılır, bildirim çıkmaz.
- **Kutu:** maç bildirimleriyle aynı yer ve biçim (üstte kayarak gelir, 314×68), ikisi aynı anda çıkmaz
  (`match_ui::busy` / `agents_ui::busy`). Bekliyor: kırmızı çerçeve, "Claude seni bekliyor", proje, sebep, 10 sn,
  dokununca oturum sayfası. Bitti: yeşil çerçeve, "Claude işini bitirdi", "7 dk sürdü", 6 sn. En çok 4 bildirim
  sıraya girer. Aynı oturumun bekliyor kutusu en çok dakikada bir çıkar (izin verip duran oturum her seferinde
  ekranı kaplamasın).
- **Rumy:** bekliyor kutusunda yıldıza bakıp göz kırpar (`rumy::poke`, `scr_home::glance`); bitti kutusunda 4 sn
  `HAPPY` (`agents_ui::reaction`, `pickMood`'da maç tepkisinin yanında).
- **WiZ uyarısı** (`wiz::alert`): kutlamayla aynı mekanizma (`handleParty`); ampulün o anki hâli saklanır, 400 ms
  kırmızı %100 → 400 ms eski hâl → 400 ms kırmızı → eski hâl. 30 sn'de en çok bir kez. Ampul çevrimdışıysa ya da hâli
  bilinmiyorsa yapılmaz. Seri `wiz uyari` ile denenir.
- Gece modunda (uyanık değilken) kutu, tepki ve ampul uyarısı yok; yıldız ve sayfa güncellenir.

### 28.3 Oturum sayfası (`src/ui/scr_agents.cpp`, `SCR_AGENTS`)

Başlıkta durumun yıldızı (animasyonlu), "Claude" ve özet ("2 bekliyor · 2 çalışıyor · 1 bitti"); altında en çok 6
satır (34 px): küçük yıldız, proje adı, ikinci satırda ne yaptığı ve süresi ("Dosya düzenliyor · 3 dk", "İzin
istiyor: Bash · 44 sn", "Bitti · 12 dk önce · 7 dk sürdü", "Yeni oturum · 2 dk önce"). Süreler kartta saniyede bir
ilerletilir (`agents::spanS`). Boş hâller: "Açık Claude oturumu yok", "Köprü bağlı değil". Ana ekrandaki yıldıza
dokununca ya da `ekran ajan` ile açılır, sağa kaydırınca geri döner. LVGL ihtiyacı tablosunda 3 KB.

### 28.4 Bilgisayar tarafı

- **Hook** (`tools/claude_hook.py`): Claude Code her olayda `pythonw -S claude_hook.py`'yi `async` çalıştırır
  (exec biçimi, kabuk yok). Betik olay JSON'unu stdin'den okur, `~/.claude/rumy/oturumlar/<oturum>.json`'u bir kilit
  dosyasıyla (O_EXCL, 2 sn bayat) okuyup günceller ve atomik yazar (geçici dosya + `os.replace`). Daha yeni bir olayın
  yazdığı dosyanın üstüne yazmaz (eşzamansız hook'lar sırasız bitebilir). Asla hata vermez, çıktı üretmez.
  Ölçülen süre olay başına 52–72 ms (pythonw açılışı dahil); `async` olduğu için Claude beklemez.
- **Olay eşlemesi:** `SessionStart` (startup/resume/clear) → bitti "Yeni oturum" (`compact` durumu değiştirmez);
  `UserPromptSubmit` → çalışıyor "Düşünüyor" (iş başlangıcı); `PreToolUse` → çalışıyor + aracın Türkçe karşılığı
  (Bash/PowerShell "Komut çalıştırıyor", Edit/Write "Dosya düzenliyor", Read/Grep/Glob "Kod okuyor", Web/tarayıcı
  "Web'e bakıyor", Agent "Yardımcı ajan çalışıyor", MCP "Araç kullanıyor"); `AskUserQuestion` / `ExitPlanMode` →
  bekliyor; `PostToolUse(Failure)` → çalışıyor "Düşünüyor"; `PermissionRequest` → bekliyor "İzin istiyor: <araç>";
  `Notification` (`permission_prompt`, `elicitation_*`, `agent_needs_input`) → bekliyor, `idle_prompt` yok sayılır;
  `Stop` → bitti, iş süresi = şimdi − iş başlangıcı; `StopFailure` → bekliyor "API hatası: kullanım sınırı" vb.;
  `SessionEnd` → dosya silinir. Kaydedilen `pid` hook'un üst süreci, yani oturumun `claude.exe`'sidir (masaüstü
  uygulamasında ölçüldü: oturum başında açılan `claude.exe`, ana `claude.exe`'nin çocuğu).
- **İzleyici** (`tools/claude_agents.py`, köprü açılınca iş parçacığı olarak başlar): klasörü 0,5 sn'de bir okur
  (değişmeyen dosya yeniden okunmaz, `mtime`), 24 saattir haber vermeyen dosyayı siler, 30 dk'dan eski bitenleri ve
  60 sn'dir haber vermeyen, süreci de kapanmış oturumları (`OpenProcess` + `GetExitCodeProcess`) gizler; satırları
  bekleyen > çalışan > biten sırasıyla dizer, proje adını çalışma klasöründen alır (`.claude/worktrees/<ad>` ise
  worktree adı), metinleri bayt sınırına göre kısaltır (proje 27, ayrıntı 39). İçerik değişince (en sık 0,5 sn'de bir)
  ya da 10 sn'de bir gönderir; köprü Rumy'ye yeniden bağlanınca hemen gönderir. Durum geçişlerini köprü günlüğüne
  yazar (`claude: Rumy -> seni bekliyor (İzin istiyor: Bash)`).
- **Kurulum:** `python tools/claude_agents.py --kur` 10 olay için hook ekler (önceki Rumy hook'larını yeniler, başka
  hook'lara dokunmaz), önce `settings.json.rumy-yedek-<tarih>` yedeği alır; `--kaldir` yalnızca Rumy'ninkileri siler;
  `--durum` listeyi yazar; `--deneme` Rumy'ye gidecek satırları yazar. Kurulumdan sonra masaüstü
  uygulamasındaki açık oturumlar yeniden başlatmadan görünür.

### 28.5 Protokol ve kart (`lib/rumy_core/src/agents_proto.*`, `src/agents/agents.*`)

    @ajanlar <sürüm-hex> <satır> <çalışan> <bekleyen> <boşta>
    @ajan⇥sıra⇥kimlik-hex8⇥c|b|i⇥durum_sn⇥iş_sn⇥proje⇥ayrıntı

Sayılar listenin tamamı içindir (6'dan fazla oturum varsa sayfada "+N oturum daha"). Kimlik oturum numarasının
CRC32'si. Çalışıyorda `durum_sn` iş başlangıcından, diğerlerinde durumun başlangıcından beri geçen süre; `iş_sn`
bitende son işin süresi. Satırlar UI görevinde `media::lineEnd`'den `agents::head/row`'a gelir; bütün satırlar gelince
liste tek seferde geçerli olur (`commit`), `agents::version()` artar, UI `M_AGENTS` maskesini dağıtır. 30 sn başlık
gelmezse liste temizlenir. Ayrıştırıcı ve durum farkı bilgisayarda testli (`test/test_agents`, 8 test: başlık, satır,
UTF-8'i bölmeden kısaltma, toplam durum, bildirim farkı, sessiz durumlar, sınır, süre metni).

### 28.6 Bellek ve doğrulama

- Statik RAM +2 136 B (kartta iki `rc::Agents` kopyası, arayüzdeki önceki liste ve bildirim kuyruğu); uygulama
  2 145 344 B. LVGL: simülatörde ana ekran + yıldız 8,8 KB, oturum sayfası 9,8 KB, tepe 12,4 KB (64 bit önizleme);
  sayfa ihtiyacı 3 KB olarak tabloya eklendi.
- Simülatör: `RUMY_SIM_ONLY=ajan python tools/ui_preview.py --film` → 11 ekran ve 4 film (ışığın dönüşü, nabzı,
  bildirimin girişi, sayfa); otomatik denetim 0 bulgu.
- Bilgisayarda: hook 12 olaylık sırayla denendi (her geçiş doğru, 52–72 ms); kurulumdan sonra bu oturumun durumu
  ("Rumy · Komut çalıştırıyor") köprü günlüğünde göründü.
- 4 ortam uyarısız, 135/135 test.

### 28.7 Codex (25 Ekim 2026)

- **Hook'lar:** `~/.codex/hooks.json` (yoktu, oluşturuldu), 8 olay: `SessionStart`, `UserPromptSubmit`, `PreToolUse`,
  `PostToolUse`, `PermissionRequest`, `Stop`, `Interrupt`, `SessionEnd`; komut
  `<python>/pythonw.exe -S .../tools/claude_hook.py codex` (boşluksuz yollar; Codex komutu kabukla çalıştırır,
  tırnaksız yol cmd, PowerShell ve bash'te aynı çalışır), `async`, zaman aşımı 3 sn (`SessionEnd` ve `Interrupt`'ın
  sınırı). Codex hook'a güveni tanımın özetine ve dosyadaki sırasına göre tutar; bu yüzden kurulum başka araçların
  gruplarını yerinden oynatmaz, kaldırma önde kalan Rumy grubunu boş bir yer tutucuya çevirir (ClawLight'la aynı
  yöntem). Codex'in `config.toml`'una hiç yazılmaz: yeni hook'lar kullanıcı Codex içinde (`/hooks` ya da uygulamanın
  uyarısı) güvenene kadar çalışmaz. `codex features list` → `hooks stable true`.
- **Olay eşlemesi:** Claude'unkiyle aynı; ek olarak `Interrupt` (kullanıcı işi durdurdu) → bitti "Durduruldu",
  `SessionEnd` (konuşma arşivlendi/silindi ya da 30 dk kapalı) → dosya silinir, tek seferlik `codex exec` oturumu
  `Stop`'ta silinir (konuşma kaydının ilk satırında `source: exec`). Codex araç adları: `apply_patch` → "Dosya
  düzenliyor" (izin isteğinde "İzin istiyor: dosya düzenleme"), `update_plan` → "Plan yapıyor", `spawn_agent` →
  "Yardımcı ajan çalışıyor", `Bash` → "Komut çalıştırıyor".
- **Oturum süreci:** hook artık üst süreç yerine ata zincirinde ilk `claude.exe` ya da `codex.exe`'yi kaydeder
  (Toolhelp anlık görüntüsü, ctypes; bulunamazsa üst süreç). Codex hook'u bir kabuk üzerinden başlattığı için üst süreç
  hemen kapanıyor, süreç denetimi oturumu yanlışlıkla düşürürdü.
- **Ad:** Codex oturumunun adı Codex'in kendi konuşma başlığıdır: `~/.codex/state_*.sqlite` → `threads.title`
  (`mode=ro` ile salt okunur), yoksa `session_index.jsonl`, o da yoksa çalışma klasörü. Başlık bulunamazsa 30 sn'de bir
  yeniden aranır.
- **Protokol:** `@ajan` satırına ajan alanı eklendi (`claude|codex`, durum ve sürelerden sonra; 8 alan). Kart henüz eski
  biçimle yüklenmediği için geriye uyumluluk gerekmedi.
- **Kart:** `rc::AgentRow::agent`, `rc::agentName`; bildirim başlığı "Codex seni bekliyor" / "Claude işini bitirdi",
  proje adı başlığın sağında küçük yazıyla, sebep alt satırda tam genişlikte (uzun sebepte proje "S…" diye
  kesiliyordu); oturum sayfasının başlığı "Kod ajanları", her satırın sağında ajan adı; boş sayfa "Açık ajan oturumu
  yok · Claude ya da Codex'te bir iş başlatınca görünür"; deneme `ajan test codex`.
- **Doğrulama:** hook Codex olay sırasıyla denendi (9 olay, her geçiş doğru, 48–86 ms, ata zincirinden doğru süreç);
  135/135 test (`test_agents`'e Codex satırı ve `agentName` eklendi); simülatörde 12 görüntü, denetim 0 bulgu.

---

## 29. Kablosuz güncelleme (WiFi)

Yeni yazılım karta USB kablosuz, köprünün WiFi bağlantısı (TCP 7777, anahtarlı) üzerinden gider:

```bash
python tools/spotify_bridge.py --guncelle                               # dist/Rumy.bin
python tools/spotify_bridge.py --guncelle dist/Rumy-guncelleyici.bin    # güncelleyicinin kendisi
```

### 29.1 Neden SD kart + güncelleyici

| Yöntem | Ölçüm | Sonuç |
|---|---|---|
| Klasik çift OTA (iki uygulama bölümü) | 2 × 2,15 MB > 3,01 MB (bölümler için kalan alan) | Sığmaz |
| WiFi'li küçük güncelleyici | 899 168 B | Bölüm küçültmesi uygulamayı sıkıştırır |
| **SD + ekranlı güncelleyici** | 414 336 B ölçüm, son hâli 421 968 B | **Seçildi** (448 KB `ota_1`) |

Yazılım WiFi'den gelirken ana uygulama çalışmaya devam eder ve dosyayı SD karta yazar; ana bölümü yazan,
yeniden başlayınca açılan küçük güncelleyicidir. Böylece ana bölüm hiçbir an kendi üzerine yazılmaz.

### 29.2 Akış

```
Bilgisayar (--guncelle)      Köprü                 Rumy (app0)                     Güncelleyici (ota_1)
  imajı denetle
  istek dosyası ───────────▶ @guncelle <bayt> <sha> ──▶ yer/SD/bölüm denetimi
                             ◀── @guncelle git
                             2048 B veri ───────────▶ SD'ye ekle, iki SHA-256
                             ◀── @guncelle ok <n>      (her parçada)
                                                       özetler tutuyor → .bin + .sha
                                                       otadata → ota_1
                             ◀── @guncelle tamam <elf8>
                                                       yeniden başla ──────────────▶ SD dosyasını yeniden doğrula
                                                                                     app0'ı sil ve yaz (3 deneme)
                                                       ◀────────────── yeniden başla otadata → ota_0, SD'yi temizle
  DOĞRULANDI ◀────────────── @merhaba ... <elf8> <güncelleyici elf8>
```

- **Bilgisayar** (`tools/fw_update.py`): imajın ESP32 imajı olduğunu (0xE9, çip kimliği 0, uygulama tanımı, "özet
  ekli" bayrağı), bölüme sığdığını ve sonundaki SHA-256'nın içerikle tuttuğunu denetler; bozuk dosya gönderilmez.
  Köprü çalışıyorsa isteği ona dosyayla iletir ve `guncelle_durum.txt`'den ilerlemeyi izler (köprü 45 sn ses
  vermezse vazgeçer); çalışmıyorsa `--pi/--wifi` adresine (ya da USB'ye) kendisi bağlanır. Rumy'de aynı yazılım
  varsa göndermez (`--zorla` ile gönderir). "Rumy güncelleniyor" yazısı geçen imaj güncelleyici sayılır.
- **Kart** (`src/media/fwupdate.*`): teklifte bölümün, güncelleyicinin, SD'nin ve yerin (imaj + 256 KB) varlığını
  denetler; aktarım sırasında `bridge::bulk` açıktır (HTTPS istekleri ertelenir). Her parça SD'deki
  `/rumy/guncelleme.part` dosyasına eklenir (`max_files = 1` olduğu için parça başına aç-yaz-kapat), dosyanın tamamının
  ve son 32 bayt hariç gövdesinin SHA-256'sı birlikte hesaplanır. İlk parçadan sonra imaj başlığı denetlenir
  (güncelleyici imajı ana bölüme gidemez). Sonda iki özet de tutarsa dosya `guncelleme.bin` olur, yanına
  `guncelleme.sha` (`<sha256> <bayt>`) yazılır, açılış bölümü güncelleyici yapılır ve 1,5 sn sonra yeniden başlanır.
  6 sn veri gelmezse aktarım bırakılır, yarım dosya silinir.
- **Güncelleyici** (`updater/src/main.cpp`): LovyanGFX ve kurulum ekranının yazı tipiyle (NVS'teki ekran çevirme
  ayarına uyar) ilerlemeyi gösterir; SD'deki dosyanın SHA-256'sını, gövde özetini ve başlığını **ana bölümü silmeden
  önce** yeniden doğrular, sonra `esp_ota_begin/write/end` ile `app0`'a yazar (üç deneme), açılış bölümünü
  `app0` yapar, SD'deki dosyaları siler. Dosya yoksa ya da bozuksa eski yazılımla açılır; ana bölüme yazamazsa
  "USB ile yükleyin" der.
- **Güncelleyicinin kendisi:** teklifin sonunda `guncelleyici` sözcüğü varsa Rumy SD'ye uğramadan imajı doğrudan
  `ota_1`'e yazar (`OTA_WITH_SEQUENTIAL_WRITES`, sektör sektör siler), `esp_ota_end` doğrular, yeniden başlar.

### 29.3 Ekranlar

Kart tarafı (`src/ui/update_ui.*`, LVGL, saniyede bir güncellenir, dokunmaları keser): "Güncelleme alınıyor" +
çubuk + "%41 · 0,9 / 2,1 MB"; sonra yeşil gözlerle "Güncelleme alındı · Yeniden başlıyor…"; hatada kırmızı gözlerle
"Güncelleme alınamadı" ve sebep (bağlantı koptu, dosya bozuk geldi, SD karta yazılamadı...) 6 sn. Güncelleyici
aynı yerleşimde "Rumy güncelleniyor" + "Doğrulanıyor / Yazılıyor · %37"; "Güncelleme tamam"; hatada "Güncelleme
yapılamadı" + sebep + "Eski yazılımla açılıyor" ya da "USB ile yükleyin". Simülatör senaryosu:
`RUMY_SIM_ONLY=guncelle python tools/ui_preview.py --film`. Görüntüler: `docs/preview_guncelleme/` (`01`–`05` kart,
simülatör; `g1`–`g5` güncelleyici, kurulum yazı tipinin VLW glifleriyle çizildi), hepsi bir arada
`docs/ui_guncelleme.png`.

### 29.4 Protokol ve tanılama

- `@guncelle <bayt> <sha256> [guncelleyici]` → `@guncelle git` | `@guncelle hata <neden>`; her 2048 baytta
  `@guncelle ok <n>`; sonda `@guncelle tamam <elf8>`. Nedenler: `bicim`, `guncelleyici yok`, `bolum`, `boyut`,
  `sd yok`, `sd dolu`, `mesgul`, `sd yazma`, `okuma`, `imaj`, `imaj ozeti`, `guncelleyici imaji`, `sha`,
  `zaman asimi`, `acilis bolumu`, `yazma`.
- `@merhaba <kapak> <kamera> <uygulama_elf8> <guncelleyici_elf8>`: ELF özetinin ilk 8 hanesi derlemeye özgüdür
  (Arduino'da uygulama tanımının sürüm/tarih alanları çekirdek kütüphanesinden gelir, ayırt etmez).
- `rapor`: `[guncelleme] bekliyor 0/0 B · calisan 659df252 (app0) · guncelleyici e7cb576d (448 KB) · basarili 0 ·
  hata 0 · son -`.
- Ortak ayrıştırıcılar `lib/rumy_core/src/fw_image.h` (kart ve güncelleyici aynı dosyayı kullanır), testleri
  `test/test_fwimage`.

### 29.5 Ölçümler (25 Ekim, kartta)

| Ölçüm | Değer |
|---|---|
| Ana yazılım aktarımı (2,15 MB, WiFi, SD'ye) | 67–69 sn, ~32 KB/sn |
| Komuttan doğrulamaya toplam (güncelleyici + yeniden bağlanma dahil) | ~95 sn |
| Güncelleyici aktarımı (422 KB, doğrudan flash'a) | 9 sn, ~48 KB/sn; toplam 16 sn |
| Boş heap (açılıştan sonra `rapor`) | 54,3 KB → 53,1 KB (statik +~1,2 KB) |
| Aktarım ortasında köprü öldürüldü (841 KB'ta) | Rumy 6 sn sonra "zaman asimi", yarım dosya silindi, eski yazılım çalışmaya devam etti |
| Aynı yazılım yeniden istendi | Gönderilmedi ("zaten bu yazılım var") |
| Bozuk imaj (bir bayt değiştirilmiş) | Bilgisayarda reddedildi |

---

## 30. Telefon uygulaması (iPhone)

### 30.1 Seçilen yol

| | Ana ekrana eklenen web uygulaması | Yerel iOS uygulaması |
|---|---|---|
| Gereken | Safari → Paylaş → Ana Ekrana Ekle | Mac + Xcode |
| Apple hesabı | Gerekmez | Ücretsizde 7 günde bir yeniden yükleme; kalıcı için yıllık geliştirici hesabı |
| Güncelleme | Rumy'nin yazılımıyla birlikte (kablosuz güncelleme) | Ayrı derleme ve yükleme |

Bilgisayar Windows olduğu için ilk yol seçildi. Sayfa Rumy'nin flash'ında durur, Rumy güncellenince uygulama da
güncellenir.

### 30.2 Ekranlar

- **Eşleştirme:** gözler, "Rumy'ye bağlan", 4 kutulu kod alanı (`autocomplete=one-time-code`, sayısal klavye).
- **Ana sayfa:** durum kartı (çevrimiçi / uyuyor / ulaşılamıyor, adres, sürüm, çalışma süresi) ve **Şimdi uyut /
  Uyandır** düğmesi; altında iOS Ayarlar biçiminde liste: Genel, Ekran, Futbol, Işık, 3D yazıcı, Önemli günler,
  Bakım. Ana sayfa açıkken durum 15 sn'de bir yenilenir.
- Değişiklikler anında kaydedilir (500 ms toplanıp tek `POST /api/settings`), altta "✓ Kaydedildi".
- **Futbol:** Süper Lig'in 18 takımı ESPN numaralarıyla sayfada (27 Ekim 2026 puan tablosundan); seçili takım
  listede yoksa "Takım N" olarak eklenir.
- Ulaşılamazsa liste soluklaşır, "Tekrar dene" çıkar; son görülme saati telefonda tutulur.

### 30.3 Rumy tarafı

- `src/ui/scr_phone.cpp`: QR (`lv_qrcode`, `LV_USE_QRCODE` + `LV_USE_CANVAS` açıldı), üç adım, büyük kod,
  `rumy.local · IP`, eşli telefon sayısı ve "Unut". QR, mDNS açıksa `http://rumy.local/`, değilse IP adresidir.
- Ayarlar ekranındaki "Web arayüzü" anahtarı yerine "Telefon" satırı (eşli sayısı).
- `wiz.cpp`: uyarı/kutlama başlamadan ayarlara bakar; `wonly` açıkken kapalı ampule dokunmaz (`wiz ... atlandi
  (ayar)` günlüğü). `match_ui.cpp`: `leagueWanted()` Süper Lig / ŞL gol bildirimlerini süzer.
- `rapor` çıktısında `[web]` satırı: açık/kapalı, mDNS, eşli telefon, istek, eşleşme ve ret sayıları.
- Testler: `test/test_phone` (uyku bitişi, belirteç biçimi ve karşılaştırma, kod ayrıştırma; 7 test, toplam 151).

### 30.4 Bilgisayarda

- Pencere ≥ 900 px genişse sayfa iki sütuna geçer (`#shell` ızgarası, `wide` medya sorgusu): solda durum kartı,
  "Şimdi uyut" ve bölüm listesi hep görünür (seçili bölüm vurgulanır), sağda bölümün ayarları. Geri düğmeleri
  gizlenir (Günler → gün düzenleme hariç); başlıklar büyük ve sola yaslı; fareyle üzerine gelme vurgusu.
  Ana sayfa yenilemesi geniş düzende her bölümde sürer. Dar ekranda telefon düzeni değişmedi.
- Telefona özgü metinler ("Telefonun aynı WiFi'de olmalı" vb.) "bu cihaz" diye genelleştirildi.
- `tools/pc_kisayol.py --kur`: `web/icon.png`'den `tools/assets/rumy.ico` üretir, masaüstüne ve Başlat menüsüne
  `Rumy.lnk` koyar: `msedge.exe --app=http://rumy.local/ --window-size=1180,800` (adres çubuğu ve sekme yok).
  Başlat menüsü `AppData` altında olduğu için kurulum Claude'un kapsayıcısı dışında, tek seferlik bir zamanlanmış
  görevle çalıştırıldı. `--kaldir` ikisini siler. Bilgisayar da eşli 4 cihazdan birini kullanır.

### 30.5 Görünüm (geçici ifade seçimi)

- `src/rumy/look.*`: görevler arası paylaşılan istek (ifade, aksesuar, efekt, sıra no) ve gösterilen görünüm.
  Web (`POST /api/look {"mood","acc","fx"}` ya da `{"auto":true}`) ve seri `gorunum ...` isteği yazar.
- `scr_home::updateMood` (her saniye): yeni istek gelince o anki otomatik görünümün imzasını saklar; otomatik
  görünüm bu imzadan farklılaşınca (hava, müzik, maç tepkisi, gece, deprem, ajan, göz uykusu) istek bırakılır.
  Gösterilen görünüm `/api/state` içinde `look` olarak ve `rapor`'da `[gorunum]` satırında görünür.
- Uygulamada ifade resimleri `web/yuzler.png` (13 × 80×56, ~3 KB), `python tools/ui_preview.py --yuzler` ile
  simülatörden üretilir, `build_web.py` onu `web_faces.h`'ye gömer, `/yuzler.png`'den sunulur.
- Kartta denendi: `gorunum 1 2 13` → mutlu + parti şapkası + konfeti; `hava kar` ile hâl değişince otomatiğe döndü.
