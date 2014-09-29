#include <usbd_cdc_core.h>
#include <usbd_usr.h>
#include <usbd_conf.h>
#include <usbd_desc.h>
#include <usbd_cdc_vcp.h>

#include <upgrade_agent.h>
#include <xbvc_core.h>

USB_OTG_CORE_HANDLE USB_OTG_dev __attribute__ ((aligned (4)));


/* This is called by the XBVC state machine */
static void upgrade_agent_usb_init(void *params)
{
	/* Initialize the USB hardware */
	USBD_Init(&USB_OTG_dev,
		  USB_OTG_FS_CORE_ID,
		  &USR_desc,
		  &USBD_CDC_cb,
		  &USR_cb);
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
	xbvc_init(NULL,
		  upgrade_agent_usb_read,
		  upgrade_agent_usb_write,
		  upgrade_agent_usb_init);

	while(1)
		xbvc_run();
}
