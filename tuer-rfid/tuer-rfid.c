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

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include "LUFA/Drivers/Peripheral/TWI.h"

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "led.h"
#include "anyio.h"

#include "heartbeat.h"
#include "stepper.h"
#include "ledmatrix.h"
#include "sl018.h"
#include "keystore.h"
#include "statemachine.h"
#include "eventqueue.h"
#include "limits.h"
#include "manual.h"
#include "ajar.h"

void handle_cmd(uint8_t cmd)
{
  switch(cmd) {
    case 'r':
             reset2bootloader();
             break;
    case 'R':
             if(!sl018_reset())
               printf("ok\r\n");
             break;
    case 'f': {
                unsigned char * firmware_str = sl018_get_firmware_version();
                if(firmware_str)
                  printf("%s\r\n",firmware_str);

                break;
              }
    case 'e': //flash eeprom
             keystore_flash_from_stdio();
             break;
    case 'd': //dump eeprom - this breaks security!
             keystore_dump_to_stdio();
             break;
    case 'o':
             eventqueue_push(cmd_open);
             break;
    case 'c':
             eventqueue_push(cmd_close);
             break;
    case 't':
             eventqueue_push(cmd_toggle);
             break;
    case 's':
             printf("Status: %s %s %s\r\n", limits_to_string(limits_get()), statemachine_get_state_as_string(), ajar_to_string(ajar_get()));
             break;
    /* case '0': ledmatrix_set(off); break; */
    /* case '1': ledmatrix_set(red); break; */
    /* case '2': ledmatrix_set(red_moving); break; */
    /* case '3': ledmatrix_set(red_blink); break; */
    /* case '4': ledmatrix_set(green); break; */
    /* case '5': ledmatrix_set(green_moving); break; */
    /* case '6': ledmatrix_set(green_blink); break; */
    /* case '7': ledmatrix_set(rg_moving); break; */
    /* case '8': ledmatrix_set(rg_blink); break; */
    default: printf("Error(cmd): unknown command %02X '%c'\r\n", cmd, cmd); return;
  }
}

void handle_card(void)
{
  uid_t uid;
  sl018_read_card_uid(&uid);
  if (uid.length)
  {
    printf("Info(card): card(");
    for (uint8_t pos=0; pos<uid.length; pos++)
      printf("%02X",uid.buffer[uid.length-pos-1]);
    printf(") ");

    if(keystore_check_card(uid.buffer,uid.length)) {
      printf("found - opening/closing door\r\n");
      eventqueue_push(card);
    } else {
      printf("not found - ignoring\r\n");
    }
  }
}

int main(void)
{
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  cpu_init();
  jtag_disable();
  led_init();
  anyio_init(115200, false);
  TWI_Init(TWI_BIT_PRESCALE_1, TWI_BITLENGTH_FROM_FREQ(1, 200000));

  heartbeat_init();
  stepper_init();
  ledmatrix_init();
  eventqueue_init();
  limits_init();
  manual_init();
  ajar_init();
  sei();

  sl018_reset();

  for(;;) {
    statemachine_task();

    anyio_task();
    manual_task();
    ajar_task();
    limits_task();

    int16_t bytes_received = anyio_bytes_received();
    if(bytes_received > 0)
      handle_cmd(fgetc(stdin));

    if (sl018_check_for_new_card())
      handle_card();

    heartbeat_task();
  }
}
