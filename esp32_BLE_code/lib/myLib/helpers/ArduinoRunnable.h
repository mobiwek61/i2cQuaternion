
// #pragma once// these prevent double inclusion from different including files
#ifndef ARDUINO_RUNNABLE_H
#define ARDUINO_RUNNABLE_H

/** In main.cpp. add new demos by extending this interface.  
 *  Then add call and comment out calls to other demo, 
 *  effectively replacing the main android app.
 *  deepseek:"abstract base class that serves as an interface in C++"
 */
class ArduinoRunnable
{
public:
    virtual ~ArduinoRunnable() {} // destructor
    virtual void setup() = 0;
    virtual void loop() = 0;
};

#endif
