#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Fonts/FreeMono9pt7b.h"
#include "Fonts/FreeMonoBold12pt7b.h"
#include "Fonts/FreeMono18pt7b.h"

#define PIN_DC      5
#define PIN_RESET   4
#define PIN_CS      15

Adafruit_ILI9341 display = Adafruit_ILI9341(PIN_CS, PIN_DC, PIN_RESET);
void display_init() {
    // Инициализируем дисплей
   display.begin();
   display.setRotation(1);
   display.fillScreen(ILI9341_BLACK);
   display.startWrite();
   display.setTextColor(ILI9341_YELLOW);
   display.setFont(&FreeMonoBold12pt7b);
   display.setCursor(10,16);
   display.print("PowerMeter v1.0");
   display.endWrite();
}