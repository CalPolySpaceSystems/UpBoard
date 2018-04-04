#include <assert.h>
#include <os/os_cputime.h>

#include "mcu/sam3x8.h"
#include "mcu/sam3x8_hal.h"

#include "bsp/bsp.h"

#include "hal/hal_bsp.h"
#include "hal/hal_flash.h"
#include "hal/hal_timer.h"
#include "hal/hal_gpio.h"
#include "hal/hal_i2c.h"

#include "syscfg/syscfg.h"
#include "sysinit/sysinit.h"

static const struct hal_bsp_mem_dump dump_cfg[] = {
    [0] = {
	.hbmd_start = &_ram_start,
        .hbmd_size = RAM_SIZE
    }
};

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

const struct hal_bsp_mem_dump *
hal_bsp_core_dump(int *area_cnt)
{
    *area_cnt = sizeof(dump_cfg) / sizeof(dump_cfg[0]);
    return dump_cfg;
}

/* I2C Struct 
static twi_options_t i2c_cfg0 = {
    .master_clk = 84000000;
    .speed = 9600;
    .chip = 0;
    .smbus = 0;
};
*/

uint32_t hal_bsp_get_nvic_priority(int irq_num, uint32_t pri)
{
    /* Add any interrupt priorities configured by the bsp here */
    return pri;
}

void hal_bsp_init(void) {
    int rc;
    struct sam3x8_timer_cfg tmr_cfg;

    tmr_cfg.hwtimer = TC0;
    tmr_cfg.irq = TC1_IRQn;
    tmr_cfg.irq_ps = TC0_IRQn;
    rc = hal_timer_init(0, &tmr_cfg);
    assert(rc == 0);

    rc = os_cputime_init(MYNEWT_VAL(OS_CPUTIME_FREQ));

    assert(rc == 0);

    // Startup gpio pins
    rc &= hal_gpio_init_out(LED_1_PIN, 0);
    rc &= hal_gpio_init_out(LED_2_PIN, 0);
    rc &= hal_gpio_init_out(LED_FAULT_PIN, 0);
    rc &= hal_gpio_init_out(LS_PIN, 0);
    
    assert(rc == 0);

}
