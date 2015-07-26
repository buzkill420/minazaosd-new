/*

Copyright (c) 2011.  All rights reserved.
An Open Source Arduino based OSD and Camera Control project.

Program  : ArduCAM-OSD (Supports the variant: minimOSD)
Version  : V1.9, 14 February 2012
Author(s): Sandro Benigno
Coauthor(s):
Jani Hirvinen   (All the EEPROM routines)
Michael Oborne  (OSD Configutator)
Mike Smith      (BetterStream and Fast Serial libraries)
Special Contribuitor:
Andrew Tridgell by all the support on MAVLink
Doug Weibel by his great orientation since the start of this project
Contributors: James Goppert, Max Levine
and all other members of DIY Drones Dev team
Thanks to: Chris Anderson, Jordi Munoz


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>

*/


 /*************************************************************************/
 /* TODO: Check stability! GPS Telemetry seems to stop after 1 minute!!!  */
 /* TODO: Implement BMP183 Barometric SPI-Sensor code                     */
 /*************************************************************************/
  
  

/* ************************************************************ */
/* **************** MAIN PROGRAM - MODULES ******************** */
/* ************************************************************ */

#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 

#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 


/* **********************************************/
/* ***************** INCLUDES *******************/

//#define membug 
//#define FORCEINIT  // You should never use this unless you know what you are doing 


// AVR Includes

#include <PinChangeInt.h>
#include <BetterStream.h>
#include <AP_Common.h>
#include <AP_Math.h>
#include <math.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

// Get the common arduino functions
#include "Arduino.h"
#include <EEPROM.h>
#include <SimpleTimer.h>
#include <GCS_MAVLink.h>

#ifdef membug
#include <MemoryFree.h>
#endif

// Configurations
#include "OSD_Config.h"
#include "ArduCam_Max7456.h"
#include "OSD_Vars.h"
#include "OSD_Func.h"

#include "NAZATalk.h"
#include "NazaLed.h"
#include "NazaInt.h"
#include "FrSky_Telemetry.h"

#if defined (SETUP_VOLT_DIFF_RATIO) || defined (SETUP_AMP_OFFSET) || defined (SETUP_AMP_PER_VOLT)
#include "PWMRead.h"
#endif

#include "FlightBatt.h"
//#include "AnalogRssi.h"



/* *************************************************/
/* ***************** DEFINITIONS *******************/

//OSD Hardware 
#define MinimOSD

#define TELEMETRY_SPEED  115200 // How fast our data is coming to serial port

// Objects and Serial definitions
//FastSerialPort0(Serial);

OSD osd; //OSD object 

SimpleTimer  mavlinkTimer;
unsigned long lastNazaInt = 0;


/* **********************************************/
/* ***************** SETUP() *******************/

void setup() 
{
  
    pinMode(MAX7456_SELECT,  OUTPUT); // OSD CS
    Serial.begin(TELEMETRY_SPEED);
    
#ifdef membug
    Serial.println(freeMem());
#endif
    
    // Prepare OSD for displaying 
    unplugSlaves();
    osd.init();

    // Start 
    startPanels();
    delay(500);

    // OSD debug for development (Shown at start)
#ifdef membug
    osd.setPanel(1,1);
    osd.openPanel();
    osd.printf("%i",freeMem()); 
    osd.closePanel();
#endif

    // Just to easy up development things
#ifdef FORCEINIT
    InitializeOSD();
#endif

    // Check EEPROM to see if we have initialized the battery values already
    if (readEEPROM(BATT_CHK) != BATT_VER) {
	writeBattSettings();
    }

    // Get correct panel settings from EEPROM
    readSettings();
    for(panel = 0; panel < npanels; panel++) readPanelSettings();
    panel = 0; //set panel to 0 to start in the first navigation screen

    flight_batt_init();
    //analog_rssi_init();
    naza_led_init();
    telemetry_init();
        

#if defined (SETUP_VOLT_DIFF_RATIO) || defined (SETUP_AMP_OFFSET) || defined (SETUP_AMP_PER_VOLT)
    pwm_read_init();
#endif

    // Startup MAVLink timers  
    mavlinkTimer.Set(&OnMavlinkTimer, 100);

    // House cleaning, clear display and enable timers
    osd.clear();
    mavlinkTimer.Enable();
    
} // END of setup();



/* ***********************************************/
/* ***************** MAIN LOOP *******************/

// Mother of all happenings, The loop()
// As simple as possible.
void loop() 
{
    if (nazatalk_read()) {
        OnMavlinkTimer();
    } else {
	mavlinkTimer.Run();
    }
    telemetry_send();
}

/* *********************************************** */
/* ******** functions used in main loop() ******** */

void OnMavlinkTimer()			// duration is up to approx. 10ms depending on choosen display features
{
    if (millis() - lastNazaInt >= 100) {// measure pwm values about every 100ms
      telemetry_stop();                 // Stop telemetry to avoid interrupt timing problems
      Serial.end();                     // Stop GPS communication to avoid interrupt timing problems 
      naza_int_init();                  // measure throttle, screenswitch, roll and pitch pwm signals
      Serial.begin(TELEMETRY_SPEED);    // Start GPS communication again
      telemetry_start();                 // Start telemetry again 
      lastNazaInt = millis();
    }

//    baro_read();
    flight_batt_read();
//    analog_rssi_read();
//    rssi = (int16_t) osd_rssi;
//    if (!rssiraw_on) rssi = (int16_t)((float)(rssi - rssipersent)/(float)(rssical-rssipersent)*100.0f);
//    if (rssi < -99) rssi = -99;

//    updateTravelDistance();		// calculate travel distance
    setHeadingPattern();		// generate the heading pattern
    setHomeVars(osd);			// calculate and set Distance from home and Direction to home
    writePanels();			// writing enabled panels (check OSD_Panels Tab)
}

void unplugSlaves(){
    //Unplug list of SPI
    digitalWrite(MAX7456_SELECT,  HIGH); // unplug OSD
}
