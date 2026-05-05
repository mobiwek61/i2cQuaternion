#include <Arduino.h>
#include <i2c_helper.h>
#include "StarterDemo.h"

/** simple example demo to serve as starting template for new app 
 *  It doesn't use the library of this project; it only scans the
 *  i2c bus for connected devices.  
*/
StarterDemo::StarterDemo() {
  // constructor (empty for now)
}

// these are the esp32 pins physically jumpered or soldered to sensor chip
#define I2C_SDA_DATAPIN 8 
#define I2C_SCL_CLOCKPIN 9 
// 4.7kΩ Pull-up resistors may be REQUIRED!

void StarterDemo::setup() {
  Serial.printf("now scanning i2c bus %s\n", " right now"); // cpp printf() !
  I2C_Helper::scanI2C(I2C_SDA_DATAPIN, I2C_SCL_CLOCKPIN);
}

static int loopCounter = 0;
void StarterDemo::loop() {
  Serial.printf("loop number %d\n", loopCounter++); 
  delay(1000);
}
