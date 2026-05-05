
### Code for demonstrating the library of this project. 

#### main.cpp:  
this arduino app is just a switcher to choose the example to run, without needing to rename and move source files to switch examples.
- set variable "ArduinoRunnable* myRunnable" to the desired example, and setup() and loop() will be called for it.  

#### TDK_dmp_demo:   
this demonstrates all features of the library. It outputs the current yaw/pitch/roll to the Serial monitor window.    

#### starter_demo:
just an empty framework to serve as a starting point for a new app.