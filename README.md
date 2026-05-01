## i2cQuaternion
This project uses an esp32s3 to obtain continuous quaternion output from an i2c-connected navigation chip.
- It uses the **TDK ICM-20948 chip**, a gyro/accelerometer/magnetometer featuring DMP [**D**igital **M**otion **P**rocessor]. The **on-chip DMP** combines and integrates sensor data over time to provide a direct **Quaternion** output over **i2c** at regular intervals. It raises the chip's INT pin when it has **new data**, typically many times per second.   
- The process done by the **DMP** is referred to as **sensor fusion**.   
- see https://product.tdk.com/en/search/sensor/mortion-inertial/imu/info?part_no=ICM-20948 for the vendor's description. 
Typically purchased by hobbyists on a "breakout board" with support hardware (power supply, level shifters, solder pads etc).   
**Note:** I have had bad luck with no-name breakout boards; a brand name board is suggested.  
- Currently does not address calibration of the navigation device, essential for proper use.   
#### More about **quaternion:** 
- it's a set of 4 float values. Easily converted to roll/pitch.yaw **"euler angles"** "oiler" for display or troubleshooting. Popular 3D libraries such as Three.js and OpenGL take quaternions as a parameter for orienting models, lights or cameras. 
- **Gimbal Lock:** Quaternions are less susceptible to **Gimbal Lock** then euler angles. Example: your flight sim points vertically and starts flipping back and forth 180degrees. Also refer to the movie "Apollo 13" for a dramatic example.    
  Quaternions use matrices and imaginary numbers and a stone bridge in Ireland is named after it. That's about how far my understanding goes. 

### Objective 
  - **Sailing Drone** is a tentative goal for this project to make it easier to for me to visualize.  
For the forseeable future, an actual sailing drone is not happening, although it sounds like fun. 

### How this system works  
- First there is a clock driving updates done by the DMP on the chip.   
It's timing is set by ```setDMPODRrate()```.   
When data is ready (at regular intervals set above) the INT (Interrupt) pin goes high.
  This typically be used as the app clock, driving graphics updates, rudder corrections and more.  
- Handling hardware interrupts: arduino code senses the physical interrupt pin (raised by the **DMP**) to trigger an **Interrupt Service Routine (ISR)**. [a function]
  - we can read the sensor and handle the data now, **but we don't** because the processor is running the ship and **can't be interrupted**, such as responding instantly to user input or updating graphics or avoiding a reef.
- instead, **freeRTOS** saves the day:
  - "**R**eal **T**ime **O**perating **S**ystem" is builtin to the esp32 and enables **tasks** (aka **threads**), and **semaphores**. Threads are like sailors carrying out shipboard tasks and semaphores are like spoken orders to a sailor by name, shouted for all the crew to hear.     
  *[threads, semaphores and mutex way back in the 80's at UW-Madison without the boat stuff]*
  - The ISR works in 2 steps:  
    - a **worker task** [sailor] running in an endless loop, which **STOPS AND WAITS** 99% of the time. It waits for a freeRTOS **task notification**, a form of semaphore [captain calls the sailor's name]. This task does not block anything because it's in its own thread.    
    - When an interrupt happens, "boatswain's whistle is blown", captain hears it and runs the **ISR**. In this routine, the captain sends a freeRTOS **task notification** [calls out sailor's name] to the **worker task** then immediately returns to running the ship because she can't be distracted by one task.  There is only one INT pin and **ISR** per system, so that's analogous to the job of the captain. 
    - Now the **worker task** takes it time to query the DMP over i2c for current data and act on it, including moving servos etc. When done, it goes back to the blocked state, waiting again. 
- **UH OH!** 2 threads access i2c === Crash Computer === Flying Dutchman  
  Yes, the esp32 crashes when this happens.  
  - This is where openRTOS **semaphores** used as **mutex** [mutual exclusion] come into play.   
  - The app sets up 2 **SemaphoreHandle_t** objects, one for the **i2c bus**.
    This are passed to all objects used by the main app.   
    This guarantee one-at-a-time access. [think the conch shell in the shipwreck movie]  
    ```xSemaphoreTake(xSemaphore, blockTime)``` will **block** until xSemaphore becomes available.  
    ```xSemaphoreGive(xSemaphore)``` must be called to   
  **release the semaphore**.    
  If not done the system locks up and sailor gets an Albatross to wear.  
   *try-catch exception handler is needed to release the semaphore if runtime error prevents its release.*
  - I may be wrong but BLE **B**luetooth **L**ow **E**nergy takes care of its own concurrency.  

### Libraries and Helper Classes
- Look in platformio.ini ship's manifest to see the Sparkfun library used to access the chip. 
- I added helper functions, organized as those tied to specific hardware and universal use:  
Structure of /lib:   
```
    lib/myLib
        |     \
     helpers   hdwreHelpers
       /  \             |
  i2chelper.cpp    TDK_dmp_helper.cpp
  Mathhelper.cpp   [interrupt & semaphore here]
  and more
```

### C++ vs Java, Typescript  
- The esp32 code here uses the ancient mariner's C++ language.
- C++'s  distictive **pointers** and **address-of** operators aka sharks are versatile and dangerous but provide it with specific control over hardware needed for this system.  They reference locations in physical memory, unheard of in Java. 
### My experience with AI  
- Use AI to help learn C++. Ask it the right questions and it will help sort out confusion about C++.  Best to learn from a book and use AI as a coach.  
It makes great short coding examples, but tends to **bury simple code beneath layers of indirection** and **wrappers it invents**. Try to use prompts to avoid this.  
- AI was terrible at writing code to set the chip's registers (it will look up reference tables, but the wrong ones), and insists its code is great, even after "correcting" problems you point out.  
- AI wrote functioning BLE code by ignoring the BLE framework and inventing its own send/receive protocol on top of the real protocol. I followed its example until I realized nothing on the internet used that "protocol". 

### Monitoring i2c and interrupt using Oscilloscope 
- This is not a necessary or typical part of a project of this type --but is fun to do. 
- My first breakout board was a no-name board which crashed the i2c bus now and then and would intermittently provide interrupts.   
I used it as an excuse to buy a 2-trace oscilloscope to analyze the i2c clock and data lines.   
I later got a "brand name" board which works perfectly.


I use this oscilloscope: FNRSI 2T53T 
- Trigger level: "trigger" button won't adust it, use "select" button instead as follows:   

  ![Screenshot 1](pics/OscilloscopeTrigger.jpg)  
- Effect of pullup resistor on i2c. Doesn't seem to be necessary:  

  ![Screenshot 2](pics/WithPullupResistor.jpg)&nbsp;&nbsp;&nbsp;![Screenshot 2](pics/NoPullupResistor.jpg)   


## VSCode settings:  
Here are settings I find useful. "sameLine" tells the code-format feature not to put newline after open brace (takes too much space)  
I have these here for when I setup a different computer and have forgotten all the settings...    
```
# settings.json in .vscode for project 
# or for user: (?)  ~/.config/Code/User/settings.json
{
    "C_Cpp.formatting": "vcFormat",
    "C_Cpp.vcFormat.newLine.beforeOpenBrace.function": "sameLine"
}
```
## VSCode notes about platformIO project folder vs GitHub location: 
* In windows disable "hide hidden folders" in file manager to you can **see** the **.git folders**.  
* During development, I sometimes have many independent unrelated projects in subfolders, all inside one GitHub repo. I do that so everything can be backed up in one place.    
* in vscode platformIO (for esp32, not reactjs), need to point platformIO to one of the projects, using filesystem location. Note that this is **independent** from the GitHub repo project location.
* After copying an existing project into a github repo, be sure to **delete the .git folder**. 

