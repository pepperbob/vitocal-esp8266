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

struct DataPointValue {
  const char* name;
  const float value;
};

std::vector<DataPointValue> currentDatapoints;

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

void collectDatapoint(ReadEvent event) {
  currentDatapoints.push_back({event.address.name, event.value.toTemp()});
  blink.blinkShort(1);
}

void sendDatapoints() {
  assureWifiConnected();
  
  WiFiClient wifiClient;
  HTTPClient http;

  StaticJsonDocument<150> doc;

  doc["device"] = "vitocal";

  for (auto p : currentDatapoints) {
    doc[p.name] = p.value;
  }

  currentDatapoints.clear();

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

  // register handler functions
  vito.onRead(collectDatapoint);
  vito.onQueueProcessed(sendDatapoints);
  
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
  
  } else if (isIdle && !shouldUpdate) {
    // sleep to save power
    delay(interval - millisSince);    
  }
}