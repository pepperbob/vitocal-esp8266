#include <Callback.hpp>
#include <ArduinoJson.h>

const CO_4* processMessageToAddress(const char* message) {
    StaticJsonDocument<150> doc;
    const auto err = deserializeJson(doc, message);
    if (err) {
        return nullptr;
    }
    auto name = doc["name"].as<std::string>();
    auto len = doc["len"].as<uint8_t>();
    len = len < 1 || len > 4 ? 4 : len;
    
    return new CO_4(name, doc["addr"].as<uint16_t>(), len);
}