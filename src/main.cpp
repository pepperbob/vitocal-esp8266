#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Blink.hpp>
#include <ArduinoJson.h>
#include <Vitocal.hpp>
#include <secret.h>
#include <MQTT.h>

WiFiClient wifiClient;
MQTTClient mqttClient(200);
Blinker blink(LED_BUILTIN);
Vitocal vito;

unsigned long interval = 60000;
unsigned long started = interval*-1;

struct DataPointValue {
  const char* name;
  const float value;
};

std::vector<DataPointValue> currentDatapoints;

void assureWifiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  while (WiFi.status() != WL_CONNECTED) {
    // effectively delay
    blink.blink(1, 500);
  }

  WiFi.mode(WIFI_STA);
  wifi_set_sleep_type(LIGHT_SLEEP_T);
}

void assureMqttConnected() {
  if (mqttClient.connected()) {
    return;
  }

  mqttClient.setWill(mqtt_TopicLWT, "Offline", true, 1);

  do {
    // we are not connected to mqtt
    // make sure wifi is ok, wait a bit, check status
    assureWifiConnected();
    blink.blink(1, 500);
  } while(!mqttClient.connect(mqtt_ClientId, mqtt_User, mqtt_Password));

  mqttClient.publish(mqtt_TopicLWT, "Online", true, 1);

}

void mqttLogHandler(LogEvent event) {
  assureMqttConnected();

  StaticJsonDocument<150> logJson;
  logJson["msg"] = event.message;
  logJson["time"] = event.millis;

  mqttClient.publish(mqtt_TopicLOG, logJson.as<String>());
}

void collectDatapoint(ReadEvent event) {
  currentDatapoints.push_back({event.address.name, event.value.toTemp()});
}

void sendDatapoints() {
  assureMqttConnected();

  StaticJsonDocument<150> doc;

  doc["device"] = "vitocal";

  for (auto p : currentDatapoints) {
    doc[p.name] = p.value;
  }

  currentDatapoints.clear();

  if (mqttClient.publish(mqtt_TopicSensor, doc.as<String>())) {
    blink.blinkShort(2);
  } else {
    mqttLogHandler({"err: " + std::to_string(mqttClient.lastError()), millis()});
    blink.blink(5, 30);
  }

}

void setup() {

  vito.setup(&Serial);

  // register handler functions
  vito.onRead(collectDatapoint);
  vito.onQueueProcessed(sendDatapoints);
  vito.onLog(mqttLogHandler);
  
  while(!Serial) continue;
  
  WiFi.begin(wifi_SSID, wifi_Pass);
  assureWifiConnected();

  mqttClient.begin(mqtt_Host, wifiClient);
  assureMqttConnected();

  mqttLogHandler({"setup complete", millis()});
}

void loop() {
  if (!mqttClient.loop()) {
    assureMqttConnected();
  }
  
  // handle vitocal
  auto millisSince = millis() - started;
  auto shouldUpdate = millisSince > interval;

  if (vito.loop() && shouldUpdate) {
    
    blink.blink(1);
    
    vito.doRead(ADDR_AU);
    vito.doRead(ADDR_WW);
    vito.doRead(ADDR_VL);
    vito.doRead(ADDR_RL);

    started = millis();
  }

}