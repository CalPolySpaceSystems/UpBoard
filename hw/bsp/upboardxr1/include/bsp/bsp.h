#ifndef __UPBOARD_X_R1_BSP_H__
#define __UPBOARD_X_R1_BSP_H__

#include <inttypes.h>

#include <syscfg/syscfg.h>

#ifdef __cplusplus
extern "C" {
#endif


extern uint8_t _ram_start;
// TODO: This can be expanded if necessary
#define RAM_SIZE 0x00008000

/* Define pin functions */
#define LED_1_PIN           (103)
#define LED_2_PIN           (104)
#define LED_FAULT_PIN       (59)
#define LS_PIN              (85)

// Flash pins
#define FLASH_HOLD_PIN      (86)
#define FLASH_CS_PIN        (87)
#define FLASH_WP_PIN        (88)

// SD Chip Select

// Communications
#define HAL_SPI_0           (0)

#ifdef __cplusplus
}
#endif

#endif
