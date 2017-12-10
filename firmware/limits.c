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

#define LIMITS_RINGBUF_SIZE 5
/* HINT: this is compared to a sliding sum not an average! */
#define LIMITS_TH_CLOSE 1200
#define LIMITS_TH_OPEN 2300

void limits_init(void)
{
  ADC_Init(ADC_FREE_RUNNING | ADC_PRESCALE_64);
  ADC_SetupChannel(LIMITS_ADC_CHAN_NUM);
  ADC_StartReading(ADC_REFERENCE_AVCC | ADC_RIGHT_ADJUSTED | LIMITS_ADC_CHAN);
}

static uint16_t sum = 0;

void limits_task(void)
{
  static uint16_t r[LIMITS_RINGBUF_SIZE] = { 0 };
  static uint8_t idx = 0;

  if(ADC_IsReadingComplete()) {
    r[idx] = ADC_GetResult();
    idx = (idx + 1) % LIMITS_RINGBUF_SIZE;
    cli();
    sum = 0;
    uint8_t i;
    for(i=0; i<LIMITS_RINGBUF_SIZE; ++i) sum += r[i];
    sei();
  }
}

limits_t limits_get(void)
{
  if(sum < LIMITS_TH_CLOSE)
    return close;

  if(sum < LIMITS_TH_OPEN)
   return moving;

  return open;
}

uint16_t limits_get_raw_for_tuning(void)
{
  return sum;
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
