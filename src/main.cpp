#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Blink.hpp>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Vitocal.hpp>
#include <secret.h>

Blinker blink(LED_BUILTIN);
Vitocal vito;

unsigned long started = 0;
unsigned long interval = 60000;

void assureWifiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.begin(wifi_SSID, wifi_Pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  WiFi.mode(WIFI_STA);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
}

void sendDatapoint(ReadEvent event) {
  assureWifiConnected();
  
  WiFiClient wifiClient;
  HTTPClient http;

  StaticJsonDocument<100> doc;

  doc["device"] = "vitocal";
  doc[event.address.name] = event.value.toTemp();

  http.begin(wifiClient, metricHost);
  int stat = http.POST(doc.as<String>());
  http.end();

  if (stat >= 200 && stat < 300) {
    blink.blinkShort(2);
  } else {
    blink.blink(5, 30);
  }
}

void setup() {

  vito.setup(&Serial);
  vito.onRead(sendDatapoint);
  
  while(!Serial) continue;
  
  assureWifiConnected();
}

void loop() {  
  auto isIdle = vito.loop();
  auto millisSince = millis() - started;
  auto shouldUpdate = millisSince > interval;

  if (isIdle && shouldUpdate) {
    
    blink.blink(1);
    
    vito.doRead(ADDR_AU);
    vito.doRead(ADDR_WW);
    vito.doRead(ADDR_VL);
    vito.doRead(ADDR_RL);

    started = millis();
  
  } else if (isIdle) {
    // sleep to save power
    delay(30000);
  }
}