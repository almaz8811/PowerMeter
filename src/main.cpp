#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Fonts/FreeMono9pt7b.h"
#include "Fonts/FreeMonoBold12pt7b.h"
#include "Fonts/FreeMono18pt7b.h"
#include "Blynk_init.h"
#include "WC_NTP.h"

#define PIN_DC 5
#define PIN_RESET 4
#define PIN_CS 15
#define M_PAGE_SIZE 150

Adafruit_ILI9341 display = Adafruit_ILI9341(PIN_CS, PIN_DC, PIN_RESET);
//extern void display_init();

int m_page0[M_PAGE_SIZE];
int m_page_count0 = 0;
char s[100];
float u1 = 0.0, i1 = 0.0, p1 = 0.0, e1 = 0.0;
float p_max = 0, p_min = 99999999;
long t_correct = 0;
uint32_t t_cur = 0;
uint32_t tm = 0;
uint32_t ms, ms1 = 0,ms2 = 0, ms3 = 0, ms4 = 0, ms_ok = 0;

void display_init()
{
   // Инициализируем дисплей
   display.begin();
   display.setRotation(1);
   display.fillScreen(ILI9341_BLACK);
   display.startWrite();
   display.setTextColor(ILI9341_YELLOW);
   display.setFont(&FreeMonoBold12pt7b);
   display.setCursor(10, 16);
   display.print("PowerMeter v1.0");
   display.endWrite();
}

void displayEE()
{
   display.startWrite();
   display.fillRect(0, 50, 320, 40, ILI9341_BLACK);
   display.drawRect(0, 50, 159, 20, ILI9341_WHITE);
   display.drawRect(160, 50, 159, 20, ILI9341_WHITE);
   display.drawRect(0, 70, 159, 20, ILI9341_WHITE);
   display.drawRect(160, 70, 159, 20, ILI9341_WHITE);
   display.setFont(&FreeMonoBold12pt7b);
   display.setTextColor(ILI9341_GREEN);

   display.setFont(&FreeMonoBold12pt7b);
   display.setTextColor(ILI9341_GREEN);

   sprintf(s, "%d.%02d V", (int)u1, ((int)(u1 * 100)) % 100);
   display.setCursor(2, 68);
   display.print(s);

   sprintf(s, "%d.%02d A", (int)i1, ((int)(i1 * 100)) % 100);
   display.setCursor(162, 68);
   display.print(s);

   sprintf(s, "%d.%02d W", (int)p1, ((int)(p1 * 100)) % 100);
   display.setCursor(2, 88);
   display.print(s);

   sprintf(s, "%d.%02d Wh", (int)e1, ((int)(e1 * 100)) % 100);
   display.setCursor(162, 88);
   display.print(s);

   // Отображение времени
   if (t_correct)
   {
      sprintf(s, "%02d:%02d", (int)(tm / 3600) % 24, (int)(tm / 60) % 60);
      display.fillRect(240, 0, 80, 20, ILI9341_BLACK);
      display.setCursor(240, 16);
      display.print(s);
   }
   display.endWrite();
}

void displayGRAPH()
{
   // Стираем область графика
   display.startWrite();
   display.fillRect(0, 120, 320, 120, ILI9341_WHITE);

   // Рисуем сетку
   for (int i = 0; i < 15; i++)
   {
      int x = 18 + i * 20;
      display.drawLine(x, 138, x, 238, ILI9341_BLACK);
   }
   for (int i = 0; i < 5; i++)
   {
      int y = 138 + i * 20;
      display.drawLine(18, y, 318, y, ILI9341_BLACK);
   }
   // Формируем график
   int n = 0;
   sprintf(s, "Pmax=%d.%02d Pmin=%d.%02d", (int)p_max, ((int)(p_max * 100)) % 100, (int)p_min, ((int)(p_min * 100)) % 100);
   // Пишем минимум максимум
   display.setFont(&FreeMono9pt7b); // устанавливаем маленький шрифт
   display.setTextColor(ILI9341_RED);
   display.setCursor(30, 135);
   display.print(s);

   for (int i = 0; i < m_page_count0; i++)
   {
      float yf = 1;
      if (p_max > 0)
         yf = m_page0[i] * 100 / (p_max);
      int y = 238 - (int)yf;
      int x = 19 + n * 2;
      n++;
      display.drawLine(x, y, x, 237, ILI9341_RED);
   }
   display.endWrite();
}

void setup()
{
   Serial.begin(9600);
   Serial.println();
   display_init();
   displayEE();
   displayGRAPH();
   NTP_begin();
   blynk_init();
}

void loop()
{
   ms = millis();
   // Опрос NTP сервера
   if(( ms3 == 0 || ms < ms3 || (ms - ms3)>NTP_TIMEOUT )){
       uint32_t t = GetNTP();
       if( t!=0 ){
          ms3 = ms;
          t_correct = t - t_cur;
       }
   }
   Blynk.run();
}
