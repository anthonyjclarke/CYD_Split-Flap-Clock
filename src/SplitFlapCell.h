#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "splitflap_bitmaps.h"
#include "config.h"

// Animates a single 48×72 split-flap tile on the CYD display.
//
// Animation model (per flip):
//   Phase 1 — old top flap rotates away (back face = dark gradient covers new top)
//   Phase 2 — new bottom flap falls in  (front face = scaled new-char pixels)
//
// Call tick() from loop().  Call setChar() whenever the displayed character
// should change; it queues the new target and tick() drives each individual
// flip in sequence, cycling through intermediate characters until target is
// reached (authentic multi-click behaviour on startup catch-up).

class SplitFlapCell {
public:
  SplitFlapCell();
  ~SplitFlapCell();

  // tileW/tileH default to the full 48×72 asset size.  Pass smaller values to
  // render a downscaled tile (source bitmaps are sampled by nearest-neighbour).
  // fullSeq=true uses the Solari ' '→A→…→Z→0→…→9→' ' cycle; false = digits only.
  void begin(TFT_eSPI* tft, int x, int y,
             int tileW = TILE_W, int tileH = TILE_H, bool fullSeq = false);

  // Set target character.  If animate=false, jumps instantly with no sprite work.
  void setChar(char c, bool animate = true);

  // Advance animation state.  Call every loop iteration with millis().
  void tick(uint32_t nowMs);

  bool isAnimating() const { return _animPhase != 0 || _currentChar != _targetChar; }
  char currentChar()  const { return _currentChar; }

private:
  TFT_eSPI*    _tft    = nullptr;
  TFT_eSprite* _sprite = nullptr;
  int  _x = 0, _y = 0;
  int  _tileW = TILE_W, _tileH = TILE_H, _tileHalf = TILE_HALF;
  bool _fullSeq = false;

  char     _currentChar = ' ';
  char     _targetChar  = ' ';

  int      _animPhase = 0;
  int      _animStep  = 0;
  uint32_t _nextStepMs = 0;

  // ── helpers ──────────────────────────────────────────────────────────────

  // Digits-only cycle: ' '→0→1→…→9→0 (wraps).
  static char nextInSeqDigit(char c);
  // Full Solari cycle: ' '→A→…→Z→0→…→9→' '.
  static char nextInSeqFull(char c);

  // Render helpers — all write to _sprite and call pushSprite.
  void renderIdle();
  void renderPhase1();  // old top falling (dark gradient)
  void renderPhase2();  // new bottom rising (compressed character pixels)

  // Copy a 48×TILE_HALF bitmap into the sprite at (0, dstY).
  void blitHalf(const SplitflapBitmap& bmp, int dstY);

  // Copy new-char bottom half into sprite at (0, TILE_HALF), compressed to
  // dstH rows with per-pixel brightness ramp (dark at hinge, bright at edge).
  void blitScaledDark(const SplitflapBitmap& bmp, int dstH);

  // RGB565 with per-channel brightness multiply (0=black, 255=unchanged).
  static uint16_t dim(uint16_t rgb565, uint8_t bright);

  // Pack 8-bit R,G,B into RGB565.
  static uint16_t pack565(uint8_t r, uint8_t g, uint8_t b);
};
