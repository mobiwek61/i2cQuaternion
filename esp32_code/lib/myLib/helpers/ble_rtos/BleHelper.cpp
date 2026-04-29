#include "BleHelper.h"

// nRF Connect app (Android/iOS) to debug
/* Welcome to c++ and its neanderthal tendencies.
   Classes "declared in .h" file and their methods "definition in cpp" file.
   Method definitions in cpp are qualified with their class name, but their
   declarations in .h file are not. Note: method declarations in cpp file
   do not appear encased in class definition like java, which has
   c++ genes but is way more modern.  */

   // allocate now to avoid linker error....
SemaphoreHandle_t BleHelper::staticBleMutex = NULL;

void BleHelper::MyBLEServerCallbacks::onConnect(BLEServer* bleHdwreHandle_pServer) {
    BleHelper::staticBleMutex = xSemaphoreCreateMutex();
    Serial.println("✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅ BleHelper instance Connected");
}

void BleHelper::MyBLEServerCallbacks::onDisconnect(BLEServer* bleHdwreHandle_pServer) {
    Serial.println("❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌ BleHelper instance Disconnected");
    vSemaphoreDelete(staticBleMutex);
    staticBleMutex = NULL;
    delay(500);
    bleHdwreHandle_pServer->startAdvertising();
}

void BleHelper::setupBLE_stuff() {
    // Create unique name
    String mac = String((uint32_t)ESP.getEfuseMac(), HEX);
    String name = "demo11"; // + mac.substring(mac.length() - 4);

    // setup ble hardware on esp32
    NimBLEDevice::init(name.c_str());
    BLE_HARDWARE_POINTER = NimBLEDevice::createServer();
    BLE_HARDWARE_POINTER->setCallbacks(new BleHelper::MyBLEServerCallbacks());

    /* gemini: to prevent esp32 crash when using >1 service,
     put this into platformini.ini:  build_flags =
    -DCONFIG_BT_NIMBLE_MAX_SERVICES=5    */
    // setup ble services and their characteristics [ble-speak is weird]
    BLEService* DEMO_SVC = BLE_HARDWARE_POINTER->createService(BLEUUID(BT_DEMO_SERVICE));
    DEMO_NOTIFY_BRGC =
        DEMO_SVC->createCharacteristic(BT_DEMO_NOTIFY, NIMBLE_PROPERTY::NOTIFY);
    DEMO_NOTIFY_REPEAT_BRGC =
        DEMO_SVC->createCharacteristic(BT_DEMO_NOTIFY_REPEAT, NIMBLE_PROPERTY::NOTIFY);
    DEMO_WRITE_BRGC =
        DEMO_SVC->createCharacteristic(BT_DEMO_WRITE, NIMBLE_PROPERTY::WRITE);
    DEMO_READ_BRGC =
        DEMO_SVC->createCharacteristic(BT_DEMO_READ, NIMBLE_PROPERTY::READ);
    // setup callbacks
    class MyBleServerEventCallbacks : public BLECharacteristicCallbacks {
        BleHelper* _parent = nullptr; // Unlike JAVA, C++ "nested" or "inner" class doesnt have parent pointer !
        public: MyBleServerEventCallbacks(BleHelper* _parent) : _parent(_parent) {} // constructor REQUIRED
        void onWrite(BLECharacteristic* dataInHndle) {
              std::string value = dataInHndle->getValue();
              if (value.length() > 0) {
                  // c_str() <- Return const pointer to null-terminated contents.
                  Serial.println(("onWrite() esp received this: " + value).c_str());
                  // now write back a response
                  std::string msg = "onWrite from BleDemo here, you said to me: ";
                  msg += value;
                  Serial.println(("8pm to browser: " + msg).c_str());
                  // this->_parent->chunkSend_DEMO_NOTIFY(msg);

                  // THESE 2 LINES NEED TO BE INSIDE MUTEX OR RACE CONDITION !!
                  this->_parent->DEMO_NOTIFY_BRGC->setValue(msg);
                  // WHAT IF CORE 1 DOES SETVALUE() BEFORE NOTIFY HAPPENS HERE ??
                  this->_parent->DEMO_NOTIFY_BRGC->notify();
              }
          }
    };
    DEMO_WRITE_BRGC->setCallbacks(new MyBleServerEventCallbacks(this));

    class ble_read_timeOfDay : public BLECharacteristicCallbacks {
        void onRead(BLECharacteristic* dataInHndle) {
            dataInHndle->setValue("the time is 10:10 twice a day");
        }
    };
    DEMO_READ_BRGC->setCallbacks(new ble_read_timeOfDay());

    // ble_QUATERNION_NOTIFY =
    //     pService->createCharacteristic(BT_QUATERNION_NOTIFY, NIMBLE_PROPERTY::NOTIFY);

    // Start service
    DEMO_SVC->start();

    //////// now for sensors service  ////////////////////
    // esp32 crashes if below service is NOT started!
    SENSOR_SVC = BLE_HARDWARE_POINTER->createService(BLEUUID(BT_SENSORS_SERVICE));
    QUATERNION_BRGC =
        SENSOR_SVC->createCharacteristic(BT_QUATERNION_NOTIFY, NIMBLE_PROPERTY::NOTIFY);
    SENSOR_SVC->start();
    Serial.println("after sensorBLsvc");


    // Start advertising
    BLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(BT_DEMO_SERVICE);
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    pAdv->setMaxPreferred(0x12);

    NimBLEDevice::startAdvertising();

    Serial.println("BLE Ready!");
    Serial.print("Name: ");
    Serial.println(name);
    Serial.println("Scan with Serial Bluetooth Terminal app");
}

// !!! called outside this cpp !!!
void BleHelper::sendNotifyTest(const std::string& msg) {
    Serial.println("sendNotifyTest");
    this->DEMO_NOTIFY_REPEAT_BRGC->setValue(msg);
    this->DEMO_NOTIFY_REPEAT_BRGC->notify();
}




