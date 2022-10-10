#pragma once

#include <functional>

#include <Address.hpp>

/**
 * @brief Event that is published upon successfull reading.
 */
struct ReadEvent {
    const Address* address;
    const AddressValue value;
};

/**
 * @brief Event that is published to log messages
 */
struct LogEvent {
    const std::string message;
    const unsigned long millis;
};

/// Callback definition of an eventhandler subscribing to read events.
typedef std::function<void(ReadEvent)> ReadEventHandler;
typedef std::function<void()> QueueProcessedHandler;
typedef std::function<void(LogEvent)> LogEventHandler;