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
static void updateDemo() {
  static uint32_t lastChange = 0;
  uint32_t now = millis();
  if (now - lastChange < DEMO_CHANGE_MS) return;
  lastChange = now;
  for (int i = 0; i < 4; i++) {
    cells[i].setChar('0' + random(10));
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
  updateDemo();   // populate cells immediately rather than waiting first interval
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
