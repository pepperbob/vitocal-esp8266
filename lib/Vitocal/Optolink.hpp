#pragma once

#include <Arduino.h>

#define VITO_ACK 0x01
#define VITO_HELLO 0x05

struct ReadResult {
    const std::vector<uint8_t> result;
    const bool isError;
};
class Optolink {
    public:
    /// Setup Serial Port
    void setup(HardwareSerial* serial);
    
    /// try to sync, returns true if Vitocal could be detected.
    bool sync();
    
    /// sends data to serial output; returns true if data could be sent.
    bool send(const std::vector<uint8_t> sendBuffer);

    /// Read length of data from serial input.
    ReadResult read(uint8_t length);

    private:
    Stream* _serial;
};