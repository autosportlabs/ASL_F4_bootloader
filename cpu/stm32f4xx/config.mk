CPU_ARCH = ARMCM4
CPU_TYPE = cortex-m4
CPU_BASE = cpu/stm32f4xx

CPU_FLAGS = -mfpu=fpv4-sp-d16 -mfloat-abi=softfp

# Uncomment the following to enable STM32 Peripheral libraries
STM32F4XX_LIBS = 1

CPU_C_FILES += $(wildcard $(CPU_BASE)/cdc_acm/*.c)
CPU_INCLUDES += -I$(CPU_BASE)/cdc_acm -I$(CPU_BASE)

# CPU specific drivers (flash/crc/etc)
CPU_C_FILES += $(wildcard $(CPU_BASE)/*.c)

CPU_DEFINES = -D$(CPU_ARCH) -DSTM32F4XX
