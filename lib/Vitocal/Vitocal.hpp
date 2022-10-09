#pragma once

#include <Arduino.h>
#include <functional>
#include <queue>

#include <Address.hpp>
#include <Callback.hpp>

#define VITO_ACK 0x01
#define VITO_HELLO 0x05

template <typename T>
struct ReadResult {
    const T result;
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
    ReadResult<std::vector<uint8_t>> read(uint8_t length);

    private:
    Stream* _serial;
};

class Vitocal {
    public:
    void setup(HardwareSerial* serial);
    
    /// event loop for state-machine; returns true if idle.
    bool loop();

    /// Queue read operation for address
    bool doRead(Address address);

    /// Register Event Handler that is called upon successful read
    void onRead(ReadEventHandler handler);
    void onQueueProcessed(QueueProcessedHandler handler);
    void onLog(LogEventHandler handler);

    private:

    void _doLog(std::string message);

    /// states of the state-machine
    enum VitoState { SYNC_REQUIRED, SYNCHED, PROCESS_REQUIRED, RESPONSE_EXPECTED, RESPONSE_COMPLETED };
    
    /// current state
    VitoState state = SYNC_REQUIRED;

    enum ActionType { DoRead, DoWrite };
    struct Action {
        ActionType type;
        const Address addr;
        int retry = 0;

        const std::vector<uint8_t> toSendBuffer() {
            std::vector<uint8_t> buff;
            if (type == DoRead) {
                // Encode Read
                buff.push_back(0xF7);
                buff.push_back((addr.addr >> 8) & 0xFF);
                buff.push_back(addr.addr & 0xFF);
                buff.push_back(addr.length);
            }
            return buff;
        }
    };
    
    ReadEventHandler _readHandler;
    QueueProcessedHandler _processedHandler;
    LogEventHandler _logHandler;

    std::queue<Action>_queue;

    Optolink opto;
};
