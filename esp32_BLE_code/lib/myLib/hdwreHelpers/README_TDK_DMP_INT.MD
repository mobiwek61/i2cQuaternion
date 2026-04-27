### Notes on using hardware interrupt for TDK ICM-20948 imu with dmp (digital motion processor)
- it's builtin dmp lets it output quaternions with sensor fusion between gyro, accell, magnet already done  
- it gets its update rate set for dmp computations in setup; get values by periodically polling from caller.  
- it has interrupt pin which tested ok on esp32s3, BUT it triggers at very high unsettable rate (raw sensor data) based on its internal clock. 
  - setup Interrupt Service Routine worked ok.  
  - could turn off high-rate interrupts but unable to setup DMP interrupt.  
  - problem is that setting registers for DMP interrupt did not change value of register and never found why or how to fix it.

#### from pdf of icm20948 docs:  
       INT_ENABLE  addr 16 (10h) bank 0
       reset val 0x00    DMP_INT1_EN bit 1 (of 0-7)
       1-enable dmp interrupt to propagate to interrupt pin 1
       0-disabled
    
## Highlights of code  
#### Last attempt at setting up interrupts  
- file: lib\myLib\hdwreHelpers\skipCompile\TDK_dmp_helper.cpp-int-worked-once  
  This code worked properly a few times even when reboot/reconnect, but next time I tried it failed.  
    - tried delay() to check for race conditions but no luck.
#### Setting callback 
```
.h file:
class TDK_dmp_helper {
public:
    using CallbackType = std::function<void(std::string)>;
    ... TDK_dmp_helper(TwoWire &wirePort, CallbackType dataRdyCallback);
    ... CallbackType _dataRdyCallback;

.cpp file: 
TDK_dmp_helper::TDK_dmp_helper(TwoWire& wirePort, CallbackType dataRdyCallback)
    : _wire(wirePort), _dataRdyCallback(dataRdyCallback) {}
... 
volatile int interruptCt = 0;
// WARNING!!! never use Serial.printf inside an ISR. 
// Must be IRAM_ATTR and must take a void* pointer
void IRAM_ATTR dmpIntHandler(void* arg) {
    interruptCt++;
...
_myICM.setBank(0);
// set interrupt to latch, not pulse
uint8_t pinCfg = 0x30; // pulse // 0x30; cant figure out howto reset latch...
if (_myICM.write(0x10, &pinCfg, 1) != ICM_20948_Stat_Ok)
    Serial.println("DMP failed: INT_PIN_CFG");
delay(5);
// disable raw sensor interrupts:
uint8_t masterEn = 0x00; //0x01
if (_myICM.write(0x11, &masterEn, 1) != ICM_20948_Stat_Ok)
    Serial.println("DMP failed: Master INT_ENABLE");
delay(5);
// Enable DMP interrupt routing
_myICM.write(0x12, (uint8_t)0x08);   // INT_ENABLE_2 = 0x08 (DMP_INT1_EN)
delay(10);
...
pinMode(4, INPUT_PULLDOWN);
// this calls handler when interrupt
attachInterruptArg(digitalPinToInterrupt(4), dmpIntHandler, &_myICM, RISING);
...
```
### set bit while keeping old values:
```
// setBitSaveOld(_myICM, 0x11, 0x00, 1, "INT_ENABLE_1");
void setBitSaveOld(ICM_20948_I2C icm, uint8_t reg, uint8_t bitToSet, int val, std::string desc) {
    Serial.printf("\nsetting reg %02X bit %02X to %02X %s\n", reg, bitToSet, val, desc.c_str());
    uint8_t rwVal; uint8_t _bitToSet = bitToSet;
    if (icm.read(reg, &rwVal, 1) == ICM_20948_Stat_Ok) {
        Serial.printf("  reg %02X old val:%02X \n", reg, rwVal);
        if (val) { rwVal |= (1 << bitToSet); }  // set bit
        else {rwVal &= ~(1 << bitToSet); } // clear bit
        Serial.printf("  reg %02X new val:%02X\n", reg, rwVal);
        icm.write(reg, &rwVal, 1); // 4. Write it back to wake the chip
        delay(20);
    } else { Serial.println("cant read register"); }
}
```