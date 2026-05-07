#include <Arduino.h>
#include <i2c_helper.h>
#include "DmpTest.h"
#include <TDK_dmp_helper.h>
#include <MathHelper.h>

/**
 * Demo of 
 */
DmpTest::DmpTest() {
  // constructor (empty for now)
}

// these are the esp32 pins physically jumpered or soldered to sensor chip
#define I2C_SDA_DATAPIN 8 
#define I2C_SCL_CLOCKPIN 9 
#define HARDWRE_INT_PIN 4

// 4.7kΩ resistors Pull-up resistors may be REQUIRED!

TDK_dmp_helper* myDemo1 = nullptr;

/* use this to prevent >1 threads/callbacks from trainwrecking i2c or serial bus
   These are singletons, used throughout the application to assure 1 at a time */
SemaphoreHandle_t i2cMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t serialMutex = xSemaphoreCreateMutex();

/** This callback is supplied to the library and is run many times per second
 *  when new data is available. It's run in it's own thread, so it won't be slowing
 *  down anything else. Search "buoy88" to see when it's called.  
 *  "⛵this is where a sailor gets its orders from the captain and does them"
 */
void callbackFn_A(std::string strA, Quaternion4 quat) {
  // quaternions are awkward to read, convert it to euler angles for display. 
  std::string eulerStr = getEulerString(quat.x, quat.y, quat.z);
  I2C_Helper::serialPrintf(serialMutex, "%s\n", eulerStr.c_str());
  // I2C_Helper::serialPrintf(serialMutex, "DmpTest strA %s eulerStr %s\n", strA.c_str(), eulerStr.c_str());
  // I2C_Helper::serialPrintf(serialMutex, "DmpTest strA %s\n", strA.c_str());
}

void DmpTest::setup() {
  // other project... I2C_Helper::testQMC5883_magnetometr_Connection(I2C_SDA_DATAPIN, I2C_SCL_CLOCKPIN); return;
  ////// not needed... this is at startup..  if (xSemaphoreTake(i2cMutex, MUTEX_TIMEOUT_MS)) {
  I2C_Helper::scanI2C(I2C_SDA_DATAPIN, I2C_SCL_CLOCKPIN);
  ////// xSemaphoreGive(i2cMutex); // "next"
  ///// } else { throw std::runtime_error("Failed to take i2c mutex in setup"); }

  //  Wire is a GLOBAL object defined in Arduino library, Wire.h
  //  This SAME object is used by the device libraries used here, so
  //  that lets you setup the "Wire" object here, and have it work for ALL i2c devices.  
  Wire.setPins(I2C_SDA_DATAPIN, I2C_SCL_CLOCKPIN); // tell it which pins for data and clock i2c.
  myDemo1 = new TDK_dmp_helper(Wire, callbackFn_A, i2cMutex, serialMutex, HARDWRE_INT_PIN);
  delay(100); // let bus settle
  // this looks a bit strange because the published library works this way
  boolean my_i2c_address_69_true_68_false = true; // OR it's 0x68; use scanI2c to determine.
  if (!myDemo1->begin(my_i2c_address_69_true_68_false)) {
    Serial.println("begin failed. Press key to continue");
    while (!Serial.available()) delay(10);
  }
  else Serial.println("DmpTest setup complete");
  //imuHelper.setDeviceRegsNgetReady();
  //// PASTE CALIBRATION RESULT IN BELOW SECTION 
  /////  gyroHelper.performCalibration();

  // **** CALIBRATION, run this to get above 2 lines to COPY from:  ****
  ///////////////// magHelper.getMagCalibrateData(); return;
}

static int loopCounter = 0;
void DmpTest::loop() {
  // no loop code because timing comes from interrupt pin on DMP chip
  delay(20000);
}

