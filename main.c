/* STM32F4xx Bootloader
 *
 * Jeff Ciesielski <jeff@autosportlabs.com>
 */

#include <stdbool.h>
#include <stddef.h>
#include <img_utils.h>
#include <upgrade_agent.h>
#include <stm32f4xx.h>
#include <bootmode.h>

int main(void)
{
	struct app_info_block *app;
	bool upgrade_requested;

	/* Check to see if an upgrade has been specifically requested
	 * by the user */
	upgrade_requested = bootmode_upgrade_requested();
	
	/* Check to see if we have an application in our flash area */
	app = scan_for_app();
	
	if (upgrade_requested || app == NULL)
		upgrade_agent_usb_loader();

	jump_to_app(app->start_addr);

	while(1);
}
