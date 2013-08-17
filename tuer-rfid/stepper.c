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

#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

#include "stepper.h"
#include "limits.h"
#include "eventqueue.h"

uint8_t step_table [] =
{
  10, // 1010
  9,  // 1001
  5,  // 0101
  6,  // 0110
};

#define STEPPER_PORT PORTF
#define STEPPER_DDR DDRF
#define STEPPER_FIRST_BIT 4
#define STEPPER_ENABLE_BIT 1
#define LENGTH_STEP_TABLE (sizeof(step_table)/sizeof(uint8_t))
#define STEPPER_OUTPUT_BITMASK (~(0xF << STEPPER_FIRST_BIT ))

volatile uint16_t step_cnt = 0;
#define STEP_CNT_STOP (LENGTH_STEP_TABLE*400)
#define STEP_CNT_OFF  (STEP_CNT_STOP + 125)
stepper_direction_t step_direction = dir_open;

inline void stepper_stop(void)
{
  STEPPER_PORT &= ~(0xF << STEPPER_FIRST_BIT | 1<<STEPPER_ENABLE_BIT);
  TCCR1B = 0; // no clock source
  TIMSK1 = 0; // disable timer interrupt
}

static inline uint8_t stepper_handle(void)
{
  static uint8_t step_idx = 0;

  limits_t l = limits_get();
  if((step_direction == dir_open && l == open) ||
     (step_direction == dir_close && l == close) || l == both)
    step_cnt = STEP_CNT_STOP + 1;

  if(step_cnt < STEP_CNT_STOP) {
    step_idx += (step_direction == dir_open) ? 1 : -1;
    step_idx %= LENGTH_STEP_TABLE;
  } else if(step_cnt == STEP_CNT_STOP) {
    eventqueue_push(move_timeout);
    return 0;
  }

  uint8_t stepper_output = step_table[step_idx];
  stepper_output <<= STEPPER_FIRST_BIT;
  STEPPER_PORT = (STEPPER_PORT & STEPPER_OUTPUT_BITMASK ) | stepper_output;

  step_cnt++;
  if(step_cnt >= STEP_CNT_OFF) {
    if(step_direction == dir_open)
      eventqueue_push(open_fin);
    else
      eventqueue_push(close_fin);

    return 0;
  }

  return 1;
}

void stepper_init(void)
{
  STEPPER_PORT &= ~(0xF << STEPPER_FIRST_BIT | 1<<STEPPER_ENABLE_BIT);
  STEPPER_DDR |= (0xF << STEPPER_FIRST_BIT) | (1<<STEPPER_ENABLE_BIT);
}

uint8_t stepper_start(stepper_direction_t direction)
{
  step_cnt = 0;
  step_direction = direction;
  STEPPER_PORT |= 1<<STEPPER_ENABLE_BIT;
  TCCR1A = 0;                    // prescaler 1:256, WGM = 4 (CTC)
  TCCR1B = 1<<WGM12 | 1<<CS12;   //
  OCR1A = 124;                   // (1+124)*256 = 32000 -> 2 ms @ 16 MHz
  TCNT1 = 0;
  TIMSK1 = 1<<OCIE1A;

  return 1;
}

ISR(TIMER1_COMPA_vect)
{
  if(!stepper_handle())
    stepper_stop();
}
