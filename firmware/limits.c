/*
 *  tuer-rfid
 *
 *
 *  Copyright (C) 2013-2014 Christian Pointner <equinox@spreadspace.org>
 *                          Othmar Gsenger <otti@wirdorange.org>
 *
 *  This file is part of tuer-rfid.
 *
 *  tuer-rfid is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  tuer-rfid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with tuer-rfid. If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include "limits.h"
#include <LUFA/Drivers/Peripheral/ADC.h>

#define LIMITS_ADC_CHAN_NUM 8
#define LIMITS_ADC_CHAN ADC_CHANNEL8

#define LIMITS_RINGBUF_SIZE 4
/* HINT: this is compared to a sliding sum not an average! */
#define LIMITS_TH_CLOSE 275 * LIMITS_RINGBUF_SIZE
#define LIMITS_TH_OPEN  600 * LIMITS_RINGBUF_SIZE

void limits_init(void)
{
  ADC_Init(ADC_FREE_RUNNING | ADC_PRESCALE_128);
  ADC_SetupChannel(LIMITS_ADC_CHAN_NUM);
  ADC_StartReading(ADC_REFERENCE_AVCC | ADC_RIGHT_ADJUSTED | LIMITS_ADC_CHAN);
  ADCSRA |= (1 << ADIE);
}

// sum must not be used directly
static uint16_t sum = 0;

// these variables must not be used by anything outside of the ISR
static uint16_t r[LIMITS_RINGBUF_SIZE] = { 0 };
static uint8_t idx = 0;
ISR(ADC_vect)
{
  r[idx] = ADC;
  idx = (idx + 1) % LIMITS_RINGBUF_SIZE;
  sum = 0;
  uint8_t i;
      // a sliding sum might be faster but the size of the ringbuffer is low and
      // it is safer to always compute the sum from scratch
  for(i=0; i<LIMITS_RINGBUF_SIZE; ++i) sum += r[i];
}

inline uint16_t limits_get_raw(void)
{
  cli();
  uint16_t s = sum;
  sei();

  return s;
}

limits_t limits_get(void)
{
  uint16_t s = limits_get_raw();

  if(s < LIMITS_TH_CLOSE)
    return close;

  if(s < LIMITS_TH_OPEN)
   return moving;

  return open;
}

const char* limits_to_string(limits_t limits)
{
  switch(limits) {
    case moving: return "...";
    case open: return "opened";
    case close: return "closed";
    case both: return "error";
  }
  return "invalid"; // gcc - shut the fuck up!!!
}
