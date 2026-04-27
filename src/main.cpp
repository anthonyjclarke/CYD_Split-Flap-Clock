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

// Time digit cells: H-tens, H-units, M-tens, M-units, S-tens, S-units.
SplitFlapCell cells[TIME_DIGITS];
SplitFlapCell dowCells[DOW_CELLS];
SplitFlapCell ampmCells[AMPM_CELLS];
SplitFlapCell dateCells[DATE_CELLS];

static const uint8_t TIME_DIGIT_SLOTS[TIME_DIGITS] = {0, 1, 3, 4, 6, 7};

static const char* const DAY_NAMES[] = {
  "", "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"
};

static const char* const MONTH_NAMES[] = {
  "", "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

// ── Init functions ─────────────────────────────────────────────────────────────

static void initDisplay() {
  DBG_INFO("Boot: initialising TFT display...");
  tft.init();
  DBG_INFO("Boot: setting TFT rotation...");
  tft.setRotation(1);           // landscape, USB connector on right
  DBG_INFO("Boot: clearing screen...");
  tft.fillScreen(SCREEN_BG);

  DBG_INFO("Boot: enabling backlight...");
  ledcAttach(TFT_BL, BACKLIGHT_FREQ, BACKLIGHT_RES_BITS);
  ledcWrite(TFT_BL, BACKLIGHT_DUTY);

  DBG_INFO("Boot: TFT display ready");
}

static int timeCharX(int slot) {
  return TIME_X0 + slot * (TIME_TILE_W + TIME_TILE_GAP);
}

static void drawStaticTimeChar(char c, int slot) {
  // Static separator tiles never animate, so push them once during setup.
  const SplitflapBitmap& bmp = splitflapBitmap(c, SplitflapPart::Full);
  tft.startWrite();
  for (int y = 0; y < TIME_TILE_H; y++) {
    int sy = y * TILE_H / TIME_TILE_H;
    const uint16_t* row = bmp.pixels + (size_t)sy * TILE_W;
    for (int x = 0; x < TIME_TILE_W; x++) {
      int sx = x * TILE_W / TIME_TILE_W;
      tft.drawPixel(timeCharX(slot) + x, CELL_Y + y, pgm_read_word(row + sx));
    }
  }
  tft.endWrite();
}

static void initCells() {
  DBG_INFO("Boot: initialising time cells...");
  for (int i = 0; i < TIME_DIGITS; i++) {
    cells[i].begin(&tft, timeCharX(TIME_DIGIT_SLOTS[i]), CELL_Y,
                   TIME_TILE_W, TIME_TILE_H);
  }
  DBG_INFO("Boot: drawing static time separators...");
  drawStaticTimeChar(':', 2);
  drawStaticTimeChar(':', 5);
  DBG_INFO("Boot: time cells ready");
}

static void initTextCells() {
  DBG_INFO("Boot: initialising day/date/AMPM cells...");
  for (int i = 0; i < DOW_CELLS; i++)
    dowCells[i].begin(&tft, DOW_X0 + i * (DOW_TILE_W + DOW_TILE_GAP), DOW_Y,
                      DOW_TILE_W, DOW_TILE_H, true);
  for (int i = 0; i < AMPM_CELLS; i++)
    ampmCells[i].begin(&tft, AMPM_X0 + i * (TEXT_TILE_W + TEXT_TILE_GAP), AMPM_Y,
                       TEXT_TILE_W, TEXT_TILE_H, true);
  for (int i = 0; i < DATE_CELLS; i++)
    dateCells[i].begin(&tft, DATE_X0 + i * (TEXT_TILE_W + TEXT_TILE_GAP), DATE_Y,
                       TEXT_TILE_W, TEXT_TILE_H, true);
  DBG_INFO("Boot: day/date/AMPM cells ready");
}

static void initWiFi() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);  // 3-minute portal timeout

  DBG_INFO("Boot: connecting to WiFi...");
  DBG_INFO("WiFi: AP fallback SSID '%s', portal timeout 180s", WIFI_AP_NAME);
  if (!wm.autoConnect(WIFI_AP_NAME)) {
    DBG_ERROR("WiFi connect failed — restarting");
    delay(1000);
    ESP.restart();
  }
  DBG_INFO("WiFi: connected, IP %s", WiFi.localIP().toString().c_str());
}

static void initTime() {
  DBG_INFO("Boot: configuring timezone %s", TIMEZONE);
  myTZ.setLocation(F(TIMEZONE));
  DBG_INFO("Boot: time display mode %s", USE_24_HOUR_TIME ? "24-hour" : "12-hour");
  DBG_INFO("Boot: syncing NTP...");
  waitForSync(30);   // 30 s timeout; carries on regardless
  if (timeStatus() == timeSet) {
    DBG_INFO("NTP: time set, %s", myTZ.dateTime().c_str());
  } else {
    DBG_WARN("NTP sync timed out — will retry in background");
  }
}

// ── Day/date helpers ──────────────────────────────────────────────────────────

static void centerText(char* out, int width, const char* text) {
  int len = strlen(text);
  if (len > width) len = width;
  int left = (width - len + 1) / 2;
  memset(out, ' ', width);
  memcpy(out + left, text, len);
  out[width] = '\0';
}

static void setTextCells(SplitFlapCell* targetCells, int count, const char* text,
                         bool animate) {
  for (int i = 0; i < count; i++)
    targetCells[i].setChar(text[i], animate);
}

static void tickAllCells(uint32_t now) {
  for (int i = 0; i < TIME_DIGITS; i++) cells[i].tick(now);
  for (int i = 0; i < DOW_CELLS;  i++) dowCells[i].tick(now);
  for (int i = 0; i < AMPM_CELLS; i++) ampmCells[i].tick(now);
  for (int i = 0; i < DATE_CELLS; i++) dateCells[i].tick(now);
}

static bool anyCellAnimating() {
  for (int i = 0; i < TIME_DIGITS; i++)
    if (cells[i].isAnimating()) return true;
  for (int i = 0; i < DOW_CELLS; i++)
    if (dowCells[i].isAnimating()) return true;
  for (int i = 0; i < AMPM_CELLS; i++)
    if (ampmCells[i].isAnimating()) return true;
  for (int i = 0; i < DATE_CELLS; i++)
    if (dateCells[i].isAnimating()) return true;
  return false;
}

static void waitForCellAnimations() {
  while (anyCellAnimating()) {
    tickAllCells(millis());
    delay(1);
  }
}

static void blankTextCells() {
  for (int i = 0; i < DOW_CELLS; i++) dowCells[i].setChar(' ', false);
  for (int i = 0; i < AMPM_CELLS; i++) ampmCells[i].setChar(' ', false);
  for (int i = 0; i < DATE_CELLS; i++) dateCells[i].setChar(' ', false);
}

static void blankAllAnimatedCells() {
  for (int i = 0; i < TIME_DIGITS; i++) cells[i].setChar(' ', false);
  blankTextCells();
}

static void runStartupSelfTest() {
  if (!RUN_STARTUP_SELF_TEST) return;

  DBG_INFO("Boot: running split-flap startup self-test...");

  for (int i = 0; i < TIME_DIGITS; i++) cells[i].setChar('8');
  setTextCells(dowCells, DOW_CELLS, "WEDNESDAY", true);
  setTextCells(ampmCells, AMPM_CELLS, "PM", true);
  setTextCells(dateCells, DATE_CELLS, "88 XXX 8888", true);
  waitForCellAnimations();

  DBG_INFO("Boot: clearing startup self-test pattern...");
  blankAllAnimatedCells();

  DBG_INFO("Boot: startup self-test complete");
}

static void updateDateRows(bool animate) {
  char dowText[DOW_CELLS + 1];
  char ampmText[AMPM_CELLS + 1];
  char dateText[DATE_CELLS + 1];

  uint8_t weekday = myTZ.weekday();
  uint8_t month = myTZ.month();
  const char* dayName = (weekday >= 1 && weekday <= 7) ? DAY_NAMES[weekday] : "";
  const char* monthName = (month >= 1 && month <= 12) ? MONTH_NAMES[month] : "";

  centerText(dowText, DOW_CELLS, dayName);
  centerText(ampmText, AMPM_CELLS,
             USE_24_HOUR_TIME ? "" : (myTZ.isAM() ? "AM" : "PM"));
  snprintf(dateText, sizeof(dateText), "%02u %s %04u",
           (unsigned)myTZ.day(), monthName, (unsigned)myTZ.year());

  if (animate) blankTextCells();
  setTextCells(dowCells, DOW_CELLS, dowText, animate);
  setTextCells(ampmCells, AMPM_CELLS, ampmText, animate);
  setTextCells(dateCells, DATE_CELLS, dateText, animate);
}

// ── Clock update ──────────────────────────────────────────────────────────────

static void updateClock() {
  if (timeStatus() != timeSet) return;

  static time_t lastSec = 0;
  static int lastMinute = -1;
  static bool dateRowsInitialized = false;
  time_t nowSec = myTZ.now();
  if (nowSec == lastSec) return;
  lastSec = nowSec;

  int h = USE_24_HOUR_TIME ? myTZ.hour() : myTZ.hourFormat12();
  int m = myTZ.minute();
  int s = myTZ.second();

  DBG_VERBOSE("Time %02d:%02d:%02d", h, m, s);

  if (!dateRowsInitialized) {
    updateDateRows(true);
    dateRowsInitialized = true;
  } else if (m != lastMinute) {
    updateDateRows(true);
  }
  lastMinute = m;

  cells[0].setChar('0' + h / 10);
  cells[1].setChar('0' + h % 10);
  cells[2].setChar('0' + m / 10);
  cells[3].setChar('0' + m % 10);
  cells[4].setChar('0' + s / 10);
  cells[5].setChar('0' + s % 10);
}

// ── Arduino entry points ──────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(50);
  DBG_INFO("Boot: Split-flap clock starting");
  initDisplay();
  initCells();
  initTextCells();
  runStartupSelfTest();
  DBG_INFO("Boot: live clock mode enabled");
  initWiFi();
  initTime();
  DBG_INFO("Boot: startup complete, free heap %d bytes", ESP.getFreeHeap());
}

void loop() {
  uint32_t now = millis();

  events();       // ezTime background sync
  updateClock();
  tickAllCells(now);
}
