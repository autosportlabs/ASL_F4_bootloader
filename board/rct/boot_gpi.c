
#include <stdbool.h>
#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>

bool boot_gpi_asserted(void)
{	bool ret = false;
	uint8_t bit = 0;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Check the state of the GPI Pin to see if a load was requested */
	bit = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15);

	if (bit == 0)
		ret = true;

	GPIO_DeInit(GPIOC);

	return ret;
}
