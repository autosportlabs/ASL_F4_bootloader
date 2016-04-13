#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <stm32f30x_flash.h>
#include <img_utils.h>
#include <util.h>
#include <flash_utils.h>

#define PAGE_SIZE 2048
#define NUM_PAGES (256 * 1024 / PAGE_SIZE)

static bool page_erased[NUM_PAGES] = {false};

/* If successful, returns the page number,
 * Returns negative number on error */
static int8_t flash_get_page(uint32_t address)
{
	/* First, check to make sure the address is even in our
	 * address space */
	if (!IS_FLASH_PROGRAM_ADDRESS(address))
		return -1;

	return (address & 0xfffff) / PAGE_SIZE;
}

/* Returns False==0 / True==1 / Invalid_page==-1 */
static int8_t flash_check_page_erased(uint32_t offset)
{

	int8_t page = flash_get_page(offset);

	if (page >= ARRAY_SIZE(page_erased) || page < 0)
		return -1;

	if (page_erased[page]) {
		return 1;
	} else {
		return 0;
	}
}

/* Returns 0 on success, anything else is a failure */
static int8_t flash_erase_page(uint32_t offset)
{
	int8_t ret = 0;
	FLASH_Status st;

	int8_t page = flash_get_page(offset);

	if (page >= ARRAY_SIZE(page_erased))
		return -1;

	st = FLASH_ErasePage(offset);

	if (st != FLASH_COMPLETE)
		ret = -2;

	/* Mark this page as erased */
	page_erased[page] = true;

	return ret;
}

int8_t flash_write_block(uint32_t *data, size_t len, uint32_t offset)
{
	int8_t ret = 0;
	
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	/* Loop through all of the data we were provided */
	for (int i = 0; i < len; i++, offset+=4) {
		/* Get the erasure status of the page that this
		 * address falls into */
		int8_t erase_stat = flash_check_page_erased(offset);

		/* If there was an error, propegate the error up */
		if (erase_stat < 0) {
			ret = -2;
			break;
		}

		/* We need to erase this block and make sure
		 * it was successful */
		if (erase_stat == 0) {
			int8_t res = flash_erase_page(offset);
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
