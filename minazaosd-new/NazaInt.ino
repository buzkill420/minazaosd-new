/**
 ******************************************************************************
 *
 * @file       NazaInt.ino
 * @author     Joerg-D. Rothfuchs
 * @brief      Implements RC-RX channel detection and NAZA artificial horizon 
 *             using interrupts.
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

#include <PinChangeInt.h>
#include "NazaInt.h"
#include "pins_arduino.h"

volatile long start_time;				// start time
volatile long pulse_time		= 1500;		// pulse duration
long screenswitch_pulse;

volatile bool finished = false;
volatile bool triggered = false;             


void int_pulse(void) {
	if (PCintPort::pinState){            
		start_time = micros();				// positive edge: start
                triggered = true;
	} else {
                if (triggered) {
      		  pulse_time = micros() - start_time;		// negative edge: calculate pulsewidth
                  finished = true;
                } 
                triggered = false;
       }
}

void naza_int_init(void)
{
        triggered=false;    
        finished=false;    
	pinMode(PWM_PIN_THROTTLE, INPUT);
	digitalWrite(PWM_PIN_THROTTLE, HIGH);				// turn on pullup resistor
        PCintPort::attachInterrupt(PWM_PIN_THROTTLE, &int_pulse, CHANGE);
        while (!finished) {}
        PCintPort::detachInterrupt(PWM_PIN_THROTTLE);
        osd_throttle = constrain((int16_t)((float)(pulse_time - THROTTLE_LOWEST) / (float)(THROTTLE_HIGHEST - THROTTLE_LOWEST) * 100.0 + 0.5), 0, 100);
        
        triggered=false;
        finished=false;
	pinMode(PWM_PIN_SCREENSWITCH, INPUT);
	digitalWrite(PWM_PIN_SCREENSWITCH, HIGH);			// turn on pullup resistor
	PCintPort::attachInterrupt(PWM_PIN_SCREENSWITCH, &int_pulse, CHANGE);
        while (!finished) {}
        PCintPort::detachInterrupt(PWM_PIN_SCREENSWITCH);
        screenswitch_pulse = pulse_time;

        triggered=false;
        finished=false;
	pinMode(PWM_PIN_GIMBAL_F1, INPUT);
	digitalWrite(PWM_PIN_GIMBAL_F1, HIGH);				// turn on pullup resistor
	PCintPort::attachInterrupt(PWM_PIN_GIMBAL_F1, &int_pulse, CHANGE);
        while (!finished) {}
        PCintPort::detachInterrupt(PWM_PIN_GIMBAL_F1);
        osd_roll = (int16_t) (((int16_t) pulse_time - GIMBAL_ROLL_MIDDLE) * GIMBAL_ROLL_FACTOR);
        
        triggered=false;
        finished=false;        
	pinMode(PWM_PIN_GIMBAL_F2, INPUT);
	digitalWrite(PWM_PIN_GIMBAL_F2, HIGH);				// turn on pullup resistor
	PCintPort::attachInterrupt(PWM_PIN_GIMBAL_F2, &int_pulse, CHANGE);
        while (!finished) {}
        PCintPort::detachInterrupt(PWM_PIN_GIMBAL_F2);
        osd_pitch = (int16_t) (((int16_t) pulse_time - GIMBAL_PITCH_MIDDLE) * GIMBAL_PITCH_FACTOR);
}

// screenswitch in micro seconds
int16_t naza_screenswitch_get(void)
{
	return (int16_t) screenswitch_pulse;
}

#if defined (SETUP_TS)

// throttle  in micro seconds
int16_t naza_throttle_us_get(void)
{
	return (int16_t) throttle_pulse;
}

#endif // NAZA_INT_DEBUG
