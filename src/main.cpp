#include <FS.h> //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp8266.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

//define your default values here, if there are different values in config.json, they are overwritten.
char blynk_token[34] = "3534391eaa0346bda6090415430ceb23";
const char *host = "esp8266-webupdate"; // Хост обновления
//flag for saving data
bool shouldSaveConfig = false;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  // WiFiManager
    //Очистить FS, для теста
    //SPIFFS.format();

    //Считать конфигурацию из FS json
    Serial.println("Монтирование FS...");

    if (SPIFFS.begin())
    {
      Serial.println("mounted file system");
      if (SPIFFS.exists("/config.json"))
      {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile)
        {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);
          DynamicJsonBuffer jsonBuffer;
          JsonObject &json = jsonBuffer.parseObject(buf.get());
          json.printTo(Serial);
          if (json.success())
          {
            Serial.println("\nparsed json");

            //strcpy(mqtt_server, json["mqtt_server"]);
            //strcpy(mqtt_port, json["mqtt_port"]);
            strcpy(blynk_token, json["blynk_token"]);
          }
          else
          {
            Serial.println("failed to load json config");
          }
        }
      }
    }
    else
    {
      Serial.println("failed to mount FS");
    }
    //end read

    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    //WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    //WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 33);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    //set static ip
    //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //add all your parameters here
    //wifiManager.addParameter(&custom_mqtt_server);
    //wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_blynk_token);

    //reset settings - for testing
    //wifiManager.resetSettings();

    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("Schetchik");
    if (!wifiManager.autoConnect("AutoConnectAP", "password"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    //read updated parameters
    //strcpy(mqtt_server, custom_mqtt_server.getValue());
    //strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(blynk_token, custom_blynk_token.getValue());

    //save the custom parameters to FS
    if (shouldSaveConfig)
    {
      Serial.println("saving config");
      DynamicJsonBuffer jsonBuffer;
      JsonObject &json = jsonBuffer.createObject();
      //json["mqtt_server"] = mqtt_server;
      //json["mqtt_port"] = mqtt_port;
      json["blynk_token"] = blynk_token;

      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile)
      {
        Serial.println("failed to open config file for writing");
      }

      json.printTo(Serial);
      json.printTo(configFile);
      configFile.close();
      //end save
    }

    Serial.println("local ip");
    Serial.println(WiFi.localIP());
  // WiFiManager конец
  Blynk.config(blynk_token);
  Blynk.begin(blynk_token, WiFi.SSID().c_str(), WiFi.psk().c_str());

  // Обновление
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your браузере\n", host);
}

void loop()
{
  Blynk.run();
  httpServer.handleClient();
  MDNS.update();
}