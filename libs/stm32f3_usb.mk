#STM32F3 USB Library makefile
#
#Jeff Ciesielski <jeff@autosportlabs.com>

ifneq ($(STM32F3_USB_LIBS),)

F3_USB_BASE = libs/STM32_USB-FS-Device_Driver

LIB_C_FILES += $(wildcard $(F3_USB_BASE)/src/*.c)
LIB_INCLUDES += -I$(F3_USB_BASE)/inc

endif #STM32_USB_LIBS
