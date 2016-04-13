#include <stddef.h>
#include <stdbool.h>

#include <stm32f30x_misc.h>
#include <stm32f30x_crc.h>
#include <stm32f30x_rcc.h>

/* CRC's a block of 32 bit words */
uint32_t crc_block(uint32_t *start, size_t len, bool reset)
{
	uint32_t res;
	/* Enable CRC clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	
	if (reset)
		CRC_ResetDR();

	res = CRC_CalcBlockCRC(start, len);
	
	/* Disable CRC clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, DISABLE);
	
	return res;
}
