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

#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

#include "ledmatrix.h"

#define LEDMATRIX_PORT PORTB
#define LEDMATRIX_DDR DDRB
#define LEDMATRIX_RED 6
#define LEDMATRIX_GREEN 7
#define LEDMATRIX_NUM_LEDS 6
#define LEDMATRIX_MASK 0x3F
#define BLINK_DELAY 3

ledmatrix_mode_t mode = off;
uint8_t moving_cnt = 0;
uint8_t wait_cnt = 0;

void ledmatrix_start_timer(void)
{
  TCCR3A = 0;                             // prescaler 1:1024, WGM = 4 (CTC)
  TCCR3B = 1<<WGM32 | 1<<CS32 | 1<<CS30;  //
  OCR3A = 1561;                           // (1+1561)*1024 = 1599488 -> ~100 ms @ 16 MHz
  TCNT3 = 0;
  TIMSK3 = 1<<OCIE3A;
}

void ledmatrix_stop_timer(void)
{
  TCCR3B = 0;
  TIMSK3 = 0;
}

void ledmatrix_off_init(void)
{
  LEDMATRIX_PORT = LEDMATRIX_MASK;
  LEDMATRIX_PORT |= 1<<LEDMATRIX_RED | 1<<LEDMATRIX_GREEN;
}


void ledmatrix_red_init(void)
{
  LEDMATRIX_PORT = LEDMATRIX_MASK | 1<<LEDMATRIX_GREEN;
  LEDMATRIX_PORT &= ~(1<<LEDMATRIX_RED);
}


void ledmatrix_red_moving_init(void)
{
  moving_cnt = 0;
  LEDMATRIX_PORT = (1<<LEDMATRIX_GREEN) | (LEDMATRIX_MASK & (1<<moving_cnt));
  ledmatrix_start_timer();
}

void ledmatrix_red_moving_handle(void)
{
  moving_cnt++;
  if(moving_cnt >= LEDMATRIX_NUM_LEDS)
    moving_cnt = 0;
  LEDMATRIX_PORT = (1<<LEDMATRIX_GREEN) | (LEDMATRIX_MASK & (1<<moving_cnt));
}


void ledmatrix_red_blink_init(void)
{
  wait_cnt = 0;
  ledmatrix_red_init();
  ledmatrix_start_timer();
}

void ledmatrix_red_blink_handle(void)
{
  if(++wait_cnt >= BLINK_DELAY) {
    LEDMATRIX_PORT ^= 1<<LEDMATRIX_RED;
    wait_cnt = 0;
  }
}


void ledmatrix_green_init(void)
{
  LEDMATRIX_PORT = LEDMATRIX_MASK | 1<<LEDMATRIX_RED;
  LEDMATRIX_PORT &= ~(1<<LEDMATRIX_GREEN);
}


void ledmatrix_green_moving_init(void)
{
  moving_cnt = 0;
  LEDMATRIX_PORT = (1<<LEDMATRIX_RED) | (LEDMATRIX_MASK & (1<<(LEDMATRIX_NUM_LEDS - moving_cnt - 1)));
  ledmatrix_start_timer();
}

void ledmatrix_green_moving_handle(void)
{
  moving_cnt++;
  if(moving_cnt >= LEDMATRIX_NUM_LEDS)
    moving_cnt = 0;
  LEDMATRIX_PORT = (1<<LEDMATRIX_RED) | (LEDMATRIX_MASK & (1<<(LEDMATRIX_NUM_LEDS - moving_cnt - 1)));
}


void ledmatrix_green_blink_init(void)
{
  wait_cnt = 0;
  ledmatrix_green_init();
  ledmatrix_start_timer();
}

void ledmatrix_green_blink_handle(void)
{
  if(++wait_cnt >= BLINK_DELAY) {
    LEDMATRIX_PORT ^= 1<<LEDMATRIX_GREEN;
    wait_cnt = 0;
  }
}


void ledmatrix_rg_moving_init(void)
{
  moving_cnt = 0;
  LEDMATRIX_PORT = (1<<LEDMATRIX_GREEN) | (LEDMATRIX_MASK & (1<<moving_cnt));
  ledmatrix_start_timer();
}

void ledmatrix_rg_moving_handle(void)
{
  moving_cnt++;
  if(moving_cnt >= 2*LEDMATRIX_NUM_LEDS)
    moving_cnt = 0;

  if(moving_cnt >= LEDMATRIX_NUM_LEDS) {
    uint8_t offset = moving_cnt - LEDMATRIX_NUM_LEDS;
    LEDMATRIX_PORT = (1<<LEDMATRIX_RED) | (LEDMATRIX_MASK & (1<<(LEDMATRIX_NUM_LEDS - offset - 1)));
  } else {
    LEDMATRIX_PORT = (1<<LEDMATRIX_GREEN) | (LEDMATRIX_MASK & (1<<moving_cnt));
  }
}


void ledmatrix_rg_blink_init(void)
{
  wait_cnt = 0;
  ledmatrix_red_init();
  ledmatrix_start_timer();
}

void ledmatrix_rg_blink_handle(void)
{
  if(++wait_cnt >= BLINK_DELAY) {
    LEDMATRIX_PORT ^= ~(LEDMATRIX_MASK);
    wait_cnt = 0;
  }
}


void ledmatrix_init(void)
{
  LEDMATRIX_DDR = 0xFF;
  LEDMATRIX_PORT = 0xFF;
}

void ledmatrix_set(ledmatrix_mode_t m)
{
  if(m == mode)
    return;

  mode = m;
  ledmatrix_stop_timer();
  switch(mode)
  {
  case off: ledmatrix_off_init(); break;
  case red: ledmatrix_red_init(); break;
  case red_moving: ledmatrix_red_moving_init(); break;
  case red_blink: ledmatrix_red_blink_init(); break;
  case green: ledmatrix_green_init(); break;
  case green_moving: ledmatrix_green_moving_init(); break;
  case green_blink: ledmatrix_green_blink_init(); break;
  case rg_moving: ledmatrix_rg_moving_init(); break;
  case rg_blink: ledmatrix_rg_blink_init(); break;
  }
}

ISR(TIMER3_COMPA_vect)
{
  switch(mode)
  {
  case red_moving: ledmatrix_red_moving_handle(); break;
  case red_blink: ledmatrix_red_blink_handle(); break;
  case green_moving: ledmatrix_green_moving_handle(); break;
  case green_blink: ledmatrix_green_blink_handle(); break;
  case rg_moving: ledmatrix_rg_moving_handle(); break;
  case rg_blink: ledmatrix_rg_blink_handle(); break;
  default: break;
  }
}
