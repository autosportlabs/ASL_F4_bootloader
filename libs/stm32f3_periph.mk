# STM32f3xx standard peripheral library makefile
#
# Jeff Ciesielski <jeff@autosportlabs.com>

ifneq ($(STM32F3XX_LIBS),)

F3_PERIPH_BASE = libs/STM32F30x_StdPeriph_Driver
F3_CMSIS_BASE = $(F3_PERIPH_BASE)/CMSIS
F3_CMSIS_DEVICE = $(F3_CMSIS_BASE)/Device/ST/STM32F30x

LIB_C_FILES += $(wildcard $(F3_PERIPH_BASE)/src/*.c)
LIB_C_FILES += $(F3_CMSIS_DEVICE)/Source/Templates/system_stm32f30x.c
LIB_S_FILES += $(F3_CMSIS_DEVICE)/Source/Templates/TrueSTUDIO/startup_stm32f30x.s

LIB_INCLUDES += -I$(F3_PERIPH_BASE)/inc
LIB_INCLUDES += -I$(F3_CMSIS_DEVICE)/Include
LIB_INCLUDES += -I$(F3_CMSIS_BASE)/Include

endif
