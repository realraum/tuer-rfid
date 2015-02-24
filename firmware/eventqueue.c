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

#include "eventqueue.h"
#include <LUFA/Drivers/Misc/RingBuffer.h>

static RingBuffer_t event_queue;
static uint8_t event_queue_data[16];

void eventqueue_init(void)
{
  RingBuffer_InitBuffer(&event_queue, event_queue_data, sizeof(event_queue_data));
}

event_t eventqueue_pop(void)
{
  if (RingBuffer_IsEmpty(&event_queue))
    return none;
  return RingBuffer_Remove(&event_queue);
}

void eventqueue_push(event_t event)
{
  RingBuffer_Insert(&event_queue,event);
}

const char* event_to_string(event_t event)
{
  switch(event) {
    case none: return "none";
    case cmd_open: return "cmd_open";
    case cmd_close: return "cmd_close";
    case cmd_toggle: return "cmd_toggle";
    case btn_toggle: return "btn_toggle";
    case card: return "card";
    case open_fin: return "open_fin";
    case close_fin: return "close_fin";
    case move_timeout: return "move_timeout";
  }
  return "invalid"; // gcc - shut the fuck up!!!
}
