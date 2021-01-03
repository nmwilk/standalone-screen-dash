#include "tft.h"
#include "simhubreader.h"
#include <FastLED.h>

#define SCREEN_WIDTH 480  // screen width
#define SCREEN_HEIGHT 320  // screen height

#define CELL_WIDTH (SCREEN_WIDTH / 6)
#define CELL_HEIGHT (SCREEN_HEIGHT / 4)

#define CELL_BORDER_THICKNESS 2
#define CELL_PADDING 4

#define REVS_LED_COUNT 16
#define STATUS_LED_COUNT 16

#define RevsChangeLimit 97

static lgfx::LGFX_SPI<LGFX_Config> tft;
static LGFX_Sprite sprite(&tft);
static lgfx::Panel_ILI9488 panel;

int oldGear = -1;
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

void setup() {
  Serial.begin(115200);
  simHubReader.begin();

  setTft(&tft, &panel);
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  drawCell(1, 1, "Delta", 0, 0, TFT_WHITE);
  drawCell(1, 1, "SPD", 1, 0, TFT_WHITE);
  drawCell(2, 2, "GEAR", 2, 0, TFT_WHITE);
  drawCell(1, 1, "LAP TIME", 4, 0, TFT_WHITE);
  drawCell(1, 1, "SPD", 5, 0, TFT_WHITE);

  drawCell(1, 1, "SPD", 0, 1, TFT_WHITE);
  drawCell(1, 1, "T-FL", 0, 2, TFT_WHITE);
  drawCell(1, 1, "T-RL", 0, 3, TFT_WHITE);
  drawCell(1, 1, "T-RF", 1, 2, TFT_WHITE);
  drawCell(1, 1, "T-RR", 1, 3, TFT_WHITE);
  drawCell(1, 1, "T-RL", 5, 2, TFT_WHITE);
  drawCell(1, 1, "T-RR", 5, 3, TFT_WHITE);

  FastLED.addLeds<NEOPIXEL, 18>(leds, REVS_LED_COUNT + STATUS_LED_COUNT);
}

void loop() {
  int ms = millis();

  simHubReader.tick(ms);

  int newGear = (ms % 10000) / 1000;
  drawNumber(newGear, &oldGear, &fonts::Font8, 1, SCREEN_WIDTH / 2 - 28, 20);

  int newSpeed = (ms % 10000) / 10;
  //  drawNumber(newSpeed, &oldSpeed, &fonts::Orbitron_Light_32, 1, 10, 10);

  //  drawNumber(newSpeed, &oldTime, &fonts::Orbitron_Light_32, 1, 300, 10);


  drawRevBar(ms, ms % 1000 / 10);
  drawStatusLights(ms);

  FastLED.show();
}

void drawNumber(int newValue, int *oldValue, const lgfx::RLEfont* font, float fontScale, int posX, int posY) {
  if (*oldValue != newValue) {
    tft.setFont(font);
    tft.setTextSize(fontScale);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(posX, posY);
    tft.print(newValue);

    *oldValue = newValue;
  }
}

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

void drawNumber(int newValue, int *oldValue, const lgfx::GFXfont* font, float fontScale, int posX, int posY) {
  if (newValue != *oldValue ) {
    tft.setFont(font);
    tft.setTextSize(fontScale);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(posX, posY);
    tft.print(newValue);

    *oldValue = newValue;
  }

}

void drawCell(int width, int height, const char* label, int atX, int atY, int color) {
  int x = atX * CELL_WIDTH + CELL_PADDING - CELL_BORDER_THICKNESS / 2;
  int y = atY * CELL_HEIGHT + CELL_PADDING - CELL_BORDER_THICKNESS / 2;
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
