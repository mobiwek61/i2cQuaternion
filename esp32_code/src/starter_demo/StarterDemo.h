#pragma once
#include <Arduino.h>
#include <ArduinoRunnable.h>
#include <i2c_helper.h>

/** c++ uses header files like this to define classes and their functions and members.  
 *  Note they may be public or private. 
 *  Here it declares setup() and loop() as required by an arduino app.  */
class StarterDemo : public ArduinoRunnable {
    public:
        StarterDemo();
        void setup() override;
        void loop() override;
};



