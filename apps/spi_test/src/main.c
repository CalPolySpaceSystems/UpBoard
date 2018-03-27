#include <assert.h>
#include <string.h>
#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <sysinit/sysinit.h>
#include <os/os.h>
#include <hal/hal_spi.h>


#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#define SPI_0 0

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
volatile int test;

uint8_t spi_rx;
uint8_t spi_tx = 0b10011101;

//struct spi_cfg spicfg;

int main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    hal_bsp_init();
    sysinit();
    hal_spi_init(SPI_0,NULL,0);

    hal_gpio_init_out(LED_BLINK_PIN, 0);

    //hal_spi_config();
    __asm__("bkpt");
    hal_spi_enable(SPI_0);

    while(1) {
        __asm__("bkpt");
        spi_rx = hal_spi_tx_val(SPI_0,spi_tx);


        //__asm__("bkpt");
        if(spi_rx == spi_tx){
            hal_gpio_write(LED_BLINK_PIN,1);

        }

        os_cputime_delay_ticks(1000000);
        hal_gpio_write(LED_BLINK_PIN,0);
        spi_rx = 0;
        os_cputime_delay_ticks(1000000);
    }

    assert(0);

    return rc;
}
