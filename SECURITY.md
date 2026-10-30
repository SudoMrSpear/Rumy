# Güvenlik

[English](#security-english) · **Türkçe**

## Güven modeli

Rumy ev ağında, güvenilen cihazlarla birlikte kullanılmak üzere tasarlandı.

- **Uygulama (port 80):** telefon ve bilgisayar uygulaması şifresiz HTTP ile konuşur. Eşleştirme, Rumy'nin ekranında
  Ayarlar → Telefon açıkken görünen 4 haneli kodla yapılır; 5 hatalı denemede 1 dakika kilit vardır. Eşleşen cihaz
  128 bitlik rastgele bir belirteç alır (en çok 4 cihaz). Belirteçler ve ayarlar kartın NVS belleğinde durur.
- **Köprü (port 7777):** bilgisayardaki köprü WiFi ile bağlanırken 128 bitlik paylaşılan bir anahtar gönderir; anahtar
  kartın NVS'inde ve bilgisayarda `tools/.cache/pi_anahtar.txt` dosyasında (git dışı) durur. Bağlantı şifrelenmez.
  Köprü komutları ve kablosuz güncelleme WiFi'de yalnızca bu anahtarla kurulan bağlantıdan kabul edilir; USB'de
  karta fiziksel erişim gerekir.
- **WiFi şifresi** ilk kurulum portalında girilir ve yalnızca NVS'te tutulur; kodda ya da depoda yer almaz.
- **Spotify:** herkes kendi geliştirici uygulamasının Client ID'sini kullanır (PKCE, client secret yok); yenileme
  anahtarı yalnızca bilgisayarda `tools/.cache/spotify_api.json`'da durur.

## Öneriler

- 80 ve 7777 portlarını **internete açma** (modemde port yönlendirme yapma).
- Rumy'yi misafir ağı yerine kendi ağında tut; ağında güvenmediğin cihaz varsa köprüyü USB ile kullan.
- Seri port `rapor` çıktısını paylaşırken ağ adreslerini ve MAC adreslerini silebilirsin.

## Açık bildirme

Bir güvenlik açığı bulursan lütfen herkese açık bir issue açma. GitHub'daki depo sayfasında
**Security → Report a vulnerability** ile gizli olarak bildir. Açığın nasıl tekrarlanacağını ve etkisini yaz;
düzeltme yayınlanana kadar ayrıntıları paylaşmamanı rica ederiz.

---

## Security (English)

### Trust model

Rumy is designed to be used on a home network, together with trusted devices.

- **App (port 80):** the phone and computer app talk plain HTTP. Pairing uses the 4-digit code shown on Rumy's screen
  while Settings → Phone is open; there is a 1-minute lock after 5 wrong tries. A paired device gets a random 128-bit
  token (at most 4 devices). Tokens and settings live in the board's NVS memory.
- **Bridge (port 7777):** when the bridge on the computer connects over WiFi it sends a 128-bit shared key; the key is
  kept in the board's NVS and on the computer in `tools/.cache/pi_anahtar.txt` (outside git). The connection is not
  encrypted. Over WiFi, bridge commands and wireless updates are accepted only on a connection set up with this key;
  over USB, physical access to the board is needed.
- **The WiFi password** is entered in the first setup portal and kept only in NVS; it is never in the code or the
  repository.
- **Spotify:** everyone uses the Client ID of their own developer app (PKCE, no client secret); the refresh token is
  kept only on the computer in `tools/.cache/spotify_api.json`.

### Recommendations

- **Do not expose** ports 80 and 7777 to the internet (no port forwarding on the router).
- Keep Rumy on your own network rather than a guest network; if there are devices on your network you do not trust,
  use the bridge over USB.
- When sharing the serial `rapor` output you can remove network addresses and MAC addresses.

### Reporting a vulnerability

If you find a security vulnerability, please do not open a public issue. Report it privately with
**Security → Report a vulnerability** on the repository page on GitHub. Describe how to reproduce it and its impact;
we ask that you do not share the details until a fix is released.
