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

#include <USB-CDC_device.h>
#include <hw_config.h>
#include <stdbool.h>
#include <string.h>
#include <usb_desc.h>
#include <usb_istr.h>
#include <usb_lib.h>
#include <usb_mem.h>
#include <usb_prop.h>
#include <usb_pwr.h>

#define USB_BUF_ELTS(in, out, bufsize) ((in - out + bufsize) % bufsize)

static struct {
	uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
	uint8_t USB_Tx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
	volatile uint32_t USB_Tx_ptr_in;
	volatile uint32_t USB_Tx_ptr_out;
	volatile uint32_t USB_Rx_ptr_in;
	volatile uint32_t USB_Rx_ptr_out;
	volatile int _init_flag;
} usb_state;

/*
 * ST's USB hardware doesn't include an internal pullup resistor on
 * USB_DP for some odd reason and instead relies on an externally
 * gated (via a set of transistors) pull up attached to a
 * USB_DISCONNECT pin. As this is only used during startup (we don't
 * do any dynamic re-enumeration), we can shortcut this by having a
 * static pullup, and simply causing a glitch on the line by forcing
 * USB_DP low before we initialize the rest of the USB hardware. It's
 * a little hacky, but doesn't seem to cause any ill effects.
 */
static int USB_CDC_force_reenumeration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_ResetBits(GPIOA, GPIO_Pin_12);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	return 0;
}

/* Public API */
int USB_CDC_device_init(void)
{
	/* Perform a full USB reset */
	USB_CDC_force_reenumeration();

	Set_System();
	Set_USBClock();

	USB_Interrupts_Config();
	USB_Init();

	usb_state._init_flag = 1;

	return 0;
}

int USB_CDC_device_deinit(void)
{
	PowerOff();

	return 0;
}

void USB_CDC_flush(void)
{
	uint16_t usb_buf_elements = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
						 usb_state.USB_Tx_ptr_out,
						 VIRTUAL_COM_PORT_DATA_SIZE);

	while (0 < usb_buf_elements) {
		usb_buf_elements = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
						usb_state.USB_Tx_ptr_out,
						VIRTUAL_COM_PORT_DATA_SIZE);
	}

	/* Allow a little time for the hardware to clear out */
	for(int i = 0; i < 10000; i++) {
		asm volatile("NOP");
	}

}

void USB_CDC_write(uint8_t *src, int len)
{
	if (bDeviceState != CONFIGURED) {
		return;
	}
	while (len--) {
		/*
		 * If the transmit buffer is full, wait until the USB
		 * hardware has had time to flush it out
		 */
		uint16_t usb_buf_elements = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
							 usb_state.USB_Tx_ptr_out,
							 VIRTUAL_COM_PORT_DATA_SIZE);

		while (VIRTUAL_COM_PORT_DATA_SIZE - 1 == usb_buf_elements) {
			usb_buf_elements = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
							usb_state.USB_Tx_ptr_out,
							VIRTUAL_COM_PORT_DATA_SIZE);
		}

		usb_state.USB_Tx_Buffer[usb_state.USB_Tx_ptr_in] = *src++;

		/* Handle wrapping */
		if(VIRTUAL_COM_PORT_DATA_SIZE - 1 == usb_state.USB_Tx_ptr_in) {
			usb_state.USB_Tx_ptr_in = 0;
		} else {
			usb_state.USB_Tx_ptr_in++;
		}

	}
}

int USB_CDC_read(uint8_t *dst, int len)
{
	int received = 0;

	if (bDeviceState != CONFIGURED) {
		return 0;
	}

	while (len--) {

		if (usb_state.USB_Rx_ptr_out != usb_state.USB_Rx_ptr_in) {
			*dst++ = usb_state.USB_Rx_Buffer[usb_state.USB_Rx_ptr_out++];
			received++;
			/*
			 * If we've cleared the buffer, send a signal to the
			 * USB hardware to stop NAKing packets and to
			 * continue receiving
			 */
			if (usb_state.USB_Rx_ptr_out == usb_state.USB_Rx_ptr_in) {
				/* Enable the receive of data on EP3 */
				SetEPRxValid(ENDP3);
			}

			/*
			 * Note: We don't need to handle wrapping in
			 * the receive calls because we always fill
			 * the rx buffer from the start and drain to
			 * completion
			 */
		} else {
			break;
		}
	}

	return received;
}

int USB_CDC_is_initialized()
{
	return usb_state._init_flag;
}

/* Private USB Stack callbacks */

/*
 * This code has been adapted from the ST Microelectronics CDC
 * Example, which is covered under the V2 Liberty License:
 * http://www.st.com/software_license_agreement_liberty_v2
 */
void usb_handle_transfer(void)
{
	uint16_t USB_Tx_length = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
					      usb_state.USB_Tx_ptr_out,
					      VIRTUAL_COM_PORT_DATA_SIZE);

	if(0 == USB_Tx_length) {
		return;
	}

	/*
	 * Handle the situation where we can only transmit to the end
	 * of the buffer in this packet
	 */
	if(usb_state.USB_Tx_ptr_out > usb_state.USB_Tx_ptr_in) {
		USB_Tx_length = (VIRTUAL_COM_PORT_DATA_SIZE -
				 usb_state.USB_Tx_ptr_out);
	}

	UserToPMABufferCopy(&usb_state.USB_Tx_Buffer[usb_state.USB_Tx_ptr_out],
			    ENDP1_TXADDR, USB_Tx_length);
	usb_state.USB_Tx_ptr_out += USB_Tx_length;

	/* Handle wrapping */
	if (VIRTUAL_COM_PORT_DATA_SIZE == usb_state.USB_Tx_ptr_out) {
		usb_state.USB_Tx_ptr_out = 0;
	}

	SetEPTxCount(ENDP1, USB_Tx_length);
	SetEPTxValid(ENDP1);
}

void EP1_IN_Callback (void)
{
	usb_handle_transfer();
}

void EP3_OUT_Callback(void)
{
	/* Get the received data buffer and clear the counter */
	usb_state.USB_Rx_ptr_in = USB_SIL_Read(EP3_OUT, usb_state.USB_Rx_Buffer);
	usb_state.USB_Rx_ptr_out = 0;

	/*
	 * USB data will be processed by handler threads, we will
	 * continue to NAK packets until such a time as all of the
	 * prior data has been handled
	 */
}

void SOF_Callback(void)
{
	static uint32_t FrameCount = 0;

	if(bDeviceState == CONFIGURED) {
		if (VIRTUAL_COM_PORT_IN_FRAME_INTERVAL == FrameCount++) {
			/* Reset the frame counter */
			FrameCount = 0;

			/* Check the data to be sent through IN pipe */
			usb_handle_transfer();
		}
         }
}
