# Changelog

## Todo

- Add a simple on-device WiFi/time status screen for failed WiFi or NTP sync.
- Add config options for brightness presets or auto-dimming by time of day.
- Add a button or touch gesture to toggle 12/24-hour display without reflashing.
- Add optional date formats, such as `DD MMM YYYY`, `MMM DD YYYY`, or ISO style.
- Add a brief DST/timezone diagnostic line to serial output after NTP sync.
- Colour options for Display (day, time, am/pm and date), configurable in config.h
- Add Version numbering

## Unreleased

### Added
- Startup self-test flips all animated time, day, AM/PM, and date cells to a
  diagnostic pattern, clears them, then lets the live clock roll in after NTP.

## [0.1.4] 2026-04-27

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

## [0.1.3] 2026-04-27

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

## [0.1.2] 2026-04-27

### Added
- `DEMO_MODE` define in `config.h`: skips WiFi/NTP and randomly flips all four
  digit cells every 2.5 s (`DEMO_CHANGE_MS`) to showcase the animation without
  needing network access. Comment out to restore clock mode.

## [0.1.1] 2026-04-27

### Fixed
- ESP32 Arduino core 3.x compatibility: replace deprecated `ledcSetup`/`ledcAttachPin`
  with `ledcAttach(pin, freq, res)` + `ledcWrite(pin, duty)` (channel-free API)
- Add `#include <FS.h>` before WiFiManager to expose `fs::FS` in global scope,
  fixing `FS was not declared in this scope` errors from WebServer.h on core 3.x
- Remove non-existent `wm.setAPName()` call (AP name is already passed to `autoConnect`)
- Remove unused `BACKLIGHT_CHANNEL` constant from `config.h`

## [0.1.0] 2026-04-27

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
