
#pragma once

// bad thing to do for a utility but wanna do it anyway.
#include "ICM_20948.h"

void setBitSaveOld(ICM_20948_I2C icm, uint8_t reg, uint8_t bitMask, int val, 
    std::string desc, boolean verifyRead = false);

class I2C_Helper {
public:
    static void serialPrintf(SemaphoreHandle_t serialMutex, const char *fmt, ...);
    static void scanI2C(int I2C_SDA, int I2C_SCL);
    // Defaults are defined here in the header
    static String toBinaryString(int value, int bits = 16, bool addSpaces = false);
}; // <--- Must have this semicolon
