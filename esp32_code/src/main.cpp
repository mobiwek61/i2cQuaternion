#include <Arduino.h>
// dont do this for file under /lib it makes duplicate functions:    #include "./xyz.cpp"
#include "TDK_dmp_demo/DmpTest.h"
#include "starter_demo/StarterDemo.h"
#include "S3LedHelper.h"
 
/** 
 * This **main.cpp** app is nothing more than a switcher to go between examples.  
 * To make your own app, add reference to it here and set myRunnable to it  
 */
DmpTest myDmpTest;   
StarterDemo myStarterDemo;

/** Java coders: the * means myRunnable is a **pointer**. The & means 
 *  "get the address of this class instance". */
ArduinoRunnable* myRunnable = 
     &myDmpTest;  // the working example code
     // &myStarterDemo; // a good starting point for app using this library

// hardware solder connections. Not used at this level but ought to be. 
// #define I2C_SDA 8 // data pin
// #define I2C_SCL 9 // clock pin

void setup() {
    Serial.begin(115200); // initialize for Serial.printf() ...
    delay(100); // time to stabilize
    Serial.println("======++++++++++++ top level setup()");
    // Following is useful to detect your device if you're unsure of its address. AI
    // I2C_Helper::scanI2C(I2C_SDA, I2C_SCL); return; 

    // LED as shipped is painfully bright on breakout board esp32-s3-devkitc
    S3LedHelper painfullyBrightLED;
    painfullyBrightLED.setColor(0, 122, 122, 3); 
    // call setup() of your chosen app setup above
    myRunnable->setup();
}
void loop() {
    myRunnable->loop();
}
