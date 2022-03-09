#include <Arduino.h>
#include <Blink.hpp>

Blinker::Blinker(int led) {
    pinMode(led, OUTPUT);
    _led = led;
    blink(1);
}

void Blinker::blink(int times) {
    blink(times, 250);
}

void Blinker::blink(int times, int delay_ms) {
    for (int x = times; x > 0; x--) {
        digitalWrite(_led, LOW);
        delay(delay_ms);
        digitalWrite(_led, HIGH);
        delay(delay_ms);
    }
}

void Blinker::blinkShort(int times) {
    blink(times, 100);
}