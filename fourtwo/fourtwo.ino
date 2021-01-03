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

char oldGear = '@';
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


CRGB leds[REVS_LED_COUNT + STATUS_LED_COUNT];
SimHubReader simHubReader;

char drawCharBuf[2];

void setup() {
  Serial.begin(115200);
  simHubReader.begin();

  setTft(&tft, &panel);
  tft.begin();
  tft.fillScreen(TFT_BLACK);

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

  drawCellValue(3, 1, simHubReader.getLapTime(), 6, 0, TFT_WHITE, false, -3);
  drawCellValue(3, 1, simHubReader.getLastLapTime(), 6, 1, TFT_WHITE, false, -3);
  drawCellValue(3, 1, simHubReader.getBestLapTime(), 6, 2, TFT_WHITE, false, -3);

  drawCellValueInt(1, 1, simHubReader.getTcLevel(), 3, 2, TFT_WHITE, true, 0);
  drawCellValueInt(1, 1, simHubReader.getTcLevel(), 3, 3, TFT_WHITE, true, 0);
  drawCellValueInt(1, 1, simHubReader.getMapLevel(), 6, 3, TFT_WHITE, true, 0);

  drawCellValueFloat(2, 1, simHubReader.getBrakeBias(), 7, 3, TFT_WHITE, false, 0, 2);

  drawTyres();

  drawCellValueInt(1, 1, simHubReader.getPosition(), 0, 1, TFT_WHITE, true, 0);
  drawCellValueInt(1, 1, simHubReader.getLapNumber(), 1, 1, TFT_WHITE, true, 0);

  drawRevBar(ms, ms % 1000 / 10);
  drawStatusLights(ms);

  FastLED.show();
}

void drawTyres() {
  
}

void drawGaps() {
  drawCellValueFloat(2, 1, simHubReader.getGapAhead(), 0, 0, TFT_WHITE, false, -14, 2);
  drawCellValueFloat(2, 1, simHubReader.getGapBehind(), 0, 0, TFT_WHITE, false, 12, 2);
}

void drawGear() {
  char newGear = simHubReader.getGear();
  int x;
  int y;
  fillXY(2, 2, 4, 1, -6, &x, &y);
  drawChar(newGear, &oldGear, 8, 1, x, y);
}

void drawSpeed() {
  drawCellValue(2, 1, simHubReader.getSpeed(), 2, 1, TFT_WHITE, true, 0);
}

void drawLapDelta() {
  float lapDelta = simHubReader.getLapDelta();
  drawCellValueFloat(2, 1, lapDelta, 4, 0, lapDelta < 0 ? TFT_GREEN : TFT_RED, false, -3, 2);
}

void drawChar(char newValue, char *oldValue, int font, float fontScale, int posX, int posY) {
  if (*oldValue != newValue) {
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setTextFont(font);
    tft.setTextSize(fontScale);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    sprintf(drawCharBuf, "%c", newValue);

    tft.drawString(drawCharBuf, posX, posY);

    *oldValue = newValue;
  }
}
//
//void drawNumber(int newValue, int *oldValue, const lgfx::BaseFont* font, float fontScale, int posX, int posY) {
//  if (*oldValue != newValue) {
//    tft.setTextDatum(textdatum_t::middle_center);
////    tft.setFont(font);
//tft.setTextFont(9);
//    tft.setTextSize(fontScale);
//
//    tft.setTextColor(TFT_WHITE, TFT_BLACK);
//    tft.drawNumber(newValue, posX, posY);
//
//    *oldValue = newValue;
//  }
//}

long firstRevsChangeTime = 0;

void drawRevBar(long ms, int rpm) {
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

void drawStatusLights(long ms) {
  const byte *color = ms % 1000 > 800 ? ColorNone : (ms % 10000 > 5000 ? ColorGreen : (ms % 10000 > 2000 ? ColorYellow : ColorRed));
  for (int led = 0; led < 16; led++) {
    leds[led + REVS_LED_COUNT].setRGB(color[0], color[1], color[2]);
  }
}

void drawCell(int width, int height, const char* label, int atX, int atY, int color) {
  int x = (atX * CELL_WIDTH + CELL_PADDING - CELL_BORDER_THICKNESS / 2) - CELL_PADDING / 2;
  int y = (atY * CELL_HEIGHT + CELL_PADDING - CELL_BORDER_THICKNESS / 2) - CELL_PADDING / 2;
  int w = (width * CELL_WIDTH) - (CELL_PADDING * 2);
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

void drawCellValue(int width, int height, const char* value, int atX, int atY, int color, bool large, int yOffset) {
  int x;
  int y;

  fillXY(width, height, atX, atY, yOffset, &x, &y);
  configureText(large, color);

  tft.drawString(value, x, y);
}

void drawCellValueInt(int width, int height, int value, int atX, int atY, int color, bool large, int yOffset) {
  if (value != NO_INT) {
    int x;
    int y;

    fillXY(width, height, atX, atY, yOffset, &x, &y);

    configureText(large, color);
    tft.drawNumber(value, x, y);
  }
}

void drawCellValueFloat(int width, int height, float value, int atX, int atY, int color, bool large, int yOffset, int decPlaces) {
  if (value != NO_FLOAT) {
    int x;
    int y;

    fillXY(width, height, atX, atY, yOffset, &x, &y);
    configureText(large, color);

    tft.drawFloat(value, decPlaces, x, y + yOffset);
  }
}

void fillXY(int width, int height, int atX, int atY, int yOffset, int *px, int *py) {
  int w = (width * CELL_WIDTH) - (CELL_PADDING * 2);
  int h = (height * CELL_HEIGHT);
  *px = ((atX * CELL_WIDTH + CELL_PADDING - CELL_BORDER_THICKNESS / 2) - CELL_PADDING / 2) + w / 2;
  *py = (atY * CELL_HEIGHT + CELL_PADDING * 4) + h / 2;
}

void configureText(bool large, int color) {
  tft.setTextFont(large ? 6 : 4);
  tft.setTextSize(1);
  tft.setTextDatum(textdatum_t::middle_center);
  tft.setTextColor(color, TFT_BLACK);
}
