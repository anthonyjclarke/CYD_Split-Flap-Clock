# Project: CYD Split-Flap Clock

ESP32-2432S028R (CYD) displays a Solari-style airport split-flap animation for HH:MM time. Each digit flips through intermediate characters with a two-phase fade: old top half darkens, new bottom half compresses and brightens. Version 1.0.

## Target Hardware

- Board: ESP32-2432S028R (Cheap Yellow Display)
- Display: ILI9341, 240×320, landscape rotation 1 (effective 320×240)
- No PSRAM; all bitmaps in PROGMEM flash

## Pin Mapping

| Function | GPIO | Notes |
|----------|------|-------|
| CS (display chip select) | 15 | ILI9341 SPI |
| DC (data/command) | 2 | ILI9341 SPI |
| RST (reset) | 4 | ILI9341 SPI |

Standard CYD SPI pins (CLK=14, MOSI=13, MISO=12) use global defaults.

## Libraries

- WiFiManager 2.0.16-rc.1
- Arduino core ESP32 (stable 2.x)
- ILI9341_2.4 (or compatible TFT_eSPI variant)

## Assets & Build

Pre-generated 48×72 RGB565 bitmap tiles in PROGMEM. Rebuild with:
```bash
python3 tools/generate_splitflap_bitmaps.py "0123456789: "
```
Overwrites `include/splitflap_bitmaps.h` and `src/splitflap_bitmaps.cpp`. Clock-only subset (~160 KB).

## Layout & Timing

Layout (landscape 320×240):
- H0 (hour tens) @ x=24, H1 @ x=80, colon @ x=136, M0 (min tens) @ x=192, M1 @ x=248
- All tiles 48×72 px with 8 px gaps, y=84

Animation: `FLAP_STEPS=4`, `FLAP_STEP_MS=20` → 160 ms per flip (tunable in `config.h`).

## Architecture & Quirks

- **Two-phase flip:** old digit's top half fades via dark gradient while new digit's bottom half compresses and brightens. Mimics mechanical flap rotation.
- **Bitmap blitting:** uses `drawPixel` loops adequate for 4 cells at 20 ms intervals; no `pushImage` because colour swap issues arise post-library-update (add `setSwapBytes(false)` if needed).
- **NVS:** WiFiManager stores WiFi credentials; first boot spawns "SplitFlapClock" AP.
- **No PSRAM rule:** every bitmap must live in PROGMEM; no runtime allocation of tile data.