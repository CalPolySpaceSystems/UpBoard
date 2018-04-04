/*
 * HAL I2C source for sam3x8e
 *
 */

/* Include */

// general
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// idk
#include <syscfg/syscfg.h>

// hw specific settings
#include "mcu/hal_i2c.h"

// hal
#include "hal/hal_i2c.h"
#include "hal/hal_gpio.h"

// platform specific
#include <sam3x8e.h>
#include <twi.h>
#include <pio.h>
#include <pmc.h>

#define i2c_0 0;
#define i2c_1 1;

static twi_packet_t i2c_pkt[2];

/**
 * Initialize a new i2c device with the I2C number.
 *
 * @param i2c_num The number of the I2C device being initialized
 * @param cfg The hardware specific configuration structure to configure
 *            the I2C with.  This includes things like pin configuration.
 *
 * @return 0 on success, and non-zero error code on failure
 */
int hal_i2c_init(uint8_t i2c_num, void *usercfg)
{

    uint8_t rc; // return code

    // Error if no configuration is set up
	assert(usercfg != NULL);

    // Set write protection statuses
    pmc_set_writeprotect(0);

    // Enable the twi, peripherals, and clock
    
    // TWI1 
	if (i2c_num){
        pmc_enable_periph_clk(23);
        pio_set_writeprotect(PIOA,0); 
        pio_set_peripheral(PIOA,PIO_PERIPH_A,(3<<17));
        pio_clear(PIOA,(3<<17));
		//twi_set_write_protection(TWI0,0);
        rc = twi_master_init(TWI1, (twi_options_t*)usercfg);
        pio_set_writeprotect(PIOA,1);
    }

    // TWI0
	else {
        pmc_enable_periph_clk(22);
        pio_set_writeprotect(PIOB,0); 
        pio_set_peripheral(PIOB,PIO_PERIPH_A,(3<<12));
        pio_clear(PIOB,(3<<12));
        //twi_set_write_protection(TWI1,0);
		rc = twi_master_init(TWI0, (twi_options_t*)usercfg);
        pio_set_writeprotect(PIOB,1);

	}
	
    // Set write protection statuses
    pmc_set_writeprotect(1);

    return rc;
}

/**
 * Sends a start condition and writes <len> bytes of data on the i2c bus.
 * This API does NOT issue a stop condition unless `last_op` is set to `1`.
 * You must stop the bus after successful or unsuccessful write attempts.
 * This API is blocking until an error or NaK occurs. Timeout is platform
 * dependent.
 *
 * @param i2c_num The number of the I2C device being written to
 * @param pdata The data to write to the I2C bus
 * @param timeout How long to wait for transaction to complete in ticks
 * @param last_op Master should send a STOP at the end to signify end of
 *        transaction.
 *
 * @return 0 on success, and non-zero error code on failure
 */
int hal_i2c_master_write(uint8_t i2c_num, struct hal_i2c_master_data *pdata,
    uint32_t timo, uint8_t last_op)
{
    uint8_t rc; // return code

	// update the i2c_pkt struct
	i2c_pkt[i2c_num].addr[0] = pdata->address;
    i2c_pkt[i2c_num].length = pdata->len;
    i2c_pkt[i2c_num].buffer = pdata->buffer;
	
	// actually write the byte, referencing TWI0 and the pkt
    if (i2c_num)
    {
        rc = twi_master_write(TWI1, &i2c_pkt[1]);
    }

    else{
        rc = twi_master_write(TWI0, &i2c_pkt[0]);
    }
    
    return rc;
}

/**
 * Sends a start condition and reads <len> bytes of data on the i2c bus.
 * This API does NOT issue a stop condition unless `last_op` is set to `1`.
 * You must stop the bus after successful or unsuccessful write attempts.
 * This API is blocking until an error or NaK occurs. Timeout is platform
 * dependent.
 *
 * @param i2c_num The number of the I2C device being written to
 * @param pdata The location to place read data
 * @param timeout How long to wait for transaction to complete in ticks
 * @param last_op Master should send a STOP at the end to signify end of
 *        transaction.
 *
 * @return 0 on success, and non-zero error code on failure
 */
int hal_i2c_master_read(uint8_t i2c_num, struct hal_i2c_master_data *pdata,
    uint32_t timo, uint8_t last_op)
{
    // master read
    uint8_t rc; // return code

    	// update the i2c_pkt struct
	i2c_pkt[i2c_num].addr[0] = pdata->address;
    i2c_pkt[i2c_num].length = pdata->len;
    i2c_pkt[i2c_num].buffer = pdata->buffer;

	// actually write the byte, referencing TWI0 and the pkt
    if (i2c_num)
    {
        rc = twi_master_read(TWI1, &i2c_pkt[1]);
    }

    else{
        rc = twi_master_read(TWI0, &i2c_pkt[0]);
    }

    return rc;
    
}

/**
 * Probes the i2c bus for a device with this address.  THIS API
 * issues a start condition, probes the address using a read
 * command and issues a stop condition.
 *
 * @param i2c_num The number of the I2C to probe
 * @param address The address to probe for
 * @param timeout How long to wait for transaction to complete in ticks
 *
 * @return 0 on success, non-zero error code on failure
 */
int hal_i2c_master_probe(uint8_t i2c_num, uint8_t address, uint32_t timo)
{
    uint8_t rc; // return code
	
    // actually write the byte, referencing TWI0 and the pkt
    if (i2c_num)
    {
        rc = twi_probe(TWI1, address);
    }

    else{
        rc = twi_probe(TWI0, address);
    }

    return rc;
}

void i2c_hw_cfg	(void *usercfg, uint32_t mclk, uint32_t baud){

    ((twi_options_t*)usercfg)->master_clk = mclk;
    ((twi_options_t*)usercfg)->speed = baud;
    
}
