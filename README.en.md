# Rumy

**English** · [Türkçe](README.md) · [Install from your browser](https://sudomrspear.github.io/Rumy/)

A desk companion for the ESP32-2432S028R ("Cheap Yellow Display", CYD). Rumy is two glowing cyan eyes on a black
screen; it shows the time, the weather, earthquakes, holidays and your own special days, and controls your WiZ bulb.

![Rumy](docs/tanitim.png)

> **Language.** The device, the phone app, the setup portal and the wireless updater are in English by default.
> Switch to Turkish in Settings → Language / Dil on the device or in General → Language / Dil in the app (Rumy
> restarts). Serial console commands and log lines are in Turkish; this README gives their meaning next to each one.

- **Home screen:** saturated neon colours on black. Rumy's eyes, the location in green, a bulb button, a big clock
  with a blinking colon (hours green, minutes white), and to its right a left-aligned column: a greeting at the top
  ("Good night Deniz"), below it a big weather icon with the temperature coloured by how warm it is, and below that
  today's high/low; the weekday in yellow and the date in green, temperature / humidity / air quality bars, and an
  info card that changes every 5 s (wind name, 6-hour rain and "Take an umbrella", sunrise/sunset, sea temperature,
  air quality, special day, latest earthquake).
- **Spotify screen:** cover art, title, progress (drag to seek) and touch controls for the song playing on a computer
  connected over USB or WiFi; the button in the corner of the cover opens your Spotify playlists (with covers), and
  the one you tap starts in Spotify on the computer (Premium, sign in once). While music plays, Rumy on the home
  screen puts on headphones and nods to the beat; a spinning record (purple centre) and the song title (scrolling if
  it does not fit) appear at the top of the column right of the clock. With no music the same spot shows a faded
  record and "Nothing playing"; the home screen layout never changes.
- **3D printer (Creality K2):** print status, errors, finish time and a progress bar; on an error the page opens by
  itself (at night too). If the PC bridge is running, the printer's camera is shown as well.
- **Football:** any team from the Süper Lig or the top five European leagues (Premier League, LaLiga, Serie A,
  Bundesliga, Ligue 1), picked in the app or switched off entirely with one toggle. On match day the bottom card shows
  kick-off; during the match the live score and minute; on a goal a full-screen "GOOOAL!" in your team's colours,
  and opponent goals, yellow/red cards and substitutions on top of the screen for 6 s. Tap the card for every event
  of the match. Data comes from ESPN, no computer needed; no notifications in night mode.
- **Coding agent light (adapted from ClawLight):** the state of your Claude Code and Codex sessions on the computer
  appears as a small star on the home screen: it spins green while working, pulses red while waiting for you and
  turns yellow when done. When a session starts waiting for permission or an answer, a "Claude is waiting for you"
  (or "Codex is waiting for you") box appears at the top and the WiZ bulb blinks red twice; when a job longer than
  2 minutes finishes, "Claude finished its job". Tap the star for the session list.
- **Eye sleep:** off by default; Rumy only sleeps in night mode. If you pick a time in Settings → Display, it also
  sleeps after that long without a touch and wakes up drowsily when touched.
- **Weather screen:** a gradient that changes colour with the conditions, a 64 px icon, a rain chart for the next
  hour, a horizontally scrolling 24-hour strip (sunrise/sunset included), 7-day min–max bars and a 10-tile details
  grid.
- **Light screen:** hue, brightness (in the lamp's colour) and white-temperature sliders with an icon on the left and
  the value on the right; the knob of the mode not in use is a hollow ring. 8 scenes for everyday use (Morning, Work,
  Reading, Dinner, Evening, Movie, Candle, Night); each button shows its icon in the lamp's colour for that scene, the
  selected one is framed in its own colour, and the bulb in the header takes the lamp's colour. Tap the box on the
  home screen to toggle the bulb, long-press to open the light screen.
- **Special days:** Turkish national holidays and remembrance days, Eid al-Fitr and Eid al-Adha (Nager.Date + the
  Diyanet table), and birthdays and anniversaries you add in the phone app. A full-screen celebration card on the day.
- **Night mode:** the screen dims, Rumy sleeps, a small clock stays; touching wakes it for 30 s.
- **Phone app:** a Rumy app you add to the iPhone home screen; all settings, a "Sleep now" button, team and bulb
  options, special days (pair once).

## Contents

1. [Before you start](#1-before-you-start)
2. [Preparing the SD card](#2-preparing-the-sd-card)
3. [Installing with Launcher](#3-installing-with-launcher-recommended)
4. [Installing over USB](#4-installing-over-usb)
5. [First boot](#5-first-boot)
6. [Using Rumy](#6-using-rumy)
7. [Phone app](#7-phone-app)
8. [RAM and flash budget](#8-ram-and-flash-budget)
9. [Development](#9-development)
10. [Endurance test](#10-endurance-test)
11. [Troubleshooting](#11-troubleshooting)
12. [Licences](#12-licences)
13. [Raspberry Pi version (experimental)](#13-raspberry-pi-version-experimental)
14. [Contributing, security and legal note](#14-contributing-security-and-legal-note)

## 1. Before you start

| You need | Note |
|---|---|
| ESP32-2432S028R (CYD) | 2.8" 320×240, resistive touch, SD slot. No PSRAM needed. |
| microSD card (optional) | FAT32. For your special days, holiday dates and a backup of the latest data. Icons are in flash, so everything works without a card; only your special days cannot be saved. |
| 2.4 GHz WiFi | The ESP32 cannot join 5 GHz networks. |
| WiZ bulb (optional) | Must be on the same network. It cannot be found on networks with "guest network" or "AP isolation" turned on. |
| Case (optional) | knkeeb's "CYD Desk Stand 2.8"" model (MakerWorld, model 2880776) keeps it upright on the desk. The print file is not in this repository; download it from the model's own page under its licence. |

Panel type: most CYDs use an **ILI9341**, which is the default. If colours are inverted or the picture is garbled,
use the **ST7789** build.

Ready-made images are on the GitHub **Releases** page: a Launcher file for each panel
(`Rumy-<version>-ILI9341-launcher.bin`, `…-ST7789-launcher.bin`) and a full image written over USB
(`Rumy-<version>-ILI9341-tam-0x0.bin`, `…-ST7789-tam-0x0.bin`; *tam* means *full*). If you build from source the same
files land in `dist/` (`Rumy.bin`, `Rumy-full-0x0.bin`; see [Development](#9-development)).

## 2. Preparing the SD card

1. Format the card as FAT32 (other files on the card, Bruce's for example, can stay). A card with several partitions
   works too: Rumy uses the first FAT32 partition (tested with a card holding a Raspberry Pi image).
2. If you will use Launcher, copy the Launcher file (`Rumy-<version>-ILI9341-launcher.bin`, or `dist/Rumy.bin` if you
   built it) to the root of the card as `Rumy.bin`.

There is nothing else to prepare. Rumy creates `/rumy/cache` (latest weather/earthquake data), `/rumy/holidays`
(holiday dates) and `/rumy/events.json` (your special days, added from the phone app) itself. Weather icons have been
built into flash since v1.1.0; the `sd_card/rumy/img` folder is only the icons' source.

## 3. Installing with Launcher (recommended)

[Launcher](https://github.com/bmorcelli/Launcher) lets you keep several apps side by side on the board. `Rumy.bin`
contains only the app; Launcher installs it into a partition it creates, and settings stay in NVS.

1. If the board does not have Launcher yet: on the computer, open the
   [Launcher web flasher](https://bmorcelli.github.io/Launcher/webflasher.html) in Chrome/Edge, choose
   **CYD-2432S028** as the device (**CYD-2-USB** for boards with two USB ports, USB-C + micro-USB), connect the board
   over USB and flash. This step erases the current firmware (e.g. Bruce).
2. Insert the SD card (with `Rumy.bin` at its root) and power the board on.
3. In the Launcher menu choose **SD** → `Rumy.bin` → **Install**. Rumy starts when installation finishes.

## 4. Installing over USB

If you do not want Launcher you can write the full image directly. **Everything on the board (Bruce, Launcher) is
erased.** Make a full backup of the current firmware first; to go back, write the same file to address `0x0`:

```bash
python -m esptool --chip esp32 --baud 460800 read_flash 0 0x400000 yedek/eski_yazilim_4MB.bin
```

```bash
python -m esptool --chip esp32 --baud 460800 write_flash --erase-all 0x0 Rumy-1.3.0-ILI9341-tam-0x0.bin
```

Without Python installed, the easiest way is the **[web installer](https://sudomrspear.github.io/Rumy/)**: open it in
Chrome or Edge on a computer, choose the panel type, connect the board over USB and press **Install**; tick **Erase
device** in the window that opens. The page writes the latest release's full image
(`.github/workflows/web_kurulum.yml` republishes it for every new release). You can also write the same file with
Espressif's browser flasher (address `0x0`).

The CH340 bridge on the CYD puts the board into download mode by itself; no need to press BOOT. If it cannot connect,
hold BOOT, press and release RST, then release BOOT: the board waits in download mode until it is restarted.
921600 baud drops with some cables; 460800 has been reliable. The `yedek/` (backup) folder is in `.gitignore` because
the old firmware's NVS may contain WiFi passwords.

**Updating (settings are kept):** write only the app; WiFi, name and location stay in NVS.

```bash
python -m esptool --chip esp32 --baud 460800 write_flash 0x10000 dist/Rumy.bin
```

Or from source (see [Development](#9-development)):

```bash
pio run -e cyd -t upload
```

### Wireless update (WiFi)

While Rumy is connected to the bridge over WiFi (see [Spotify screen](#spotify-screen), `--pi/--wifi`) you can send
new firmware wirelessly; the cable is only for power:

```bash
python tools/spotify_bridge.py --guncelle
```

The default file is `dist/Rumy.bin` (give `dist/Rumy-st7789.bin` on an ST7789 board). If the bridge is running the
request goes through it; otherwise the command connects by itself. While Rumy writes the file to the SD card the
screen shows "Receiving update" and a progress bar (2.1 MB in ~68 s); then it restarts into the small updater, which
verifies the file again, installs it and boots the new version. The command prints "TAMAM" (OK) once Rumy reconnects
and reports the new version (~95 s in total). If the connection drops or the file arrives corrupted, Rumy keeps
running the old firmware. If Rumy already runs the same firmware it is not sent (`--zorla` forces it).

- An SD card must be inserted (image + 256 KB free).
- The updater itself is refreshed the same way: `--guncelle dist/Rumy-guncelleyici.bin` (skips the SD card, ~16 s).
- **The first install is done over USB:** wireless updates need this repository's partition table (main partition
  2.5 MB + updater 448 KB); the full image or `pio run -e cyd -t upload` writes it. There is no wireless update in a
  Launcher install.
- The `[guncelleme]` line in Rumy's `rapor` output shows the running version, the updater and the last result.

## 5. First boot

Without WiFi credentials Rumy shows a plain setup screen without starting LVGL at all:

1. Join the **Rumy-Kurulum** network with your phone (you can scan the QR code on the screen).
2. On the page that opens press **Configure WiFi**, pick your home network and type the password.
3. Fill in **Your name** and **City (province)** (suggestions appear as you type a province name; "istanbul" or the
   plate number "34" also work).
4. **Save**. Rumy connects and restarts. If the page does not open by itself, go to `http://192.168.4.1` in the
   browser.

The WiFi password and all settings live in the device's NVS memory, not in the code.

## 6. Using Rumy

| Gesture | Result |
|---|---|
| Swipe left / right | Home ⇄ Weather ⇄ Light ⇄ Settings |
| Swipe right on sub-screens | Back |
| Tap the weather block (icon, temperature) | Weather screen |
| Swipe right on the home screen | Spotify screen (if the bridge is running; otherwise the printer page) |
| Swipe right on the Spotify screen | 3D printer page (if a printer is set) |
| Tap the print card | 3D printer page |
| Tap the song strip with the record on the home screen | Spotify screen |
| Tap the Claude star under the city name | Claude sessions page (the star only shows while a session is open) |
| Tap / long-press the bulb button at the top right | On–off / Light screen |
| Tap the info card | Next card |
| Tap Rumy | It looks where you touched and blinks |
| 60 s without a touch | Back to the home screen (adjustable) |
| No touch (if eye sleep is on) | Rumy sleeps and wakes when touched (Settings → Display → Eye sleep; default "Off") |
| Drag / tap the progress bar on the Spotify screen | Seeks to that second |

The bulb button's frame shows its state: yellow = on, grey = off, red = bulb unreachable.

**Rumy's expressions:** normal (also with the light on; the eyes take on a hint of the bulb's colour), sleepy (a short
moment while waking), asleep (night), sad (no connection), surprised (M4+ earthquake, 30 s; wakes up if asleep),
looking up + umbrella (rain approaching), celebrating + flag/party hat + confetti (special day), calm + flag
(10 November), listening to music + headphones (while music plays on the computer). With the light on, the eye colour
shifts slightly towards the bulb's colour.

**Weather expressions** (1 October; when none of the above applies):

| Weather | Rumy |
|---|---|
| Sunny | Happy, pink cheeks, hopping |
| Sunny and hot (feels like ≥27 °C or UV ≥6) | Sunglasses, glints on the lenses |
| Sweltering (feels like ≥33 °C) | Dozing, a sweat drop, rising heat waves |
| Overcast | Calm; looking at a cloud drifting by |
| Foggy | Squinting, peering between fog bands |
| Drizzle / rain | Umbrella, drops and splashes on the ground; sad and teary in rain |
| Snow | Happy, pink cheeks, snowflakes |
| Thunderstorm | Scared; the face strip lights up on lightning, the eyes flinch, trembling |
| Windy (wind ≥35 or gusts ≥50 km/h) | Squinting, leaning into the wind, wind lines, a flying leaf |
| Cold (feels like ≤2 °C) | `>  <` squeezed eyes, ice blue, shivering, breath vapour |
| Clear night | Calm, crescent moon and twinkling stars |

To try them from the serial console: `hava gunesli|sicak|bunaltici|kapali|sisli|cisenti|yagmur|kar|firtina|ruzgar|soguk|gece`
(sunny|hot|sweltering|overcast|foggy|drizzle|rain|snow|storm|wind|cold|night; shown for 2 minutes), `hava kapat`
returns to the real weather. While asleep (eye sleep and the night screen) small "z"s are born beside the right eye
and drift up, growing and swaying gently, then fade; meanwhile only the z's small area is redrawn at 20 fps. Night
mode is 02:00–07:00 by default (Settings → Night mode, or `gece 02:00 07:00` on the serial port).

### Spotify screen

While Rumy is connected to the bridge on the computer over USB or WiFi it shows the song playing there: a 144×144
cover, title, artist, album, a progress bar and touch controls (volume −, previous, play/pause, next, volume +). The
volume buttons change only Spotify's volume by 5%, not Windows' master volume (the Spotify session in the Windows
volume mixer; the same value as Spotify's own slider; needs `pycaw`, otherwise it falls back to the Windows volume
keys). When you press a button or the volume changes elsewhere, the progress bar turns into a "Spotify volume 60%" bar
for 2 s. A tap changes the volume one step at once; holding it adds a step every 0.15 s after 0.4 s (0 to 100 in
~3 s). Late volume replies from the computer do not make the bar jump back while you hold; the real value is shown
when the last reply arrives. The progress bar has a white handle: it grows while you drag and the target time is
written in pink; when you let go Spotify jumps to that second (a single tap on the bar works too; swiping on the bar
does not change the page). The accent colour is picked from the cover. When music starts and Rumy is on the home
screen it switches to this screen by itself (not on song changes); it goes back to the home screen if playback stays
paused for 1 min or the bridge closes. Open it by swiping right on the home screen, close it by swiping left.

**"Now playing" on the home screen:** while music plays, a small strip appears at the top of the column right of the
clock: on the left a record with a purple centre spinning at real record speed (33⅓ rpm), on the right the song title
in small text. If the title fits it stays still; if not, it waits 1.5 s and scrolls at 30 px per second, then waits
again after a full loop (fading softly at the edges). On pause the record stops and the text turns grey; if it stays
paused for 1 min or the bridge closes, a faded record and "Nothing playing" remain. The greeting and the weather are
always below this strip, so nothing moves when a song starts or ends. The date is centred under the clock; because
the weather icon reaches into the date row, if it does not fit the weekday is shortened first ("Mon 28 September"),
then the month ("Fri 28 Jul"). Tap the strip to open the Spotify screen. A picture of every weekday/month case:
`python tools/ui_preview.py --tarih` → `docs/ui_tarih.png`.

**Rumy listens too:** while music plays, Rumy on the home screen puts on neon-pink headphones, lowers its head a
little, narrows its eyes happily ("^ ^"), nods to the beat, and notes (♪) in the album's colour float out of the right
earcup. If `numpy` and `pyaudiowpatch` are installed, the bridge listens to every audio output, picks the device the
sound is actually flowing to (even if it is not the default, e.g. Bluetooth headphones), finds the beats and sends
the tempo to Rumy; the nodding locks onto the beat (in a synthetic test, tempo ±1%, beat timing ±20 ms). Because
Bluetooth headphones play sound ~150–200 ms late, the bridge delays the nodding by 200 ms on those devices by itself
(wired: 50 ms); `--beat-offset <ms>` sets it by hand. Without those packages Rumy nods at its own slow pace. Rumy does
not fall into eye sleep while music plays; in night mode it sleeps with the headphones on.

**Playlists:** the list button in the bottom-right corner of the cover opens the "My playlists" screen: each row has
the playlist's cover, name and track count; the playing list is green and highlighted, Liked Songs at the top. Tapping
a list shows "Starting on computer…" and a spinner in the row; once the list starts in Spotify on the computer you
return to the Spotify screen (if Spotify is closed the bridge opens it and waits 15 s). If the "Shuffle" button at the
top right is on, the list starts shuffled from a random song; the choice is stored in the bridge. Errors appear in
red in the row (e.g. "Spotify Premium required"). At most 30 lists are shown; the bridge caches lists for 5 min and
refreshes the playing list when the screen opens. Covers are written in 3×3 tiles to the same flash cache as song
covers (no RAM; 4 slots for 30 lists). This feature uses the Spotify Web API and needs a one-time setup:

1. https://developer.spotify.com/dashboard → **Create app**. Redirect URI: `http://127.0.0.1:8888/callback`
   (`localhost` is not accepted), API: **Web API**. Since March 2026 the account owner of an app in development mode
   needs Premium; the endpoint that starts a list only works with Premium anyway.
2. With the app's **Client ID** (no client secret needed, PKCE is used):
   `python tools/spotify_bridge.py --spotify-giris <client_id>`. Spotify's consent page opens in the browser; once you
   allow it the refresh token is written to `tools/.cache/spotify_api.json` (not committed, never printed). Run the
   command from a normal (non-administrator) PowerShell; it waits 15 min for consent.
3. Run the bridge as usual. Without a sign-in the list screen says "Spotify account not connected".

Run the bridge on the computer (song info needs no Spotify account or API key; it is read from the Windows media
session, the account is only for playlists):

```bash
python -m pip install pyserial pillow numpy pyaudiowpatch pycaw psutil winrt-runtime winrt-Windows.Media.Control winrt-Windows.Storage.Streams winrt-Windows.Foundation winrt-Windows.Foundation.Collections
python tools/spotify_bridge.py
```

- `--any`: if Spotify is not playing, show other players such as the browser or VLC. `--log`: also print Rumy's log.
- `--no-beat`: turns off beat tracking from the audio output. `--beat-offset 250`: increase if Rumy is ahead of the
  beat, decrease if it lags. `--send "rapor"`: sends a serial command to Rumy on connect.
- `--autostart ac` / `--autostart kapat`: starts the bridge without a console at Windows sign-in, or removes that.
  What starts is a **watchdog** (`--bekci`): it opens the bridge and reopens it 5 s after it closes for any reason;
  only one watchdog and one bridge run at a time. A "Rumy kopru" task is also added to Task Scheduler: it tries to
  start the watchdog every 5 min (does nothing if it is running; no time limit). `--durdur` stops the watchdog and the
  bridge (the task reopens it within 5 min; use `--autostart kapat` to stop it for good). Without a console, events
  are written with timestamps to `tools/.cache/kopru.log` (rotated to `.1` at 512 KB); the bridge does not close on an
  error, it logs it and reconnects (over WiFi if given together with `--wifi`).
- **Over WiFi (cable only for power):** connect the CYD to USB once and run
  `python tools/spotify_bridge.py --anahtar-yaz`. The bridge writes the key in `tools/.cache/pi_anahtar.txt`
  (generating it if missing) to the board; the board keeps it in NVS and the key is never printed. Then run
  `python tools/spotify_bridge.py --wifi <CYD's IP>` (TCP 7777). Songs, covers, camera, location changes, screenshots
  and commands sent from the bridge (like `rapor`, whose reply also comes back over WiFi) go over this connection.
  Giving the CYD a fixed address in the router is recommended so the IP does not change.
- In USB mode the serial port is busy while the bridge runs; close the bridge (Ctrl+C) before uploading to the board.
  In WiFi mode USB is free.
- Covers are kept in a separate flash partition (`art`, 960 KB, 21 slots); they use no RAM and show instantly when the
  same album comes again. A new cover arrives over USB in ~1.2 s.
- The idea and the protocol were adapted from open-source projects that send "now playing" over a serial port
  ([NowPLayingDisplay](https://github.com/RodgerE1/NowPLayingDisplay)) and from CYD Spotify displays
  ([Spotify-Diy-Thing](https://github.com/witnessmenow/Spotify-Diy-Thing), MIT).

### Football (ESPN)

Pick your team in the **Football** section of the phone or computer app: every team is listed with coloured badges
under the Süper Lig, Premier League, LaLiga, Serie A, Bundesliga and Ligue 1 tabs. "No team" keeps only goal
notifications from other matches; the **Match tracking** toggle at the top switches football off completely (Rumy
does not query matches, no notifications or match card; the Football page on Rumy can still be opened by hand). No
team is selected by default. From the serial port: `takim <espn-no> [colour1 colour2]` (e.g.
`takim 436 13235b ffd60a`), `takim kapat`, `futbol ac` / `futbol kapat`. Every 6 hours Rumy fetches your team's
fixtures in all competitions (league, cup, European cups) from ESPN and keeps the next match.

**Team colours:** the app sends Rumy two club colours along with the team. The goal screen background, the match card,
the highlight of "our" rows and the bulb's goal celebration are derived from them: the background is always dark, and
the accent is white if it does not stand out enough from the background. Without colours the navy-and-yellow default
is used. The app's team list and colours are generated from ESPN with `tools/make_teams.py` (Süper Lig club colours
come from a table in the tool); since leagues change every season, run it again at the start of each season.

- **Match day:** the bottom info card says "Today 21:45 · FB – Aston Villa"; 3 hours before kick-off the card stays
  pinned.
- **During the match** (every 20 s from 15 min before kick-off): the card turns into the live score in your team's
  colours ("FB 2 – 1 AVL · 67'", "HT" at half-time). On your team's goal, a full-screen "GOOOAL!" for 8 s (scorer,
  minute, score); opponent goals, yellow cards, red cards and substitutions (on ↑, off ↓) appear on top of the screen
  for 6 s. Tap to close. If the match is already under way when Rumy starts, earlier events are not shown as
  notifications.
- **Tap the card:** the match screen (competition, stadium, score, minute and every event, newest first; swipe right
  to go back).
- **After the match** the result stays on the card for 3 hours, then the next match is looked up.
- **Fast goal alerts:** goals arrive from LiveScore in ~20–45 s (ESPN is 50–100 s late). If the bridge on the computer
  is running, it watches your team's, Süper Lig and Champions League matches and sends them to the board (Rumy tells
  the bridge the selected team with an `@futbol` line); if the bridge is off, the board watches your team's match
  itself. ESPN details follow later; the same goal is never announced twice.
- **Player photo on the goal screen** (bridge running): the bridge finds the scorer's photo on TheSportsDB, Wikimedia
  Commons or Wikipedia and crops it square; photos only live in your computer's cache, not in the repository
  (`tools/player_photo.py`). If none is found, the scorer's initials are shown.
- **League days (Süper Lig and Champions League):** every match played that day is checked once a minute; when any
  match has a goal a "Galatasaray 1–1 Barcelona · scorer · minute" box appears at the top for 6 s (also while your
  team's match is on, after its events; the box carries that league's badge). Below the match screen, all of that
  day's matches are listed league by league with live scores.
- **League badges:** the repository has no club or competition logos; cards show plain abbreviated badges (SL, ŞL,
  AL, KL, TK, SK, PL, LL, SA, BL, L1). For your own use you can put logos at `tools/assets/lig_logolari/<name>.png` and
  run `python tools/make_match_art.py` to use them (the folder is outside git).
- **In night mode** no notifications appear and the screen does not wake; the card keeps updating.
- Source: ESPN's public, undocumented service (`site.api.espn.com`, `sports.core.api.espn.com`; no account or key,
  plain HTTP). If ESPN changes its format an update is needed. Delay ~20–60 s.
- To try it: `mac test basla` (start), `mac test gol` (goal), `mac test rakip` (opponent goal), `mac test sari`
  (yellow), `mac test kirmizi` (red), `mac test degisiklik` (substitution), `mac test devre` (half-time),
  `mac test bitti` (full-time), `mac test once` (before kick-off); `mac test kapat` returns to real data (the test
  ends by itself after 10 min).

### Football page

Opens by swiping right on the home screen: Süper Lig, Champions League, Premier League, LaLiga, Serie A, Bundesliga,
Ligue 1. Tap a league for its current matchday (live score, minute; ‹ › for the previous/next matchday). The
**Standings** button at the top right opens the table: position, played (P), wins/draws/losses, goal difference,
points; European cup / relegation colours on the left with their legend below. Data is fetched from ESPN only while
the page is open.

### Coding agent light: Claude Code and Codex (adapted from ClawLight)

[Claw Light](https://clawlight.dev) is a product that shows coding agents' sessions with a light on your desk: green
working, yellow idle, red waiting for you. Rumy does the same with its screen. The state comes from Claude Code's and
Codex's hooks (desktop apps, terminal and IDE sessions alike); no internet or account needed. Notifications and the
session page show which agent each session belongs to.

| Star on the home screen | Meaning |
|---|---|
| Green, slowly spinning | At least one session is working |
| Red, pulsing | A session is waiting for you: permission, a question, plan approval or an API error |
| Yellow | Sessions have finished, your turn (finished ones drop off the list after 30 min) |
| Not shown | No open session, or the bridge is off |

The most urgent state wins: if a session is waiting for you, the star is red even if others are working. If several
sessions share a state, their count is shown next to the star.

- **Notifications:** when a session starts waiting, a red "Claude is waiting for you · project · Asking permission:
  Bash" box shows at the top for 10 s (tap for the session page); Rumy looks at the star and blinks; the WiZ bulb
  flashes red twice briefly and returns to its previous state (at most once every 30 s). The box appears at most once
  a minute for the same session. When a job longer than 2 min finishes, a green "Claude finished its job · took 7 min"
  for 6 s and Rumy cheers for 4 s. In night mode there are no notifications or bulb alerts; the star keeps updating.
- **Session page ("Coding agents"):** each session's name, its agent on the right (Claude / Codex), and below what it
  is doing and for how long ("Editing files · 3 min", "Asking permission: Bash · 40 s", "Done · 12 min ago · took
  7 min"). A Claude session is named after its working folder, a Codex session after the conversation title in Codex.
  At most 6 rows; the rest is "+N more sessions".
- **Setup (once):** `python tools/claude_agents.py --kur` (`--kur claude` / `--kur codex` for only one). Background
  (`async`) hooks are added for 10 events to Claude Code's `~/.claude/settings.json` and for 8 events to Codex's
  `~/.codex/hooks.json`; the files are backed up first (`*.rumy-yedek-…`). **Codex asks for approval once before
  running new hooks:** when Codex shows the hook warning, or via `/hooks`, review Rumy's hooks and trust them. Codex's
  `config.toml` is not touched; other hooks in Codex stay where they are. Remove with
  `python tools/claude_agents.py --kaldir`. See sessions on the computer with `--durum`.
- **How it works:** on every event `tools/claude_hook.py` (pythonw, ~60 ms, never blocks the agent) writes the
  session's state to `~/.claude/rumy/oturumlar/<session>.json`. The bridge reads this folder every 0.5 s and forwards
  it to Rumy when the state changes (and every 10 s even if not). If the bridge is silent for 30 s the star goes out.
  Sessions whose agent process (`claude.exe` / `codex.exe`) has exited drop off the list. Interrupting a job in Codex
  shows as "Stopped".
- **Trying it:** serial `ajan test calisiyor` (working) → `ajan test bekliyor` (waiting) / `ajan test bitti` (done) /
  `ajan test codex` (the real state returns within 10 s if the bridge is running), `ajan` state report,
  `ekran ajan` opens the page, `wiz uyari` tries the bulb alert.

### 3D printer (Creality K2)

While the printer is on the network Rumy follows the print by itself (no computer needed; the local channel Creality
Print also uses, port 9999). Swiping right on the home screen opens Spotify, swiping once more opens the printer page
(directly the printer if the bridge is not running).

- **What is shown:** printer name, state (printing, paused, completed, stopped, error), job name and material, layer,
  nozzle/bed/chamber temperature, percentage, finish time ("Done 15:12"), a progress bar, time remaining and elapsed.
- **On an error:** the printer page opens by itself whatever screen you are on, the frame turns red and the printer's
  error code is shown; if the filament ran out it says "Filament ran out, print paused". At night the screen wakes for
  60 s to show it, and a red error line stays on the night screen.
- **Home screen:** while printing, "Print 45% · 1 h 12 min left" is added to the info cards (tap for the printer page);
  the night screen shows a dim "Print 45% · done 04:22".
- **Setup:** enter the printer's IP address in the "3D printer" section of the phone app (on the printer's screen:
  Settings → Network), or `yazici 192.168.1.50` on the serial port. Only an IPv4 address is accepted (not the printer's
  name, e.g. "K2-PRINTER"). Leave it empty to turn the feature off.
- **Camera:** the K2's camera only provides WebRTC video, which the ESP32 cannot decode. So the picture comes through
  the bridge on the computer: when the printer page opens, the bridge starts [go2rtc](https://github.com/AlexxIT/go2rtc)
  (with K2-specific support, reachable only from this computer), shrinks the picture and sends one frame per second
  over USB; it stops when you leave the page. With the chamber light off the picture is dark.

```bash
python -m pip install av
```

Download go2rtc from its [releases page](https://github.com/AlexxIT/go2rtc/releases) (`go2rtc_win64.zip`, 1.9.10 or
newer) and put it at `tools/go2rtc/go2rtc.exe` (the folder is in `.gitignore`).

**Pairing the bulb:** Settings → Bulb (or tap the light box) → tap your bulb in the list. The bulb blinks once. The
pairing is kept by MAC address; Rumy finds the bulb again even if its IP changes.

**Settings screen:** location (81 provinces), bulb, WiFi (reset with confirmation), the **Display** sub-page
(brightness, return-to-home delay, card duration, eye sleep, touch orientation; the orientation is reverted if not
confirmed within 10 s), the **Night mode** sub-page (off / by the sun / by the clock, night brightness, start–end),
**Phone** (pairing), earthquake threshold, the 04:00 maintenance restart, diagnostics, about, factory reset and
**Language / Dil**. You can change your name from the phone app.

**Serial port commands** (460800 baud). The commands themselves are Turkish words:

| Command | What it does |
|---|---|
| `rapor` | Full report: memory, stacks, network sources, SD, media, camera, bulb and printer |
| `yenile` | Fetches all data again right away |
| `web ac` / `web kapat` | Phone app server on / off (on at boot; `kapat` lasts until restart) |
| `telefon unut` | Forgets all paired phones |
| `gorunum <expression> <accessory> <effect>` / `gorunum oto` | Temporary look (same as Look in the app) / back to automatic |
| `sdtest` | Writes to the SD card and reads it back 50 times |
| `dokunmatik 0-7` | Changes and saves the touch orientation without touching the screen |
| `goruntu ters` / `goruntu duz` | Rotates the picture 180° (if the board sits upside down in its case); touch and scrolling follow, the setting is permanent |
| `goz uykusu kapat` / `goz uykusu <1-60>` | Turns eye sleep off or sets it in minutes |
| `kopru` | Bridge connection state (USB/WiFi, key present, session, bytes transferred) |
| `ajan` / `ajan test calisiyor\|bekliyor\|bitti\|codex\|kapat` | Claude and Codex session state / test with a fake session |
| `takim <espn-no> [colour1 colour2]` / `takim kapat` | Selects the team (e.g. `takim 436 13235b ffd60a`; colours optional) / removes it; stored in NVS |
| `futbol ac` / `futbol kapat` | Turns match tracking fully on / off (the app's "Match tracking" toggle) |
| `mac` / `mac yenile` | Match tracking state (including the Champions League list) / fetch fixtures again now |
| `lig yenile` | Fetches Süper Lig and Champions League scores again now |
| `ekran futbol` / `ligsayfa <league> [puan]` | Opens the football page / a league's matchday or standings (`tur.1`, `uefa.champions`, `eng.1`, `esp.1`, `ita.1`, `ger.1`, `fra.1`) |
| `mac test once\|basla\|gol\|rakip\|sari\|kirmizi\|degisiklik\|devre\|bitti\|kapat` | Tries the screens with a fake match |
| `kopru anahtar <32 hex>` / `kopru anahtar sil` | Saves / deletes the WiFi bridge key (the bridge's `--anahtar-yaz` option does this) |
| `bench` / `bench gecis` / `bench kaydir` | The board does page transitions and scrolling by itself and measures each step's frame rate |
| `bench bellek` | Visits every screen and measures the LVGL pool peak, the largest free block and fragmentation at each step |
| `lvgl yuk <KB>` / `lvgl yuk 0` | Deliberately fills / releases the LVGL pool (to test the "out of memory" behaviour) |
| `perf ac` / `perf kapat` | Prints a drawing measurement every second while you navigate |
| `spitest <MHz>` | Tests SPI speed by writing a pattern to the screen and reading it back (back to 40 MHz on restart) |
| `kayma ters` / `kayma duz` | Reverses the hardware scroll direction (for a different panel) |
| `dokunma ac` / `dokunma kapat` | Prints the screen coordinate of every touch (to check touch calibration) |
| `gece 02:00 07:00` | Sets night mode by the clock and saves it |
| `yazici <ip>` / `yazici kapat` | Saves the Creality K2's address / turns the printer feature off |
| `ekran ana` / `ekran spotify` / `ekran liste` / `ekran yazici` / `ekran mac` / `ekran ajan` | Switches to that page without touching the screen (home, Spotify, playlists, printer, match, agents) |
| `ekran goruntusu` | Sends a screenshot over the serial port; the bridge saves it as `tools/.cache/rumy_ekran.png` |
| `wiz durum` / `wiz tara` / `wiz esle <mac>` / `wiz unut` | Bulb state, scan the network for bulbs, pair, forget the pairing |
| `wiz ac` / `wiz kapat` / `wiz parlak <10-100>` / `wiz sicak <K>` / `wiz renk <0-359>` / `wiz sahne <no>` | Controls the bulb without touching the screen (on, off, brightness, white temperature, hue, scene) |
| `wiz uyari` | Tries the bulb alert from the Claude notification (two short red flashes) |
| `reboot`, `wifi-sifirla` | Restart, WiFi reset |

While the bridge is running the serial port is busy, so you can write commands line by line into
`tools/.cache/rumy_komut.txt`; the bridge sends them to the board and deletes the file (with `--log` the reply shows
in the bridge's output).

## 7. Phone app

Rumy serves its own small web app; added to the iPhone home screen it opens like an app, with its own icon and no
browser bar. No App Store, Mac or Apple developer account needed.

**Setup (once):**
1. On Rumy open **Settings → Phone**. A QR code and a 4-digit code appear on the screen.
2. Scan the QR code with the iPhone camera; `http://rumy.local` opens in Safari.
3. In Safari choose **Share → Add to Home Screen**.
4. Open it from the Rumy icon on the home screen and enter the 4-digit code shown on Rumy. The phone stays paired
   from then on.

The app added to the home screen uses storage separate from Safari, so you have to enter the code inside the app.

**Use:** tap the icon; the settings open and every change is saved to Rumy immediately.
- **Sleep now:** puts Rumy to sleep right away until the end of night mode (07:00 by default). Touching the screen
  wakes it briefly as in night mode; "Wake up" ends the sleep. The sleep survives a restart.
- **General:** language, name, location (province or custom coordinates), earthquake threshold.
- **Look:** pick Rumy's expression (13 expressions, with thumbnails), accessory and effect; applied at once. It is not
  permanent: it returns to automatic the next time Rumy's own state changes (weather, music, match, night, agent…).
  "Back to automatic now" also releases it.
- **Display:** brightness, return to home, card change, eye sleep, night mode and its hours, night brightness.
- **Football:** the "Match tracking" toggle; a team from the Süper Lig and the top five leagues (league tabs, coloured
  badges) or "No team"; Süper Lig / Champions League goal notifications.
- **Light:** bulb name and state; coding agent alert, team goal celebration, "don't touch if the bulb is off".
- **3D printer**, **Special days** (if you enter a year it shows "age 34", "year 10" and so on), **Maintenance**
  (04:00 restart, refresh data, restart, version and memory info).

- The phone and Rumy must be on the same WiFi; it does not work from outside the home. If Rumy is off the app says
  "Unreachable".
- Up to 4 phones can pair (a fifth replaces the oldest). To remove all of them: on Rumy, Settings → Phone → Forget.
  The pairing code is valid only while that screen is open; after 5 wrong tries it locks for 1 min.
- While the app is in use (until 20 s after the last request), Rumy's own HTTPS requests are postponed.
- If the name `rumy.local` does not work on your network, the app also opens at the IP address shown on the Phone
  screen.

**On a computer:** the same app opens in a wide window with two columns (status, "Sleep now" and sections on the
left; the selected section on the right). To install a "Rumy" shortcut on the desktop and in the Start menu:

```bash
python tools/pc_kisayol.py --kur
```

The shortcut opens Edge in its own window without an address bar (`--app=http://rumy.local/`); on first open enter the
code from Settings → Phone on Rumy. Remove it with `--kaldir`; if `rumy.local` does not work, use
`--adres http://<ip>/`.

## 8. RAM and flash budget

Large buffers are allocated from the heap once at boot, before WiFi starts, and are never freed. Build measurements
come from the `pio run -e cyd` output and the map file. The device column was measured with v1.0.2 on an
ESP32-2432S028R (ESP32-D0WD-V3, ILI9341): the output of the `rapor` serial command after visiting every screen and
making the AFAD/Nager HTTPS requests. The same values appear on the Diagnostics screen.

| Item | Plan | Build measurement | On device (v1.0.2) |
|---|---|---|---|
| LVGL pool (TLSF) | 24 KB (was 36 KB) | 24 576 B, heap, once | peak 16 184 B / ~22.7 KB usable (27 September, `bench bellek`) |
| Draw buffers (DMA) | 2 × 20 KB (32 lines) | 40 960 B | – |
| Weather icons | flash | 148 096 B flash, 0 B RAM | before v1.1.0 there was a 20 KB RAM cache |
| JSON arena | 8 KB | 8 192 B | – |
| WiZ JSON arena | 1.5 KB | 1 536 B | – |
| Network / WiZ stack | 10 / 4 KB | 10 240 / 4 096 B | at least 4 728 / 2 756 B left free |
| UI (loop) stack | 8 KB | 8 192 B (Arduino default) | at least 2 812 B left free |
| SD / FATFS | ~11 KB | heap, `max_files=1` | 11 068 B |
| WiFi + network task | – | – | 60 432 B |
| TLS | – | – | 47 KB for the session, ~60 KB momentarily during the handshake |
| App data (static) | ~3.5 KB | 5 500 B: store copies 1 908 · UI copy 1 877 · special days 1 191 · weather 372 · earthquake 152 | – |
| Other project statics | – | ~4.6 KB: WiZ buffer 1 354 · display driver 602 · shared text styles 573 · other | – |
| Free heap (WiFi connected) | – | – | 72 012 B (v1.0.2) → 68 648 B (with Spotify) → **80 928 B** (24 KB LVGL pool) |
| Lowest free heap (during HTTPS) | ≥ 8 KB | – | 10 936 B |

Total permanent allocations: 87 552 B buffers + 14 336 B task stacks ≈ **100 KB**. In v1.1.0 the icon cache's 20 KB
went to the draw buffers; the total did not change, and free heap was also measured at 72 KB in v1.1.0.

Heap left after each boot step (the `bos bellek` / free memory line in the serial log):

| Step | Free heap | Spent in this step |
|---|---|---|
| Start (settings, diagnostics) | 239 924 B | – |
| Fixed buffers | 152 276 B | 87.6 KB |
| Display driver + LVGL | 149 780 B | 2.5 KB |
| SD card | 138 712 B | 11.1 KB |
| Caches + UI | 138 412 B | 0.3 KB |
| WiFi + network task | 77 980 B | 60.4 KB |
| WiZ task | 73 508 B | 4.5 KB |

**A bottleneck found and fixed on the board:** in v1.0.0 only 52.9 KB was free with WiFi connected, and HTTPS requests
(AFAD, Nager) never started because of the safety threshold. When the threshold was briefly lowered, free heap fell to
3 KB during the handshake. SD `max_files` 3 → 1 (FATFS allocates a 4 KB buffer for every open file; SD access is
serialised by a single lock anyway), the LVGL pool 44 → 36 KB and the network stack 12 → 10 KB freed 18 KB.

**LVGL pool use per screen** (measured on the board with a `bench bellek` run; the peak is reset at the start of each
step; with music playing, a cover and a two-line long song title on the Spotify screen, and the scrolling song strip on
the home screen):

| Open screen (the home screen is always in memory) | Used | Peak |
|---|---|---|
| Home screen (including Rumy with headphones and the song strip) | 5.7 KB | 8.0 KB |
| + Weather (scrolling, hourly strip) | 11.2 KB | 13.4 KB |
| + Light | 10.6 KB | 12.3 KB |
| + Settings (scrolling) | 15.0 KB | **16.4 KB** |
| + Display / Night mode sub-page | 12.5 / 12.2 KB | 14.0 KB / – |
| + Spotify | 9.4 KB | 10.8 KB |
| + Night / celebration screen | 6.7 / 6.8 KB | 8.2 / 8.1 KB |

On a transition the old screen is deleted before the new one is created (the hardware scroll keeps the old picture in
the panel's own memory, so no LVGL objects are needed for it); so two screens are never in memory at the same time.
Settings → sub-page transitions used to peak at 22.8 KB. The pool size was chosen as 24 KB by adding a 25% margin and
the pool's own overhead (~3.2 KB) on top of the highest peak (16.2 KB); this saved 12 KB of heap. The song strip on the
home screen raised the peak to 16.4 KB (+~250 B); the margin is still above 25% (~22.7 KB usable).

An HTTPS request only starts while the largest free block is ≥ 34 KB and free heap is ≥ 64 KB (based on the ~60 KB
momentary handshake peak measured on the device); otherwise it is postponed and the "postponed" counter goes up in
Diagnostics (thresholds in `src/app/config.h`). If the LVGL pool does not have room for a screen (the needs are in a
measured table) the screen is not opened: an "Out of memory" warning appears, Rumy stays on the home screen and the
"rejected screen" counter in `rapor` goes up. The board does not restart (LVGL used to restart it with `abort()`).
Try it with the `lvgl yuk <KB>` command.

| Icon | Size | Format | In flash | RAM on LVGL's default path |
|---|---|---|---|---|
| Weather screen main icon | 64 | I8 | 5 120 B | 16 384 B |
| Home screen weather block | 48 | I8 | 3 328 B | 9 216 B |
| Hourly strip, sunrise/sunset | 28 | I4 | 456 B | 3 136 B |
| 7-day list | 24 | I4 | 352 B | 2 304 B |

With `LV_BIN_DECODER_RAM_LOAD=0` icons are decoded from flash line by line while drawing; they are never copied to
RAM. Detailed table: `docs/icon_ram.md` ([English](docs/icon_ram.en.md)), quality preview: `docs/icons_preview.png`.

**Flash:** the app was 1 822 528 B (1.74 MB), target < 2 MB. v1.1.0 added the embedded icons (+148 KB) and `-O2` for
speed (+137 KB); partition 2.94 MB. On 25 October 2026 the app was 2 154 976 B (2.06 MB); for wireless updates the
partition shrank to 2.5 MB (82% full), and the freed 448 KB holds the updater. Static DRAM (.data + .bss, whole core
included) 59 088 B. IRAM 123 418 B (LVGL's pixel functions are in IRAM; limit ~128 KB). The web UI is stored gzipped
in flash. Every build updates `dist/size_report_<env>.txt`.

### Performance

Measured on the board with the `bench` command: the board does transitions and scrolling by itself and counts each
step's frames.

| Step | v1.0.2 | v1.1.0 |
|---|---|---|
| Page transition (220 ms slide) | 3 frames, 5–7 fps, 100–190 ms per frame | 18–30 frames, longest frame 20–35 ms |
| Opening the weather screen | 293–304 ms (icons from SD) | 9–10 ms |
| Scrolling the weather screen | 5–10 fps | 24–25 fps |
| Hourly strip | 15 fps | 27–30 fps |
| Scrolling Settings | 13 fps | 30–32 fps (SPI limit) |

What was done and what each change gained:

| Change | Effect |
|---|---|
| The ILI9341's hardware scroll (`VSCRSADD`) for page transitions | Only the newly revealed strip is drawn each frame instead of the whole screen; frames became 5–8× cheaper |
| Flash QIO 80 MHz (was DIO 40 MHz) | Font and icon data are read from flash, so scrolling became ~2× faster |
| Icons embedded in flash | Weather screen opening 300 ms → 10 ms, the stutter on first scroll is gone, 20 KB RAM freed |
| `-O2` (was `-Os`) | Drawing 20–25% faster |
| Draw buffers 16 → 32 lines | Screen chunks per frame 15 → 8, object traversal cost halved |
| LVGL pixel functions in IRAM, corner mask cache 8 | A few more ms |
| Touch SPI 250 kHz → 1 MHz | Shorter read latency while scrolling with a finger |

SPI at 80 MHz was tried too: reading back the pixels written by `spitest 80`, 3–5% were corrupted, so the display stays
at 40 MHz (full frame 30.7 ms; the scrolling ceiling on simple screens is ~32 fps). For profiling, the
`pio run -e cyd_prof` build collects LVGL's internal measurement points; at the end of `bench` it prints a per-function
timing table.

## 9. Development

```bash
pip install platformio
pio run -e cyd
```

| Environment | Description |
|---|---|
| `cyd` | ILI9341, normal build → `dist/Rumy.bin`, `dist/Rumy-full-0x0.bin` |
| `cyd_st7789` | CYDs with an ST7789 panel → `dist/Rumy-st7789.bin` |
| `cyd_endurance` | Endurance test: data every 30 s, screen change every 5 s, log every 60 s |
| `cyd_prof` | Profiling build: LVGL measurement points on, per-function timing table at the end of `bench` |
| `native` | Unit tests on the computer (`pio test -e native`) |

The updater is a separate small project (`updater/`, `pio run -d updater`); `pio run` builds it when needed and adds
it to the full image and the USB upload (0x290000).

The panel type is changed in one place, the `[rumy] panel = ILI9341` line in `platformio.ini`. For personal
development settings such as the serial port, copy `secrets.ini.example` to `secrets.ini` (in `.gitignore`).

Folders: `lib/rumy_core` (Arduino-free, tested logic: WiZ protocol, Open-Meteo/AFAD/Nager parsers, special day
calculation, cache format, language helper), `src/` (board, ui, rumy, wiz, net, weather, quake, events, clock,
settings, webui, diag, app), `web/` (the app; gzipped and embedded in flash at build time), `tools/`, `test/`.

| Tool | What it does |
|---|---|
| `tools/convert_assets.py` | Converts Meteocons SVGs into palette LVGL `.bin` icons (`sd_card/rumy/img`), then calls `embed_icons.py` |
| `tools/embed_icons.py` | Turns the `sd_card/rumy/img` icons into `src/ui/icons_embedded.cpp`, embedded in flash |
| `tools/make_fonts.py` | Builds fonts from Inter + Font Awesome with only the glyphs needed |
| `tools/make_record.py` | Generates the 24 frames of the spinning record on the home screen and embeds them in `src/ui/record_frames.cpp` (`--preview file.png` shows the frames) |
| `tools/make_match_art.py` | Embeds league badges and the ball of the goal animation in `src/ui/match_art.cpp`; uses logos in `tools/assets/lig_logolari/` if present |
| `tools/make_teams.py` | Generates the app's team list (6 leagues, ESPN number, abbreviation, two colours) from ESPN and writes it to `web/index.html` |
| `tools/gen_ca_bundle.py` | Generates the trimmed root certificate bundle for AFAD and Nager (`src/net/ca_bundle.h`) |
| `tools/pc_kisayol.py` | Installs/removes the "Rumy" shortcut (desktop + Start) that opens the Rumy app on the computer |
| `tools/veri_koprusu.py` | Inside the bridge: fetches the AFAD and Nager data Rumy asks for over HTTPS on the computer and sends a summary line (try it alone with `python tools/veri_koprusu.py afad`) |
| `tools/build_web.py` | `web/index.html` → `src/webui/web_assets.h` (automatic at build time) |
| `tools/post_build.py` | `dist/` images and the size report (automatic at build time) |
| `tools/fake_bulb.py` | A fake WiZ bulb for testing without a real one |
| `tools/endurance_log.py` | Records and summarises the endurance test |
| `tools/ui_preview.py` | Draws the UI on the computer without a board (`docs/preview/`, `docs/ui_preview.png`) |
| `tools/claude_hook.py` | Claude Code and Codex hook: writes the session's state under `~/.claude/rumy/oturumlar/` |
| `tools/claude_agents.py` | Watches Claude and Codex sessions and sends them to Rumy through the bridge; `--kur` / `--kaldir` / `--durum` |
| `tools/fw_update.py` | Wireless update: checks the image, sends it to Rumy through the bridge, verifies the new version (`spotify_bridge.py --guncelle`) |

Code rules: no comments in C++ code; LVGL calls only in the UI task; no `String` concatenation in loops; no blocking
`delay()`; a timeout on every network operation. Details and contribution steps: [CONTRIBUTING.md](CONTRIBUTING.md).

### Preview without a board

```bash
python tools/ui_preview.py
```

Compiles the real LVGL 9.5, the real screen code (`src/ui`, `src/rumy`), the real icons embedded in flash and test data
on the computer and produces screenshots; only WiFi, WiZ, SD, the clock and the display driver are faked by
`sim/sim_stubs.cpp`. It also writes the LVGL pool use for every screen (`.pio/sim_build/sim_run.log`). Needs MinGW-w64
g++ (on PATH or via `RUMY_MINGW`) and Pillow. `RUMY_SIM_POOL_KB` tries other pool sizes. The default is 36 KB: the
preview runs 64-bit (pointers twice as large), so this matches the 24 KB pool on the device.

There is a separate scenario for edge cases: `python tools/ui_preview.py --cesit` (also with `--buyuk`) draws every
weather icon, extreme temperatures, long names, missing data, song and playlist names with emoji/Japanese, and long
match and printer texts → `docs/preview_cesit/`, `docs/ui_cesit.png`. Every run does an automatic audit: overlapping
text/icons, clipped text and characters missing from the font are written to `docs/preview*/denetim.txt` (intentional
ones are silenced with `sim/denetim_izin.txt`).

For animations, `python tools/ui_preview.py --film`: the goal screen's four shows, the closing, sliding notifications,
match notifications and cards with league badges → `docs/preview_film/` (a GIF and a review strip for each
animation). To try team colours, `RUMY_SIM_ONLY=takim python tools/ui_preview.py --film` draws the live card, the goal
screen and the match screen with six teams. For every film it prints what percentage of the screen LVGL redraws on
each refresh (an early indicator of smoothness on the board).

![Rumy screens](docs/ui_preview.png)

## 10. Endurance test

```bash
pio run -e cyd_endurance -t upload
python tools/endurance_log.py --port COM7 --hours 24 --csv dist/endurance.csv
```

The summary gives the lowest free heap, the heap trend (B/hour), the largest block, LVGL fragmentation, the TLS peak,
the stack minimums, network errors and unexpected restarts. The leak suspicion threshold is −300 B/hour.

## 11. Troubleshooting

| Symptom | Fix |
|---|---|
| Inverted colours, shifted picture | Use `Rumy-st7789.bin` or build with `panel = ST7789` |
| Touches land in the wrong place | On an ILI9341 CYD the correct value is 6 (default). On another board: Settings → Display → Touch orientation; confirm within 10 s. If the screen is unusable, use the `dokunmatik 0-7` command over USB |
| "No SD card" card | Is the card inserted and FAT32? Remove and reinsert it and restart. Everything works without a card; only your special days are not saved |
| The picture slides the wrong way on page transitions | The scroll direction may be reversed on a different panel: try the `kayma ters` command over USB |
| The bulb is not found | Is the bulb on and on the same 2.4 GHz network? Are guest network / AP isolation off? Test Rumy's side with `tools/fake_bulb.py` |
| No weather | Check the code on the "Weather + 7 days" line in Diagnostics (current, hourly and 7-day data come in one request); the `yenile` command, or Maintenance → "Refresh data" in the phone app |
| No earthquake data | The path at the end of the `afad` line in `rapor` should be `kopru` (bridge), `https` or `kandilli`. With the bridge running the computer fetches AFAD; without it Rumy uses Kandilli. Look for `veri -> Rumy` lines in the bridge window |
| WiFi changed | Settings → WiFi → Reset. Rumy switches to setup mode |
| Freezes / restarts | Get the `rapor` output from the serial port; the "Boot" line in Diagnostics shows the reset reason |

## 12. Licences

Rumy's own code is licensed under the [PolyForm Noncommercial 1.0.0](LICENSE): you may use, modify and share it for
personal, hobby, educational and research use; commercial use (selling it, or putting it into a paid product or
service) is not covered by the licence and needs separate permission. For commercial permission, open an issue on
GitHub or reach out through the [@SudoMrSpear](https://github.com/SudoMrSpear) profile. So the project is not "open
source" under the OSI definition but "source-available".

The list of libraries, fonts, icons and data sources used, with licence notes, is in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) ([English](THIRD_PARTY_NOTICES.en.md)), and the licence texts are in
the `LICENSES/` folder. In short:

| Component | Licence |
|---|---|
| LVGL 9.5 | MIT |
| LovyanGFX | FreeBSD (BSD-2) |
| ArduinoJson 7 | MIT |
| WiFiManager (tzapu) | MIT |
| Arduino-ESP32 | LGPL-2.1 |
| ESP-IDF, mbedTLS, lwIP | Apache-2.0 / BSD |
| Meteocons (Bas Milius) — weather icons | MIT |
| Inter (Rasmus Andersson) — typeface | SIL OFL 1.1 |
| Font Awesome Free — icons | SIL OFL 1.1 (font), CC BY 4.0 (icons) |
| Montserrat — lettering of league badges (generation only) | SIL OFL 1.1 |
| Open-Meteo data | CC BY 4.0 |
| AFAD earthquake data | Disaster and Emergency Management Authority (AFAD), Ministry of Interior of Türkiye, public service |
| Kandilli earthquake list (fallback) | Boğaziçi University Kandilli Observatory and Earthquake Research Institute, public page |
| Nager.Date | MIT |

Root certificates come from the Mozilla store (certifi); "GlobalSign Root CA" is no longer in that store, so it is
downloaded from GlobalSign's official address and verified by its SHA-256 fingerprint (`tools/gen_ca_bundle.py`).

## 13. Raspberry Pi version (experimental)

The same UI also runs on a Raspberry Pi Zero W with a Waveshare 3.5" (A) SPI display. It is experimental and less
tested than the CYD. The code is in the `pi/` folder; the UI, `rumy_core` and the data modules are shared with the
CYD.

**One-time setup (on the Pi):**

1. Raspberry Pi Imager with "Raspberry Pi OS Lite (32-bit)". Settings: hostname `rumy-pi`, user `rumy`, SSH with a
   public key. If the WiFi name contains Turkish letters (e.g. "ı"), Imager 2.0.11 garbles it: on first boot run
   `sudo nmtui` on the Pi → "Activate a connection", pick the network from the list and enter the password.
2. Attach the display (first 26 pins), then once on the Pi: `sudo bash ~/rumy/pi/kurulum.sh` and `sudo reboot`.
   The script enables the display driver, keeps console text off the display and makes the Rumy service start at
   boot.
3. Optional but recommended: `sudo bash ~/rumy/pi/kurulum2.sh` and `sudo reboot`.
   - Turns Rumy into a system service that starts at ~16 s into boot without waiting for the network. As a user
     service Rumy only started at ~90 s.
   - Locks the core clock at 400 MHz (`core_freq_min=400`). When the Pi went idle the SPI display speed dropped from
     22 to 14 MHz.
   - Keeps swap in memory only (zram). The 19 s swap file preparation on every boot is gone.
   - Flushes the journal to disk once a minute; the last minutes are not lost on a power cut.
   - Loads the display and touch drivers early in boot (`/etc/modules-load.d/rumy-ekran.conf`).
   - To undo: `sudo bash ~/rumy/pi/kurulum2.sh geri`.
4. Recommended: `sudo bash ~/rumy/pi/kurulum3.sh` (no restart needed). Adds a watchdog to the service: if Rumy's main
   loop does not report "I'm alive" for 30 s, systemd restarts it. `Restart=always` only caught crashes, not hangs.
   If the installed Rumy does not support the watchdog, the script changes nothing. Serial command `bekci` for the
   state; `bekci dur` to test it (Rumy should restart within ~30 s). To undo: `sudo bash ~/rumy/pi/kurulum3.sh geri`.
5. To try display speeds: `sudo bash ~/rumy/pi/spi_hiz.sh 40` and `sudo reboot`. If the picture is corrupted, go back
   with `spi_hiz.sh 32` first, then `spi_hiz.sh 24` if that fails too. Full-screen scrolling (e.g. the weather screen)
   is limited by this speed: ~10 frames/s at 24, ~16 frames/s at 40 by calculation.
6. Power: feed the Pi through the "PWR" input with a solid, short cable and an adapter of at least 5 V 2.5 A. A loose
   cable makes the Pi power-cycle without warning.

**Building and deploying (from the computer):**

```bash
python tools/pi_deploy.py --deps --sd
python tools/pi_deploy.py --restart
```

`--deps` and `--sd` are only needed the first time. The build runs on the Pi itself; the first build takes long on a
Zero W, later ones only compile changed files. Give the address with `--host` or `RUMY_PI_HOST`.

Every build keeps the running version as `pi-build/rumy.onceki`. If the new version misbehaves,
`python tools/pi_deploy.py --geri` goes back to the previous version and restarts Rumy; running it again brings the new
version back. With the watchdog on, it will not go back to a version that does not support it.

**Spotify and printer camera (while the computer is on):** the bridge connects over WiFi instead of USB.

```bash
python tools/spotify_bridge.py --pi rumy-pi.local
```

The connection key is in `~/.config/rumy/gizli.json` on the Pi and in `tools/.cache/pi_anahtar.txt` on the computer
(outside git).

**Files on the Pi:** settings in `~/.config/rumy/ayarlar.json`, cache and SD content in `~/.local/share/rumy/sd`,
optionally `~/.config/rumy/ekran.json` (`{"dondur": 180}` rotates the picture, `{"olcek": 1}` shows it without scaling)
and `~/.config/rumy/dokunmatik.json` (touch calibration). On the Pi the web app is on port 8080.

**Touch calibration:**
- On this board the touch orientation is 2 (Y reversed). That is the default in the Pi build. If touches still go up
  and down the wrong way, serial command: `dokunmatik 2`.
- If touches miss buttons, open *Settings > Display > Touch calibration*. The serial command `ekran dokunmatik` opens
  it too.
- A cross appears in each of the four corners and then the centre in turn; touch the middle of each and lift your
  finger.
- Rumy computes both axes from the raw touch values. It also detects reversed or swapped axes by itself.
- The fifth cross in the centre is a check: if the touch lands within 14 pixels the calibration is written to
  `dokunmatik.json`, otherwise it starts over. If nothing is touched for 30 s it exits without changing anything.

Serial commands (`rapor`, `ekran ...`, `yazici ...`, `wiz ...`) are sent over the bridge connection (TCP 7777).

**Display:**
- The Pi version draws the UI at the display's native resolution, 480×320 (`RUMY_SCREEN_W/H`, `pi/Makefile`). There is
  no scaling and the picture is sharp. The CYD stays at 320×240; both sizes are built from the same code.
  - Layout is written with the `theme::S()` (4/3 scale), `R()` (right-aligned), `CX()` (centred) and `FW()` (full
    width) helpers. On the CYD they return values one to one; the CYD picture does not change.
  - Large fonts, icons, accessories and record frames are in separate files and only go into the 480×320 build:
    `python tools/make_fonts.py --buyuk`, `python tools/convert_assets.py --buyuk`, `python tools/make_record.py --buyuk`.
  - Simulator: `python tools/ui_preview.py --buyuk` → `docs/preview_buyuk/`.
  - Cover 192 px, camera 213×160. On connect the Pi says `@merhaba 192 213x160`; the bridge learns the cover and
    camera sizes from it.
- Rumy writes directly to the SPI display with DRM and sends only the changed rectangle; the transfer runs in a
  separate thread (`panel`).
- This display has no hardware scroll, so page transitions reveal the new page strip by strip from the edge it comes
  from instead of sliding.
- The backlight cannot be dimmed; brightness is applied by darkening pixels, following eye perception (gamma 2.2).
- Environment variables for comparison: `RUMY_FBDEV=1` the old framebuffer path, `RUMY_KAYDIR=1` the old sliding
  transition.

**Health log:** every minute Rumy writes the time, uptime, temperature, power flags (`vcgencmd get_throttled`), load
and free memory to `~/.local/share/rumy/son_durum.txt`. At boot it prints the previous record to the journal
(`[saglik] onceki son kayit`). The boot reason is told apart by boot ID: "power applied" after a power cut, "crash" if
Rumy crashed.

**WiZ:** with newer WiZ firmware (e.g. 1.38.0) the bulb may reject unsigned local commands (`-32602 Invalid params`);
the light screen shows "No command permission · WiZ". On/off appears to work but colour and mode do not change. The fix
is "Allow all controls" under *Settings > Current home > Security* in the WiZ app ("Only verified controls" off in the
English app). With this setting every device on the home network can send commands to the bulb. Reading is not
affected. Diagnostics: `wiz durum | tara | esle <mac> | ac | kapat | parlak <10-100> | sicak <K> | renk <0-359> | sahne <no>`.

## 14. Contributing, security and legal note

- **Contributing:** bug reports and ideas go to GitHub Issues; rules and checks for code contributions are in
  [CONTRIBUTING.md](CONTRIBUTING.md). Release notes: [CHANGELOG.md](CHANGELOG.md) ([English](CHANGELOG.en.md)).
- **Security:** Rumy is designed for use on a home network only; the trust model and how to report a vulnerability
  are in [SECURITY.md](SECURITY.md). Do not expose ports 80 (app) and 7777 (bridge) to the internet.
- **Technical document:** architecture, screens, memory, network and measurements are in
  [Rumy_Project_Document.md](Rumy_Project_Document.md) (Turkish original:
  [Rumy_Proje_Dokumani.md](Rumy_Proje_Dokumani.md)).
- **Legal note:** Rumy is an independent hobby project; it is not affiliated with or endorsed by ESPN, LiveScore,
  TheSportsDB, UEFA, TFF or the leagues and clubs mentioned, Spotify, WiZ (Signify), Creality, Anthropic, OpenAI, Claw
  Light, AFAD, Kandilli Observatory or Open-Meteo. Names appear only to describe compatibility. Match data comes from
  ESPN's and LiveScore's public but undocumented services; these services may change or shut down, and are for
  personal use.
