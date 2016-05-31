#ifndef _CRC_H_
#define _CRC_H_

#include <stdbool.h>
#include <stddef.h>

uint32_t crc_block(uint32_t *start, size_t len, bool reset);
#endif
