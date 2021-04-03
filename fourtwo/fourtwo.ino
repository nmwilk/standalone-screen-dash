#include "tft.h"
#include "simhubreader.h"
#include <FastLED.h>

#define SCREEN_WIDTH 480  // screen width
#define SCREEN_HEIGHT 320  // screen height

#define CELL_WIDTH ((SCREEN_WIDTH) / 9)
#define CELL_HEIGHT ((SCREEN_HEIGHT) / 4)

#define CELL_BORDER_THICKNESS 2
#define CELL_PADDING 2

#define REVS_LED_COUNT 16
#define STATUS_LED_COUNT 16

#define RevsChangeLimit 97

static lgfx::LGFX_SPI<LGFX_Config> tft;
static LGFX_Sprite sprite(&tft);
static lgfx::Panel_ILI9488 panel;

char oldGear = '0';
int oldSpeed = -1;
int oldTime = -1;

#define COLOR_BG TFT_BLACK
#define COLOR_TEXT TFT_WHITE

const unsigned int GEAR_COLOR = tft.color565(0, 0, 0);
const unsigned int RED_COLOR = tft.color565(255, 0, 0);

const byte ColorRed[3] = {100, 0, 0};
const byte ColorOrange[3] = {100, 60, 00};
const byte ColorYellow[3] = {100, 90, 00};
const byte ColorGreen[3] = {0, 100, 0};
const byte ColorDeepOrange[3] = {100, 37, 00};
const byte ColorBlue[3] = {0, 0, 100};
const byte ColorWhite[3] = {100, 100, 100};
const byte ColorNone[3] = {0, 0, 0};

const byte RevBoundaries[8] = {0, 10, 30, 50, 70, 80, 90, RevsChangeLimit};
const byte RevColors[8][4] = {
  {RevBoundaries[0], 0,  80, 0},
  {RevBoundaries[1], 80, 80, 0},
  {RevBoundaries[2], 80, 80, 0},
  {RevBoundaries[3], 80, 50, 0},
  {RevBoundaries[4], 80, 30, 0},
  {RevBoundaries[5], 80, 20, 0},
  {RevBoundaries[6], 80, 10, 0},
  {RevBoundaries[7], 0,  30, 80}
};

const byte* PitLimiterColors[4] = { ColorWhite, ColorNone, ColorRed, ColorBlue };

enum FlagsState {
  None,
  Green,
  Yellow,
  Blue
};

CRGB leds[REVS_LED_COUNT + STATUS_LED_COUNT];
SimHubReader simHubReader;

char drawCharBuf[2];
FlagsState flagState = None;

char prevLapTime[LAP_TIME_STRLEN];
char prevBestLapTime[LAP_TIME_STRLEN];
char prevLastLapTime[LAP_TIME_STRLEN];
char prevTyreTempFL[TYRE_PRESSURE_BUF_SIZE + 1];
char prevTyreTempFR[TYRE_PRESSURE_BUF_SIZE + 1];
char prevTyreTempRL[TYRE_PRESSURE_BUF_SIZE + 1];
char prevTyreTempRR[TYRE_PRESSURE_BUF_SIZE + 1];

char prevTcLevel = -1;
char prevAbsLevel = -1;
char prevMapLevel = -1;
int prevSpeed = 0;

int prevFuel = -1;
int prevBrakeBias = -1;
int prevPosition = -1;
int prevLapNumber = -1;

float prevGapAhead;
float prevGapBehind;
float prevLapDelta;

void setup() {
  Serial.begin(115200);
  simHubReader.begin();

  setTft(&tft, &panel);
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  memset(prevLapTime, 0x0, LAP_TIME_STRLEN);
  memset(prevBestLapTime, 0x0, LAP_TIME_STRLEN);
  memset(prevLastLapTime, 0x0, LAP_TIME_STRLEN);

  memset(prevTyreTempFL, 0x0, TYRE_PRESSURE_BUF_SIZE + 1);
  memset(prevTyreTempFR, 0x0, TYRE_PRESSURE_BUF_SIZE + 1);
  memset(prevTyreTempRL, 0x0, TYRE_PRESSURE_BUF_SIZE + 1);
  memset(prevTyreTempRR, 0x0, TYRE_PRESSURE_BUF_SIZE + 1);

  drawCell(2, 1, "Gaps", 0, 0, TFT_GREEN);

  drawCell(2, 2, "Gear", 4, 1, TFT_WHITE);
  drawCell(2, 1, "Delta", 4, 0, TFT_WHITE);
  drawCell(2, 1, "Fuel", 4, 3, TFT_WHITE);

  drawCell(3, 1, "Lap Time", 6, 0, TFT_GREEN);
  drawCell(3, 1, "Last Lap", 6, 1, TFT_WHITE);
  drawCell(3, 1, "Best Lap", 6, 2, TFT_GREEN);

  drawCell(2, 1, "Speed", 2, 1, TFT_WHITE);

  drawCell(1, 1, "Pos", 0, 1, TFT_WHITE);
  drawCell(1, 1, "Lap", 1, 1, TFT_WHITE);
  drawCell(1, 1, "DRS", 2, 0, TFT_ORANGE);
  drawCell(1, 1, "P2P", 3, 0, TFT_ORANGE);

  drawCell(3, 2, "Tyres", 0, 2, TFT_ORANGE);

  drawCell(2, 1, "Bias", 7, 3, TFT_RED);
  drawCell(1, 1, "Map", 6, 3, TFT_RED);
  drawCell(1, 1, "TC", 3, 2, TFT_YELLOW);
  drawCell(1, 1, "ABS", 3, 3, TFT_ORANGE);

  FastLED.addLeds<NEOPIXEL, 18>(leds, REVS_LED_COUNT + STATUS_LED_COUNT);
}

void loop() {
  int ms = millis();

  simHubReader.tick(ms);

  drawGear();
  drawSpeed();

  drawLapDelta();

  drawGaps();

  drawLapTimes();

  drawCellValueChar(1, 1, simHubReader.getTcLevel(), &prevTcLevel, 3, 2, TFT_WHITE, true, 0);
  drawCellValueChar(1, 1, simHubReader.getAbsLevel(), &prevAbsLevel, 3, 3, TFT_WHITE, true, 0);
  drawCellValueChar(1, 1, simHubReader.getMapLevel(), &prevMapLevel, 6, 3, TFT_WHITE, true, 0);

  drawTyres();

  drawCellValueInt(2, 1, simHubReader.getFuel(), &prevFuel, 4, 3, TFT_WHITE, true, 0);
  drawCellValueInt(2, 1, simHubReader.getBrakeBias(), &prevBrakeBias, 7, 3, TFT_WHITE, true, 0);
  drawCellValueInt(1, 1, simHubReader.getPosition(), &prevPosition, 0, 1, TFT_WHITE, false, 0);
  drawCellValueInt(1, 1, simHubReader.getLapNumber(), &prevLapNumber, 1, 1, TFT_WHITE, false, 0);

  drawRevBar(ms, simHubReader.getRpm());
  drawStatusLights(ms);

  FastLED.show();
}

void drawLapTimes() {
  char* lapTime = simHubReader.getLapTime();
  char* lastLapTime = simHubReader.getLastLapTime();
  char* bestLapTime = simHubReader.getBestLapTime();

  drawCellValue(3, 1, lapTime, prevLapTime, 6, 0, TFT_WHITE, false, -3);
  drawCellValue(3, 1, lastLapTime, prevLastLapTime, 6, 1, TFT_WHITE, false, -3);
  drawCellValue(3, 1, bestLapTime, prevBestLapTime, 6, 2, TFT_WHITE, false, -3);
}

void drawTyres() {
  drawCellValue(1.25, 1, simHubReader.getTyreTempFL(), prevTyreTempFL, 0.25, 2, TFT_WHITE, false, -2);
  drawCellValue(1.25, 1, simHubReader.getTyreTempFR(), prevTyreTempFR, 1.5, 2, TFT_WHITE, false, -2);
  drawCellValue(1.25, 1, simHubReader.getTyreTempRL(), prevTyreTempRL, 0.25, 2.8, TFT_WHITE, false, -8);
  drawCellValue(1.25, 1, simHubReader.getTyreTempRR(), prevTyreTempRR, 1.5, 2.8, TFT_WHITE, false, -8);
}

void drawGaps() {
  drawCellValueFloat(2, 1, simHubReader.getGapAhead(), &prevGapAhead, 0, 0, TFT_WHITE, false, -14, 2);
  drawCellValueFloat(2, 1, simHubReader.getGapBehind(), &prevGapBehind, 0, 0, TFT_WHITE, false, 12, 2);
}

void drawGear() {
  char newGear = simHubReader.getGear();
  int x;
  int y;
  fillXY(2, 2, 4, 1, -6, &x, &y);
  drawChar(newGear, &oldGear, &fonts::Font8, 1, x, y);
}

void drawSpeed() {
  drawCellValueInt(2, 1, simHubReader.getSpeed(), &prevSpeed, 2, 1, TFT_WHITE, true, 0);
}

void drawLapDelta() {
  float lapDelta = simHubReader.getLapDelta();
  drawCellValueFloat(2, 1, lapDelta, &prevLapDelta, 4, 0, lapDelta < 0 ? TFT_GREEN : TFT_RED, false, -3, 2);
}

void drawChar(char newValue, char *oldValue, const lgfx::BaseFont* font, float fontScale, int posX, int posY) {
  if (*oldValue != newValue) {
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setFont(font);
    tft.setTextSize(fontScale);

    tft.setTextColor(TFT_BLACK);
    sprintf(drawCharBuf, "%c", *oldValue);
    tft.drawString(drawCharBuf, posX, posY);

    tft.setTextColor(TFT_WHITE);
    sprintf(drawCharBuf, "%c", newValue);
    tft.drawString(drawCharBuf, posX, posY);

    *oldValue = newValue;
  }
}

long firstRevsChangeTime = 0;

void drawRevBar(long ms, int rpm) {
  if (simHubReader.isPitLimiter()) {
    int tick = (ms % 800) / 200;
    int colorA = tick;
    int colorB = (tick + 2) % 4;
    for (int led = 0; led < 8; led++) {
      const byte* color = led % 2 == 0 ? PitLimiterColors[colorA] : PitLimiterColors[colorB];
      leds[7 - led].setRGB(color[0], color[1], color[2]);
      leds[8 + led].setRGB(color[0], color[1], color[2]);
    }
  } else {
    if (rpm > RevsChangeLimit) {
      if (firstRevsChangeTime == 0) {
        firstRevsChangeTime = ms;
      }
      for (int led = 0; led < 8; led++) {
        if ((ms - firstRevsChangeTime) % 100 < 50) {
          leds[7 - led].setRGB(RevColors[7][1], RevColors[7][2], RevColors[7][3]);
          leds[8 + led].setRGB(RevColors[7][1], RevColors[7][2], RevColors[7][3]);
        } else {
          leds[7 - led].setRGB(0, 0, 0);
          leds[8 + led].setRGB(0, 0, 0);
        }
      }
    } else {
      firstRevsChangeTime = 0;
      for (int led = 0; led < 8; led++) {
        bool draw = rpm >= RevBoundaries[led];
        if (draw) {
          leds[led].setRGB(RevColors[led][1], RevColors[led][2], RevColors[led][3]);
          leds[REVS_LED_COUNT - led - 1].setRGB(RevColors[led][1], RevColors[led][2],
                                                RevColors[led][3]);
        } else {
          leds[led].setRGB(0, 0, 0);
          leds[REVS_LED_COUNT - led - 1].setRGB(0, 0, 0);
        }
      }
    }
  }
}

long statusChangeTime = 0;

void drawStatusLights(long ms) {
  FlagsState currentState = simHubReader.isBlueFlag() ? Blue : simHubReader.isYellowFlag() ? Yellow : Green;
  bool changed = currentState != flagState;
  if (changed) {
    flagState = currentState;
    statusChangeTime = ms;
  }
  const byte *color;
  switch (flagState) {
    case None:
      color = ColorNone;
      break;
    case Yellow:
      color = (ms - statusChangeTime) % 500 < 250 ? ColorYellow : ColorNone;
      break;
    case Blue:
      color = (ms - statusChangeTime) % 200 < 100 ? ColorBlue : ColorNone;
      break;
    case Green:
      color = ColorGreen;
      break;
  }
  for (int led = 0; led < 16; led++) {
    leds[led + REVS_LED_COUNT].setRGB(color[0], color[1], color[2]);
  }
}

void drawCell(int width, int height, const char* label, int atX, int atY, int color) {
  int x = (atX * CELL_WIDTH + CELL_PADDING - CELL_BORDER_THICKNESS / 2) - CELL_PADDING / 2;
  int y = (atY * CELL_HEIGHT + CELL_PADDING - CELL_BORDER_THICKNESS / 2) - CELL_PADDING / 2;
  int w = (width * CELL_WIDTH) - (CELL_PADDING * 2);
  w = x + w > SCREEN_WIDTH - 10 ? w + 4 : w;
  int h = (height * CELL_HEIGHT) - CELL_PADDING;
  int topLineWidth = max(0, (w - 10 * (int)strlen(label)) / 2);

  tft.setFont(&fonts::Font2);
  tft.setTextSize(1);
  tft.setTextDatum(textdatum_t::top_center);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(label, x + w / 2, y);

  tft.fillRect(x + CELL_BORDER_THICKNESS, y, topLineWidth, CELL_BORDER_THICKNESS, color);
  tft.fillRect(x + w - topLineWidth, y, topLineWidth, CELL_BORDER_THICKNESS, color);
  tft.fillRect(x, y, CELL_BORDER_THICKNESS, h, color);
  tft.fillRect(x + w, y, CELL_BORDER_THICKNESS, h, color);
  tft.fillRect(x + CELL_BORDER_THICKNESS, y + h - CELL_BORDER_THICKNESS, w - CELL_BORDER_THICKNESS, CELL_BORDER_THICKNESS, color);
}

void drawCellValue(float width, float height, const char* value, char* prevValue, float atX, float atY, int color, bool large, int yOffset) {
  if (strcmp(value, prevValue) != 0) {
    int x;
    int y;

    fillXY(width, height, atX, atY, yOffset, &x, &y);
    configureText(large);

    tft.setTextColor(TFT_BLACK);
    tft.drawString(prevValue, x, y);

    tft.setTextColor(color);
    tft.drawString(value, x, y);

    strcpy(prevValue, value);
  }
}

void drawCellValueInt(int width, int height, int value, int *oldValue, int atX, int atY, int color, bool large, int yOffset) {
  if (value != *oldValue) {
    int x;
    int y;

    fillXY(width, height, atX, atY, yOffset, &x, &y);
    configureText(large);

    tft.setTextColor(TFT_BLACK);
    tft.drawNumber(*oldValue, x, y);

    tft.setTextColor(color);
    tft.drawNumber(value, x, y);

    *oldValue = value;
  }
}

void drawCellValueChar(int width, int height, char value, char *oldValue, int atX, int atY, int color, bool large, int yOffset) {
  if (value != *oldValue) {
    int x;
    int y;

    fillXY(width, height, atX, atY, yOffset, &x, &y);
    configureText(large);

    tft.setTextColor(TFT_BLACK);
    sprintf(drawCharBuf, "%c", *oldValue);
    tft.drawString(drawCharBuf, x, y);

    tft.setTextColor(color);
    sprintf(drawCharBuf, "%c", value);
    tft.drawString(drawCharBuf, x, y);

    *oldValue = value;
  }
}

void drawCellValueFloat(int width, int height, float value, float *oldValue, int atX, int atY, int color, bool large, int yOffset, int decPlaces) {
  if (value != *oldValue) {
    int x;
    int y;

    fillXY(width, height, atX, atY, yOffset, &x, &y);
    configureText(large);

    tft.setTextColor(TFT_BLACK);
    tft.drawFloat(*oldValue, decPlaces, x, y + yOffset);

    tft.setTextColor(color);
    tft.drawFloat(value, decPlaces, x, y + yOffset);

    *oldValue = value;
  }
}

void fillXY(float width, float height, float atX, float atY, int yOffset, int *px, int *py) {
  int w = (width * CELL_WIDTH) - (CELL_PADDING * 2);
  int h = (height * CELL_HEIGHT);
  *px = ((atX * CELL_WIDTH + CELL_PADDING - CELL_BORDER_THICKNESS / 2) - CELL_PADDING / 2) + w / 2;
  *py = (atY * CELL_HEIGHT + CELL_PADDING * 4) + h / 2;
}

void configureText(bool large) {
  tft.setTextFont(large ? 6 : 4);
  tft.setTextSize(1);
  tft.setTextDatum(textdatum_t::middle_center);
}
