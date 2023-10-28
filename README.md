# apds9960_web_eval

Having an APDS9960 connected to an ESP8266 which provides a web-interface to enable/disable all the ADSP-settings and shows output of the APDS9960.
---------------
I wrote this in order to test various settings of the ADPS9960
Changes can be submitted and sensor output is polled/displayed.
Note: While "playing" with various the setups the sensor seems to become "unstable"
So, it is a good idea to make a reset (disconnetc ESP from Power) if you discover unreliable output in such situations.


Used Library: https://github.com/VArt67/APDS-9960_Gesture_Sensor_esp8266_Library

I copied the cpp/h files of that library into this repo

Web Interface
---------------
![grafik](https://github.com/VArt67/apds9960_web_eval/assets/132200455/9450ce2a-c1a7-4571-a9a4-a15f6f61adfa)
