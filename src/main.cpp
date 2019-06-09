#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "Blynk_init.h"
#include "Display.h"
//extern void display_init();

void setup()
{
   Serial.begin(9600);
   Serial.println();
   display_init();
   blynk_init();   
}

void loop()
{
   Blynk.run();
}