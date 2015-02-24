##
##  tuer-rfid
##
##
##  Copyright (C) 2013-2014 Christian Pointner <equinox@spreadspace.org>
##                          Othmar Gsenger <otti@wirdorange.org>
##
##  This file is part of tuer-rfid.
##
##  tuer-rfid is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  any later version.
##
##  tuer-rfid is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with tuer-rfid. If not, see <http://www.gnu.org/licenses/>.
##

NAME := tuer-rfid
BOARD_TYPE := teenstep
OBJ := $(NAME).o heartbeat.o stepper.o ledmatrix.o sl018.o keystore.o statemachine.o eventqueue.o limits.o manual.o ajar.o
LIBS := util led lufa-descriptor-usbserial anyio
EXTERNAL_LIBS := lufa
SPREADAVR_PATH := ../contrib/avr-utils
RESET_FUNC := $(SPREADAVR_PATH)/tools/reset_lufa_cdc
RESET_PARAM := 'r'

LUFA_PATH := $(SPREADAVR_PATH)/contrib/LUFA-120219
LUFA_OPTS  = -D USB_DEVICE_ONLY
LUFA_OPTS += -D DEVICE_STATE_AS_GPIOR=0
LUFA_OPTS += -D ORDERED_EP_CONFIG
LUFA_OPTS += -D FIXED_CONTROL_ENDPOINT_SIZE=8
LUFA_OPTS += -D FIXED_NUM_CONFIGURATIONS=1
LUFA_OPTS += -D USE_FLASH_DESCRIPTORS
LUFA_OPTS += -D USE_STATIC_OPTIONS="(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)"
LUFA_OPTS += -D INTERRUPT_CONTROL_ENDPOINT

LUFA_OPTS += -D USB_MANUFACTURER="L\"equinox\"" -D USB_MANUFACTURER_LEN=7
LUFA_OPTS += -D USB_PRODUCT="L\"realraum door firmware\"" -D USB_PRODUCT_LEN=22

LUFA_COMPONENTS := USB USBCLASS TWI SERIAL

CLEAN_TARGETS += clean-pdf clean-update-keys

include $(SPREADAVR_PATH)/include.mk

update-keys: update-keys.c
	gcc -o $@ $<

clean-update-keys:
	rm -f update-keys

pdf: statemachine.dot
	dot -Tpdf statemachine.dot -o statemachine.pdf

clean-pdf:
	rm -f statemachine.pdf
