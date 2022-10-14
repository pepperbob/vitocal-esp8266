#pragma once

#include <Arduino.h>
#include <functional>
#include <queue>

#include <Optolink.hpp>
#include <Address.hpp>
#include <Callback.hpp>

class Vitocal {
    public:
    void setup(HardwareSerial* serial);
    
    /// event loop for state-machine; returns true if idle.
    bool loop();

    /// Queue read operation for address
    bool doRead(const Address* address);

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

    enum class ActionType { DoRead, DoWrite };
    struct Action {
        const ActionType type;
        const Address* addr;
        
        int retry = 0;

        const std::vector<uint8_t> toSendBuffer() {
            std::vector<uint8_t> buff;
            if (type == ActionType::DoRead) {
                // Encode Read
                buff.push_back(0xF7);
                buff.push_back((addr->addr >> 8) & 0xFF);
                buff.push_back(addr->addr & 0xFF);
                buff.push_back(addr->length);
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
