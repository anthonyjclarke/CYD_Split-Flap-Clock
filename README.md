# CYD Split-Flap Clock

Animated split-flap clock firmware for a Cheap Yellow Display style ESP32 TFT
module. The display shows time as flipping tiles, with day of week above and
AM/PM plus date below.

## Current Behaviour

- Runs on a 320x240 ILI9341 CYD display in landscape orientation.
- Uses WiFiManager for first-run WiFi provisioning.
- Syncs time with ezTime/NTP using the configured timezone.
- Shows six animated time digits as `HH:MM:SS`, with static colon tiles.
- Shows a split-flap day row, an AM/PM row in 12-hour mode, and a `DD MMM YYYY`
  date row.
- Runs a startup self-test pattern before entering live clock mode.
- Renders generated RGB565 bitmap assets from PROGMEM, not from the filesystem at
  runtime.

Default configuration is in `include/config.h`:

- Timezone: `Australia/Sydney`
- Time format: 12-hour
- WiFiManager fallback AP: `SplitFlapClock`
- Startup self-test: enabled

## Hardware Target

The PlatformIO environment targets an ESP32 CYD-style board:

- `board = esp32dev`
- ILI9341 TFT via `TFT_eSPI`
- Display pins and SPI settings are defined in `platformio.ini`
- Backlight is driven from `TFT_BL` with LEDC PWM

## Project Layout

- `src/main.cpp` - display, WiFi, NTP, and clock update loop
- `src/SplitFlapCell.*` - sprite-based split-flap animation engine
- `include/config.h` - timezone, layout, animation, and display constants
- `include/debug.h` - serial debug macros
- `include/splitflap_bitmaps.h` and `src/splitflap_bitmaps.cpp` - generated
  PROGMEM bitmap lookup table used by the firmware
- `tools/generate_splitflap_bitmaps.py` - converts source PNGs into C++ bitmap
  arrays
- `splitflap_cyd_assets/` - source PNG asset pack
- `data/splitflap/` - filesystem copies of the PNG tiles; retained for asset
  reference, but not required by the current firmware path
- `partitions_custom.csv` - OTA-capable partition table with a SPIFFS partition

## Building And Uploading

Install PlatformIO, then build:

```sh
pio run
```

Upload firmware:

```sh
pio run -t upload
```

Monitor serial output:

```sh
pio device monitor
```

The current code includes `include/secrets.h`, which is intentionally ignored by
Git. If it is missing in a fresh checkout, create it with:

```cpp
#pragma once

#define SECRET_WIFI_SSID "your-ssid-here"
#define SECRET_WIFI_PASS "your-password-here"
```

The current WiFi flow uses the WiFiManager captive portal, so these values are
only a local placeholder unless the firmware is later changed to seed the portal
from them.

## First Run

1. Flash the firmware and open the serial monitor at 115200 baud.
2. If the ESP32 cannot connect to a saved WiFi network, join the
   `SplitFlapClock` access point.
3. Use the captive portal to configure WiFi.
4. After NTP sync, the display enters live clock mode.

If NTP does not sync within the initial timeout, the firmware continues running
and ezTime retries in the background.

## Regenerating Bitmap Assets

The firmware uses generated RGB565 arrays derived from the PNG assets in
`splitflap_cyd_assets/`.

Install Pillow if needed:

```sh
python3 -m pip install pillow
```

Regenerate the default character set:

```sh
python3 tools/generate_splitflap_bitmaps.py
```

Regenerate a smaller subset:

```sh
python3 tools/generate_splitflap_bitmaps.py "0123456789: "
```

The generator writes:

- `include/splitflap_bitmaps.h`
- `src/splitflap_bitmaps.cpp`

## Asset Pack

`splitflap_cyd_assets/` contains the original source assets:

- `png_full_tiles_48x72/` - complete tiles for A-Z, 0-9, colon, and blank
- `png_top_halves_48x36/` - top half of each tile
- `png_bottom_halves_48x36/` - bottom half of each tile
- `glyph_masks_1bit_32x48/` - monochrome glyph masks
- `splitflap_assets.h` - helper for PNG file names
- `manifest.json` - asset metadata

The generated C++ bitmap table is the source used by the current firmware.
