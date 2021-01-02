#ifndef __FT_TFT__
#define __FT_TFT__

#include <LovyanGFX.hpp>
#include "makerfabs_pin.h"

struct LGFX_Config
{
    static constexpr spi_host_device_t spi_host = ESP32_TSC_9488_LCD_SPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_sclk = ESP32_TSC_9488_LCD_SCK;
    static constexpr int spi_mosi = ESP32_TSC_9488_LCD_MOSI;
    static constexpr int spi_miso = ESP32_TSC_9488_LCD_MISO;
};

void setTft(lgfx::LGFX_SPI<LGFX_Config> *tft, lgfx::Panel_ILI9488 *panel);

#endif
