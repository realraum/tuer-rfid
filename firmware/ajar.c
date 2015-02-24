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
#include <stdio.h>
#include "ajar.h"

#define AJAR_PIN PINC
#define AJAR_PORT PORTC
#define AJAR_DDR DDRC
#define AJAR_BIT 7

#define AJAR_LP_MAX 20000

void ajar_init(void)
{
  AJAR_DDR = AJAR_DDR & ~(1<<AJAR_BIT);
  AJAR_PORT |= (1<<AJAR_BIT);
}

ajar_t ajar_get(void)
{
  static uint8_t last_state = (1<<AJAR_BIT);
  static uint16_t lp_cnt = 0;

  uint8_t state = AJAR_PIN & (1<<AJAR_BIT);
  if(state != last_state)
    lp_cnt++;
  else
    lp_cnt += lp_cnt ? -1 : 0;

  if(lp_cnt >= AJAR_LP_MAX) {
    last_state = state;
    lp_cnt = 0;
  }

  if(last_state)
    return ajar;
  return shut;
}

const char* ajar_to_string(ajar_t a)
{
  return a == ajar ? "ajar" : "shut";
}

void ajar_task(void)
{
  static ajar_t last_state = shut;

  ajar_t state = ajar_get();
  if(last_state != state)
    printf("Info(ajar): door is now %s\r\n", ajar_to_string(state));

  last_state = state;
}
