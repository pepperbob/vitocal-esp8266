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
                _doLog("sync ok");
                state = SYNCHED;
            }
            break;

        case SYNCHED:
            if (_queue.empty()) {
                state = SYNC_REQUIRED;
            } else {
                state = PROCESS_REQUIRED;
            }
            break;

        case PROCESS_REQUIRED:
            opto.send(_queue.front().toSendBuffer());
            _queue.front().retry++;
            state = RESPONSE_EXPECTED;
            
            _doLog("Read started");
            
            break;

        case RESPONSE_EXPECTED:
            {
                auto reading = opto.read(_queue.front().addr.length);

                if(reading.isError && _queue.front().retry < 5) {
                    _doLog("Read error");
                    state = SYNC_REQUIRED;
                } else if(reading.isError) {
                    _doLog("discarded queue head");
                    _queue.pop();
                    state = SYNC_REQUIRED;
                } else {
                    if (_readHandler) {
                        _readHandler({_queue.front().addr, {reading.result}});
                    }
                    state = RESPONSE_COMPLETED;
                }
            }
            break;

        case RESPONSE_COMPLETED:
            _queue.pop();

            if (_queue.empty() && _processedHandler) {
                _processedHandler();
            }

            state = SYNC_REQUIRED;
    }

    return _queue.empty();
}

void Vitocal::onRead(ReadEventHandler handler) {
    _readHandler = handler;
}

void Vitocal::onQueueProcessed(QueueProcessedHandler handler) {
    _processedHandler = handler;
}

void Vitocal::onLog(LogEventHandler handler) {
    _logHandler = handler;
}

void Vitocal::_doLog(std::string message) {
    if (_logHandler) {
        _logHandler({ message, millis() });
    }
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
    /// we are synched at this point
    /// this could be optimised, ACK is just required after HELLO 
    _serial->write(VITO_ACK);
    
    /// send payload
    _serial->write(sb.data(), sb.size());

    return true;
}

bool Optolink::sync() {
    int reading = _serial->read();
    return reading == VITO_HELLO;
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
