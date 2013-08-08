/*
 *  spreadspace avr utils
 *
 *
 *  Copyright (C) 2013 Christian Pointner <equinox@spreadspace.org>
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

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <termios.h>

#define EEPROM_SIZE 1024
typedef uint8_t keyslot_t[8];

/* this generates a Fletcher8 checksum  */
/* code from: http://stackoverflow.com/questions/13491700/8-bit-fletcher-checksum-of-16-byte-data */
uint8_t generate_csum(keyslot_t data)
{
  uint16_t sum1 = 0xf, sum2 = 0xf, len = sizeof(keyslot_t)-1;
  do { sum2 += ( sum1 += *data++ ); } while (--len);
  return sum2<<4 | sum1;
}

int send_key(keyslot_t key, FILE* dev)
{
  fwrite(key, sizeof(keyslot_t), 1, dev);
  fflush(dev);
  char tmp;
  while(fread(&tmp, 1, 1, dev)) {
    fwrite(&tmp, 1, 1, stdout);
    if(tmp == 0) return 1;
    if(tmp == '.') return 0;
  }
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc<2) {
    fprintf(stderr, "Usage: update-keys <device>\n");
    return -1;
  }
  
  FILE* dev;
  dev = fopen(argv[1], "r+");
  if(!dev) {
    fprintf(stderr, "fopen failed!\n");
    return -2;
  }

  int fd = fileno(dev);
  struct termios t;
  tcgetattr(fd, &t);
  t.c_lflag &= ~(ICANON | ECHO);
  t.c_iflag &= ~(ICRNL | INLCR | IXON | IXOFF);
  cfmakeraw(&t);
  tcflush(fd, TCIOFLUSH);
  tcsetattr(fd, TCSANOW, &t);

  fprintf(dev, "e");

  char* line = NULL;
  size_t len = 0;
  int line_num = 0;
  keyslot_t key;
  int key_num = 0;

  for(;;) {
    ssize_t ret = getline(&line, &len, stdin);
    if(ret <= 0) break;
    line_num++;

    int i;
    for(i=0; i<len; ++i) {
      if(!isxdigit(line[i])) {
        line[i] = 0;
        break;
      }
    }
    if(i & 1 || i == 0 || i > (sizeof(keyslot_t)-1)*2) {
      fprintf(stderr, "ignoring invalid key (odd number of digits or empty string or too long) at line %d\n", line_num);
      continue;
    }
    uint8_t tmp[3];
    int j;
    tmp[2] = 0;
    for(j = 0; j<(i/2); ++j) {
      tmp[0] = line[j*2];
      tmp[1] = line[j*2 + 1];
      key[j] = (char)strtoul(tmp, NULL, 16);
    }
    for(j=i/2; j < sizeof(keyslot_t)-1; ++j) {
      key[j] = 0;
    }
    key[sizeof(keyslot_t)-1] = generate_csum(key);
    if(send_key(key, dev)) {
      fprintf(stderr, "send_key failed at keyslot %d\n", key_num);
      return 1;
    }

    key_num++;
    if(key_num > EEPROM_SIZE/sizeof(keyslot_t)) {
      fprintf(stderr, "reached maximum number of key slots (%d), will ignore remaining keys\n", EEPROM_SIZE/sizeof(keyslot_t));
      break;
    }
  }
  printf("\nread %d keys from STDIN - filling rest of keystore with invalid keys\n", key_num);
  
  int i;
  for(i=0; i<=sizeof(keyslot_t)-1; ++i) key[i] = 0xFF;
  for(i=key_num; i < EEPROM_SIZE/sizeof(keyslot_t); ++i) {
    if(send_key(key, dev)) {
      fprintf(stderr, "send_key failed at keyslot %d\n", key_num);
      return 1;
    }
    key_num++;
  }

  printf("\nwrite of %d keys finished\n", key_num);

  return 0;
}
