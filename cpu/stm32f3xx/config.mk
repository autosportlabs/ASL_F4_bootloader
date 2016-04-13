CPU_ARCH = ARMCM4
CPU_TYPE = cortex-m4
CPU_BASE = cpu/stm32f3xx

CPU_FLAGS = -mfpu=fpv4-sp-d16 -mfloat-abi=softfp

STM32F3XX_LIBS = 1

CPU_C_FILES += $(wildcard $(CPU_BASE)/cdc_acm/*.c)
CPU_INCLUDES += -I$(CPU_BASE)/cdc_acm -I$(CPU_BASE)

# CPU Specific drivers (flash/bootmode gpi/crc/etc)
CPU_C_FILES += $(wildcard $(CPU_BASE)/*.c)

CPU_DEFINES = -D$(CPU_ARCH) -DSTM32F30X
