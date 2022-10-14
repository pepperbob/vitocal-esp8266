#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>

#include <Blink.hpp>
#include <Vitocal.hpp>
#include <secret.h>


WiFiClient wifiClient;
MQTTClient mqttClient(200);
Blinker led(LED_BUILTIN);
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
    led.blink(1, 1000);
  }

  WiFi.setAutoConnect(true);
}

void assureMqttConnected() {
  if (mqttClient.connected()) {
    return;
  }

  while(!mqttClient.connect(mqtt_ClientId, mqtt_User, mqtt_Password)) {
    // we are not connected to mqtt
    led.blink(2, 500);
  }

  mqttClient.publish(mqtt_TopicLWT, "Online", true, 1);
  mqttClient.subscribe(mqtt_TopicCMD);
}

void sendLogOverMqtt(LogEvent event) {
  assureMqttConnected();

  StaticJsonDocument<150> logJson;
  logJson["msg"] = event.message;
  logJson["time"] = event.millis;

  mqttClient.publish(mqtt_TopicLOG, logJson.as<String>());
}

// convenience
void sendLog(std::string msg) {
  sendLogOverMqtt({msg, millis()});
}

void collectDatapoint(const ReadEvent event) {
  currentDatapoints.push_back({ event.address, event.value });
}

void sendDatapoints() {
  assureMqttConnected();

  StaticJsonDocument<200> doc;
  doc["device"] = "vitocal";

  for (DataPointValue p : currentDatapoints) {
    p.address->output(doc, p.value);
    
    // free memory
    delete p.address;
    p.address = 0;
  }

  currentDatapoints.clear();

  if (mqttClient.publish(mqtt_TopicSensor, doc.as<String>())) {
    led.blinkShort(2);
  } else {
    sendLog("err: " + std::to_string(mqttClient.lastError()));
    led.blink(5, 30);
  }

}

void freeAddressAfterFailure(ReadErrorEvent event) {
  delete event.address;
  event.address = 0;
}

void mqtt_parse_message(String &topic, String &payload) {
  const auto address = processMessageToAddress(payload.c_str());
  
  if (address) {
    std::string msg_read = "Read: " + address->name + "@";
    msg_read += std::to_string(address->addr) + "(";
    msg_read += std::to_string(address->length) + ")";

    sendLog(msg_read);
    
    vito.doRead(address);
  } else {
    sendLog("JSON not parsable");
  }
}

void setup() {

  initWifi();

  vito.setup(&Serial);

  // register callbacks
  vito.onRead(collectDatapoint);
  vito.onReadError(freeAddressAfterFailure);
  vito.onQueueProcessed(sendDatapoints);
  vito.onLog(sendLogOverMqtt);
  
  while(!Serial) continue;
  
  mqttClient.begin(mqtt_Host, wifiClient);
  mqttClient.setWill(mqtt_TopicLWT, "Offline", true, 1);
  mqttClient.onMessage(mqtt_parse_message);
  
  assureMqttConnected();

  sendLog("Setup complete.");
}

void loop() {
  if (!mqttClient.loop()) {
    assureMqttConnected();
  }
  
  // handle vitocal
  auto millisSince = millis() - started;
  auto shouldUpdate = millisSince > interval;

  if (vito.loop() && shouldUpdate) {
    
    led.blink(1);

    vito.doRead(new Temperature(ADDR_AU));
    vito.doRead(new Temperature(ADDR_WW));
    vito.doRead(new Temperature(ADDR_VL));
    vito.doRead(new Temperature(ADDR_RL));

    started = millis();
  }

}