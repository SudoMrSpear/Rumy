# Changelog

**English** · [Türkçe](CHANGELOG.md)

The version number is the `[rumy] version` value in `platformio.ini`; it appears on Rumy's Diagnostics screen, in the
`rapor` output and in the Maintenance section of the app.

## Unreleased

- **English / İngilizce:** the device interface, the phone app, the first setup portal and the wireless updater now
  work in English too. English is the default for new installs; switch to Turkish in Settings → Language / Dil, or in
  General → Language / Dil in the app (Rumy restarts). The setting is stored under the `dil` key in NVS.
- **English documentation:** `README.en.md` (full translation), `Rumy_Project_Document.md`, `CHANGELOG.en.md`,
  `THIRD_PARTY_NOTICES.en.md`, `docs/icon_ram.en.md`, `sd_card/README.txt`; `CONTRIBUTING.md`, `SECURITY.md` and the
  issue templates are bilingual.
- **Web installer:** one-click install from Chrome/Edge at <https://sudomrspear.github.io/Rumy/> (ESP Web Tools);
  choose ILI9341 or ST7789. For every new release `web_kurulum.yml` republishes the page with that release's full
  images.

## 1.3.0 — first source-available release

- **Team selection:** any team from the Süper Lig and the top five leagues (Premier League, LaLiga, Serie A,
  Bundesliga, Ligue 1) is chosen in the Football section of the app; league tabs and a team list with coloured badges
  (generated from ESPN with `tools/make_teams.py`). No team is selected by default.
- **Match tracking toggle:** football can be switched off completely with one toggle; while off, no matches are
  queried and no notifications or match card appear. Serial commands `futbol ac|kapat`, `takim <no> [colour1 colour2]`.
- **Team colours:** the goal screen, the match card, highlights on the match screen and the bulb's goal celebration
  are derived from the team's two colours (`rc::teamPalette`); without colours, the previous navy-and-yellow look.
- **Bridge:** Rumy reports the selected team with an `@futbol` line; the fast goal watcher and the player photo tool
  now work for this team. If TheSportsDB rejects the request, the tool switches to Wikimedia Commons and Wikipedia.
- **League badges:** the repository has no club or competition logos; cards show plain abbreviated badges. Users who
  put their own logos in `tools/assets/lig_logolari/` can use them.
- Default location Istanbul; sample data and preview images cleaned of personal information.
- PolyForm Noncommercial 1.0.0 licence (commercial use needs permission), third-party licence notes, contribution and
  security documents, GitHub Actions build.

## Development after 1.2.0 (October 2026)

The version number stayed at 1.2.0 during this period; the changes are part of 1.3.0.

- **Raspberry Pi version (experimental):** a 480×320 interface on a Pi Zero W with a Waveshare 3.5" SPI display,
  direct drawing with DRM, a service watchdog, a health log, setup scripts.
- **Weather expressions:** separate Rumy states for sunny, hot, sweltering, overcast, foggy, drizzle, rain, snow,
  thunderstorm, windy, cold and clear night.
- **Home screen:** the "now playing" strip at the head of the column next to the clock, a fixed layout, a centred
  date.
- **Light:** eight bulb scenes for everyday use.
- **WiFi bridge:** the bridge can also talk to the board over a keyed TCP connection (the cable only for power).
- **Spotify:** Spotify's own volume level, volume that speeds up when held, playlists (Web API, PKCE).
- **Football (ESPN):** a live score card, goal/card/substitution notifications, a match screen; an animated goal
  screen (four shows), a bulb celebration on goals, disallowed goal notifications, a player photo on the goal screen.
- **League day and the Football page:** goals from every Süper Lig and Champions League match; matchdays and standings
  for seven leagues.
- **Fast goal alerts:** from LiveScore in ~20–45 s.
- **Simulator:** edge case scenarios and an automatic screen audit (overlapping, clipped text, characters missing from
  the font).
- **Bridge watchdog:** the bridge reopens by itself if it closes; starts at Windows sign-in.
- **Coding agent light:** the state of Claude Code and Codex sessions as a star on the home screen, a notification and
  a bulb alert while waiting (inspired by Claw Light).
- **Wireless update:** the image goes from the bridge to the SD card, and from there to the main partition with a
  small updater.
- **Phone and computer app:** a web app served by Rumy (added to the home screen), 4-digit pairing, "Sleep now", a Look
  section (temporary expression/accessory/effect), a two-column layout and a shortcut on the computer.
- **Earthquake and holiday data:** since the board's memory is not enough for HTTPS, AFAD and Nager go through the
  bridge; without the bridge, Kandilli's plain-HTTP list.

## 1.2.0

- Spotify screen and PC bridge (song, cover, controls), a cover cache in flash.
- Rumy with headphones and beat tracking from the audio output.
- LVGL pool 36 → 24 KB (measured per screen).
- A "now playing" strip on the home screen.
- Creality K2 printer screen and a camera through the bridge.
- Weather data in one request, Nager once a day; a printer address that accepts IPv4 only.

## 1.1.0

- Performance: page transitions with the ILI9341 hardware scroll, flash QIO 80 MHz, icons in flash, `-O2`.
- A new home screen in saturated neon colours.
- Eye sleep; the `bench`, `perf`, `profil`, `spitest`, `kayma` tools.

## 1.0.2

- First install on the board and fixes: memory freed for HTTPS (SD `max_files` 3 → 1, smaller LVGL pool and network
  stack), touch orientation, data fetched before the clock was synchronised showing a wrong date, the simulator build.

## 1.0.0

- First release: screens, Rumy's face and expressions, weather/earthquake/holiday sources, WiZ bulb, web interface,
  first setup portal, SD cache, unit tests, board-less preview.
