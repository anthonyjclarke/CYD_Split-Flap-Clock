# CLAUDE.md — CYD Split-Flap Clock

## What this project does

Simulates an airport-style Solari split-flap display as an HH:MM clock on the
ESP32-2432S028R (CYD — Cheap Yellow Display).  Each digit flips through
intermediate characters with a two-phase animation that mimics the mechanical
rotation of physical flaps: the old top half fades away as a dark gradient,
then the new bottom half appears as compressed, brightening character pixels.

## Hardware

- Board: ESP32-2432S028R
- Display: ILI9341 · 240×320 · landscape rotation 1 → effective 320×240
- No PSRAM on standard CYD; all bitmap data lives in flash (PROGMEM)

## Bitmap assets

Pre-generated 48×72 RGB565 tiles in `splitflap_cyd_assets/` (PT Sans Narrow
Bold, rasterised by a prior AI tool).  Run the generator to (re)build the C++
PROGMEM arrays:

```bash
# Clock subset only — fast compile (~160 KB flash)
python3 tools/generate_splitflap_bitmaps.py "0123456789: "

# Full A–Z + 0–9 subset — all tiles (~510 KB flash)
python3 tools/generate_splitflap_bitmaps.py
```

Both commands overwrite `include/splitflap_bitmaps.h` and
`src/splitflap_bitmaps.cpp`.

## Layout (landscape 320×240)

```
  x=24  x=80  x=136  x=192  x=248
   H0    H1     :      M0     M1      y=84, tile height 72 px
```

All tiles 48×72 px, 8 px gaps.  Colon is a static push at setup; digits are
`SplitFlapCell` instances ticked every loop().

## Animation timing

`FLAP_STEPS = 4`, `FLAP_STEP_MS = 20` → 160 ms per character flip.
Tune in `include/config.h`.

## Known issues / quirks

- If `pushImage` produces wrong colours after a library update, add
  `_sprite->setSwapBytes(false)` in `SplitFlapCell::begin()`.
- `drawPixel` loops used for bitmap blitting — adequate for 4 cells at 20 ms
  step intervals but could be replaced with `pushImage` for a speed-up.
- No touch support in this version.

## Flashing

```
pio run -t upload -e cyd
pio device monitor -e cyd
```

First boot: connect to "SplitFlapClock" AP, enter WiFi credentials.
Credentials are stored in NVS by WiFiManager; subsequent boots reconnect
automatically.
