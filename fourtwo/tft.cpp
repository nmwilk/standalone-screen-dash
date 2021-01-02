#include "tft.h"

void setTft(lgfx::LGFX_SPI<LGFX_Config> *tft, lgfx::Panel_ILI9488 *panel)
{
  panel->freq_write = 60000000;
  panel->freq_fill = 60000000;
  panel->freq_read = 16000000;
  panel->spi_mode = 0;
  panel->spi_mode_read = 0;
  panel->len_dummy_read_pixel = 8;
  panel->spi_read = true;
  panel->spi_3wire = false;
  panel->spi_cs = ESP32_TSC_9488_LCD_CS;
  panel->spi_dc = ESP32_TSC_9488_LCD_DC;
  panel->gpio_rst = ESP32_TSC_9488_LCD_RST;
  panel->gpio_bl = ESP32_TSC_9488_LCD_BL;
  panel->pwm_ch_bl = -1;
  panel->backlight_level = true;
  panel->invert = false;
  panel->rgb_order = true;
  panel->memory_width = ESP32_TSC_9488_LCD_WIDTH;
  panel->memory_height = ESP32_TSC_9488_LCD_HEIGHT;
  panel->panel_width = ESP32_TSC_9488_LCD_WIDTH;
  panel->panel_height = ESP32_TSC_9488_LCD_HEIGHT;
  panel->offset_x = 0;
  panel->offset_y = 0;
  panel->rotation = 1;
  panel->offset_rotation = 0;
  tft->setPanel(panel);
}
