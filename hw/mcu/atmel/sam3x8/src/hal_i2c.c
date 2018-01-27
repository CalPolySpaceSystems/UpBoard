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

    uint8_t rc; // return code

    assert(usercfg != NULL);
    
    // May need to clear i2c bus before init
    if (i2c_num == 0) {
        rc = twi_master_init(TWI0, usercfg);
    } else {
        return -1;
    }

    /*
    // Original twi options struct based on usercfg
    struct twi_options {
        //! MCK for TWI.
        uint32_t master_clk = usercfg->master_clk;
        //! The baud rate of the TWI bus.
        uint32_t speed = usercfg->baud;
        //! The desired address.
        uint8_t chip = usercfg->chip_addr;
        //! SMBUS mode (set 1 to use SMBUS quick command, otherwise don't).
        uint8_t smbus = 0;
    } twi_options_t;
    */

    return rc;
}

int hal_i2c_master_write(uint8_t i2c_num, struct hal_i2c_master_data *data,
    uint32_t timo, uint8_t last_op)
{
    uint8_t rc; // return code

    struct twi_packet pkt;

    pkt.addr[0] = data->address;
    pkt.length = data->len;
    pkt.buffer = data->buffer;

    // If last_op is zero, it means we don't put a stop at the end
    
    rc = twi_master_write(TWI0, &pkt);

    return rc;
}

int hal_i2c_master_read(uint8_t i2c_num, struct hal_i2c_master_data *data,
    uint32_t timo, uint8_t last_op)
{
    // master read
    uint8_t rc; // return code
    
    struct twi_packet pkt;

    pkt.addr[0] = data->address;
    pkt.length = data->len;
    pkt.buffer = data->buffer;

    rc = twi_master_read(TWI0, &pkt);

    return rc;
}

int hal_i2c_master_probe(uint8_t i2c_num, uint8_t address, uint32_t timo)
{
    // owo what's this?
    uint8_t rc=0; // return code

    // twi_probe();

    return rc;
}
