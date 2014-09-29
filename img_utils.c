#include <stdbool.h>
#include <stddef.h>

#include <stm32f4xx.h>
#include <core_cm4.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_crc.h>
#include <stm32f4xx_rcc.h>

#include <img_utils.h>

/* CRC's a block of 32 bit words */
static uint32_t crc_block(uint32_t *start, size_t len, bool reset)
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

static int validate_app(struct app_info_block *info_block)
{
	size_t working_len;

	/* First, validate that the info block is legit.
	 *  Subtract 1 from the working length to avoid
	 *  trying to crc our own crc. */
	working_len = sizeof(struct app_info_block);
	uint32_t crc_res = crc_block((uint32_t*)info_block,
				     working_len / 4 - 1,
				     true);

	/* CRCs don't match, return an error */
	if (crc_res != info_block->info_crc)
		return -1;

	/* CRC everything up to (but not including) the app info block */
	working_len = (uint32_t)info_block - info_block->start_addr;
	crc_res = crc_block((uint32_t*)info_block->start_addr,
			    working_len / 4,
			    true);

	/* Skip the info block, and crc until the end of the
	 * application */

	/* Calculate the length from the end of the info block until
	 * the end of the application*/
	working_len = (info_block->start_addr + info_block->app_len)
		- (uint32_t)info_block - sizeof(struct app_info_block);

	crc_res = crc_block((uint32_t*)((uint32_t)info_block + sizeof(struct app_info_block)),
			    working_len / 4,
			    false);

	/* If CRC doesn't match the app CRC, return an error */
	if (crc_res != info_block->app_crc)
		return -2;

	/* We haz valid app.  WOOHOO! */
	return 0;
}

struct app_info_block *scan_for_app(void)
{
	/* Start with our pointer at the beginning of the flash area */
	/* Note that this is a u32 pointer, the info block should be
	 * aligned on a 4 octet boundary */
	uint32_t *info_block_p = (uint32_t*)FLASH_BASE;

	/* calculate the last possible starting position of an app
	 * info block, this will be used to ensure that we don't walk
	 * off the end of flash and hard-fault.
	 * Subtract the size of an app info block
	 * (plus one additional byte) to determine
	 * the last possible position that we could find an info block */
	uint32_t last_pos = FLASH_END - (sizeof(struct app_info_block) - 1);

	while ((uint32_t)info_block_p < last_pos) {
		/* If the magic number isn't correct, just move forward */
		struct app_info_block *ib = (struct app_info_block*)info_block_p;
		if (ib->magic_number != APP_INFO_MAGIC_NUMBER) {
			info_block_p++;
			continue;
		}

		/* If we reach this point, the magic number matches.
		 * check to see if it is valid */
		int res = validate_app((struct app_info_block*)info_block_p);

		/* Not a valid block, move the info block pointer
		 * forward and continue to the next check */
		if (res) {
			info_block_p += sizeof(uint32_t);
			continue;
		}

		/* If we reach this point, we have found a valid
		 * image, return it */
		return (struct app_info_block*)info_block_p;
	}

	/* If we're here, sadly, there is no valid image in the
	 * device flash.  Return null */
	return NULL;
}

void jump_to_app(uint32_t address)
{
	typedef void (*func_ptr)(void);

	/* Function pointer to our app */
	func_ptr reset_vec;

	/* Variable to hold the start address of the application */
	const uint32_t *vec_table = (uint32_t*) address;

	/* Get the reset vector from the vector table */
	reset_vec = (func_ptr)((uint32_t)vec_table[1]);

	/* Disable all interrupts */
	int i;
	for (i = 0; i < 8; i++)
		NVIC->ICER[i] = NVIC->IABR[i];

	/* Set the stack pointer to the first word in the vector table */
	__set_MSP((uint32_t)(vec_table[0]));

	/* Execute the reset vector (we don't return from this) */
	reset_vec();
}
