#!/usr/bin/env python3
import sys
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ASSET_ROOT = ROOT / "splitflap_cyd_assets"
OUT_HEADER = ROOT / "include" / "splitflap_bitmaps.h"
OUT_SOURCE = ROOT / "src" / "splitflap_bitmaps.cpp"

# Pass a chars argument to generate a subset, e.g.:
#   python3 generate_splitflap_bitmaps.py "0123456789: "
CHARS = sys.argv[1] if len(sys.argv) > 1 else "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789: "


def asset_name(ch: str) -> str:
    if ch == ":":
        return "COLON"
    if ch == " ":
        return "BLANK"
    return ch


def symbol_name(prefix: str, ch: str) -> str:
    if ch == ":":
        token = "COLON"
    elif ch == " ":
        token = "BLANK"
    elif ch.isdigit():
        token = f"DIGIT_{ch}"
    else:
        token = ch
    return f"{prefix}_{token}"


def rgb565_from_rgba(pixel):
    r, g, b, a = pixel
    if a < 255:
        r = (r * a) // 255
        g = (g * a) // 255
        b = (b * a) // 255
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def read_png(path: Path):
    image = Image.open(path).convert("RGBA")
    width, height = image.size
    if hasattr(image, "get_flattened_data"):
        raw_pixels = image.get_flattened_data()
    else:
        raw_pixels = image.getdata()
    pixels = [rgb565_from_rgba(pixel) for pixel in raw_pixels]
    return width, height, pixels


def emit_array(lines, name: str, pixels):
    lines.append(f"static const uint16_t {name}[] PROGMEM = {{")
    for start in range(0, len(pixels), 12):
        chunk = pixels[start : start + 12]
        values = ", ".join(f"0x{value:04X}" for value in chunk)
        lines.append(f"  {values},")
    lines.append("};")
    lines.append("")


def source_path(kind: str, ch: str) -> Path:
    name = asset_name(ch)
    if kind == "full":
        return ASSET_ROOT / "png_full_tiles_48x72" / f"{name}.png"
    if kind == "top":
        return ASSET_ROOT / "png_top_halves_48x36" / f"{name}_TOP.png"
    if kind == "bottom":
        return ASSET_ROOT / "png_bottom_halves_48x36" / f"{name}_BOTTOM.png"
    raise ValueError(kind)


def main():
    records = []
    source_lines = [
        '#include "splitflap_bitmaps.h"',
        "",
        "#include <Arduino.h>",
        "",
    ]

    for ch in CHARS:
        record = {"ch": ch}
        for kind in ("full", "top", "bottom"):
            path = source_path(kind, ch)
            width, height, pixels = read_png(path)
            name = symbol_name(f"SPLITFLAP_{kind.upper()}", ch)
            emit_array(source_lines, name, pixels)
            record[kind] = (name, width, height)
        records.append(record)

    source_lines.append("struct SplitflapRecord {")
    source_lines.append("  char ch;")
    source_lines.append("  SplitflapBitmap full;")
    source_lines.append("  SplitflapBitmap top;")
    source_lines.append("  SplitflapBitmap bottom;")
    source_lines.append("};")
    source_lines.append("")
    source_lines.append("static const SplitflapRecord SPLITFLAP_RECORDS[] = {")
    for record in records:
        source_lines.append(
            "  {"
            + repr(record["ch"])
            + ", {"
            + f"{record['full'][0]}, {record['full'][1]}, {record['full'][2]}"
            + "}, {"
            + f"{record['top'][0]}, {record['top'][1]}, {record['top'][2]}"
            + "}, {"
            + f"{record['bottom'][0]}, {record['bottom'][1]}, {record['bottom'][2]}"
            + "}},"
        )
    source_lines.append("};")
    source_lines.append("")
    source_lines.append("static const SplitflapBitmap& blankBitmap(SplitflapPart part) {")
    source_lines.append("  const SplitflapRecord& blank = SPLITFLAP_RECORDS[sizeof(SPLITFLAP_RECORDS) / sizeof(SPLITFLAP_RECORDS[0]) - 1];")
    source_lines.append("  if (part == SplitflapPart::Top) return blank.top;")
    source_lines.append("  if (part == SplitflapPart::Bottom) return blank.bottom;")
    source_lines.append("  return blank.full;")
    source_lines.append("}")
    source_lines.append("")
    source_lines.append("const SplitflapBitmap& splitflapBitmap(char ch, SplitflapPart part) {")
    source_lines.append("  if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';")
    source_lines.append("  for (const SplitflapRecord& record : SPLITFLAP_RECORDS) {")
    source_lines.append("    if (record.ch == ch) {")
    source_lines.append("      if (part == SplitflapPart::Top) return record.top;")
    source_lines.append("      if (part == SplitflapPart::Bottom) return record.bottom;")
    source_lines.append("      return record.full;")
    source_lines.append("    }")
    source_lines.append("  }")
    source_lines.append("  return blankBitmap(part);")
    source_lines.append("}")
    source_lines.append("")

    header = """#pragma once

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
"""

    OUT_HEADER.write_text(header)
    OUT_SOURCE.write_text("\n".join(source_lines))


if __name__ == "__main__":
    main()
