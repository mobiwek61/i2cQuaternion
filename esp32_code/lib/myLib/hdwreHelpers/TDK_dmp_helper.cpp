#include "TDK_dmp_helper.h"
#include <i2c_helper.h>
#include <bitset>
#include <MathHelper.h>

int msecBlok = 200;
        
// REQUIRES THIS FLAG IN PLATFORMIO.INI: "-D ICM_20948_USE_DMP".
// This code Originated from Example6_DMP_Quat9_Orientation.ino example in SparkFun ICM-20948 library
// ref: https://randomnerdtutorials.com/esp32-freertos-mutex-arduino/ 
// [above is plagarized by ai]... 
  
// define STATIC CLASS members (declared in .h) here, to allocate memory or linker fails.
SemaphoreHandle_t TDK_dmp_helper::_i2cMutex = NULL;
SemaphoreHandle_t TDK_dmp_helper::_serialMutex = NULL; 
/** This is a C++ Instantiator
 *  @param wire is i2c bus object.
 *  @param dataRdyCallback function invoked upon interrupt from DMP.   
 *  it gets run by this class from within a separate RTOS task/thread, so
 *  no need to finish quickly.
 *  @param i2cMutex is FreeRTOS mutex object to ensure single access to i2c at a time.
 *  @param serialMutex similar Serial.print()
 */
TDK_dmp_helper::TDK_dmp_helper(TwoWire& i2cWire, DmpInterrCallback dataRdyCallback,
    SemaphoreHandle_t i2cMutex, SemaphoreHandle_t serialMutex, int hardwre_int_pin) : 
    _wire(i2cWire), _dataRdyCallback(dataRdyCallback), _hardwre_int_pin(hardwre_int_pin) {
        _i2cMutex = i2cMutex; // init STATIC member in body of constructor.
        _serialMutex = serialMutex; // init STATIC member in body of constructor.
}

volatile int interruptCt = 0;

/** Sets up the interrupts for the DMP. Numbers set here are from PDF document downloaded
 *  from manufacturer. I'm not sure about copyrights, so it's not in this project.
*/
void TDK_dmp_helper::setupPinInterrupt() {
    setBitSaveOld(_myICM, 0x0F, 0x20, 0, "Latch, INT_PIN_CFG");
    setBitSaveOld(_myICM, 0x10, 0x02, 1, "dmp int: DMP_INT1_EN");
    setBitSaveOld(_myICM, 0x11, 0x01, 0, "all ints: INT_ENABLE_1");
    // hey esp32, listen on this pin for interrupt when DMP   
    // [or maybe another chip] pulls it low.  
    pinMode(_hardwre_int_pin, INPUT_PULLDOWN);
    delay(10); // Let the pin settle
    // assure got the same object..  Serial.printf("I am: %X flavor: %s\n", this, this->flavor.c_str());
    // tie the ISR to the interrupt pin. 
    attachInterruptArg(digitalPinToInterrupt(_hardwre_int_pin),
        digiMotionProcessorInterruptSvcRoutine, this, RISING);
}

/** THREAD SAFE 
 * Sets up:
 * - DMP Digital Motion Processor on ICM-20948 chip
 * - sets chip registers for interrupt when DMP data ready
 * - sets up esp32 pin interrupt and ISR Interrupt Service Routine
 * - sets up RTOS task/thread as endless loop, which WAITS for next ISR event.  
 *   It invokes callback supplied by app to handle data how it wants to.  
*/
bool TDK_dmp_helper::begin(boolean my_i2c_address_69_true_68_false) {
    // lock access to buses to prevent simultaneus access -> crash
    xSemaphoreTake(TDK_dmp_helper::_serialMutex, pdMS_TO_TICKS(msecBlok)); 
    xSemaphoreTake(TDK_dmp_helper::_i2cMutex, pdMS_TO_TICKS(msecBlok)); 
    _wire.begin();// assumes caller Wire.setPins(I2C_SDA_DATAPIN, I2C_SCL_CLOCKPIN); 
    _wire.setClock(40000); // slow for DMP
    bool initialized = false;
    uint8_t retryCount = 0;
    while (!initialized && retryCount < 10) {
        // set this var to reflect chip i2c addr; varies between breakout board mfrs.
        bool my_i2c_address_69_true_68_false = true; // to conform to library...
        _myICM.begin(_wire, my_i2c_address_69_true_68_false); 
        Serial.printf("ICM-20948 Status: %s", _myICM.statusString());
        if (_myICM.status != ICM_20948_Stat_Ok) {
            Serial.println(F("*********ICM-20948 initialize failure Retrying..."));
            delay(500); retryCount++;   }
        else {
            Serial.println(F("ICM-20948 initialized successfully."));
            initialized = true; }
    }
    _myICM.setBank(0);
    uint8_t rwVal; uint8_t WHO_AM_I_REG = 0x00; // ai gives wrong value this register
    _myICM.read(WHO_AM_I_REG, &rwVal, 1); 
    Serial.printf("reg 0x00 WHO_AM_I val: %02X\n", rwVal);
    if (rwVal != 0xEA) {
        Serial.println("k WHO_AM_I register mismatch, i2c problem?. Any key to continue");
        while (!Serial.available()) delay(10); return false;
    }

    /* ref: https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary/blob/main/DMP.md
    initializeDMP downloads the DMP (for quaternion) firmware; and configures registers.
    Quaternions obtained by mixing gyro, accelerometer, mag data using heavy math. 
    */
    if (_myICM.initializeDMP() != ICM_20948_Stat_Ok)
        Serial.println(F("DMP failed to initialize."));
    if (_myICM.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) != ICM_20948_Stat_Ok)
        Serial.println(F("DMP failed ORIENTATION initialize."));
    Serial.printf("init dmp and dmp sensor done\n");
    /* data rates:  0;   225.0; | 1;   112.5; | 10;  20.4;  | 21;  10.2;  |
    * | 44;  5.0;   | 111; 2.0;   | 224; 1.0;   | 449; 0.5;   | */
    if (_myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 10) != ICM_20948_Stat_Ok)
        Serial.println(F("DMP failed to set DMP_ODR_Reg_Quat9"));
    if (_myICM.enableFIFO() != ICM_20948_Stat_Ok)
        Serial.println(F("DMP failed to enable FIFO."));
        
    if (_myICM.enableDMP() != ICM_20948_Stat_Ok)
        Serial.println(F("DMP failed to enable."));
    if (_myICM.intEnableDMP(true) != ICM_20948_Stat_Ok)
        Serial.println(F("DMP failed to enable."));
    if (_myICM.resetDMP() != ICM_20948_Stat_Ok)
        Serial.println(F("DMP failed to reset."));
    if (_myICM.resetFIFO() != ICM_20948_Stat_Ok)
        Serial.println(F("FIFO failed to reset."));
    Serial.printf("dmp settings group done");
    setupPinInterrupt();
    setupRTOSWorkerTask();
    // we're done now, release access to buses
    xSemaphoreGive(TDK_dmp_helper::_i2cMutex);  
    xSemaphoreGive(TDK_dmp_helper::_serialMutex);  
    return initialized;
}


// ref: https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/11-Deferred-interrupt-handling
// ref: https://freertos.org/Documentation/02-Kernel/04-API-references/05-Direct-to-task-notifications/02-vTaskNotifyGiveFromISR
// ref: https://wina.freertos.org/Documentation/02-Kernel/04-API-references/05-Direct-to-task-notifications/03-ulTaskNotifyTake
// ref: https://ece353.engr.wisc.edu/freertos/freertos-interrupt-management/

/** ISR "Interrupt Service Routine" implemented as a FreeRTOS task/ endless loop. 
 *  FREERTOS gives ISR param "pvParameters" as void* pointer, so cast to type is needed.  
 *  Framework only gives ISR a pointer to void as arg, so cast it here.
 *  Nobody cares how much time taken here because its own RTOS task-thread.  
 *  Semaphores used twice here:
 *  - one to block loop until data is ready
 *  - another (mutex) to get semaphore to i2c bus.   
 *  ⛵⛵ "this is a sailor who waits for a semaphore. This sailor gets notified to wake up, told there is
 *   data ready from the DMP and fetches it. It then gives the quaternion to another sailor [the callback _dataRdyCallback] 
 *   who moves the helm over or something like that. Note: it waits for the other sailor to complete." ⛵⛵ 
 */
void isr_worker_loop_wait4semaphore(void* pvParameters) {
    while (true) {
        // ⬇️⬇️⬇️ BLOCK ⬇️⬇️⬇️ until notified by ISR that data is ready
        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
        TDK_dmp_helper* thisHelper = static_cast<TDK_dmp_helper*>(pvParameters);
        Quaternion4 quat;
        // ⛵ use mutex semaphore to avoid a tiny train wreck on i2c bus, when more than 1 thread
        // accesses it at same time. Think of one-track viaduct over a river as the  
        // i2c bus and each train as each task/thread active at a time.  At this writing
        // its not a problem but if more sensors added later, is a big problem. ⛵
        if (xSemaphoreTake(TDK_dmp_helper::_i2cMutex, pdMS_TO_TICKS(msecBlok)) == pdTRUE) {
            // now I got the semaphore and nobody else has it.
            // now get on the i2c bus and request quaternion data from the IMU. 
            thisHelper->fetchDMPdata(&quat);
            // now give semaphore back ⛵⛵ so it's available to the next train waiting at the siding.  
            xSemaphoreGive(TDK_dmp_helper::_i2cMutex);  
        }
        // invoke callback ⬇️⬇️⬇️ from app using this. Typically sends data over ble.
        // buoy88
        thisHelper->_dataRdyCallback("thurs7 ", quat);
        // now repeat loop and WAIT for "blocking" semaphore. 
    }
}

// ref: https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html
/** use freeRTOS to create a worker task and attach existing function to it.
 *  In this application, this function should be endless loop with semaphore blocking call.  */
void TDK_dmp_helper::setupRTOSWorkerTask() {
    xTaskCreatePinnedToCore(  
        isr_worker_loop_wait4semaphore,
        "dmp_isr_Worker_Task", 4096,  // Stack size (Bytes)
        // below is passed to task as pvParameters, later to cast to TDK_dmp_helper* in task function.
        (void*)this, 1,  // Priority 
        // CONFUSION!!! set ISR "HANDLE"; the ISR pointer is NOT USED for now on!
        &_wakeMeUpOnInterrupt, 
        0             // Core 0; esp32s3 got 2, esp32c3 got 1.
    );
    //identity of object is same?? Serial.printf("task handle in setup: %X flavor: %s\n", this->_taskHandle, flavor.c_str());
}

/** Interrupt Service Routine setup by attachInterruptArg() which ties
 *  hardware interrupt pin to this ISR. This ISR cannot hold up the thread, so
 *  instead of doing work, it wakes up a worker task-thread.
 *  ⛵⛵ "the captain listens for the boatswain's whistle's tune at which time
 *  she run this to figure out what to do about it. She can't be distracted  
 *  by she unblocks the proper worker thread [says make it so] and immediately returns
 *  to running the ship. " ⛵⛵
 */
void digiMotionProcessorInterruptSvcRoutine(void* thisHelperAsAVoidPtr) {
    // Serial.printf("..interruptCt: %d \n", interruptCt++);
    TDK_dmp_helper* thisHelper = static_cast<TDK_dmp_helper*>(thisHelperAsAVoidPtr);
    // I2C_Helper::serialPrintf(TDK_dmp_helper::_serialMutex, "int: %d ", interruptCt++);
    // Serial.printf("..task handle in ISR: %X flavor:%s \n", thisHelper->_wakeMeUpOnInterrupt, thisHelper->flavor.c_str());
    BaseType_t IwasBlockedAndHigherPriorityThanYou = pdFALSE; // for output
    // this line returns immediately with pdTRUE if the task was waiting.
    // Now UNBLOCK the worker thread. CONFUSION!!! use the ISR "HANDLE", not the
    // actual ISR pointer...
    vTaskNotifyGiveFromISR(thisHelper->_wakeMeUpOnInterrupt, &IwasBlockedAndHigherPriorityThanYou);
    // SAVE...   if (IwasBlockedAndHigherPriorityThanYou == pdFALSE) 
    //     Serial.println("WARNING task NOT waiting or not higher.....");
    // ⬇️⬇️⬇️ conditionally yield to the higher priority task !
    portYIELD_FROM_ISR(IwasBlockedAndHigherPriorityThanYou); 
}

/** !! NOT THREAD SAFE: call only within lock of i2c bus !!
 *  read data from DMP, maybe reset FIFO */
void TDK_dmp_helper::fetchDMPdata(Quaternion4* quat) {
    icm_20948_DMP_data_t data;
    _myICM.readDMPdataFromFIFO(&data); // resets status!
    if (_myICM.status == ICM_20948_Stat_FIFOMoreDataAvail) {
        uint16_t fifoCount; _myICM.getFIFOcount(&fifoCount); // resets status!!
        I2C_Helper::serialPrintf(_serialMutex, 
            "FIFO overload.. resetFIFO. Count: %d  \n", fifoCount);
        _myICM.resetFIFO(); // at startup fifo usually overflowed race condition.
    }
    // if (_myICM.status != ICM_20948_Stat_Ok) {
    // Retrieve quaternion computed by in-chip Digital Motion Processor DMP. this data from ICM-20948 chip!
    // ref: https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary/blob/main/DMP.md
    if ((data.header & DMP_header_bitmap_Quat9) == 0) 
        throw std::runtime_error("DMP not returning quat9");
    // scaling from demo...
    quat->x = ((double)data.Quat9.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
    quat->y = ((double)data.Quat9.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
    quat->z = ((double)data.Quat9.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
    quat->w = sqrt(1.0 - ((quat->x * quat->x) + (quat->y * quat->y) + (quat->z * quat->z)));
}

  
    /* trash
    device reset
       PDF: bit 6 SLEEP When set, the chip is set to sleep mode.
       Clearing the bit wakes the chip from sleep mode.
       bit 7 DEVICE_RESET  */
    // setBitSaveOld(_myICM, 0x06, 0x80, 1, "PWR_MGMT_1 DEVICE_RESET");
    // above puts chip in sleep mode
    // setBitSaveOld(_myICM, 0x06, 0x40, 0, "PWR_MGMT_1 SLEEP"); // WAKE UP !!!!
    
    // _myICM.read(0x06, &rwVal, 1); 
    // Serial.printf("reg 0x06 PWR_MGMT_1 bit 7 reset, bit 6 SLEEP val: %02X bits:%s\n", rwVal,
    //     std::bitset<8>(rwVal).to_string().c_str());
    
