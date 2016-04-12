# The name of our project (and the associated artifacts created)
TARGET = main


#Base directory of our application (assumes FreeRTOS_Base is '.')
APP_PATH = .

RCP_RELEASE_DIR ?= .
RELEASE_NAME = ASL_F4_bootloader-$(MAJOR).$(MINOR).$(BUGFIX)
RELEASE_NAME_ZIP = $(RELEASE_NAME).zip
RELEASE_NAME_BIN = $(RELEASE_NAME).bin
RELEASE_NAME_ELF = $(RELEASE_NAME).elf
RCP_INSTALL_DIR = ASL_F4_bootloader

INCLUDE_DIR = $(APP_PATH)/include
HAL_SRC = $(APP_PATH)/stm32_base/hal
RCP_SRC = $(APP_PATH)/src

# The source files of our application
APP_C_FILES = $(APP_PATH)/main.c \
	$(APP_PATH)/gqueue.c \
	$(APP_PATH)/img_utils.c \
	$(APP_PATH)/bootmode.c \
	$(APP_PATH)/upgrade_agent/upgrade_agent_usb.c \
	$(APP_PATH)/upgrade_agent/xbvc_core.c \
	$(APP_PATH)/upgrade_agent/cobs.c \
	$(APP_PATH)/upgrade_agent/upgrade_agent_handlers.c \
	$(APP_PATH)/util/newlib.c

# Adds this directory to the global application includes
APP_INCLUDES += -I. \
	-I$(APP_PATH) \
	-Iutil \
	-Iupgrade_agent

#Uncomment the following to use the ITM (trace macrocell) for printf
APP_DEFINES += -DUSE_ITM -DSD_SDIO -DMAJOR_REV=$(MAJOR) -DMINOR_REV=$(MINOR) -DBUGFIX_REV=$(BUGFIX)

#Optional command to flash the board using an ST-Link
APP_FLASH = st-flash erase write $(TARGET).bin 0x08000000

