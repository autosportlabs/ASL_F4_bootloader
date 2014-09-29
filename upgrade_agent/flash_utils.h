#ifndef _FLASH_UTILS_H_
#define _FLASH_UTILS_H_
#include <stdint.h>
#include <stddef.h>

int8_t flash_write_block(uint32_t *data, size_t len, uint32_t offset);
#endif	/* FLASH_UTILS_H */
