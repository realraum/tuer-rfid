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

#include <avr/io.h>
#include "limits.h"
#include <LUFA/Drivers/Peripheral/ADC.h>

#define LIMITS_ADC_CHAN_NUM 8
#define LIMITS_ADC_CHAN ADC_CHANNEL8

#define LIMITS_RANGE_MIN 99 //maximum closed
#define LIMITS_RANGE_MAX 792 //maximum opened

/// HINT: Voltage Measurement *200 = LIMITS value

#define LIMITS_RINGBUF_SIZE 4
/* HINT: this is compared to a sliding sum not an average! */
// LIMITS_TH should be close to / just beyond actual lock/unlock position
#define LIMITS_TH_CLOSE 407 * LIMITS_RINGBUF_SIZE //1.9V
#define LIMITS_TH_OPEN  510 * LIMITS_RINGBUF_SIZE

// HYSTERESIS is how detection of state will have some drift in both directions
#define LIMITS_HYSTERESIS ((LIMITS_RANGE_MAX) - (LIMITS_RANGE_MIN)) * LIMITS_RINGBUF_SIZE / 80 //width of the hysteresis
#define LIMITS_TH_CLOSE_HIGH ((LIMITS_TH_CLOSE) + (LIMITS_HYSTERESIS))
#define LIMITS_TH_OPEN_LOW   ((LIMITS_TH_OPEN)  - (LIMITS_HYSTERESIS))

// EXTRA_MOVEMENT is how far the motor will move beyond LIMITS_TH
// #define LIMITS_MOTOR_EXTRA_MOVEMENT 150 * LIMITS_RINGBUF_SIZE //add / substract to motor movement beyond LIMITS_TH (see https://github.com/realraum/tuer-rfid/issues/2 )
#define LIMITS_MOTOR_EXTRA_MOVEMENT_TH_CLOSE (LIMITS_TH_CLOSE - (LIMITS_TH_CLOSE - (LIMITS_RANGE_MIN*LIMITS_RINGBUF_SIZE))/3)
#define LIMITS_MOTOR_EXTRA_MOVEMENT_TH_OPEN  (LIMITS_TH_OPEN + ((LIMITS_RANGE_MAX*LIMITS_RINGBUF_SIZE) - LIMITS_TH_OPEN)/3)

#define LIMITS_DIFFERENCE_TH_FOR_NO_MOVEMENT 2 //minimum difference between adc sampling sums to count as movement
#define LIMITS_MAX_COUNT_FOR_NO_MOVEMENT 10    //number of counts of sum difference being below LIMITS_DIFFERENCE_TH_FOR_NO_MOVEMENT before motor stops

void limits_init(void)
{
  ADC_Init(ADC_FREE_RUNNING | ADC_PRESCALE_128);
  ADC_SetupChannel(LIMITS_ADC_CHAN_NUM);
  ADC_StartReading(ADC_REFERENCE_AVCC | ADC_RIGHT_ADJUSTED | LIMITS_ADC_CHAN);
  ADCSRA |= (1 << ADIE);
}

// sum must not be used directly
// the sum is like an average of 4 ADC sample values where we avoid the divison by LIMITS_RINGBUF_SIZE for performance reasons
static uint16_t sum = 0;
static uint16_t prev_sum = 0;
static bool state_initialized = false;
static uint8_t motor_not_moving_ctr_ = 0;
static limits_t state = open;
static limits_t state_for_motor = open; //extra state for motor, so motor moves further that LIMITS_TH_CLOSE/OPEN.  (see https://github.com/realraum/tuer-rfid/issues/2 )

// without applying hysteresis, used only for initialization
static limits_t sum_to_state(uint16_t s)
{
  if(s < LIMITS_TH_CLOSE)
    return close;

  if(s < LIMITS_TH_OPEN)
   return moving;

  return open;
}

static limits_t sum_to_state_for_motor(uint16_t s)
{
  if(s < LIMITS_MOTOR_EXTRA_MOVEMENT_TH_CLOSE)
    return close;

  if(s < LIMITS_MOTOR_EXTRA_MOVEMENT_TH_OPEN)
   return moving;

  return open;
}

// must be called from ISR or with disabled interrupts
static void update_state(void)
{
  state_for_motor = sum_to_state_for_motor(sum);

  //calc hysteresis for state
  if (state_initialized)
  {
    if (sum < LIMITS_TH_CLOSE)
    {
      state = close;
    }
    else if (sum < LIMITS_TH_CLOSE_HIGH)
    {
    }
    else if (sum < LIMITS_TH_OPEN_LOW)
    {
      state = moving;
    }
    else if (sum < LIMITS_TH_OPEN)
    {
    }
    else
    {
      state = open;
    }
  }
  else
  {
    state = sum_to_state(sum);
    state_initialized = true;
  }
}

// these variables must not be used by anything outside of the ISR
static uint16_t r[LIMITS_RINGBUF_SIZE] = { 0 };
static uint8_t idx = 0;
ISR(ADC_vect)
{
  r[idx] = ADC;
  idx = (idx + 1) % LIMITS_RINGBUF_SIZE;
  sum = 0;
  uint8_t i;
      // a sliding sum might be faster but the size of the ringbuffer is low and
      // it is safer to always compute the sum from scratch
  for(i=0; i<LIMITS_RINGBUF_SIZE; ++i) sum += r[i];

  update_state();
}

inline uint16_t limits_get_raw(void)
{
  cli();
  uint16_t s = sum;
  sei();

  return s;
}

static limits_t limits_get_state(void)
{
  cli();
  limits_t st = state;
  sei();

  return st;
}

limits_t limits_get(void)
{
  return limits_get_state();
}


limits_t limits_get_for_motor(void)
{
  cli();
  limits_t st = state_for_motor;
  sei();

  return st;
}

//these functions should only ever be called from the same context and same function, so that motor_not_moving_ctr_ does not need to be locked
void limits_reset_motor_move_check(void)
{
  motor_not_moving_ctr_ = 0;
}

//these functions should only ever be called from the same context and same function, so that motor_not_moving_ctr_ does not need to be locked
uint8_t limits_check_motor_moving(void)
{
  uint16_t cur_sum = limits_get_raw();
  int16_t diff = prev_sum - cur_sum;
  prev_sum = cur_sum;
  bool adc_values_currently_changing = (diff > LIMITS_DIFFERENCE_TH_FOR_NO_MOVEMENT || diff < -1* LIMITS_DIFFERENCE_TH_FOR_NO_MOVEMENT);

  if (adc_values_currently_changing) {
    if (motor_not_moving_ctr_ > 1) {
        motor_not_moving_ctr_--;
        motor_not_moving_ctr_--;
      }
  } else if (motor_not_moving_ctr_ < 0xff) {
    motor_not_moving_ctr_++;
  }

  return (motor_not_moving_ctr_ < LIMITS_MAX_COUNT_FOR_NO_MOVEMENT);
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
