#ifndef __UPBOARD_X_BSP_H__
#define __UPBOARD_X_BSP_H__

#include <inttypes.h>
#include <mcu/mcu.h>

#include <syscfg/syscfg.h>

#ifdef __cplusplus
extern "C" {
#endif


extern uint8_t _ram_start;
// TODO: This can be expanded if necessary
#define RAM_SIZE 0x00008000

#ifdef __cplusplus
}
#endif

#endif
