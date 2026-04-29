#pragma once
#include <Arduino.h>
#include <ArduinoRunnable.h>
#include <i2c_helper.h>

class DmpTest : public ArduinoRunnable {
    public:
        DmpTest();
        //void pullUpPinsInSoftware() override;
        void setup() override;
        void loop() override;
        void sampleData();

};



