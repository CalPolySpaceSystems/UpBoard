#include <assert.h>
#include <string.h>
#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <sysinit/sysinit.h>
#include <os/os.h>
#include <hal/hal_uart.h>
#include <bsp/bsp.h>

#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

/**
 * main
 *
 * This application does nothing, it's primary goal is to test compiling with the sam3x8 mcu
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

    hal_gpio_init_out(LED_1_PIN, 0);
    hal_gpio_init_out(LED_2_PIN, 0);
    hal_gpio_init_out(LED_FAULT_PIN, 0);
    hal_gpio_init_out(LS_PIN, 0);
    led_dir = 1;
    loops = 0;

    int f_test = 1/loops;

    while(1) {
        //__asm__("bkpt");
        test = os_cputime_get32();
        loops++;
        f_test++;
        //os_time_delay(OS_TICKS_PER_SEC);
        os_cputime_delay_ticks(1000000);
        led_dir = !led_dir;
        hal_gpio_write(LED_1_PIN, led_dir);
    }

    assert(0);

    return rc;
}
