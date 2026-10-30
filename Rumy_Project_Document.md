# Rumy — Project Document

**English translation** of [Rumy_Proje_Dokumani.md](Rumy_Proje_Dokumani.md). If the two differ, the Turkish original
is the reference. Serial commands, log lines and many file and identifier names are Turkish words; they are kept
as they are in the code, with the meaning given where it helps.

**Scope:** everything done from the start of the project until today (27 September 2026): what was done, in which
language, with which libraries and tools, how, and why it was done that way. Version running on the board: **1.2.0**
(after 1.1.0: Spotify screen, Rumy with headphones, beat tracking, animated sleep "z"s, new home screen layout, a
"now playing" strip with a spinning record on the home screen, Creality K2 printer screen and camera, improvements
1–2).

This document does not repeat the README; the README is the user guide (installation, commands, troubleshooting).
This document describes the project's **internal structure and history**. Code is referenced by file and function
names, since line numbers may change.

---

## Contents

1. [Project summary](#1-project-summary)
2. [Hardware](#2-hardware)
3. [Languages, libraries and tools used](#3-languages-libraries-and-tools-used)
4. [Folder structure](#4-folder-structure)
5. [Overall architecture](#5-overall-architecture)
6. [Memory and flash layout](#6-memory-and-flash-layout)
7. [Display, touch and drawing infrastructure](#7-display-touch-and-drawing-infrastructure)
8. [Screens](#8-screens)
9. [Rumy's face and animations](#9-rumys-face-and-animations)
10. [Network, data sources and cache](#10-network-data-sources-and-cache)
11. [Special days, earthquakes, clock](#11-special-days-earthquakes-clock)
12. [WiZ smart bulb](#12-wiz-smart-bulb)
13. [Settings, first setup and the phone app](#13-settings-first-setup-and-the-phone-app)
14. [SD card](#14-sd-card)
15. [Spotify screen and the PC bridge](#15-spotify-screen-and-the-pc-bridge)
16. [Rumy with headphones and beat tracking](#16-rumy-with-headphones-and-beat-tracking)
17. [Diagnostics, serial commands, measurement tools](#17-diagnostics-serial-commands-measurement-tools)
18. [Performance work](#18-performance-work)
19. [Testing and verification](#19-testing-and-verification)
20. [Build, upload and distribution](#20-build-upload-and-distribution)
21. [Code rules and constraints](#21-code-rules-and-constraints)
22. [Version history](#22-version-history)
23. [Problems met and their solutions](#23-problems-met-and-their-solutions)
24. [Known limits and open work](#24-known-limits-and-open-work)
25. [Creality K2 printer screen and camera](#25-creality-k2-printer-screen-and-camera)
26. [Football (ESPN)](#26-football-espn)
27. [Spotify playlists](#27-spotify-playlists)
28. [Coding agent light: Claude Code and Codex (adapted from ClawLight)](#28-coding-agent-light-claude-code-and-codex-adapted-from-clawlight)
29. [Wireless update (WiFi)](#29-wireless-update-wifi)
30. [Phone app (iPhone)](#30-phone-app-iphone)
31. [Language support (English / Turkish)](#31-language-support-english--turkish)

---

## 1. Project summary

Rumy is a **desk companion** written for the **ESP32-2432S028R** ("Cheap Yellow Display", CYD for short) board. It
is a character made of two glowing cyan eyes on a black screen. It reacts to what is going on with its eye
expressions; on the same screen it shows:

- **Time and date** (synchronised with NTP, Turkey time),
- **Weather** (Open-Meteo: current, hourly, 7-day, air quality, sea temperature),
- **Marmara earthquakes** (AFAD; Kandilli when there is no bridge and not enough memory),
- **Holidays and remembrance days** (Nager.Date + the Diyanet table) and personal special days,
- **WiZ smart bulb control** (UDP, local network),
- **Music playing on the computer** (Spotify screen: cover, song title, controls) and Rumy **"listening"** to it
  too (putting on headphones and nodding to the beat).

The whole interface was originally Turkish; since the language support in section 31 it is English by default with
Turkish selectable. Settings can be changed from the device's touch screen, from a phone (the phone app added to the
home screen, section 13.3) or over the USB serial port. The WiFi password and personal settings live in the device's
NVS memory, **not in the code**.

---

## 2. Hardware

| Part | Spec | Use in Rumy |
|---|---|---|
| Processor | ESP32-D0WD-V3, 2-core Xtensa LX6, 240 MHz | Core 1: UI; core 0: network and bulb tasks |
| RAM | 520 KB SRAM (~240 KB usable as heap), no PSRAM | Fixed buffers allocated once at boot |
| Flash | 4 MB, QIO 80 MHz | App (1.81 MB), NVS, cover cache (960 KB) |
| Display | 2.8" 320×240 TFT, **ILI9341** (ST7789 on some boards), SPI | 40 MHz SPI, DMA, hardware scroll |
| Touch | XPT2046 resistive, separate SPI | Software SPI 1 MHz, calibration + orientation fix |
| microSD | SPI (VSPI) | Optional: personal days, holiday table, data cache |
| RGB LED | GPIO 4/16/17 | Turned off at boot |
| USB-serial | CH340 | Upload (auto reset) and the 460 800 baud serial line (commands + Spotify bridge) |
| Backlight | GPIO 21, PWM | Brightness and night dimming |

**Pin table** (`src/app/config.h`):

| Signal | Pin | Signal | Pin |
|---|---|---|---|
| TFT SCLK / MOSI / MISO | 14 / 13 / 12 | TFT DC / CS / BL | 2 / 15 / 21 |
| Touch SCLK / MOSI / MISO | 25 / 32 / 39 | Touch CS / IRQ | 33 / 36 |
| SD SCK / MISO / MOSI / CS | 18 / 19 / 23 / 5 | LED R / G / B | 4 / 16 / 17 |

Raw touch range: X 300–3900, Y 200–3700. On the ILI9341 board the touch orientation (`offset_rotation`) is **6** (the
X axis was found to be reversed on the board and fixed; 0 for ST7789).

---

## 3. Languages, libraries and tools used

### 3.1 Languages

| Language | Where | Why |
|---|---|---|
| **C++17** (`-std=gnu++17`) | Board firmware (`src/`, `lib/rumy_core/`), simulator (`sim/`), unit tests (`test/`) | The Arduino framework on the ESP32 runs on C++; `constexpr`, namespaces and templates (`store::putT`) are used. No exceptions or RTTI, dynamic memory only at boot. |
| **C** | The LVGL library, generated font files (`src/ui/fonts/*.c`) | LVGL is written in C; configured with `lv_conf.h`. |
| **Python 3.10** | Every tool under `tools/`: build scripts, icon/font generators, the simulator runner, the endurance logger, the **Spotify bridge** | Quick to write, has libraries like Pillow/numpy, and PlatformIO scripts are Python too. |
| **HTML + CSS + JavaScript** | Phone app (`web/index.html`, `web/icon.png`) | A single file; gzipped and embedded in flash at build time. |
| **PowerShell / Bash** | Build, upload and measurement commands during development | Windows development environment. |

### 3.2 Board-side libraries (PlatformIO `lib_deps`)

| Library | Version | Role |
|---|---|---|
| **Arduino-ESP32** (espressif32 platform `6.13.0`) | 2.0.17 (based on ESP-IDF 4.4) | FreeRTOS, WiFi, NVS (`Preferences`), SD, UART, esp_partition |
| **LVGL** | 9.5.0 | UI drawing: objects, styles, animation timers, software rendering engine |
| **LovyanGFX** | 1.2.30 | ILI9341/ST7789 driver (SPI+DMA), XPT2046 touch, first setup screen |
| **ArduinoJson** | 7.4.3 | Streaming, filtered parsing in a fixed arena of the weather/earthquake/holiday JSON |
| **WiFiManager** (tzapu) | 2.0.17 | The "Rumy-Kurulum" access point and WiFi selection page at first setup |
| mbedTLS, lwIP | Inside ESP-IDF | HTTPS (AFAD, Nager; bridge/Kandilli when memory is short, section 10.6) and TCP/UDP |

### 3.3 Computer-side tools and libraries

| Tool / library | Where it is used |
|---|---|
| **PlatformIO** (`platformio.exe` inside penv) | Build, upload, unit tests, environments (`cyd`, `cyd_st7789`, `cyd_endurance`, `cyd_prof`, `native`) |
| **esptool** 4.x | Flash write/read (backups, app-only updates) |
| Xtensa GCC (PlatformIO toolchain) | Compiler for the board |
| **MinGW-w64 g++** | Compiler for the board-less preview (simulator) |
| **Unity** | Unit test framework for `pio test -e native` |
| **Pillow** | Icon conversion, preview PNGs, cover image processing |
| **resvg-py, pypng, lz4** | The Meteocons SVG → PNG → LVGL icon pipeline |
| **Node.js + lv_font_conv 1.5.3** | Generating glyph-subset fonts from Inter and Font Awesome |
| **pyserial** | Serial port (endurance log, Spotify bridge, tests) |
| **winrt** (`winrt-Windows.Media.Control` etc.) | Reading the playing song and its cover from the Windows media session (SMTC) |
| **numpy + pyaudiowpatch** | Listening to the audio output (WASAPI loopback) to find the beat/tempo |
| **pycaw** | Only during development, to verify that the volume buttons changed the Windows volume |

### 3.4 Data and asset sources

| Source | Licence | Use |
|---|---|---|
| Meteocons (Bas Milius) | MIT | Weather icons (64/48/28/24 px) |
| Inter (Rasmus Andersson) | SIL OFL 1.1 | All text (12/16/22 px and 56 px digits) |
| Font Awesome 5 Free | OFL (font), CC BY 4.0 (icons) | Symbols (bulb, WiFi, play, Spotify logo...) |
| Open-Meteo | CC BY 4.0 | Weather, air quality, marine |
| AFAD | Public service | Earthquake list |
| Kandilli Observatory (Boğaziçi Univ.) | Public page, plain HTTP | Earthquake list when there is no bridge and no memory for HTTPS |
| Nager.Date | MIT | Public holidays |
| Mozilla CA store (certifi) + GlobalSign | – | Trimmed root certificate bundle |

---

## 4. Folder structure

```
Rumy/
├── platformio.ini          Environments, build flags, version (1.3.0), panel selection
├── partitions/rumy_4mb.csv Flash partition table (nvs, otadata, app0, updater, coredump, art)
├── updater/                The wireless update's updater: a small separate app living in ota_1
│                           (platformio.ini, src/main.cpp, copy_bin.py; section 29)
├── include/
│   ├── lv_conf.h           LVGL configuration
│   └── rumy_prof.h         Profiling macros (cyd_prof only)
├── src/                    Board firmware (C++)
│   ├── main.cpp            Boot sequence and main loop
│   ├── app/                config.h (all constants), mem (fixed buffers), store (data between tasks),
│   │                       shared.h (shared data structures), portal (first setup), app (restart requests)
│   ├── board/              display (LovyanGFX + LVGL bridge, hardware scroll), lgfx_cyd.h (panel setup), sdcard
│   ├── ui/                 ui.cpp (screen manager), scr_*.cpp (each screen), theme, draw_util, slide (transition),
│   │                       icons + icons_embedded (icons in flash; icons_big for 480×320), record_frames (+_big),
│   │                       bench, lvmem, rows, fonts/
│   ├── rumy/               rumy.cpp (face, expressions, animations, headphones), accessories (+_big; flag/hat/umbrella)
│   ├── media/              media.cpp (Spotify bridge protocol, cover cache, tempo), camera.cpp (decodes printer
│   │                       camera frames streamed over serial and draws them on the panel), fwupdate.cpp
│   │                       (receives the wireless update, section 29)
│   ├── printer/            printer.cpp (local WebSocket client for the Creality K2, runs in the network task)
│   ├── net/                net.cpp (network task and scheduler), http_min (small HTTP/HTTPS client),
│   │                       ca_bundle.h (root certificates), cache (SD cache)
│   ├── weather/ quake/ events/ clock/ settings/ wiz/ webui/ diag/
├── lib/rumy_core/src/      Logic that does not depend on Arduino and is tested on the computer (parsers,
│                           special day calculation, WiZ protocol, cache format, texts and the language helper,
│                           bridge protocol media_proto, weather kind wx_mood, team palette color)
├── test/                   Unit test suites + fixtures/ (samples of API and printer replies)
├── sim/                    Board-less preview: sim_main.cpp (scenario), sim_variants.cpp (edge cases),
│                           sim_audit.cpp (automatic screen audit), sim_stubs.cpp (hardware fakes),
│                           denetim_izin.txt (audit findings that are intentional)
├── pi/                     Raspberry Pi version (experimental): Makefile, lv_conf.h, include/ (Linux equivalents
│                           of Arduino/ESP-IDF calls), src/ (display/DRM, touch, network, files, clock, TCP bridge,
│                           health log, service watchdog), kurulum.sh, kurulum2.sh and kurulum3.sh (administrator
│                           steps), spi_hiz.sh (display speed),
│                           rumy.service (user service)
├── tools/                  Python tools (see sections 3 and 20); tools/go2rtc/ (go2rtc.exe, in .gitignore);
│                           pi_deploy.py (send to the Pi, build there, restart)
├── web/index.html, icon.png Phone app and home screen icon
├── sd_card/                Content that can be copied to the SD card, and icon sources (img/)
├── docs/                   Preview images, icon RAM table
├── dist/                   Build outputs (.bin) and size reports (in .gitignore)
├── certs/                  Root certificate sources
├── LICENSE, LICENSES/       Project licence and third-party licence texts (THIRD_PARTY_NOTICES.md)
├── .github/                GitHub Actions build, bug and idea templates
├── README.md               User guide (README.en.md in English)
├── CHANGELOG.md            Release notes
├── CONTRIBUTING.md, SECURITY.md
└── Rumy_Proje_Dokumani.md  This document (Rumy_Project_Document.md in English)
```

**Amount of code** (excluding generated font/icon/web files): about **18 000 lines**. The largest files:
`src/rumy/rumy.cpp` (face and animations), `tools/spotify_bridge.py`, `src/ui/scr_home.cpp`,
`src/ui/scr_weather.cpp`, `src/ui/ui.cpp`, `src/ui/scr_light.cpp`, `src/media/media.cpp`, `src/wiz/wiz.cpp`.
Generated files: `src/ui/icons_embedded.cpp` (148 KB of icon data), `src/ui/record_frames.*` (24 record frames,
30 KB; `tools/make_record.py`), `src/ui/fonts/*.c` (4 fonts), `src/webui/web_assets.h` (gzipped web page),
`src/app/portal_font.h` (setup screen font), `src/ui/match_art.*` (league badges and the spinning ball frame;
`tools/make_match_art.py`).

---

## 5. Overall architecture

### 5.1 Boot sequence (`src/main.cpp`, `setup()`)

1. `Serial.setRxBufferSize(2304)` and `Serial.begin(460800)`: the serial line is opened large enough to buffer the
   Spotify bridge's 2 KB cover chunks.
2. The RGB LED is turned off, `store::init()` (data areas shared between tasks), `settings::load()` (settings from
   NVS; this also sets the interface language), the boot counter is incremented, `diag::begin()` (records the reset
   reason).
3. If WiFi has never been set up, `portal::run()`: without starting LVGL, a plain setup screen drawn with LovyanGFX
   and the WiFiManager access point open (see 13.2).
4. `mem::init()`: every large buffer is allocated **once** (LVGL pool 36 KB, two draw buffers 2×20 KB, JSON arenas).
   On failure an error is written to the screen and the board restarts after 10 s.
5. `display::begin()` (panel + touch), `lv_init()`, `display::startLvgl()` (LVGL display, touch input, hardware scroll
   setup).
6. `sdcard::begin()` (without a card a warning is logged and work goes on), the latest weather and earthquake data
   from the caches, `events::begin()`.
7. `media::begin()`: the cover partition in flash is mapped and the used slots are counted.
8. `ui::begin()`: theme; the home screen is created and loaded.
9. `net::begin()`: the network task (core 0) starts; WiFi connection, NTP, data fetch scheduler.
10. `wiz::begin()`: the bulb task (core 0).
11. After each step the remaining free heap is measured and written to the serial port on a single line
    (`bos bellek: basla ... wiz ...`, "free memory: start ... wiz ...").

On every pass `loop()` runs: `ui::loop()` (LVGL timers, drawing, screen transitions, data polling) → `diag::loop()`
(serial commands + bridge lines) → `media::loop()` (connection and transfer timeouts) → `app::poll()` (pending restart
requests).

### 5.2 Tasks and cores

| Task | Core | Priority | Stack | Job |
|---|---|---|---|---|
| Arduino `loopTask` (UI) | 1 | 1 | 8 KB | LVGL, every screen, the serial line, the Spotify bridge, writing covers |
| `net` | 0 | 3 | 10 KB | WiFi, NTP, HTTP/HTTPS requests, JSON parsing, SD cache |
| `wiz` | 0 | 4 | 4 KB | Bulb discovery over UDP, reading state, sending commands |

**Rule:** LVGL calls are made **only in the UI task**. Other tasks write their results to `store`; the UI task polls
every 250 ms.

### 5.3 Data flow: `store` (`src/app/store.*`)

`store` is the single home of data shared between tasks. For each **domain** it keeps a copy and a version number:
`FORECAST, DAILY, AIR, MARINE, QUAKE, EVENTS, BULB, NET, LVGL, SETTINGS`.

- A producer (e.g. the network task) copies under a lock with `store::putT(store::FORECAST, data)`, and the version
  goes up.
- The UI task calls `store::getT(domain, copy, seenVersion)` in `pollData()`; it copies only if the version changed
  and sets the matching bit mask (`M_FORECAST`, `M_BULB`...).
- `dispatch(mask)`: an `update(mask)` call goes to the home screen and to the currently open screen. Each screen
  invalidates (has redrawn) only the objects affected by the masks it cares about.
- Per-second events (`M_SECOND`), minutes (`M_MINUTE`), night mode (`M_NIGHT`) and media (`M_MEDIA`, Spotify bridge)
  are dispatched with the same mask system.

Thanks to this, tasks never wait for each other, LVGL stays in one task and redrawing only happens when needed.

### 5.4 Screen manager (`src/ui/ui.cpp`)

- Screen IDs: `SCR_HOME, SCR_WEATHER, SCR_LIGHT, SCR_SETTINGS` (the main row navigated by horizontal swipes) and the
  sub-screens `SCR_DIAG, SCR_PAIR, SCR_LOCATION, SCR_NIGHT, SCR_INFO, SCR_EVENT, SCR_DISPLAY_CFG, SCR_NIGHT_CFG,
  SCR_MEDIA`.
- **The home screen stays in memory**; other screens are created when opened and deleted when closed (to protect the
  LVGL pool).
- `go(screen, direction)` records the transition to apply on the next pass; `doGo()` in order: **deletes the old
  screen** (unless it is home), checks whether the pool has room for the new screen (`roomFor`), creates the screen,
  updates it and starts the hardware-scrolled transition (`slide::run`). Since the old screen's picture stays in the
  panel's own memory during the slide, no LVGL objects are needed for it; with this order two screens are never in
  memory at the same time (pool peak 22.8 KB → 16.2 KB).
- **If memory runs short:** every screen's measured need (with a 25% margin, e.g. Settings 13.5 KB, Weather 10 KB,
  Spotify 6.5 KB) is kept in a table. If the free space in the pool is less than that, or the largest free block is
  smaller than 2 KB, the screen is not opened: an "Out of memory, screen could not be opened" warning appears, Rumy
  stays on the home screen, the reason is written to the serial port (at most once every 5 s) and the "rejected
  screen" counter in `rapor` goes up. LVGL used to restart the board with `abort()` in this situation.
- Gestures: swiping left/right moves along the main row; on sub-screens swiping right goes back; on the home screen
  swiping right opens the Spotify screen (if the bridge is connected); on the Spotify screen swiping left returns
  home.
- Automatic rules (`perSecond()`):
  - In night mode hours, if untouched, it switches to the night screen, and back to home when night ends.
  - On sub-screens, after the set time (60 s by default) without a touch it returns home (except the Spotify
    screen).
  - On the Spotify screen, if music has been paused for 1 min and untouched for 1 min, it returns home.
  - On the first boot of a special day the full-screen celebration card is shown.
- When music **starts** and we are on the home screen it switches to the Spotify screen (not on song changes); if the
  bridge connection drops it leaves the Spotify screen.

---

## 6. Memory and flash layout

### 6.1 RAM budget

Large buffers are allocated at boot, before WiFi starts, and are never freed (prevents fragmentation):

| Item | Size |
|---|---|
| LVGL pool (TLSF) | 24 KB (36 KB until 27 September) |
| Draw buffers (DMA, 2 × 32 lines × 320 × 2 B) | 2 × 20 KB |
| JSON arena / WiZ JSON arena | 8 KB / 1.5 KB |
| Network task stack / WiZ stack | 10 KB / 4 KB |
| Serial receive buffer (for the Spotify bridge) | 2.3 KB |
| WiFi driver + network task | ~60 KB |
| SD / FATFS (`max_files = 1`) | ~11 KB |

Measured values (on the board, `rapor` command): with WiFi connected **free heap ~80.8 KB** (68.6 KB before the LVGL
pool was shrunk, 72 KB before Spotify was added). An HTTPS request only starts while free heap is ≥ 64 KB and the
largest block is ≥ 34 KB (the handshake's momentary peak was measured at ~60 KB); otherwise it is postponed and a
counter goes up. The margin above the threshold went from 4.6 KB to **~16.8 KB**.

**LVGL pool use per screen** (a `bench bellek` run on the board; the peak is reset at the start of each step;
measured with music playing, a cover and a two-line long song title on the Spotify screen):

| Screen / step | Use | Peak |
|---|---|---|
| Home screen (Rumy with headphones, notes) | 5.4 KB | 6.8–7.6 KB |
| Weather (scrolling, hourly strip) | 10.9 KB | 13.2 KB |
| Light | 10.3 KB | 12.0 KB |
| Settings (scrolling) | 14.8 KB | **16.2 KB** (highest) |
| Display / Night mode sub-page | 12.3 / 11.9 KB | 13.7 KB / – |
| Location / Bulb / Diagnostics / About | 8.2 / 7.5 / 9.0 / 7.4 KB | – / – / 10.0 KB / – |
| Spotify screen | 9.1 KB | 10.5 KB |
| Night screen / Celebration screen | 6.5 / 6.5 KB | 8.0 / 7.8 KB |

The pool size was set from this measurement: highest peak 16 196 B, plus a 25% margin (20 245 B), plus the pool's
own overhead (TLSF control structure + block headers, ~3.2 KB at the peak) → 23.5 KB → **24 KB**. Before shrinking,
the old and new screens were in memory together during transitions, so the peak was 22.8 KB; deleting the old screen
first (section 5.4) brought it down to 16.2 KB.

### 6.2 Flash partitions (`partitions/rumy_4mb.csv`)

| Partition | Type | Address | Size | Content |
|---|---|---|---|---|
| `nvs` | data/nvs | 0x9000 | 20 KB | Settings (WiFi, name, location, brightness, night mode, language...) |
| `otadata` | data/ota | 0xE000 | 8 KB | Which app to boot (main firmware or updater) |
| `app0` | app/ota_0 | 0x10000 | 2.5 MB | App (25 October: 2 154 976 B = 2.06 MB, 82% of the partition) |
| `updater` | app/ota_1 | 0x290000 | 448 KB | **Updater** (421 968 B, section 29) |
| `coredump` | data/coredump | 0x300000 | 64 KB | Crash dump |
| `art` | data (0x40) | 0x310000 | 960 KB | **Album cover cache** (21 slots × 44 KB) |

The `art` partition was added in this session; before that the last 960 KB of flash sat empty. When the table changed,
NVS stayed at the same address, so settings were kept. On 25 October, for wireless updates, `app0` was reduced from a
2.94 MB `factory` partition to a 2.5 MB `ota_0`, and the updater (`ota_1`) was put in the freed 448 KB; `nvs`,
`coredump` and `art` stayed at the same addresses, so settings and the cover cache were kept again. Two full app
partitions (classic dual OTA) would not fit: 2 × 2.15 MB > 3.01 MB.

### 6.3 Static memory

Static DRAM (.data + .bss, core included) ~60 KB; IRAM ~123 KB / ~128 KB (LVGL's frequently called pixel functions
were moved to IRAM for speed; close to the limit).

---

## 7. Display, touch and drawing infrastructure

### 7.1 Panel driver (`src/board/lgfx_cyd.h`, `display.cpp`)

- ILI9341 (or ST7789) on HSPI with LovyanGFX, write 40 MHz, read 16 MHz, DMA.
- Rotation 1: landscape 320×240. In this rotation the ILI9341's MADCTL is `MV|MX|MH`.
- The backlight is set as a percentage with PWM; separate brightness in night mode.
- Touch: XPT2046, software SPI 1 MHz (was 250 kHz), calibration range + `offset_rotation` 6.
- LVGL's "flush" callback sends each chunk to the panel with DMA; LVGL colour format `RGB565_SWAPPED` (the panel's
  byte order).
- While the `dokunma ac` serial command is on, the coordinate of every touch is written to the serial port
  (calibration check; added in this session and used to diagnose the volume button problem).

### 7.2 LVGL configuration (`include/lv_conf.h`)

- Partial rendering, two 32-line buffers: one is drawn into while the other goes to the panel.
- `LV_MEM_SIZE` 24 KB (was 36 KB; see 6.1), TLSF allocator, caches off (`LV_CACHE_DEF_SIZE 0`). The pool is allocated
  from the heap once (`LV_MEM_POOL_ALLOC`); it must equal `LVGL_POOL_BYTES` in `config.h` (`static_assert`).
- `LV_ATTRIBUTE_FAST_MEM = IRAM_ATTR`: pixel fill/blend functions in IRAM.
- `LV_DRAW_SW_CIRCLE_CACHE_SIZE 8`: cache for rounded corner masks.
- `LV_BIN_DECODER_RAM_LOAD 0`: icons are decoded from flash line by line, never copied to RAM.
- `LV_USE_FS_ARDUINO_SD` for SD; in the profiling build LVGL's measurement points are tied to
  `rumy_prof_begin/end`.

### 7.3 Custom draw objects (`src/ui/draw_util.*`)

To save RAM, most screens are drawn directly **in a single object's `DRAW_MAIN` event** instead of dozens of label
objects. Helpers: `du::text` (single-line text), `du::box` (rounded fill), `du::hgrad` (horizontal gradient),
`du::hfade` (horizontal fade from one colour to transparent; softens the edges of scrolling text), `du::image`
(icon), `du::Clip` (temporary clip region), `du::visible` (is a row visible).
Example: the clock, weather block, date row, "now playing" strip and bars on the home screen are five custom draw
objects; the home screen therefore takes only ~5.5 KB in the LVGL pool.

### 7.4 Hardware-scrolled page transition (`src/ui/slide.cpp`)

In the first version the page transition redrew the whole screen every frame, so it managed only 3 frames /
5–7 fps. The fix is the ILI9341's **vertical scroll** feature (which becomes horizontal scroll in landscape use):

1. At boot `VSCRDEF (0x33)` defines the whole screen as the scroll area.
2. When a transition starts, LVGL's display refresh timer is paused, the new screen is loaded and its layout is
   computed.
3. Each frame, the target position is found on a smoothly decelerating (cubic ease-out) curve; **only the newly
   revealed column strip** is drawn (LVGL's invalid area list is set by hand and `lv_display_refr_timer` is called),
   then the panel's start line is shifted with `VSCRSADD (0x37)`: the panel scrolls the old picture in its own
   memory.
4. When the transition ends the timer resumes and it waits for the finger to lift (to avoid a stray touch).

Result: 18–30 frames in the 220 ms transition, longest frame 20–35 ms. If the direction is reversed on a different
panel, the `kayma ters` command flips it.

### 7.5 Fonts (`tools/make_fonts.py`)

- **Inter** (12 px regular, 16 px medium, 22 px semi-bold) and **56 px** digits for the clock (with a fixed-width
  digit fix, so the text does not shift as the time changes).
- **Font Awesome** symbols are embedded in the same fonts (`RS_BULB`, `RS_WIFI`, `RS_PLAY`, `RS_PAUSE`, `RS_NEXT`,
  `RS_PREV`, `RS_VOL_UP`, `RS_VOL_DOWN`, `RS_MUSIC`, `RS_SPOTIFY`... in `src/ui/fonts/symbols.h`).
- Only the glyphs needed are generated (lv_font_conv, 4-bit greyscale). In this session the character set was
  widened for song titles: Latin-1 (é, ñ, ü, ø...) and Latin Extended-A (ő, ł, č, ś...) were added; Turkish letters
  were already there. The extended fonts and the Spotify code together grew the app by ~74 KB (1.74 → 1.81 MB).

### 7.6 Icons (`tools/convert_assets.py`, `tools/embed_icons.py`)

Meteocons SVG → PNG at the target size with resvg → palette PNG with Pillow → LVGL binary format with LVGL's
`LVGLImage.py` script → C array with `embed_icons.py` (`src/ui/icons_embedded.cpp`, 148 096 B, in flash).

| Size | Format | Use |
|---|---|---|
| 64 px | I8 (256-colour palette) | Weather screen main icon |
| 48 px | I8 | Home screen weather block |
| 28 px | I4 (16 colours) | Hourly strip, sunrise/sunset |
| 24 px | I4 | 7-day list |

Before v1.1.0 icons were read from the SD card and held in a 20 KB RAM cache; opening the weather screen took
~300 ms. Embedded in flash it dropped to 10 ms and the 20 KB of RAM went to the draw buffers.

### 7.7 Theme (`src/ui/theme.h`)

Black background (`C_BG`), cyan eye colour `0x19D3F0`, neon palette: green `0x2BFF6A`, lime `0xB6FF3B`, cyan
`0x00E0FF`, yellow `0xFFD60A`, orange `0xFF9F1C`, red `0xFF4040`, pink `0xFF4FA3` (headphones), purple `0xB84DFF`,
blue `0x1E6BFF`; bar background `0x1B222B`. Temperature is coloured by value (`theme::tempColor`).

---

## 8. Screens

### 8.1 Home screen (`src/ui/scr_home.cpp`)

Layout (320×240, pixel coordinates):

| Area | Position | Content |
|---|---|---|
| Face | 0,0 – 320×88 | Rumy's eyes (see section 9); headphones and notes while music plays |
| Location | 8,8 | Green location icon + city |
| Bulb button | 272,4 – 42×42 circle | Frame: yellow on, grey off, red unreachable; tap = on/off, long press = Light screen |
| Clock | 6,88 – 182×66 | Hours neon green, the colon cyan and blinking every second, minutes white (56 px) |
| Weather column | left edge x = 184 (`COL_X`), top y = 95 (`COL_Y`); object 174,95 – 146×96 | A single left-aligned column right of the clock: at the top the greeting ("Good night Deniz", cyan), below it the weather icon (CYD 64 px, Pi 85 px) + the current temperature coloured by value (CYD 29 px, Pi 39 px `fontTemp`), below that ↑high (orange) ↓low (cyan) (CYD 12 px, Pi 21 px; the temperature sits 4 px closer to the icon on the CYD, 12 px on the Pi). The song row is always at the top of the column, so the greeting and the weather are always `NP_SHIFT` (record + 5 px) lower (since 3 October the playing-music layout is the default; nothing shifts when a song starts or ends); the icon reaches to the right of the date row. Tap for the Weather screen. |
| Date | 6,154 – 156×29 (`DATE_W`, up to 14 px left of the column) | Weekday yellow, date green (22 px), centred under the clock (clock width `clockW()`, pushed left or right if it does not fit). Fitting order: full weekday → short weekday (Monday → Mon) → short month (`rc::monthShortTr`, "Fri 28 Jul") → no weekday. 20 scenarios are drawn with `ui_preview.py --tarih` |
| Now playing | 184,95 – 124×17 (Pi 245,127 – 219×22) | At the top of the column: with music, a spinning record on the left (purple centre; CYD 17 px, Pi 22 px), the song title in small text on the right (CYD 12 px, Pi 16 px), a title that does not fit scrolls; without music a still record at 40% opacity and a grey "Nothing playing". Tap for the Spotify screen (details below) |
| Bars | 6,184 – 308×26 | Thermometer + temperature bar (within the day's min–max range), drop + humidity, leaf + air quality (green to purple by AQI) |
| Info card | 6,212 – 308×26 | Changes every 5 s: wind name (poyraz, lodos... the traditional Turkish wind names), 6-hour rain / "Take an umbrella", sunrise–sunset and day length, sea temperature, air quality, special day, latest earthquake; warnings (no connection, no SD) have a red frame |

Rumy's mood is chosen by `pickMood()` in order of priority: night sleep → earthquake surprise → **listening to
music** → eye sleep (if on, after the set time without a touch; off by default) → waking drowsiness (1.2 s) → sad if
there is no connection → special day (celebrating/calm + accessory) → looking up + umbrella if rain is approaching
(and it is not raining now) → **an expression for the current weather** (`rc::wxKind` + `rumy::weatherLook`, section
9.6) → normal. With the light on the expression does not change (removed on 30 September; eyes cut off from below
looked "clipped"); the eyes only take on a hint of the bulb's colour.

**"Now playing" strip** (`drawNowPlaying`, `npMeasure`, `npDrawSpan`, `updateNowPlaying`, `npTick`): at the top of the
column right of the clock. Until 2 October it was to the right of the date row; then it moved to the head of the
column.
- **When it shows:** while the bridge is connected, the session is active and there is a song title. While playing
  the record spins and the text is white. On pause the record stops, the text turns grey and the scrolling text goes
  back to the start. If it stays paused for 1 min (`MEDIA_PAUSED_HOME_MS`) or the bridge drops, the strip switches to
  the faded record and "Nothing playing" (until 3 October it was hidden and the greeting/weather moved up).
- **Record:** 17 px across on the CYD (25 px until 2 October), 22 px on the Pi; the drawing measurements are defined
  for 25 px and scaled with `DESIGN_PX`: a 1 px grey rim, a dark body, two thin groove rings, two opposing light-grey
  glint arcs, a purple (`C_NEON_PURPLE`) label, two opposing light-purple arcs on the label and a black hole in the
  middle. The glint and label arcs make the rotation visible. The speed is a real record's 33⅓ rpm (200°/s); the
  angle advances with elapsed time at 0.1° precision. The record is not drawn every frame: `tools/make_record.py`
  draws it 8× larger on the computer and shrinks it (soft edges) and embeds 24 rotation frames as RGB565 in
  `src/ui/record_frames.cpp` (the pattern repeats every 180°, so half a turn is 24 frames = 7.5°; 13.9 KB of flash on
  the CYD, 23.2 KB on the Pi). On the board each step draws a single image; if the frame number did not change the
  record area is not invalidated. The first version drew the record with 10 separate LVGL shapes (circle, ring, arc)
  and took ~3.5 ms per frame; as an image it dropped to ~1 ms.
- **Scrolling text:** small text (CYD 12 px, Pi 16 px), the window runs from 6 px right of the record to 12 px left
  of the screen's right edge (CYD ~93 px, Pi ~189 px; ~85 px in its old place). If the title fits it stays still. If
  not, it waits 1.5 s and flows left at 30 px per second; the same title follows 32 px behind (circular), and it
  waits 1.5 s again when it comes back to the start. A 10 px soft fade on the right edge, and on the left edge too
  while scrolling. It restarts on a new song. LVGL's ready-made scrolling label was not used: a single draw object
  takes very little room in the pool and the pauses can be set as wanted. When the song changes, a copy of the title
  is taken and each character's byte offset and x position are computed once (`npMeasure`); each frame LVGL gets
  only the part that falls in the window (~12 characters, `npDrawSpan`) rather than the whole title (up to 76
  characters), because LVGL scans all the characters several times on every text draw.
- **Layout:** the strip stays fixed at the head of the column at x = 184 (`NP_X = COL_X`). The greeting and the
  weather block are always `NP_SHIFT` lower, and the weather icon reaches to the right of the date row. So the date
  must end at x = 170: first the weekday is shortened (`rc::dayShortTr`), then the month (`rc::monthShortTr`, "Fri 28
  Jul").
- **Cost:** one object + one timer (33 ms). The timer only runs while the home screen is visible, the strip is shown
  and music is playing; each frame only the record area and (if scrolling) the text window are invalidated. While the
  strip is visible the date object is narrowed up to the left of the column. The timings below were measured on the
  CYD with the strip in its old place (on the date row). Draw time per frame measured on the board (home screen with
  music, 30 fps): face alone 10.8 ms, + spinning record 12.7 ms, + scrolling long title 14.5 ms (16.4 ms in the first
  version). ~180 B in the LVGL pool, ~580 B on the heap (the title copy and the character position tables). Tap to
  open the Spotify screen.

### 8.2 Weather screen (`scr_weather.cpp`)

A gradient background that changes colour with the weather, a 64 px icon, a rain chart for the next hour, a
horizontally scrolling 24-hour strip (sunrise/sunset included), 7-day min–max bars and a 10-tile details grid (feels
like, humidity, wind, pressure, UV, visibility, cloud cover, dew point, air quality, sea). The strip, the 7 days and
the grid are single-object custom drawings.

### 8.3 Light screen (`scr_light.cpp`)

Hue, brightness and white-temperature sliders, 8 preset scenes; commands go to the bulb task through a queue and are
not sent more often than every 80 ms. The 3 October revision: a bulb in the lamp's colour in the header (inside a
semi-transparent circle), an icon left of each slider (palette, sun, thermometer) and the value on the right; the
lamp's current colour right of the colour row. Knobs have a white ring and a coloured centre; the knob of the mode not
in use is a hollow ring (it used to be drawn at 50% opacity). The brightness fill is in the lamp's colour (a flat
colour; a gradient was tried and dropped because it raised LVGL's drawing peak by 11 KB). Scenes are 40 px high; an
icon in the lamp's colour for that scene on top (`fontM`), the name below (`fontS`); the selected scene is shown with
a frame in its own colour and an 18% background.

| Scene | Icon | Bulb command |
|---|---|---|
| Morning | sun | 6500 K, 100% |
| Work | laptop | 5000 K, 100% |
| Reading | book | 4000 K, 80% |
| Dinner | fork and knife | 3000 K, 80% |
| Evening | couch | 2700 K, 50% |
| Movie | film strip | WiZ scene 18 (TV), 40% |
| Candle | flame | WiZ scene 29 (candlelight), 50% |
| Night | moon | WiZ scene 14 (night light), 10% |

The selected scene is marked when the mode, brightness and temperature/scene number read from the bulb match
exactly.

### 8.4 Settings and sub-pages

`scr_settings` (location, bulb, WiFi reset, Language / Dil, Phone (`scr_phone`: QR + pairing code), earthquake
threshold, maintenance restart, diagnostics, about, factory reset), `scr_display` (brightness, return-to-home delay,
card duration, **eye sleep**, touch orientation; the orientation is reverted if not confirmed within 10 s),
`scr_nightcfg` (night mode: off / by the sun / by the clock, night brightness, start–end), `scr_location`
(81 provinces), `scr_pair` (bulb pairing), `scr_diag` (diagnostics), `scr_info` (about).

### 8.5 Night screen (`scr_night.cpp`)

Dim light, sleeping Rumy (animated "z"s), a small clock and date. Touching wakes it for 30 s. If music is playing
Rumy sleeps with its headphones on (the headphones are dimmed too). Default night hours **02:00–07:00**.

### 8.6 Celebration screen (`scr_event.cpp`)

Full screen on the first boot of a special day: a large Rumy (celebrating, or a calm expression + flag on
10 November), and the day's name.

### 8.7 Spotify screen (`scr_media.cpp`) and playlists (`scr_playlist.cpp`) — see sections 15 and 27.

### 8.8 Printer screen (`scr_printer.cpp`) — see section 25.

### 8.9 Match screen (`scr_match.cpp`) and match notifications (`match_ui.cpp`) — see section 26.

---

## 9. Rumy's face and animations

Source: `src/rumy/rumy.cpp` and `rumy.h`. The face is a single LVGL object; everything is drawn in the `DRAW_MAIN`
event. 320×88 on the home screen with 52 px eyes; larger on the night and celebration screens. Every measurement is
proportional to the eye size, so the same code works at every size.

### 9.1 Eye geometry and expressions

Each expression (`Mood`) is defined by a few parameters (`Params`): width `w`, height `h`, offset `dx/dy`, tired
(`tired`, a triangular cut from the top corner), angry (`angry`), happy (`happy`, a round cut from below: "^ ^"), flat
(`flat`, a straight cut from above), head position (`head`: lowers the eyes, headphones and notes together; only in
MUSIC, 0.12e ≈ 6 px). Parameters are kept at quarter-pixel precision (`Q = 4`) and approach their target smoothly by
one third each frame (`ease`).

| Expression | Look | When |
|---|---|---|
| NORMAL | Rounded square eyes | Default, partly cloudy |
| HAPPY | A deep cut from below "^ ^" + pink cheeks, hopping now and then | Sunny, snow |
| SLEEPY | Heavy lids; every 5.6 s the eyes close slowly and snap open (dozing) | 1.2 s while waking; sweltering heat |
| ASLEEP | A thin line, low | Night, eye sleep |
| SAD | Drooping lids, low; a tear every 4.2 s | No connection, rain |
| SURPRISED | Very large (1.15 × 1.36), grows even bigger for a moment on entry then settles, trembling | M4+ earthquake (30 s) |
| LOOKUP | Looking up | Rain approaching or drizzle (+ umbrella) |
| CELEBRATE | Happy, cheeks, frequent hops, colourful confetti | Special day (+ flag/party hat) |
| CALM | Flat top cut | 10 November (+ flag), overcast, clear night |
| **MUSIC** | Happily narrowed "^ ^", no blinking, no looking around | While music plays on the computer |
| SQUINT | Narrowed, inner corners down (`angry`) | Foggy, windy |
| SCARED | Large, worried, eyes close together (`pinch`) | Thunderstorm |
| SQUEEZE | `>  <` squeezed eyes (drawn with lines) | Cold |

A **glow** is drawn around each eye with two semi-transparent layers.

### 9.2 Movements

- **Blinking:** every 3–7 s, 150 ms; sometimes twice in a row.
- **Looking around:** a random gaze direction every 2.5–7.5 s (not when sad, sleepy, calm or listening to music).
- **Touch:** touching Rumy makes it look at the touched point and blink (`poke`).
- **Hopping:** in the happy and celebrating expressions.
- **Trembling:** 900 ms when surprised.
- **Frame rate:** normally 30 fps (33 ms), 20 fps (50 ms) while asleep. Redrawing only happens when the look changes:
  a summary of the look (`signature`) is computed every frame, and if it did not change nothing is drawn; if it did,
  only the rectangle covered by the face (`faceBox`) is invalidated.

### 9.3 Animated sleep "z"s (added in this session)

There used to be a fixed "Z z" text beside the right eye while asleep. Now `drawZs()`:
- 3 "z"s are born 1.2 s apart in a 3.6 s cycle.
- Each is drawn **with lines** (top stroke, diagonal, bottom stroke) so its size can grow smoothly (5→14 px).
- Path: diagonally up-right from the top corner of the right eye; sways ±3 px with a sine; fades in during the first
  15% and out during the last 38%.
- For a soft glow it is drawn thick and transparent first, then thin and bright.
- Only the small area of the z's is invalidated each frame; the rest of the screen is not drawn.
- With headphones on, the z's come out beside the earcup.

### 9.4 Headphones, notes and beat movement — see section 16.

### 9.5 Accessories (`src/rumy/accessories.*`)

Flag, party hat and umbrella, 32 px I4 palette images (generated by convert_assets). Accessories are not shown while
music plays (so they do not clash with the headphones).

### 9.6 Weather expressions and effects (1 October 2026)

Drafts were tried in the simulator; an eyebrow experiment was disliked and removed, and the stronger version was
chosen.

- **Classification (`rumy_core/wx_mood`, tested):** `wxKind(WxNow)` order of priority: thunderstorm (95–99) → snow
  (71–77, 85–86) → rain (61–67, 80–82) → drizzle (51–57) → fog (45, 48) → wind (gusts ≥50 or wind ≥35 km/h) → cold
  (feels like ≤2 °C) → sweltering (≥33 °C) → clear day ("sunny and hot" if ≥27 °C or UV ≥6) → clear night →
  overcast (3) → none. If there is no feels-like value, the temperature is used.
- **Look (`rumy::weatherLook`):** mood + accessory + effect for each kind; the table is in the README.
- **Effects (`rumy::Effect`, `rumy.cpp`):** particles are a pure function of time (`part(t)`); drawing and
  invalidation use the same function. Each frame only the small rectangle covering a particle's old and new position
  is invalidated (at most ~10 areas; LVGL's limit is 32). Extra effects tied to the mood: a tear when sad, confetti
  when celebrating.
- **Face movements:** dozing (`doze`), flinching in a thunderstorm (`wide/doze`), leaning in the wind and shivering
  in the cold (`swayX`), looking at a cloud when overcast and peering around in fog (gaze direction), an ice-blue eye
  colour in the cold.
- **The thunderstorm flash** refreshes the whole face strip twice in 4.8 s; the other effects are small areas.
- **Trying it:** serial command `hava <kind>` (2 min), `hava kapat`.
- **Simulator:** `hava_*` (12 kinds, home screen) and `rumy_ifadeler_tum_1..4` (13 moods, real size).

---

## 10. Network, data sources and cache

### 10.1 Network task (`src/net/net.cpp`)

It manages the WiFi connection (retrying every 20 s when it drops), starts NTP and fetches sources in turn with a job
scheduler. Each source has its own period and an **exponential back-off** after errors (30 s up to 15 min). Data
fetched before the clock is synchronised is marked "unsynchronised" and fetched again once the time arrives (an old
bug produced timestamps dated 1970).

| Source (name in `rapor`) | Server | Protocol | Period |
|---|---|---|---|
| Current + hourly + 7-day weather (`hava+7gun`) | api.open-meteo.com | HTTP | 15 min, one request |
| Sea temperature (`deniz`) | marine-api.open-meteo.com | HTTP | 60 min |
| Air quality (`havakalitesi`) | air-quality-api.open-meteo.com | HTTP | 60 min |
| Earthquakes (`afad`) | servisnet.afad.gov.tr | HTTPS | 10 min |
| Public holidays (`nager`) | date.nager.at | HTTPS | 24 hours |
| Match (`mac`) | site.api.espn.com, sports.core.api.espn.com | HTTP | 6 hours; 20 s during a match (section 26) |

(In the endurance build the periods drop to 30–60 s.) Since improvement 2, current/hourly and 7-day data come in one
request (`current` + `minutely_15` + `hourly` + `daily` at the same address); the `FORECAST` and `DAILY` domains in
store stayed separate, and the UI did not change. Although 7-day data now arrives every 15 min, it is not written to
the SD card every time: the cache layer (`cache::save`) skips the write if nothing but the timestamp changed, and
refreshes it once an hour even if nothing changed (`REWRITE_AFTER_S`). First fetches start 3 / 30 / 45 / 60 / 90 s
after WiFi connects; on errors each source backs off exponentially from 30 s to 15 min.

### 10.2 Small HTTP client (`src/net/http_min.*`)

A memory-friendly client written instead of Arduino's `HTTPClient`: 6 s connect and 10 s read timeouts, `chunked`
transfer decoding, and it presents the response body as a stream (`ByteSource`); JSON is parsed straight from the
stream and the full body is never held in RAM. Error codes: connection, timeout, protocol, TLS memory, parsing, etc.

### 10.3 HTTPS and certificates

`tools/gen_ca_bundle.py` picks only the root certificates AFAD and Nager need from the Mozilla store (certifi) and
writes them to `src/net/ca_bundle.h`. "GlobalSign Root CA" is no longer in that store, so it is downloaded from
GlobalSign's official address and verified by its SHA-256 fingerprint. mbedTLS uses 16 KB in/out buffers for TLS; the
handshake needs ~60 KB of momentary memory, so there is a memory threshold check (section 6.1). While the phone app is
in use (until 20 s after the last request, `webui::busy()`), HTTPS requests are postponed.

### 10.4 JSON parsing (`lib/rumy_core/src/openmeteo.cpp`, `afad.cpp`, `nager.cpp`)

**Filtered** parsing with ArduinoJson 7 on a fixed 8 KB arena (`JsonArena`): only the fields needed are kept in
memory. Results are converted into small fixed-size structures (`rc::Forecast`, `rc::Daily`, `rc::QuakeInfo`...).
This code does not depend on Arduino and is tested on the computer with real API responses.

The combined weather response is parsed as one document with `omParseWeather`. The filter and reading code is shared
with the partial parsers (`omParseForecast`, `omParseDaily`); a test verifies that both produce byte-for-byte the same
structure from the same response. Filter keys are passed as static strings (`JsonString(k, true)`) so they are not
copied into the arena. The arena's highest use since boot appears on the `[ag]` (network) line of `rapor`
(`json tepe`, JSON peak); all parsing in the network task (weather, AFAD, Nager, printer) uses the same 8 KB in turn.

### 10.5 Cache (`src/net/cache.*`, `lib/rumy_core/src/cache_fmt.*`)

The latest successful weather and earthquake data are written under `/rumy/cache` on the SD card in a versioned,
checksummed binary format; they are read back at boot so the screen is full before WiFi comes up. Nothing is written
to the cache while the clock is unsynchronised.

---

## 11. Special days, earthquakes, clock

- **Clock** (`src/clock/clock.cpp`): `configTzTime("<+03>-3", "pool.ntp.org", "time.google.com",
  "tr.pool.ntp.org")`; Turkey is UTC+3 with no daylight saving time.
- **Special days** (`lib/rumy_core/src/events_calc.cpp`, `src/events/events.cpp`):
  - National holidays and remembrance days from a fixed table.
  - Eid al-Fitr and Eid al-Adha: from Nager.Date; if Nager's dates are marked "provisional", the embedded Diyanet
    table is preferred.
  - Personal days (birthday, anniversary) are added from the phone app and kept on the SD card in
    `/rumy/events.json`; if a year is entered they show as "age 34", "year 10" and so on.
  - Rumy celebrates today's events (celebrating expression + accessory), and a full-screen card appears on first
    boot.
- **Earthquakes** (`src/quake/quake.cpp`, `afad.cpp`, `kandilli.cpp`): the last 2 days of earthquakes in the Marmara
  region from AFAD; the threshold is adjustable (M2.0 by default). A new M4+ earthquake stays pinned on the card for
  1 hour, Rumy is surprised for 30 s and wakes up if asleep. Source order (27 October, section 10.6): if the bridge is
  connected the computer fetches AFAD; otherwise, if memory allows, the board fetches it over HTTPS; failing that,
  Kandilli Observatory's plain-HTTP list is used.

### 10.6 HTTPS through the bridge and the Kandilli fallback (`src/net/relay.*`, `tools/veri_koprusu.py`)

**Problem (measured on 27 October):** static RAM grew from 60.8 to 89.2 KB since September (match, league, Spotify
playlists, agent light, wireless update, phone app); free heap after boot is ~36–40 KB, at most ~46 KB over a long run.
HTTPS needs ≥ 64 KB free + a ≥ 34 KB block (the handshake peak had been measured at 47–58 KB). Result: during one
38-hour boot AFAD and Nager were each postponed 4 598 times and never fetched. Measured shares (12 samples each): web
server + mDNS ~4.5 KB, the idle printer connection too small to measure.

**Fix (bridge + Kandilli):**
- Rumy says `@veri afad <path>` (the same AFAD path the board builds) or `@veri nager <year>`. The bridge
  (`tools/veri_koprusu.py`) fetches it over HTTPS on the computer, summarises it with the same rules as the board's
  parsers and returns a single line: `@deprem <24 h count> <latest: time magnitude×10 depth×10 lat×1e4 lon×1e4>
  <largest: ...>\t<place>\t<place>` or `@bayram <year> <eid al-fitr month day> <eid al-adha month day>
  <provisional 0|1>`. The board waits 12 s (`cfg::RELAY_TIMEOUT_MS`); the network task prepares the request, the loop
  task sends it (`relay::service`), and the reply arrives through `media::lineEnd` → `relay::line`. Decoding is in
  `rc::relayQuakeLine` / `rc::relayHolidayLine`.
- If there is no bridge or it does not answer: direct HTTPS as before if memory allows; if not, for earthquakes
  `http://www.koeri.boun.edu.tr/scripts/lst0.asp` (windows-1254, the last 500 earthquakes in fixed columns, Turkey
  time). `rc::kandilliParse` reads it line by line, applies the same box, threshold and 2-day window as AFAD, and stops
  when it leaves the window (~25 KB of the page is read). Magnitude is Mw, else ML, else MD. Place names
  "VILLAGE-DISTRICT (PROVINCE)" → "District (Province)". `QuakeInfo.source` records whether the data came from AFAD or
  Kandilli. Nager's fallback is the embedded Diyanet table.
- Closing the printer connection for HTTPS jobs (`printer::holdForTls`) now happens inside `net::get`, only when HTTPS
  is actually attempted. The `tls` column of the job table is gone; each source's last path shows in `rapor` (`http`,
  `https`, `kopru` (bridge), `kandilli`), and the `[veri]` (data) line counts bridge requests.
- Measurement (on the board, boot #170): with the bridge, AFAD and Nager 200 (bridge reply 275 ms); with the bridge
  off, the AFAD job got 200 from Kandilli, 129 ms, 25 024 B. The real Kandilli page was cross-checked with Python on
  the computer (same latest and largest earthquake).

---

## 12. WiZ smart bulb

`src/wiz/wiz.cpp` + `lib/rumy_core/src/wiz_proto.cpp`, `pairing.cpp`:

- WiZ bulbs talk JSON commands on the local network on **UDP 38899** (`getPilot`, `setPilot`, `getSystemConfig`...).
  Rumy never connects to the cloud service.
- **Discovery:** bulbs are found by broadcast and candidates are listed; during pairing the chosen bulb blinks once.
  The pairing is kept by **MAC address**; even if the IP changes it is found by rediscovery every 15 s.
- The state is read every 5 s (on/off, colour, brightness); commands are at least 80 ms apart, and the last command
  is verified after 1 s of silence.
- With the bulb on, the eye colour blends slightly towards the bulb's colour (`rc::pilotColor`, `lv_color_mix`).
- `tools/fake_bulb.py`: a fake WiZ bulb for testing without a real one (same UDP protocol).
- The starting point was a small trial sketch that controlled the WiZ bulb from the CYD.

---

## 13. Settings, first setup and the phone app

### 13.1 Settings and NVS (`src/settings/settings.*`)

The `Settings` structure is stored in NVS (`Preferences`) under individual keys:

| Key | Field | Meaning |
|---|---|---|
| `name`, `city` | name, city | Shown in the greeting and the location |
| `lat`, `lon`, `prov` | coordinates, province code | For the weather and the region |
| `bri`, `nbri` | brightness, night brightness | % |
| `nmode`, `nstart`, `nend` | night mode, start, end (minutes) | Default by the clock, 02:00–07:00 |
| `home`, `card` | return to home (s), card duration (s) | 60 / 5 |
| `maint` | 04:00 maintenance restart | On/off |
| `trot` | touch orientation | 0–7, 6 on ILI9341 |
| `wifi` | WiFi set up | First setup |
| `qmag` | earthquake threshold ×10 | 20 = M2.0 |
| `esleep` | eye sleep (min) | 0 = off (default since 3 October) |
| `bmac`, `bname` | bulb MAC and name | Pairing |
| `team` | team (ESPN no.) | 0 = no team (default); e.g. Fenerbahçe 436 |
| `fut` | match tracking on | On by default; while off no match is queried |
| `tc1`, `tc2` | the team's two colours (0xRRGGBB) | 0 = default navy-and-yellow palette |
| `wagent`, `wgoal`, `wonly` | bulb: agent alert, goal celebration, only while the bulb is on | All three on by default |
| `gsl`, `gucl` | Süper Lig / Champions League goal notifications | On by default |
| `uyku` | end of "Sleep now" (UTC s) | 0 = none; survives a restart |
| `dil` | interface language | 1 = English (default for new installs), 0 = Turkish (section 31) |
| `boots` | boot counter | Diagnostics |

Paired phones' tokens are kept in a separate `telefon` (phone) namespace (`t0`–`t3`, `next`).

Values read are clamped to their limits with `sane()` (against corrupted NVS). Changes bump the `store::SETTINGS`
version and reach the UI task as `M_SETTINGS`.

### 13.2 First setup (`src/app/portal.cpp`)

If WiFi is not set up, LVGL is not started; a plain screen drawn with LovyanGFX (an embedded VLW font, a QR code) and
the **Rumy-Kurulum** access point with WiFiManager open. The page has WiFi selection and **name** and **province**
fields (a province name or plate code is accepted). On save Rumy restarts. If the connection fails, the reason (wrong
password, network not found...) is caught and shown to the user.

### 13.3 Phone app (`src/webui/webui.cpp`, `src/ui/scr_phone.cpp`, `web/index.html`)

Rumy runs a small HTTP server on port 80 (always on while WiFi is connected, with the mDNS name `rumy.local`). The
single page it serves becomes a full-screen app on the iPhone with **Add to Home Screen**
(`apple-mobile-web-app-capable`, a 180 px `/icon.png` icon). Use and screens are in README section 7; design and
measurements in section 30.

- **Pairing:** while Settings → Phone is open on Rumy a 4-digit code is generated (`webui::setPairing`). The phone
  sends this code with `POST /api/pair {"code"}`; if correct, a random 128-bit token (32 hex) is generated, written
  to NVS and returned to the phone (at most 4 phones, a fifth replaces the oldest). The phone keeps the token in its
  own storage and sends it with every request in an `X-Token` header. When the screen closes the code becomes
  invalid; the code is renewed after every successful pairing. A 1 min lock after 5 wrong codes or tokens.
- **API:** `GET /`, `GET /icon.png`, `GET /api/provinces`, `POST /api/pair` (no token); `GET /api/state`,
  `POST /api/settings` (only the fields sent change), `POST /api/events`, `POST /api/sleep {"on"}`,
  `POST /api/refresh`, `POST /api/reboot`, `POST /api/ping`. Body at most 3 KB.
- **Sleep now:** `settings::setSleepUntil()` writes a sleep until the next time the night end hour comes round
  (`rc::sleepUntilUtc`, `lib/rumy_core/src/phone_app.h`); the UI task's `computeNight()` forces night during that time.
  Touch and printer errors wake it briefly as in night mode; `{"on":false}` ends it.
- The page is a single HTML file; before the build `tools/build_web.py` gzips it into `web_assets.h` and embeds the
  icon in `web_icon.h`.

---

## 14. SD card

`src/board/sdcard.cpp`: 20 MHz on VSPI (4 MHz if that fails). FATFS allocates a 4 KB buffer for every open file, so
`max_files = 1` (SD access is serialised with a single lock; part of the memory bottleneck fix). Use: `/rumy/cache`
(latest data), `/rumy/holidays` (holiday tables), `/rumy/events.json` (personal days). The `sdtest` command tests the
card by writing and reading 50 times. Without a card everything works; only personal days are not saved and a warning
shows on the info card.

---

## 15. Spotify screen and the PC bridge

### 15.1 Research and the chosen approach

Open-source projects were reviewed:
- [Spotify-Diy-Thing](https://github.com/witnessmenow/Spotify-Diy-Thing) (MIT), [llamafi](https://github.com/bvarbanov90/llamafi),
  [CarDisplay-OS](https://github.com/ServetAKC/CarDisplay-OS): the board connects **directly to the Spotify Web API**
  (OAuth, developer account, HTTPS, JPEG decoding).
- [NowPLayingDisplay](https://github.com/RodgerE1/NowPLayingDisplay): the computer reads the playing song from the
  Windows media session and sends it to the ESP32.

Since the ~60 KB of memory HTTPS needs was already borderline for weather/earthquakes on Rumy, and the board is used
connected to a computer, the **second approach** was chosen: a Python bridge on the computer sends the song info and
cover over the **USB serial line**. No Spotify account or API key is needed; any player that reports to Windows
(browser, VLC... with `--any`) works.

### 15.2 PC bridge (`tools/spotify_bridge.py`)

- **Song info:** `GlobalSystemMediaTransportControlsSessionManager` is opened with `winrt`; the Spotify session (or the
  current session with `--any`) is selected. Title, artist, album, playback state, position and duration are read. The
  position is corrected by adding the time elapsed since Spotify's last update.
- **Cover:** the session's thumbnail (a 300×300 PNG in Spotify) is read → cropped square from the centre → shrunk to
  144×144 with LANCZOS → corners rounded by 12 px and masked to black → converted to raw **RGB565 little-endian** data
  (41 472 B). Its ID is the CRC32 of the raw image.
- **Accent colour:** the cover is shrunk to 24×24, colours are roughly grouped, the best colour is picked by
  saturation × brightness × frequency, and its saturation is raised to ≥ 0.65 and brightness to ≥ 0.9 so it looks
  neon. The progress bar, play button, artist name and the notes of Rumy with headphones are painted in this colour.
- **Text clean-up:** characters not in the board's font are stripped of accents with Unicode NFKD or dropped;
  tabs/newlines become spaces; fields are shortened with "…" to their byte limits.
- **Controls:** `@cmd play/next/prev` commands from Rumy become SMTC's `try_toggle_play_pause_async`,
  `try_skip_next_async`, `try_skip_previous_async` calls; `volup/voldown` become the Windows volume keys
  (`keybd_event`, 2 steps per press).
- **Connection:** the CH340 port is found automatically (or `--port`); DTR/RTS are kept low while opening the port so
  the board does not reset. When disconnected it reconnects every 2 s. `--log` also prints Rumy's log, `--send
  "command"` sends a serial command to the board on connect, `--autostart ac/kapat` adds/removes a console-less
  (pythonw) launcher in the Windows startup folder.

### 15.3 Serial protocol

Rumy's log lines also go over the same line; protocol lines start with `@` and are kept apart from the log.

| Direction | Message | Meaning |
|---|---|---|
| PC → Rumy | `@np⇥playing⇥position_ms⇥duration_ms⇥cover_hash⇥colour⇥app⇥title⇥artist⇥album` | Playback state every second (or on change); tab-separated |
| Rumy → PC | `@need <hash>` | This cover is not in the cache (may be repeated every 5 s) |
| PC → Rumy | `@art <hash> <w> <h> <bytes> <crc32>` | Cover transfer starts |
| Rumy → PC | `@go <hash>` | The first sector is erased, data may be sent (with the ID since 3 October; the bridge drops a `@go` without it) |
| PC → Rumy | A raw 2048-byte chunk | Cover data |
| Rumy → PC | `@ok <received>` | Chunk written, send the next (the bridge only accepts the `<received>` it expects) |
| Rumy → PC | `@art ok <hash>` / `@art err <reason>` | Transfer finished / error (crc, timeout, space...) |
| Rumy → PC | `@cmd play\|next\|prev\|volup\|voldown` | Touch buttons |
| Rumy → PC | `@cmd seek <ms>` | The progress bar was dragged; the bridge seeks with SMTC `try_change_playback_position_async` and trusts the target position for 3 s |
| PC → Rumy | `@tempo <beat_ms> <ms_to_next_beat>` | Beat info (section 16); `0 0` = unknown |
| PC → Rumy | `@vol <0-100>` / `@vol -` | Spotify's volume level (on change and every second); `-` = no Spotify audio session |
| Rumy ↔ PC | `@cmd pls`, `@pls`, `@pl`, `@plnow`, `@cmd plplay`, `@plst`, `@cmd plmix` | Playlists (section 27.3) |
| PC → Rumy | `@ajanlar <version> <rows> <working> <waiting> <idle>` + one `@ajan⇥order⇥id⇥c\|b\|i⇥state_s⇥job_s⇥project⇥detail` per row | Claude Code sessions (section 28) |
| Rumy → PC | `@foto <hash> <px> <player name>` | Player photo for the goal screen; the bridge sends a px×px square with `@art` or says `@foto yok <hash>` (none) |
| PC → Rumy / Rumy → PC | `@ping` / `@pong` | Test |

If Rumy receives no `@np` for 6 s it assumes the bridge is gone. If a cover or camera transfer ends with an error,
Rumy ignores incoming bytes until the line has been quiet for 300 ms (at most 2 s) (`rapor` → `[medya] … atilan N B`,
"dropped N B"); so pieces of half-finished data are not read as command lines.

### 15.4 Receiving on the board and the cover cache (`src/media/media.cpp`)

- Serial reading is in `diag::readSerial()`: if a line starts with `@`, it is routed to the media module (up to 320
  bytes); during a cover transfer incoming bytes go straight to the media module.
- **Covers are kept in flash, not RAM:** the `art` partition is divided into 21 slots (0xB000 = 44 KB each). Each
  slot starts with a 32-byte header (magic `RART`, ID, sequence number, width, height, CRC32, size), followed by
  RGB565 pixel data.
- At boot all headers are read; valid ones go into a table. The whole partition is **mapped into memory** with
  `esp_partition_mmap`; LVGL draws the cover straight from its flash address (just like the embedded icons), using no
  RAM.
- For a new cover the **oldest slot** (lowest sequence number, except the one on screen) is chosen; this spreads flash
  wear across slots.
- The data is written first and **the header last**: if the transfer stops halfway, the slot counts as invalid. The
  CRC32 is computed with `esp_rom_crc32_le` on the board and matches Python's `zlib.crc32` exactly (verified).
- **Important lesson:** in the first attempt covers never arrived. Reason: during a flash erase/write the CPU cache is
  disabled and the UART interrupt, which is not in IRAM, cannot run; at 460 800 baud the hardware FIFO (128 bytes)
  fills in ~3 ms and data is lost. The fix is **flow control**: the board does not start reading until the whole 2 KB
  chunk has collected in the serial buffer; it writes the chunk to flash, erases the next chunk's sector and **only
  then** sends `@ok`. Since the computer sends no new data before `@ok`, flash operations happen while the line is
  idle.
- The serial speed was raised from 115 200 to **460 800 baud** and the receive buffer grown to 2 304 bytes. Cover
  transfer measurement: ~77 ms per chunk on the board, **1.2–1.8 s** in total; instant when the same album comes again.
  22+ covers were transferred without errors.
- Second lesson: in the first version the bridge's read loop used `read(256)` with a 0.2 s timeout, so every `@ok`
  was 0.2 s late (5.8 s); `read(in_waiting or 1)` brought it down to 1.2 s.

### 15.5 Spotify screen (`src/ui/scr_media.cpp`)

| Area | Content |
|---|---|
| 10,10 – 144×144 | Cover (if none, a dark box + a note symbol in the accent colour) |
| Right column, top | Spotify logo (green) + "Spotify" ("· paused" when paused) |
| Right column | Song title 22 px, at most 2 lines, "…" if it does not fit; artist 16 px in the accent colour; album 12 px, 2 lines |
| 10,164 | Progress bar (accent colour), elapsed and total time below; the position is estimated on the board every second |
| Bottom row | 5 round buttons: volume −, previous, **play/pause** (filled with the accent colour, 46 px), next, volume + |
| No bridge | "Computer not connected / spotify_bridge.py" |
| No music | "No music playing / Play a song in Spotify" |

Pressing play/pause updates the screen **immediately** (optimistic update); the real state arrives with the next
`@np`.

**The volume button problem (fixed in this session):** while play/next worked, the volume buttons at the edges of the
screen did not. Coordinates collected with `dokunma ac` showed the touches landing around the buttons; the buttons'
touch area was widened from 4 px to 10 px and the problem was solved (touches at x 275–297 for volume + and 31–47 for
volume − turned into commands).

---

## 16. Rumy with headphones and beat tracking

### 16.1 Drawing on the board (`rumy.cpp`: `phones`, `drawPhones`, `band`, `cup`)

While music plays (bridge connected + session active + playing) Rumy on the home screen switches to the **MUSIC**
expression and puts on headphones. Every measurement is proportional to the eye size (`e`); e = 52 px on the home
screen:

- **Earcups:** 3 px beyond the eyes' outer edges, width 0.22e (11 px), height 0.58e (30 px), capsule-shaped. A dark
  **cushion** strip on the side facing the eye, a thin white **highlight** on the outside, a two-layer glow around.
- **Band:** two quarter-circle arcs (`lv_draw_arc`) + a straight line; it passes 5+ px above the eyes' glow. It is
  drawn thick and transparent first (glow, flat ends), then thin and opaque (round ends). The flat-ended glow keeps
  transparency from overlapping into bright dots where the arcs meet the line (noticed and fixed at 4× zoom).
- **Colour:** neon pink `0xFF4FA3` (contrasting with the cyan eyes, matching the neon theme). A dim shade on the night
  screen.
- **Notes (♪):** rise upward from outside the right earcup; head (ellipse), stem and flag are drawn with lines, size
  grows 5→8 px, sways ±2 px, fades in and out; 2 notes in a 2.4 s cycle. The colour is the album cover's accent
  colour. The spot was chosen on purpose: the top right of the screen has the bulb button and the top left the city
  text; the only free column is between the right earcup and the bulb button.
- **The head goes down:** in the MUSIC expression the whole head (eyes, headphones, notes) moves ~6 px down
  (`Params::head`); the headphone band does not stick to the top edge of the screen. The transition is smooth like the
  other parameters (~0.3 s). No change on the night screen (sleeping Rumy).
- While music plays **eye sleep does not kick in** (Rumy is listening); in night mode it sleeps with the headphones
  on, and the "z"s come out beside the earcup. Accessories are hidden.

### 16.2 Beat movement (`groove`)

A beat period P and a beat moment (anchor) are known. Each frame the phase p (0–1) within the beat and the beat count
k are computed:
- **Swaying side to side:** `swayX = A_s · sin(π · (k + p))` → one full sway every two beats (A_s ≈ 0.06e = 3 px).
- **Nodding:** a fast drop starting a little before the beat (smoothstep, 20% of the period) and a slow return
  (quadratic decay): `bobY = A_b · g(p)` (A_b ≈ 0.08e = 4 px). The peak is ~10% of a period after the beat.
- The eyes **and** the headphones move together (a shared "head" position).
- Without tempo info a slow, generic 1.1 s sway is used.

**Phase locking on the board (`setTempo`):** a tempo arrives from the bridge every second. If the new tempo is within
4% of the old one and the beat moment differs by less than 90 ms, the anchor is shifted by a third of the difference
and the period is averaged 3:1 (a soft PLL); otherwise it jumps straight to the new value. All maths is done with
integers and `lv_trigo_sin`.

### 16.3 Beat tracking (bridge: `BeatTracker`, `Loopback`, `start_beat_tracker`)

1. **Audio source:** `pyaudiowpatch` opens a WASAPI loopback stream for **every** audio output on Windows. Every
   second, the loudest device among those that delivered data in the last 0.5 s above a threshold level is chosen;
   the source does not change unless the playing device stays silent for 2 s (so a system sound does not steal it).
   This was added for a setup where sound is listened to through **Bluetooth headphones** instead of the default
   output (monitor); the first version only listened to the default device and got no data at all (Windows sends no
   data to a silent device's loopback).
2. **Onset strength:** steps of 1024 samples at 48 kHz (21.3 ms), a 2048-sample Hann-windowed FFT; the positive
   difference of log-compressed magnitudes (spectral flux). Only **below 3 kHz** is used, and below 200 Hz
   (drums/bass) is weighted 4×. The first version used all bands and locked onto hi-hats playing on the half-beat
   (phase off by 240 ms); limiting the band fixed it.
3. **Tempo:** the onset envelope of the last 8 s is detrended from its local mean, normalised and its
   **autocorrelation** computed. Among lags for 60–200 BPM, the peak weighted by a log-Gaussian prior centred on
   120 BPM is chosen, with sub-sample precision from parabolic interpolation. The autocorrelation accumulates across
   measurements with an exponential average (0.75/0.25); a single measurement cannot swing the tempo.
4. **Octave correction:** if the tempo found is below 80 BPM and double it is also strong (≥ 50%), double is chosen;
   above 170 and half is strong, half is chosen (this fixed a 65 ↔ 129 BPM mix-up on a real song).
5. **Phase:** a comb filter over the last 4 s: for every possible shift the onsets at the beat points are summed,
   and the largest gives the last beat moment. Using the last 4 s keeps small tempo errors from accumulating.
6. **Stability:** if the confidence (autocorrelation peak) is below 0.12 the measurement does not count; the last
   tempo is kept for up to 8 consecutive failed measurements; a value more than 6% off the current tempo is accepted
   only if it comes 3 times in a row.
7. **Latency margin:** sound reaches the ear ~50 ms late from a speaker and **~150–200 ms late from Bluetooth
   headphones**. At start-up the bridge gets Bluetooth device names with PowerShell
   (`Get-PnpDevice -Class Bluetooth`); if the name of the output being listened to contains a Bluetooth device, the
   margin is 200 ms, otherwise 50 ms (set by hand with `--beat-offset`). PowerShell output is forced to UTF-8 and
   decoded fault-tolerantly (a device name with Turkish characters crashed the first version).
8. **Sending:** on every heartbeat (once a second) the bridge sends `@tempo <period_ms> <ms_to_next_beat>`; when music
   stops, `@tempo 0 0`.

**Verification:**
- Synthetic songs generated at known tempos (drums + off-beat hi-hat + noise + tone): 124/96/140 BPM → estimated
  123.0/96.7/140.4 BPM, beat timing error +9/−16/−17 ms.
- On real music (Spotify, Bluetooth headphones): an estimate that wandered between 86/129/65 in the previous version
  held steady at 129 BPM for 34 s in the final version; 105 BPM on another song.

**Important lesson (COM):** in the first version the bridge's main loop hung completely: `PyAudio()` initialised
Windows COM single-threaded (STA) on the main thread, which blocked the callbacks of `winrt`'s asynchronous calls.
Moving the audio library's initialisation entirely onto its own thread fixed it.

---

## 17. Diagnostics, serial commands, measurement tools

Serial line 460 800 baud. Commands (`src/diag/diag.cpp`):

| Command | Job |
|---|---|
| `rapor` / `mem` | Version, uptime, reset reason, memory (free, lowest, largest block, trend), LVGL pool (including the peak and the number of screens not opened for lack of memory), task stacks, WiFi/RSSI/clock, TLS peak, number of postponed HTTPS requests, JSON arena peak (`json tepe`), each data source's (`hava+7gun`, `deniz`, `havakalitesi`, `afad`, `nager`) last code/duration/bytes/errors and time to the next fetch, SD, **media** (bridge, cover slots, transfer/error counts), **camera** (frames, errors, frame time), bulb, **printer** (connection, name, state, progress, time left, error code, message/connection/parse counters) |
| `yenile` | Fetch all data now |
| `web ac` / `web kapat` | The phone app's server (on at boot; `kapat` until restart) |
| `telefon unut` | Forgets all paired phones |
| `sdtest` | Write/read the SD card 50 times |
| `dokunmatik 0-7` | Change and save the touch orientation |
| `dokunma ac` / `dokunma kapat` | Print the coordinate of every touch |
| `bench`, `bench gecis`, `bench kaydir` | The board does transitions and scrolling by itself and measures each step's frames, longest frame and fps |
| `bench bellek` | Visits every screen and sub-page; at each step it resets the LVGL pool peak and prints use, peak, largest free block and fragmentation; at the end it summarises the highest peak. Automatic screen changes (night, return home, Spotify) pause during the run |
| `lvgl yuk <KB>` | Holds memory from the LVGL pool in 1 KB pieces (`lvgl yuk 0` releases it); to test the "out of memory" fallback |
| `perf ac` / `perf kapat` | A drawing measurement every second while navigating |
| `profil ac` / `profil` | (cyd_prof) Timing table of LVGL's internal functions |
| `spitest <MHz>` | Tests SPI speed by writing a pattern to the screen and reading it back |
| `kayma ters` / `kayma duz` | Hardware scroll direction |
| `gece 02:00 07:00` | Set night mode by the clock and save |
| `yazici <ip>` / `yazici kapat` | Save the Creality K2 printer's IP address to NVS (section 25) or turn the feature off |
| `ekran ana` / `ekran spotify` / `ekran yazici` | Switch to that page (for testing without touching the board) |
| `ekran goruntusu` | Reads the panel back line by line and sends it as `@shot` / `@px <y> <x> <base64>` / `@shotend` lines; the bridge saves it as `tools/.cache/rumy_ekran.png` |
| `wiz durum`, `wiz tara`, `wiz esle <mac>`, `wiz unut` | Bulb pairing/online/IP/light state and command permission; network scan and the list of candidates; pairing; forgetting the pairing |
| `wiz ac`, `wiz kapat`, `wiz parlak <10-100>`, `wiz sicak <K>`, `wiz renk <0-359>`, `wiz sahne <no>` | Controlling the bulb without touching (for remote testing on the Pi) |
| `reboot`, `wifi-sifirla` | Restart, reset WiFi |

While the bridge is running the serial port is busy, so the bridge forwards lines written to
`tools/.cache/rumy_komut.txt` (e.g. `rapor`, `ekran goruntusu`) to the board and deletes the file; with `--log` the
board's reply lands in the bridge's output.

A one-line summary (`diag up=... heap=... lvgl=... fps=... tls=...`) is also written every 5 min, and a step-by-step
free heap line at boot. The Diagnostics screen shows the same information on the device.

---

## 18. Performance work

When v1.0.2 was measured on the board, page transitions and scrolling on the weather screen stuttered. With `bench`
measurements:

| Step | v1.0.2 | v1.1.0 |
|---|---|---|
| Page transition (220 ms) | 3 frames, 5–7 fps, frame 100–190 ms | 18–30 frames, longest frame 20–35 ms |
| Opening the weather screen | 293–304 ms | 9–10 ms |
| Scrolling the weather screen | 5–10 fps | 24–25 fps |
| Hourly strip | 15 fps | 27–30 fps |
| Scrolling Settings | 13 fps | 30–32 fps (SPI limit) |

| Change | Effect |
|---|---|
| Hardware-scrolled transition (section 7.4) | Frames became 5–8× cheaper |
| Flash QIO 80 MHz (was DIO 40) | Faster font/icon reads, scrolling ~2× |
| Icons embedded in flash | 300 ms → 10 ms to open, 20 KB RAM freed |
| `-O2` (was `-Os`), `build_unflags = -Os` | Drawing 20–25% faster (+137 KB flash) |
| Draw buffers 16 → 32 lines | Chunks per frame halved |
| LVGL pixel functions in IRAM, corner mask cache | A few ms |
| Touch SPI 250 kHz → 1 MHz | Shorter read latency while scrolling |
| SPI 80 MHz tried | `spitest 80`: 3–5% of pixels corrupted → stayed at 40 MHz |

The animations in this session were also designed with performance in mind: the sleep "z"s run at 20 fps over a
small area only; in headphone mode the face runs at 30 fps but only the area of the face and the note column is
redrawn.

**Measuring and slimming the "now playing" strip** (`perf ac`, home screen with music, daytime; night mode was moved
temporarily for the measurement and restored). To isolate the strip's share, a temporary build with the strip
animation off was measured too:

| State | Draw time per frame (avg.) |
|---|---|
| Face alone (headphones, nodding, notes) | 10.8 ms |
| + record, first version drawn with 10 LVGL shapes | 14.3 ms |
| + record and scrolling long title, first version | 16.4 ms |
| + record, a single image from 24 pre-made frames | 11.9–12.7 ms |
| + record and scrolling long title (image record, drawing only the visible part, narrowed date object) | 14.5 ms |

The frame rate stayed at 30 fps in every case. Drawing the record as an image gave the biggest gain (~2.5 ms/frame).
The effect of drawing only the visible characters of the scrolling text stayed within measurement noise in normal
operation (±1 ms); in slides to the home screen while music plays, the longest frame went from 23–32 ms to 22 ms (the
slide draws the screen in vertical column strips, so the strip is drawn several times per transition). A slide to the
home screen takes 20 frames with music and 22–23 without (the face with headphones and the strip make the frame a
little heavier; the 200 ms duration does not change).

---

## 19. Testing and verification

### 19.1 Unit tests (`test/`, `pio test -e native`)

Logic that does not depend on Arduino (`lib/rumy_core`) is tested on the computer with Unity: **19 suites, 168 tests**
(27 October), all passing. Added later to the table below: `test_espn` (fixtures, match events, league day,
standings), `test_livescore` (fast goals), `test_agents` (Claude sessions, section 28.5), `test_fwimage` (wireless
update offer, image header, SHA file, separating image body/summary; 9 tests, section 29), `test_phone` (phone app:
sleep end, pairing token and code; 7 tests, section 30), `test_kandilli` (Kandilli line and page, place names, bridge
lines; 9 tests, section 10.6). With the language support (section 31), `test_text` and `test_events` gained English
checks.

| Suite | Topic |
|---|---|
| test_openmeteo | The combined weather request's address (fits the buffer even at extreme coordinates), parsing a real combined response, combined and partial parsers producing the same structure, missing/half responses, a small arena; partial parsing of current/hourly/7-day/air quality/marine (real response samples; 10 tests). The test output prints the arena peak |
| test_afad | Earthquake list, region and threshold filter |
| test_nager | Holiday parsing, preferring the Diyanet table for provisional dates |
| test_events | Special day calculation, age/anniversary (Turkish and English) |
| test_cache | Cache format, version and checksum |
| test_color | Colour conversions, bulb colour |
| test_pairing | Bulb pairing logic |
| test_text | Texts (weekday/month names, durations, greetings, AQI levels) in Turkish and English |
| test_wind | Wind names (poyraz, lodos... and compass directions in English) |
| test_wiz | WiZ protocol messages |
| test_wxmood | Weather kind classification: precipitation codes, precipitation taking priority over wind/temperature, wind and gust thresholds, temperature when there is no feels-like value, clear day/night, hot day (feels-like or UV), overcast and unknown codes, names (7 tests) |
| test_media | Bridge protocol (`media_proto`): the `np` line (full, short, with empty fields, with Turkish letters), song summary, `art` cover offer and slot header validation (144 and 192 px), position estimate and wrap detection, the `tempo` line, beat lock (first lock, smoothing on small errors, jumping on big changes, counter overflow; 12 tests) |
| test_k2 | Creality K2 status messages: a real full message taken from the printer, partial updates, print/error/filament out/heartbeat samples, job name clean-up, request messages, printer address validation (IPv4 only; 11 tests) |

Sample responses are under `test/fixtures/` (turned into C arrays with `make_fixtures.py`).

### 19.2 Board-less preview (simulator, `tools/ui_preview.py`)

The real LVGL 9.5, the real screen code (`src/ui`, `src/rumy`), the real embedded icons and test data are compiled
on the computer with MinGW-w64 g++; only WiFi, WiZ, SD, the clock, the display driver and the media bridge are faked by
`sim/sim_stubs.cpp`. `sim/sim_main.cpp` plays a scenario (day, rain, light on, weather screen scrolling, settings,
night, earthquake, holiday, eye sleep, Spotify, Rumy with headphones, the "now playing" strip with short title / long
title / scrolling / paused / hidden / short weekday / long date, the printer screen, the source lines of the
diagnostics screen...) and produces **59 screenshots** (`docs/preview/`,
`docs/ui_preview.png`; at 480×320 with `--buyuk`: `docs/preview_buyuk/`), and prints the LVGL pool use for every
screen. Since the preview runs 64-bit, pointers take twice the space; so the preview pool is 1.5× the board's (24 KB
on the board → 36 KB in the preview, `RUMY_SIM_POOL_KB`). In the preview time runs ~2× faster than on the board (the
faked `vTaskDelay` fast-forwards time). Every visual change in this session (z's, Spotify screen, headphones, weather
block) was first seen and fixed here, then uploaded to the board.

**Edge cases (`--cesit`, 4 October):** `sim/sim_variants.cpp` plays a separate 100-screen scenario →
`docs/preview_cesit/`, `docs/ui_cesit.png` (480×320 with `--buyuk`). Coverage: 14 weather icons on the home screen
(day/night), temperatures from −25 °C to 45 °C and a missing high/low, the extreme values of the bars (humidity
0%/100%, AQI 0/500), long name / no name / long city / no city, clock not synchronised, no data at all, a 14-card cycle
(long earthquake place, long special day name, 123 km/h wind, a 99-hour print), an M7.8 earthquake; on the weather
screen winter (−18 °C, heavy snow) and every scroll with missing data; on the light screen coloured/dim/offline and a
long bulb name; pairing with 8 candidates and while scanning; on Spotify very long / emoji / Japanese / no duration /
no artist / past-the-duration songs and volume 0%–100%; in playlists emoji, very long, single-letter and Japanese
names and 0–10 000 songs, 30 lists; in a match long team and player names, 22 events, 90+12', half-time; on the
printer a long file name, a long error, an unknown state; in diagnostics every source failing; match + print + music +
special day + earthquake at the same time; a long notification.

**Animation films (`--film`, 10 October):** `sim/sim_film.cpp` plays the goal screen, notifications and league cards.
During a film every LVGL refresh is saved as a frame (`film_<name>/NNNN_<ms>.ppm`); `ui_preview.py` turns them into a
GIF and a 12-frame review strip. For every film it prints the frame count, the share of the screen redrawn per frame
(average / maximum) and the number of full-screen frames. Film time is measured by the faked clock (the `vTaskDelay`
inside `ui::loop()` advances time), so the timing is the same as on the board.

**Automatic screen audit (`sim/sim_audit.cpp`, in every mode):** the preview adds a listen-only draw unit to LVGL (it
takes no memory from the pool; measurements and images do not change). While each screenshot is drawn with a
single-piece buffer, every text and image draw is collected: the real ink box of the text (character by character),
the non-transparent pixels of images, the clip area and the drawing object. Three kinds of findings are written:
**overlap** (text-text or text-image; those covered by an opaque fill in between do not count), **clipped** (text
spills out of its clip area; the visible area of scrollable lists and scrolling text do not count), **missing glyph**
(a character not in the font). Findings are written to `<output>/denetim.txt`; intentional ones (scrolling song
title, list button over the cover...) are silenced with `sim/denetim_izin.txt`. What the first run found is in
CHANGELOG.md.

### 19.3 Verification on the board

After every upload, the boot log and `rapor` were read from the serial port (free heap, LVGL, stacks, media).
Performance was measured with `bench`. Cover transfer was tested with test scripts (a fake cover with known data, CRC
check, time per chunk). That the volume buttons really changed the Windows volume was measured with `pycaw`
(84 → 88 → back to 84).

### 19.4 Endurance test

The `cyd_endurance` environment fetches data every 30 s and changes screens every 5 s; `tools/endurance_log.py`
collects the serial log and summarises the lowest heap, the heap trend (B/hour, leak threshold −300), fragmentation,
the TLS peak, stack minimums, errors and unexpected restarts. (A full 24-hour run has not been done yet.)

---

## 20. Build, upload and distribution

### 20.1 Environments (`platformio.ini`)

| Environment | Description |
|---|---|
| `cyd` | ILI9341, normal build |
| `cyd_st7789` | Boards with an ST7789 panel |
| `cyd_endurance` | Endurance test |
| `cyd_prof` | LVGL profiling on |
| `native` | Unit tests on the computer |

Shared flags: `-O2`, `RUMY_VERSION`, ArduinoJson settings, `CORE_DEBUG_LEVEL=1`; source files are compiled with
`-Wall -Wextra` and kept **warning-free**. Flash QIO 80 MHz, CPU 240 MHz.

### 20.2 Build scripts

- `tools/build_web.py` (before): `web/index.html` → gzip → `src/webui/web_assets.h`.
- `tools/post_build.py` (after): `dist/Rumy.bin` (app only), `dist/Rumy-full-0x0.bin` (bootloader + partition table +
  app + updater, a single file), the ST7789/endurance/profiling builds and `dist/size_report_<env>.txt`; it warns if
  the app exceeds the 2 MB target and also reports whether it fits the app partition (2.5 MB) and the updater
  partition (448 KB). If the updater (`dist/Rumy-guncelleyici[-st7789].bin`) is missing or older than its sources, it
  is first built with `pio run -d updater`; the USB upload writes it to 0x290000 too.

### 20.3 Upload

- From source: `pio run -e cyd -t upload` (port from `secrets.ini` or `--upload-port COM7`) (CH340 auto reset, no BOOT
  button needed).
- App only (settings kept): `esptool write_flash 0x10000 dist/Rumy.bin`.
- Full image: `esptool write_flash --erase-all 0x0 dist/Rumy-full-0x0.bin`.
- Backup first: `esptool read_flash 0 0x400000 yedek/...bin` (in this project a backup of the Bruce firmware on the
  board was taken).
- With Launcher: `Rumy.bin` at the root of the SD card, installed from the Launcher menu (Launcher uses its own
  partition table, so there is no wireless update in this install).
- **Wireless (after 25 October):** `python tools/spotify_bridge.py --guncelle` (section 29). The new partition table
  must have been uploaded to the board once over USB.
- **Web installer:** <https://sudomrspear.github.io/Rumy/> writes the full image from Chrome/Edge with ESP Web Tools;
  `.github/workflows/web_kurulum.yml` republishes it for every release.
- **Note:** the Spotify bridge holds the serial port; close it before uploading.

On the development computer, connection drops (Windows "Code 43") were seen while the board was plugged into the USB
hub on the front panel; on a rear port the problem went away.

---

## 21. Code rules and constraints

Rules followed since the start of the project (for contribution steps see [CONTRIBUTING.md](CONTRIBUTING.md)):

- **No comment lines in C++ code.** Explanations are in the README and this document; the code is kept readable
  through naming. (Python tools have a usage description at the top.)
- **WiFi and personal settings are not hard-coded:** they live in NVS; `secrets*.ini` and `yedek/` are in
  `.gitignore`.
- LVGL calls only in the UI task.
- No `String` concatenation in loops (heap fragmentation), no blocking `delay()`, a timeout on every network
  operation.
- Large memory is allocated only at boot; no large `malloc` at run time.
- Warning-free builds (`-Wall -Wextra`).
- Interface texts are in English and Turkish through `rc::L(tr, en)` (section 31); serial log lines are Turkish kept
  close to ASCII.

---

## 22. Version history

Release notes are in [CHANGELOG.md](CHANGELOG.md) ([English](CHANGELOG.en.md)). Design decisions and measurements
are described in the relevant sections.

---

## 23. Problems met and their solutions

| Problem | Root cause | Fix |
|---|---|---|
| USB connection drops, "Code 43" | The USB hub on the front panel | The board was plugged into a rear USB port |
| HTTPS requests never start | Only 52.9 KB free heap with WiFi connected | SD buffers, the LVGL pool and the network stack were shrunk; threshold 64 KB |
| Touch is mirrored | The X axis is reversed on the board | `offset_rotation` 6 |
| Page transition 5–7 fps | Full-screen drawing every frame | ILI9341 hardware scroll |
| Weather screen opens in 300 ms | Icons from SD | Icons embedded in flash |
| SPI 80 MHz | The panel is unreliable at that speed | Measured with `spitest`, stayed at 40 MHz |
| Covers do not arrive | The UART interrupt cannot run while flash is erased | Flow control: flash work only while the line is idle |
| A cover takes 5.8 s | A 0.2 s read timeout in the bridge | `read(in_waiting or 1)` → 1.2 s |
| Volume buttons do not work | Narrow touch area for buttons at the edge | Touch area widened by 10 px |
| Eyes narrowed by default | A "sleepy" rule before night | Rule removed |
| The tail of "y" is clipped | Object height shorter than the font line | 29 px |
| The beat locks onto the hi-hat | Treble bands dominate | 3 kHz upper limit, bass 4× |
| The bridge's main loop hangs | PyAudio initialises COM as STA, winrt callbacks never arrive | The audio library on its own thread |
| No audio data | Music on non-default Bluetooth headphones | All outputs are listened to and the active one chosen |
| The bridge crashes at start-up | Turkish characters in PowerShell output | Forced to UTF-8 and decoded fault-tolerantly |
| Tempo wanders between 86/129/65 | Single-measurement autocorrelation, octave ambiguity | Accumulated autocorrelation, octave correction, voting, hold |
| Rumy ahead of the beat on Bluetooth | Wireless latency | A 200 ms margin when Bluetooth is detected |
| Blank display on the Pi | Nothing enables the display pipeline with `fbcon=map:9` | Rumy enables the pipeline itself (`SETCRTC`) |
| Wrong colours on the Pi | In packed RGB565 blending, red's fractional bits spill into blue | Channels split into 32 bits |
| On the Pi touching the bottom registers the top | Touch Y is reversed on this board | `dokunmatik 2`, the calibration screen |
| WiZ shows on/off but the colour does not change | Bulb firmware 1.38.0 rejects unsigned commands (-32602) | "Allow all controls" in the WiZ app |
| Weather screen scrolling ~10 fps on the Pi | Full-screen frame 108 ms at SPI 22 MHz | `spi_hiz.sh 40` to be tried |
| The bridge drops while taking a screenshot over WiFi (3 October) | Free heap fell below the speed threshold (60 KB), every chunk waited; HTTPS held 47–52 KB at the same time; sending timed out after 1 s | No TLS starts during a screenshot, thresholds 40/24 KB, timeout 4 s |
| AFAD/Nager not fetched for days (27 October) | Static RAM grew by 28 KB, free heap stuck at 36–46 KB, HTTPS needs ≥ 64 KB | The bridge does HTTPS on the computer; without a bridge, Kandilli (plain HTTP) |
| The record spins inside a square on the CYD | When the record went from 25 → 17 px the drawing sizes did not shrink (12.4 px radius in a 17 px frame), the rim ring filled the frame | Sizes scale against `DESIGN_PX = 25`; the Pi frames stayed the same |

---

## 24. Known limits and open work

- **Data sources are not official:** match data comes from ESPN's and LiveScore's public but undocumented services;
  if the format changes the parsers need updating. On one test machine LiveScore's certificate could not be
  verified; in that case fast goal alerts do not work and goals come from ESPN (50–100 s late). For player photos,
  TheSportsDB's free trial key may reject requests (403); the tool then switches to Wikimedia Commons and Wikipedia.
- **The earthquake region is fixed to Marmara:** the AFAD query and the Kandilli filter use the Marmara box
  (`rc::marmaraBox`, 39.5–41.8 N, 26.0–30.5 E). Choosing the box by location for users in other provinces is open
  work.
- **Tight memory margin:** static RAM ~89 KB; with WiFi connected free heap stays at 36–46 KB, so the board cannot do
  HTTPS itself (threshold 64 KB). AFAD and Nager come through the bridge or from Kandilli (section 10.6). The heap's
  largest free block drops to ~37–45 KB after a long run; the 24-hour endurance test (`cyd_endurance`) has not been
  run yet.
- **The bridge is Windows-only:** the Spotify screen, Rumy with headphones, the coding agent light, the printer camera,
  fast goals and player photos need the bridge on the computer (SMTC and WASAPI are Windows-specific). For the bridge
  to start at sign-in, `--autostart ac` is needed once.
- **Beat tracking** may slip to half or one-and-a-half tempo on songs with an ambiguous beat (6/8, slow ballads);
  Bluetooth latency varies by device (`--beat-offset`).
- **Fonts** contain only Latin, Latin Extended-A and the symbols needed; Japanese, Arabic and emoji song titles are not
  displayed (the bridge drops them or converts them to unaccented equivalents).
- **Flash and IRAM:** the app uses ~85% of the 2.5 MB `app0` partition; IRAM ~123/128 KB. If shrinking is needed, the
  first candidates are the large fonts and embedded images. The English strings added for language support also live
  in flash.
- **Wireless update** needs an SD card (image + 256 KB free); not available in a Launcher install or on the Pi. The
  board's panel type is not checked: the ST7789 image must be picked by hand for an ST7789 board.
- **Printer:** there is no Creality error code table; the printer's own code is shown. The camera picture needs the
  bridge, go2rtc and `av`; the UI task waits while a frame is received (~200 ms per frame with the chamber light on).
- **The SD card may fail to mount after a reset:** if esptool resets the board in the middle of an SD operation, the
  card stays unresponsive and only recovers after a power cycle. Suggestion: if `sdcard::begin` fails, send ≥ 600
  bytes of 0xFF with CS low and retry.
- **HTTPS while printing:** when the printer socket's buffers build up the lowest free heap can drop to ~7 KB (only
  while the board does HTTPS; not with the bridge). Suggestion: close the printer socket for the duration of a TLS
  request or shrink `SO_RCVBUF`.
- **The Raspberry Pi version is experimental:** less tested than the CYD. On the 480×320 coding agent page the tail
  of the "y" in the project name touches the line below by 2 px (the only finding in the simulator audit).
- **Language:** English strings are longer than Turkish ones in places; the simulator audit has not been rerun for
  every screen in English (section 31).

---

## 25. Creality K2 printer screen and camera

Goal: a printer page like the Spotify screen (left of Spotify); the print status, any error flagged, the page opening
by itself on an error (at night too), the printer's camera if possible, and a bar showing the finish.

### 25.1 Research and the chosen approach

The printer used during development was scanned on the network (port probing only); model code F021. Only read
requests were sent to the printer.

| Port | Service | Use |
|---|---|---|
| 9999 | Creality's local WebSocket (used by Creality Print) | **State, progress, time left, error code** |
| 7125 | Stock Moonraker (Klipper) | Tried: works without a password, but because the slicer is "Unknown" there is no estimated or remaining time, and no Creality-specific errors |
| 8000 | `webrtc_local` (camera) | WebRTC only (H.264, 1280×720, ~17 fps, a key frame every 1 s); no MJPEG or single-frame address |

The protocol was taken from the code of the open-source Home Assistant integration **ha_creality_ws** (on connect
`{"method":"get","params":{"ReqPrinterPara":1}}`, an `ok` reply to the printer's `{"ModeCode":"heart_beat"}` message,
messages are partial updates to be merged). For the camera, **go2rtc**'s K2-specific `#format=creality` support was
used (the K2 sends RTP with a payload type it does not announce in the SDP answer; generic WebRTC clients connect but
get no picture). Sources: github.com/3dg1luk43/ha_creality_ws, helixscreen.org (K2 page), go2rtc #1600,
sw3defy.github.io (K2 Plus camera page), github.com/skilly00/K2-Camera-Fix, github.com/pdromnt/k2dash.

### 25.2 Parser (`lib/rumy_core/src/k2.*`, tested on the computer)

- `k2Parse`: reads only the fields needed with an ArduinoJson filter: `state`, `printProgress`, `printLeftTime`,
  `printJobTime`, `printStartTime`, `printFileName`, `layer`, `TotalLayer`, `nozzleTemp`, `targetNozzleTemp`,
  `bedTemp0`, `targetBedTemp0`, `boxTemp`, `err{errcode,key,value}`, `withSelfTest`, `materialStatus`, `hostname`,
  `model`, `curFeedratePct`, `ModeCode`. Temperatures sometimes come as text ("23.330000") and sometimes as numbers;
  both are read. Partial messages are written over the previous state.
- `k2Phase`: error code ≠ 0 → Error; `withSelfTest` 1–99 → Calibrating; `state` 1 Printing, 2 Completed, 4 Stopped,
  5 Paused; others Ready. `k2Runout`: `materialStatus` 1 and the print running/paused.
- `k2JobName`: "Telefon_Standi_V2.stl_PLA_54m36s.gcode" → "Telefon Standi V2" + material "PLA".
- A bug was caught during testing: integers were read through a float, so `printStartTime` (1790423916) became
  1790423936; integers are now read directly.

### 25.3 Connection on the board (`src/printer/printer.cpp`)

- No separate task was created: the client runs **in the network task's loop** (`printer::service`, every 40 ms) and
  shares the network task's 8 KB JSON area (the network task does its jobs in turn, so there is no clash). So no extra
  stack and no extra JSON memory were needed.
- A **non-blocking** connection with an lwIP socket (3 s timeout), the HTTP Upgrade handshake (key via
  `mbedtls_base64`), masked client frames, ping/pong, close. Messages are parsed **as a stream from the socket with no
  intermediate buffer** (`WsSource`, a 128 B read buffer); there is no message size limit (a full status message is
  1.9 KB).
- A status request every 30 s; reconnect if no data comes for 45 s; if it cannot connect, a wait growing from 5 s to
  60 s. Every read/write has a 400 ms timeout.
- The result goes to the UI through `store::PRINTER` (at most once a second; immediately when the phase or error
  changes).
- The address is in NVS (`phost`): the "3D printer" section of the phone app or serial `yazici 192.168.1.19`. Only
  IPv4 is accepted (`rc::k2HostValid`, 4 parts, 0–255, no leading zeros; `settings::sane` and the web page apply the
  same rule). Previously a name (e.g. "K2-PRINTER") was also accepted but could not be connected to.

### 25.4 Screen and behaviour

- **Placement:** swiping right on the home screen opens Spotify (if the bridge is connected), swiping once more opens
  the printer; without the bridge, the printer directly. Swipe left to go back.
- **Layout** (three custom draw objects, ~900 B in the LVGL pool on top of the home screen): a 160×120 camera box on
  the left; on the right the printer name, a coloured state badge, the job name, material and layer, nozzle/bed/chamber
  temperatures or a red error box; at the bottom the percentage, the finish time ("Done 15:12" = now +
  `printLeftTime`), a bar showing the finish, time remaining and elapsed. When complete, a green bar and "Took
  56 min"; without a connection the IP and "Is it on and on the same network?"; if not set up, setup instructions.
- **On an error** (`ui::printerChanged`): when a new error code or a filament run-out is seen, the printer screen
  opens whatever screen you are on, and the frame turns red. In the middle of the night the screen **wakes for 60 s**,
  then returns to the night screen; a red "Printer error · code …" line stays on the night screen. The same error does
  not trigger again.
- While printing or with an error, the printer screen is not closed by the "return home" delay.
- **Home screen:** a "Print 45% · 1 h 12 min left" card joins the info card rotation (tap for the printer screen); with
  an error the card is red and takes priority; for 30 min after the print ends, "Print finished: …". **Night screen:**
  while printing, a dim "Print 45% · done 04:22".

### 25.5 Camera

- **Computer (bridge, `PrinterCamera`):** when the board opens the printer screen it sends `@cam on <ip>` every 5 s.
  The bridge starts `tools/go2rtc/go2rtc.exe` (v1.9.14) windowless and reachable only from 127.0.0.1, and ties it to a
  Windows job object (go2rtc closes however the bridge closes). The picture is decoded from RTSP with PyAV; once a
  second the latest frame is cropped to 4:3, shrunk to 160×120 and made into a JPEG, lowering the quality until it is
  ≤ 6 KB. If no request comes for 12 s sending stops, and go2rtc closes after 60 s. Decoding every frame costs ~1% of
  the computer's CPU.
- **Transfer:** `@cam <bytes> <crc32>` → board `@go <crc32>` → 2048-byte chunks, each followed by `@ok <n>` →
  `@cam ok` or `@cam err …` (the same flow control as the cover transfer; the serial buffer does not overflow).
- **Board (`src/media/camera.cpp`):** the JPEG is not collected in RAM; **as it arrives** over serial it is decoded
  with TJpgDec (LovyanGFX's `lgfx_jd_*` functions) and pushed straight to the panel in the camera box
  (`display::drawJpeg`), and the CRC is checked at the end. Only a 4 KB decoder area is allocated, while the printer
  screen is open. The first design had a 10 KB frame buffer; with the camera on, the lowest free heap during HTTPS
  requests fell to 9 952 B, and with streamed decoding it rose to 16 972 B. LovyanGFX's ready-made `drawJpg` was not
  used because it `malloc`s 3.9 KB on every frame.
- While camera frames arrive LVGL does not draw that area (the placeholder is drawn only when there is no frame); the
  red frame on the screen is set again only when the error state changes (setting it on every update made the whole
  screen redraw and wiped the camera picture).

### 25.6 Verification and measurements (on the board)

- Connected to the real printer: "K2-YAZICI · Completed · 100% · Telefon Standi V2"; 0 parse errors in 51 messages;
  "No connection" with a wrong IP, reconnected when switched back to the right IP.
- Camera: 120 frames, 0 errors; receiving + decoding + drawing a dark frame (~960 B) 33 ms. A colourful test image was
  sent to the board and read back with `ekran goruntusu`: colour, position and size exactly right.
- Memory: the printer screen +~900 B in the LVGL pool (3 KB in the screen needs table), the highest peak did not
  change (16 440 B). Heap: 79.3 KB at boot (80.3 KB before; printer state copies and the connection), ~78.5 KB with the
  connection open, 74.3 KB with the camera on. At least 4 652 B free on the network task's stack.
- 4 environments built without warnings, 79/79 tests passed, 9 new screenshots in the simulator.
- **Live print (27 September 20:38, `ekran goruntusu`):** "Printing", Telefon Standi V2, PLA, layer 14/198, nozzle
  222/220°, bed 50°, chamber 28°, 8%, done 21:25, 47 min left, 6 min elapsed. 20:38 + 47 min = 21:25, so the finish
  time is consistent. The camera showed the print clearly with the chamber light on. The memory drop during HTTPS was
  found on this same print (section 24).

---

## 26. Football (ESPN)

Goal: Rumy relays goals, substitutions and cards from your team's match, and never wakes up in night mode. The first
version was only for Fenerbahçe; since 1.3.0 the team is chosen from the Süper Lig and the top five leagues (section
26.7).

### 26.1 Source research

| Source | Result |
|---|---|
| ESPN (undocumented, public) | Chosen. No account/key, **plain HTTP** works (no TLS memory needed on the CYD), Süper Lig + European cups + cup, with goal/card/substitution minutes. Risk: not official, the format may change |
| API-Football (official) | Free plan 100 requests a day, account + key; with a 30 s query it runs out in ~50 min |
| football-data.org | No Süper Lig on the free plan |
| SofaScore, FotMob | Not official, block non-browser clients / require a signature |

ESPN endpoints measured (with the Beşiktaş–Fenerbahçe match of 5 September):

| Endpoint | Size | Use |
|---|---|---|
| `site…/soccer/all/teams/436/schedule?fixture=true` | 276 KB (35 matches) | The next match every 6 hours; parsed match by match from the stream |
| `sports.core…/leagues/all/events/<id>/competitions/<id>/status` | 341 B | Minute, half, state (`pre`/`in`/`post`, `STATUS_HALFTIME`) |
| `…/plays?limit=10&page=<n>` | ≤ 19 KB | Goals (penalty, own goal), yellow/red cards, substitutions; the score at the time of each event |
| `sports.core…/soccer/athletes/<id>` | 4.8 KB | The name of the player going off in a substitution (the event only has a link) |

The name in a substitution event is the player **coming on** (`subbedIn` in the line-up data); the second participant
is the one going off.

### 26.2 Code

- `lib/rumy_core/src/espn.*` (tested on the computer, `test/test_espn`, 8 tests): path builders, stream parsers (find
  the top-level key and read the array element by element with a filter; the 8 KB JSON area is enough), `matchApply`
  (adds only events with a new sequence, works out who scored from the increase in the score, keeps the latest 20
  events), Turkish names of Süper Lig teams ("Besiktas" → "Beşiktaş", "BJK"), competition names (English names in
  English mode, section 31).
- `src/match/match.*` (in the network task, the `mac` job): fixtures every 6 hours; from 15 min before kick-off the
  status every 60 s, during the match the status + new event pages every 20 s (at most 3 pages, 2 player names). The
  job period can now vary by job (`Job::periodFn`). If a match is under way when Rumy starts, the first events fetched
  count as the "seed" (`seedSeq`) and no notification appears; if ESPN deletes an event (VAR) the list is rebuilt, again
  with no notification. When it ends the result stays for 3 hours. The team is in NVS (`team`, the `takim` command;
  0 = off).
- `src/ui/match_ui.*`: notifications on the top layer. A Fenerbahçe goal (`ours`) gets the full-screen animated goal
  screen (`goal_screen.*`, 11 s); others get a 6 s box sliding in at the top. They are shown in turn; skipped in night
  mode or during bench; while the match screen is open only our goals appear.
- `src/ui/scr_match.cpp`: the match screen (scrollable event list, a single draw object).
- Home screen: `CARD_MATCH` (match day; pinned from 3 hours before kick-off; a pinned navy card while live; 3 hours
  after it ends).
- A new font `font_rumy_goal` (40 px, only "GOL!"/"GOAL!"; 54 px on the Pi), icons `FUTBOL`, `EXCHANGE`.
- Pi: the `net_pi.cpp` job table, body limit 256 → 512 KB (the fixtures are 276 KB).
- Trying it: `mac test …` sets up a fake match in the network task (10 min), `mac test kapat` returns to real data.

### 26.3 Memory

`rc::Match` 1 496 B; one copy each in the network task, store and the UI (~4.5 KB of static RAM). An event page is
480 B. In the simulator the LVGL peak did not change (24 760 B); +550 B in the pool while the goal screen is open.
The Champions League list (26.4) is a separate structure: `rc::Board` 1 760 B, three copies.

### 26.4 League day: every Süper Lig and Champions League match (13 October 2026)

Separate from team tracking, a second network job (`lig`) watches every match of the day in the `cfg::BOARD_LEAGUES`
leagues (`tur.1`, `uefa.champions`). Each round fetches the leagues in turn; if one fails the other's is still
published. Every match has a `league` field; the "seed" count of the first fetch is per league (the `Board::seeded`
bit mask). A Süper Lig matchday is 15–50 KB (10 October: 4 matches 42.6 KB), a day without matches 1.8 KB. Since every
row in the Süper Lig list is a Turkish team, the grey highlight is only used in European leagues; the selected team's
row is in the team colours in every league (by team number).

- **Source:** `site.api.espn.com/apis/site/v2/sports/soccer/uefa.champions/scoreboard?dates=YYYYMMDD` (plain HTTP).
  One request gives every match of the day: state, minute, score and every goal in `details` (minute, team, scorer,
  penalty / own goal). Measured: a day without matches 1.8 KB, a 9-match night before kick-off 115 KB, a night with 6
  matches finished 71 KB. Lighter endpoints were tried: `personalized/v2/scoreboard/header` 137–716 KB (larger), core
  `events` only gives links (a separate request per match for the score).
- **Code:** `lib/rumy_core/src/espn_board.h` + `espn.cpp` (`espnParseBoard`, tested: two-stage fixtures from real
  10 September data, `test/test_espn`): the events array is read match by match with a filter; each match keeps its
  goal count, and new goals are added to the `goals` ring (8) with a sequence number. On the first fetch (start of day
  or boot) existing goals count as the seed and no notification appears; if VAR removes a goal the count drops and no
  notification appears. For own goals, the team side is flipped if it does not match the score. Fenerbahçe's own match
  stays highlighted in the list but its goals are not announced by this job (the `mac` job shows them full-screen).
  Turkish teams' names are in Turkish (`espnTeamTurkish`), others use ESPN's short name ("Man City", "S Bratislava").
- **Timing** (`src/match/board.*`): every 60 s while there are matches; starts 10 min before the first match; when the
  day's matches are over, or there are none, it waits until 2 min after midnight (at most 6 hours). The list is
  cleared when the day changes. Serial: the `[sl]` line and the match list under the `mac` report; `sl yenile` fetches
  now; every new goal is written to the diagnostics log.
- **UI:** `match_ui` shows the team's events first, then the Champions League goals in turn: a 6 s box at the top,
  the league badge on the left, "Galatasaray 1–1 Barcelona" (the scoring side bright), below it a ball icon + scorer
  + minute (penalty "pen.", own goal "o.g."). Not shown in night mode or during bench. Under the event list on the
  match screen, a "Champions League · today" section: time/minute (red if live), home, score, away; rows with a Turkish
  team grey, the team's row in the team colours. Drafts in `docs/preview_film/*sl_*`.
- **Memory:** `rc::Board` 1 760 B (18 matches, 8 goals); one copy each in the network task, store and the UI (5.3 KB
  of static RAM). JSON area: on a 64-bit computer the peak for these fixtures is 9 KB (16 B slots); on the board slots
  are 8 B, so the same data is ~5 KB and fits the 8 KB area. That is why the computer test runs with a 16 KB area. In a
  match with many cards (≈30 events) the area may not be enough; then the fetch returns `E_PARSE` and is retried the
  next minute.
- **Measured on the board (13 October 21:57, Champions League only):** a 9-match night 116 524 B, 613 ms; JSON area
  peak 4 376 / 8 192 B; free memory 69 → 63 KB, 5.9 KB free on the network stack.
- **A bug caught on the first upload:** in pre-match events,
  `competitors[].leaders[].leaders[].athlete.links[].rel[]` is nested 13 deep; ArduinoJson's default depth limit is 10,
  so the third match got `E_PARSE` (-6) (finished matches lack this field, so the test did not show it). `eachElement`
  now reads with `NestingLimit(24)`; a test with real pre-match data was added (`espn_board_pre.json`).
- **Live verification (13 October 22:10):** 14 requests, 0 errors, 3 goals caught in 7 live matches (Atlético 1-0,
  Viking 1-0, Villarreal 0-1). With the two-league version (22:16): 2 requests per round, 108 KB, 2.2 s; free memory
  63.6 KB, 5.8 KB free on the network stack.

### 26.5 Football page (13 October 2026)

- **Navigation:** swiping right on the home screen opens Football (`SCR_FOOTBALL`); from there right for Spotify /
  printer, left for home. Tapping a league opens `SCR_LEAGUE`; swiping right or ‹ in the header returns to Football.
  Serial: `ekran futbol`, `ligsayfa <tur.1|uefa.champions|eng.1|esp.1|ita.1|ger.1|fra.1> [puan]`.
- **Football page:** 7 leagues (Süper Lig, Champions League, Premier League, LaLiga, Serie A, Bundesliga, Ligue 1),
  badge + name; for the Süper Lig and Champions League, today's state from the league day job (`3 live`,
  `Today 4 matches`).
- **League page, matchday:** without a date, ESPN `scoreboard` returns the "current day" (the last finished day or
  the next matchday); ‹ › go one day back/forward, skipping empty days up to 14 days (1.8 KB per empty day). A date
  range (`dates=A-B`) returns 400 on this endpoint, so it is not used. Rows use the same drawing as the league day list
  (`match_ui::drawGameRow`).
- **Standings** (the top-right button; press again for matches): `apis/v2/sports/soccer/<league>/standings`, plain
  HTTP, 61–69 KB (Champions League with 36 teams 143 KB). The `entries` key is sought in the stream (`seekKey`) and each
  team read with a filter: position, P, W, D, L, goal difference, points; ESPN's zone note (colour + description) is
  translated and used for the coloured strip on the left and the legend under the table (Champions League, …
  qualifying, Relegation, Round of 16, Play-off, Eliminated). The team's row is in the team colours.
- **Code:** `rc::Standings` (36 rows, 6 zones), `rc::LeagueView` (matchday `Board` or table, a union, 1.8 KB),
  `espnParseStandings`, `espnLeagueSlug`; the `src/match/leagues.*` network job `ligsayfa` only runs while the page is
  open (the request order comes from the UI through `volatile` fields, `net::refresh`): table every 10 min, matchday
  every 5 min, every 60 s while a match is live; it stops when the page closes. `src/ui/scr_football.cpp` has two
  screens (a single draw object + a scroll container).
- **Badges:** the repository has no club or competition logos. For each competition `tools/make_match_art.py` uses
  `tools/assets/lig_logolari/<name>.png` if present (outside git, logos the user puts there), otherwise it draws a
  plain abbreviated badge (SL, ŞL, AL, KL, TK, SK, PL, LL, SA, BL, L1; lettering in the Montserrat typeface from the
  LVGL package, OFL 1.1).
- **Measured on the board:** Süper Lig table 55 KB, Premier League table 61 KB / 362 ms, Champions League matchday
  112 KB; free memory 63 → 58 KB (at least 44 KB), LVGL peak 8.0 / 22.5 KB, 0 rejected screens, 4.7 KB free on the
  network stack.

### 26.6 Fast goal alerts (17 October 2026)

**Measurement (17 October, live matches).** ESPN keeps the real time of each goal (`wallclock`); the moment it was
first seen was compared with it (scoreboard and event list every 6 s, plus FastCast, the live channel of ESPN's own
website, `gp-soccer-<league>-<match>` JSON-patch stream):

| Goal | ESPN scoreboard | ESPN live channel | LiveScore |
|---|---|---|---|
| Inter 44' | +57 s | – | – |
| PSG 77' | +92 s | – | – |
| Bayern 26' | +100 s | – | – |
| PSG 90+3' | +49 s | +48 s | – |
| Aston Villa 36' | +69 s | +73 s | – |
| Bayern 34' | +69 s | +81 s | – |
| Fenerbahçe 47' (İrfan Can) | +93 s (event list +77) | +69 s | already there when the measurement started |
| Barcelona 41' (Rodri) | more than +82 s | +69 s | **+22 s** |
| Bayern 52' (with the bridge module) | – | – | **+44 s** |

ESPN's delay comes from its own data entry (50–100 s); the cache is 8–10 s, and the live channel is not fast either.
The board's polling interval to ESPN (Fenerbahçe 20 s, league day 60 s) was added on top. LiveScore is 45–60 s ahead
of ESPN; it also showed a VAR cancellation (Fenerbahçe 47') 32 s before ESPN's live channel and 93 s before its
scoreboard. SofaScore's and FotMob's match details are protected (403 / signature); the protections were not
bypassed. LiveScore, like ESPN, is an unofficial source.

**LiveScore endpoints** (`prod-public-api.livescore.com`, HTTPS only, 10 s CDN cache):
`/v1/api/app/live/soccer/3?MD=1` all live matches (~58 KB), `/v1/api/app/incidents/soccer/<Eid>?locale=en` one
match's events (0.4–4 KB: score `Tr1/Tr2`, events half by half). Event codes were compared with ESPN's records:
36 goal, 37 penalty goal, 39 own goal (`Nm` the side that benefits), 38 disallowed goal (`IR: VAR`), 62 missed penalty,
43 yellow, 44 second yellow, 45 red, 63 assist. The score updates a few seconds before the event list.

**Bridge** (`tools/fast_goals.py`, starts with the bridge): takes today's Süper Lig and European cup matches and the
team's fixtures from ESPN; at match time it checks the LiveScore live list every 5 s, matches games by team names
(unaccented, word prefixes, reversed home/away), and when the score goes up it takes the scorer from the event list
and sends `@gol <espn-no> <home> <away> <h|a> <g|p|o> <minute> <player>`; if the name is not there yet it sends it
without a name, and sends the same goal again with the name when it arrives within 90 s. If the score drops,
`@skor`. Every 30 s, `@skor` for live matches (liveness). Try it: `python tools/fast_goals.py --deneme esp.1 ita.1`
(on 17 October the matching was right in 5 live matches, and the Inter 84' and Bayern 52' lines were produced).

**Board:** `src/match/fast.*` puts bridge lines into a mailbox (8) and wakes the `mac` job; in the network task they
are applied to the team's match if they belong to it (`rc::matchAddFast`: a goal event with ID `MATCH_FAST_ID`, the
score), otherwise to the league day list (`board::fastMsg`). If the bridge is silent for 45 s and the team's match is
live, the board asks itself: one match-up from the live list per match (TLS, 58 KB, streamed; retried after 2 min if
it fails), then the event list every 10 s (TLS, small). When ESPN later brings the same goal it is matched by the
score signature (home-away): the fast event's ID changes to ESPN's and no new notification appears (`matchApply`); in
the league day list a bit set of signatures keeps the same goal from being announced twice. As long as the fast
source is fresh (90 s) the score comes from it; ESPN's lagging score does not trigger a false "goal disallowed"
alert, while a score that LiveScore drops is announced as a VAR cancellation after 25 s of confirmation. ESPN
intervals were shortened too: 10 s while the team's match is live (cache 8–10 s), 20 s on league days. Serial: in the
`mac` report `[hizli] kaynak kopru|kart|yok · livescore <Eid> · requests · errors · fast goals · bridge messages`
(source bridge|board|none), `[lig] hizli gol N` (fast goals N). Memory: static RAM +1.7 KB (the LiveScore match,
signature sets).

### 26.7 Team selection and team colours (1.3.0)

- **Team list:** `tools/make_teams.py` takes the teams of six leagues (Süper Lig, Premier League, LaLiga, Serie A,
  Bundesliga, Ligue 1) from ESPN's `soccer/<league>/teams` endpoint and writes them to the `LEAGUES` and `TEAMS` lines
  in `web/index.html`: ESPN number, name, abbreviation, two colours, league order (114 teams, ~5 KB, ~2 KB gzipped).
  The Turkish names and abbreviations of Süper Lig teams are read from the board's `espn.cpp` table; ESPN gives no
  colour (000000) for most Süper Lig teams, so club colours come from the tool's `RENK` (colour) table (a few big
  European clubs' colours are corrected too). Run it again at the start of each season.
- **App:** on the Football page, a "Match tracking" toggle (`fut`) at the top, below it league tabs and team rows with
  coloured badges; the badge text is the team's second colour, or white or black if its contrast with the background
  is below 3:1. Choosing a team sends `{"team", "tc1", "tc2"}`; "No team" sets `team` to 0.
- **Board:** `settings` adds `fut`, `tc1`, `tc2` to NVS. While `fut` is 0 the match and league day jobs make no
  queries (`match.cpp`, `board.cpp`), goal notifications from other matches do not come (`match_ui::leagueWanted`), and
  `@futbol 0` goes to the bridge. Serial commands: `futbol ac|kapat`, `takim <no> [colour1 colour2]`.
- **Palette:** `rc::teamPalette(c1, c2)` (`lib/rumy_core/src/color.*`) produces from two colours a dark background, a
  deep shade, a stripe, an accent, light shades and two bulb colours. The background comes from the colour that is not
  light (HSV value lowered to 80/255); if both are light or colourless, a neutral dark grey is used. The accent is the
  other colour if it gives at least 3:1 contrast with the background, otherwise white. The bulb colours are saturated
  versions of the team's two colours (white if colourless; if both are the same, one is warm white). Without colours
  (0, 0) the palette gives today's navy-and-yellow constants. `theme::team*()` returns this palette in the UI task;
  `theme::setTeamColors` recomputes it when settings change. Tests in `test/test_color`.
- **Bridge:** when Rumy connects to the bridge, when settings change and once a minute it sends
  `@futbol <0|1> <team> <colour1> <colour2>` (`rc::footballLine`). `tools/fast_goals.py` fetches the team's fixtures
  with this number (and watches nothing if it is off); `tools/player_photo.py` gets the team's name and league from
  ESPN once and caches them, checks shirts against the team's two colours and searches Commons with the team's name.
- **Simulator:** `RUMY_SIM_ONLY=takim python tools/ui_preview.py --film` draws the live card, two goal screens and the
  match screen with six sample teams.

## 27. Spotify playlists

Goal: opening Spotify playlists from Rumy. Design drafts were shown (2 entry points, 3 layouts); the choice: A (list:
cover + name + track count), a "Shuffle" button, and a separate button on the Spotify playback screen.

### 27.1 Research

- Song info comes from the Windows media session (SMTC); SMTC knows nothing about playlists and cannot start one.
  Lists and starting them need the **Spotify Web API**.
- February–March 2026 changes
  ([migration guide](https://developer.spotify.com/documentation/web-api/tutorials/february-2026-migration-guide)):
  for apps in development mode **the account owner must have Premium**, 5 users per app; `GET /me/playlists` stays,
  list contents only for owned lists; a list's track count is now `items.total` (`tracks` is deprecated; both are
  read).
- `PUT /me/player/play` is Premium only; a list with `context_uri`, the starting song with `offset.position`.
- The redirect address cannot be `localhost`; it must be `http://127.0.0.1:<port>`. Since it is a desktop tool,
  **PKCE** is used instead of a secret key (no client secret needed).
- Liked Songs starts with the `spotify:user:<id>:collection` context (4 October, a real account). Spotify's own lists
  (Daily Mix etc.) have not been tried yet (section 24).

### 27.2 Bridge (`tools/spotify_api.py`, `tools/spotify_bridge.py`)

- `spotify_api.py`: PKCE sign-in (`--spotify-giris <client_id>`: a one-shot HTTP server on 127.0.0.1:8888, consent in
  the browser, code → token), token refresh (retries once on 401; if the refresh is rejected it falls back to the "not
  signed in" state), `playlists()` (first the Liked Songs count, then at most 30 lists), `context()` (the playing
  list), `device()` (the "Computer" device matching this computer's name; otherwise the active device), `play()` (if
  there is no device it opens Spotify with `spotify:` and waits 15 s; with "Shuffle" on it starts from a random song
  and turns shuffle on). An 8 s timeout on every request. Errors become short messages ("Spotify Premium required",
  "Could not open Spotify on the computer"...). Tokens are in `tools/.cache/spotify_api.json` (outside git), never
  printed.
- `Playlists` in the bridge lives across connections; lists are cached for 5 min. When the screen opens (`@cmd pls`)
  the cache is sent at once and refreshed on a separate thread if stale; the playing list is asked again on every
  open. Covers are downloaded at 128 px (6 in parallel) and stored.
- **Tiles:** since the board's RAM is tight, list covers are not sent one by one; 9 covers are arranged into a single
  ART×ART "cover" image (cell ART/3 = 48 px, cover ART×5/18 = 40 px, at the top left of the cell, rounded). The board
  requests the tile with `@need` like a normal cover and writes it to the same flash cache; the list screen draws it
  clipped (`du::Clip`). 30 lists = 4 tiles = 4 slots (out of 21); tiles are protected when choosing slots.

### 27.3 Protocol

| Direction | Message | Meaning |
|---|---|---|
| Rumy → PC | `@cmd pls` | The list screen opened, send the lists |
| PC → Rumy | `@pls <version> <n> hazir\|yukleniyor\|giris\|hata <shuffle 0\|1> [message]` | List header (ready / loading / sign-in needed / error); the version stays the same unless the content changes |
| PC → Rumy | `@pl⇥order⇥track_count⇥tile_hash⇥cell⇥name` | Each list (name at most 39 bytes) |
| PC → Rumy | `@plnow <order\|-1>` | The playing list |
| Rumy → PC | `@cmd plplay <version> <order>` | Start the list; if the version does not match the bridge says "List changed" and sends the list again |
| PC → Rumy | `@plst <order> bas\|tamam\|hata [message]` | Start state (starting / done / error) |
| Rumy → PC | `@cmd plmix 0\|1` | "Shuffle" preference (stored in the bridge) |

The parsers are in `lib/rumy_core/src/media_proto.*`, tested on the computer (`test/test_media`).

### 27.4 Board (`src/media/media.cpp`, `src/ui/scr_playlist.cpp`)

- `media::Playlists` (static ~1.4 KB: 30 × 44 B rows + state): the list is reset when the header version changes, and
  rows are added in order (a row is rejected if one is missing in between). If a start gets no answer within 25 s,
  "No reply from the computer". If the bridge drops, the state is reset and the list screen returns home.
- List screen: a fixed header (back, "My playlists", "Shuffle"), below it a single vertically scrolling draw object
  (only visible rows are drawn). The playing list is green, highlighted and has a speaker icon; when tapped the row
  gets a green frame, "Starting on computer…" and a spinner (only the spinner's area is redrawn every 60 ms); on
  success it returns to the Spotify screen, on error the row keeps a red frame and the message. Long names are
  shortened with "…". On opening it scrolls so the playing list is visible. Empty states: loading, account not
  connected (sign-in command), network error, no lists.
- Spotify screen: a black list button framed in the accent colour at the bottom-right corner of the cover (32 px,
  touch area +8 px).
- New icons `LIST`, `RANDOM`; serial: `ekran liste`, `rapor` → `[medya] listeler …` (media, playlists).

## 28. Coding agent light: Claude Code and Codex (adapted from ClawLight)

It was built for Claude Code first (28.1–28.6), then Codex was added (28.7).

### 28.1 States and look

| State | When | Star on the home screen |
|---|---|---|
| Working (`c`) | A request was sent, a tool is in use | Green, turning 60° per second, breathing between 84–100% size |
| Waiting for you (`b`) | Permission request, question (`AskUserQuestion`), plan approval (`ExitPlanMode`), MCP elicitation, API error | Red, its glow pulsing every 1.1 s |
| Done / idle (`i`) | The job finished or the session just opened | Yellow, steady; drops off the list after 30 min |
| None | No open session, the bridge is off, or nothing heard for 30 s | Hidden |

The overall colour comes from the most urgent state (waiting > working > done). The star has eight rays (long/short),
is drawn with lines (`agents_ui::drawBeacon`) and has a glow of two transparent circles; on the home screen it sits
under the city name (x 4, y 26, a 58×28 area, radius 9 px), a nod to the star Claude Code spins while working. If
several sessions share a state, their count is shown next to it. The animation runs at 20 fps only while the home
screen is visible and the state is working/waiting, redrawing only the star's ~28×28 px area (simulator film: on
average 6–7% of the screen per frame).

### 28.2 Notifications and reactions (`src/ui/agents_ui.*`)

- **State changes** are found with `rc::agentsNotes` (old and new list, by session ID): `AN_WAIT` when a session
  starts waiting, `AN_DONE` when it goes from working to done and the job took ≥ 120 s. When the bridge reconnects the
  first list counts as the seed and no notification appears.
- **Box:** the same place and style as match notifications (slides in at the top, 314×68); the two never appear at the
  same time (`match_ui::busy` / `agents_ui::busy`). Waiting: red frame, "Claude is waiting for you", project, reason,
  10 s, tap for the session page. Done: green frame, "Claude finished its job", "took 7 min", 6 s. At most 4
  notifications queue up. The waiting box for the same session appears at most once a minute (so a session that keeps
  stopping for permission does not cover the screen every time).
- **Rumy:** on a waiting box it looks at the star and blinks (`rumy::poke`, `scr_home::glance`); on a done box it is
  `HAPPY` for 4 s (`agents_ui::reaction`, next to the match reaction in `pickMood`).
- **WiZ alert** (`wiz::alert`): the same mechanism as the celebration (`handleParty`); the bulb's current state is
  saved, 400 ms red 100% → 400 ms previous state → 400 ms red → previous state. At most once every 30 s. Not done if
  the bulb is offline or its state is unknown. Try it with serial `wiz uyari`.
- In night mode (while not awake) there is no box, reaction or bulb alert; the star and the page keep updating.

### 28.3 Session page (`src/ui/scr_agents.cpp`, `SCR_AGENTS`)

The header has the state's star (animated), "Claude" and a summary ("2 waiting · 2 working · 1 done"); below it at
most 6 rows (34 px): a small star, the project name, and on the second line what it is doing and for how long
("Editing files · 3 min", "Asking permission: Bash · 44 s", "Done · 12 min ago · took 7 min", "New session · 2 min
ago"). Durations advance on the board every second (`agents::spanS`). Empty states: "No open Claude session", "Bridge
not connected". Opened by tapping the star on the home screen or with `ekran ajan`; swipe right to go back. 3 KB in
the LVGL needs table.

### 28.4 Computer side

- **Hook** (`tools/claude_hook.py`): on every event Claude Code runs `pythonw -S claude_hook.py` `async` (exec form,
  no shell). The script reads the event JSON from stdin, reads and updates
  `~/.claude/rumy/oturumlar/<session>.json` with a lock file (O_EXCL, stale after 2 s) and writes it atomically
  (temporary file + `os.replace`). It does not overwrite a file written by a newer event (async hooks may finish out of
  order). It never fails and produces no output. Measured time 52–72 ms per event (including pythonw start-up); since
  it is `async`, Claude does not wait.
- **Event mapping:** `SessionStart` (startup/resume/clear) → done "New session" (`compact` does not change the state);
  `UserPromptSubmit` → working "Thinking" (job start); `PreToolUse` → working + a description of the tool (Bash/PowerShell
  "Running a command", Edit/Write "Editing files", Read/Grep/Glob "Reading code", Web/browser "Looking at the web",
  Agent "Helper agent running", MCP "Using a tool"); `AskUserQuestion` / `ExitPlanMode` → waiting;
  `PostToolUse(Failure)` → working "Thinking"; `PermissionRequest` → waiting "Asking permission: <tool>";
  `Notification` (`permission_prompt`, `elicitation_*`, `agent_needs_input`) → waiting, `idle_prompt` is ignored;
  `Stop` → done, job time = now − job start; `StopFailure` → waiting "API error: usage limit" etc.; `SessionEnd` → the
  file is deleted. The recorded `pid` is the hook's parent process, i.e. the session's `claude.exe` (measured in the
  desktop app: the `claude.exe` opened at session start, a child of the main `claude.exe`). These texts are written by
  the hook in Turkish; the board shows them as they come.
- **Watcher** (`tools/claude_agents.py`, started as a thread when the bridge opens): reads the folder every 0.5 s (an
  unchanged file is not reread, `mtime`), deletes files silent for 24 hours, hides finished sessions older than 30 min
  and sessions silent for 60 s whose process has also exited (`OpenProcess` + `GetExitCodeProcess`); orders rows
  waiting > working > done, takes the project name from the working folder (the worktree name for
  `.claude/worktrees/<name>`), and shortens texts to byte limits (project 27, detail 39). It sends when the content
  changes (at most every 0.5 s) or every 10 s; it sends at once when the bridge reconnects to Rumy. It writes state
  transitions to the bridge log (`claude: Rumy -> seni bekliyor (İzin istiyor: Bash)`).
- **Setup:** `python tools/claude_agents.py --kur` adds hooks for 10 events (renewing earlier Rumy hooks, leaving other
  hooks alone), first taking a `settings.json.rumy-yedek-<date>` backup; `--kaldir` removes only Rumy's; `--durum`
  prints the list; `--deneme` prints the lines that would go to Rumy. After setup, open sessions in the desktop app show
  up without restarting.

### 28.5 Protocol and board (`lib/rumy_core/src/agents_proto.*`, `src/agents/agents.*`)

    @ajanlar <version-hex> <rows> <working> <waiting> <idle>
    @ajan⇥order⇥id-hex8⇥c|b|i⇥state_s⇥job_s⇥project⇥detail

The counts are for the whole list (with more than 6 sessions the page shows "+N more sessions"). The ID is the CRC32
of the session number. For working, `state_s` is the time since the job started; for the others, the time since the
state began; `job_s` is the last job's duration for done. Rows reach `agents::head/row` from `media::lineEnd` in the UI
task; once all rows have arrived the list takes effect at once (`commit`), `agents::version()` goes up and the UI
dispatches the `M_AGENTS` mask. If no header comes for 30 s the list is cleared. The parser and the state diff are
tested on the computer (`test/test_agents`, 8 tests: header, row, shortening without splitting UTF-8, overall state,
notification diff, silent states, limits, duration text).

### 28.6 Memory and verification

- Static RAM +2 136 B (two `rc::Agents` copies on the board, the previous list in the UI and the notification queue);
  app 2 145 344 B. LVGL: in the simulator home screen + star 8.8 KB, session page 9.8 KB, peak 12.4 KB (64-bit
  preview); the page need was added to the table as 3 KB.
- Simulator: `RUMY_SIM_ONLY=ajan python tools/ui_preview.py --film` → 11 screens and 4 films (the light spinning,
  pulsing, a notification entering, the page); automatic audit 0 findings.
- On the computer: the hook was tried with a 12-event sequence (every transition right, 52–72 ms); after setup this
  session's state ("Rumy · Running a command") appeared in the bridge log.
- 4 environments without warnings, 135/135 tests.

### 28.7 Codex (25 October 2026)

- **Hooks:** `~/.codex/hooks.json` (did not exist, was created), 8 events: `SessionStart`, `UserPromptSubmit`,
  `PreToolUse`, `PostToolUse`, `PermissionRequest`, `Stop`, `Interrupt`, `SessionEnd`; command
  `<python>/pythonw.exe -S .../tools/claude_hook.py codex` (paths without spaces; Codex runs the command through a
  shell, and an unquoted path works the same in cmd, PowerShell and bash), `async`, timeout 3 s (the limit for
  `SessionEnd` and `Interrupt`). Codex keeps trust in a hook by a digest of its definition and its order in the file;
  so setup does not move other tools' groups, and removal turns the Rumy group that stays in front into an empty
  placeholder (the same method as ClawLight). Codex's `config.toml` is never written to: new hooks do not run until
  the user trusts them inside Codex (`/hooks` or the app's warning). `codex features list` → `hooks stable true`.
- **Event mapping:** the same as Claude's; in addition `Interrupt` (the user stopped the job) → done "Stopped",
  `SessionEnd` (conversation archived/deleted or closed for 30 min) → the file is deleted, and a one-shot `codex exec`
  session is deleted on `Stop` (`source: exec` in the first line of the conversation log). Codex tool names:
  `apply_patch` → "Editing files" (in a permission request "Asking permission: file edit"), `update_plan` → "Making a
  plan", `spawn_agent` → "Helper agent running", `Bash` → "Running a command".
- **Session process:** the hook now records the first `claude.exe` or `codex.exe` in its ancestor chain instead of the
  parent process (a Toolhelp snapshot, ctypes; the parent process if none is found). Codex starts the hook through a
  shell, so the parent process closes at once and the process check would have dropped the session by mistake.
- **Name:** a Codex session's name is Codex's own conversation title: `~/.codex/state_*.sqlite` → `threads.title`
  (read-only with `mode=ro`), otherwise `session_index.jsonl`, otherwise the working folder. If no title is found it is
  looked up again every 30 s.
- **Protocol:** an agent field was added to the `@ajan` line (`claude|codex`, after the state and durations; 8
  fields). The board had not been uploaded with the old format yet, so no backward compatibility was needed.
- **Board:** `rc::AgentRow::agent`, `rc::agentName`; the notification title "Codex is waiting for you" / "Claude
  finished its job", the project name in small text right of the title, the reason on the line below at full width (a
  long reason used to cut the project to "S…"); the session page header "Coding agents", the agent name on the right of
  each row; the empty page "No open agent session · Appears when you start a job in Claude or Codex"; test with
  `ajan test codex`.
- **Verification:** the hook was tried with the Codex event sequence (9 events, every transition right, 48–86 ms, the
  right process from the ancestor chain); 135/135 tests (a Codex row and `agentName` added to `test_agents`); 12 images
  in the simulator, audit 0 findings.

---

## 29. Wireless update (WiFi)

New firmware goes to the board without a USB cable, over the bridge's WiFi connection (TCP 7777, keyed):

```bash
python tools/spotify_bridge.py --guncelle                               # dist/Rumy.bin
python tools/spotify_bridge.py --guncelle dist/Rumy-guncelleyici.bin    # the updater itself
```

### 29.1 Why SD card + updater

| Method | Measurement | Result |
|---|---|---|
| Classic dual OTA (two app partitions) | 2 × 2.15 MB > 3.01 MB (space left for partitions) | Does not fit |
| A small updater with WiFi | 899 168 B | The partition shrink squeezes the app |
| **SD + updater with a screen** | 414 336 B measured, final 421 968 B | **Chosen** (448 KB `ota_1`) |

While the firmware arrives over WiFi, the main app keeps running and writes the file to the SD card; the one that
writes the main partition is the small updater that boots on restart. So the main partition is never written over
itself.

### 29.2 Flow

```
Computer (--guncelle)        Bridge                Rumy (app0)                     Updater (ota_1)
  check the image
  request file ────────────▶ @guncelle <bytes> <sha> ─▶ space/SD/partition check
                             ◀── @guncelle git
                             2048 B data ──────────▶ append to SD, two SHA-256s
                             ◀── @guncelle ok <n>      (on every chunk)
                                                       digests match → .bin + .sha
                                                       otadata → ota_1
                             ◀── @guncelle tamam <elf8>
                                                       restart ─────────────────────▶ verify the SD file again
                                                                                     erase and write app0 (3 tries)
                                                       ◀─────────────────── restart  otadata → ota_0, clean the SD
  VERIFIED ◀─────────────────── @merhaba ... <elf8> <updater elf8>
```

(`git` = go, `tamam` = done.)

- **Computer** (`tools/fw_update.py`): checks that the image is an ESP32 image (0xE9, chip ID 0, app descriptor, the
  "hash appended" flag), that it fits the partition and that the SHA-256 at its end matches the content; a corrupted
  file is not sent. If the bridge is running it hands the request over with a file and follows progress through
  `guncelle_durum.txt` (giving up if the bridge is silent for 45 s); if not, it connects to the `--pi/--wifi` address
  (or USB) itself. If Rumy already runs the same firmware it is not sent (`--zorla` sends it). An image containing the
  text "Rumy güncelleniyor" (Rumy is updating) counts as the updater.
- **Board** (`src/media/fwupdate.*`): on the offer it checks that the partition, the updater, the SD card and space
  (image + 256 KB) are there; during the transfer `bridge::bulk` is on (HTTPS requests are postponed). Each chunk is
  appended to `/rumy/guncelleme.part` on the SD card (open-write-close per chunk because `max_files = 1`), and the
  SHA-256 of the whole file and of its body without the last 32 bytes are computed together. After the first chunk the
  image header is checked (an updater image cannot go to the main partition). At the end, if both digests match, the
  file becomes `guncelleme.bin` with `guncelleme.sha` (`<sha256> <bytes>`) beside it, the boot partition is set to the
  updater, and it restarts after 1.5 s. If no data comes for 6 s the transfer is dropped and the half file deleted.
- **Updater** (`updater/src/main.cpp`): shows progress with LovyanGFX and the setup screen's font (following the
  screen rotation setting in NVS, and the language setting since section 31); re-verifies the SD file's SHA-256, body
  digest and header **before erasing the main partition**, then writes it to `app0` with `esp_ota_begin/write/end`
  (three tries), sets the boot partition to `app0` and deletes the files on the SD card. If the file is missing or
  corrupted it boots the old firmware; if it cannot write the main partition it says "Install over USB".
- **The updater itself:** if the offer ends with the word `guncelleyici` (updater), Rumy writes the image straight to
  `ota_1` without touching the SD card (`OTA_WITH_SEQUENTIAL_WRITES`, erasing sector by sector), `esp_ota_end`
  verifies, and it restarts.

### 29.3 Screens

Board side (`src/ui/update_ui.*`, LVGL, updated every second, blocks touches): "Receiving update" + a bar + "41% ·
0.9 / 2.1 MB"; then green eyes with "Update received · Restarting…"; on an error red eyes with "Update failed" and the
reason (connection lost, file arrived corrupted, could not write to the SD card...) for 6 s. The updater uses the same
layout: "Rumy is updating" + "Verifying / Writing · 37%"; "Update complete"; on an error "Update could not be done" +
the reason + "Starting the old firmware" or "Install over USB". Simulator scenario:
`RUMY_SIM_ONLY=guncelle python tools/ui_preview.py --film`. Images: `docs/preview_guncelleme/` (`01`–`05` board,
simulator; `g1`–`g5` updater, drawn with the VLW glyphs of the setup font), all together in
`docs/ui_guncelleme.png`.

### 29.4 Protocol and diagnostics

- `@guncelle <bytes> <sha256> [guncelleyici]` → `@guncelle git` | `@guncelle hata <reason>`; `@guncelle ok <n>` every
  2048 bytes; `@guncelle tamam <elf8>` at the end. Reasons: `bicim` (format), `guncelleyici yok` (no updater), `bolum`
  (partition), `boyut` (size), `sd yok` (no SD), `sd dolu` (SD full), `mesgul` (busy), `sd yazma` (SD write), `okuma`
  (read), `imaj` (image), `imaj ozeti` (image digest), `guncelleyici imaji` (updater image), `sha`, `zaman asimi`
  (timeout), `acilis bolumu` (boot partition), `yazma` (write).
- `@merhaba <cover> <camera> <app_elf8> <updater_elf8>`: the first 8 digits of the ELF digest are unique to a build (in
  Arduino the version/date fields of the app descriptor come from the core library and do not tell builds apart).
- `rapor`: `[guncelleme] bekliyor 0/0 B · calisan 659df252 (app0) · guncelleyici e7cb576d (448 KB) · basarili 0 ·
  hata 0 · son -` (pending, running, updater, succeeded, errors, last).
- Shared parsers in `lib/rumy_core/src/fw_image.h` (the board and the updater use the same file), tests in
  `test/test_fwimage`.

### 29.5 Measurements (25 October, on the board)

| Measurement | Value |
|---|---|
| Main firmware transfer (2.15 MB, WiFi, to SD) | 67–69 s, ~32 KB/s |
| Total from command to verification (including the updater + reconnecting) | ~95 s |
| Updater transfer (422 KB, straight to flash) | 9 s, ~48 KB/s; 16 s in total |
| Free heap (`rapor` after boot) | 54.3 KB → 53.1 KB (static +~1.2 KB) |
| Bridge killed mid-transfer (at 841 KB) | Rumy reported "zaman asimi" (timeout) 6 s later, the half file was deleted, the old firmware kept running |
| The same firmware requested again | Not sent ("already this firmware") |
| Corrupted image (one byte changed) | Rejected on the computer |

---

## 30. Phone app (iPhone)

### 30.1 The chosen approach

| | Web app added to the home screen | Native iOS app |
|---|---|---|
| Needs | Safari → Share → Add to Home Screen | Mac + Xcode |
| Apple account | Not needed | Free: reinstall every 7 days; permanent: yearly developer account |
| Updates | Together with Rumy's firmware (wireless update) | Separate build and install |

Since the computer runs Windows, the first approach was chosen. The page lives in Rumy's flash; when Rumy is updated,
the app is updated too.

### 30.2 Screens

- **Pairing:** eyes, "Connect to Rumy", a 4-box code field (`autocomplete=one-time-code`, numeric keypad).
- **Home page:** a status card (online / sleeping / unreachable, address, version, uptime) and a **Sleep now / Wake
  up** button; below it a list in iOS Settings style: General, Display, Football, Light, 3D printer, Special days,
  Maintenance. While the home page is open the status refreshes every 15 s.
- Changes are saved at once (collected for 500 ms into a single `POST /api/settings`), with "✓ Saved" at the bottom.
- **Football:** the 18 Süper Lig teams with their ESPN numbers on the page (from the 27 October 2026 table); if the
  selected team is not in the list it is added as "Team N".
- If unreachable the list fades and "Try again" appears; the last-seen time is kept on the phone.

### 30.3 Rumy side

- `src/ui/scr_phone.cpp`: QR (`lv_qrcode`, `LV_USE_QRCODE` + `LV_USE_CANVAS` turned on), three steps, a large code,
  `rumy.local · IP`, the number of paired phones and "Forget". The QR is `http://rumy.local/` if mDNS is on, otherwise
  the IP address.
- The "Web interface" toggle on the Settings screen was replaced by a "Phone" row (with the paired count).
- `wiz.cpp`: checks the settings before an alert/celebration starts; with `wonly` on it does not touch a bulb that is
  off (`wiz ... atlandi (ayar)`, "skipped (setting)", in the log). `match_ui.cpp`: `leagueWanted()` filters Süper Lig /
  Champions League goal notifications.
- A `[web]` line in the `rapor` output: on/off, mDNS, paired phones, requests, pairings and rejections.
- Tests: `test/test_phone` (sleep end, token format and comparison, code parsing; 7 tests, 151 in total).

### 30.4 On a computer

- If the window is ≥ 900 px wide the page switches to two columns (the `#shell` grid, a `wide` media query): on the
  left the status card, "Sleep now" and the section list always visible (the selected section highlighted), on the
  right the section's settings. Back buttons are hidden (except Days → editing a day); headings are large and
  left-aligned; hover highlights. The home page refresh keeps running in every section in the wide layout. The phone
  layout on narrow screens did not change.
- Phone-specific texts ("Your phone must be on the same WiFi" etc.) were generalised to "this device".
- `tools/pc_kisayol.py --kur`: makes `tools/assets/rumy.ico` from `web/icon.png` and puts `Rumy.lnk` on the desktop and
  in the Start menu: `msedge.exe --app=http://rumy.local/ --window-size=1180,800` (no address bar or tabs). Because the
  Start menu is under `AppData`, the install was run outside Claude's container with a one-off scheduled task.
  `--kaldir` removes both. The computer also uses one of the 4 paired devices.

### 30.5 Look (temporary expression choice)

- `src/rumy/look.*`: a request shared between tasks (expression, accessory, effect, sequence no.) and the look being
  shown. The web (`POST /api/look {"mood","acc","fx"}` or `{"auto":true}`) and serial `gorunum ...` write the request.
- `scr_home::updateMood` (every second): when a new request arrives it saves the signature of the current automatic
  look; when the automatic look moves away from that signature (weather, music, match reaction, night, earthquake,
  agent, eye sleep), the request is released. The look being shown appears as `look` in `/api/state` and on the
  `[gorunum]` (look) line of `rapor`.
- The expression pictures in the app are `web/yuzler.png` (13 × 80×56, ~3 KB), generated from the simulator with
  `python tools/ui_preview.py --yuzler`; `build_web.py` embeds it in `web_faces.h` and it is served from
  `/yuzler.png`.
- Tried on the board: `gorunum 1 2 13` → happy + party hat + confetti; when `hava kar` changed the state it returned to
  automatic.

---

## 31. Language support (English / Turkish)

Rumy was written in Turkish from the start. Language support makes every user-facing text available in English and
Turkish; **English is the default for new installs**, and Turkish is one tap away.

### 31.1 Choosing the language

- **Device:** Settings → **Language / Dil** (the row label is bilingual on purpose, so it can be found in either
  language). Tapping switches the language, saves it and restarts Rumy (`app::request(app::ACT_REBOOT)`) with a short
  toast.
- **Phone app:** General → **Language / Dil** (a select with English / Türkçe). The app sends `{"lang":0|1}` to
  `POST /api/settings`; the board saves it and restarts 1.5 s later. The page stores the choice in `localStorage`
  (`rumy.lang`) and reloads after 9 s.
- **NVS:** the `dil` key (0 = Turkish, 1 = English), default `rc::LANG_EN`, clamped by `sane()`. Devices upgraded from
  an older version have no `dil` key and therefore also come up in English; one tap switches back.

### 31.2 How it works on the board

- `lib/rumy_core/src/text_tr.h` holds the language helper: `enum Lang { LANG_TR = 0, LANG_EN = 1 }`,
  `rc::setLang()`, `rc::lang()`, `rc::langEn()` and **`rc::L(tr, en)`**, which simply returns one of two pointers.
  Both strings are literals in flash, so the helper costs no RAM and works with `lv_label_set_text_static`.
- The language is set exactly once, in `settings::load()` right after `sane()`, before any screen is built. Since a
  language change always restarts the board, nothing needs to be rebuilt at run time.
- **Static tables:** file-scope arrays (weekday and month names, wind names, league names, the fixed special days, the
  light scenes, the diagnostics keys, the licence list...) are filled during static initialisation, before
  `setLang()` runs, so `rc::L` cannot be used in them. They got parallel English arrays instead (`kDaysEn`,
  `kMonthsEn`, `kNamesEn`/`kAbbrEn` in `wind_tr.cpp`, `kLeaguesEn` in `espn.cpp`, `kFixedEn` in `events_calc.cpp`,
  `kSceneNamesEn` in `scr_light.cpp`, `kKeysEn` in `scr_diag.cpp`, `kLicensesEn` in `scr_info.cpp`), picked at the
  point of use. Function-local `static` arrays are fine, since they are initialised on first call.
- **Format strings:** `snprintf` with a translated format and no arguments is written as
  `snprintf(buf, cap, "%s", rc::L(...))` to keep `-Wformat-security` quiet.
- **Grammar:** Turkish dative suffixes and possessive forms have no English counterpart; `events_calc.cpp` uses an
  empty string for the dative in English, and gives English its own patterns ("Today: %s", "Tomorrow: %s",
  "%s · %d days", "Today is %s!", "%u %s · age %u", "%u %s · year %u"). Eid al-Fitr and Eid al-Adha both greet with
  "Eid Mubarak!".
- **What changes:** every screen (home, weather, light, settings and sub-pages, night, celebration, Spotify,
  playlists, printer, football pages and the league table headers (Pts, P, W, D, L, GD), match notifications, coding
  agents, diagnostics, about, phone pairing, update screens), the first setup portal (`portal.cpp`), the fatal error
  screen (`main.cpp`), network error texts (`http_min.cpp`, Pi `net_pi.cpp`), weather descriptions (`wmo_tr.cpp`),
  wind names, UV/AQI levels, the rain summary, dates (`formatDateTr` gives "Friday, September 25" in English), K2
  printer phases and coding agent durations.
- **What stays Turkish:** serial commands and log lines, NVS keys, file names, identifiers, and texts that come from
  outside (city and team names, song titles, earthquake place names, the Turkish activity texts written by the coding
  agent hook).
- **Updater:** `updater/src/main.cpp` reads `dil` from NVS at start-up (default English) and wraps its 16 screen
  strings in a small `tx(tr, en)` helper.

### 31.3 Phone app

- `web/index.html` carries a `TR_EN` dictionary and a `T()` function. Static HTML text is translated once on load
  with a DOM TreeWalker (text nodes plus `placeholder`, `aria-label` and `title`); strings built in JavaScript are
  wrapped in `T()`. `pct()` puts the percent sign after the number in English ("60%") and before it in Turkish
  ("%60"). Times use `toLocaleTimeString("en-GB")` in English.
- The page reads `LANG` from `localStorage` (`rumy.lang`, default English). If the device reports a different
  `s.lang` in `/api/state`, the page saves the device's value and reloads, so the app always follows the device.
- The `LEAGUES` and `TEAMS` lines are generated by `tools/make_teams.py` and are left untouched; league names are
  translated at display time with `T()`.
- The gzipped page in `src/webui/web_assets.h` grew to 18 389 B.

### 31.4 Verification and open points

- CI (`pio test -e native`, `pio run -e cyd -e cyd_st7789`) passes without warnings; `test_text` and `test_events`
  gained English checks (dates, wind, greetings, AQI, "Republic Day · 34 days", "Today: Republic Day", "Happy Republic
  Day!", "29 October · year 103"), and restore Turkish at the end so the other tests keep their expectations.
- The phone app was checked in both languages against a mock server: switching languages and the reload work, no
  console errors.
- **Not yet done:** a simulator audit run in English for every screen (some English strings are longer than the
  Turkish ones and may clip), and a test on real hardware.
