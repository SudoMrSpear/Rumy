# Rumy

[English](README.en.md) · **Türkçe** · [Tarayıcıdan kur](https://sudomrspear.github.io/Rumy/)

ESP32-2432S028R ("Cheap Yellow Display", CYD) için masaüstü arkadaşı. Siyah ekranda parlayan iki camgöbeği gözden oluşan
Rumy; saati, hava durumunu, Marmara depremlerini, bayramları ve senin önemli günlerini gösterir, WiZ ampulünü yönetir.

![Rumy](docs/tanitim.png)

> **English summary.** Rumy is a desk companion for the ESP32-2432S028R "Cheap Yellow Display" (CYD, 320×240): two
> glowing eyes that react to weather, music, football and time of day, a big clock, Open-Meteo weather, earthquakes in
> Turkey (AFAD / Kandilli), Turkish holidays and your own special days, WiZ bulb control, live football for any team in
> the Süper Lig or the top five European leagues (ESPN; the goal screen and the bulb take your team's colours),
> Spotify "now playing" through a small PC bridge, Creality K2 print status and a Claude Code / Codex agent light.
> All settings live in a phone/PC web app served by the device itself. The interface is in English by default with
> Turkish selectable; the full English documentation is in [README.en.md](README.en.md). Flash a release image (see
> *USB ile kurulum*) or build with PlatformIO: `pio run -e cyd`. Source-available
> under the PolyForm Noncommercial License 1.0.0: free for personal, hobby, educational and research use, including
> modifications; commercial use (selling it or putting it into a paid product or service) needs separate permission —
> open an issue or contact [@SudoMrSpear](https://github.com/SudoMrSpear). Third-party notices are in
> `THIRD_PARTY_NOTICES.md`.

- **Ana ekran:** siyah zemin üzerinde doygun neon renkler. Rumy'nin gözleri, yeşil konum, ampul düğmesi, iki noktası
  yanıp sönen büyük saat (saat yeşil, dakika beyaz), sağında sola hizalı bir sütun: üstte selam ("İyi geceler
  Deniz"), altında büyük hava ikonu ile sıcaklığa göre renklenen derece ve onun altında günün en yüksek/en düşük
  derecesi; sarı gün ve yeşil tarih,
  sıcaklık / nem / hava kalitesi çubukları ve 5 sn'de bir değişen bilgi kartı (rüzgâr adı, 6 saatlik yağış ve
  "Şemsiye al", gün doğumu/batımı, deniz suyu, hava kalitesi, önemli gün, son deprem).
- **Spotify ekranı:** USB ya da WiFi ile bağlı bilgisayarda çalan şarkının kapağı, adı, ilerlemesi (sürükleyerek
  şarkıda ileri/geri gidilir) ve dokunmatik kontrolleri; kapağın köşesindeki düğmeden Spotify çalma listelerin
  (kapaklarıyla) açılır, dokunulan liste bilgisayardaki Spotify'da başlar (Premium, bir kez giriş);
  müzik çalarken ana ekrandaki Rumy kulaklık takıp ritme göre kafa sallar; saatin sağındaki sütunun en üstünde
  dönen bir plak (ortası mor) ve sığmazsa kayan şarkı adı görünür. Müzik yokken aynı yerde soluk plak ve "Çalan
  yok" durur; ana ekranın düzeni değişmez.
- **3D yazıcı (Creality K2):** baskının durumu, hata, bitiş saati ve bitişi gösteren çubuk; hata olunca sayfa
  kendiliğinden açılır (gece de). Bilgisayar köprüsü çalışıyorsa yazıcının kamerası da görünür.
- **Maç takibi:** Süper Lig ve beş büyük ligden (Premier Lig, LaLiga, Serie A, Bundesliga, Ligue 1) istediğin takım;
  uygulamadan seçilir ya da tek düğmeyle tamamen kapatılır. Maç günü alttaki kartta başlama saati, maç sırasında canlı
  skor ve dakika; golde takımının renklerinde tam ekran "GOOOL!", rakip golü, sarı/kırmızı kart ve oyuncu değişikliği
  ekranın üstünde 6 sn. Karta dokununca maçın bütün olayları. Veri ESPN'den gelir, bilgisayar gerekmez; gece modunda
  bildirim çıkmaz.
- **Kod ajanı ışığı (ClawLight uyarlaması):** bilgisayardaki Claude Code ve Codex oturumlarının durumu ana ekranda
  küçük bir yıldızla görünür: çalışırken yeşil döner, seni beklerken kırmızı nabız atar, iş bitince sarı yanar.
  Bir oturum izin ya da cevap beklemeye başlayınca üstte "Claude seni bekliyor" (ya da "Codex seni bekliyor")
  kutusu çıkar ve WiZ ampul iki kez kırmızı yanıp söner; 2 dakikadan uzun süren bir iş bitince "Claude işini
  bitirdi". Yıldıza dokununca oturum listesi.
- **Göz uykusu:** varsayılan olarak kapalı; Rumy yalnızca gece modunda uyur. Ayarlar → Ekran'dan bir süre seçilirse
  o kadar dokunulmayınca da uyur, dokununca mahmur mahmur uyanır.
- **Hava ekranı:** duruma göre renk değiştiren gradyan, 64 px ikon, sonraki 1 saatin yağış grafiği, yatay kaydırılan
  24 saatlik şerit (gün doğumu/batımı dahil), 7 günlük min–maks barları ve 10 kutuluk detay ızgarası.
- **Işık ekranı:** solda ikon, sağda değer olan renk tonu, parlaklık (lambanın renginde) ve beyaz ışık sıcaklığı
  kaydırıcıları; o an kullanılmayan kipin topuzu içi boş halkadır. Günlük kullanım için 8 sahne (Sabah, Çalışma,
  Okuma, Yemek, Akşam, Film, Mum, Gece); her düğmede lambanın o sahnedeki renginde ikon, seçili olan kendi renginde
  çerçevelenir; başlıktaki ampul lambanın rengini alır. Ana ekrandaki kutuya dokunmak aç/kapa yapar, uzun basmak ışık
  ekranını açar.
- **Önemli günler:** milli bayramlar ve anma günleri, Ramazan ve Kurban bayramları (Nager.Date + Diyanet tablosu),
  telefon uygulamasından eklenen doğum günleri ve yıl dönümleri. Günün kendisinde tam ekran kutlama kartı.
- **Gece modu:** ekran kısılır, Rumy uyur, küçük saat kalır; dokununca 30 sn uyanır.
- **Telefon uygulaması:** iPhone'un ana ekranına eklenen Rumy uygulaması; bütün ayarlar, "Şimdi uyut" düğmesi,
  takım ve ampul seçenekleri, önemli günler (bir kez eşleştirilir).
- **Dil:** yeni kurulumlarda arayüz İngilizce açılır; Ayarlar → Language / Dil ya da uygulamada Genel → Language / Dil
  ile Türkçeye geçilir (Rumy yeniden başlar).

## İçindekiler

1. [Kuruluma hazırlık](#1-kuruluma-hazırlık)
2. [SD kartı hazırlama](#2-sd-kartı-hazırlama)
3. [Launcher ile kurulum](#3-launcher-ile-kurulum-önerilen)
4. [USB ile kurulum](#4-usb-ile-kurulum)
5. [İlk açılış](#5-ilk-açılış)
6. [Kullanım](#6-kullanım)
7. [Telefon uygulaması](#7-telefon-uygulaması)
8. [RAM ve flash bütçesi](#8-ram-ve-flash-bütçesi)
9. [Geliştirme](#9-geliştirme)
10. [Dayanıklılık testi](#10-dayanıklılık-testi)
11. [Sorun giderme](#11-sorun-giderme)
12. [Lisanslar](#12-lisanslar)
13. [Raspberry Pi sürümü (deneysel)](#13-raspberry-pi-sürümü-deneysel)
14. [Katkı, güvenlik ve yasal not](#14-katkı-güvenlik-ve-yasal-not)

## 1. Kuruluma hazırlık

| Gereken | Not |
|---|---|
| ESP32-2432S028R (CYD) | 2,8" 320×240, dirençli dokunmatik, SD yuvası. PSRAM gerekmez. |
| microSD kart (isteğe bağlı) | FAT32. Kişisel günler, bayram tarihleri ve son verinin yedeği için. İkonlar flash'ta olduğundan kartsız da tam çalışır; yalnızca kişisel günler kaydedilemez. |
| 2,4 GHz WiFi | ESP32 5 GHz ağlara bağlanamaz. |
| WiZ ampul (isteğe bağlı) | Aynı ağda olmalı. "Misafir ağı" ya da "AP izolasyonu" açık ağlarda bulunamaz. |
| Kutu (isteğe bağlı) | Masada dik durması için knkeeb'in "CYD Desk Stand 2.8"" modeli (MakerWorld, model 2880776). Baskı dosyası bu depoda yok; modelin kendi sayfasından ve lisansıyla indirilir. |

Panel tipi: CYD'lerin çoğu **ILI9341** kullanır ve varsayılan budur. Renkler ters ya da görüntü bozuk çıkarsa
**ST7789** sürümünü kullan.

Hazır imajlar GitHub'daki **Releases** sayfasındadır: her panel için Launcher dosyası
(`Rumy-<sürüm>-ILI9341-launcher.bin`, `…-ST7789-launcher.bin`) ve USB ile yazılan tam imaj
(`Rumy-<sürüm>-ILI9341-tam-0x0.bin`, `…-ST7789-tam-0x0.bin`). Kaynaktan derlersen aynı dosyalar `dist/` klasörüne
düşer (`Rumy.bin`, `Rumy-full-0x0.bin`; bkz. [Geliştirme](#9-geliştirme)).

## 2. SD kartı hazırlama

1. Kartı FAT32 biçimlendir (kartta başka dosyalar, örneğin Bruce'unkiler, kalabilir). Birden çok bölümlü bir kart
   da olur: Rumy ilk FAT32 bölümünü kullanır (Raspberry Pi görüntüsü yazılı bir kartla denendi).
2. Launcher kullanacaksan Launcher dosyasını (`Rumy-<sürüm>-ILI9341-launcher.bin` ya da derlediysen `dist/Rumy.bin`)
   `Rumy.bin` adıyla kartın köküne kopyala.

Kartta hazırlaman gereken başka bir şey yok. `/rumy/cache` (son hava/deprem verisi), `/rumy/holidays` (bayram
tarihleri) ve `/rumy/events.json` (kişisel günler, telefon uygulamasından eklenir) dosyalarını Rumy kendisi oluşturur.
Hava ikonları v1.1.0'dan beri flash'a gömülüdür; `sd_card/rumy/img` klasörü yalnızca ikonların kaynağıdır.

## 3. Launcher ile kurulum (önerilen)

[Launcher](https://github.com/bmorcelli/Launcher), kartta birden fazla uygulamayı yan yana tutmanı sağlar.
`Rumy.bin` sadece uygulamayı içerir; Launcher onu kendi açtığı bölüme yükler, ayarlar NVS'te kalır.

1. Kartta Launcher yoksa: bilgisayarda Chrome/Edge ile
   [Launcher web flasher](https://bmorcelli.github.io/Launcher/webflasher.html) sayfasını aç, cihaz olarak
   **CYD-2432S028** seç (iki USB portlu, USB-C + micro-USB kartlarda **CYD-2-USB**), kartı USB'den bağla ve yükle.
   Bu adım karttaki mevcut yazılımı (ör. Bruce) siler.
2. SD kartı (kökünde `Rumy.bin` ile) karta tak, kartı aç.
3. Launcher menüsünde **SD** → `Rumy.bin` → **Install** seç. Kurulum bitince Rumy açılır.

## 4. USB ile kurulum

Launcher istemiyorsan tam imajı doğrudan yazabilirsin. **Karttaki her şey (Bruce, Launcher) silinir.** Önce
mevcut yazılımın tam yedeğini al; geri dönmek istersen aynı dosyayı `0x0` adresine yazman yeterli:

```bash
python -m esptool --chip esp32 --baud 460800 read_flash 0 0x400000 yedek/eski_yazilim_4MB.bin
```

```bash
python -m esptool --chip esp32 --baud 460800 write_flash --erase-all 0x0 Rumy-1.3.0-ILI9341-tam-0x0.bin
```

Kurulu bir Python yoksa en kolayı **[web kurulum sayfası](https://sudomrspear.github.io/Rumy/)**: bilgisayarda
Chrome ya da Edge ile aç, panel tipini seç, kartı USB'den bağla ve **Kur**'a bas; açılan pencerede **Erase device**
seçeneğini işaretle. Sayfa son sürümün tam imajını yazar (`.github/workflows/web_kurulum.yml` her yeni sürümde
yeniden yayınlar). Aynı dosyayı Espressif'in tarayıcıdan yükleyicisiyle de yazabilirsin (adres `0x0`).

CYD'deki CH340 köprüsü kartı kendisi yükleme moduna alır, BOOT tuşuna basmak gerekmez. Bağlanamazsa BOOT'a basılı
tut, RST'ye bas-bırak, BOOT'u bırak: kart yeniden başlatılana kadar yükleme modunda bekler. 921600 baud bazı
kablolarla kopuyor; 460800 güvenilir çalıştı. `yedek/` klasörü `.gitignore`'dadır, çünkü eski yazılımın NVS'inde
WiFi şifreleri olabilir.

**Güncelleme (ayarlar korunur):** yalnızca uygulamayı yaz; WiFi, ad ve konum NVS'te kalır.

```bash
python -m esptool --chip esp32 --baud 460800 write_flash 0x10000 dist/Rumy.bin
```

Ya da kaynak koddan (bkz. [Geliştirme](#9-geliştirme)):

```bash
pio run -e cyd -t upload
```

### Kablosuz güncelleme (WiFi)

Rumy köprüye WiFi'den bağlıyken (bkz. [Spotify ekranı](#spotify-ekranı), `--pi/--wifi`) yeni yazılımı kablosuz
gönderebilirsin; kablo yalnızca güç için kalır:

```bash
python tools/spotify_bridge.py --guncelle
```

Varsayılan dosya `dist/Rumy.bin`'dir (ST7789'lu kartta `dist/Rumy-st7789.bin`'i ver). Köprü çalışıyorsa istek ona
iletilir, çalışmıyorsa komut kendisi bağlanır. Rumy dosyayı SD karta yazarken ekranda "Güncelleme alınıyor" ve bir
çubuk görünür (2,1 MB ~68 sn); bitince küçük güncelleyiciyle yeniden başlar, dosyayı yeniden doğrulayıp kurar ve
yeni sürümle açılır. Komut, Rumy yeniden bağlanıp yeni sürümü bildirince "TAMAM" yazar (toplam ~95 sn). Bağlantı
koparsa ya da dosya bozuk gelirse Rumy eski yazılımla devam eder. Rumy'de aynı yazılım varsa göndermez (`--zorla`).

- SD kart takılı olmalı (imaj + 256 KB boş yer).
- Güncelleyicinin kendisi de aynı yolla yenilenir: `--guncelle dist/Rumy-guncelleyici.bin` (SD'ye uğramaz, ~16 sn).
- **İlk kurulum USB ile yapılır:** kablosuz güncelleme bu depodaki bölüm tablosunu (ana bölüm 2,5 MB + güncelleyici
  448 KB) ister; tam imaj ya da `pio run -e cyd -t upload` bunu yazar. Launcher kurulumunda kablosuz güncelleme yoktur.
- Rumy'nin `rapor` çıktısında `[guncelleme]` satırı çalışan sürümü, güncelleyiciyi ve son sonucu gösterir.

## 5. İlk açılış

WiFi bilgisi yoksa Rumy LVGL'yi hiç başlatmadan sade bir kurulum ekranı gösterir:

1. Telefonla **Rumy-Kurulum** ağına bağlan (ekrandaki QR kodu okutabilirsin).
2. Açılan sayfada **Configure WiFi**'ye bas, ev ağını seç ve şifreni yaz.
3. **Adın** ve **Şehir (il)** alanlarını doldur (il adı yazarken öneri çıkar; "istanbul" ya da plaka "34" da olur).
4. **Save**. Rumy bağlanıp yeniden başlar. Sayfa kendiliğinden açılmazsa tarayıcıda `http://192.168.4.1`.

WiFi şifresi ve tüm ayarlar kodda değil, cihazın NVS belleğinde durur.

## 6. Kullanım

| Hareket | Sonuç |
|---|---|
| Sola / sağa kaydır | Ana ⇄ Hava ⇄ Işık ⇄ Ayarlar |
| Alt ekranlarda sağa kaydır | Geri |
| Hava bloğuna (ikon, derece) dokun | Hava ekranı |
| Ana ekranda sağa kaydır | Spotify ekranı (köprü çalışıyorsa; çalışmıyorsa yazıcı sayfası) |
| Spotify ekranında sağa kaydır | 3D yazıcı sayfası (yazıcı ayarlıysa) |
| Baskı kartına dokun | 3D yazıcı sayfası |
| Ana ekrandaki plaklı şarkı şeridine dokun | Spotify ekranı |
| Şehir adının altındaki Claude yıldızına dokun | Claude oturumları sayfası (yıldız yalnızca açık oturum varken görünür) |
| Sağ üstteki ampul düğmesine dokun / uzun bas | Aç–kapa / Işık ekranı |
| Bilgi kartına dokun | Sonraki kart |
| Rumy'ye dokun | Dokunduğun yere bakar ve göz kırpar |
| 60 sn dokunmama | Ana ekrana dönüş (ayarlanabilir) |
| Dokunmama (göz uykusu açıksa) | Rumy uyur, dokununca uyanır (Ayarlar → Ekran → Göz uykusu; varsayılan "Kapalı") |
| Spotify ekranında ilerleme çubuğunu sürükle / çubuğa dokun | Şarkı o saniyeye atlar |

Ampul düğmesinin çerçevesi durumu gösterir: sarı = açık, gri = kapalı, kırmızı = ampule ulaşılamıyor.

**Rumy'nin ifadeleri:** normal (ışık açıkken de; gözler ampulün rengini hafifçe alır), uykulu (uyanırken kısa bir an), uyuyor (gece), üzgün (bağlantı
yok), şaşkın (M4+ deprem, 30 sn; uyuyorsa da uyanır), yukarı bakıyor + şemsiye (yağmur yaklaşıyor), kutlama +
bayrak/parti şapkası + konfeti (önemli gün), sakin + bayrak (10 Kasım), müzik dinliyor + kulaklık (bilgisayarda müzik
çalarken). Işık açıkken gözlerin rengi ampulün rengine hafifçe
kayar.

**Hava durumuna göre ifadeler** (1 Ekim; yukarıdakilerin hiçbiri yokken):

| Hava | Rumy |
|---|---|
| Güneşli | Mutlu, pembe yanaklar, zıplama |
| Güneşli ve sıcak (hissedilen ≥27 °C ya da UV ≥6) | Güneş gözlüğü, camlarda parıltı |
| Bunaltıcı sıcak (hissedilen ≥33 °C) | Uyukluyor, ter damlası, yükselen sıcak hava dalgaları |
| Kapalı | Sakin; yanında süzülen buluta bakıyor |
| Sisli | Gözleri kısık, sis bantları arasında bakınıyor |
| Çisenti / yağmurlu | Şemsiye, damlalar ve yerde sıçramalar; yağmurda üzgün ve gözyaşı |
| Karlı | Mutlu, pembe yanaklar, kar taneleri |
| Fırtınalı | Korkmuş; şimşek çakınca yüz şeridi aydınlanır, gözler irkilir, titrer |
| Rüzgarlı (rüzgar ≥35 ya da hamle ≥50 km/sa) | Gözleri kısık, rüzgara yaslanıyor, rüzgar çizgileri, uçan yaprak |
| Soğuk (hissedilen ≤2 °C) | `>  <` sıkılı gözler, buz mavisi, titreme, nefes buharı |
| Açık gece | Sakin, hilal ay ve parıldayan yıldızlar |

Seri komutla denemek için: `hava gunesli|sicak|bunaltici|kapali|sisli|cisenti|yagmur|kar|firtina|ruzgar|soguk|gece`
(2 dakika gösterir), `hava kapat` gerçek havaya döner. Uyurken (göz uykusu ve gece ekranı) sağ gözün yanından küçük "z"ler doğup büyüyerek, hafifçe salınarak
yukarı süzülür ve söner; bu sırada yalnızca z'lerin küçük alanı 20 fps'te yeniden çizilir. Gece modu varsayılan
olarak 02:00–07:00 arasıdır (Ayarlar → Gece modu ya da seri porttan `gece 02:00 07:00`).

### Spotify ekranı

Rumy USB ya da WiFi ile bilgisayardaki köprüye bağlıyken bilgisayarda çalan şarkıyı gösterir: 144×144 kapak, şarkı
adı, sanatçı, albüm, ilerleme çubuğu ve dokunmatik kontroller (ses −, önceki, oynat/duraklat, sonraki, ses +).
Ses tuşları Windows'un ana sesini değil, yalnızca Spotify'ın sesini %5 değiştirir (Windows ses karıştırıcısındaki
Spotify oturumu; Spotify'ın kendi ses çubuğuyla aynı değer, `pycaw` gerekir, yoksa Windows'un ses tuşlarına döner).
Tuşa basınca ya da ses başka yerden değişince ilerleme çubuğu 2 sn "Spotify sesi %60" çubuğuna dönüşür. Tuşa
dokununca ses hemen bir adım değişir; basılı tutunca 0,4 sn sonra her 0,15 sn'de bir adım daha (0'dan 100'e ~3 sn).
Basılı tutarken bilgisayardan geç gelen ses yanıtları çubuğu geri oynatmaz; son yanıt gelince gerçek değer gösterilir.
İlerleme çubuğunda beyaz bir tutamak vardır: sürüklerken tutamak büyür ve gidilecek süre pembe yazılır, bırakınca
Spotify o saniyeye atlar (çubuğa tek dokunuş da olur; çubuk üstünde kaydırmak sayfa değiştirmez). Vurgu rengi kapaktan
seçilir. Müzik başlayınca Rumy ana ekrandaysa kendiliğinden bu ekrana geçer (şarkı değişimlerinde geçmez); 1 dk
duraklatılmış kalırsa ya da köprü kapanırsa ana ekrana döner. Ana ekrandan sağa kaydırarak açılır, sola kaydırarak
kapanır.

**Ana ekranda "şimdi çalıyor":** müzik varken saatin sağındaki sütunun en üstünde küçük bir şerit belirir: solda
gerçek plak hızında (33⅓ devir/dk) dönen, ortası mor bir plak, sağında küçük yazıyla şarkının adı. Ad sığıyorsa
sabit durur; sığmıyorsa 1,5 sn bekleyip saniyede 30 px kayar, bir tur dönünce yine bekler (kenarlarda yumuşakça
kaybolur). Duraklatınca plak durur ve yazı grileşir; 1 dk duraklatılmış kalırsa ya da köprü kapanırsa yerinde soluk
plak ve "Çalan yok" kalır. Selam ve hava her zaman bu şeridin altındadır, şarkı başlayıp bitince hiçbir şey yer
değiştirmez. Tarih saatin altında ortalanır; hava ikonu tarih satırına uzandığı için sığmazsa önce gün adı kısalır
("Pzt 28 Eylül"), sonra ay kısalır ("Cum 28 Tem"). Şeride dokununca Spotify ekranı açılır. Bütün gün/ay
durumlarının görüntüsü: `python tools/ui_preview.py --tarih` → `docs/ui_tarih.png`.

**Rumy de dinler:** müzik çalarken ana ekrandaki Rumy neon pembe bir kulaklık takar, başı biraz aşağı iner, gözleri
keyifle kısılır ("^ ^"), kafasını ritme göre sallar ve sağ kulaklıktan albüm renginde notalar (♪) süzülür. Köprü `numpy` ve `pyaudiowpatch`
kuruluysa bütün ses çıkışlarını dinler, o an sesin aktığı cihazı kendisi seçer (varsayılan cihaz olmasa bile, ör.
Bluetooth kulaklık), vuruşları bulur ve tempoyu Rumy'ye gönderir; kafa sallama vuruşa oturur (sentetik testte tempo
±%1, vuruş zamanı ±20 ms). Bluetooth kulaklıkta ses ~150–200 ms geç duyulduğu için köprü o cihazlarda sallamayı
kendiliğinden 200 ms geciktirir (kablolu: 50 ms); `--beat-offset <ms>` ile elle ayarlanır. Paketler yoksa Rumy kendi
yavaş ritmiyle sallanır. Müzik çalarken Rumy göz uykusuna dalmaz; gece modunda kulaklığı takılı uyur.

**Çalma listeleri:** kapağın sağ alt köşesindeki liste düğmesi "Çalma listelerim" ekranını açar: her satırda listenin
kapağı, adı ve şarkı sayısı; çalan liste yeşil ve vurgulu, en üstte Beğenilen Şarkılar. Bir listeye dokununca satırda
"Bilgisayarda başlatılıyor…" ve dönen halka görünür; liste bilgisayardaki Spotify'da başlayınca Spotify ekranına
dönülür (Spotify kapalıysa köprü açar ve 15 sn bekler). Sağ üstteki "Karışık" düğmesi açıksa liste karışık ve rastgele
bir şarkıdan başlar; seçim köprüde saklanır. Hata olursa satırda kırmızı yazıyla görünür (ör. "Spotify Premium
gerekli"). En çok 30 liste gösterilir; köprü listeleri 5 dk önbellekte tutar, ekran açılınca çalan listeyi tazeler.
Kapaklar 3×3'lük paftalar halinde şarkı kapaklarıyla aynı flash önbelleğine yazılır (RAM kullanmaz; 30 liste için 4
yuva). Bu özellik Spotify Web API kullanır ve bir kez kurulum ister:

1. https://developer.spotify.com/dashboard → **Create app**. Redirect URI: `http://127.0.0.1:8888/callback`
   (`localhost` kabul edilmiyor), API olarak **Web API**. Mart 2026'dan beri geliştirme modundaki uygulamalar için
   hesap sahibinin Premium olması gerekir; listeyi başlatma ucu zaten yalnızca Premium'da çalışır.
2. Uygulamanın **Client ID**'si ile (client secret gerekmez, PKCE kullanılır):
   `python tools/spotify_bridge.py --spotify-giris <client_id>`. Tarayıcıda Spotify'ın izin sayfası açılır; izin
   verilince yenileme anahtarı `tools/.cache/spotify_api.json` dosyasına yazılır (git'e girmez, ekrana basılmaz).
   Komutu normal (yönetici olmayan) PowerShell'den çalıştır; izin için 15 dk beklenir.
3. Köprüyü her zamanki gibi çalıştır. Giriş yapılmamışsa liste ekranı "Spotify hesabı bağlı değil" der.

Bilgisayarda köprüyü çalıştır (şarkı bilgisi için Spotify hesabı ya da API anahtarı gerekmez; bilgiler Windows'un
medya oturumundan okunur, hesap yalnızca çalma listeleri için):

```bash
python -m pip install pyserial pillow numpy pyaudiowpatch pycaw psutil winrt-runtime winrt-Windows.Media.Control winrt-Windows.Storage.Streams winrt-Windows.Foundation winrt-Windows.Foundation.Collections
python tools/spotify_bridge.py
```

- `--any`: Spotify yoksa tarayıcı, VLC gibi diğer oynatıcıları da göster. `--log`: Rumy'nin günlüğünü de yazdır.
- `--no-beat`: ses çıkışından ritim takibini kapatır. `--beat-offset 250`: Rumy ritmin önündeyse artır, gerisindeyse
  azalt. `--send "rapor"`: bağlanınca Rumy'ye seri komut gönderir.
- `--autostart ac` / `--autostart kapat`: köprüyü Windows açılışında konsolsuz başlatır ya da bunu kaldırır.
  Başlatılan şey bir **bekçidir** (`--bekci`): köprüyü açar, köprü ne sebeple kapanırsa kapansın 5 sn sonra
  yeniden açar; aynı anda tek bekçi ve tek köprü çalışır. Ayrıca Görev Zamanlayıcı'ya "Rumy kopru" görevi
  eklenir: 5 dk'da bir bekçiyi başlatmayı dener (çalışıyorsa bir şey yapmaz; süre sınırı yok). `--durdur`
  bekçiyi ve köprüyü kapatır (görev 5 dk içinde yeniden açar; tamamen kapatmak için `--autostart kapat`).
  Konsolsuz çalışırken olaylar zaman damgasıyla `tools/.cache/kopru.log`'a yazılır (512 KB'ta `.1`'e döner);
  köprü bir hata yüzünden kapanmaz, hatayı kaydedip yeniden bağlanır
  (`--wifi` ile birlikte verilirse WiFi'den bağlanır).
- **WiFi ile (kablo yalnızca güç için):** CYD'yi bir kez USB'ye bağlayıp `python tools/spotify_bridge.py --anahtar-yaz`
  çalıştır. Köprü `tools/.cache/pi_anahtar.txt` dosyasındaki anahtarı (yoksa üretir) karta yazar; kart onu NVS'te
  saklar, anahtar ekrana basılmaz. Sonra `python tools/spotify_bridge.py --wifi <CYD'nin IP'si>` (TCP 7777). Şarkı,
  kapak, kamera, konum değiştirme, ekran görüntüsü ve köprüden gönderilen komutlar (`rapor` gibi, cevabı da WiFi'den
  döner) bu bağlantıdan geçer. IP'nin değişmemesi için modemde CYD'ye sabit adres verilmesi önerilir.
- USB modunda köprü çalışırken seri port meşguldür; karta yükleme yapmadan önce köprüyü kapat (Ctrl+C). WiFi
  modunda USB boştur.
- Kapaklar flash'taki ayrı bir bölümde (`art`, 960 KB, 21 yuva) saklanır; RAM kullanmaz, aynı albüm tekrar
  gelince anında gösterilir. Yeni bir kapak USB'den ~1,2 sn'de gelir.
- Fikir ve protokol, seri porttan "now playing" gönderen açık kaynak projelerden
  ([NowPLayingDisplay](https://github.com/RodgerE1/NowPLayingDisplay)) ve CYD Spotify ekranlarından
  ([Spotify-Diy-Thing](https://github.com/witnessmenow/Spotify-Diy-Thing), MIT) uyarlandı.

### Maç takibi (ESPN)

Takım telefon ya da bilgisayar uygulamasındaki **Futbol** bölümünden seçilir: Süper Lig, Premier Lig, LaLiga, Serie A,
Bundesliga ve Ligue 1 sekmelerinde bütün takımlar renkli rozetleriyle listelenir. "Takımım yok" yalnızca diğer
maçların gol bildirimlerini bırakır; en üstteki **Maç takibi** anahtarı ise futbolu tamamen kapatır (Rumy maç
sorgulamaz, bildirim ve maç kartı çıkmaz; Rumy'deki Futbol sayfası elle açılabilir). Varsayılan olarak takım seçili
değildir. Seri porttan da olur: `takim <espn-no> [renk1 renk2]` (ör. `takim 436 13235b ffd60a`), `takim kapat`,
`futbol ac` / `futbol kapat`. Rumy 6 saatte bir takımın bütün kupalardaki fikstürünü (lig, kupa, Avrupa kupaları)
ESPN'den çeker ve sıradaki maçı tutar.

**Takım renkleri:** uygulama takımla birlikte iki kulüp rengini de Rumy'ye yollar. Gol ekranının zemini, maç kartı,
"bizim" satırların vurgusu ve ampulün gol kutlaması bu renklerden türetilir: zemin her zaman koyu, vurgu rengi zeminle
yeterince ayrışmıyorsa beyazdır. Renk verilmemişse lacivert-sarı varsayılan kullanılır. Uygulamadaki takım listesi ve
renkleri `tools/make_teams.py` ile ESPN'den üretilir (Süper Lig'in kulüp renkleri araçtaki tablodan gelir); ligler her
sezon değiştiği için sezon başında yeniden çalıştırılır.

- **Maç günü:** alttaki bilgi kartında "Bugün 21:45 · FB – Aston Villa"; başlamaya 3 saat kala kart sabitlenir.
- **Maç sırasında** (başlamadan 15 dk önceden itibaren 20 sn'de bir): kart takımının renklerinde canlı skora döner
  ("FB 2 – 1 AVL · 67'", devre arasında "İY"). Takımının golünde 8 sn tam ekran "GOOOL!" (golcü, dakika, skor);
  rakip golü, sarı kart, kırmızı kart ve oyuncu değişikliği (giren ↑, çıkan ↓) ekranın üstünde 6 sn. Dokununca
  kapanır. Rumy açıldığında maç zaten sürüyorsa o ana kadarki olaylar bildirim olarak çıkmaz.
- **Karta dokununca:** maç ekranı (kupa, stadyum, skor, dakika ve bütün olaylar, en yenisi üstte; sağa kaydırınca
  geri).
- **Maç bitince** sonuç 3 saat kartta kalır, sonra sıradaki maç aranır.
- **Hızlı gol bildirimi:** goller LiveScore'dan ~20–45 sn'de gelir (ESPN 50–100 sn geç). Bilgisayardaki köprü
  açıksa takımının, Süper Lig ve ŞL maçlarını o izler ve karta yollar (Rumy seçili takımı köprüye `@futbol` satırıyla
  bildirir); köprü kapalıysa kart takımının maçını kendisi izler. ESPN ayrıntıları sonra gelir, aynı gol iki kez
  bildirilmez.
- **Gol ekranındaki oyuncu fotoğrafı** (köprü açıksa): köprü golcünün fotoğrafını TheSportsDB, Wikimedia Commons ya
  da Vikipedi'den bulur ve kareye çevirir; fotoğraflar yalnızca bilgisayarındaki önbellekte durur, depoda yoktur
  (`tools/player_photo.py`). Bulunamazsa golcünün baş harfleri gösterilir.
- **Lig günü (Süper Lig ve Şampiyonlar Ligi):** o gün oynanan bütün maçlar dakikada bir izlenir; herhangi bir maçta gol olunca
  üstte 6 sn "Galatasaray 1–1 Barcelona · golcü · dakika" kutusu çıkar (takımının maçı sürerken de, onun
  olaylarından sonra; kutuda o ligin rozeti). Maç ekranının altında o günün bütün maçları lig lig, canlı skorla
  listelenir.
- **Lig rozetleri:** depoda hiçbir kulüp ya da turnuva logosu yoktur; kartlarda kısaltmalı sade rozetler görünür (SL,
  ŞL, AL, KL, TK, SK, PL, LL, SA, BL, L1). Kendi kullanımın için logoları `tools/assets/lig_logolari/<ad>.png` olarak
  koyup `python tools/make_match_art.py` çalıştırırsan onlar kullanılır (klasör git dışıdır).
- **Gece modunda** bildirim çıkmaz ve ekran uyanmaz; kart güncellenmeye devam eder.
- Kaynak: ESPN'in herkese açık, belgelenmemiş servisi (`site.api.espn.com`, `sports.core.api.espn.com`; hesap ve
  anahtar gerekmez, şifresiz HTTP). ESPN biçimi değiştirirse güncelleme gerekir. Gecikme ~20–60 sn.
- Denemek için: `mac test basla`, `mac test gol`, `mac test rakip`, `mac test sari`, `mac test kirmizi`,
  `mac test degisiklik`, `mac test devre`, `mac test bitti`, `mac test once`; `mac test kapat` gerçek veriye döner
  (deneme 10 dk sonra kendiliğinden biter).

### Futbol sayfası

Ana ekrandan sağa kaydırınca açılır: Süper Lig, Şampiyonlar Ligi, Premier Lig, LaLiga, Serie A, Bundesliga, Ligue 1.
Lige dokununca o ligin güncel maç günü (canlı skor, dakika; ‹ › ile önceki/sonraki maç günü). Sağ üstteki
**Puan durumu** tuşu tabloyu açar: sıra, oynanan (O), galibiyet/beraberlik/mağlubiyet, averaj, puan; solda
Avrupa kupası / küme düşme renkleri, altta açıklamaları. Veri yalnızca sayfa açıkken ESPN'den çekilir.

### Kod ajanı ışığı: Claude Code ve Codex (ClawLight uyarlaması)

[Claw Light](https://clawlight.dev), kod ajanlarının oturumlarını masadaki bir ışıkla gösteren bir üründür: yeşil
çalışıyor, sarı boşta, kırmızı seni bekliyor. Rumy aynı işi ekranıyla yapar. Durum, Claude Code'un ve Codex'in
hook'larından gelir (masaüstü uygulamaları, terminal ve IDE oturumlarının hepsi); internet ya da hesap gerekmez.
Bildirimler ve oturum sayfası her oturumun hangi ajana ait olduğunu yazar.

| Ana ekrandaki yıldız | Anlamı |
|---|---|
| Yeşil, yavaşça dönüyor | En az bir oturum çalışıyor |
| Kırmızı, nabız atıyor | Bir oturum seni bekliyor: izin, soru, plan onayı ya da API hatası |
| Sarı | Oturumlar işini bitirdi, sıra sende (bitenler 30 dk sonra listeden düşer) |
| Görünmüyor | Açık oturum yok ya da köprü kapalı |

En acil durum kazanır: bir oturum seni bekliyorsa diğerleri çalışsa da yıldız kırmızıdır. Aynı durumda birden çok
oturum varsa yıldızın yanında sayısı yazar.

- **Bildirimler:** bir oturum beklemeye başlayınca üstte 10 sn kırmızı "Claude seni bekliyor · proje · İzin istiyor:
  Bash" kutusu (dokununca oturum sayfası); Rumy yıldıza bakıp göz kırpar; WiZ ampul iki kez kısa kırmızı yanıp eski
  hâline döner (30 sn'de en çok bir kez). Aynı oturum için kutu en çok dakikada bir çıkar. 2 dk'dan uzun süren bir iş
  bitince 6 sn yeşil "Claude işini bitirdi · 7 dk sürdü" ve Rumy 4 sn sevinir. Gece modunda bildirim ve ampul uyarısı
  yok; yıldız güncellenmeye devam eder.
- **Oturum sayfası ("Kod ajanları"):** her oturumun adı, sağında ajanı (Claude / Codex), altında ne yaptığı ve
  süresi ("Dosya düzenliyor · 3 dk", "İzin istiyor: Bash · 40 sn", "Bitti · 12 dk önce · 7 dk sürdü"). Claude
  oturumunun adı çalışma klasörüdür, Codex oturumununki Codex'teki konuşma başlığı. En çok 6 satır; fazlası "+N oturum
  daha".
- **Kurulum (bir kez):** `python tools/claude_agents.py --kur` (yalnızca biri için `--kur claude` / `--kur codex`).
  Claude Code'un `~/.claude/settings.json` dosyasına 10, Codex'in `~/.codex/hooks.json` dosyasına 8 olay için arka
  planda çalışan (`async`) hook eklenir; dosyaların önce yedeği alınır (`*.rumy-yedek-…`). **Codex yeni hook'ları
  çalıştırmadan önce bir kez onay ister:** Codex'te hook uyarısı çıkınca ya da `/hooks` ile Rumy'nin hook'larını
  gözden geçirip güven. Codex'in `config.toml`'una dokunulmaz; Codex'teki başka hook'ların yeri değişmez. Kaldırmak
  için `python tools/claude_agents.py --kaldir`. Oturumları bilgisayarda görmek için `--durum`.
- **Nasıl çalışır:** her olayda `tools/claude_hook.py` (pythonw, ~60 ms, ajanı bekletmez) oturumun durumunu
  `~/.claude/rumy/oturumlar/<oturum>.json` dosyasına yazar. Köprü bu klasörü 0,5 sn'de bir okur ve durum değişince
  (değişmese de 10 sn'de bir) Rumy'ye iletir. Köprü 30 sn haber vermezse yıldız söner. Ajanın süreci (`claude.exe` /
  `codex.exe`) kapanan oturumlar listeden düşer. Codex'te bir işi durdurmak "Durduruldu" olarak görünür.
- **Deneme:** seri `ajan test calisiyor` → `ajan test bekliyor` / `ajan test bitti` / `ajan test codex` (köprü açıksa
  gerçek durum 10 sn içinde geri gelir), `ajan` durum raporu, `ekran ajan`, `wiz uyari` ampul uyarısını dener.

### 3D yazıcı (Creality K2)

Yazıcı ağa bağlıyken Rumy baskıyı kendi takip eder (bilgisayar gerekmez; Creality Print'in de kullandığı yerel
kanal, port 9999). Ana ekrandan sağa kaydırınca Spotify, bir kez daha kaydırınca yazıcı sayfası açılır (köprü
çalışmıyorsa doğrudan yazıcı).

- **Gösterilenler:** yazıcının adı, durum (yazdırılıyor, duraklatıldı, tamamlandı, durduruldu, hata), iş adı ve
  malzeme, katman, nozul/tabla/kabin sıcaklığı, yüzde, bitiş saati ("Bitiş 15:12"), bitişi gösteren çubuk, kalan
  ve geçen süre.
- **Hata olunca:** hangi ekrandaysan yazıcı sayfası kendiliğinden açılır, çerçeve kırmızı olur, yazıcının hata kodu
  yazılır; filament bitmişse "Filament bitti, baskı duraklatıldı" yazar. Gece de ekran 60 sn uyanıp gösterir, gece
  ekranında kırmızı bir hata satırı kalır.
- **Ana ekran:** baskı sürerken bilgi kartlarına "Baskı %45 · 1 sa 12 dk kaldı" eklenir (dokununca yazıcı sayfası);
  gece ekranında sönük "Baskı %45 · bitiş 04:22" görünür.
- **Kurulum:** telefon uygulamasındaki "3D yazıcı" bölümüne yazıcının IP adresini yaz (yazıcının ekranında Ayarlar → Ağ),
  ya da seri porttan `yazici 192.168.1.50`. Yalnızca IPv4 adresi kabul edilir (yazıcının adı, ör. "K2-YAZICI", olmaz). Boş
  bırakırsan özellik kapanır.
- **Kamera:** K2'nin kamerası yalnızca WebRTC video verir, ESP32 bunu çözemez. Bu yüzden görüntü bilgisayardaki
  köprü üzerinden gelir: köprü, yazıcı sayfası açıldığında [go2rtc](https://github.com/AlexxIT/go2rtc)'yi (K2'ye özel
  destekli, yalnızca bu bilgisayardan erişilebilir şekilde) başlatır, görüntüyü küçültüp saniyede bir kare USB'den
  gönderir; sayfadan çıkınca durur. Kabin ışığı kapalıyken görüntü karanlıktır.

```bash
python -m pip install av
```

go2rtc'yi [sürümler sayfasından](https://github.com/AlexxIT/go2rtc/releases) indir (`go2rtc_win64.zip`, 1.9.10 ya da
yenisi) ve `tools/go2rtc/go2rtc.exe` olarak koy (klasör `.gitignore`'da).

**Ampul eşleştirme:** Ayarlar → Ampul (ya da ışık kutusuna dokun) → listeden ampulüne dokun. Ampul bir kez yanıp
söner. Eşleşme MAC adresiyle tutulur; ampulün IP'si değişse bile Rumy onu yeniden bulur.

**Ayarlar ekranı:** konum (81 il), ampul, WiFi (sıfırlama onaylı), **Ekran** alt sayfası (parlaklık, ana ekrana
dönüş süresi, kart süresi, göz uykusu, dokunmatik yönü; yön 10 sn içinde onaylanmazsa geri alınır), **Gece modu** alt sayfası
(kapalı / güneşe göre / saatlere göre, gece parlaklığı, başlangıç–bitiş), **Telefon** (eşleştirme), deprem eşiği,
04:00 bakım yeniden başlatması, tanılama, hakkında ve fabrika ayarları. Adını telefon uygulamasından değiştirebilirsin.

**Seri port komutları** (460800 baud):

| Komut | Ne yapar |
|---|---|
| `rapor` | Bellek, yığınlar, ağ kaynakları, SD, medya, kamera, ampul ve yazıcının tam durumu |
| `yenile` | Bütün verileri hemen yeniden çeker |
| `web ac` / `web kapat` | Telefon uygulamasının sunucusu (açılışta açıktır; `kapat` yeniden başlatana kadar) |
| `telefon unut` | Eşli bütün telefonları siler |
| `gorunum <ifade> <aksesuar> <efekt>` / `gorunum oto` | Geçici görünüm (uygulamadaki Görünüm ile aynı) |
| `sdtest` | SD'ye yazıp 50 kez geri okur |
| `dokunmatik 0-7` | Ekrana dokunmadan dokunmatik yönünü değiştirir ve kaydeder |
| `goruntu ters` / `goruntu duz` | Görüntüyü 180° çevirir (kart kutuya ters oturduysa); dokunmatik ve kaydırma kendiliğinden uyar, ayar kalıcıdır |
| `goz uykusu kapat` / `goz uykusu <1-60>` | Göz uykusunu kapatır ya da dakika olarak ayarlar |
| `kopru` | Köprü bağlantısının durumu (USB/WiFi, anahtar var mı, oturum, aktarılan bayt) |
| `ajan` / `ajan test calisiyor\|bekliyor\|bitti\|codex\|kapat` | Claude ve Codex oturumlarının durumu / sahte oturumla deneme |
| `takim <espn-no> [renk1 renk2]` / `takim kapat` | Takımı seçer (ör. `takim 436 13235b ffd60a`; renkler isteğe bağlı) / takımı kaldırır; ayar NVS'te |
| `futbol ac` / `futbol kapat` | Maç takibini tamamen açar / kapatır (uygulamadaki "Maç takibi" anahtarı) |
| `mac` / `mac yenile` | Maç takibinin durumu (ŞL listesi dahil) / fikstürü hemen yeniden çeker |
| `lig yenile` | Süper Lig ve Şampiyonlar Ligi skorlarını hemen yeniden çeker |
| `ekran futbol` / `ligsayfa <lig> [puan]` | Futbol sayfasını / bir ligin maç gününü ya da puan durumunu açar (`tur.1`, `uefa.champions`, `eng.1`, `esp.1`, `ita.1`, `ger.1`, `fra.1`) |
| `mac test once\|basla\|gol\|rakip\|sari\|kirmizi\|degisiklik\|devre\|bitti\|kapat` | Ekranları sahte bir maçla dener |
| `kopru anahtar <32 onaltılık>` / `kopru anahtar sil` | WiFi köprü anahtarını kaydeder / siler (köprünün `--anahtar-yaz` seçeneği bunu yapar) |
| `bench` / `bench gecis` / `bench kaydir` | Kart kendi kendine sayfa geçişleri ve kaydırmalar yapar, her adımın kare hızını ölçer |
| `bench bellek` | Bütün ekranları gezip her adımda LVGL havuzunun tepesini, en büyük boş bloğu ve parçalanmayı ölçer |
| `lvgl yuk <KB>` / `lvgl yuk 0` | LVGL havuzunu kasıtlı doldurur / bırakır ("bellek yetersiz" davranışını sınamak için) |
| `perf ac` / `perf kapat` | Sen gezinirken saniyede bir çizim ölçümü basar |
| `spitest <MHz>` | Ekrana desen yazıp geri okuyarak SPI hızını sınar (yeniden başlatınca 40 MHz'e döner) |
| `kayma ters` / `kayma duz` | Donanım kaydırmasının yönünü çevirir (farklı panel için) |
| `dokunma ac` / `dokunma kapat` | Her dokunuşun ekran koordinatını yazar (dokunmatik kalibrasyonunu kontrol için) |
| `gece 02:00 07:00` | Gece modunu saatlere göre ayarlar ve kaydeder |
| `yazici <ip>` / `yazici kapat` | Creality K2'nin adresini kaydeder / yazıcı özelliğini kapatır |
| `ekran ana` / `ekran spotify` / `ekran liste` / `ekran yazici` / `ekran mac` / `ekran ajan` | Ekrana dokunmadan o sayfaya geçer |
| `ekran goruntusu` | Ekranın görüntüsünü seri porttan gönderir; köprü bunu `tools/.cache/rumy_ekran.png` olarak kaydeder |
| `wiz durum` / `wiz tara` / `wiz esle <mac>` / `wiz unut` | Ampulün durumu, ağda ampul arama, eşleştirme, eşleşmeyi silme |
| `wiz ac` / `wiz kapat` / `wiz parlak <10-100>` / `wiz sicak <K>` / `wiz renk <0-359>` / `wiz sahne <no>` | Ampulü ekrana dokunmadan kumanda eder |
| `wiz uyari` | Claude bildirimindeki ampul uyarısını dener (iki kısa kırmızı) |
| `reboot`, `wifi-sifirla` | Yeniden başlatma, WiFi sıfırlama |

Köprü çalışırken seri port meşgul olduğundan komutları `tools/.cache/rumy_komut.txt` dosyasına satır satır
yazabilirsin; köprü bunları karta gönderip dosyayı siler (`--log` ile cevap köprünün çıktısında görünür).

## 7. Telefon uygulaması

Rumy kendi küçük web uygulamasını sunar; iPhone'da ana ekrana eklenince tarayıcı çubuğu olmadan, kendi simgesiyle
bir uygulama gibi açılır. App Store, Mac ya da Apple geliştirici hesabı gerekmez.

**Kurulum (bir kez):**
1. Rumy'de **Ayarlar → Telefon**'u aç. Ekranda bir QR kod ve 4 haneli bir kod görünür.
2. iPhone kamerasıyla QR'ı okut; Safari'de `http://rumy.local` açılır.
3. Safari'de **Paylaş → Ana Ekrana Ekle**.
4. Ana ekrandaki Rumy simgesinden aç ve Rumy'deki 4 haneli kodu gir. Telefon bundan sonra eşli kalır.

Ana ekrana eklenen uygulama Safari'den ayrı saklama alanı kullandığı için kodu uygulamanın içinde girmen gerekir.

**Kullanım:** simgeye dokun; ayarlar açılır, değiştirdiğin anda Rumy'ye kaydedilir.
- **Şimdi uyut:** Rumy'yi gece modunun bitiş saatine kadar (varsayılan 07:00) hemen uyutur. Ekrana dokununca
  gece modunda olduğu gibi kısa süre açılır; "Uyandır" uykuyu bitirir. Yeniden başlatmada da uyku sürer.
- **Genel:** ad, konum (il ya da özel koordinat), deprem eşiği.
- **Görünüm:** Rumy'nin ifadesini (13 ifade, küçük resimlerle), aksesuarını ve efektini seç; hemen uygulanır.
  Kalıcı değildir: Rumy'nin kendi hâli bir dahaki değiştiğinde (hava, müzik, maç, gece, ajan…) otomatiğe döner.
  "Şimdi otomatiğe dön" ile de bırakılır.
- **Ekran:** parlaklık, ana ekrana dönüş, kart değişimi, göz uykusu, gece modu ve saatleri, gece parlaklığı.
- **Futbol:** "Maç takibi" anahtarı; Süper Lig ve beş büyük ligden takım (lig sekmeleri, renkli rozetler) ya da
  "Takımım yok"; Süper Lig / Şampiyonlar Ligi gol bildirimleri.
- **Işık:** ampul adı ve durumu; kod ajanı uyarısı, takım golü kutlaması, "ampul kapalıysa dokunma".
- **3D yazıcı**, **Önemli günler** (yıl yazarsan "34. yaş", "10. yıl" gibi gösterilir), **Bakım** (04:00 yeniden
  başlatma, verileri yenile, yeniden başlat, sürüm ve bellek bilgisi).

- Telefon ile Rumy aynı WiFi'de olmalı; evin dışından çalışmaz. Rumy kapalıysa uygulama "Ulaşılamıyor" der.
- En fazla 4 telefon eşleşir (beşincisi en eskinin yerine geçer). Hepsini silmek için Rumy'de Ayarlar → Telefon →
  Unut. Eşleştirme kodu yalnızca o ekran açıkken geçerlidir; 5 hatalı denemede 1 dk kilitlenir.
- Uygulama kullanılırken (son istekten 20 sn sonrasına kadar) Rumy'nin kendi HTTPS istekleri ertelenir.
- `rumy.local` adı ağda çalışmazsa Telefon ekranındaki IP adresiyle de açılır.

**Bilgisayarda:** aynı uygulama geniş pencerede iki sütunlu açılır (solda durum, "Şimdi uyut" ve bölümler; sağda
seçilen bölüm). Masaüstüne ve Başlat menüsüne "Rumy" kısayolu kurmak için:

```bash
python tools/pc_kisayol.py --kur
```

Kısayol Edge'i adres çubuğu olmadan kendi penceresinde açar (`--app=http://rumy.local/`); ilk açılışta Rumy'deki
Ayarlar → Telefon kodunu gir. Kaldırmak için `--kaldir`; `rumy.local` çalışmazsa `--adres http://<ip>/`.

## 8. RAM ve flash bütçesi

Büyük bufferlar açılışta, WiFi başlamadan önce bir kez heap'ten ayrılır ve hiç serbest bırakılmaz. Derleme ölçümleri
`pio run -e cyd` çıktısından ve harita dosyasından alındı. Cihaz sütunu ESP32-2432S028R (ESP32-D0WD-V3, ILI9341)
üzerinde v1.0.2 ile ölçüldü: bütün ekranlar gezildikten ve AFAD/Nager HTTPS istekleri yapıldıktan sonra seri porttaki
`rapor` komutunun çıktısı. Aynı değerler Tanılama ekranında da görünür.

| Kalem | Plan | Derleme ölçümü | Cihazda (v1.0.2) |
|---|---|---|---|
| LVGL havuzu (TLSF) | 24 KB (önce 36 KB) | 24 576 B, heap, bir kez | tepe 16 184 B / ~22,7 KB kullanılabilir (27 Eylül, `bench bellek`) |
| Çizim bufferları (DMA) | 2 × 20 KB (32 satır) | 40 960 B | – |
| Hava ikonları | flash | 148 096 B flash, 0 B RAM | v1.1.0'dan önce 20 KB'lık RAM önbelleği vardı |
| JSON arenası | 8 KB | 8 192 B | – |
| WiZ JSON arenası | 1,5 KB | 1 536 B | – |
| Ağ / WiZ yığını | 10 / 4 KB | 10 240 / 4 096 B | en az 4 728 / 2 756 B boş kaldı |
| UI (loop) yığını | 8 KB | 8 192 B (Arduino varsayılanı) | en az 2 812 B boş kaldı |
| SD / FATFS | ~11 KB | heap, `max_files=1` | 11 068 B |
| WiFi + ağ görevi | – | – | 60 432 B |
| TLS | – | – | oturum boyunca 47 KB, el sıkışmada anlık ~60 KB |
| Uygulama verisi (statik) | ~3,5 KB | 5 500 B: store kopyaları 1 908 · UI kopyası 1 877 · kişisel günler 1 191 · hava 372 · deprem 152 | – |
| Diğer proje statikleri | – | ~4,6 KB: WiZ buffer 1 354 · ekran sürücüsü 602 · ortak metin stilleri 573 · diğer | – |
| Boş heap (WiFi bağlı) | – | – | 72 012 B (v1.0.2) → 68 648 B (Spotify eklenince) → **80 928 B** (LVGL havuzu 24 KB) |
| Boş heap en düşük (HTTPS sırasında) | ≥ 8 KB | – | 10 936 B |

Kalıcı ayırmaların toplamı: 87 552 B buffer + 14 336 B görev yığını ≈ **100 KB**. v1.1.0'da ikon önbelleğinin
20 KB'ı çizim bufferlarına verildi; toplam değişmedi, v1.1.0'da da boş heap 72 KB ölçüldü.

Açılışta her adımdan sonra kalan heap (seri kayıttaki `bos bellek` satırı):

| Adım | Boş heap | Bu adımda harcanan |
|---|---|---|
| Başlangıç (ayarlar, tanılama) | 239 924 B | – |
| Sabit bufferlar | 152 276 B | 87,6 KB |
| Ekran sürücüsü + LVGL | 149 780 B | 2,5 KB |
| SD kart | 138 712 B | 11,1 KB |
| Önbellekler + arayüz | 138 412 B | 0,3 KB |
| WiFi + ağ görevi | 77 980 B | 60,4 KB |
| WiZ görevi | 73 508 B | 4,5 KB |

**Kartta bulunan ve düzeltilen darboğaz:** v1.0.0'da WiFi bağlıyken yalnızca 52,9 KB boş kalıyordu ve HTTPS istekleri
(AFAD, Nager) güvenlik eşiği yüzünden hiç başlamıyordu. Eşik kısa süre düşürülünce el sıkışma sırasında boş heap
3 KB'a indi. SD `max_files` 3 → 1 (FATFS her açık dosya için 4 KB tampon ayırıyor; SD erişimi zaten tek kilitle
sıralı), LVGL havuzu 44 → 36 KB ve ağ yığını 12 → 10 KB yapılarak 18 KB açıldı.

**LVGL havuzunun ekranlara göre kullanımı** (kartta `bench bellek` turuyla ölçüldü; her adımın başında tepe
sıfırlanır; müzik çalarken, Spotify ekranında kapak ve iki satırlık uzun şarkı adıyla, ana ekranda kayan şarkı
şeridiyle):

| Açık ekran (ana ekran her zaman bellekte) | Kullanılan | Tepe |
|---|---|---|
| Ana ekran (kulaklıklı Rumy ve şarkı şeridi dahil) | 5,7 KB | 8,0 KB |
| + Hava (kaydırma, saatlik şerit) | 11,2 KB | 13,4 KB |
| + Işık | 10,6 KB | 12,3 KB |
| + Ayarlar (kaydırma) | 15,0 KB | **16,4 KB** |
| + Ekran / Gece modu alt sayfası | 12,5 / 12,2 KB | 14,0 KB / – |
| + Spotify | 9,4 KB | 10,8 KB |
| + Gece / kutlama ekranı | 6,7 / 6,8 KB | 8,2 / 8,1 KB |

Geçişlerde eski ekran yeni ekran oluşturulmadan önce silinir (donanım kaydırması eski görüntüyü panelin kendi
belleğinde tuttuğu için LVGL nesnelerine gerek yoktur); böylece iki ekran hiçbir zaman aynı anda bellekte olmaz.
Eskiden Ayarlar → alt sayfa geçişlerinde tepe 22,8 KB'a çıkıyordu. Havuz boyutu en yüksek tepenin (16,2 KB) üstüne
%25 pay ve havuzun kendi yönetim yükü (~3,2 KB) eklenerek 24 KB seçildi; bu 12 KB heap kazandırdı. Ana ekrandaki
şarkı şeridi tepeyi 16,4 KB'a çıkardı (+~250 B); pay hâlâ %25'in üstünde (kullanılabilir alan ~22,7 KB).

HTTPS isteği yalnızca en büyük boş blok ≥ 34 KB ve boş heap ≥ 64 KB iken başlatılır (el sıkışmanın cihazda ölçülen
~60 KB'lık anlık tepesine göre); değilse ertelenir ve Tanılama'da "erteleme" sayacı artar (eşikler
`src/app/config.h`). LVGL havuzunda bir ekrana yetecek yer yoksa (ihtiyaçlar ölçülmüş bir tabloda durur) ekran
açılmaz: "Bellek yetersiz" uyarısı çıkar, Rumy ana ekranda kalır, `rapor`'daki "reddedilen ekran" sayacı artar.
Kart yeniden başlamaz (eskiden LVGL `abort()` ile yeniden başlatıyordu). `lvgl yuk <KB>` komutuyla denenebilir.

| İkon | Boyut | Biçim | Flash'ta | LVGL'nin varsayılan yolunda RAM |
|---|---|---|---|---|
| Hava ekranı ana ikonu | 64 | I8 | 5 120 B | 16 384 B |
| Ana ekran hava bloğu | 48 | I8 | 3 328 B | 9 216 B |
| Saatlik şerit, gün doğumu/batımı | 28 | I4 | 456 B | 3 136 B |
| 7 günlük liste | 24 | I4 | 352 B | 2 304 B |

İkonlar `LV_BIN_DECODER_RAM_LOAD=0` ile flash'tan satır satır çözülerek çizilir; RAM'e hiç kopyalanmaz. Ayrıntılı
tablo: `docs/icon_ram.md`, kalite önizlemesi: `docs/icons_preview.png`.

**Flash:** uygulama 1 822 528 B (1,74 MB), hedef < 2 MB. v1.1.0'da gömülü ikonlar (+148 KB) ve hız için `-O2`
(+137 KB) eklendi; bölüm 2,94 MB. 25 Ekim 2026'da uygulama 2 154 976 B (2,06 MB); kablosuz güncelleme için bölüm
2,5 MB'a indi (%82 dolu), boşalan 448 KB'ta güncelleyici duruyor. Statik DRAM (.data + .bss, tüm çekirdek dahil) 59 088 B. IRAM 123 418 B (LVGL'nin
piksel işlevleri IRAM'de; sınır ~128 KB). Web arayüzü gzip'lenmiş olarak flash'ta. Her derlemede
`dist/size_report_<ortam>.txt` güncellenir.

### Performans

Ölçümler kartta `bench` komutuyla alındı: kart kendi kendine geçiş ve kaydırma yapar, her adımın karelerini sayar.

| Adım | v1.0.2 | v1.1.0 |
|---|---|---|
| Sayfa geçişi (220 ms kayma) | 3 kare, 5–7 fps, kare başına 100–190 ms | 18–30 kare, en uzun kare 20–35 ms |
| Hava ekranını açma | 293–304 ms (ikonlar SD'den) | 9–10 ms |
| Hava ekranında kaydırma | 5–10 fps | 24–25 fps |
| Saatlik şerit | 15 fps | 27–30 fps |
| Ayarlarda kaydırma | 13 fps | 30–32 fps (SPI sınırı) |

Neler yapıldı, hangisi ne kazandırdı:

| Değişiklik | Etki |
|---|---|
| Sayfa geçişlerinde ILI9341'in donanım kaydırması (`VSCRSADD`) | Her karede tüm ekran yerine yalnızca yeni açılan şerit çizilir; kareler 5–8 kat ucuzladı |
| Flash QIO 80 MHz (önce DIO 40 MHz) | Yazı tipi ve ikon verisi flash'tan okunduğu için kaydırma ~2 kat hızlandı |
| İkonlar flash'a gömüldü | Hava ekranı açılışı 300 ms → 10 ms, ilk kaydırmadaki takılma bitti, 20 KB RAM boşaldı |
| `-O2` (önce `-Os`) | Çizim %20–25 hızlandı |
| Çizim bufferları 16 → 32 satır | Kare başına ekran parçası 15 → 8, nesne gezme maliyeti yarıya indi |
| LVGL piksel işlevleri IRAM'de, köşe maskesi önbelleği 8 | Birkaç ms daha |
| Dokunmatik SPI 250 kHz → 1 MHz | Parmakla kaydırırken okuma gecikmesi kısaldı |

SPI 80 MHz de denendi: `spitest 80` ekrana yazılan pikselleri geri okuyunca %3–5'i bozuk çıktı, bu yüzden ekran
40 MHz'de kaldı (tam kare 30,7 ms; basit ekranlarda kaydırmanın tavanı ~32 fps). Profil için `pio run -e cyd_prof`
sürümü LVGL'nin iç ölçüm noktalarını toplar; `bench` sonunda işlev bazında süre tablosu basar.

## 9. Geliştirme

```bash
pip install platformio
pio run -e cyd
```

| Ortam | Açıklama |
|---|---|
| `cyd` | ILI9341, normal sürüm → `dist/Rumy.bin`, `dist/Rumy-full-0x0.bin` |
| `cyd_st7789` | ST7789 panelli CYD'ler → `dist/Rumy-st7789.bin` |
| `cyd_endurance` | Dayanıklılık testi: 30 sn'de bir veri, 5 sn'de bir ekran değişimi, 60 sn'de bir log |
| `cyd_prof` | Profil sürümü: LVGL'nin ölçüm noktaları açık, `bench` sonunda işlev bazında süre tablosu |
| `native` | Bilgisayarda birim testleri (`pio test -e native`, 19 takım, 168 test) |

Güncelleyici ayrı bir küçük projedir (`updater/`, `pio run -d updater`); `pio run` onu gerektiğinde kendisi derler,
tam imaja ve USB yüklemesine (0x290000) ekler.

Panel tipi `platformio.ini` içindeki `[rumy] panel = ILI9341` satırından tek yerden değişir. Seri port gibi kişisel
geliştirme ayarları için `secrets.ini.example` dosyasını `secrets.ini` olarak kopyala (`.gitignore`'da).

Klasörler: `lib/rumy_core` (Arduino'suz, test edilen mantık: WiZ protokolü, Open-Meteo/AFAD/Nager ayrıştırıcıları,
önemli gün hesabı, önbellek biçimi), `src/` (board, ui, rumy, wiz, net, weather, quake, events, clock, settings,
webui, diag, app), `web/` (arayüz; derlemede gzip'lenip flash'a gömülür), `tools/`, `test/`.

| Araç | Ne yapar |
|---|---|
| `tools/convert_assets.py` | Meteocons SVG'lerini paletli LVGL `.bin` ikonlarına (`sd_card/rumy/img`) çevirir, sonra `embed_icons.py`'yi çağırır |
| `tools/embed_icons.py` | `sd_card/rumy/img` ikonlarını flash'a gömülen `src/ui/icons_embedded.cpp` dosyasına çevirir |
| `tools/make_fonts.py` | Inter + Font Awesome'dan yalnızca gereken glifleri içeren fontları üretir |
| `tools/make_record.py` | Ana ekrandaki dönen plağın 24 karesini üretip `src/ui/record_frames.cpp`'ye gömer (`--preview dosya.png` ile kareleri gösterir) |
| `tools/make_match_art.py` | Lig rozetlerini ve gol animasyonundaki topu `src/ui/match_art.cpp`'ye gömer; `tools/assets/lig_logolari/` varsa oradaki logoları kullanır |
| `tools/make_teams.py` | Uygulamadaki takım listesini (6 lig, ESPN numarası, kısaltma, iki renk) ESPN'den üretip `web/index.html`'e yazar |
| `tools/gen_ca_bundle.py` | AFAD ve Nager için kırpılmış kök sertifika paketini (`src/net/ca_bundle.h`) üretir |
| `tools/pc_kisayol.py` | Bilgisayarda Rumy uygulamasını açan "Rumy" kısayolunu (masaüstü + Başlat) kurar/kaldırır |
| `tools/veri_koprusu.py` | Köprünün içinde: Rumy'nin istediği AFAD ve Nager verisini bilgisayarda HTTPS ile çekip özet satırı yollar (`python tools/veri_koprusu.py afad` ile tek başına denenir) |
| `tools/build_web.py` | `web/index.html` → `src/webui/web_assets.h` (derlemede otomatik) |
| `tools/post_build.py` | `dist/` imajları ve boyut raporu (derlemede otomatik) |
| `tools/fake_bulb.py` | Gerçek ampul olmadan test için sahte WiZ ampul |
| `tools/endurance_log.py` | Dayanıklılık testini kaydeder ve özetler |
| `tools/ui_preview.py` | Arayüzü kart olmadan bilgisayarda çizer (`docs/preview/`, `docs/ui_preview.png`) |
| `tools/claude_hook.py` | Claude Code ve Codex hook'u: oturumun durumunu `~/.claude/rumy/oturumlar/` altına yazar |
| `tools/claude_agents.py` | Claude ve Codex oturumlarını izleyip köprü üzerinden Rumy'ye yollar; `--kur` / `--kaldir` / `--durum` |
| `tools/fw_update.py` | Kablosuz güncelleme: imajı denetler, köprü üzerinden Rumy'ye yollar, yeni sürümü doğrular (`spotify_bridge.py --guncelle`) |

Kod kuralları: C++ kodunda yorum satırı yok; LVGL çağrıları yalnızca UI görevinde; döngülerde `String` birleştirme
yok; bloklayan `delay()` yok; tüm ağ işlemlerinde zaman aşımı var. Ayrıntılar ve katkı adımları:
[CONTRIBUTING.md](CONTRIBUTING.md).

### Kartsız önizleme

```bash
python tools/ui_preview.py
```

Gerçek LVGL 9.5'i, gerçek ekran kodunu (`src/ui`, `src/rumy`), flash'a gömülen gerçek ikonları ve test verilerini
bilgisayarda derleyip ekran görüntülerini üretir; yalnızca WiFi, WiZ, SD, saat ve ekran sürücüsü `sim/sim_stubs.cpp`
ile taklit edilir. Her ekran için LVGL havuzu kullanımını da yazar (`.pio/sim_build/sim_run.log`). MinGW-w64 g++
(PATH'te ya da `RUMY_MINGW` ile) ve Pillow gerekir. `RUMY_SIM_POOL_KB` ile havuz boyutu denenebilir. Varsayılan 36 KB'tır: önizleme 64 bit
çalıştığından (işaretçiler iki kat) cihazdaki 24 KB'lık havuzun karşılığıdır.

Uç durumlar için ayrı bir senaryo vardır: `python tools/ui_preview.py --cesit` (`--buyuk` ile birlikte de olur)
bütün hava ikonlarını, uç sıcaklıkları, uzun adları, eksik veriyi, emojili/Japonca şarkı ve liste adlarını, uzun
maç ve yazıcı metinlerini çizer → `docs/preview_cesit/`, `docs/ui_cesit.png`. Her koşuda otomatik denetim yapılır:
üst üste binen yazı/ikon, kesilen yazı ve fontta olmayan harfler `docs/preview*/denetim.txt`'ye yazılır (bilerek
böyle olanlar `sim/denetim_izin.txt` ile susturulur).

Animasyonlar için `python tools/ui_preview.py --film`: gol ekranının dört gösterisi, kapanış, kayarak gelen
bildirimler, maç bildirimleri ve lig rozetli kartlar → `docs/preview_film/` (her animasyon bir GIF ve bir inceleme
şeridi). Takım renklerini denemek için `RUMY_SIM_ONLY=takim python tools/ui_preview.py --film` altı takımla canlı
kartı, gol ekranını ve maç ekranını çizer. Her film için LVGL'nin her yenilemede ekranın yüzde kaçını yeniden çizdiği yazdırılır (kartta akıcılığın
ön göstergesi).

![Rumy ekranları](docs/ui_preview.png)

## 10. Dayanıklılık testi

```bash
pio run -e cyd_endurance -t upload
python tools/endurance_log.py --port COM7 --hours 24 --csv dist/endurance.csv
```

Özet; en düşük boş heap, heap eğilimini (B/saat), en büyük blok, LVGL parçalanması, TLS tepesi, yığın en düşükleri,
ağ hataları ve beklenmeyen yeniden başlatmaları verir. Sızıntı şüphesi eşiği −300 B/saat.

## 11. Sorun giderme

| Belirti | Çözüm |
|---|---|
| Renkler ters, görüntü kaymış | `Rumy-st7789.bin` kullan ya da `panel = ST7789` ile derle |
| Dokunma yanlış yere gidiyor | ILI9341'li CYD'de doğru değer 6'dır (varsayılan). Başka bir kartta Ayarlar → Ekran → Dokunmatik yönü; onayı 10 sn içinde ver. Ekran hiç kullanılamıyorsa USB'den `dokunmatik 0-7` komutu |
| "SD kart yok" kartı | Kart takılı ve FAT32 mi? Kartı çıkarıp tak ve yeniden başlat. Kartsız her şey çalışır, yalnızca kişisel günler kaydedilmez |
| Sayfa geçişinde görüntü ters yöne kayıyor | Farklı panelde kaydırma yönü ters olabilir: USB'den `kayma ters` komutunu dene |
| Ampul bulunamıyor | Ampul açık ve aynı 2,4 GHz ağda mı? Misafir ağı / AP izolasyonu kapalı mı? `tools/fake_bulb.py` ile Rumy tarafını test et |
| Hava durumu gelmiyor | Tanılama'da "Hava + 7 gün" satırındaki koda bak (anlık, saatlik ve 7 günlük veri tek istekte gelir); `yenile` komutu ya da telefon uygulamasında Bakım → "Verileri yenile" |
| Deprem verisi gelmiyor | `rapor`'daki `afad` satırının sonundaki yol `kopru`, `https` ya da `kandilli` olmalı. Köprü açıksa AFAD'ı bilgisayar çeker; kapalıysa Rumy Kandilli'yi kullanır. Köprü penceresinde `veri -> Rumy` satırlarına bak |
| WiFi değişti | Ayarlar → WiFi → Sıfırla. Rumy kurulum moduna geçer |
| Donma / yeniden başlama | Seri portta `rapor` çıktısını al; Tanılama'da "Açılış" satırı sıfırlanma nedenini gösterir |

## 12. Lisanslar

Rumy'nin kendi kodu [PolyForm Noncommercial 1.0.0](LICENSE) lisanslıdır: kişisel kullanım, hobi, eğitim ve
araştırma için kullanabilir, değiştirebilir ve paylaşabilirsin; ticari kullanım (satmak, ücretli bir ürüne ya da
hizmete koymak) lisansa dahil değildir ve ayrıca izin gerektirir. Ticari izin için GitHub'da bir issue aç ya da
[@SudoMrSpear](https://github.com/SudoMrSpear) profilinden ulaş. Bu yüzden proje OSI tanımıyla "açık kaynak" değil,
"kaynağı açık" bir projedir.

Kullanılan kütüphaneler, yazı tipleri, ikonlar ve veri kaynaklarının
listesi ile lisans notları [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) dosyasında, lisans metinleri `LICENSES/`
klasöründedir. Özetle:

| Bileşen | Lisans |
|---|---|
| LVGL 9.5 | MIT |
| LovyanGFX | FreeBSD (BSD-2) |
| ArduinoJson 7 | MIT |
| WiFiManager (tzapu) | MIT |
| Arduino-ESP32 | LGPL-2.1 |
| ESP-IDF, mbedTLS, lwIP | Apache-2.0 / BSD |
| Meteocons (Bas Milius) — hava ikonları | MIT |
| Inter (Rasmus Andersson) — yazı tipi | SIL OFL 1.1 |
| Font Awesome Free — simgeler | SIL OFL 1.1 (font), CC BY 4.0 (simgeler) |
| Montserrat — lig rozetlerinin yazısı (yalnızca üretimde) | SIL OFL 1.1 |
| Open-Meteo verisi | CC BY 4.0 |
| AFAD deprem verisi | T.C. İçişleri Bakanlığı AFAD, kamuya açık servis |
| Kandilli deprem listesi (yedek) | Boğaziçi Üniversitesi Kandilli Rasathanesi ve Deprem Araştırma Enstitüsü, kamuya açık sayfa |
| Nager.Date | MIT |

Kök sertifikalar Mozilla deposundan (certifi) alınır; "GlobalSign Root CA" artık o depoda olmadığı için
GlobalSign'ın resmî adresinden indirilip SHA-256 parmak iziyle doğrulanır (`tools/gen_ca_bundle.py`).

## 13. Raspberry Pi sürümü (deneysel)

Aynı arayüz Raspberry Pi Zero W ve Waveshare 3,5" (A) SPI ekranda da çalışır. Deneyseldir ve CYD kadar sınanmamıştır.
Kod `pi/` klasöründe; arayüz, `rumy_core` ve veri modülleri CYD ile ortak.

**Bir kerelik kurulum (Pi'de):**

1. Raspberry Pi Imager ile "Raspberry Pi OS Lite (32-bit)". Ayarlar: ana makine adı `rumy-pi`, kullanıcı `rumy`, SSH açık
   anahtarla. WiFi adında Türkçe harf varsa (ör. "ı") Imager 2.0.11 adı bozuyor: ilk açılışta Pi'de `sudo nmtui`
   → "Activate a connection" ile ağı listeden seçip şifreyi girin.
2. Ekranı takın (ilk 26 pin), sonra Pi'de bir kez: `sudo bash ~/rumy/pi/kurulum.sh` ve `sudo reboot`.
   Betik ekran sürücüsünü açar, konsol yazılarını ekrandan uzak tutar, Rumy servisinin açılışta başlamasını sağlar.
3. İsteğe bağlı ama önerilir: `sudo bash ~/rumy/pi/kurulum2.sh` ve `sudo reboot`.
   - Rumy'yi ağ beklemeden, açılışın ~16. saniyesinde başlayan bir sistem servisine çevirir. Kullanıcı servisiyle Rumy
     ancak ~90. saniyede başlıyordu.
   - Çekirdek saatini 400 MHz'de sabitler (`core_freq_min=400`). Pi boşa düşünce SPI ekran hızı 22'den 14 MHz'e
     iniyordu.
   - Takası yalnızca bellekte (zram) tutar. Her açılışta 19 sn süren takas dosyası hazırlığı kalkar.
   - Günlüğü dakikada bir diske yazdırır; elektrik kesilince son dakikalar kaybolmaz.
   - Ekran ve dokunmatik sürücülerini açılışın başında yükletir (`/etc/modules-load.d/rumy-ekran.conf`).
   - Geri almak için: `sudo bash ~/rumy/pi/kurulum2.sh geri`.
4. Önerilir: `sudo bash ~/rumy/pi/kurulum3.sh` (yeniden başlatma gerekmez). Servise bekçi ekler: Rumy'nin ana
   döngüsü 30 sn boyunca "yaşıyorum" bildirmezse systemd onu yeniden başlatır. `Restart=always` yalnızca çökmeyi
   yakalıyordu, takılmayı yakalamıyordu. Betik, yüklü Rumy bekçiyi desteklemiyorsa hiçbir şeyi değiştirmez. Durum için
   seri komut `bekci`; denemek için `bekci dur` (Rumy ~30 sn içinde yeniden başlamalı). Geri almak için
   `sudo bash ~/rumy/pi/kurulum3.sh geri`.
5. Ekran hızını denemek için: `sudo bash ~/rumy/pi/spi_hiz.sh 40` ve `sudo reboot`. Ekranda bozulma görülürse önce
   `spi_hiz.sh 32`, o da olmazsa `spi_hiz.sh 24` ile geri alınır. Tam ekran kaydırma (ör. hava ekranı) bu hızla
   sınırlıdır: 24 ayarında ~10 kare/sn, 40'ta hesapla ~16 kare/sn.
6. Güç: Pi'yi "PWR" girişinden, sağlam ve kısa bir kabloyla, en az 5 V 2,5 A'lik bir adaptörle besleyin. Temassız kablo
   Pi'yi uyarı vermeden kapatıp açar.

**Derleme ve gönderme (bilgisayardan):**

```bash
python tools/pi_deploy.py --deps --sd
python tools/pi_deploy.py --restart
```

`--deps` ve `--sd` yalnızca ilk seferde gerekir. Derleme Pi'nin kendisinde yapılır; ilk derleme Zero W'da uzun sürer,
sonrakilerde yalnızca değişen dosyalar derlenir. Adres `--host` ya da `RUMY_PI_HOST` ile verilir.

Her derlemede çalışan sürüm `pi-build/rumy.onceki` olarak saklanır. Yeni sürüm sorun çıkarırsa
`python tools/pi_deploy.py --geri` önceki sürüme döner ve Rumy'yi yeniden başlatır; bir daha çalıştırmak yeni sürüme
geri getirir. Bekçi açıkken bekçiyi desteklemeyen bir sürüme dönülmez.

**Spotify ve yazıcı kamerası (bilgisayar açıkken):** köprü USB yerine WiFi ile bağlanır.

```bash
python tools/spotify_bridge.py --pi rumy-pi.local
```

Bağlantı anahtarı Pi'de `~/.config/rumy/gizli.json` dosyasında; bilgisayarda `tools/.cache/pi_anahtar.txt` (git dışı).

**Pi'deki dosyalar:** ayarlar `~/.config/rumy/ayarlar.json`, önbellek ve SD içeriği `~/.local/share/rumy/sd`,
isteğe bağlı `~/.config/rumy/ekran.json` (`{"dondur": 180}` görüntüyü çevirir, `{"olcek": 1}` büyütmeden gösterir) ve
`~/.config/rumy/dokunmatik.json` (dokunmatik ayarı). Web arayüzü Pi'de 8080 portundadır.

**Dokunmatik ayarı:**
- Bu kartta dokunmatik yönü 2'dir (Y ters). Pi derlemesinde varsayılan budur. Dokunuş yine yukarı-aşağı ters gidiyorsa seri komut: `dokunmatik 2`.
- Dokunuşlar düğmeleri ıskalıyorsa *Ayarlar > Ekran > Dokunmatik ayarı* açılır. Seri komutla da açılır:
  `ekran dokunmatik`.
- Sırayla dört köşede ve ortada birer artı çıkar; her birinin ortasına dokunup parmak kaldırılır.
- Rumy ham dokunmatik değerlerinden iki ekseni hesaplar. Eksenlerin ters ya da yer değiştirmiş olmasını da kendisi
  bulur.
- Ortadaki beşinci artı sağlamadır: dokunuş 14 piksel içinde kalırsa ayar `dokunmatik.json`'a yazılır, kalmazsa baştan
  başlar. 30 sn dokunulmazsa hiçbir şey değişmeden çıkar.
Seri komutlar (`rapor`, `ekran ...`, `yazici ...`, `wiz ...`) köprü bağlantısından (TCP 7777) gönderilir.

**Ekran:**
- Pi sürümü arayüzü ekranın kendi çözünürlüğünde, 480×320'de çizer (`RUMY_SCREEN_W/H`, `pi/Makefile`). Büyütme
  yoktur, görüntü keskindir. CYD 320×240'ta kalır; iki boyut aynı koddan derlenir.
  - Yerleşim `theme::S()` (4/3 ölçek), `R()` (sağa yaslı), `CX()` (ortalı) ve `FW()` (tam genişlik) yardımcılarıyla
    yazılır. CYD'de bunlar birebir değer döndürür; CYD görüntüsü değişmez.
  - Büyük yazı tipleri, ikonlar, aksesuarlar ve plak kareleri ayrı dosyalardadır ve yalnızca 480×320 derlemesine girer:
    `python tools/make_fonts.py --buyuk`, `python tools/convert_assets.py --buyuk`, `python tools/make_record.py --buyuk`.
  - Simülatör: `python tools/ui_preview.py --buyuk` → `docs/preview_buyuk/`.
  - Kapak 192 px, kamera 213×160. Pi bağlantıda `@merhaba 192 213x160` der; köprü kapak ve kamera boyutunu buradan
    öğrenir.
- Rumy SPI ekrana DRM ile doğrudan yazar ve yalnızca değişen dikdörtgeni gönderir; aktarım ayrı bir iş parçacığında
  (`panel`) yapılır.
- Bu ekranda donanım kaydırması olmadığı için sayfa geçişi kaydırma yerine, yeni sayfayı geldiği kenardan şerit şerit
  açarak yapılır.
- Arka ışık ayarlanamaz; parlaklık pikselleri koyulaştırarak ve göz algısına göre (gama 2,2) uygulanır.
- Karşılaştırma için ortam değişkenleri: `RUMY_FBDEV=1` eski çerçeve arabelleği yolu, `RUMY_KAYDIR=1` eski kaydırmalı
  geçiş.

**Sağlık kaydı:** Rumy her dakika `~/.local/share/rumy/son_durum.txt` dosyasına saat, çalışma süresi, sıcaklık, güç
bayrakları (`vcgencmd get_throttled`), yük ve boş bellek yazar. Açılışta bir önceki kaydı günlüğe basar
(`[saglik] onceki son kayit`). Açılış nedeni de açılış kimliğine göre ayrılır: elektrik kesilmişse "güç verildi", Rumy
çökmüşse "çökme".

**WiZ:** Yeni WiZ yazılımlarında (ör. 1.38.0) ampul, imzasız yerel komutları reddedebilir (`-32602 Invalid params`);
ışık ekranında "Komut izni yok · WiZ" görünür. Açılıp kapandığı görünür ama renk ve mod değişmez. Çözüm, WiZ
uygulamasında *Ayarlar > Geçerli ev > Güvenlik* altında "Tüm kontrollere izin ver" (İngilizce uygulamada "Only verified
controls" kapalı) seçeneğidir. Bu ayarla ev ağındaki her cihaz ampule komut gönderebilir. Okuma bu ayardan etkilenmez.
Tanılama: `wiz durum | tara | esle <mac> | ac | kapat | parlak <10-100> | sicak <K> | renk <0-359> | sahne <no>`.

## 14. Katkı, güvenlik ve yasal not

- **Katkı:** hata bildirimleri ve öneriler GitHub Issues'tan; kod katkısı için kurallar ve denetim adımları
  [CONTRIBUTING.md](CONTRIBUTING.md) dosyasında. Sürüm notları: [CHANGELOG.md](CHANGELOG.md).
- **Güvenlik:** Rumy yalnızca ev ağında kullanılmak üzere tasarlandı; güven modeli ve açık bildirme yolu
  [SECURITY.md](SECURITY.md) dosyasında. 80 (uygulama) ve 7777 (köprü) portlarını internete açma.
- **Teknik doküman:** mimari, ekranlar, bellek, ağ ve ölçümler [Rumy_Proje_Dokumani.md](Rumy_Proje_Dokumani.md)
  dosyasında (İngilizcesi: [Rumy_Project_Document.md](Rumy_Project_Document.md)).
- **Yasal not:** Rumy bağımsız bir hobi projesidir; ESPN, LiveScore, TheSportsDB, UEFA, TFF ya da adı geçen ligler ve
  kulüpler, Spotify, WiZ (Signify), Creality, Anthropic, OpenAI, Claw Light, AFAD, Kandilli Rasathanesi, Open-Meteo ile
  bağlantılı değildir ve onlar tarafından desteklenmez. Adlar yalnızca uyumluluğu anlatmak için geçer. Maç verisi
  ESPN'in ve LiveScore'un herkese açık ama belgelenmemiş servislerinden gelir; bu servisler değişebilir ya da
  kapanabilir, kişisel kullanım içindir.
