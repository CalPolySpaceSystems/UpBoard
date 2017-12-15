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
#include <hal/hal_i2c.h>
#include <hal/hal_gpio.h>

// platform specific
#include <sam3x8e.h> //???
#include <twi.h>


int hal_i2c_init(uint8_t i2c_num, void *usercfg)
{
	// twi_master_init
}

int hal_i2c_master_write(uint8_t i2c_num, struct hal_i2c_master_data *data,
	uint32_t timo, uint8_t last_op)
{
	// master write
}

int hal_i2c_master_read(uint8_t i2c_num, struct hal_i2c_master_data *pdata,
	uint32_t timo, uint8_t last_op)
{
	// master read
}

int hal_i2c_master_probe(uint8_t i2c_num, uint8_t address, uint32_t timo)
{
	// owo what's this?
}
