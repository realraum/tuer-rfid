/*
 *  spreadspace avr utils
 *
 *
 *  Copyright (C) 2013-2014 Christian Pointner <equinox@spreadspace.org>
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
#include "manual.h"
#include "eventqueue.h"

#define MANUAL_PIN PIND
#define MANUAL_PORT PORTD
#define MANUAL_DDR DDRD
#define MANUAL_BIT 7

#define MANUAL_LP_MAX 255

void manual_init(void)
{
  MANUAL_DDR = MANUAL_DDR & ~(1<<MANUAL_BIT);
  MANUAL_PORT |= (1<<MANUAL_BIT);
}

void manual_task(void)
{
  static uint8_t last_state = (1<<MANUAL_BIT);
  static uint8_t lp_cnt = 0;

  uint8_t state = MANUAL_PIN & (1<<MANUAL_BIT);
  if(state != last_state)
    lp_cnt++;
  else
    lp_cnt += lp_cnt ? -1 : 0;

  if(lp_cnt >= MANUAL_LP_MAX) {
    if(!state)
      eventqueue_push(btn_toggle);
    last_state = state;
    lp_cnt = 0;
  }
}
