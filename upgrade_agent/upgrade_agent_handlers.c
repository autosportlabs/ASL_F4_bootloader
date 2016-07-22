#include <stddef.h>
#include <xbvc_core.h>
#include <img_utils.h>
#include <flash_utils.h>
#include <upgrade_agent.h>
#include <version.h>

static struct app_info_block *last_known_info;
extern uint32_t _flash_start;
extern uint32_t _flash_end;

static int offset_in_bootloader(uint32_t offset)
{
	return ((offset < (uint32_t)&_flash_end) &&
		(offset >= (uint32_t)&_flash_start));
}

void xbvc_handle_flash_command(struct x_flash_command *msg)
{
	struct x_flash_response rsp;

	rsp.error = ERR_FLASH_FAIL;
	if (!offset_in_bootloader(msg->offset) &&
	    !flash_write_block(msg->data, msg->data_len, msg->offset)) {

		rsp.error = ERR_SUCCESS;
	}
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
	upgrade_agent_usb_flush();

	if (last_known_info != NULL) {
		/* Shut down the USB interface */
		upgrade_agent_usb_deinit();

		/* Get going */
		jump_to_app(last_known_info->start_addr);
	}
}

void xbvc_handle_ping_command(struct x_ping_command *msg)
{
	struct x_ping_response rsp;

	xbvc_send(&rsp, E_MSG_PING_RESPONSE);
	upgrade_agent_usb_flush();
}

void xbvc_handle_get_version_command(struct x_get_version_command *msg)
{
	struct x_get_version_response rsp;

	memset(&rsp, 0, sizeof(struct x_get_version_response));
	rsp.major = VER_MAJOR;
	rsp.minor = VER_MINOR;
	rsp.bugfix = VER_BUGFIX;

	xbvc_send(&rsp, E_MSG_GET_VERSION_RESPONSE);
	upgrade_agent_usb_flush();
}

