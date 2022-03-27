#pragma once

#include <Arduino.h>
#include <functional>
#include <queue>

#define VITO_ACK 0x01
#define VITO_HELLO 0x05


/**
 * @brief Wrapper for a value read from a address.
 */
struct AddressValue {
    std::vector<uint8_t> val;

    uint8_t* toRaw() {
        return val.data();
    }

    float toTemp() const {
        return toInt()/10.0f;
    }

    int16_t toInt() const {
        switch(val.size()) {
            case 0:
                return 0;
            case 1:
                return val[0];
            case 2:
                return (val[1] << 8 | val[0]);
            default:
                return -255;
        }
    }
};

/**
 * @brief Defines a specific address.
 * 
 * Address has a name, a data point, if it can be written and the length of 
 * the expected data at this address.
 * 
 */
struct Address {
    const char* name;
    const uint16_t addr;

    const bool writable = false;
    const uint8_t length = 2;
};

/**
 * @brief Event that is published upon successfull reading.
 */
struct ReadEvent {
    const Address address;
    const AddressValue value;
};

template <typename T>
struct ReadResult {
    const T result;
    const bool isError;
};

class Optolink {
    public:
    /// Setup Serial Port
    void setup(HardwareSerial* serial);
    
    /// try to sync, retruns true if Vitocal could be detected.
    bool sync();
    
    /// sends data to serial output; returns true if data could be sent.
    bool send(std::vector<uint8_t> sendBuffer);

    /// Read length of data from serial input.
    ReadResult<std::vector<uint8_t>> read(uint8_t length);

    private:
    Stream* _serial;
};

/// Callback definition of an eventhandler subscribing to read events.
typedef std::function<void(ReadEvent)> ReadEventHandler;
typedef std::function<void()> QueueProcessedHandler;

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

    private:
    /// states of the state-machine
    enum VitoState { SYNC_REQUIRED, IDLE, PROCESS_QUEUE, EXPECT_RESPONSE };
    
    /// current state
    VitoState state = SYNC_REQUIRED;

    enum ActionType { DoRead, DoWrite };
    struct Action {
        ActionType type;
        Address addr;
        int retry = 0;

        std::vector<uint8_t> toSendBuffer() {
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

    std::queue<Action>_queue;

    Optolink opto;
};

const Address ADDR_AU = { "temp_au", 0x0101 };
const Address ADDR_WW = { "temp_ww", 0x010D };
const Address ADDR_VL = { "temp_vl", 0x0105 };
const Address ADDR_RL = { "temp_rl", 0x0106 };
