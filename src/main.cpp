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

// interval to read addresses
unsigned long interval = 60000;
unsigned long started = interval*-1;

struct DataPointValue {
  const Address *address;
  const AddressValue value;
};

std::vector<DataPointValue> currentDatapoints;

void sendLogOverMqtt(LogEvent event) {
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

void assureMqttConnected() {

  if (mqttClient.connected()) {
    return;
  }

  if (mqttClient.connect(mqtt_ClientId, mqtt_User, mqtt_Password)) {
      mqttClient.publish(mqtt_TopicLWT, "Online", true, 1);
      mqttClient.subscribe(mqtt_TopicCMD);
      return;
  }
  
  led.blink(1, 1000);
  return;
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_SSID, wifi_Pass);

  while (WiFi.status() != WL_CONNECTED) {
    // effectively delay
    led.blink(2, 500);
  }

  while(!Serial) continue;

  // setup vito, register callbacks
  vito.setup(&Serial);
  vito.onRead(collectDatapoint);
  vito.onReadError(freeAddressAfterFailure);
  vito.onQueueProcessed(sendDatapoints);
  vito.onLog(sendLogOverMqtt);
  
  mqttClient.setTimeout(2000);
  mqttClient.begin(mqtt_Host, wifiClient);
  mqttClient.setWill(mqtt_TopicLWT, "Offline", true, 1);
  mqttClient.onMessage(mqtt_parse_message);
  
}

void loop() {

  // millis since last read
  auto millisSince = millis() - started;

  // maintain mqtt-connection
  if (!mqttClient.loop()) {

    // we're not connceted (anymore)
    assureMqttConnected();
    
    // if no update after a while...
    auto long_time_no_update = started > 0 && millisSince > 300*1000;

    // ... restart the ESP
    if (long_time_no_update) ESP.restart();
    
    return;
  }
  
  // compare with interval...
  auto shouldUpdate = millisSince > interval;

  // loop in any case and update if needed
  if (vito.loop() && shouldUpdate) {
    
    led.blink(1);

    vito.doRead(new Temperature(ADDR_AU));
    vito.doRead(new Temperature(ADDR_WW));
    vito.doRead(new Temperature(ADDR_VL));
    vito.doRead(new Temperature(ADDR_RL));

    started = millis();
  }

}