#include <Arduino.h>
#include <Wire.h>
#include "i2c_helper.h"
#include <bitset>

/** 
 *  NOTE: setBitSaveOld() depends on ICM_20948 which is bad practice for a universal library.
 *  If not using this chip comment out include and this function or modify
 *  for chip/library you're using. 
 *  Sets requested bit on register without changing other values, then optionally 
 *  verifies the write.  Good for checking for read-only registers.  
 *  example: setBitSaveOld(_myICM, 0x10, 1, 1, "DMP_INT1_EN");
 */
void setBitSaveOld(ICM_20948_I2C icm, uint8_t reg, uint8_t bitMask, int val,
    std::string desc, boolean verifyRead) {
  Serial.printf("\nsetting reg 0x%02X bitMask 0x%02X to bool %d %s ", reg, bitMask, val, desc.c_str());
  std::bitset<8> inBits(bitMask);
  Serial.printf("input bits:%s\n", inBits.to_string().c_str());
  uint8_t rwVal;
  if (icm.read(reg, &rwVal, 1) == ICM_20948_Stat_Ok) {
    delay(111);
    // rwVal now has current value of register
    Serial.printf("  reg 0x%02X old val: 0x%02X bits:%s ", reg, rwVal,
      std::bitset<8>(rwVal).to_string().c_str());
    if (val) { rwVal |= (bitMask); }  // set bits
    else { rwVal &= ~(bitMask); } // clear bits
    Serial.printf(", new val: 0x%02X bits:%s\n", rwVal,
      std::bitset<8>(rwVal).to_string().c_str());
    icm.write(reg, &rwVal, 1);
    if (verifyRead) {
        Serial.printf(" verifying after write\n");
        delay(111);
        icm.read(reg, &rwVal, 1);
        Serial.printf("  reg 0x%02X verify read: 0x%02X bits:%s\n", reg, rwVal,
          std::bitset<8>(rwVal).to_string().c_str());
        delay(100);
        Serial.flush();
    }
  }
  else { Serial.println("cant read register for update"); }
}

// variadic function written by AI which also taught me word varadic
void I2C_Helper::serialPrintf(SemaphoreHandle_t serialMutex, const char *fmt, ...)
{
    if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE)
    {
        char buffer[128]; // Adjust size based on your longest expected string
        va_list args;
        va_start(args, fmt);
        // handles the "..." arguments
        vsnprintf(buffer, sizeof(buffer), fmt, args); 
        va_end(args);
        Serial.print(buffer); // Now we print the safely formatted string
        xSemaphoreGive(serialMutex);
    }
}

void I2C_Helper::scanI2C(int I2C_SDA, int I2C_SCL) {
  unsigned long startMillis = millis();
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
  Wire.setTimeOut(50); // Reduced slightly for faster recovery/scan
  Serial.println("=== scanI2C ===");
  for (uint8_t addr = 1; addr < 127; addr++) {
    yield();
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();
    switch (error) {
    case 0: {
      Serial.printf("Found 0x%02X ", addr); 
      Wire.beginTransmission(addr); 
      uint8_t WHO_AM_I_REG = 0x00;
      // caution: ICM-20948 PDF says 0x00 but ai is wrong!
      Wire.write(WHO_AM_I_REG);
      if (Wire.endTransmission(false) == 0) {
        Wire.requestFrom(addr, (uint8_t)1);
        if (Wire.available()) {
          uint8_t who = Wire.read();
          Serial.printf(" | WHO_AM_I: 0x%02X \n", who);
        }
      }
      break;
    }
    case 4:
      Serial.printf("Unknown error at 0x%02X (Possible Bus Hang)\n", addr);
      break;
    default:
      break;
    }
  }
  Serial.printf("=== scanI2C done === Time msec used: %lu\n", millis() - startMillis);
}

// in .h file, static and defaults defined BUT NOT HERE !
String I2C_Helper::toBinaryString(int value, int bits, bool addSpaces) {
  String result = "";
  for (int i = bits - 1; i >= 0; i--) {
    result += ((value >> i) & 1) ? '1' : '0';
    if (addSpaces && i % 4 == 0 && i > 0) {
      result += ' ';
    }
  }
  return result;
}
