CPU = stm32f407
CPU_LINK_MEM = f407_mem.ld

BOARD_BASE := board/pc

#Uncomment the following line to enable stm32 USB libraries
STM32F4_USB_LIBS = 1

#Uncomment the following to include any USB device classes you might want
#Uncomment any of the following three to build in support for USB DEV/OTG/Host
STM32_USB_DEV = 1
STM32_USB_DEV_CDC = 1
#STM32_USB_HOST = 1
#STM32_USB_OTG = 1

#this board uses the stmf4discovery board which has an 8mhz external
#oscillator
BOARD_DEFINES += -DHSE_VALUE=8000000 -DPLL_M=8

BOARD_C_FILES += $(wildcard $(BOARD_BASE)/*.c)

include cpu/stm32f4xx/config.mk
