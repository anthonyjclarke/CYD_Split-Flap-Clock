# CYD Split-Flap Clock

Animated split-flap clock firmware for a Cheap Yellow Display style ESP32 TFT
module. The display shows time as flipping tiles, with day of week above and
AM/PM plus date below.

## Install

**[anthonyjclarke.github.io/CYD_Split-Flap-Clock][installer]** installs the
latest release from the browser – no PlatformIO, no drivers to build. It needs
desktop Chrome, Edge or Opera.

1. Pick your board – CYD 2.8″ (ESP32-2432S028R).
2. Plug it in with a USB data cable, click **Connect & install** and choose its
   port.
3. On a new board, say yes to erasing it. When flashing finishes, choose
   **Configure WiFi** and pick your network. Alternatively, join the
   `SplitFlapClock` hotspot and set WiFi from its portal.
4. The clock runs its flip self-test, joins WiFi and shows the time after NTP.

A board already running this firmware is recognised and offered **Update**,
which keeps its WiFi settings. Each [release][releases] also carries the images
for flashing by hand. `*-firmware.bin` is the app alone, for a web OTA update
page (this firmware has none yet). `*-merged.bin` is a clean install at `0x0`
with esptool, and it **erases settings and WiFi**.

Nothing else is needed: no API keys, no filesystem upload. The timezone
(`Australia/Sydney`) and 12/24-hour mode are compile-time settings in
`include/config.h`.

[installer]: https://anthonyjclarke.github.io/CYD_Split-Flap-Clock/
[releases]: https://github.com/anthonyjclarke/CYD_Split-Flap-Clock/releases

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

The build needs no `secrets.h`. WiFi is set at runtime through Improv (the web
installer) or the WiFiManager portal. The platform is pinned to
`espressif32@6.12.0` (arduino-esp32 2.0.17).

Release images come only from CI on a `v*` tag (`.github/workflows/firmware.yml`,
using [cyd-web-installer](https://github.com/anthonyjclarke/cyd-web-installer)).
Never publish a local build or a local `_site/`, because they can contain
whatever is in your local `include/` folder. Installer tests are recorded in
[`docs/WEB_INSTALLER.md`](docs/WEB_INSTALLER.md).

## First Run

1. Flash the firmware (or use the web installer above) and open the serial
   monitor at 115200 baud.
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
