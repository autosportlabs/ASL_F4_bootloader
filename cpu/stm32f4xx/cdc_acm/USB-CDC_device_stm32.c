#include <USB-CDC_device.h>
#include <stdint.h>
#include <string.h>
#include <usbd_cdc_core.h>
#include <usbd_cdc_vcp.h>
#include <usbd_conf.h>
#include <usbd_desc.h>
#include <usbd_usr.h>

static volatile int _init_flag = 0;
USB_OTG_CORE_HANDLE USB_OTG_dev __attribute__ ((aligned (4)));

int USB_CDC_device_init(void)
{

    /* Initialize the USB hardware */
    USBD_Init(&USB_OTG_dev,
              USB_OTG_FS_CORE_ID,
              &USR_desc,
              &USBD_CDC_cb,
              &USR_cb);
    _init_flag = 1;

    return 0;
}

int USB_CDC_device_deinit(void)
{
	USBD_DeInit(&USB_OTG_dev);

	return 0;
}

int USB_CDC_read(uint8_t *dst, int len )
{
	return vcp_rx(dst, len, 0xffffffff);
}

void USB_CDC_write(uint8_t *src, int len )
{
	vcp_tx(src, len);
}

int USB_CDC_is_initialized()
{
    return _init_flag;
}

void USB_CDC_flush(void)
{
	vcp_flush_tx();
}

