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


#include "statemachine.h"
#include "stepper.h"
#include "eventqueue.h"
#include "limits.h"
#include "ledmatrix.h"
#include <stdio.h>

typedef enum {reset, closed, closing, opened, opening, timeout_after_open, timeout_after_close, error, manual_movement} state_t;
state_t state = reset;

const char* state_to_string(state_t s)
{
  switch(s) {
    case reset: return "reset";
    case error: return "error";
    case closing: return "closing";
    case opening: return "opening";
    case manual_movement: return "manual_movement";
    case timeout_after_open: return "timeout_after_open";
    case timeout_after_close: return "timeout_after_close";
    case closed: return "closed";
    case opened: return "opened";
  }
  return "invalid"; // gcc - shut the fuck up!!!
}

void change_state(state_t new_state)
{
  if (new_state == state)
    return;
  printf("State: %s\r\n", state_to_string(new_state));
  switch(new_state) {
    case reset:
      break;
    case closed: ledmatrix_set(red); break;
    case closing:
      ledmatrix_set(red_moving);
      stepper_start(dir_close);
      break;
    case opened: ledmatrix_set(green); break;
    case opening:
      ledmatrix_set(green_moving);
      stepper_start(dir_open);
      break;
    case timeout_after_open: ledmatrix_set(green_blink); break;
    case timeout_after_close: ledmatrix_set(red_blink); break;
    case error: ledmatrix_set(rg_blink); break;
    case manual_movement: ledmatrix_set(rg_moving); break;
      break;
  }
  state = new_state;
}

void statemachine_task_limits(void)
{
  limits_t limits = limits_get();
  if (limits == both)
    return change_state(error);

  switch(state) {
    case reset:
      switch(limits) {
        case open:
          return change_state(opened);
        case close:
          return change_state(closed);
        default:
          return change_state(closing);
      }
    case error:
    case closed:
    case opened:
      switch(limits) {
        case open:
          return change_state(opened);
        case close:
          return change_state(closed);
        default:
          return change_state(manual_movement);
      }
    case manual_movement:
    case timeout_after_open:
    case timeout_after_close:
      switch(limits) {
        case open:
          return change_state(opened);
        case close:
          return change_state(closed);
        default:
          return;
      }
    case closing:
      break;
    case opening:
      break;
  }
}

void statemachine_task_event(void)
{
  event_t event = eventqueue_pop();
  if(event == none)
    return;

  switch(state) {
    case closing:
    case opening:
      switch(event) {
        case open_fin:
          return change_state(opened);
        case close_fin:
          return change_state(closed);
        case move_timeout:
          return change_state(state==opening?timeout_after_open:timeout_after_close);
        default:
          printf("Error(state): event %s not allowed in state %s\r\n", event_to_string(event), state_to_string(state));
          return;
      }
    case reset:
    case error:
      printf("Error(state): Not accepting commands in state %s\r\n", state_to_string(state));
      break; // Not accepting commands
    case manual_movement:
    case timeout_after_open:
    case timeout_after_close:
    case closed:
    case opened:
      switch(event) {
        case none:
          return;
        case cmd_open:
          return change_state(opening);
        case cmd_close:
          return change_state(closing);
        case cmd_toggle:
        case btn_toggle:
        case card:
          return change_state(
            (state==closed || state == timeout_after_close) ?
              opening:
              closing);
        case open_fin:
        case close_fin:
        case move_timeout:
          printf("Error(state): event %s not allowed in state %s\r\n", event_to_string(event), state_to_string(state));
          return;
      }
  }

}

void statemachine_task(void)
{
  statemachine_task_limits();
  statemachine_task_event();
}

const char* statemachine_get_state_as_string(void)
{
  return state_to_string(state);
}
