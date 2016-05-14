/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stm32f30x_rcc.h>
#include <stm32f30x_gpio.h>

void led_init(void)
{
	GPIO_InitTypeDef gpio_conf;

	/* Clear the GPIO Structure */
	GPIO_StructInit(&gpio_conf);

	/* Set up the LED */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	gpio_conf.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_conf.GPIO_Mode = GPIO_Mode_OUT;
	gpio_conf.GPIO_OType = GPIO_OType_PP;
	gpio_conf.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &gpio_conf);
}

void led_deinit(void)
{
	GPIO_DeInit(GPIOA);

}

void led_toggle(void)
{
	const uint8_t led_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);

	if (led_state) {
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	} else {
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
	}
}


