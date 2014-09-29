#include <stdbool.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

bool bootmode_upgrade_requested(void)
{
	bool ret = false;
	uint8_t bit = 0;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_Init(GPIOA,&GPIO_InitStructure);

	/* Check the state of the GPI Pin to see if a load was requested */
	bit = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8);

	if (bit == 0)
		ret = true;

	GPIO_DeInit(GPIOA);

	return ret;
}
