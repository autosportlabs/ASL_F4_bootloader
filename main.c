/* STM32F4xx Hello World project (via blinkenlights)
 *
 * Jeff Ciesielski <jeff.ciesielski@gmail.com>
 */
#include <stm32f4xx_misc.h>

#ifdef USE_ITM
#include <itm.h>
#endif

#include <stdio.h>
#include "core_cm4.h"
#define FLASH_USER_OFFSET 		0x08020000
#define FLASH_USER_BASE         ((uint32_t)(0x08000000 + 0x20000))

//-----------------------------------------------------------------------------
static void jumpToApp(uint32_t address)
{
	typedef void (*pFunction)(void);

	pFunction Jump_To_Application;

	// variable that will be loaded with the start address of the application
	vu32* JumpAddress;
	const vu32* ApplicationAddress = (vu32*) address;

	// get jump address from application vector table
	JumpAddress = (vu32*) ApplicationAddress[1];

	// load this address into function pointer
	Jump_To_Application = (pFunction) JumpAddress;

	// reset all interrupts to default
	//TODO do this the CMSIS way
//	chSysDisable();

	// Clear pending interrupts just to be on the save side
	//TODO do we need this?
	//SCB_ICSR = ICSR_PENDSVCLR;

	// Disable all interrupts
	int i;
	for (i = 0; i < 8; i++)
		NVIC->ICER[i] = NVIC->IABR[i];

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);

	// set stack pointer as in application's vector table
	__set_MSP((u32)(ApplicationAddress[0]));
	Jump_To_Application();
}

int main(void)
{

#ifdef USE_ITM
	itm_init();
	printf("STM32F4xx Blinkenlights Demo\n");
#endif

	jumpToApp(FLASH_USER_BASE);
	while(1);
}
