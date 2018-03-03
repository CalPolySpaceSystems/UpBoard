#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

static volatile int g_task1_loops;

/* For LED toggling */
int g_led_pin;

/* The timer callout */
static struct os_callout blinky_callout;

/*
 * Event callback function for timer events. It toggles the led pin.
 */
static void
timer_ev_cb(struct os_event *ev)
{
    assert(ev != NULL);

    ++g_task1_loops;
    hal_gpio_toggle(g_led_pin);

    os_callout_reset(&blinky_callout, OS_TICKS_PER_SEC/10);
}

static void
init_timer(void)
{
    /*
     * Initialize the callout for a timer event.
     */
    os_callout_init(&blinky_callout, os_eventq_dflt_get(),
                    timer_ev_cb, NULL);

    os_callout_reset(&blinky_callout, OS_TICKS_PER_SEC/10);
}

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */
int main(int argc, char **argv)
{
    int rc;
    board_init();
#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    g_led_pin = 21;
    hal_gpio_init_out(g_led_pin, 1);

    init_timer();
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);

    return rc;
}

