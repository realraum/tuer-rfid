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

#ifndef R3TUER_keystore_h_INCLUDED
#define R3TUER_keystore_h_INCLUDED

#include <stdint.h>

void keystore_flash_from_stdio(void);
void keystore_dump_to_stdio(void);
uint8_t keystore_check_card(const uint8_t * uid, uint8_t uid_len);

#endif
