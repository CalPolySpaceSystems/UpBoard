#include <mcu/hal_uart.h>
#include <uart.h>
#include <usart.h>
#include <hal/hal_uart.h>
#include <stdlib.h>


void hal_uart_blocking_tx(int port, uint8_t data){

}

int hal_uart_config(int port, int32_t baudrate, uint8_t databits, uint8_t stopbits,
                        enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ct){
                return 0;
}

int hal_uart_close(int port){
    return 0;
}

int hal_uart_init(int port, void *arg){
    return 0;
}