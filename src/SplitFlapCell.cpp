#include "SplitFlapCell.h"
#include "debug.h"
#include <pgmspace.h>

// Dark horizontal line at the centre of each tile — the physical hinge gap.
static const uint16_t SHADOW = 0x0000;

// ── Constructor / destructor ───────────────────────────────────────────────────

SplitFlapCell::SplitFlapCell() {}

SplitFlapCell::~SplitFlapCell() {
  if (_sprite) {
    _sprite->deleteSprite();
    delete _sprite;
  }
}

void SplitFlapCell::begin(TFT_eSPI* tft, int x, int y) {
  _tft = tft;
  _x   = x;
  _y   = y;
  _sprite = new TFT_eSprite(tft);
  _sprite->setColorDepth(16);
  _sprite->createSprite(TILE_W, TILE_H);
  renderIdle();
}

// ── Public API ────────────────────────────────────────────────────────────────

void SplitFlapCell::setChar(char c, bool animate) {
  if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
  if (_targetChar == c) return;  // nothing to do
  _targetChar = c;
  if (!animate) {
    _currentChar = c;
    _targetChar  = c;
    _animPhase   = 0;
    renderIdle();
  }
}

void SplitFlapCell::tick(uint32_t nowMs) {
  // Start a new flip if idle and target differs from current.
  if (_animPhase == 0) {
    if (_currentChar == _targetChar) return;
    _animPhase   = 1;
    _animStep    = 0;
    _nextStepMs  = nowMs + FLAP_STEP_MS;
    renderPhase1();
    return;
  }

  // Not time for the next step yet.
  if ((int32_t)(nowMs - _nextStepMs) < 0) return;
  _nextStepMs += FLAP_STEP_MS;
  _animStep++;

  if (_animPhase == 1) {
    // ── Phase 1: old top falls away ─────────────────────────────────────────
    if (_animStep >= FLAP_STEPS) {
      _animPhase = 2;
      _animStep  = 0;
      renderPhase2();
    } else {
      renderPhase1();
    }
  } else {
    // ── Phase 2: new bottom rises into place ─────────────────────────────────
    if (_animStep >= FLAP_STEPS) {
      _currentChar = nextInSeq(_currentChar);
      if (_currentChar == _targetChar) {
        _animPhase = 0;
        renderIdle();
      } else {
        // More flips needed — start the next one immediately.
        _animPhase  = 1;
        _animStep   = 0;
        renderPhase1();
      }
    } else {
      renderPhase2();
    }
  }
}

// ── Static helpers ────────────────────────────────────────────────────────────

char SplitFlapCell::nextInSeq(char c) {
  if (c == ' ')               return '0';
  if (c >= '0' && c <= '8')  return c + 1;
  return '0';   // '9' wraps back to '0'
}

uint16_t SplitFlapCell::dim(uint16_t rgb, uint8_t bright) {
  uint8_t r = (uint8_t)((((rgb >> 11) & 0x1F) * bright) >> 8);
  uint8_t g = (uint8_t)((((rgb >>  5) & 0x3F) * bright) >> 8);
  uint8_t b = (uint8_t)(( (rgb        & 0x1F) * bright) >> 8);
  return ((uint16_t)r << 11) | ((uint16_t)g << 5) | b;
}

uint16_t SplitFlapCell::pack565(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void SplitFlapCell::blitHalf(const SplitflapBitmap& bmp, int dstY) {
  for (int y = 0; y < TILE_HALF; y++) {
    const uint16_t* row = bmp.pixels + (size_t)y * TILE_W;
    for (int x = 0; x < TILE_W; x++) {
      _sprite->drawPixel(x, dstY + y, pgm_read_word(row + x));
    }
  }
}

void SplitFlapCell::blitScaledDark(const SplitflapBitmap& bmp, int dstH) {
  // Draw dstH rows starting at y=TILE_HALF in the sprite.
  // Each row maps to a source row in bmp (48×36), vertically compressed.
  // Brightness goes from very dark at the hinge (y=TILE_HALF, dy=0)
  // to near-full at the free edge (dy=dstH-1).
  if (dstH <= 0) return;
  const int srcH  = TILE_HALF;
  const int steps = max(dstH - 1, 1);
  for (int dy = 0; dy < dstH; dy++) {
    int sy     = dy * (srcH - 1) / steps;
    // bright 50 → 230 as the flap settles (hinge dark, free edge bright)
    uint8_t bright = (uint8_t)(50 + 180 * dy / steps);
    const uint16_t* row = bmp.pixels + (size_t)sy * TILE_W;
    for (int dx = 0; dx < TILE_W; dx++) {
      _sprite->drawPixel(dx, TILE_HALF + dy, dim(pgm_read_word(row + dx), bright));
    }
  }
}

void SplitFlapCell::renderIdle() {
  const SplitflapBitmap& full = splitflapBitmap(_currentChar, SplitflapPart::Full);
  for (int y = 0; y < TILE_H; y++) {
    const uint16_t* row = full.pixels + (size_t)y * TILE_W;
    for (int x = 0; x < TILE_W; x++) {
      _sprite->drawPixel(x, y, pgm_read_word(row + x));
    }
  }
  // Reinforce the centre shadow line — the physical gap between flaps.
  _sprite->drawFastHLine(0, TILE_HALF - 1, TILE_W, SHADOW);
  _sprite->drawFastHLine(0, TILE_HALF,     TILE_W, SHADOW);
  _sprite->pushSprite(_x, _y);
}

void SplitFlapCell::renderPhase1() {
  // Background: new-char top half revealed, old-char bottom half static.
  char newChar = nextInSeq(_currentChar);
  blitHalf(splitflapBitmap(newChar,       SplitflapPart::Top),    0);
  blitHalf(splitflapBitmap(_currentChar,  SplitflapPart::Bottom), TILE_HALF);

  // Old top flap falling: a dark gradient rectangle shrinks from TILE_HALF→0.
  // step 0 → flapH=TILE_HALF (covers new top completely)
  // step FLAP_STEPS-1 → flapH=TILE_HALF/FLAP_STEPS (thin sliver)
  int flapH = TILE_HALF - TILE_HALF * _animStep / FLAP_STEPS;
  for (int dy = 0; dy < flapH; dy++) {
    // Free edge (top, dy=0) catches a little ambient light — slightly lighter.
    // Hinge (bottom, dy=flapH-1) is deepest shadow.
    // Values stay well below tile background (0x0841 = 8,8,8) so the
    // back face always looks visually darker than the revealed new character.
    int denom = max(flapH - 1, 1);
    uint8_t rv = (uint8_t)(6 + 6 * (flapH - 1 - dy) / denom); // 6..12
    uint16_t c = pack565(rv, rv, rv);
    _sprite->drawFastHLine(0, TILE_HALF - flapH + dy, TILE_W, c);
  }

  _sprite->drawFastHLine(0, TILE_HALF - 1, TILE_W, SHADOW);
  _sprite->drawFastHLine(0, TILE_HALF,     TILE_W, SHADOW);
  _sprite->pushSprite(_x, _y);
}

void SplitFlapCell::renderPhase2() {
  // Background: new-char top static, old-char bottom (visible until covered).
  char newChar = nextInSeq(_currentChar);
  blitHalf(splitflapBitmap(newChar,       SplitflapPart::Top),    0);
  blitHalf(splitflapBitmap(_currentChar,  SplitflapPart::Bottom), TILE_HALF);

  // New bottom flap falling in: character pixels appear, growing 0→TILE_HALF.
  int flapH = TILE_HALF * _animStep / FLAP_STEPS;
  blitScaledDark(splitflapBitmap(newChar, SplitflapPart::Bottom), flapH);

  _sprite->drawFastHLine(0, TILE_HALF - 1, TILE_W, SHADOW);
  _sprite->drawFastHLine(0, TILE_HALF,     TILE_W, SHADOW);
  _sprite->pushSprite(_x, _y);
}
