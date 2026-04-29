#include <Arduino.h>
// dont do this for file under /lib it makes duplicate functions:    #include "./xyz.cpp"
#include "TDK_dmp_demo/DmpTest.h"
#include "starter_demo/StarterDemo.h"
#include "S3LedHelper.h"
 
/** This app structures so you can add your own implementations of 
 * ArduinoRunnable while keeping myDmpTest as a functioning example.  */
DmpTest myDmpTest;   
StarterDemo myStarterDemo;

ArduinoRunnable* myRunnable = 
     // &myDmpTest;
     &myStarterDemo;

// #define I2C_SDA 8 // data pin
// #define I2C_SCL 9 // clock pin

void setup() {
    Serial.begin(115200); // for Serial.printf() ...
    delay(3000); // time to stabilize
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
