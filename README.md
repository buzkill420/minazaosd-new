# minazaosd-new

MinimOSD project based on minazaosd.
Implementing the FrSky telemetry library and the Adafruit BMP183 SPI-barometric sensor library.


This code is higly unstable! Use at your own risk!

Everything is subject to change at the moment. I'll change the pinning (to reduce pin-change-interrupt usage) as soon as I've got my new MinimOSD and there will be also a new soldering manual. 

Mandatory requirements:

* Arduino-IDE 1.6.3 (or higher, not tested)
* DJI Naza lite or Naza V2 Flightcontroller
* FrSky receiver with S.Port connector (X8R, X6R, X4R)
* FrSky Taranis X9D radio 
* a MinimOSD board
* a sharp-tiped soldering iron
* thin wire, at best insulated & pre-tinned "Bodge Wire"
* some decent soldering experience 

Optional:

* Voltage/Current Sensor (tested with TBS 50A sensor)
* Adafruit BMP183 barometric pressure/altitude sensor
