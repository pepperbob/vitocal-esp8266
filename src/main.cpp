#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>

#include <Blink.hpp>
#include <Vitocal.hpp>
#include <secret.h>


WiFiClient wifiClient;
MQTTClient mqttClient(200);
Blinker blink(LED_BUILTIN);
Vitocal vito;

unsigned long interval = 60000;
unsigned long started = interval*-1;

struct DataPointValue {
  const Address *address;
  const AddressValue value;
};

std::vector<DataPointValue> currentDatapoints;

void initWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  
  WiFi.mode(WIFI_STA);
  delay(200);
  WiFi.begin(wifi_SSID, wifi_Pass);

  while (WiFi.status() != WL_CONNECTED) {
    // effectively delay
    blink.blink(1, 1000);
  }

  WiFi.setAutoConnect(true);
}

void assureMqttConnected() {
  if (mqttClient.connected()) {
    return;
  }

  while(!mqttClient.connect(mqtt_ClientId, mqtt_User, mqtt_Password)) {
    // we are not connected to mqtt
    blink.blink(2, 500);
  }

  mqttClient.publish(mqtt_TopicLWT, "Online", true, 1);
  mqttClient.subscribe(mqtt_TopicCMD);
}

void mqttLogHandler(LogEvent event) {
  assureMqttConnected();

  StaticJsonDocument<150> logJson;
  logJson["msg"] = event.message;
  logJson["time"] = event.millis;

  mqttClient.publish(mqtt_TopicLOG, logJson.as<String>());
}

void collectDatapoint(const ReadEvent event) {
  const DataPointValue dp = { event.address, event.value };
  currentDatapoints.push_back(dp);
}

void sendDatapoints() {
  assureMqttConnected();

  StaticJsonDocument<150> doc;
  doc["device"] = "vitocal";

  for (DataPointValue p : currentDatapoints) {
    p.address->output(doc, p.value);
    delete p.address;
    p.address = 0;
  }

  currentDatapoints.clear();

  if (mqttClient.publish(mqtt_TopicSensor, doc.as<String>())) {
    blink.blinkShort(2);
  } else {
    mqttLogHandler({"err: " + std::to_string(mqttClient.lastError()), millis()});
    blink.blink(5, 30);
  }

}

void mqtt_parse_message(String &topic, String &payload) {
  const auto address = processMessageToAddress(payload.c_str());
  
  if (address) {
    std::string msg_read = "Read: " + address->name + "@";
    msg_read += std::to_string(address->addr) + "(";
    msg_read += std::to_string(address->length) + ")";

    mqttLogHandler({ msg_read, millis() });
    
    vito.doRead(address);
  } else {
    mqttLogHandler( {"JSON not parsable", millis() });
  }
}

void setup() {

  initWifi();

  vito.setup(&Serial);

  // register handler functions
  vito.onRead(collectDatapoint);
  vito.onQueueProcessed(sendDatapoints);
  vito.onLog(mqttLogHandler);
  
  while(!Serial) continue;
  
  mqttClient.begin(mqtt_Host, wifiClient);
  mqttClient.setWill(mqtt_TopicLWT, "Offline", true, 1);
  mqttClient.onMessage(mqtt_parse_message);
  
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

    vito.doRead(new Temperature(ADDR_AU));
    vito.doRead(new Temperature(ADDR_WW));
    vito.doRead(new Temperature(ADDR_VL));
    vito.doRead(new Temperature(ADDR_RL));

    started = millis();
  }

}