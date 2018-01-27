#include <assert.h>
#include <string.h>
#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <sysinit/sysinit.h>
#include <os/os.h>
#include <hal/hal_uart.h>


#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#define LED_BLINK_PIN (59)

/**
 * main
 *
 * This application does nothing, it's primary goal is to test compiling with the sam3x8 mcu
 *
 * @return int NOTE: this function should never return!
 */
volatile int led_dir;
volatile int loops;


int tx_funct(void *garbage){
    return (int) 'f';
}

int main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    hal_bsp_init();
    sysinit();
    hal_gpio_init_out(LED_BLINK_PIN, 1);
    led_dir = 1;
    loops = 0;
    #define USART_USING 3
    if (hal_uart_init(USART_USING, NULL) == -1){
        assert(0);
    }
    if (hal_uart_config(USART_USING, 9600, 8, 2, HAL_UART_PARITY_NONE, HAL_UART_FLOW_CTL_NONE)){
        assert(0);
    }
    if (hal_uart_init_cbs(USART_USING, &tx_funct, &tx_funct, &tx_funct, &tx_funct)){
        assert(0);
    }
    while(1) {
        hal_uart_start_tx(USART_USING);
        /*loops++;
        os_time_delay(OS_TICKS_PER_SEC);
        led_dir = !led_dir;
        hal_gpio_write(LED_BLINK_PIN, led_dir);*/
    }

    assert(0);

    return rc;
}
