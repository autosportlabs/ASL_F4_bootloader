#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <stm32f4xx_flash.h>
#include <img_utils.h>
#include <util.h>
#include <flash_utils.h>

static struct {
	uint8_t size; 		/* Size in kB */
	uint16_t sector; 	/* STM32 sector */
	bool erased;
}  __attribute__((__packed__)) flash_layout[] = {
	{
		.size = 16,
		.sector = FLASH_Sector_0,
		.erased = false,
	},
	{
		.size = 16,
		.sector = FLASH_Sector_1,
		.erased = false,
	},
	{
		.size = 16,
		.sector = FLASH_Sector_2,
		.erased = false,
	},
	{
		.size = 16,
		.sector = FLASH_Sector_3,
		.erased = false,
	},
	{
		.size = 64,
		.sector = FLASH_Sector_4,
		.erased = false,
	},
	{
		.size = 128,
		.sector = FLASH_Sector_5,
		.erased = false,
	},
	{
		.size = 128,
		.sector = FLASH_Sector_6,
		.erased = false,
	},
	{
		.size = 128,
		.sector = FLASH_Sector_7,
		.erased = false,
	},
	{
		.size = 128,
		.sector = FLASH_Sector_8,
		.erased = false,
	},
	{
		.size = 128,
		.sector = FLASH_Sector_9,
		.erased = false,
	},
	{
		.size = 128,
		.sector = FLASH_Sector_10,
		.erased = false,
	},
	{
		.size = 128,
		.sector = FLASH_Sector_11,
		.erased = false,
	},
};

/* If successful, returns the sector number,
 * Returns negative number on error */
static int8_t flash_get_sector(uint32_t address)
{
	uint32_t sect_start = FLASH_BASE;
	uint32_t sect_end = 0;
	int ret = -1;

	/* First, check to make sure the address is even in our
	 * address space */
	if (address > FLASH_END)
		return -1;

	/* If it is in our address space, figure out which sector it
	 * is in */
	for (int i = 0; i < ARRAY_SIZE(flash_layout); i++) {

		/* Calculate the end of the current sector */
		sect_end = sect_start + flash_layout[i].size * 1024;

		/* If the address falls within the bounds of this
		 * sector, set the destination and get out of this loop */
		if (address >= sect_start && address < sect_end) {
			ret = i;
			break;
		}

		/* Otherwise, move the sector_start counter forward */
		sect_start += (sect_end - sect_start);
	}

	/* Return our success */
	return ret;
}

/* Returns False==0 / True==1 / Invalid_sector==-1 */
static int8_t flash_check_sec_erased(uint8_t sec)
{
	if (sec >= ARRAY_SIZE(flash_layout))
		return -1;

	if (flash_layout[sec].erased)
		return 1;

	return 0;
}

/* Returns 0 on success, anything else is a failure */
static int8_t flash_erase_sector(int8_t sec)
{
	int8_t ret = 0;
	FLASH_Status st;

	if (sec >= ARRAY_SIZE(flash_layout))
		return -1;

	st = FLASH_EraseSector(flash_layout[sec].sector, VoltageRange_3);

	st = FLASH_WaitForLastOperation();

	if (st != FLASH_COMPLETE)
		ret = -2;

	/* Mark this sector as erased */
	flash_layout[sec].erased = true;

	return ret;
}

int8_t flash_write_block(uint32_t *data, size_t len, uint32_t offset)
{
	int8_t ret = 0;
	int8_t sect = 0;
	
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
			FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

	/* Loop through all of the data we were provided */
	for (int i = 0; i < len; i++, offset+=4) {
		/* Get the flash sector we're working with */
		sect = flash_get_sector(offset);

		/* Make sure it is valid */
		if (sect <= 0) {
			ret = -1;
			break;
		}

		/* Get the erasure status of the sector that this
		 * address falls into */
		int8_t erase_stat = flash_check_sec_erased(sect);

		/* If there was an error, propegate the error up */
		if (erase_stat < 0) {
			ret = -2;
			break;
		}

		/* We need to erase this block and make sure
		 * it was successful */
		if (erase_stat == 0) {
			int8_t res = flash_erase_sector(sect);
			if (res) {
				ret = -3;
				break;
			}
		}

		/* If we got here, just program the word we were provided */
		/* Swap the endianness of the data that was sent to
		 * us as the flash stores it in the opposite configuration */
		uint32_t dataword = __REV(data[i]);
		FLASH_Status st = FLASH_ProgramWord(offset, dataword);

		/* Make sure the programming went off without a hitch */
		if (st != FLASH_COMPLETE) {
			ret = -4;
			break;
		}
	}

	FLASH_Lock();
	return ret;
}
