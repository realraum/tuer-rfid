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

#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

#include "stepper.h"
#include "limits.h"
#include "eventqueue.h"

uint8_t step_table [] =
{
    /* full steps */
  /* 6,  // 0110 */
  /* 5,  // 0101 */
  /* 9,  // 1001 */
  /* 10, // 1010 */

    /* half steps */
  2,  // 0010
  6,  // 0110
  4,  // 0100
  5,  // 0101
  1,  // 0001
  9,  // 1001
  8,  // 1000
  10, // 1010
};

#define STEPPER_PORT PORTF
#define STEPPER_DDR DDRF
#define STEPPER_FIRST_BIT 4
#define STEPPER_ENABLE_A_BIT 0
#define STEPPER_ENABLE_B_BIT 1
#define LENGTH_STEP_TABLE (sizeof(step_table)/sizeof(uint8_t))
#define STEPPER_OUTPUT_BITMASK (~(0xF << STEPPER_FIRST_BIT ))

volatile uint16_t step_cnt = 0;
#define STEP_CNT_STOP (LENGTH_STEP_TABLE*150)
stepper_direction_t step_direction = dir_open;

inline void stepper_stop(void)
{
  STEPPER_PORT &= ~(0xF << STEPPER_FIRST_BIT | 1<<STEPPER_ENABLE_A_BIT | 1<<STEPPER_ENABLE_B_BIT);
  TCCR1B = 0; // no clock source
  TIMSK1 = 0; // disable timer interrupt

  if(step_direction == dir_open)
    eventqueue_push_from_isr(open_fin);
  else
    eventqueue_push_from_isr(close_fin);
}

static inline uint8_t stepper_handle(void)
{
  static uint8_t step_idx = 0;

  uint8_t stepper_output = step_table[step_idx];
  stepper_output <<= STEPPER_FIRST_BIT;
  STEPPER_PORT = (STEPPER_PORT & STEPPER_OUTPUT_BITMASK ) | stepper_output;
  step_idx += (step_direction == dir_open) ? 1 : -1;
  step_idx %= LENGTH_STEP_TABLE;

  if(step_cnt++ < STEP_CNT_STOP
    && (step_direction != dir_close || limits_check_motor_moving()) //if closing, stop motor if blocked
    ) {
    limits_t l = limits_get_for_motor();
    if((step_direction == dir_open && l == open) ||
       (step_direction == dir_close && l == close) || l == both)
      return 0; //stop
  } else {
    eventqueue_push_from_isr(move_timeout);
    return 0; //stop
  }
  return 1;
}

void stepper_init(void)
{
  STEPPER_PORT &= ~(0xF << STEPPER_FIRST_BIT | 1<<STEPPER_ENABLE_A_BIT | 1<<STEPPER_ENABLE_B_BIT);
  STEPPER_DDR |= (0xF << STEPPER_FIRST_BIT) | (1<<STEPPER_ENABLE_A_BIT) | (1<<STEPPER_ENABLE_B_BIT);
}

uint8_t stepper_start(stepper_direction_t direction)
{
  step_cnt = 0;
  step_direction = direction;
  STEPPER_PORT |= (1<<STEPPER_ENABLE_A_BIT) | (1<<STEPPER_ENABLE_B_BIT);
  TCCR1A = 0;                    // prescaler 1:256, WGM = 4 (CTC)
  TCCR1B = 1<<WGM12 | 1<<CS12;   //
  OCR1A = 150;
  TCNT1 = 0;
  TIMSK1 = 1<<OCIE1A;

  limits_reset_motor_move_check();

  return 1;
}

ISR(TIMER1_COMPA_vect)
{
  if(!stepper_handle())
    stepper_stop();
}
