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

#include "keystore.h"
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

#include "led.h"
#include "anyio.h"

#define EEPROM_SIZE 1024
typedef uint8_t keyslot_t[8];
keyslot_t EEMEM keystore[EEPROM_SIZE/sizeof(keyslot_t)];


void keystore_flash_from_stdio(void)
{
  keyslot_t ks;
  uint8_t byte_pos=0;
  printf("Info(keystore): flashing\n\r");
  fflush(stdout);
  for(uint8_t ks_pos=0;ks_pos<EEPROM_SIZE/sizeof(ks);) {
    anyio_task();

    int16_t bytes_received = anyio_bytes_received();
    while(bytes_received > 0) {
      ks[byte_pos++]=fgetc(stdin);
      bytes_received--;
      if (byte_pos == sizeof(ks)) {
        byte_pos=0;
        eeprom_update_block(&ks,&keystore[ks_pos],sizeof(ks));
        ks_pos++;
        fputc('.', stdout);
        fflush(stdout);
        led_toggle();
      }
    }
  }
  printf("\n\r");
  fputc(0, stdout);
  led_off();
}

void keystore_dump_to_stdio(void)
{
  keyslot_t ks;
  for(uint8_t ks_pos=0;ks_pos<EEPROM_SIZE/sizeof(ks);ks_pos++) {
    eeprom_read_block(&ks,&keystore[ks_pos],sizeof(ks));
    for (uint8_t i=0; i< sizeof(ks); i++)
      printf("%02X",ks[i]);
    printf("\n\r");
  }
}

/* this generates a Fletcher8 checksum  */
/* code from: http://stackoverflow.com/questions/13491700/8-bit-fletcher-checksum-of-16-byte-data */
uint8_t generate_csum(uint8_t* data)
{
  uint16_t sum1 = 0xf, sum2 = 0xf, len = sizeof(keyslot_t) - 1;
  do { sum2 += ( sum1 += *data++ ); } while (--len);
  return sum2<<4 | sum1;
}

uint8_t compare_keyslots(const keyslot_t a, const keyslot_t b)
{
  uint8_t tmp=0;
      // constant time compare
  for(uint8_t i=0; i<sizeof(keyslot_t); ++i)
    tmp |= a[i] ^ b[i];
  return tmp;
}

uint8_t keystore_check_card(const uint8_t * uid, uint8_t uid_len)
{
  keyslot_t card, ks;
  memset(card, 0, sizeof(card));
  for (uint8_t pos=0; pos<uid_len; pos++)
    card[pos]=uid[uid_len-pos-1];
  card[sizeof(keyslot_t)-1]=generate_csum(card);
  uint8_t valid=0;
  for(uint8_t ks_pos=0;ks_pos<(EEPROM_SIZE/sizeof(ks));ks_pos++) {
    eeprom_read_block(&ks,&keystore[ks_pos],sizeof(ks));
    if(!compare_keyslots(card, ks)) {
      valid=1;
      // break;  // this would break security (not constant time)
    }
  }
  return valid;
}

