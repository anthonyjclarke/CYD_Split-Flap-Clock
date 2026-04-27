#pragma once

// ── Demo mode ─────────────────────────────────────────────────────────────────
// When defined: skips WiFi/NTP and randomly flips digits to show the animation.
// Comment out to run as a real clock.
#define DEMO_MODE
constexpr uint32_t DEMO_CHANGE_MS = 2500;   // ms between random digit changes
constexpr uint32_t DEMO_RESET_MS  = 60000;  // ms between full clear + rebuild

// ── WiFi ─────────────────────────────────────────────────────────────────────
#define WIFI_AP_NAME "SplitFlapClock"

// ── Timezone ──────────────────────────────────────────────────────────────────
#define TIMEZONE "Australia/Sydney"

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

// 5-tile layout (H0 H1 : M0 M1) centred on 320×240 landscape.
// Total content: 5×48 + 4×8 = 272 px → left margin = (320–272)/2 = 24 px
constexpr int CELL_Y     = 84;   // (240 – 72) / 2
constexpr int CELL_X_H0  = 24;
constexpr int CELL_X_H1  = 80;   // 24 + 48 + 8
constexpr int CELL_X_COL = 136;  // 80 + 48 + 8
constexpr int CELL_X_M0  = 192;  // 136 + 48 + 8
constexpr int CELL_X_M1  = 248;  // 192 + 48 + 8

// Screen background — matches the tile background colour (0x0841 ≈ RGB 8,8,8)
constexpr uint16_t SCREEN_BG = 0x0841;

// ── Text tile layout (demo mode — half-size tiles for day/date rows) ──────────
// Source bitmaps are always 48×72; these tiles are nearest-neighbour scaled.
constexpr int TEXT_TILE_W   = TILE_W / 2;   // 24 px
constexpr int TEXT_TILE_H   = TILE_H / 2;   // 36 px
constexpr int TEXT_TILE_GAP = 4;            // gap between text tiles

// Day-of-week row: always 9 cells — shorter names are centre-padded with spaces.
constexpr int DOW_CELLS = 9;
constexpr int DOW_X0    = (320 - (DOW_CELLS  * TEXT_TILE_W + (DOW_CELLS  - 1) * TEXT_TILE_GAP)) / 2;
constexpr int DOW_Y     = (CELL_Y - TEXT_TILE_H) / 2;

// Date row: 11 cells — always "DD MMM YYYY" (exactly 11 chars).
constexpr int DATE_CELLS = 11;
constexpr int DATE_X0    = (320 - (DATE_CELLS * TEXT_TILE_W + (DATE_CELLS - 1) * TEXT_TILE_GAP)) / 2;
constexpr int DATE_Y     = CELL_Y + TILE_H + (240 - CELL_Y - TILE_H - TEXT_TILE_H) / 2;

// ── Animation timing ─────────────────────────────────────────────────────────
// Each flip has two phases (old-top falls, new-bottom rises), FLAP_STEPS each.
// Total time per character step = 2 × FLAP_STEPS × FLAP_STEP_MS ms.
// Default: 2 × 4 × 20 = 160 ms per flip — close to a real Solari board.
constexpr int FLAP_STEPS   = 4;   // steps per animation phase
constexpr int FLAP_STEP_MS = 20;  // ms per step
