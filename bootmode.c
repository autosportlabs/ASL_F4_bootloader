#include <stdbool.h>
#include <stdint.h>
#include <boot_gpi.h>
#include <app_info.h>

static bool bootmode_check_software_flag(void)
{
	struct app_handshake_block *handshake = (struct app_handshake_block*)HANDSHAKE_ADDR;
	bool ret = false;
	
	if (handshake->loader_magic == LOADER_KEY) {
		handshake->loader_magic = 0;
		ret = true;
	}

	return ret;
}

bool bootmode_upgrade_requested(void)
{
	bool gpi, sw;
	bool ret = false;

	gpi = boot_gpi_asserted();
	sw = bootmode_check_software_flag();

	if (gpi || sw)
		ret = true;

	return ret;
}
