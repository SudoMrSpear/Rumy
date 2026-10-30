# Rumy

**English** · [Türkçe](README.md)

A desk companion for the ESP32-2432S028R "Cheap Yellow Display" (CYD, 2.8", 320×240). Rumy is a pair of glowing cyan
eyes on a black screen that react to the weather, your music, football and the time of day, next to a big clock,
the weather, earthquakes in Turkey, holidays and your own special days. It also drives a WiZ smart bulb.

![Rumy](docs/tanitim.png)

**[Install it from your browser →](https://sudomrspear.github.io/Rumy/)** (desktop Chrome or Edge, no tools needed)

> The on-device interface and the main documentation are in Turkish. This page is an English overview; the full
> details live in the [Turkish README](README.md) and the
> [technical document](Rumy_Proje_Dokumani.md).

## Features

- **Home screen:** Rumy's eyes, a big clock, a greeting, current weather with today's high/low, temperature /
  humidity / air-quality bars and a rotating info card (wind, rain in the next 6 hours with an "take an umbrella"
  hint, sunrise/sunset, sea temperature, air quality, special days, latest earthquake).
- **Weather screen:** colour gradient by condition, next-hour rain graph, a scrollable 24-hour strip, 7-day min/max
  bars and a 10-tile detail grid. Data from [Open-Meteo](https://open-meteo.com).
- **Earthquakes:** recent earthquakes around the Marmara region from AFAD, with Kandilli Observatory as a fallback, and a
  configurable magnitude threshold.
- **Live football:** follow any team in the Turkish Süper Lig or the top five European leagues. Kick-off time on
  match day, live score and minute, a full-screen "GOOOL!" in your team's colours, opponent goals, cards and
  substitutions, plus the full event list. Data from ESPN; no computer needed.
- **Spotify "now playing":** cover art, progress (drag to seek), playback and volume controls and your playlists,
  through a small PC bridge over USB or WiFi. While music plays, Rumy puts on headphones and nods to the beat.
- **Agent light for Claude Code and Codex** (a take on ClawLight): a small star shows whether your coding-agent
  sessions are working (green), waiting for you (red pulse, plus a banner and a red flash on the WiZ bulb) or done
  (yellow). Driven by local hooks; no account or internet needed.
- **3D printer (Creality K2):** print status, errors, ETA and a progress bar; the page opens itself on an error.
- **WiZ bulb:** on/off from the home screen, hue / brightness / white-temperature sliders and 8 scenes.
- **Special days:** Turkish national and religious holidays plus birthdays and anniversaries you add from the app,
  each with a full-screen card on the day.
- **Night mode:** dim screen, sleeping Rumy, a small clock; touch to wake for 30 seconds.
- **Phone app:** every setting lives in a web app served by Rumy itself (`http://rumy.local`). Add it to your iPhone
  home screen and pair it once with a 4-digit code.

More screenshots: [`docs/preview/`](docs/preview).

## What you need

| Item | Notes |
|---|---|
| ESP32-2432S028R (CYD) | 2.8" 320×240, resistive touch, SD slot. No PSRAM needed. |
| 2.4 GHz WiFi | The ESP32 cannot join 5 GHz networks. |
| microSD card (optional) | FAT32. Stores your special days and a cache of the last data. Everything else works without it. |
| WiZ bulb (optional) | Must be on the same network, without guest-network / AP isolation. |

**Panel type:** most CYDs use an **ILI9341** panel (the default). If colours look inverted or the image is garbled,
use the **ST7789** build.

## Installing

### Option 1: web installer (easiest)

1. Open **<https://sudomrspear.github.io/Rumy/>** in desktop Chrome or Edge.
2. Pick your panel type, connect the board over USB and press **Install**.
3. Tick **Erase device** in the dialog. **This wipes the board** (Bruce, Launcher or anything else on it).

### Option 2: Launcher

[Launcher](https://github.com/bmorcelli/Launcher) lets you keep several apps on the board.

1. If Launcher is not on the board yet, flash it with the
   [Launcher web flasher](https://bmorcelli.github.io/Launcher/webflasher.html) (device **CYD-2432S028**, or
   **CYD-2-USB** for boards with both USB-C and micro-USB).
2. Download `Rumy-<version>-ILI9341-launcher.bin` (or the ST7789 one) from
   [Releases](https://github.com/SudoMrSpear/Rumy/releases), copy it to the root of a FAT32 SD card as `Rumy.bin`.
3. In Launcher: **SD** → `Rumy.bin` → **Install**.

Wireless updates are not available with Launcher.

### Option 3: esptool

Back up what is on the board first, then write the full image to `0x0`:

```bash
python -m esptool --chip esp32 --baud 460800 read_flash 0 0x400000 backup_4MB.bin
```

```bash
python -m esptool --chip esp32 --baud 460800 write_flash --erase-all 0x0 Rumy-1.3.0-ILI9341-tam-0x0.bin
```

The CH340 USB bridge puts the board into download mode by itself. If it will not connect: hold BOOT, press and release
RST, release BOOT. 460800 baud is reliable; 921600 drops with some cables.

## First boot

1. Join the **Rumy-Kurulum** WiFi network with your phone (or scan the QR code on screen).
2. Tap **Configure WiFi**, pick your home network and enter its password.
3. Enter your **name** and **city** (a Turkish province, or its plate number such as `34`; custom coordinates can be
   set later in the app), then **Save**. If the page does not open, browse to `http://192.168.4.1`.

WiFi credentials and all settings are stored in the device's NVS, never in the code.

## Updating

To update **without losing your settings**, write only the application image:

```bash
python -m esptool --chip esp32 --baud 460800 write_flash 0x10000 Rumy.bin
```

If you installed with the full image (web installer or esptool) and the PC bridge is running, you can also update
over WiFi: `python tools/spotify_bridge.py --guncelle`. An SD card must be inserted. See *Kablosuz güncelleme* in
the [Turkish README](README.md#kablosuz-güncelleme-wifi).

## Using it

| Gesture | Result |
|---|---|
| Swipe left / right | Home ⇄ Weather ⇄ Light ⇄ Settings |
| Swipe right on sub-screens | Back |
| Tap the weather block | Weather screen |
| Swipe right on Home | Spotify screen (if the bridge is running), otherwise the printer page |
| Tap / long-press the bulb button | Toggle the bulb / open the Light screen |
| Tap the info card | Next card |
| Tap Rumy | Rumy looks where you touched and blinks |
| No touch for 60 s | Back to Home (configurable) |

The PC bridge (`tools/spotify_bridge.py`) powers Spotify, the agent light, printer camera and wireless updates. Its
setup and options are described in the [Turkish README](README.md#spotify-ekranı).

## Building from source

```bash
pip install platformio
pio run -e cyd
```

| Environment | Description |
|---|---|
| `cyd` | ILI9341, normal build → `dist/Rumy.bin`, `dist/Rumy-full-0x0.bin` |
| `cyd_st7789` | ST7789 panels → `dist/Rumy-st7789.bin` |
| `cyd_endurance` | Endurance test build |
| `cyd_prof` | Profiling build |
| `native` | Unit tests on your PC: `pio test -e native` |

`python tools/ui_preview.py` renders every screen on your PC with the real LVGL code and no board (needs MinGW-w64 g++
and Pillow). An experimental Raspberry Pi Zero W version lives in `pi/`.

## Troubleshooting

| Symptom | Fix |
|---|---|
| Inverted colours, shifted image | Use the ST7789 build |
| Touch lands in the wrong place | Settings → Display → touch orientation, or the serial command `dokunmatik 0-7` |
| Bulb not found | Same 2.4 GHz network? Guest network / AP isolation off? |
| WiFi changed | Settings → WiFi → Reset; Rumy returns to setup mode |
| Freezes or restarts | Send `rapor` over serial (460800 baud) and attach the output to an issue |

## Contributing

Bug reports and ideas are welcome as [issues](https://github.com/SudoMrSpear/Rumy/issues), in English or Turkish.
For code, read [CONTRIBUTING.md](CONTRIBUTING.md) first (Turkish): it lists the code rules and the checks every PR
must pass.

## License

Rumy's own code is licensed under [PolyForm Noncommercial 1.0.0](LICENSE): free for personal, hobby, educational and
research use, including modifications. Commercial use (selling it, or putting it into a paid product or service)
needs separate permission: open an issue or contact [@SudoMrSpear](https://github.com/SudoMrSpear). This makes Rumy
**source-available**, not open source in the OSI sense.

Third-party libraries, fonts, icons and data sources are listed in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Rumy is an independent hobby project and is not affiliated with or endorsed by ESPN, Spotify, WiZ (Signify), Creality,
Anthropic, OpenAI, Claw Light, AFAD, Kandilli Observatory, Open-Meteo or any league or club. Names appear only to
describe compatibility. Match data comes from public but undocumented services that may change or stop working.
