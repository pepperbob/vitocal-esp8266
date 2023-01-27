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
    
    /// @brief Execute Event Loop; needs to be called in the loop function.
    /// @return True indicates idle/no tasks in queue, False busy/at least one task in queue. 
    bool loop();

    /// Queues read task for specified address.
    bool doRead(const Address* address);

    /// @brief Executed when address has been read successfully.
    /// @param handler 
    void onRead(ReadEventHandler handler);
    /// @brief Handler is executed when an address could not be read and was removed from the queue.
    /// @param handler 
    void onReadError(ReadErrorEventHandler handler);

    /// @brief Handler is executed when all reads are executed and queue is empty.
    /// @param handler 
    void onQueueProcessed(QueueProcessedHandler handler); 
    void onLog(LogEventHandler handler);

    private:

    void _doLog(std::string message);

    /// states of the state-machine
    enum VitoState { SYNC_REQUIRED, SYNCHED, PROCESS_REQUIRED, RESPONSE_EXPECTED, RESPONSE_COMPLETED };
    
    /// current state
    VitoState _state = SYNC_REQUIRED;

    enum class ActionType { DoRead, DoWrite };
    struct Action {
        const ActionType type;
        const Address* addr;
        
        int retry = 0;

        const std::vector<uint8_t> toSendBuffer() {
            std::vector<uint8_t> buff;
            // implements reads only currently
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
    ReadErrorEventHandler _readErrorHandler;
    QueueProcessedHandler _processedHandler;
    LogEventHandler _logHandler;

    std::queue<Action> _queue;

    Optolink _opto;
};
