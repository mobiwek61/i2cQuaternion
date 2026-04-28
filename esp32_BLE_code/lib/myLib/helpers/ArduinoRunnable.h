
// #pragma once// these prevent double inclusion from different including files
#ifndef ARDUINO_RUNNABLE_H
#define ARDUINO_RUNNABLE_H

/** convenience base class to enable pointers to "arduino apps" when
 *  used in development, testing, demos. Just defines setup(), loop()  
*/
class ArduinoRunnable
{
public:
    virtual ~ArduinoRunnable() {} // destructor
    virtual void setup() = 0;
    virtual void loop() = 0;
};

#endif
