/*
 *  spreadspace avr utils
 *
 *
 *  Copyright (C) 2013 Christian Pointner <equinox@spreadspace.org>
 *                     Othmar Gsenger <otti@wirdorange.org>
 *
 *  This file is part of spreadspace avr utils.
 *
 *  spreadspace avr utils is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  spreadspace avr utils is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with spreadspace avr utils. If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

#include "led.h"
#include "heartbeat.h"

#define HEARTBEAT_DURATION 10 // *10 ms, duration of heartbeat pulse
#define HEARTBEAT_DELAY 200   // *10 ms, 1/heartbeat-frequency
uint8_t heartbeat_cnt = 0;                    
uint8_t heartbeat_flag;

#define FASTBEAT_PORT PORTD
#define FASTBEAT_DDR DDRD
#define FASTBEAT_BIT 5

// while running this gets called every ~10ms
ISR(TIMER0_COMPA_vect)
{
  heartbeat_cnt++;
  if(heartbeat_cnt == HEARTBEAT_DURATION)
    heartbeat_flag = 0;
  else if(heartbeat_cnt >= HEARTBEAT_DELAY) {
    heartbeat_flag = 1;
    heartbeat_cnt = 0;
  }
}

void heartbeat_init(void)
{
  led_off();
  heartbeat_cnt = 0;
  heartbeat_flag = 1;

  TCCR0A = 1<<WGM01;           // OC0A and OC0B as normal output, WGM = 2 (CTC)
  TCCR0B = 1<<CS02 | 1<<CS00;  // Prescaler 1:1024
  OCR0A = 155;                 // (1+155)*1024 = 159744 -> ~10 ms @ 16 MHz
  TCNT2 = 0;
  TIMSK0 = 1<<OCIE0A;

  FASTBEAT_DDR |= 1<<FASTBEAT_BIT;
}

void heartbeat_task(void)
{
  FASTBEAT_PORT ^= 1<<FASTBEAT_BIT;

  if(heartbeat_flag)
    led_on();
  else
    led_off();
}
