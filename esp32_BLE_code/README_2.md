
## README_2.MD: additional notes


# Setting up vscode in ubuntu
- using ubuntu instead of win11 for esp32 because w11 c++ compile is very slow.  
- ubuntu file manager "nautilus" is bad. Install nemo:  
```
sudo apt update
sudo apt install nemo
```
```
# stuff to install for vscode on ubuntu
# first do update/upgrade or it fails later
sudo apt update
sudo apt upgrade -y
# install git
apt-get install git
# after installing platformIO plugin, install python
sudo apt install python3-venv

# Set your global name
git config --global user.name "ubuntu17"
# Set your global email
git config --global user.email "ubuntu17@example.com"
```
## platformIO ubuntu
- when starting vscode with platformIO, for about 10 seconds is asks what project to open -DONT DO ANYTHING! - after a bit it opens the last project by itself. 
- from error in Ubuntu: 
Looking for upload port...Warning! Please install 99-platformio-udev.rules. 
Pasted message to gemini ai, got this answer but needed to modify url.
```
# this gets url into the file using -O argument
sudo apt  install curl
curl -O https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules 
cat 99-platformio-udev.rules 
# apply the file
sudo cp 99-platformio-udev.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo usermod -a -G dialout $USER
# run history and trim off leading command count
history | cut -c 8- > hist_99rules 
```


How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-espressif32/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell
# Change directory to example
$ cd platform-espressif32/examples/arduino-ble5-advertising

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e esp32-c3-devkitm-1

# Upload firmware for the specific environment
$ pio run -e esp32-c3-devkitm-1 --target upload

# Clean build files
$ pio run --target clean
```

### Monitoring interrupt pin on DS212 "toy oscilloscope"
- Not as good as FNRSI scope; mine intermittenly shows big square wave randomly....   
- set timebase; trigger takes more time to fire and produce output the longer the timebase: 20mS-immediate; .1Sec-3 Sec; .2Sec-6 Sec; 50mS-1 Sec.    
  (per grating division on screen). 
- at this high speed, impossible for repeating interrupts to appear periodically on screen. Instead, do this way:  
  - Using X_Window to view whole sweep; it only works on "single" trigger mode.  
  - set trigger to "single", then trigger sweep by pressing button on top ">||", the "stop" icon appears, meaning the sweep is acquired.  
  - Then to go "X_Window" menu and set "enable on" and "Depth 2k", then move "Post" to up and the whole sweep will scroll across screen with invisible data coming into view. Note that trigger for pulse is middle of screen when starting this.  
  Don't forget to set "Post" back to zero or next sweep will have pulse scrolled out of view. 
  
