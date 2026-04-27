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

void SplitFlapCell::begin(TFT_eSPI* tft, int x, int y, int tileW, int tileH, bool fullSeq) {
  _tft      = tft;
  _x        = x;
  _y        = y;
  _tileW    = tileW;
  _tileH    = tileH;
  _tileHalf = tileH / 2;
  _fullSeq  = fullSeq;
  _sprite = new TFT_eSprite(tft);
  _sprite->setColorDepth(16);
  _sprite->createSprite(_tileW, _tileH);
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
      _currentChar = _fullSeq ? nextInSeqFull(_currentChar) : nextInSeqDigit(_currentChar);
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

char SplitFlapCell::nextInSeqDigit(char c) {
  if (c == ' ')               return '0';
  if (c >= '0' && c <= '8')  return c + 1;
  return '0';   // '9' wraps back to '0'
}

char SplitFlapCell::nextInSeqFull(char c) {
  if (c == ' ')              return 'A';
  if (c >= 'A' && c < 'Z')  return c + 1;
  if (c == 'Z')              return '0';
  if (c >= '0' && c < '9')  return c + 1;
  return ' ';   // '9' wraps to blank
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
  // bmp source is always TILE_W × TILE_HALF; scale to _tileW × _tileHalf.
  for (int oy = 0; oy < _tileHalf; oy++) {
    int sy = oy * TILE_HALF / _tileHalf;
    const uint16_t* row = bmp.pixels + (size_t)sy * TILE_W;
    for (int ox = 0; ox < _tileW; ox++) {
      int sx = ox * TILE_W / _tileW;
      _sprite->drawPixel(ox, dstY + oy, pgm_read_word(row + sx));
    }
  }
}

void SplitFlapCell::blitScaledDark(const SplitflapBitmap& bmp, int dstH) {
  if (dstH <= 0) return;
  const int steps = max(dstH - 1, 1);
  for (int dy = 0; dy < dstH; dy++) {
    int sy = dy * (TILE_HALF - 1) / steps;
    uint8_t bright = (uint8_t)(50 + 180 * dy / steps);
    const uint16_t* row = bmp.pixels + (size_t)sy * TILE_W;
    for (int dx = 0; dx < _tileW; dx++) {
      int sx = dx * TILE_W / _tileW;
      _sprite->drawPixel(dx, _tileHalf + dy, dim(pgm_read_word(row + sx), bright));
    }
  }
}

void SplitFlapCell::renderIdle() {
  const SplitflapBitmap& full = splitflapBitmap(_currentChar, SplitflapPart::Full);
  for (int oy = 0; oy < _tileH; oy++) {
    int sy = oy * TILE_H / _tileH;
    const uint16_t* row = full.pixels + (size_t)sy * TILE_W;
    for (int ox = 0; ox < _tileW; ox++) {
      int sx = ox * TILE_W / _tileW;
      _sprite->drawPixel(ox, oy, pgm_read_word(row + sx));
    }
  }
  _sprite->drawFastHLine(0, _tileHalf - 1, _tileW, SHADOW);
  _sprite->drawFastHLine(0, _tileHalf,     _tileW, SHADOW);
  _sprite->pushSprite(_x, _y);
}

void SplitFlapCell::renderPhase1() {
  char newChar = _fullSeq ? nextInSeqFull(_currentChar) : nextInSeqDigit(_currentChar);
  blitHalf(splitflapBitmap(newChar,       SplitflapPart::Top),    0);
  blitHalf(splitflapBitmap(_currentChar,  SplitflapPart::Bottom), _tileHalf);

  int flapH = _tileHalf - _tileHalf * _animStep / FLAP_STEPS;
  for (int dy = 0; dy < flapH; dy++) {
    int denom = max(flapH - 1, 1);
    uint8_t rv = (uint8_t)(6 + 6 * (flapH - 1 - dy) / denom);
    uint16_t c = pack565(rv, rv, rv);
    _sprite->drawFastHLine(0, _tileHalf - flapH + dy, _tileW, c);
  }

  _sprite->drawFastHLine(0, _tileHalf - 1, _tileW, SHADOW);
  _sprite->drawFastHLine(0, _tileHalf,     _tileW, SHADOW);
  _sprite->pushSprite(_x, _y);
}

void SplitFlapCell::renderPhase2() {
  char newChar = _fullSeq ? nextInSeqFull(_currentChar) : nextInSeqDigit(_currentChar);
  blitHalf(splitflapBitmap(newChar,       SplitflapPart::Top),    0);
  blitHalf(splitflapBitmap(_currentChar,  SplitflapPart::Bottom), _tileHalf);

  int flapH = _tileHalf * _animStep / FLAP_STEPS;
  blitScaledDark(splitflapBitmap(newChar, SplitflapPart::Bottom), flapH);

  _sprite->drawFastHLine(0, _tileHalf - 1, _tileW, SHADOW);
  _sprite->drawFastHLine(0, _tileHalf,     _tileW, SHADOW);
  _sprite->pushSprite(_x, _y);
}
