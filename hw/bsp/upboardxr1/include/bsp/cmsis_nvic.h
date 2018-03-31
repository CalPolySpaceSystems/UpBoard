#ifndef CMSIS_NVIC_H
#define CMSIS_NVIC_H

#include <stdint.h>


/* TODO: Are these right? */
#define NVIC_NUM_VECTORS      (16 + 29)   // CORE + MCU Peripherals
#define NVIC_USER_IRQ_OFFSET  16

#include <mcu/cortex_m3.h>

#ifdef __cplusplus
extern "C" {
#endif

void NVIC_Relocate(void);
void NVIC_SetVector(IRQn_Type IRQn, uint32_t vector);
uint32_t NVIC_GetVector(IRQn_Type IRQn);

#ifdef __cplusplus
}
#endif

#endif
