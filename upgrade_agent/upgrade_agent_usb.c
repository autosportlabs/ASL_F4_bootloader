#include <stdbool.h>
#include <stdint.h>

#include <usbd_cdc_core.h>
#include <usbd_usr.h>
#include <usbd_conf.h>
#include <usbd_desc.h>
#include <usbd_cdc_vcp.h>

#include <upgrade_agent.h>
#include <xbvc_core.h>
#include <core_cm4.h>

USB_OTG_CORE_HANDLE USB_OTG_dev __attribute__ ((aligned (4)));

/* This is called by the XBVC state machine */
static void upgrade_agent_usb_init(void *params)
{
	GPIO_InitTypeDef gpio_conf;

	/* Clear the GPIO Structure */
	GPIO_StructInit(&gpio_conf);

	/* Initialize the USB hardware */
	USBD_Init(&USB_OTG_dev,
		  USB_OTG_FS_CORE_ID,
		  &USR_desc,
		  &USBD_CDC_cb,
		  &USR_cb);


	/* Set up the LED */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
	gpio_conf.GPIO_OType = GPIO_OType_PP;
	gpio_conf.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOD, &gpio_conf);

	/* Start the systick timer.  We'll abuse this for LED
	 * goodness. */

}

int upgrade_agent_usb_read(uint8_t *dst, int len)
{
	uint16_t res;
	res = vcp_rx(dst, len, 0);

	return (int)res;
}

int upgrade_agent_usb_write(uint8_t *buf, int len)
{
        /* Unfortunately the usb system doesn't return the amount
	 * sent, just success or fail.  If we succeed, return the
	 * amount we were supposed to send, otherwise 0*/
	uint16_t res;
	res = vcp_tx(buf, len);

	if (res == USBD_FAIL)
		res = 0;
	else
		res = (uint16_t)len;

	return res;
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
			GPIO_ToggleBits(GPIOD, GPIO_Pin_10);
			i = 0;
		}
		xbvc_run();
	}
}

void upgrade_agent_usb_deinit(void)
{
	/* Shut down USB */
	USBD_DeInit(&USB_OTG_dev);

	GPIO_DeInit(GPIOD);
}

void upgrade_agent_usb_flush(void)
{
	vcp_flush_tx();
}
