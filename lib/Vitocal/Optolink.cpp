#include <Optolink.hpp>

void Optolink::setup(HardwareSerial* serial) {
    _serial = serial;
    serial->begin(4800, SERIAL_8E2);
}

bool Optolink::send(const std::vector<uint8_t> sb) {
    /// we are synched at this point
    /// this could be optimised, ACK is just required after HELLO 
    _serial->write(VITO_ACK);
    
    /// send payload
    _serial->write(sb.data(), sb.size());

    return true;
}

bool Optolink::sync() {
    int reading = _serial->read();
    return !_serial->available() && reading == VITO_HELLO;
}

ReadResult Optolink::read(const uint8_t length) {
    std::vector<uint8_t> buff;
    if(length > 0) {
        buff.reserve(length);
    }
    
    auto time = millis();

    // expected bytes in return within at max 100ms
    while ((length == 0 || buff.size() < length) && (millis() - time < 100)) {
        if (_serial->available()) {
            buff.push_back(_serial->read());
        }
    }

    boolean isErr = length !=0 && buff.size() != length;

    return { buff, isErr };
}
