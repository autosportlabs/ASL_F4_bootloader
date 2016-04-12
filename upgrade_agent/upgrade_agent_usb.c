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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <USB-CDC_device.h>
#include <upgrade_agent.h>
#include <xbvc_core.h>
#include <led.h>

/* This is called by the XBVC state machine */
static void upgrade_agent_usb_init(void *params)
{
	USB_CDC_device_init();
	led_init();
}

int upgrade_agent_usb_read(uint8_t *dst, int len)
{
	return USB_CDC_read(dst, len);
}

int upgrade_agent_usb_write(uint8_t *buf, int len)
{
        /* Unfortunately the usb system doesn't return the amount
	 * sent, just success or fail.  If we succeed, return the
	 * amount we were supposed to send, otherwise 0*/
	USB_CDC_write(buf, len);
	return len;
}

void upgrade_agent_usb_loader(void)
{
	int i = 0;
	xbvc_init(NULL,
		  upgrade_agent_usb_read,
		  upgrade_agent_usb_write,
		  upgrade_agent_usb_init);

	while(1) {
		if (i++ == 100000) {
			led_toggle();
			i = 0;
		}
		xbvc_run();
	}
}

void upgrade_agent_usb_deinit(void)
{
	/* TODO: Shut down USB */
	USB_CDC_device_deinit();
	led_deinit();
}

void upgrade_agent_usb_flush(void)
{
	USB_CDC_flush();
}
