#include <uart.h>
#include <usart.h>
#include <hal/hal_uart.h>
#include <stdlib.h>
#include <assert.h>
#include <sysclk.h>
#include <pio.h>

/*
 The board has 1 UART and 3 USARTS
*/
#define UART_COUNT (4)

#define TX_BUFFER_SIZE (8)

union sam_uart_options_union {
    sam_usart_opt_t usart_options;
    sam_uart_opt_t uart_options;
};

struct hal_uart {
    void * uart;
    uint8_t u_open;
    uint8_t tx_on;
    uint32_t rxdata;
    uint8_t uart_rxdata;
    uint8_t txdata[TX_BUFFER_SIZE];
    /* Called with the read data */
    hal_uart_rx_char u_rx_func;
    hal_uart_tx_char u_tx_func;
    hal_uart_tx_done u_tx_done;
    void *u_func_arg;
    union sam_uart_options_union options;
};
typedef struct hal_uart hal_uart_t;
static hal_uart_t uarts[UART_COUNT] = {'\0'};

int fill_tx_buffer(hal_uart_t *u){
    int i, val;
    for (i = 0; i < TX_BUFFER_SIZE; i++){
        val = u->u_tx_func(u->u_func_arg);
        if (val < 0){
            break;
        }
        u->txdata[i] = val;
    }
    return i;
}

/* Internal helper function */
void *translate_port_to_uart(int port){
    switch (port){
        case 0:
            return USART0;
        break;
        case 1:
            return USART1;
        break;
        case 2:
            return USART2;
        break;
        case 3:
            return UART;
        break;
    }
    assert(false);
    return NULL;
}

int is_usart(void *uart){
    if (uart == UART){
        return 0;
    }
    return 1;
}

/**
 * Initialize the HAL uart.
 *
 * @param uart  The uart number to configure
 * @param cfg   Hardware specific uart configuration.  This is passed from BSP
 *              directly to the MCU specific driver.
 */
int hal_uart_init(int uart, void *cfg){
    /* Default config is console */
    if (uart >= UART_COUNT){
        return -1;
    }
    uarts[uart].uart = translate_port_to_uart(uart);
    /* Potential to copy cfg to options here */

    return 0;
}

int hal_uart_init_cbs(int uart, hal_uart_tx_char tx_func,
  hal_uart_tx_done tx_done, hal_uart_rx_char rx_func, void *arg){
    if (uart >= UART_COUNT){
        return -1;
    }
    uarts[uart].u_tx_done = tx_done;
    uarts[uart].u_rx_func = rx_func;
    uarts[uart].u_tx_func = tx_func;
    uarts[uart].u_func_arg = arg;
    return 0;
}


int hal_usart_config(hal_uart_t *uart, int32_t speed, uint8_t databits, uint8_t stopbits,
                        enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ctl){
    uart->options.usart_options.baudrate = speed;
    /* Set char length */
    switch (databits){
        case 5:
            uart->options.usart_options.char_length = US_MR_CHRL_5_BIT;
        break;
        case 6:
            uart->options.usart_options.char_length = US_MR_CHRL_6_BIT;
        break;
        case 7:
            uart->options.usart_options.char_length = US_MR_CHRL_7_BIT;
        break;
        case 8:
            uart->options.usart_options.char_length = US_MR_CHRL_8_BIT;
        break;
        case 9:
            uart->options.usart_options.char_length = US_MR_MODE9;
        break;
        default:
            /* Unsupported mode */
            return -1;
    }

    /* Set parity mode */
    switch (parity){
        case HAL_UART_PARITY_NONE:
            uart->options.usart_options.parity_type = US_MR_PAR_NO;
        break;
        case HAL_UART_PARITY_ODD:
            uart->options.usart_options.parity_type = US_MR_PAR_ODD;
        break;
        case HAL_UART_PARITY_EVEN:
            uart->options.usart_options.parity_type = US_MR_PAR_EVEN;
        break;
        default:
            /* Unsupported mode (there are a few that usart supports that this interface does not) */
            return -1;
    }

    /* Set stop bits */
    switch(stopbits){
        case 1:
            uart->options.usart_options.stop_bits = US_MR_NBSTOP_1_BIT;
        break;
        case 2:
            uart->options.usart_options.stop_bits = US_MR_NBSTOP_2_BIT;
        break;
        default:
            return -1;
    }   

    /* Ignore flow control, but throw error on illegal argument */
    switch (flow_ctl){
        case HAL_UART_FLOW_CTL_NONE:
        case HAL_UART_FLOW_CTL_RTS_CTS:
            break;
        default:
            return -1;
    }
    uart->u_open = 1;
    return usart_init_rs232(uart->uart, &(uart->options.usart_options), sysclk_get_peripheral_hz());
}

int hal_uart_config_internal(hal_uart_t *uart, int32_t speed, uint8_t databits, uint8_t stopbits,
  enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ctl){
    uart->options.uart_options.ul_baudrate = speed;
    uart->options.uart_options.ul_mck = sysclk_get_peripheral_hz();
    uart->options.uart_options.ul_mode = parity;
    return uart_init(uart->uart, &(uart->options.uart_options));
}

/**
 * hal uart config
 *
 * Applies given configuration to UART.
 */
int hal_uart_config(int uart, int32_t speed, uint8_t databits, uint8_t stopbits,
  enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ctl){
      if (uart >= UART_COUNT){
          return -1;
      }
      void *uart_ptr = uarts[uart].uart;
      if (is_usart(uart_ptr)){
          /* USART configuration */
          /* Assume USART clock and Board are initialized */
        return hal_usart_config(&uarts[uart], speed, databits, stopbits, parity, flow_ctl);
      }else{
          /* UART configuration */
        return hal_uart_config_internal(&uarts[uart], speed, databits, stopbits, parity, flow_ctl);
      }
}

/*
 * Close UART port. Can call hal_uart_config() with different settings after
 * calling this.
 */
int hal_uart_close(int port){
    hal_uart_t *u = &uarts[port];
    if (is_usart(u->uart)){
        usart_disable_rx(u->uart);
        usart_disable_tx(u->uart);
    }else{
        uart_disable(u->uart);
    }
    u->u_open = 0;
    return 0;
}

/**
 * hal uart start tx
 *
 * More data queued for transmission. UART driver will start asking for that
 * data.
 */
void hal_uart_start_tx(int port){
    hal_uart_t *u = &uarts[port];
    int sz, i = 0;
    if (port >= UART_COUNT){
        return;
    }
    /* While there is data to write */
    u->tx_on = 1;
    while ((sz = fill_tx_buffer(u))){
        if (is_usart(u)){
            usart_enable_tx(u->uart);
            for (i = 0; i < sz; i++){
                /* TODO - replace with a sleep */
                while (!usart_is_tx_ready(u->uart));
                usart_write(u->uart, (uint32_t) u->txdata[i]);  
            }
        }else{
            uart_enable_tx(u->uart);
            for (i = 0; i < sz; i++){
                while (!uart_is_tx_ready(u->uart));
                uart_write(u->uart, (uint8_t) u->txdata[i]);
            }
        }
    }

    /* Call back the user tx done function */
    if (u->u_tx_done){
        u->u_tx_done(u->u_func_arg);
    }
    u->tx_on = 0;
}

/**
 * hal uart start rx
 *
 * Upper layers have consumed some data, and are now ready to receive more.
 * This is meaningful after uart_rx_char callback has returned -1 telling
 * that no more data can be accepted.
 */
void hal_uart_start_rx(int port){
    hal_uart_t *u = &uarts[port];
    if (port >= UART_COUNT){
        return;
    }
    if (is_usart(u->uart)){
        usart_enable_rx(u->uart);
        while (!usart_is_rx_buf_end(u->uart)){
            /* TODO - replace with a sleep */
            while(!usart_is_rx_ready(u->uart));
            usart_read(u->uart, &u->rxdata);
            if (u->u_rx_func){
                u->u_rx_func(u->u_func_arg, u->rxdata);
            }
        }
    }else{
        uart_enable_rx(u->uart);
        while (!uart_is_rx_buf_end(u->uart)){
            uart_read(u->uart, &u->uart_rxdata);
            while (!uart_is_rx_ready(u->uart));
            if (u->u_rx_func){
                u->u_rx_func(u->u_func_arg, u->uart_rxdata);
            }
        }
    }

}

/**
 * hal uart blocking tx
 *
 * This is type of write where UART has to block until character has been sent.
 * Used when printing diag output from system crash.
 * Must be called with interrupts disabled.
 */
void hal_uart_blocking_tx(int port, uint8_t data){
    hal_uart_t *u = &uarts[port];
    int sz, i = 0;
    uint32_t interrupts;
    if (port >= UART_COUNT || !u->u_open){
        return;
    }
    /* While there is data to write */
    u->tx_on = 1;
    while ((sz = fill_tx_buffer(u))){
        if (is_usart(u)){
            usart_enable_tx(u->uart);
            for (i = 0; i < sz; i++){
                while (!usart_is_tx_ready(u->uart));
                usart_write(u->uart, (uint32_t) u->txdata[i]);  
            }
            usart_disable_tx(u->uart);
        }else{
            interrupts = uart_get_interrupt_mask(u->uart);
            uart_disable_interrupt(u->uart, UINT32_MAX);
            uart_enable_tx(u->uart);
            for (i = 0; i < sz; i++){
                while (!uart_is_tx_ready(u->uart));
                uart_write(u->uart, (uint8_t) u->txdata[i]);
            }
            uart_disable_tx(u->uart);
            uart_enable_interrupt(u->uart, interrupts);
        }
    }

    /* Call back the user tx done function */
    if (u->u_tx_done){
        u->u_tx_done(u->u_func_arg);
    }
    u->tx_on = 0;
}
