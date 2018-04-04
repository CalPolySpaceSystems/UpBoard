#include <assert.h>
#include <string.h>

#include <mcu/hal_i2c.h>

#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <hal/hal_i2c.h>

#include <bsp/bsp.h>
#include <sysinit/sysinit.h>
#include <os/os.h>

#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#define LSM_DEV_ADDR    (0x6A)
#define LSM_WHOAMI      (0x0F)

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

uint8_t i2c_rxtx;
uint8_t i2c_addr = 0x6A;

struct hal_i2c_master_data lsm_data;
foo_t bar;


int main(int argc, char **argv)
{
    uint8_t rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    // Startup
    sysinit();
    hal_bsp_init();

    // Configure and enable i2c
    /* TODO: Find the actual definition of the mck frequency */
    i2c_hw_cfg(&bar,7000000,10000);
    
    /* TODO: De-configure TWI */
    hal_i2c_init(1,&bar);
    //__asm__("bkpt");

    // Set buffer location
    lsm_data.buffer = &i2c_rxtx;

    // Set slave address
    lsm_data.address = LSM_DEV_ADDR;

    // Set length
    lsm_data.len = 1;

    while(1) {
        
        //__asm__("bkpt");

        // Load buffer with value to transmit
        i2c_rxtx = LSM_WHOAMI;

        // Transmit command and wait for response
        rc = hal_i2c_master_write(1,&lsm_data,30000,true);
        //__asm__("bkpt");
        if(rc){
            hal_gpio_write(LED_FAULT_PIN,1);
        }
        
        rc = hal_i2c_master_read(1,&lsm_data,30000,true);
        
        if(rc){
           hal_gpio_write(LED_2_PIN,1);
        }
        
        /*

        rc = hal_i2c_master_probe(1,LSM_DEV_ADDR,30000);
        if (rc){
             __asm__("bkpt");
        }
        */
        //__asm__("bkpt");
        loops++;
        os_cputime_delay_ticks(1000000);
        hal_gpio_write(LED_2_PIN,0);
        os_cputime_delay_ticks(1000000);
        hal_gpio_write(LED_FAULT_PIN,0);
        //__asm__("bkpt");
    }

    assert(0);

    return rc;
}
