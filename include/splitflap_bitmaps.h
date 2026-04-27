#pragma once

#include <Arduino.h>

constexpr uint16_t SPLITFLAP_TILE_W = 48;
constexpr uint16_t SPLITFLAP_TILE_H = 72;
constexpr uint16_t SPLITFLAP_HALF_H = 36;

enum class SplitflapPart : uint8_t {
  Full,
  Top,
  Bottom,
};

struct SplitflapBitmap {
  const uint16_t* pixels;
  uint16_t width;
  uint16_t height;
};

const SplitflapBitmap& splitflapBitmap(char ch, SplitflapPart part = SplitflapPart::Full);
