#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>

#define  BT_DEMO_SERVICE "0fd8d562-b821-47ad-9265-99b96ab26f04" // service
#define  BT_DEMO_READ "653b8ab6-ee64-4901-bd87-e1e1333ee999" // characteristic 
#define  BT_DEMO_WRITE "fe29e0ad-f703-4f5b-8a97-172830b3ade4" // characteristic 
#define  BT_DEMO_NOTIFY "653b8ab6-ee64-4901-bd87-e1e1333ee774" // characteristic 
#define  BT_DEMO_NOTIFY_REPEAT "28fd6056-489f-4e63-a785-0cf3120cfea4"

#define  BT_SENSORS_SERVICE "91d5b028-4b12-4770-b8db-c0f6a7323a9e"
#define  BT_QUATERNION_NOTIFY "071a0397-db8c-4bc3-9f4a-4b22b087e7cd"    

class BleHelper {
private:
    // using RTOS (built into esp32) assures NO simultaneous access to ble hardware
    static SemaphoreHandle_t staticBleMutex;
    // handle to bluetooth hardware radio
    BLEServer* BLE_HARDWARE_POINTER = NULL;
    
    BLEService* DEMO_SVC = NULL; // demo service
    // the "characteristics" [ble-speak is weird] of above service.
    BLECharacteristic* DEMO_NOTIFY_BRGC = NULL;
    BLECharacteristic* DEMO_NOTIFY_REPEAT_BRGC = NULL;
    BLECharacteristic* DEMO_READ_BRGC = NULL;
    BLECharacteristic* DEMO_WRITE_BRGC = NULL;
    
    BLEService* SENSOR_SVC = NULL; // another service
    BLECharacteristic* QUATERNION_BRGC = NULL;
 public:
    void setupBLE_stuff();
    void sendNotifyTest(const std::string& msg);
    
    class MyBLEServerCallbacks : public BLEServerCallbacks {
        void onConnect(BLEServer* bleHdwreHandle_pServer) override;
        void onDisconnect(BLEServer* bleHdwreHandle_pServer) override;
    };

};

// NO! this goes into c++ file, not header ...  BleHelper myBleHelper;