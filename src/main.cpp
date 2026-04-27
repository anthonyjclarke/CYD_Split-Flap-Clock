#include <Arduino.h>
#include <FS.h>
#include <TFT_eSPI.h>
#include <WiFiManager.h>
#include <ezTime.h>

#include "config.h"
#include "debug.h"
#include "secrets.h"
#include "splitflap_bitmaps.h"
#include "SplitFlapCell.h"

#ifdef DEMO_MODE
#include <Fonts/GFXFF/FreeSansBold18pt7b.h>
#endif

// ── Globals ───────────────────────────────────────────────────────────────────

TFT_eSPI tft;
Timezone myTZ;

// Four digit cells: H-tens, H-units, M-tens, M-units.
SplitFlapCell cells[4];

// ── Init functions ─────────────────────────────────────────────────────────────

static void initDisplay() {
  tft.init();
  tft.setRotation(1);           // landscape, USB connector on right
  tft.fillScreen(SCREEN_BG);

  ledcAttach(TFT_BL, BACKLIGHT_FREQ, BACKLIGHT_RES_BITS);
  ledcWrite(TFT_BL, BACKLIGHT_DUTY);

  DBG_INFO("Display init OK");
}

static void drawStaticColon() {
  // The colon tile never animates — push once during setup.
  const SplitflapBitmap& col = splitflapBitmap(':', SplitflapPart::Full);
  tft.startWrite();
  for (int y = 0; y < TILE_H; y++) {
    const uint16_t* row = col.pixels + (size_t)y * TILE_W;
    for (int x = 0; x < TILE_W; x++) {
      tft.drawPixel(CELL_X_COL + x, CELL_Y + y, pgm_read_word(row + x));
    }
  }
  tft.endWrite();
}

static void initCells() {
  cells[0].begin(&tft, CELL_X_H0, CELL_Y);
  cells[1].begin(&tft, CELL_X_H1, CELL_Y);
  cells[2].begin(&tft, CELL_X_M0, CELL_Y);
  cells[3].begin(&tft, CELL_X_M1, CELL_Y);
  drawStaticColon();
  DBG_INFO("Cells init OK");
}

static void initWiFi() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);  // 3-minute portal timeout

  DBG_INFO("Starting WiFiManager...");
  if (!wm.autoConnect(WIFI_AP_NAME)) {
    DBG_ERROR("WiFi connect failed — restarting");
    delay(1000);
    ESP.restart();
  }
  DBG_INFO("WiFi OK — IP %s", WiFi.localIP().toString().c_str());
}

static void initTime() {
  DBG_INFO("Syncing NTP...");
  myTZ.setLocation(F(TIMEZONE));
  waitForSync(30);   // 30 s timeout; carries on regardless
  if (timeStatus() == timeSet) {
    DBG_INFO("Time: %s", myTZ.dateTime().c_str());
  } else {
    DBG_WARN("NTP sync timed out — will retry in background");
  }
}

// ── Demo update ───────────────────────────────────────────────────────────────

#ifdef DEMO_MODE

static const char* const DEMO_DAYS[] = {
  "MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY","SUNDAY"
};
static const char* const DEMO_MONTHS[] = {
  "JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"
};
static char demoDOW[12];   // "WEDNESDAY\0" = 10 chars max
static char demoDate[13];  // "28 SEP 2026\0" = 12 chars

static void pickDemoValues() {
  strncpy(demoDOW, DEMO_DAYS[random(7)], sizeof(demoDOW) - 1);
  demoDOW[sizeof(demoDOW) - 1] = '\0';
  snprintf(demoDate, sizeof(demoDate), "%02d %s %04d",
           (int)random(1, 29), DEMO_MONTHS[random(12)], 2025 + (int)random(3));
}

static void clearDemoLabels() {
  tft.fillRect(0, 0,             320, CELL_Y,                    SCREEN_BG);
  tft.fillRect(0, CELL_Y + TILE_H, 320, 240 - CELL_Y - TILE_H, SCREEN_BG);
}

static void drawDemoLabels() {
  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.setTextColor(TFT_WHITE, SCREEN_BG);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(demoDOW,  160, CELL_Y / 2);
  tft.drawString(demoDate, 160, CELL_Y + TILE_H + (240 - CELL_Y - TILE_H) / 2);
}

static void setDemoDigits() {
  cells[0].setChar('0' + random(10));
  cells[1].setChar('0' + random(10));
  cells[2].setChar('0' + random(6));   // tens of minutes: 0–5
  cells[3].setChar('0' + random(10));
}

static void initDemo() {
  pickDemoValues();
  drawDemoLabels();
  setDemoDigits();
}

static void updateDemo() {
  static uint32_t lastChange = 0;
  static uint32_t lastMinute = 0;
  uint32_t now = millis();

  if (now - lastMinute >= DEMO_RESET_MS) {
    lastMinute = now;
    lastChange = now;
    for (int i = 0; i < 4; i++) cells[i].setChar(' ', false);
    clearDemoLabels();
    pickDemoValues();
    drawDemoLabels();
    setDemoDigits();
    return;
  }

  if (now - lastChange >= DEMO_CHANGE_MS) {
    lastChange = now;
    setDemoDigits();
  }
}

#endif

// ── Clock update ──────────────────────────────────────────────────────────────

static void updateClock() {
  if (timeStatus() != timeSet) return;

  static uint32_t lastSec = 0;
  uint32_t nowSec = (uint32_t)myTZ.now();
  if (nowSec == lastSec) return;
  lastSec = nowSec;

  int h = myTZ.hour();
  int m = myTZ.minute();

  DBG_VERBOSE("Time %02d:%02d", h, m);

  cells[0].setChar('0' + h / 10);
  cells[1].setChar('0' + h % 10);
  cells[2].setChar('0' + m / 10);
  cells[3].setChar('0' + m % 10);
}

// ── Arduino entry points ──────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  initDisplay();
  initCells();
#ifdef DEMO_MODE
  randomSeed(esp_random());
  initDemo();
#else
  initWiFi();
  initTime();
#endif
  DBG_INFO("Free heap: %d bytes", ESP.getFreeHeap());
}

void loop() {
  uint32_t now = millis();

#ifdef DEMO_MODE
  updateDemo();
#else
  events();       // ezTime background sync
  updateClock();
#endif

  for (int i = 0; i < 4; i++) {
    cells[i].tick(now);
  }
}
