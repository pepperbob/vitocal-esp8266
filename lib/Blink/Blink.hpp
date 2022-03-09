#pragma once

class Blinker {
    public:
    Blinker(int led);
    void blink(int no);
    void blink(int no, int delay_ms);
    void blinkShort(int no);

    private:
    int _led;
};