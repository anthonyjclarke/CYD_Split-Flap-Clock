#pragma once

// ── WiFi ─────────────────────────────────────────────────────────────────────
#define WIFI_AP_NAME "SplitFlapClock"

// ── Timezone ──────────────────────────────────────────────────────────────────
#define TIMEZONE "Australia/Sydney"
constexpr bool USE_24_HOUR_TIME = false;
constexpr bool RUN_STARTUP_SELF_TEST = true;

// ── Backlight ─────────────────────────────────────────────────────────────────
constexpr uint32_t BACKLIGHT_FREQ    = 5000;
constexpr uint8_t BACKLIGHT_RES_BITS = 8;
constexpr uint8_t BACKLIGHT_DUTY     = 200;   // 0–255; ~78% brightness

// ── Tile geometry ─────────────────────────────────────────────────────────────
// Tile size matches the pre-generated PNG assets (48×72 full, 48×36 halves).
constexpr int TILE_W    = 48;
constexpr int TILE_H    = 72;
constexpr int TILE_HALF = 36;
constexpr int TILE_GAP  = 8;   // gap between tiles in pixels

// 8-tile layout (H0 H1 : M0 M1 : S0 S1) centred on 320×240 landscape.
// Tiles keep the original 72 px height and are scaled narrower to fit.
constexpr int TIME_CHARS    = 8;
constexpr int TIME_DIGITS   = 6;
constexpr int TIME_TILE_W   = 34;
constexpr int TIME_TILE_H   = TILE_H;
constexpr int TIME_TILE_GAP = 4;
constexpr int CELL_Y        = 76;
constexpr int TIME_X0       = (320 - (TIME_CHARS * TIME_TILE_W + (TIME_CHARS - 1) * TIME_TILE_GAP)) / 2;

// Screen background — matches the tile background colour (0x0841 ≈ RGB 8,8,8)
constexpr uint16_t SCREEN_BG = 0x0841;

// ── Text tile layout (half-size tiles for day/date rows) ──────────────────────
// Source bitmaps are always 48×72; these tiles are nearest-neighbour scaled.
constexpr int TEXT_TILE_W   = TILE_W / 2;   // 24 px
constexpr int TEXT_TILE_H   = TILE_H / 2;   // 36 px
constexpr int TEXT_TILE_GAP = 4;            // gap between text tiles
constexpr int TEXT_ROW_GAP  = 6;

// Day-of-week row: always 9 cells — shorter names are centre-padded with spaces.
constexpr int DOW_CELLS = 9;
constexpr int DOW_TILE_W   = 32;
constexpr int DOW_TILE_H   = 48;
constexpr int DOW_TILE_GAP = 3;
constexpr int DOW_X0       = (320 - (DOW_CELLS * DOW_TILE_W + (DOW_CELLS - 1) * DOW_TILE_GAP)) / 2;
constexpr int DOW_Y        = (CELL_Y - DOW_TILE_H) / 2;

// AM/PM row: 2 cells, blank in 24-hour mode.
constexpr int AMPM_CELLS = 2;
constexpr int AMPM_X0    = (320 - (AMPM_CELLS * TEXT_TILE_W + (AMPM_CELLS - 1) * TEXT_TILE_GAP)) / 2;
constexpr int AMPM_Y     = CELL_Y + TIME_TILE_H + TEXT_ROW_GAP;

// Date row: 11 cells — always "DD MMM YYYY" (exactly 11 chars).
constexpr int DATE_CELLS = 11;
constexpr int DATE_X0    = (320 - (DATE_CELLS * TEXT_TILE_W + (DATE_CELLS - 1) * TEXT_TILE_GAP)) / 2;
constexpr int DATE_Y     = AMPM_Y + TEXT_TILE_H + TEXT_ROW_GAP;

// ── Animation timing ─────────────────────────────────────────────────────────
// Each flip has two phases (old-top falls, new-bottom rises), FLAP_STEPS each.
// Total time per character step = 2 × FLAP_STEPS × FLAP_STEP_MS ms.
// Default: 2 × 4 × 20 = 160 ms per flip — close to a real Solari board.
constexpr int FLAP_STEPS   = 4;   // steps per animation phase
constexpr int FLAP_STEP_MS = 20;  // ms per step
