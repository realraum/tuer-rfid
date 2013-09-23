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

#include <avr/io.h>
#include "limits.h"
#include <LUFA/Drivers/Peripheral/ADC.h>

#define LIMITS_ADC_CHAN_NUM 8
#define LIMITS_ADC_CHAN ADC_CHANNEL8

#define LIMITS_RINGBUF_SIZE 5
/* HINT: this is compared to a sliding sum not an average! */
#define LIMITS_TH_CLOSE 250
#define LIMITS_TH_OPEN 1000

void limits_init(void)
{
  ADC_Init(ADC_FREE_RUNNING | ADC_PRESCALE_128);
  ADC_SetupChannel(LIMITS_ADC_CHAN_NUM);
  ADC_StartReading(ADC_REFERENCE_INT2560MV | ADC_LEFT_ADJUSTED | LIMITS_ADC_CHAN);
}

limits_t limits_get(void)
{
  static uint8_t s = 0;
  static uint8_t r[LIMITS_RINGBUF_SIZE] = { 0 };
  static uint8_t idx = 0;

  if(ADC_IsReadingComplete()) {
    r[idx] = (ADC_GetResult()>>8);
    idx = (idx + 1) % LIMITS_RINGBUF_SIZE;
    s = 0;
    uint8_t i;
    for(i=0; i<LIMITS_RINGBUF_SIZE; ++i) s += r[i];
  }

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
