# This file is part of the Racecapture/Track project
#
# Copyright (C) 2013 Autosport Labs
#
# Author(s):
# 	Andrey Smirnov <andrew.smirnov@gmail.com>
# 	Jeff Ciesielski <jeffciesielski@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This file can be used to autoconfigure your board for ease of
# building, otherwise it passes the BOARD environment variable to the
# rest of the makefile

# TODO: dep files

-include config.mk

ifeq ($(BOARD),)
  ifeq ($(wildcard board.default),)
    $(error BOARD is not defined.  Pass it in as BOARD= or create a config.mk file)
  else
    BOARD = $(shell cat board.default)
  endif
else
  $(shell echo "$(BOARD)" > board.default)
endif

LIB_PATH := libs
BOARD_PATH := board/$(BOARD)

# load the board specific configuration
include $(BOARD_PATH)/config.mk

ifeq ($(CPU_TYPE),)
$(error CPU_TYPE is not defined, please ensure it is defined in your cpu config.mk)
endif

PREFIX	?= arm-none-eabi

CC      := $(PREFIX)-gcc
AS      := $(PREFIX)-as
CXX     := $(PREFIX)-g++
AR      := $(PREFIX)-ar
NM      := $(PREFIX)-nm
OBJCOPY := $(PREFIX)-objcopy
OBJDUMP := $(PREFIX)-objdump
SIZE    := $(PREFIX)-size

OPENOCD := openocd
DDD     := ddd
GDB     := $(PREFIX)-gdb

# This file is dynamically created based on the libraries in the libs/ folder
-include libs.mk

INCLUDES += $(CPU_INCLUDES) $(BOARD_INCLUDES) $(LIB_INCLUDES) $(APP_INCLUDES)
INCLUDES += -Icpu/common -Iboard/common

CFLAGS ?= -Os -g -Wall -fno-common -c -mthumb
CFLAGS += -mcpu=$(CPU_TYPE) -MD -std=gnu99
CFLAGS += -ffunction-sections -fdata-sections -flto
CFLAGS += $(INCLUDES) $(CPU_DEFINES) $(BOARD_DEFINES) $(APP_DEFINES) $(CPU_FLAGS)

ASFLAGS += -mcpu=$(CPU_TYPE) $(FPU) -g -Wa,--warn

LIBS = -lnosys

LDFLAGS ?= --specs=nano.specs -lc -lgcc $(LIBS) -mcpu=$(CPU_TYPE) -g -gdwarf-2 
LDFLAGS += -L. -Lcpu/common -L$(CPU_BASE) -T$(CPU_LINK_MEM) -Tlink_sections.ld 
LDFLAGS += -nostartfiles -Wl,--gc-sections -mthumb -mcpu=$(CPU_TYPE)
LDFLAGS += -ffunction-sections -fdata-sections -flto -Os
LDFLAGS += -msoft-float -Wl,--Map=$(TARGET).map

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
# Do not print "Entering directory ...".
MAKEFLAGS += --no-print-directory
endif

# Object files
APP_O_FILES = $(APP_C_FILES:.c=.o) $(APP_S_FILES:.s=.o)
BOARD_O_FILES = $(BOARD_C_FILES:.c=.o) $(BOARD_S_FILES:.s=.o)
CPU_O_FILES = $(CPU_C_FILES:.c=.o) $(CPU_S_FILES:.s=.o)
LIB_O_FILES  = $(LIB_C_FILES:.c=.o) $(LIB_S_FILES:.s=.o)

ALL_O_FILES := $(APP_O_FILES) $(BOARD_O_FILES) $(CPU_O_FILES) $(LIB_O_FILES)

ifeq ($(TARGET),)
$(error TARGET is not defined, please define it in your applications config.mk)
endif

LIB_CONFIGS = $(wildcard $(LIB_PATH)/*.mk)

all: messages $(TARGET).bin pylib

libs.mk: Makefile
	@printf " Generating library includes\n"
	@( $(foreach L,$(LIB_CONFIGS),echo 'include $L';) ) >$@

$(TARGET).bin: $(TARGET).elf
	@printf "  OBJCOPY $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(PREFIX)-objcopy -Obinary $< $@
	$(SIZE) $<

$(TARGET).elf: libs.mk $(ALL_O_FILES)
	@printf "  LD      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) -o $@ $(ALL_O_FILES) $(LDFLAGS)

.c.o:
	@printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	@printf "  AS      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(CC) $(ASFLAGS) -c -o $@ $<

pylib: messages
	@printf " Generating python library\n"
	$(Q)python setup.py sdist
	$(Q)cp dist/* .
	$(Q)rm -rf asl_f4_loader.egg-info dist

messages:
	@printf " Generating messages\n"
	$(Q)xbvcgen -i upgrade_agent/messages.yaml -o msgs -l c -l python -t device -c
	$(Q)cp -f msgs/C/* upgrade_agent
	$(Q)cp -f msgs/Python/* host_tools
	$(Q)rm -rf msgs

clean:
	$(Q)rm -f *.o *.a *.d ../*.o ../*.d $(OBJS) $(LIBS_ALL)\
	$(ALL_O_FILES) \
	$(shell find . -name "*.d") \
	$(TARGET).bin $(TARGET).elf \
	upgrade_agent/xbvc_core.c \
	upgrade_agent/xbvc_core.h \
	upgrade_agent/cobs.c \
	upgrade_agent/cobs.h \
	host_tools/cobs.py* \
	host_tools/xbvc_py* \
	asl_f4_loader*.tar.gz \
	libs.mk

flash: $(TARGET).elf
	$(OPENOCD) -f $(BOARD_PATH)/debug.ocd -f flash.ocd

debug: $(TARGET).bin
	$(OPENOCD) -f $(BOARD_PATH)/debug.ocd

ddd: $(TARGET).elf
	$(DDD) --eval-command="target remote localhost:3333" --debugger $(GDB) $(TARGET).elf

gdb: $(TARGET).elf
	$(GDB) -ex "target ext localhost:3333" -ex "mon reset halt" -ex "mon arm semihosting enable" $(TARGET).elf

.PHONY: clean flash debug ddd messages
