#include "tft.h"
#include <FastLED.h>

#define SCREEN_WIDTH 480  // screen width
#define SCREEN_HEIGHT 320  // screen height

#define REVS_LED_COUNT 16
#define STATUS_LED_COUNT 16

#define RevsChangeLimit 97

static lgfx::LGFX_SPI<LGFX_Config> tft;
static LGFX_Sprite sprite(&tft);
static lgfx::Panel_ILI9488 panel;

int oldGear = -1;
int oldSpeed = -1;
int oldTime = -1;

const unsigned int BG_COLOR = tft.color565(0, 0, 0);
const unsigned int GEAR_COLOR = tft.color565(255, 255, 255);
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

void setup() {
  Serial.begin(115200);

  setTft(&tft, &panel);
  tft.begin();
  tft.fillScreen(BG_COLOR);

  FastLED.addLeds<NEOPIXEL, 18>(leds, REVS_LED_COUNT + STATUS_LED_COUNT);
}

void loop() {
  int ms = millis();
  int newGear = (ms % 10000) / 1000;
  drawNumber(newGear, &oldGear, &fonts::Font8, 1, SCREEN_WIDTH / 2 - 28, 20);

  int newSpeed = (ms % 10000) / 10;
  drawNumber(newSpeed, &oldSpeed, &fonts::Orbitron_Light_32, 1, 10, 10);

  drawNumber(newSpeed, &oldTime, &fonts::Orbitron_Light_32, 1, 300, 10);


  drawRevBar(ms, ms % 1000 / 10);
  drawStatusLights(ms);

  FastLED.show();

  delay(30);
}

void drawNumber(int newValue, int *oldValue, const lgfx::RLEfont* font, float fontScale, int posX, int posY) {
  if (*oldValue != newValue) {
    tft.setFont(font);
    tft.setTextSize(fontScale);

        tft.setTextColor(BG_COLOR);
    tft.setCursor(posX, posY);
    tft.print(*oldValue);
    
    tft.setTextColor(TFT_WHITE);
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

    tft.setTextColor(BG_COLOR);
    tft.setCursor(posX, posY);
    tft.print(*oldValue);

    tft.setTextColor(TFT_WHITE);
    tft.setCursor(posX, posY);
    tft.print(newValue);

    *oldValue = newValue;
  }

}
