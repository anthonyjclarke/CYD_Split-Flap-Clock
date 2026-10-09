# Project: CYD Split-Flap Clock

ESP32-2432S028R (CYD) Solari-style split-flap clock: animated `HH:MM:SS` tiles, a day-of-week row above, and AM/PM plus `DD MMM YYYY` rows below. Each flip is a two-phase fade: old top half darkens, new bottom half compresses and brightens. Released 1.0.0; `dev` is 1.1.0-dev.

## Target Hardware

- Board: ESP32-2432S028R (Cheap Yellow Display), single env `cyd`
- Display: ILI9341, landscape rotation 1 (effective 320×240); standard CYD pins via `build_flags`, `TFT_RST=-1` (not wired)
- Backlight GPIO 21 on LEDC channel 0 (`BACKLIGHT_CHANNEL`)
- No PSRAM; all bitmaps in PROGMEM flash

## Platform & Libraries

- `espressif32@6.12.0` (arduino-esp32 2.0.17) – pinned for releases. Never use 3.x-only APIs (`ledcAttach(pin, …)`); use `ledcSetup` + `ledcAttachPin` + `ledcWrite(ch, …)`.
- TFT_eSPI 2.5.43, WiFiManager 2.0.17, ezTime 0.8.3

## Assets & Build

Tiles are 48×72 RGB565 PROGMEM arrays generated from `splitflap_cyd_assets/`. Regenerate with the **full** set – the day/date rows need A–Z:
```bash
python3 tools/generate_splitflap_bitmaps.py
```
Overwrites `include/splitflap_bitmaps.h` and `src/splitflap_bitmaps.cpp` (~510 KB). A digits-only subset (`"0123456789: "`) breaks the letter rows.

## Layout & Timing

All geometry is in `config.h`: 8 time tiles (H H : M M : S S) at 34×72, y=76; 9 day tiles at 32×48 above; 2 AM/PM and 11 date tiles at 24×36 below. Every tile nearest-neighbour scales the 48×72 source.

Animation: `FLAP_STEPS=4`, `FLAP_STEP_MS=20` → 160 ms per flip (tunable in `config.h`).

## Architecture & Quirks

- **Two-phase flip:** old digit's top half fades via dark gradient while new digit's bottom half compresses and brightens. Mimics mechanical flap rotation.
- **Bitmap blitting:** uses `drawPixel` loops into each cell's sprite, adequate for all 28 cells at 20 ms steps; no `pushImage` because colour swap issues arise post-library-update (add `setSwapBytes(false)` if needed).
- **NVS:** WiFiManager stores WiFi credentials; first boot spawns "SplitFlapClock" AP.
- **No PSRAM rule:** every bitmap must live in PROGMEM; no runtime allocation of tile data.

## Web installer and releases

- Release images come only from CI on a `v*` tag on `main`; never publish a local build.
- Never put `firmware-merged.bin` in a manifest (it wipes NVS on Update).
- `PROJECT_NAME` and `partitions_custom.csv` are frozen; a change turns Update into Install / needs an erase.
- Improv is vendored in `lib/ImprovWiFi`; never add it back to `lib_deps`.
- `improvTick()` must run at least every ~1 s, including inside any setup wait loop.
- No filesystem image: tiles are PROGMEM only; don't add a `data/` folder back.
- Before the next release, clear *Tests owed* in docs/WEB_INSTALLER.md (RUNBOOK 5b).
