#include "Arduino.h"
#include "DemoRTOSWorker.h"
#include "BleHelper.h"

/* copilot: "BLE on the ESP32 is not thread‑safe, and two tasks calling BLE APIs at the same time will eventually corrupt the stack"
   This class uses RTOS to do a mutex to prevent ble hardware frem getting
   accessed concurrently
*/
/* originally written by gemini 3/2026 easier to see what it did than try
   to learn RTOS on my own...  it may be plagarized from:
   https://randomnerdtutorials.com/esp32-freertos-arduino-tasks/#freertos-and-esp32
*/
DemoRTOSWorker::DemoRTOSWorker(std::string pattern, std::string flavor)
    : _bleHelper(nullptr), _interval(2000),
    _counter(0), _taskHandle(NULL), _flavor(flavor), _pattern(pattern) {}

void DemoRTOSWorker::doTaskCreate(MyBleHelper* helper, uint32_t intervalMS) {
    _bleHelper = helper; _interval = intervalMS;
    _bleMutex = xSemaphoreCreateMutex();
    // Create new task on Core 0
    xTaskCreatePinnedToCore(     // https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html
        // Pointer to the task. Terminated using vTaskDelete function OR infinite loop.
        DemoRTOSWorker::bleLoopWithMutex,
        //DemoRTOSWorker::bleMsgWithMutex,
        "RTOS_Worker_Task", 4096,  // Stack size (Bytes)
        // below is passed to task as pvParameters
        this, 1,  // Priority (1 is standard)
        &_taskHandle,            // Handle to manage the task later
        0                        // Core ID (0 is the Pro CPU)
    );
}

/**
 * Terminated using vTaskDelete function OR infinite loop.
 */
void DemoRTOSWorker::bleLoopWithMutex(void* pvParameters) {
    DemoRTOSWorker* wrkrThis = (DemoRTOSWorker*)pvParameters;
    // Every FreeRTOS task MUST have an infinite loop
    while (true) {
        wrkrThis->_counter++;
        std::string msg = "DemoRTOSWorker Msg #" + std::to_string(wrkrThis->_counter)
            + " flavor:" + wrkrThis->_flavor;
        Serial.println(msg.c_str());

        // 🔒 Take the mutex before touching BLE
        const int blockForThisManyMSEC = 5;
        if (xSemaphoreTake(wrkrThis->_bleMutex, pdMS_TO_TICKS(blockForThisManyMSEC)) == pdTRUE) {
            // if (_bleHelper) {
            //     _bleHelper->sendNotification(msg);   // your BLE send
            // }
            Serial.printf("%s in mutex\n", wrkrThis->_pattern.c_str());
            xSemaphoreGive(wrkrThis->_bleMutex);  // 🔓 release
        }
        else {
            Serial.println("mutex timeout woefiowefj");
        }

        // Yield CPU control for the specified interval
        // This is where the magic happens: the CPU is free for other tasks
        vTaskDelay(pdMS_TO_TICKS(wrkrThis->_interval));
        // only call outside loop  vTaskDelete(NULL);
    }
}
void DemoRTOSWorker::bleOneMsgWithMutex(void* pvParameters) {
    DemoRTOSWorker* wrkrThis = (DemoRTOSWorker*)pvParameters;
    // Every FreeRTOS task MUST have an infinite loop
        wrkrThis->_counter++;
        std::string msg = "DemoRTOSWorker Msg #" + std::to_string(wrkrThis->_counter)
            + " flavor:" + wrkrThis->_flavor;
        Serial.println(msg.c_str());

        // 🔒 Take the mutex before touching BLE
        const int blockForThisManyMSEC = 5;
        if (xSemaphoreTake(wrkrThis->_bleMutex, pdMS_TO_TICKS(blockForThisManyMSEC)) == pdTRUE) {
            // if (_bleHelper) {
            //     _bleHelper->sendNotification(msg);   // your BLE send
            // }
            Serial.printf("%s in mutex\n", wrkrThis->_pattern.c_str());
            xSemaphoreGive(wrkrThis->_bleMutex);  // 🔓 release
        }
        else {
            Serial.println("mutex timeout woefiowefj");
        }

        // Yield CPU control for the specified interval
        // This is where the magic happens: the CPU is free for other tasks
        vTaskDelay(pdMS_TO_TICKS(wrkrThis->_interval));
        // only call outside loop  
        vTaskDelete(NULL);
}

void DemoRTOSWorker::bleMsgWithMutex(std::string msg) {
    // std::string msg = "one msg RTOS Msg #" + std::to_string(wrkrThis->_counter) + " flavor:" + wrkrThis->_flavor;
    Serial.println(msg.c_str());
    // 🔒 Take the mutex before touching BLE
    const int blockForThisManyMSEC = 5;
    if (xSemaphoreTake(this->_bleMutex, pdMS_TO_TICKS(blockForThisManyMSEC)) == pdTRUE) {
        // if (_bleHelper) {
        //     _bleHelper->sendNotification(msg);   // your BLE send
        // }
        Serial.printf("%s in mutex\n", _pattern.c_str());
        xSemaphoreGive(this->_bleMutex);  // 🔓 release
    }
    else {
        Serial.println("mutex timeout woefiowefj");
    }
}
