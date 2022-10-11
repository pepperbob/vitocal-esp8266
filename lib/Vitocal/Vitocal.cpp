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
                const auto reading = opto.read(_queue.front().addr->length);

                if(reading.isError && _queue.front().retry < 5) {
                    _doLog("Read error");
                    state = SYNC_REQUIRED;
                } else if(reading.isError) {
                    _doLog("discarded queue head");
                    _queue.pop();
                    state = SYNC_REQUIRED;
                } else {
                    if (_readHandler) {
                        ReadEvent re = {_queue.front().addr, { reading.result }};
                        _readHandler(re);
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

bool Vitocal::doRead(const Address* address) {
    _queue.push({ActionType::DoRead, address});
    return true;
}
