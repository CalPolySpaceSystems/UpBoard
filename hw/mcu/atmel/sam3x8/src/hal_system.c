#include <mcu/cortex_m3.h>
#include "hal/hal_system.h"
#include <stdlib.h>

void hal_system_reset(void)
{
    while (1) {
        if (hal_debugger_connected()) {
            /*
             * If debugger is attached, breakpoint here.
             */
            __asm__("bkpt");
        }

           /* TODO: from the M0, investigate for M3 */
           /* Cortex-M0+ Core Debug Registers (DCB registers, SHCSR, and DFSR)
           are only accessible over DAP and not via processor. Therefore
           they are not covered by the Cortex-M0 header file. */

        NVIC_SystemReset();
    }
}

int hal_debugger_connected(void)
{
    return DSU->STATUSB.reg & DSU_STATUSB_DBGPRES;
}

uint32_t HAL_GetTick(void)
{
    return 0;
}

int HAL_InitTick (uint32_t TickPriority)
{
    return 0;
}
