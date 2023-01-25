#include <iostream>
#include <algorithm>

#include "Vitocal.hpp"

void Vitocal::setup(HardwareSerial* serial) {
    _opto.setup(serial);
}

bool Vitocal::loop() {
    switch (_state) {

        case SYNC_REQUIRED:
            if (_opto.sync()) {
                _state = SYNCHED;
            }
            break;

        case SYNCHED:
            if (_queue.empty()) {
                _state = SYNC_REQUIRED;
            } else {
                _state = PROCESS_REQUIRED;
            }
            break;

        case PROCESS_REQUIRED:
            _opto.send(_queue.front().toSendBuffer());
            _queue.front().retry++;
            _state = RESPONSE_EXPECTED;
            
            _doLog("Read started");
            
            break;

        case RESPONSE_EXPECTED:
            {
                const auto reading = _opto.read(_queue.front().addr->length);

                if(reading.isError && _queue.front().retry < 5) {
                    _doLog("Read failed: " + _queue.front().addr->name);
                    _state = SYNC_REQUIRED;
                } else if(reading.isError) {
                    _doLog("Discarded " + _queue.front().addr->name);
                    if (_readErrorHandler) {
                        _readErrorHandler({ _queue.front().addr });
                    }
                    _queue.pop();
                    
                    _state = SYNC_REQUIRED;
                } else {
                    if (_readHandler) {
                        ReadEvent re = {_queue.front().addr, { reading.result }};
                        _readHandler(re);
                    }
                    _state = RESPONSE_COMPLETED;
                }
            }
            break;

        case RESPONSE_COMPLETED:
            _queue.pop();

            if (_queue.empty() && _processedHandler) {
                _processedHandler();
            }

            _state = SYNC_REQUIRED;
    }

    return _queue.empty();
}

void Vitocal::onRead(ReadEventHandler handler) {
    _readHandler = handler;
}

void Vitocal::onReadError(ReadErrorEventHandler handler) {
    _readErrorHandler = handler;
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

bool Vitocal::doRead(const Address* address) {
    _queue.push({ActionType::DoRead, address});
    return true;
}
