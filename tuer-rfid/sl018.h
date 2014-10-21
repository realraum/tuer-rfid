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

#ifndef R3TUER_sl018_h_INCLUDED
#define R3TUER_sl018_h_INCLUDED

#include <stdint.h>

typedef struct {
  uint8_t length;
  unsigned char * buffer;
} uid_t;

void sl018_set_led(uint8_t on);
uint8_t sl018_check_for_new_card(void);
void sl018_read_card_uid(uid_t * uid);
uint8_t sl018_reset(void);
unsigned char * sl018_get_firmware_version(void);

#endif
