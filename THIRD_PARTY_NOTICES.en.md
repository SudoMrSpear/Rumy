# Third-party components and notices

**English** · [Türkçe](THIRD_PARTY_NOTICES.md)

Rumy's own code is licensed under the PolyForm Noncommercial 1.0.0 (`LICENSE`); commercial use needs separate
permission. The components below are used under their own licences; the licence texts are in the `LICENSES/` folder.

## Compiled into the board firmware

These libraries are not in the repository; PlatformIO downloads them at fixed versions during the build
(`platformio.ini`). The compiled `.bin` files on Releases contain them.

| Component | Version | Licence | Text |
|---|---|---|---|
| [LVGL](https://github.com/lvgl/lvgl) | 9.5.0 | MIT, Copyright (c) LVGL Kft | `LICENSES/LVGL-MIT.txt` |
| [LovyanGFX](https://github.com/lovyan03/LovyanGFX) | 1.2.30 | FreeBSD (BSD-2), Copyright (c) lovyan03; adapted code inside it MIT/BSD | `LICENSES/LovyanGFX.txt` |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | 7.4.3 | MIT, Copyright (c) Benoit Blanchon | `LICENSES/ArduinoJson-MIT.txt` |
| [WiFiManager](https://github.com/tzapu/WiFiManager) | 2.0.17 | MIT, Copyright (c) tzapu | `LICENSES/WiFiManager-MIT.txt` |
| [Arduino-ESP32](https://github.com/espressif/arduino-esp32) | platform espressif32 6.13.0 | LGPL-2.1 | `LICENSES/LGPL-2.1.txt` |
| ESP-IDF, mbedTLS, lwIP and other Espressif components (inside Arduino-ESP32) | – | Apache-2.0 / BSD | `LICENSES/Apache-2.0.txt` |

Arduino-ESP32 is licensed under the LGPL-2.1: all the source needed to rebuild the compiled image (this repository and
the fixed versions in `platformio.ini`) is available; the library's source is at the link above.

## Assets embedded in the project

| Asset | Where | Licence |
|---|---|---|
| [Inter](https://github.com/rsms/inter) typeface — Copyright (c) 2016 The Inter Project Authors | `src/ui/fonts/*.c`, `src/app/portal_font.h` (only the glyphs needed, `tools/make_fonts.py`) | SIL OFL 1.1, `LICENSES/OFL-1.1.txt` |
| [Font Awesome Free](https://fontawesome.com) 5 icons — Copyright (c) Fonticons, Inc. | `src/ui/fonts/*.c` (icon glyphs) | Icons CC BY 4.0, font SIL OFL 1.1; `LICENSES/FontAwesome-Free.txt` |
| [Meteocons](https://github.com/basmilius/weather-icons) — Copyright (c) 2020-present Bas Milius | `src/ui/icons_embedded.cpp`, `src/ui/icons_big.cpp`, `sd_card/rumy/img/` | MIT, `LICENSES/Meteocons-MIT.txt` |
| [Montserrat](https://github.com/JulietaUla/Montserrat) — Copyright 2011 The Montserrat Project Authors | The lettering of the league badges, as images in `src/ui/match_art.cpp` (`tools/make_match_art.py`, from the file in the LVGL package) | SIL OFL 1.1, `LICENSES/OFL-1.1.txt` |
| Mozilla root certificate store (certifi) and GlobalSign Root CA | `src/net/ca_bundle.h`, `certs/` | Public root certificates; selected by `tools/gen_ca_bundle.py` |

The fonts were renamed during conversion (`font_rumy_*`); Font Awesome's reserved name is not used.

Rumy's face, accessories, the spinning record and football frames, the app icon and the league badges were drawn in
this project and are covered by the project's licence (PolyForm Noncommercial 1.0.0). The repository contains no club
or competition logos and no player photos.

## Services used at run time

These are the services Rumy or the bridge on the computer fetch data from while running; their data is not in the
repository (except small sample responses in the tests, `test/fixtures/`).

| Service | Use | Note |
|---|---|---|
| [Open-Meteo](https://open-meteo.com) | Weather, air quality, marine | Data CC BY 4.0; attribution: "Weather data by Open-Meteo.com" |
| AFAD | Earthquake list (HTTPS through the bridge) | Disaster and Emergency Management Authority, Ministry of Interior of Türkiye; public service |
| Kandilli Observatory and Earthquake Research Institute | Earthquake list (fallback) | Boğaziçi University, public page |
| [Nager.Date](https://date.nager.at) | Public holidays | The service is MIT-licensed open source |
| ESPN | Fixtures, scores, events, standings, team list | Public but undocumented endpoints; personal use |
| LiveScore | Fast goal alerts (bridge) | Public but undocumented endpoints; personal use |
| TheSportsDB, Wikimedia Commons, Wikipedia | Player photo on the goal screen (bridge) | Photos only live in the cache on the user's computer; the licence of each Commons photo is on that file's page |
| Spotify Web API | Playlists (bridge) | Each user creates their own developer app; the Spotify Developer Terms apply |
| WiZ (local UDP), Creality K2 (local WebSocket) | Bulb and printer | Local network protocols, unofficial use |

## Projects that inspired Rumy

- [Claw Light](https://clawlight.dev): the idea of showing coding agent sessions with a light (Rumy's implementation is
  its own code).
- [NowPLayingDisplay](https://github.com/RodgerE1/NowPLayingDisplay) and
  [Spotify-Diy-Thing](https://github.com/witnessmenow/Spotify-Diy-Thing): the idea of sending "now playing" over a
  serial port and of a CYD Spotify display.
- [Launcher](https://github.com/bmorcelli/Launcher): installing apps from the SD card (Rumy's `Rumy.bin` is compatible
  with it; the repository contains no Launcher code).

## No affiliation

Rumy is an independent hobby project. It is not affiliated with or endorsed by ESPN, LiveScore, TheSportsDB, UEFA, TFF
and the leagues and clubs mentioned, Spotify, WiZ (Signify), Creality, Espressif, Anthropic (Claude Code), OpenAI
(Codex), Claw Light, AFAD, Kandilli Observatory or Open-Meteo. Names and trademarks appear only to describe
compatibility and belong to their owners.
