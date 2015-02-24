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

#ifndef R3TUER_eventqueue_h_INCLUDED
#define R3TUER_eventqueue_h_INCLUDED

typedef enum { none, cmd_open, cmd_close, cmd_toggle, btn_toggle, card, close_fin, open_fin, move_timeout } event_t;

void eventqueue_init(void);
event_t eventqueue_pop(void);
void eventqueue_push(event_t);
const char* event_to_string(event_t event);

#endif
