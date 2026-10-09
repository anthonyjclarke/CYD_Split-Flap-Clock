# Changelog

## Todo

- Add a simple on-device WiFi/time status screen for failed WiFi or NTP sync.
- Add config options for brightness presets or auto-dimming by time of day.
- Add a button or touch gesture to toggle 12/24-hour display without reflashing.
- Add optional date formats, such as `DD MMM YYYY`, `MMM DD YYYY`, or ISO style.
- Add a brief DST/timezone diagnostic line to serial output after NTP sync.
- Colour options for Display (day, time, am/pm and date), configurable in config.h

## [1.1.0] Unreleased

### Fixed
- Joining the saved WiFi network now times out after 15 s
  (`WIFI_CONNECT_TIMEOUT_S`) instead of WiFiManager's default ~60 s. During
  that wait Improv can't answer, so a Connect from the web installer showed
  **Install**; the setup portal and Improv now take over 45 s sooner when the
  network is unreachable.
- Improv device name now ends in the MAC's last two bytes (`SplitFlap-AE8C`),
  not the Espressif vendor prefix (`-CBB0`). Re-copied
  `src/network/improv_setup.cpp` from cyd-web-installer `9457ba6`.

## [1.0.0] 10-10-2026

First release with the ESP Web Tools browser installer.

### Added
- Startup self-test flips all animated time, day, AM/PM, and date cells to a
  diagnostic pattern, clears them, then lets the live clock roll in after NTP.
- `FIRMWARE_VERSION` (`1.0.0-dev`) and `PROJECT_NAME` (`CYD_Split-Flap-Clock`,
  frozen) in `include/config.h`; the version is logged at boot.
- Improv-Serial, always on (vendored `lib/ImprovWiFi` with the parser fix, plus
  `src/network/improv_setup.*`): WiFi setup from the installer dialog, and
  Connect offers **Update** on a board that already runs this firmware.
  Improv is serviced in `loop()`, the startup self-test, a now non-blocking
  WiFiManager portal, and the NTP wait (replaces `waitForSync(30)`).
- Boot log line `Running from app0|app1`.
- `tools/merge_bin.py` post-script (`flash_parts.json`, `firmware-merged.bin`)
  and `custom_installer_label` / `custom_installer_hint` on the `cyd` env.
- `Firmware` GitHub Actions workflow calling the shared
  `cyd-web-installer` release workflow; `_site/` gitignored.
- README **Install** section for the browser installer at
  https://anthonyjclarke.github.io/CYD_Split-Flap-Clock/, and
  `docs/WEB_INSTALLER.md` with the smoke-test result (10-10-2026) and the tests
  still owed.

### Changed
- Platform pinned to `espressif32@6.12.0` (arduino-esp32 2.0.17); backlight
  back on the 2.x LEDC API (`ledcSetup` / `ledcAttachPin`, channel 0).
- `WIFI_AP_NAME` renamed to `AP_NAME` (the installer page reads it).

### Fixed
- Improv re-copied from cyd-web-installer `efe7cbd`: each packet now starts on
  a new line, so noise when Chrome opens the port no longer swallows the reply
  and Connect reliably offers **Update** instead of **Install**.

### Removed
- Unused `data/splitflap/` (152 PNG copies of the tiles) and
  `include/splitflap_assets.h`. The firmware never mounted a filesystem; tiles
  are compiled in from `splitflap_cyd_assets/`. No FS image ships.
- Unused `#include "secrets.h" – CI builds without it, and no `SECRET_*`
  value is compiled in.

## [0.1.4] 27-04-2026

### Changed
- Demo day-of-week and date rows now use split-flap tile cells (not TFT text):
  9 half-size (24×36) tiles above for the day name, 11 below for DD MMM YYYY
- `SplitFlapCell` is now parameterisable: `begin()` accepts `tileW`, `tileH`,
  and `fullSeq`; all render functions nearest-neighbour-scale the 48×72 source
  bitmaps to any tile size at zero memory cost (no extra assets)
- Full Solari flip sequence added: `' '→A→…→Z→0→…→9→' '` used by text cells;
  digit cells keep the existing `' '→0→1→…→9→0` cycle
- Regenerated `splitflap_bitmaps` with full A–Z set (~510 KB flash) required
  for letter tiles

## [0.1.3] 27-04-2026

### Added
- Demo mode now shows day-of-week label above and DD MMM YYYY date below the
  digit tiles, rendered in FreeSansBold18pt7b centred on the 84 px strips above
  and below the tile row
- Full reset cycle every `DEMO_RESET_MS` (60 s): instant-blanks all four digit
  tiles, wipes text areas, picks new random day/date, draws labels, then
  animates digits back in from blank
- `DEMO_RESET_MS = 60000` added to `config.h`
- Separated `initDemo()` (called once in `setup()`) from `updateDemo()` so the
  initial fill is guaranteed to run before the first 2.5 s interval elapses

## [0.1.2] 27-04-2026

### Added
- `DEMO_MODE` define in `config.h`: skips WiFi/NTP and randomly flips all four
  digit cells every 2.5 s (`DEMO_CHANGE_MS`) to showcase the animation without
  needing network access. Comment out to restore clock mode.

## [0.1.1] 27-04-2026

### Fixed
- ESP32 Arduino core 3.x compatibility: replace deprecated `ledcSetup`/`ledcAttachPin`
  with `ledcAttach(pin, freq, res)` + `ledcWrite(pin, duty)` (channel-free API)
- Add `#include <FS.h>` before WiFiManager to expose `fs::FS` in global scope,
  fixing `FS was not declared in this scope` errors from WebServer.h on core 3.x
- Remove non-existent `wm.setAPName()` call (AP name is already passed to `autoConnect`)
- Remove unused `BACKLIGHT_CHANNEL` constant from `config.h`

## [0.1.0] 27-04-2026

### Added
- Initial project scaffold: `platformio.ini`, `partitions_custom.csv`, debug/config headers
- Python bitmap generator (`tools/generate_splitflap_bitmaps.py`) updated to accept
  a character-subset argument; clock build uses `"0123456789: "` only (~160 KB flash)
- `src/splitflap_bitmaps.cpp` — PROGMEM RGB565 arrays for digits 0–9, colon, blank;
  generated from 48×72 PT Sans Narrow Bold PNG assets
- `SplitFlapCell` class — sprite-based animation engine:
  - Phase 1: old top flap rotates away (dark gradient rectangle)
  - Phase 2: new bottom flap falls in (vertically-compressed character pixels, brightness ramp)
  - Multi-step catch-up: cycles through intermediate characters on startup sync
- `main.cpp` — HH:MM clock on CYD 320×240 landscape display
  - WiFiManager captive-portal provisioning
  - ezTime NTP sync with Australia/Sydney DST handling
  - Five 48×72 tiles (H0 H1 : M0 M1), centred at x=24 y=84
  - Static colon tile; four animated digit cells
