# Katkı

[English](#contributing-english) · **Türkçe**

Rumy'ye katkı için teşekkürler. Hata bildirimi ve öneriler GitHub Issues'tan gelir; kod değişiklikleri pull request
olarak. Proje Türkçe başladı; İngilizce yazılan issue ve PR'lar da memnuniyetle okunur.

## Hata bildirirken

- Kartın panel tipi (ILI9341 ya da ST7789) ve Rumy'nin sürümü (Tanılama ekranı ya da seri port `rapor`).
- Mümkünse seri porttan (460800 baud) `rapor` çıktısı. Çıktıda WiFi şifresi yoktur, ama ağ adresleri ve ampulün MAC
  adresi olabilir; paylaşmadan önce silebilirsin.
- Ne yaptın, ne bekliyordun, ne oldu.

## Kod kuralları

Bunlar projenin başından beri uyulan kurallardır; PR'lar da bunlara göre incelenir.

- **C++ kodunda yorum satırı yok.** Açıklamalar README'de ve `Rumy_Proje_Dokumani.md`'de; kod adlandırmayla okunur
  tutulur.
- **Python araçlarında** yalnızca dosya başında, kullanımı anlatan açıklama metni olur.
- **LVGL çağrıları yalnızca UI görevinde** yapılır; diğer görevler veriyi `store` üzerinden yayınlar.
- Çalışma sırasında büyük `malloc` yok, döngülerde `String` birleştirme yok, bloklayan `delay()` yok, her ağ işleminde
  zaman aşımı var.
- **Kişisel ayarlar kodda değil NVS'te** durur. Kişisel geliştirme ayarları (`secrets*.ini`) git dışıdır.
- Kart ve köprü dışındaki servislere giden isteklerde `User-Agent` projeyi tanıtır ve kişisel bilgi taşımaz.
- Kullanıcının gördüğü metinler `rc::L("Türkçe", "English")` ile (dosya düzeyindeki dizilerde paralel İngilizce
  tabloyla) yazılır ki iki dil de çalışmaya devam etsin.

## Göndermeden önce

```bash
pio run -e cyd
pio run -e cyd_st7789
pio run -e cyd_endurance
pio run -e cyd_prof
pio test -e native
```

- Dört ortam da `-Wall -Wextra` ile **uyarısız** derlenmeli; bütün native testler geçmeli.
- Arayüzü değiştiren her şey önce **simülatörde** denenir: `python tools/ui_preview.py` (gerekirse `--buyuk`,
  `--film`, `--cesit`, `--tarih`). Denetim `denetim: 0 bulgu` vermeli; bilerek böyle olan bir bulgu varsa nedeniyle
  birlikte `sim/denetim_izin.txt`'ye eklenir. Simülatör MinGW-w64 g++ ve Pillow ister.
- Arayüz değişikliklerinde PR'a önce/sonra ekran görüntüsü ekle.
- Yeni bir ayrıştırıcı ya da protokol satırı `lib/rumy_core` altına yazılır ve `test/` altında testi eklenir.
- README'yi ve gerekiyorsa `Rumy_Proje_Dokumani.md`'yi güncelle; kullanıcıyı ilgilendiren değişiklikleri
  `CHANGELOG.md`'ye ekle.

## Varlıklar ve lisanslar

- Depoya kulüp ya da turnuva logosu, oyuncu fotoğrafı veya lisansı belirsiz görsel eklenmez.
- Yeni bir kütüphane, yazı tipi ya da ikon eklersen lisansını `THIRD_PARTY_NOTICES.md`'ye ve gerekiyorsa metnini
  `LICENSES/` altına koy.
- Katkıların projenin lisansıyla (PolyForm Noncommercial 1.0.0) yayınlanır. Bir katkı göndererek, katkının proje
  sahibi tarafından ticari lisanslara da dahil edilebilmesi için proje sahibine (Kerem Mızrak) katkın üzerinde
  süresiz, dünya çapında, devredilebilir ve alt lisans verilebilir bir kullanım hakkı tanımış olursun. Bunu kabul
  etmiyorsan PR'ın açıklamasına yaz; katkını o şekilde değerlendiririz.

---

## Contributing (English)

Thanks for contributing to Rumy. Bug reports and ideas come through GitHub Issues; code changes as pull requests. The
project started in Turkish, and issues and PRs written in English are just as welcome.

### When reporting a bug

- The board's panel type (ILI9341 or ST7789) and Rumy's version (Diagnostics screen or the serial `rapor` command).
- If possible, the `rapor` output from the serial port (460800 baud). It contains no WiFi password, but it may contain
  network addresses and the bulb's MAC address; you can remove them before sharing.
- What you did, what you expected, what happened.

### Code rules

These rules have been followed since the start of the project; PRs are reviewed against them.

- **No comment lines in C++ code.** Explanations live in the README and `Rumy_Proje_Dokumani.md`
  (`Rumy_Project_Document.md` in English); code is kept readable through naming.
- **Python tools** have only a description of their usage at the top of the file.
- **LVGL calls only in the UI task**; other tasks publish their data through `store`.
- No large `malloc` at run time, no `String` concatenation in loops, no blocking `delay()`, a timeout on every network
  operation.
- **Personal settings live in NVS, not in code.** Personal development settings (`secrets*.ini`) are outside git.
- Requests to services other than the board and the bridge identify the project in their `User-Agent` and carry no
  personal information.
- User-facing text goes through `rc::L("Türkçe", "English")` (or a parallel English table for file-scope arrays) so
  both languages keep working.

### Before submitting

```bash
pio run -e cyd
pio run -e cyd_st7789
pio run -e cyd_endurance
pio run -e cyd_prof
pio test -e native
```

- All four environments must build **without warnings** with `-Wall -Wextra`, and every native test must pass.
- Anything that changes the interface is tried in the **simulator** first: `python tools/ui_preview.py` (with
  `--buyuk`, `--film`, `--cesit`, `--tarih` as needed). The audit should report `denetim: 0 bulgu` (0 findings); a
  finding that is intentional goes into `sim/denetim_izin.txt` with its reason. The simulator needs MinGW-w64 g++ and
  Pillow.
- Add before/after screenshots to the PR for interface changes.
- A new parser or protocol line is written under `lib/rumy_core` with a test under `test/`.
- Update the README and, if needed, `Rumy_Proje_Dokumani.md`; add user-facing changes to `CHANGELOG.md`.

### Assets and licences

- No club or competition logos, player photos or images with unclear licences are added to the repository.
- If you add a new library, font or icon, put its licence in `THIRD_PARTY_NOTICES.md` and, if needed, its text under
  `LICENSES/`.
- Your contributions are published under the project's licence (PolyForm Noncommercial 1.0.0). By submitting a
  contribution you grant the project owner (Kerem Mızrak) a perpetual, worldwide, transferable and sublicensable right
  to use your contribution, so it can also be included in commercial licences by the project owner. If you do not
  agree, say so in the PR description and we will consider your contribution on that basis.
