/* STM32F4xx Hello World project (via blinkenlights)
 *
 * Jeff Ciesielski <jeff.ciesielski@gmail.com>
 */
#include <stm32f4xx_misc.h>

#ifdef USE_ITM
#include <itm.h>
#endif

#include <stdio.h>


int main(void)
{

#ifdef USE_ITM
	itm_init();
	printf("STM32F4xx Blinkenlights Demo\n");
#endif


	while(1);
}
