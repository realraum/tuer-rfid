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

#include "sl018.h"
#include "LUFA/Drivers/Peripheral/TWI.h"
#include <stdio.h>
#include <util/delay.h>

#define SL018_TWI_ADDR  0xA0
#define SL018_TAG_STA_PIN PINE
#define SL018_TAG_STA_BIT 6
#define CARD_PRESENT (!((SL018_TAG_STA_PIN >> SL018_TAG_STA_BIT) & 1))
#define MAX_UID_LEN 7


const char* SL018_cmd_tostring(const uint8_t cmd)
{
  switch(cmd) {
  case 0x01: return "Select Mifare card";
  case 0x02: return "Login to a sector";
  case 0x03: return "Read a data block";
  case 0x04: return "Write a data block";
  case 0x05: return "Read a value block";
  case 0x06: return "Initialize a value block";
  case 0x07: return "Write master key";
  case 0x08: return "Increment value";
  case 0x09: return "Decrement value";
  case 0x0A: return "Copy value";
  case 0x10: return "Read a data page";
  case 0x11: return "Write a data page";
  case 0x40: return "Control the red led";
  case 0xF0: return "Get firmware version";
  case 0xFF: return "Reset";
  default: return "unknown";
  }
}

const char* SL018_status_tostring(const uint8_t status)
{
  switch(status) {
  case 0x0: return "Operation succeed";
  case 0x1: return "No tag";
  case 0x2: return "Login succeed";
  case 0x3: return "Login fail";
  case 0x4: return "Read fail";
  case 0x5: return "Write fail";
  case 0x6: return "Unable to read after write";
  case 0xA: return "Collision occur";
  case 0xC: return "Load key fail";
  case 0xD: return "Not authenticate";
  case 0xE: return "Not a value block";
  default: return "unknown";
  }
}

const char* SL018_tagtype_tostring(const uint8_t type)
{
  switch(type) {
  case 0x1: return "Mifare 1k, 4 byte UID";
  case 0x2: return "Mifare 1k, 7 byte UID";
  case 0x3: return "Mifare Ultralight or NATG203, 7 byte UID";
  case 0x4: return "Mifare 4k, 4 byte UID";
  case 0x5: return "Mifare 4k, 7 byte UID";
  case 0x6: return "Mifare DesFire, 7 byte UID";
  default: return "unknown";
  }
}

uint8_t SL018_tagtype_to_uidlen(const uint8_t type)
{
  switch(type) {
  case 0x1:
  case 0x4: return 4;
  case 0x2:
  case 0x3:
  case 0x5:
  case 0x6: return 7;
  default: return 0;
  }
}

const uint8_t SL018_CMD_ComSelectCard[]         = {1,0x01};
const uint8_t SL018_CMD_ComRedLedOn[]           = {2,0x40,1};
const uint8_t SL018_CMD_ComRedLedOff[]          = {2,0x40,0};
const uint8_t SL018_CMD_ComGetFirmwareVersion[] = {1,0xF0};
const uint8_t SL018_CMD_ComReset[]              = {1,0xFF};

uint8_t twi_recv_buf[256];
typedef struct __attribute__((__packed__))
{
  uint8_t len;
  uint8_t command;
  uint8_t status;
  uint8_t data[sizeof(twi_recv_buf)-3];
} sl018_message_t;

sl018_message_t* twi_recv_msg = (sl018_message_t *)&twi_recv_buf;


uint8_t sl018_cmd_raw(const uint8_t* twi_send_buf, bool wait_for_answer)
{
  uint8_t pos = 0;

  if (TWI_StartTransmission(SL018_TWI_ADDR | TWI_ADDRESS_WRITE,10) == TWI_ERROR_NoError) {
    for(pos=0; pos<=twi_send_buf[0]; pos++) {
      if( ! TWI_SendByte(twi_send_buf[pos])) {
        TWI_StopTransmission();
        return 1;
      }
    }
    TWI_StopTransmission();
  } else
    return 1;

  if(!wait_for_answer) return 0;

  memset(twi_recv_buf, 0, sizeof(twi_recv_buf));
  _delay_ms(50);

  if (TWI_StartTransmission(SL018_TWI_ADDR | TWI_ADDRESS_READ,10) == TWI_ERROR_NoError) {
    TWI_ReceiveByte(twi_recv_buf, 0);
    for(pos=1; pos<=twi_recv_buf[0]; pos++) {
      if (! TWI_ReceiveByte(&twi_recv_buf[pos], (pos == twi_recv_buf[0]) ? 1:0 ) ) {
        TWI_StopTransmission();
        return 1;
      }
    }
    TWI_StopTransmission();
  } else
    return 1;

  return 0;
}

uint8_t sl018_reset(void)
{
  if(sl018_cmd_raw(SL018_CMD_ComReset, 0)) {
    printf("Error(i2c): bus error\n\r");
    return 1;
  }
  return 0;
}

uint8_t sl018_cmd(const uint8_t* twi_send_buf)
{
  if(sl018_cmd_raw(twi_send_buf, 1)) {
    printf("Error(i2c): bus error\n\r");
    return 1;
  } else {
    if(twi_recv_msg->len < 2) {
      printf("Error(SL018): short message received\n\r");
      return 1;
    }
    if(twi_recv_msg->status) {
      printf("Error(SL018): '%s','%s'\n\r",SL018_cmd_tostring(twi_recv_msg->command),SL018_status_tostring(twi_recv_msg->status));
      return 1;
    }
    sl018_message_t * twi_send_msg = (sl018_message_t *)twi_send_buf;
    if(twi_send_msg->command != twi_recv_msg->command) {
      printf("Error(SL018): mismatch of sent and received command code: %02X,%02X\n\r",twi_send_msg->command,twi_recv_msg->command);
    }
  }
  return 0;
}

void sl018_read_card_uid(uid_t * uid)
{
  uid->length=0;
  uid->buffer=NULL;
  printf( "Info(card): ");
  if(!sl018_cmd(SL018_CMD_ComSelectCard))
  {
    uint8_t uid_len = twi_recv_msg->len - sizeof(twi_recv_msg->command) - sizeof(twi_recv_msg->status) - 1;
    if(uid_len == 255 || uid_len > MAX_UID_LEN) {
      printf(" received UID length (%d) is to big for keystore \n\r", uid_len);
      return;
    }
    uint8_t type = twi_recv_msg->data[uid_len];
    uint8_t expected_uid_len = SL018_tagtype_to_uidlen(type);
    if(expected_uid_len != uid_len) {
      printf(" Invalid uid length (%d) for tag type: %s\n\r", uid_len, SL018_tagtype_tostring(type));
      return;
    }

    for (uint8_t pos=0; pos<uid_len; pos++)
      printf("%02X",twi_recv_msg->data[uid_len-pos-1]);
    printf( ", %s\n\r", SL018_tagtype_tostring(type));

    if (0 < type && type < 7) {
      uid->length= uid_len;
      uid->buffer=twi_recv_msg->data;
    } else {
      printf("Info(card): Ignoring unknown card type %02x\n\r",type);
    }
  }
}

void sl018_set_led(uint8_t on)
{
  if(on)
   sl018_cmd(SL018_CMD_ComRedLedOn);
  else
   sl018_cmd(SL018_CMD_ComRedLedOff);
}

uint8_t sl018_check_for_new_card(void)
{
  static uint8_t card_status = 0;
  if(CARD_PRESENT != card_status) {
    card_status = CARD_PRESENT;
    if(card_status)
      return 1;
  }
  return 0;
}

unsigned char * sl018_get_firmware_version(void)
{
  if(!sl018_cmd(SL018_CMD_ComGetFirmwareVersion)) {
   twi_recv_msg->data[sizeof(twi_recv_msg->data) - 1] = 0;
   return twi_recv_msg->data;
  } else {
    return NULL;
  }
}
