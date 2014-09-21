/***********************************************************************************/
/* Intel hex decoder library							   */
/* Copyright 2014 Jeff Ciesielski <jeff@autosportlabs.com			   */
/*                                                                                 */
/* This program is free software; you can redistribute it and/or		   */
/* modify it under the terms of the GNU General Public License			   */
/* as published by the Free Software Foundation; either version 2		   */
/* of the License, or (at your option) any later version.			   */
/*  										   */
/* This program is distributed in the hope that it will be useful,		   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of		   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		   */
/* GNU General Public License for more details.					   */
/* 										   */
/* You should have received a copy of the GNU General Public License		   */
/* along with this program; if not, write to the Free Software			   */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */
/***********************************************************************************/
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <ihex.h>

static uint8_t ihex_generate_checksum(uint8_t *data, uint8_t len)
{
	uint8_t sum = 0;

	/* Sum up all of the bytes in the data block, since the ihex
	 * specification requres checksumming only the last byte,
	 * just use a uint8_t since that will truncate automatically */
	while (len--) {
		sum += *data++;
	}

	/* The checksum is simply the two's compliment of the sum, simple */
	sum = ~sum + 1;
	return sum;
}

static int ihex_verify_row(struct ihex_row *row)
{
	uint8_t scratch[20] = {0};

	/* Make sure that we can even process this data */
	if (row->data_len > 16)
		return IHEX_INVALID_DATALEN;

	/* Verify the row type */
	switch (row->type) {
	case ROWTYPE_DATA:
	case ROWTYPE_EOF:
	case ROWTYPE_EXT_SEG_ADDR:
	case ROWTYPE_START_SEG_ADDR:
	case ROWTYPE_EXT_LIN_ADDR:
	case ROWTYPE_START_LIN_ADDR:
		break;
	default:
		return IHEX_INVALID_TYPE;
	}

	/* Fill in the scratch area */
	scratch[0] = row->data_len;
	scratch[1] = row->offset >> 8;
	scratch[2] = row->offset & 0xff;
	scratch[3] = (uint8_t)row->type;

	for(size_t i = 0; i < row->data_len; i++)
		scratch[4 + i] = row->data[i];

	uint8_t checksum = ihex_generate_checksum(scratch, 4 + row->data_len);

	if (checksum != row->checksum)
		return IHEX_INVALID_CHECKSUM;

	return IHEX_SUCCESS;
}

int ihex_decode_line(uint8_t *linebuf, size_t len, struct ihex_row *dest)
{
	uint8_t scratch[5] = {0};
	int ret = IHEX_SUCCESS;
	uint8_t *end = linebuf + len;

	enum {
		E_DELIM,
		E_LEN,
		E_OFFSET,
		E_TYPE,
		E_DATA,
		E_CHECKSUM,
		E_ERR,
		E_FINISHED,
	} state = E_DELIM;
	
	while (state != E_FINISHED ) {
		/* Reset the scratch area each go-round */
		memset(scratch, 0x00, sizeof(scratch));

		/* Check to make sure we haven't overrun our line buffer */
		if (linebuf > end) {
			state = E_ERR;
			ret = IHEX_INVALID_ROW;
		}
		
		switch (state) {
		case E_DELIM:
			if (*linebuf != ':') {
				ret = IHEX_INVALID_ROW;
				state = E_ERR;
			} else {
				linebuf++;
				state = E_LEN;
			}
			break;
		case E_LEN:
			memcpy(scratch, linebuf, 2);
			linebuf += 2;
			dest->data_len = (uint8_t)strtol((char*)scratch, NULL, 16);

			if (dest->data_len > IHEX_MAX_DATA_LEN) {
				ret = IHEX_INVALID_DATALEN;
				state = E_ERR;
			} else {
				state = E_OFFSET;
			}
			break;
		case E_OFFSET:
			memcpy(scratch, linebuf, 4);
			linebuf += 4;
			dest->offset = (uint16_t)strtol((char*)scratch, NULL, 16);
			state = E_TYPE;
			break;
		case E_TYPE:
			memcpy(scratch, linebuf, 2);
			linebuf += 2;
			dest->type = (uint8_t)strtol((char*)scratch, NULL, 16);
			state = E_DATA;
			break;
		case E_DATA:
			/* Convert all of the data bytes into their
			 * binary equivalent */
			for (int i = 0; i < dest->data_len; i++) {
				memcpy(scratch, linebuf, 2);
				dest->data[i] = (uint8_t)strtol((char*)scratch, NULL, 16);
				linebuf += 2;
			}

			state = E_CHECKSUM;
			break;
		case E_CHECKSUM:
			memcpy(scratch, linebuf, 2);
			linebuf += 2;
			dest->checksum = (uint8_t)strtol((char*)scratch, NULL, 16);
			state = E_FINISHED;
			break;
		case E_ERR:
			return ret;
		case E_FINISHED:
			/* Realistically, we shouldn't get here */
			break;
		}
	}

	/* Now that we've decoded the line, validate it */
	ret = ihex_verify_row(dest);
		
	return ret;
}
