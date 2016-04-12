/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USB_CDC_DEVICE_H_
#define USB_CDC_DEVICE_H_

#include <stdint.h>

int USB_CDC_device_init(void);
int USB_CDC_device_deinit(void);

int USB_CDC_read(uint8_t *dst, int len);
void USB_CDC_write(uint8_t *src, int len);
int USB_CDC_is_initialized();
void USB_CDC_flush();


#endif /* USB_CDC_DEVICE_H_ */
