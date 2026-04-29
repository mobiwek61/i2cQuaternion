#pragma once
#include <Arduino.h>
#include <ArduinoRunnable.h>
#include <i2c_helper.h>

class StarterDemo : public ArduinoRunnable {
    public:
        StarterDemo();
        void setup() override;
        void loop() override;
};



