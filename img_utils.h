#ifndef _IMG_UTILS_H_
#define _IMG_UTILS_H_

#include <app_info.h>

#define FLASH_SIZE_REG	(0x1FFF7A22)
#define FLASH_SIZE      (((*(uint32_t*)FLASH_SIZE_REG) & 0xffff) * 1024)
#define FLASH_END       (0x08000000 + FLASH_SIZE)

struct app_info_block *scan_for_app(void);
void jump_to_app(uint32_t address);
#endif
