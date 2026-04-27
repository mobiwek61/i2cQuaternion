readme.md todo:
- add ble 
- overview of functionality
  - describe ICM_20948: DMP for quaternion/sensor fusion, flag in compile for firmware upload.
- describe/justify use of RTOS
  - threads, semaphores, global mutex objects
- describe events driven by interrupt, worker thread ISR, callback to main app. 
  need for mutex.
- startup sequence ICM_20948
- notes about cpp for java users


uses sensor fusion to bring together mag, gryo/acc
from separate devices.
Now trying with imu with mag and Digital Motion Processor DMP

Now trying to use DMP, as a guide:  
https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary
