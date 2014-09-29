#include <stddef.h>
#include <xbvc_core.h>
#include <img_utils.h>
#include <flash_utils.h>
#include <usbd_cdc_vcp.h>

static struct app_info_block *last_known_info;

void xbvc_handle_flash_command(struct x_flash_command *msg)
{
	struct x_flash_response rsp;
	int8_t res;
       
	rsp.error = ERR_SUCCESS;

	res = flash_write_block(msg->data, msg->data_len, msg->offset);

	if (res)
		rsp.error = ERR_FLASH_FAIL;

	xbvc_send(&rsp, E_MSG_FLASH_RESPONSE);
}

void xbvc_handle_verify_command(struct x_verify_command *msg)
{
	struct x_verify_response rsp;
	
	last_known_info = scan_for_app();

	if (last_known_info == NULL)
		rsp.error = ERR_VERIFY_FAIL;
	else
		rsp.error = ERR_SUCCESS;

	xbvc_send(&rsp, E_MSG_VERIFY_RESPONSE);
}

void xbvc_handle_run_command(struct x_run_command *msg)
{
	struct x_run_response rsp;
	rsp.error = ERR_SUCCESS;

	if (last_known_info == NULL)
		last_known_info = scan_for_app();

	if (last_known_info == NULL)
		rsp.error = ERR_RUN_FAIL;
	else
		rsp.error = ERR_SUCCESS;

	xbvc_send(&rsp, E_MSG_RUN_RESPONSE);

	/* Make sure we sent the run response */
	vcp_flush_tx();

	if (last_known_info != NULL)
		jump_to_app(last_known_info->start_addr);
}
