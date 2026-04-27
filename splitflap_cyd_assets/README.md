# Split-Flap CYD Asset Pack

Prototype assets for recreating the airport / train-station split-flap effect on a CYD ESP32 TFT display.

## Contents

- `png_full_tiles_48x72/` — complete tiles for A–Z, 0–9, colon, blank
- `png_top_halves_48x36/` — top half of each tile
- `png_bottom_halves_48x36/` — bottom half of each tile
- `glyph_masks_1bit_32x48/` — simple monochrome glyph masks
- `splitflap_assets.h` — Arduino character-to-file-name helper
- `manifest.json` — asset metadata

## Suggested CYD clock layout

A 320 × 240 display can fit a `HH:MM` clock using five 48 × 72 tiles:

- Width: `5 × 48 = 240 px`
- X margin: `(320 – 240) / 2 = 40 px`
- Y position: about `84 px` for vertical centering

For seconds, use smaller generated tiles, e.g. 34 × 52.

## Animation model

For each changing character:

1. Draw the old character top half.
2. Draw the old character bottom half.
3. Animate a dark rotating flap over the lower half.
4. At halfway, swap to the new character.
5. Animate the top flap down into place.
6. Draw the full new tile.

On an ESP32/CYD, use a `TFT_eSprite` buffer to redraw only the affected tile rectangle.
