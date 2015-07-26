/**
 ******************************************************************************
 *
 * @file       NAZATalk.ino
 * @author     Joerg-D. Rothfuchs
 * @brief      Implements a subset of NAZA communication.
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/> or write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "NAZATalk.h"

// see http://www.rcgroups.com/forums/showthread.php?t=1995704
#include "GPS_DJI.h"


//----------------------------------------------------------------------------------
//
//  stage 1	easy soldering		solder the GPS puck cable and configure 1 cable to NAZA for +5V and GND
//	GPS lat/lon				ok
//	distance to home			ok
//	altitude				ok
//	groundspeed				ok
//	course					ok
//	direction to home			ok
//	sat count				ok
//	sat fix					ok
//	climb rate				ok
//	radar					ok
//	time display				ok
//
//
//
//  stage 2a	advanced soldering	solder 1 cable at Atmel 328P pin and 3 resistors at NAZA LED cable adapter
//	active LED symbol			ok	3 resistors + 1 ADC
//
//  stage 2b	advanced soldering	solder 2 cables at Atmel 328P pin and up to 4 resistors and use voltage/current sensor
//	battery voltage				ok	2 resistors + 1 ADC
//	battery current				ok	2 resistors + 1 ADC
//
//  stage 2c	advanced soldering	solder 1 cable at Atmel 328P pin and 2 resistors
//	analog RSSI				ok	2 resistors + 1 ADC
//
//  stage 2d	easy soldering		solder 1 cable at MinimOSD PAL pad (but only for interest for users of Simprop GigaScan RC-RX)
//	packet RSSI				ok	1 INT
//
//
//
//  stage 3a	advanced soldering	solder 2 cables at Atmel 328P pin and configure 1 double PWM cable to NAZA
//	horizon + pitch/roll			TOTEST	2 PCINT
//
//  stage 3b	advanced soldering	solder 2 cables at Atmel 328P pin and configure 2 PWM cables to RC-RX
//	throttle				TOTEST	1 PCINT
//	screen switching			TOTEST	1 PCINT
//
//----------------------------------------------------------------------------------
int nazatalk_read(void) {
	static uint8_t crlf_count = 0;
	int ret = 0;
	
	// TODO implement some of the following
	// fake the info till available
	if (!osd_armed) {
		osd_armed	= 2;
		osd_mode	= 0;
		
		osd_roll	= (int16_t) 0;
		osd_pitch	= (int16_t) 0;
		osd_throttle	= 0;
		ch_toggle	= 6;
		osd_chan5_raw	= 1100;
		osd_chan6_raw	= 1100;
		osd_chan7_raw	= 1100;
		osd_chan8_raw	= 1100;
	}
	
	// grabbing data
	while (Serial.available() > 0) {
		uint8_t c = Serial.read();

		naza_led_show(NAZA_LED_POS_X, NAZA_LED_POS_Y);
		
		// needed for MinimOSD char set upload
		if (!osd_got_home && millis() < 20000 && millis() > 5000) {
			if (c == '\n' || c == '\r') {
				crlf_count++;
			} else {
				crlf_count = 0;
			}
			if (crlf_count == 3) {
				uploadFont();
			}
		}
		
		if (parse_dji(c) == PARSER_COMPLETE_SET) {
			osd_fix_type		= get_dji_status();
			osd_satellites_visible	= get_dji_satellites();
			osd_lat			= get_dji_latitude();
			osd_lon			= get_dji_longitude();
			osd_alt			= get_dji_altitude();
			osd_heading 		= get_dji_heading();
			osd_groundspeed 	= get_dji_groundspeed();
			osd_climb		= -1.0 * get_dji_down();
		}
	}
	
	naza_led_show(NAZA_LED_POS_X, NAZA_LED_POS_Y);
	
	ch_toggle	= 6;
	osd_chan6_raw	= naza_screenswitch_get();
	
        return ret;
}


int nazatalk_state(void)
{
	return 1;
}



