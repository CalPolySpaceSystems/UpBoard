/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <hal/hal_spi.h>
#include <stdint.h>
#include <spi.h>
#include <pio.h>

/*
 * Struct to hold other SPI settings specific to the SAM3X
 *
 */ 
struct spi_cfg {

    uint8_t fault_detect;
    uint8_t loopback;
    /* TODO: Include settings for the builtin chip selects */

};

/**
 * Initialize the SPI, given by spi_num.
 *
 * @param spi_num The number of the SPI to initialize
 * @param cfg HW/MCU specific configuration,
 *            passed to the underlying implementation, providing extra
 *            configuration.
 * @param spi_type SPI type (master or slave)
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int hal_spi_init(int spi_num, void *cfg, uint8_t spi_type){

    if (spi_num){

        return -1;
    }
    
    spi_set_writeprotect(SPI0,0);

    spi_enable_clock(SPI0);
    pio_set_peripheral(PIOA,PIO_PERIPH_A,(7<<25));
    spi_reset(SPI0);

    // Slave mode
    if (spi_type){

    }

    // Master modestruct spi_cfg
    else{
        spi_set_master_mode(SPI0);

        // Toggle fault detect
        if (((struct spi_cfg *)cfg)->fault_detect){
            spi_enable_mode_fault_detect(SPI0);
        }
        else{
            spi_disable_mode_fault_detect(SPI0);
        }

        // Toggle loopback
        if (((struct spi_cfg *)cfg)->loopback){
            spi_enable_loopback(SPI0);
        }
        else{
            spi_disable_loopback(SPI0);
        }

        /* TODO: If statements for builtin CS settings */
        /*
	    spi_set_fixed_peripheral_select(SPI0);p_spi
	    spi_disable_peripheral_select_decode(SPI0);
	    spi_set_dspi_enable(SPI0);elay_between_chip_select(SPI0, cfg->cs_delay);
        */
    }

    spi_set_writeprotect(SPI0,1);
    return 0;
}

/**
 * Configure the spi. Must be called after the spi is initialized (after
 * hal_spi_init is called) and when the spi is disabled (user must call
 * hal_spi_disable if the spi has been enabled through hal_spi_enable prior
 * to calling this function). Can also be used to reconfigure an initialized
 * SPI (assuming it is disabled as described previously).
 *
 * @param spi_num The number of the SPI to configure.
 * @param psettings The settings to configure this SPI with
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int hal_spi_config(int spi_num, struct hal_spi_settings *psettings){

    int rc;
    spi_set_writeprotect(SPI0,0);

    // Set clock polarity and active level from data mode
    switch (psettings->data_mode) {

        case HAL_SPI_MODE0: 
            spi_set_clock_polarity(SPI0,0,0);
            spi_set_clock_phase(SPI0,0,0);

        case 2:
            spi_set_clock_polarity(SPI0,0,0);
            spi_set_clock_phase(SPI0,0,1);

        case 3:
            spi_set_clock_polarity(SPI0,0,1);
            spi_set_clock_phase(SPI0,0,0);

        case 4:
            spi_set_clock_polarity(SPI0,0,1);
            spi_set_clock_phase(SPI0,0,1);

        default:
            spi_set_writeprotect(SPI0,1);
            return -1;

    }

    // Set word size (bits per transfer)
    spi_set_bits_per_transfer(SPI0,0,psettings->word_size);

    // Set baudrate
    int16_t div = spi_calc_baudrate_div(psettings->baudrate,84000000);
    rc = spi_set_baudrate_div(SPI0,0,div);

    spi_set_writeprotect(SPI0,1);

    return rc;
}


/**
 * Sets the txrx callback (executed at interrupt context) when the
 * buffer is transferred by the master or the slave using the non-blocking API.
 * Cannot be called when the spi is enabled. This callback will also be called
 * when chip select is de-asserted on the slave.
 *
 * NOTE: This callback is only used for the non-blocking interface and must
 * be called prior to using the non-blocking API.
 *
 * @param spi_num   SPI interface on which to set callback
 * @param txrx      Callback function
 * @param arg       Argument to be passed to callback function    
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int hal_spi_set_txrx_cb(int spi_num, hal_spi_txrx_cb txrx_cb, void *arg);

/**
 * Enables the SPI. This does not start a transmit or receive operation;
 * it is used for power mgmt. Cannot be called when a SPI transfer is in
 * progress.
 *
 * @param spi_num
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int hal_spi_enable(int spi_num){


    spi_set_writeprotect(SPI0,0);

    // enable clock
    spi_enable(SPI0);

    spi_set_writeprotect(SPI0,1);

    return 0;
}

/**
 * Disables the SPI. Used for power mgmt. It will halt any current SPI transfers
 * in progress.
 *
 * @param spi_num
 *
 * @return int 0 on success, non-zero 
    spi_set_writeprotect(SPI0,0);

    spi_set_writeprotect(SPI0,1);error code on failure.
 */
int hal_spi_disable(int spi_num){

    spi_set_writeprotect(SPI0,0);

    spi_disable(SPI0);

    spi_set_writeprotect(SPI0,1);

    return 0;

}

/**
 * Blocking call to send a value on the SPI. Returns the value received from the
 * SPI slave.
 *
 * MASTER: Sends the value and returns the received value from the slave.
 * SLAVE: Invalid API. Returns 0xFFFF
 *
 * @param spi_num   Spi interface to use
 * @param val       Value to send
 *
 * @return uint16_t Value received on SPI interface from slave. Returns 0xFFFF
 * if called when the SPI is configured to be a slave
 */
uint16_t hal_spi_tx_val(int spi_num, uint16_t val){

    spi_set_writeprotect(SPI0,0);
    
    uint8_t cs = 0;
    uint16_t data;

    spi_write(SPI0,val,cs,1);

    if (spi_read(SPI0,&data,NULL)){
        return 0;
    }

    spi_set_writeprotect(SPI0,1);

    return data;

}

/**
 * Blocking interface to send a buffer and store the received values from the
 * slave. The transmit and receive buffers are either arrays of 8-bit (uint8_t)
 * values or 16-bit values depending on whether the spi is configured for 8 bit
 * data or more than 8 bits per value. The 'cnt' parameter is the number of
 * 8-bit or 16-bit values. Thus, if 'cnt' is 10, txbuf/rxbuf would point to an
 * array of size 10 (in bytes) if the SPI is using 8-bit data; otherwise
 * txbuf/rxbuf would point to an array of size 20 bytes (ten, uint16_t values).
 *
 * NOTE: these buffers are in the native endian-ness of the platform.
 *
 *     MASTER: master sends all the values in the buffer and stores the 
 *             values in the receive buffer if rxbuf is not NULL.
 *             The txbuf parameter cannot be NULL.
 *     SLAVE: cannot be called for a slave; returns -1
 *
 * @param spi_num   SPI interface to use
 * @param txbuf     Pointer to buffer where values to transmit are stored.
 * @param rxbuf     Pointer to buffer to store values received from peer.
 * @param cnt       Number of 8-bit or 16-bit values to be transferred.
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int hal_spi_txrx(int spi_num, void *txbuf, void *rxbuf, int cnt);

/*
{

    // Send data until txbuf is empty
    for (int i=0;i<(cnt-1);i++){

        spi_write(SPI0,((uint8_t *)txbuf)[i]);

    }
    
    // Send last item, is last transfer
    int rc = spi_write(SPI0,txbuf[(cnt-1)]);

    // Read values in to rx buffer



    // if timeout, return what you have
    return rc;
}
*/


/*
 * Non-blocking interface to send a buffer and store received values. Can be
 * used for both master and slave SPI types. The user must configure the
 * callback (using hal_spi_set_txrx_cb); the txrx callback is executed at
 * interrupt context when the buffer is sent.
 *
 * The transmit and receive buffers are either arrays of 8-bit (uint8_t)
 * values or 16-bit values depending on whether the spi is configured for 8 bit
 * data or more than 8 bits per value. The 'cnt' parameter is the number of
 * 8-bit or 16-bit values. Thus, if 'cnt' is 10, txbuf/rxbuf would point to an
 * array of size 10 (in bytes) if the SPI is using 8-bit data; otherwise
 * txbuf/rxbuf would point to an array of size 20 bytes (ten, uint16_t values).
 *
 * NOTE: these buffers are in the native endian-ness of the platform.
 *
 *     MASTER: master sends all the values in the buffer and stores the
 *             stores the values in the receive buffer if rxbuf is not NULL.
 *             The txbuf parameter cannot be NULL
 *     SLAVE: Slave "preloads" the data to be sent to the master (values
 *            stored in txbuf) and places received data from master in rxbuf
 *            (if not NULL). The txrx callback occurs when len values are
 *            transferred or master de-asserts chip select. If txbuf is NULL,
 *            the slave transfers its default byte. Both rxbuf and txbuf cannot
 *            be NULL.
 *
 * @param spi_num   SPI interface to use
 * @param txbuf     Pointer to buffer where values to transmit are stored.
 * @param rxbuf     Pointer to buffer to store values received from peer.
 * @param cnt       Number of 8-bit or 16-bit values to be transferred.
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int hal_spi_txrx_noblock(int spi_num, void *txbuf, void *rxbuf, int cnt);

/**
 * Sets the default value transferred by the slave. Not valid for master
 *
 * @param spi_num SPI interface to use
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int hal_spi_slave_set_def_tx_val(int spi_num, uint16_t val);

/**
 * This aborts the current transfer but keeps the spi enabled.
 *
 * @param spi_num   SPI interface on which transfer should be aborted.
 *
 * @return int 0 on success, non-zero error code on failure.
 *
 * NOTE: does not return an error if no transfer was in progress.
 */
int hal_spi_abort(int spi_num);

/** Utility functions; defined once for all MCUs. */
int hal_spi_data_mode_breakout(uint8_t data_mode,int *out_cpol, int *out_cpha);

