#ifndef TDK_dmp_helper1_H
#define TDK_dmp_helper1_H

#include <Arduino.h>
#include <Wire.h>

// doesnt work using platformio.ini -> #define ICM_20948_USE_DMP  
#include "ICM_20948.h"

struct Quaternion4 { double x;    double y;    double z;    double w; };
void testSetRegister(ICM_20948_I2C icm_A);
void IRAM_ATTR digiMotionProcessorInterruptSvcRoutine(void* arg);

/* Helper class for ICM-20948 Gyro, Mag, and Digital Motion Processor (sensor fusion)
 * Timing of data handled by chip's DMP which fires interrupt pin when sensor fusion
 * data is ready to read. Interval set at setDMPODRrate() in cpp.
 * Invoked callback supplied by app when data ready.
 * Uses RTOS to put interrupt service routines in separate threads/tasks.
 * Uses RTOS to semaphore-mutex i2c bus and Serial.print() to prevent concurrent access, which
 * will crash the system, as observed.
*/
class TDK_dmp_helper {
/* attention java coders: c++ declares functions here and defines in .cpp  
   But, in .cpp they are not grouped inside class definition like java, but  
   are separate, with the MyClass::myFn() style. */
public:
    std::string flavor = "pineapple";
    // set up callback profile type(?)
    using DmpInterrCallback = std::function<void(std::string, Quaternion4)>;
    TDK_dmp_helper(TwoWire& wirePort, DmpInterrCallback dataRdyCallback,
        SemaphoreHandle_t i2cMutex, SemaphoreHandle_t serialMutex, int hardwre_int_pin);

    /* true if ICM_20948 0x69 false if 0x68. Done this way to
       conform to library call.*/
    bool begin(boolean my_i2c_address_69_true_68_false);
    void fetchDMPdata(Quaternion4* quat);
    void printData();
    // TODO: send this over ble; maybe need to send as array packed into byte buffer
    static String quat4ToJSON(const Quaternion4* q) {
        return String("{\"x\":") + q->x + ",\"y\":" + q->y +
            ",\"z\":" + q->z + ",\"w\":" + q->w + "}";
    }
    /** converts quaternion to yaw/pitch/roll euler string */
    static std::string getEulerString(double q1, double q2, double q3);
    DmpInterrCallback _dataRdyCallback;
    TaskHandle_t _wakeMeUpOnInterrupt;
    static SemaphoreHandle_t _i2cMutex;
    static SemaphoreHandle_t _serialMutex;
private:
    void setupPinInterrupt();
    void setupRTOSWorkerTask();
    //void dmpIntHandler(void* thisHelperAsAVoidPtr);
    
    ICM_20948_I2C _myICM;
    
    TwoWire& _wire;
    int _hardwre_int_pin;
    // Internal helper functions
    void printPaddedInt16b(int16_t val);
    void printFormattedFloat(float val, uint8_t leading, uint8_t decimals);
};

#endif