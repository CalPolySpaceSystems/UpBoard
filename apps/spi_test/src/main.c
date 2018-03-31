#include <assert.h>
#include <string.h>

#include <sysinit/sysinit.h>
#include <os/os.h>
#include <bsp/bsp.h>

#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <hal/hal_spi.h>

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

uint8_t spi_rx[3];
uint8_t spi_tx = 0x9F;

int main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    hal_bsp_init();
    sysinit();

    // Init Flash CS pin
    hal_gpio_init_out(FLASH_CS_PIN,1);

    spi_hw_cfg(HAL_SPI_0,false,false,0);

    // Init SPI
    hal_spi_init(HAL_SPI_0, NULL,0);

    // Configure SPI

    __asm__("bkpt");
    //hal_spi_config();
    

    // Enable SPI
    hal_spi_enable(0);

    while(1) {

        // Get the transmission
        hal_gpio_write(FLASH_CS_PIN, 0);
        
        __asm__("bkpt");

        os_cputime_delay_ticks(5);

        hal_spi_txrx(HAL_SPI_0,&spi_tx,&spi_rx,1);

        os_cputime_delay_ticks(5);

        hal_gpio_write(FLASH_CS_PIN, 1);

        os_cputime_delay_ticks(2000000);
        __asm__("bkpt");

    }

    assert(0);

    return rc;
}
