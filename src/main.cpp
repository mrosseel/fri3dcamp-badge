#include <Arduino.h>
#include <Adafruit_GFX.h> // Core graphics library
#include <SPI.h>
#include "Adafruit_ST7789_Fri3d2024.h" // Hardware-specific library for GC9307 (ST7789) driver of Fri3d badge 2024

// Explicitly use hardware SPI port, default is software which is too slow
SPIClass *spi = new SPIClass(HSPI);
Adafruit_GFX_Fri3dBadge2024_TFT tft(spi, TFT_CS, TFT_DC, TFT_RST);

void setup(void)
{
    Serial.begin(115200);
    delay(3000);
    Serial.println("Hello world - Adafruit GFX GC9307/ST7789 Fri3d badge - Centered Text with Adjusted Font Size");

    spi->begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.setSPISpeed(SPI_FREQUENCY);

    tft.init(TFT_WIDTH, TFT_HEIGHT);
    tft.setRotation(3);

    tft.fillScreen(TFT_BLACK);
    tft.drawRect(0, 0, tft.width(), tft.height(), TFT_BLUE);

    // Text to be displayed
    const char *text = "Mikee";

    // Set text color
    tft.setTextColor(TFT_WHITE);
    tft.setTextWrap(false);

    // Find the largest font size that fits the screen width
    int16_t x1, y1;
    uint16_t w, h;
    uint8_t font_size = 1;
    int16_t max_width = tft.width() - 20; // Leave a 10-pixel margin on each side

    do
    {
        tft.setTextSize(font_size);
        tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        font_size++;
    } while (w < max_width && h < tft.height() - 20); // Also check height to avoid overflow
    // Step back one size to ensure it fits
    font_size -= 2;
    tft.setTextSize(font_size);

    // Recalculate the final text bounds
    tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    // Calculate position to center the text
    int16_t x = (tft.width() - w) / 2;
    int16_t y = (tft.height() - h) / 2;

    // Set cursor to the calculated position and print the text
    tft.setCursor(x, y);
    tft.print(text);

    Serial.printf("Font size: %d, Text width: %d, Text height: %d\n", font_size, w, h);

    delay(2000);
}

void loop()
{
    // Your loop code here
}
