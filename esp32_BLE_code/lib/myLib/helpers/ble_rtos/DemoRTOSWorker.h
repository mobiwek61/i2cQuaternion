#ifndef RTOS_WORKER_H
#define RTOS_WORKER_H

#include <Arduino.h>
#include <string>

// Forward declaration of your BLE class to keep headers light
class MyBleHelper; 

/**
 * This class uses RTOS to add timing loops to esp32 without blocking
 * the processor as delay() does.
 * Useful for sending periodic sensor data.
 * It does NOT setup mutex or semaphores for ble, thats done elsewhere.
 * This class uses c from RTOS library (builtin) not c++ so no class stuff.
 */
class DemoRTOSWorker {
public:
    DemoRTOSWorker(std::string pattern, std::string flavor);
    
    // Starts the background thread on a specific core
    void doTaskCreate(MyBleHelper* helper, uint32_t intervalMS = 2000);
    void bleMsgWithMutex(std::string msg);
    
private:
    static void bleLoopWithMutex(void* pvParameters);
    static void bleOneMsgWithMutex(void* pvParameters);

    MyBleHelper* _bleHelper;
    uint32_t _interval;
    uint32_t _counter;
    TaskHandle_t _taskHandle;

    // added later
    std::string _pattern;
    std::string _flavor;
    SemaphoreHandle_t _bleMutex;
};

#endif