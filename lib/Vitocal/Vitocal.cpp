#include <iostream>
#include <algorithm>

#include "Vitocal.hpp"

void Vitocal::setup(HardwareSerial* serial) {
    opto.setup(serial);
}

bool Vitocal::loop() {
    switch (state) {

        case SYNC_REQUIRED:
            if (opto.sync()) {
                state = IDLE;
            }
            break;

        case IDLE:
            if (!_queue.empty()) {
                state = PROCESS_QUEUE;
            }
            break;

        case PROCESS_QUEUE:
            if (opto.send(_queue.front().toSendBuffer())) {
                state = EXPECT_RESPONSE;
            } else {
                _queue.front().retry++;
                
                /// some arbitrary retry... ?! 
                if (_queue.front().retry > 5) {
                    _queue.pop();
                    state = SYNC_REQUIRED;
                }
            };
            
            break;

        case EXPECT_RESPONSE:
            auto reading = opto.read(_queue.front().addr.length);

            if (reading.isError) {
                state = PROCESS_QUEUE;
            } else if (!reading.result.empty()) {
                if(_handler) {
                    _handler({_queue.front().addr, {reading.result}});
                }

                _queue.pop();
                state = IDLE;
            }
            
            break;
    }

    return _queue.empty();
}

void Vitocal::onRead(ReadEventHandler handler) {
    _handler = handler;
}

bool Vitocal::doRead(Address address) {
    _queue.push({DoRead, address});
    return true;
}

void Optolink::setup(HardwareSerial* serial) {
    _serial = serial;
    serial->begin(4800, SERIAL_8E2);
}

bool Optolink::send(std::vector<uint8_t> sb) {
    if(!sync()) {
        return false;
    }

    /// this could be optimised, ACK is just required after HELLO 
    _serial->write(VITO_ACK);
    
    /// send payload
    _serial->write(sb.data(), sb.size());

    return true;
}

bool Optolink::sync() {
    int lastReading = -1;
    auto time = millis();
    while(_serial->available() || (lastReading == -1 && (millis() - time) < 500)) {
        lastReading = _serial->read();
    }
    return lastReading == VITO_HELLO;
}

ReadResult<std::vector<uint8_t>> Optolink::read(uint8_t length) {
    std::vector<uint8_t> buff;
    buff.reserve(length);
    auto time = millis();

    // expected bytes in return within at max 100ms
    while (buff.size() < length && (millis() - time < 100)) {
        if (_serial->available()) {
            buff.push_back(_serial->read());
        }
    }

    if (buff.size() != length) {
        return {buff, true};
    }

    return { buff, false };
}
