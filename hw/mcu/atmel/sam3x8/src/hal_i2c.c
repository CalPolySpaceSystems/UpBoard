# if 0
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

// hal
#include "hal/hal_i2c.h"
#include "hal/hal_gpio.h"

// platform specific
#include <sam3x8e.h>
#include <twi.h>

// mcu
#include "mcu/hal_i2c.h"

#define i2c_0 1;
#define i2c_1 2;

int hal_i2c_init(uint8_t i2c_num, void *usercfg)
{

    uint8_t rc; // return code

	assert(usercfg != NULL);
	
    // Now that settings are in place, can initialize through twi
	if (i2c_num = i2c_0){
		rc = twi_master_init(TWI0, &usercfg);
	else{
		return -1;
	}
	
    return rc;
}

int hal_i2c_master_write(uint8_t i2c_num, struct hal_i2c_master_data *pdata,
    uint32_t timo, uint8_t last_op)
{
    uint8_t rc; // return code
	struct twi_packet pkt;
	
	// update the twi_packet struct
	pkt.address = pdata->address;
    pkt.data_length = pdata->len;
    pkt.data = pdata->buffer;
	
	// actually write the byte, referencing TWI0 and the pkt
	
    rc = twi_master_write(TWI0, &pkt);
   
    return rc;
}

int hal_i2c_master_read(uint8_t i2c_num, struct hal_i2c_master_data *pdata,
    uint32_t timo, uint8_t last_op)
{
    // master read
    uint8_t rc; // return code
	struct twi_packet pkt;
	
	// update the twi_packet struct
	pkt.addr = pdata->address;
    pkt.addr_length = pdata->len;
    pkt.data = pdata->buffer;
	
	// actually read the byte, referencing TWI0 and the pkt
    rc = twi_master_read(TWI0, &pkt);
   
    return rc;
}

int hal_i2c_master_probe(uint8_t i2c_num, uint8_t address, uint32_t timo)
{
    // owo what's this?
    uint8_t rc; // return code
	
	twi_probe();
	
    return rc;
}

# endif
