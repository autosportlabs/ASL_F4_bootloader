# The name of our project (and the associated artifacts created)
TARGET = main

# board specific config file
include board/open407v_d/config.mk

#Base directory of our application (assumes FreeRTOS_Base is '.')
APP_BASE = .

# Uncomment the following to enable STM32 Peripheral libraries
STM32F4XX_LIBS = 1

#Uncomment the following line to enable stm32 USB libraries
STM32_USB_LIBS = 1

#Uncomment the following to include any USB device classes you might want
#Uncomment any of the following three to build in support for USB DEV/OTG/Host
STM32_USB_DEV = 1
STM32_USB_DEV_CDC = 1
#STM32_USB_HOST = 1
#STM32_USB_OTG = 1

#Uncomment the following line to enable ITM support (Trace Usart)
ITM = 1

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
APP_SRC = $(APP_PATH)/main.c \
	$(APP_PATH)/gqueue.c \
	$(APP_PATH)/ihex.c \
	$(APP_PATH)/img_utils.c \
	$(APP_PATH)/usb_bsp.c \
	$(APP_PATH)/usbd_cdc_vcp.c \
	$(APP_PATH)/usbd_desc.c \
	$(APP_PATH)/usbd_usr.c \
	$(APP_PATH)/upgrade_agent/flash_utils.c \
	$(APP_PATH)/upgrade_agent/upgrade_agent_usb.c \
	$(APP_PATH)/upgrade_agent/xbvc_core.c \
	$(APP_PATH)/upgrade_agent/cobs.c \
	$(APP_PATH)/upgrade_agent/upgrade_agent_handlers.c

#Macro that expands our source files into their fully qualified paths
#and adds a macro explaining how to convert them to binary
APP_OBJS = $(addprefix $(APP_BASE)/, $(APP_SRC:.c=.o))

# Adds this directory to the global application includes
APP_INCLUDES += -I. \
	-I$(APP_PATH) \
	-Iutil \
	-Iupgrade_agent

#Uncomment the following to enable newlib support
APP_INCLUDES += -Iutil
NEWLIB_SRC += newlib.c
NEWLIB_OBJS += $(addprefix util/, $(NEWLIB_SRC:.c=.o))
APP_OBJS += $(NEWLIB_OBJS)

#Uncomment the following to use the ITM (trace macrocell) for printf
APP_DEFINES += -DUSE_ITM -DSD_SDIO -DMAJOR_REV=$(MAJOR) -DMINOR_REV=$(MINOR) -DBUGFIX_REV=$(BUGFIX)

# CPU is generally defined by the Board's config.mk file
ifeq ($(CPU),)
$(error CPU is not defined, please define it in your CPU specific config.mk file)
endif

#Optional command to flash the board using an ST-Link
APP_FLASH = sudo st-flash write $(TARGET).bin 0x08000000

