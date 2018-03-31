#include <assert.h>
#include <string.h>

#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <hal/hal_uart.h>
#include <hal/hal_spi.h>
#include <hal/hal_i2c.h>

#include <sysinit/sysinit.h>
#include <os/os.h>

#include <bsp/bsp.h>

#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

/**
 * main
 *
 * This application tests the timing functionality of mynewt on the SAM3X8E
 *
 * @return int NOTE: this function should never return!
 */
volatile int led_dir;
volatile int loops;
volatile int test;

int main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    hal_bsp_init();
    sysinit();

    led_dir = 1;
    loops = 0;

    __asm__("bkpt");
    
    while(1) {
        //__asm__("bkpt");
        test = os_cputime_get32();
        loops++;
        //os_time_delay(OS_TICKS_PER_SEC);
        os_cputime_delay_ticks(1000000);
        led_dir = !led_dir;
        hal_gpio_write(LED_1_PIN, led_dir);
    }

    assert(0);

    return rc;
}
