/* STM32F4xx Bootloader
 *
 * Jeff Ciesielski <jeff@autosportlabs.com>
 */

#include <stdbool.h>
#include <stddef.h>
#include <img_utils.h>
#include <upgrade_agent.h>
int main(void)
{
	struct app_info_block *app;

	/* Check to see if we have an application in our flash area */
	app = scan_for_app();

	upgrade_agent_usb_loader();
	if (app != NULL)
		jump_to_app(app->start_addr);
	while(1);
}
