#include "FrSkySportSensor.h"
#include "FrSkySportSensorFcs.h"
// #include "FrSkySportSensorFlvss.h"
#include "FrSkySportSensorGps.h"
#include "FrSkySportSensorRpm.h"
#include "FrSkySportSensorVario.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportTelemetry.h"
#include "SoftwareSerial.h"


FrSkySportSensorFcs fcs;                               // Create FCS sensor with default ID
//FrSkySportSensorFlvss flvss;                          // Create FLVSS sensor with default ID
FrSkySportSensorGps gps;                               // Create GPS sensor with default ID
FrSkySportSensorVario vario;     
FrSkySportTelemetry telemetry;                         // Create Variometer telemetry object
FrSkySportSensorRpm rpm;

bool telemetryRunning = false;

void telemetry_init(void) {
  telemetry.begin(FrSkySportSingleWireSerial::SOFT_SERIAL_PIN_18, &gps, &fcs, &vario, &rpm);
}

void telemetry_start(void) {
    telemetryRunning=true;
}

void telemetry_stop(void) {
  // telemetry.stop();
  telemetryRunning=false;
}

void telemetry_send(void) {
  unsigned long start_time;
  
  /*************************************************************************/
  /* TODO: Check stability! GPS Telemetry seems to stop after 1 minute!!!  */
  /*************************************************************************/

  
  if (telemetryRunning) {
    
    int altitude = osd_alt - osd_home_alt; // calculate Altitude in m (can be nevative)
    
    // start_time = (int) (millis() / 1000);
    fcs.setData(osd_curr_A * .01,   // Current consumption in amps
                  osd_vbat_A);       // Battery voltage in volts
  
    gps.setData(osd_lat, osd_lon,   // Latitude and longitude in degrees decimal (positive for N/E, negative for S/W)
                  altitude,            // Altitude in m (can be nevative)
                  osd_groundspeed,    // Speed in m/s
                  osd_heading /* ,        // Course over ground in degrees
                  0, 0, 0,          // Date (year - 2000, month, day)
                  23, ((int)(start_time/60))%60, start_time%60*/);        // Time (hour, minute, second) - will be affected by timezone setings in your radio
  
    vario.setData(osd_alt,          // Altitude in Meters (can be negative)
                  osd_climb);   
                  
    // Set RPM/temperature sensor data
    // (set number of blades to 2 in telemetry menu to get correct rpm value)
    rpm.setData(osd_throttle,    // Throttle position
              osd_roll,   // roll in degree
              osd_pitch);  // pitch in degree
              
    telemetry.send(); 
  }
}

