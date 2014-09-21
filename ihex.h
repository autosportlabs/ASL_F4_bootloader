#ifndef _IHEX_H_
#define _IHEX_H_
#include <stdint.h>
#define IHEX_MAX_DATA_LEN 16

enum rowtype {
	ROWTYPE_DATA = 0x00,
	ROWTYPE_EOF = 0x01,
	ROWTYPE_EXT_SEG_ADDR = 0x02,
	ROWTYPE_START_SEG_ADDR = 0x03,
	ROWTYPE_EXT_LIN_ADDR = 0x04,
	ROWTYPE_START_LIN_ADDR = 0x05,
};

enum ihex_error {
	IHEX_SUCCESS,
	IHEX_INVALID_DATALEN,
	IHEX_INVALID_CHECKSUM,
	IHEX_INVALID_TYPE,
	IHEX_INVALID_ROW,
};

struct ihex_row {
	uint8_t data_len;
	uint16_t offset;
	enum rowtype type;
	uint8_t data[IHEX_MAX_DATA_LEN];
	uint8_t checksum;
};

#endif	/* IHEX_H */
