#include <stm32f4xx.h>
#include <core_cm4.h>

#include <stddef.h>
#include <stdbool.h>

#include <stm32f4xx_misc.h>
#include <stm32f4xx_crc.h>
#include <stm32f4xx_rcc.h>

/* CRC's a block of 32 bit words */
uint32_t crc_block(uint32_t *start, size_t len, bool reset)
{
	uint32_t res;
	/* Enable CRC clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	
	if (reset)
		CRC_ResetDR();

	res = CRC_CalcBlockCRC(start, len);
	
	/* Disable CRC clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, DISABLE);
	
	return res;
}
