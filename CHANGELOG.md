# Changelog

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
