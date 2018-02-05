#include<assert.h>
#include <os/os_cputime.h>

#include "mcu/sam3x8.h"
#include "mcu/sam3x8_hal.h"
#include "hal/hal_flash.h"
#include "hal/hal_timer.h"

const struct hal_flash* hal_bsp_flash_dev(uint8_t id)
{
    /*
     * Internal flash mapped to id 0.
     */
    if (id != 0) {
        return NULL;
    }

    return &sam3x8_flash_dev_1;
}

void hal_bsp_init(void) {
    int rc;
    struct sam3x8_timer_cfg tmr_cfg;

    tmr_cfg.hwtimer = TC0;
    tmr_cfg.irq = TC1_IRQn;
    rc = hal_timer_init(0, &tmr_cfg);
    assert(rc == 0);

    rc = os_cputime_init(MYNEWT_VAL(OS_CPUTIME_FREQ));
    assert(rc == 0);
}
