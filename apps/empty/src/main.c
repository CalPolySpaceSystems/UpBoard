#include <assert.h>
#include <string.h>
#include <hal/hal_bsp.h>
#include <hal/hal_gpio.h>
#include <sysinit/sysinit.h>
#include <os/os.h>
#include <hal/hal_uart.h>

#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#define LED_BLINK_PIN (59)

/* Define task stack and task object */
#define MY_TASK_PRI         (OS_TASK_PRI_HIGHEST) 
#define MY_STACK_SIZE       (64) 
struct os_task my_task; 
os_stack_t my_task_stack[MY_STACK_SIZE]; 

/* This is the task function */
void my_task_func(void *arg) {
    /* Set the led pin as an output */
    hal_gpio_init_out(LED_BLINK_PIN, 1);

    /* The task is a forever loop that does not return */
    while (1) {
        /* Wait one second */ 
        os_time_delay(1000);

        /* Toggle the LED */ 
        hal_gpio_write(LED_BLINK_PIN, 0);

        /* Wait one second */ 
        os_time_delay(1000);

        /* Toggle the LED */ 
        hal_gpio_write(LED_BLINK_PIN, 1);
    }
}

/* This is the main function for the project */
int main(int argc, char **argv) 
{

    /* Perform system and package initialization */
    hal_bsp_init();
    sysinit();

    __asm__("bkpt");

    /* Initialize the task */
    int rc = os_task_init(&my_task, "my_task", my_task_func, NULL, MY_TASK_PRI, 
                 OS_WAIT_FOREVER, my_task_stack, MY_STACK_SIZE);

    assert(rc == OS_OK);
    __asm__("bkpt");

    /*  Process events from the default event queue.  */
    while (1) {
       os_eventq_run(os_eventq_dflt_get());
    }
    /* main never returns */  
}