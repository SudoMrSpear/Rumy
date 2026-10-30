# Değişiklik günlüğü

Sürüm numarası `platformio.ini` içindeki `[rumy] version` değeridir ve Rumy'nin Tanılama ekranında, `rapor`
çıktısında ve uygulamanın Bakım bölümünde görünür.

## Yayınlanmamış

- **English / İngilizce:** cihaz arayüzü, telefon uygulaması, ilk kurulum portalı ve kablosuz güncelleyici artık İngilizce de çalışır. Yeni kurulumlarda varsayılan dil İngilizce; Ayarlar → Language / Dil ya da uygulamada Genel → Language / Dil ile Türkçeye geçilir (Rumy yeniden başlar). Ayar NVS'te `dil` anahtarında durur.
- **Web kurulum:** <https://sudomrspear.github.io/Rumy/> adresinde Chrome/Edge'den tek tıkla kurulum (ESP Web Tools);
  ILI9341 ve ST7789 seçilir. Her yeni sürümde `web_kurulum.yml` sürümün tam imajlarıyla sayfayı yeniden yayınlar.
- **İngilizce belgeler:** `README.en.md` (tam çeviri), `Rumy_Project_Document.md`, `CHANGELOG.en.md`,
  `THIRD_PARTY_NOTICES.en.md`, `docs/icon_ram.en.md`, `sd_card/README.txt`; `CONTRIBUTING.md`, `SECURITY.md` ve hata/öneri
  şablonları iki dilli.

## 1.3.0 — kaynağı açık ilk sürüm

- **Takım seçimi:** Süper Lig ve beş büyük ligden (Premier Lig, LaLiga, Serie A, Bundesliga, Ligue 1) istenen takım
  uygulamanın Futbol bölümünden seçilir; lig sekmeleri ve renkli rozetli takım listesi (`tools/make_teams.py` ile
  ESPN'den üretilir). Varsayılan olarak takım seçili değildir.
- **Maç takibi anahtarı:** futbol tek düğmeyle tamamen kapatılır; kapalıyken maç sorgulanmaz, bildirim ve maç kartı
  çıkmaz. Seri komutlar `futbol ac|kapat`, `takim <no> [renk1 renk2]`.
- **Takım renkleri:** gol ekranı, maç kartı, maç ekranındaki vurgular ve ampulün gol kutlaması takımın iki renginden
  türetilir (`rc::teamPalette`); renk verilmemişse önceki lacivert-sarı görünüm.
- **Köprü:** Rumy seçili takımı `@futbol` satırıyla bildirir; hızlı gol izleyicisi ve oyuncu fotoğrafı aracı artık
  bu takıma göre çalışır. TheSportsDB isteği reddederse araç Wikimedia Commons ve Vikipedi'ye geçer.
- **Lig rozetleri:** depoda kulüp ya da turnuva logosu yoktur; kartlarda kısaltmalı sade rozetler görünür. Kendi
  logolarını `tools/assets/lig_logolari/` klasörüne koyan kullanıcı onları kullanabilir.
- Varsayılan konum İstanbul; örnek veriler ve önizleme görüntüleri kişisel bilgilerden arındırıldı.
- PolyForm Noncommercial 1.0.0 lisansı (ticari kullanım için izin gerekir), üçüncü taraf lisans notları, katkı ve güvenlik belgeleri, GitHub Actions derlemesi.

## 1.2.0 sonrası geliştirmeler (Ekim 2026)

Bu dönemde sürüm numarası 1.2.0'da kaldı; değişiklikler 1.3.0'a dahildir.

- **Raspberry Pi sürümü (deneysel):** Pi Zero W ve Waveshare 3,5" SPI ekranda 480×320 arayüz, DRM ile doğrudan çizim,
  servis bekçisi, sağlık kaydı, kurulum betikleri.
- **Hava durumu ifadeleri:** güneşli, sıcak, bunaltıcı, kapalı, sisli, çisenti, yağmurlu, karlı, fırtınalı, rüzgarlı,
  soğuk ve açık gece için ayrı Rumy hâlleri.
- **Ana ekran:** "şimdi çalıyor" şeridi saatin yanındaki sütunun başında, sabit düzen, ortalı tarih.
- **Işık:** günlük kullanım için sekiz ampul sahnesi.
- **WiFi köprüsü:** köprü kartla anahtarlı TCP bağlantısıyla da konuşur (kablo yalnızca güç için).
- **Spotify:** Spotify'ın kendi ses seviyesi, basılı tutunca hızlanan ses, çalma listeleri (Web API, PKCE).
- **Maç takibi (ESPN):** canlı skor kartı, gol/kart/değişiklik bildirimleri, maç ekranı; animasyonlu gol ekranı (dört
  gösteri), golde ampul kutlaması, gol iptali bildirimi, gol ekranında oyuncu fotoğrafı.
- **Lig günü ve Futbol sayfası:** Süper Lig ve Şampiyonlar Ligi'nin bütün maçlarının golleri; yedi ligin maç günü ve
  puan durumu.
- **Hızlı gol bildirimi:** LiveScore'dan ~20–45 sn'de.
- **Simülatör:** uç durum senaryoları ve otomatik ekran denetimi (üst üste binen, kesilen yazı, fontta olmayan harf).
- **Köprü bekçisi:** köprü kapanırsa kendiliğinden yeniden açılır; Windows açılışında başlar.
- **Kod ajanı ışığı:** Claude Code ve Codex oturumlarının durumu ana ekranda yıldızla, beklerken bildirim ve ampul
  uyarısı (Claw Light'tan esinlenildi).
- **Kablosuz güncelleme:** imaj köprüden SD karta, oradan küçük güncelleyiciyle ana bölüme.
- **Telefon ve bilgisayar uygulaması:** Rumy'nin sunduğu web uygulaması (ana ekrana eklenir), 4 haneli eşleştirme,
  "Şimdi uyut", Görünüm bölümü (geçici ifade/aksesuar/efekt), bilgisayarda iki sütunlu düzen ve kısayol.
- **Deprem ve bayram verisi:** kartın belleği HTTPS'e yetmediği için AFAD ve Nager köprü üzerinden; köprü yokken
  Kandilli'nin şifresiz listesi.

## 1.2.0

- Spotify ekranı ve bilgisayar köprüsü (şarkı, kapak, kontroller), flash'ta kapak önbelleği.
- Kulaklıklı Rumy ve ses çıkışından ritim takibi.
- LVGL havuzu 36 → 24 KB (ekranlara göre ölçülerek).
- Ana ekranda "şimdi çalıyor" şeridi.
- Creality K2 yazıcı ekranı ve köprü üzerinden kamera.
- Hava verisi tek istekte, Nager günde bir; yalnızca IPv4 kabul eden yazıcı adresi.

## 1.1.0

- Performans: ILI9341 donanım kaydırmasıyla sayfa geçişleri, flash QIO 80 MHz, ikonlar flash'ta, `-O2`.
- Doygun neon renkli yeni ana ekran.
- Göz uykusu; `bench`, `perf`, `profil`, `spitest`, `kayma` araçları.

## 1.0.2

- Karta ilk kurulum ve düzeltmeler: HTTPS için bellek açıldı (SD `max_files` 3 → 1, LVGL havuzu ve ağ yığını
  küçültüldü), dokunmatik yönü, saat eşitlenmeden çekilen verinin yanlış tarihli görünmesi, simülatör derlemesi.

## 1.0.0

- İlk sürüm: ekranlar, Rumy'nin yüzü ve ifadeleri, hava/deprem/bayram kaynakları, WiZ ampul, web arayüzü, ilk kurulum
  portalı, SD önbelleği, birim testleri, kartsız önizleme.
