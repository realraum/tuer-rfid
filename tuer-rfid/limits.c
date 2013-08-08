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

#define LIMITS_PIN PINC
#define LIMITS_PORT PORTC
#define LIMITS_DDR DDRC
#define LIMITS_OPEN 6
#define LIMITS_CLOSE 7

#define LIMITS_LP_MAX 255

void limits_init(void)
{
  LIMITS_DDR = LIMITS_DDR & ~(1<<LIMITS_OPEN | 1<<LIMITS_CLOSE);
  LIMITS_PORT |= (1<<LIMITS_OPEN | 1<<LIMITS_CLOSE);
}

uint8_t limits_get_close(uint8_t pin)
{
  static uint8_t last_state = 1<<LIMITS_CLOSE;
  static uint8_t lp_cnt = 0;

  uint8_t state = pin & (1<<LIMITS_CLOSE);
  if(state != last_state)
    lp_cnt++;
  else
    lp_cnt += lp_cnt ? -1 : 0;

  if(lp_cnt >= LIMITS_LP_MAX) {
    last_state = state;
    lp_cnt = 0;
  }

  return last_state;
}

uint8_t limits_get_open(uint8_t pin)
{
  static uint8_t last_state = 1<<LIMITS_OPEN;
  static uint8_t lp_cnt = 0;

  uint8_t state = pin & (1<<LIMITS_OPEN);
  if(state != last_state)
    lp_cnt++;
  else
    lp_cnt += lp_cnt ? -1 : 0;

  if(lp_cnt >= LIMITS_LP_MAX) {
    last_state = state;
    lp_cnt = 0;
  }

  return last_state;
}

limits_t limits_get(void)
{
  uint8_t tmp = LIMITS_PIN & (1<<LIMITS_OPEN | 1<<LIMITS_CLOSE);
  if(!limits_get_open(tmp)) {
    if(limits_get_close(tmp))
      return open;
    else
      return both;
  }
  else if(!limits_get_close(tmp))
    return close;

  return moving;
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
