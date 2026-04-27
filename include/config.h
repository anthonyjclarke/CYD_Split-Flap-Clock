#pragma once

// ── Demo mode ─────────────────────────────────────────────────────────────────
// When defined: skips WiFi/NTP and randomly flips digits to show the animation.
// Comment out to run as a real clock.
#define DEMO_MODE
constexpr uint32_t DEMO_CHANGE_MS = 2500;  // ms between random digit changes

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

// ── Animation timing ─────────────────────────────────────────────────────────
// Each flip has two phases (old-top falls, new-bottom rises), FLAP_STEPS each.
// Total time per character step = 2 × FLAP_STEPS × FLAP_STEP_MS ms.
// Default: 2 × 4 × 20 = 160 ms per flip — close to a real Solari board.
constexpr int FLAP_STEPS   = 4;   // steps per animation phase
constexpr int FLAP_STEP_MS = 20;  // ms per step
