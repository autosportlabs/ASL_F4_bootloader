CPU = stm32f303
CPU_LINK_MEM = f303_mem.ld

BOARD_BASE := board/rct

#Uncomment the following line to enable stm32 USB libraries
STM32F3_USB_LIBS = 1

#this board uses an 8mhz external oscillator
BOARD_DEFINES += -DHSE_VALUE=8000000 -DPLL_M=8

BOARD_C_FILES += $(wildcard $(BOARD_BASE)/*.c)

include cpu/stm32f3xx/config.mk
