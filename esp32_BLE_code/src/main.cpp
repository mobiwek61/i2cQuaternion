#include <Arduino.h>
// dont do this:    #include "./proj2/bleServerEcho.cpp"
// bad because this file now "defines" loop() because of include, but 
// then the included file builds and has loop() also and compiler fails. 
// If platformio.ini excludes included file it builds ok CONFUSING!!
#include "TDK_dmp_demo/DmpTest.h"
#include "S3LedHelper.h"
 
DmpTest myDmpTest;   


ArduinoRunnable* myRunnable = 
     &myDmpTest;

#define I2C_SDA 8 // data pin
#define I2C_SCL 9 // clock pin

void setup() {
    Serial.begin(115200);
    delay(3000); // time to stabilize
    Serial.println("================== qwqwqwqwwqw top level setup()");
    // I2C_Helper::scanI2C(I2C_SDA, I2C_SCL); return; // diagnostic to identify I2c devices

    // LED as shipped is painfully bright on breakout board esp32-s3-devkitc
    S3LedHelper painfullyBrightLED;
    painfullyBrightLED.setColor(0, 122, 122, 3); 
    myRunnable->setup();
}
void loop() {
    myRunnable->loop();
}
