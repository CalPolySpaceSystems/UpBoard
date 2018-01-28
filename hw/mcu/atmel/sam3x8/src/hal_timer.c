#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

#include "hal/hal_timer.h"
#include "pmc.h"
#include "tc.h"
#include "mcu/sam3x8_hal.h"
#include "bsp/cmsis_nvic.h"

/* IRQ prototype */
typedef void (*hal_timer_irq_handler_t)(void);

/* Number of timers for HAL */
/* TODO: Add support for other timers */
#define SAM3X8_HAL_TIMER_MAX (1)

/* Internal timer data structure */
struct sam3x8_hal_timer {
    uint8_t tmr_channel;
    uint8_t tmr_enabled;
    IRQn_Type tmr_irq;
    uint8_t tmr_srcclk;
    uint8_t tmr_initialized;
    uint32_t tmr_cntr;
    uint32_t timer_isrs;
    uint32_t tmr_freq;
    TAILQ_HEAD(hal_timer_qhead, hal_timer) hal_timer_q;
    Tc *tc_mod;
};

struct sam3x8_hal_timer sam3x8_hal_timer0;

static const struct sam3x8_hal_timer *sam3x8_hal_timers[SAM3X8_HAL_TIMER_MAX] = {
    &sam3x8_hal_timer0,
};

/* Resolve timer number into timer structure */
#define SAM3X8_HAL_TIMER_RESOLVE(__n, __v)       \
    if ((__n) >= SAM3X8_HAL_TIMER_MAX) {         \
        rc = EINVAL;                            \
        goto err;                               \
    }                                           \
    (__v) = (struct sam3x8_hal_timer *) sam3x8_hal_timers[(__n)];            \
    if ((__v) == NULL) {                        \
        rc = EINVAL;                            \
        goto err;                               \
    }

/**
 * sam3x8 timer set ocmp
 *
 * Set the OCMP used by the timer to the desired expiration tick
 *
 * NOTE: Must be called with interrupts disabled.
 *
 * @param timer Pointer to timer.
 */
static void
sam3x8_timer_set_ocmp(struct sam3x8_hal_timer *bsptimer, uint32_t expiry)
{
    Tc *hwtimer;
    int32_t delta_t;

    hwtimer = bsptimer->tc_mod;

    /* Disable ocmp interrupt and set new value */
    hwtimer->TC_CHANNEL[bsptimer->tmr_channel].TC_IDR=0xF;
    delta_t = (int32_t)(expiry - bsptimer->tmr_cntr);
    if (delta_t < 0) {
        goto set_ocmp_late;
    } else if (delta_t == 0) {
        /* Set ocmp and check if missed it */

        /* Set output compare register to timer expiration */
        tc_write_rc(hwtimer, bsptimer->tmr_channel, expiry);

        /* Enable RC compare interrupt */
        hwtimer->TC_CHANNEL[bsptimer->tmr_channel].TC_IER=TC_IER_CPCS;
        hwtimer->TC_CHANNEL[bsptimer->tmr_channel].TC_IDR=~TC_IER_CPCS;

        /* Force interrupt to occur as we may have missed it */
        if (tc_read_cv(hwtimer, bsptimer->tmr_channel) >= expiry) {
            goto set_ocmp_late;
        }
    } else {
        /* Nothing to do; wait for overflow interrupt to set ocmp */
    }
    return;

set_ocmp_late:
    NVIC_SetPendingIRQ(bsptimer->tmr_irq);
}

/* Disable output compare used for timer */
static void
sam3x8_timer_disable_ocmp(struct sam3x8_hal_timer *bsptimer)
{
    bsptimer->tc_mod->TC_CHANNEL[bsptimer->tmr_channel].TC_IDR=0xF;
}

static uint32_t
hal_timer_read_bsptimer(struct sam3x8_hal_timer *bsptimer)
{
    Tc *hwtimer;
    hwtimer = bsptimer->tc_mod;
    cpu_irq_enter_critical();
    bsptimer->tmr_cntr = tc_read_cv(hwtimer, bsptimer->tmr_channel);
    cpu_irq_leave_critical();

    return bsptimer->tmr_cntr;
}

/**
 * hal timer chk queue
 *
 * @param bsptimer
 */
static void
hal_timer_chk_queue(struct sam3x8_hal_timer *bsptimer)
{
    uint32_t tcntr;
    struct hal_timer *timer;

    /* disable interrupts */
    cpu_irq_enter_critical();

    while ((timer = TAILQ_FIRST(&bsptimer->hal_timer_q)) != NULL) {
        tcntr = hal_timer_read_bsptimer(bsptimer);
        if ((int32_t)(tcntr - timer->expiry) >= 0) {
            TAILQ_REMOVE(&bsptimer->hal_timer_q, timer, link);
            timer->link.tqe_prev = NULL;
            timer->cb_func(timer->cb_arg);
        } else {
            break;
        }
    }

    /* Any timers left on queue? If so, we need to set OCMP */
    timer = TAILQ_FIRST(&bsptimer->hal_timer_q);
    if (timer) {
        sam3x8_timer_set_ocmp(bsptimer, timer->expiry);
    } else {
        sam3x8_timer_disable_ocmp(bsptimer);
    }

    cpu_irq_leave_critical();
}

/**
 * hal timer irq handler
 *
 * This is the global timer interrupt routine.
 *
 */
static void
hal_timer_irq_handler(struct sam3x8_hal_timer *bsptimer)
{

    /* Clear interupt from register */
    bsptimer->tc_mod->TC_CHANNEL[bsptimer->tmr_channel].TC_IDR=TC_IER_CPCS;

    /* Count # of timer isrs */
    ++bsptimer->timer_isrs;
    hal_timer_chk_queue(bsptimer);
}

void
sam3x8_timer0_irq_handler(void)
{
    hal_timer_irq_handler(&sam3x8_hal_timer0);
}

/**
 * hal timer init
 *
 * Initialize platform specific timer items
 *
 * @param timer_num     Timer number to initialize
 * @param cfg           Pointer to platform specific configuration
 *
 * @return int          0: success; error code otherwise
 */
int
hal_timer_init(int timer_num, void *cfg)
{
    int rc;
    IRQn_Type irq;
    struct sam3x8_hal_timer *bsptimer;
    struct sam3x8_timer_cfg *tmr_cfg;
    hal_timer_irq_handler_t irq_isr;

    /* Get timer. Make sure not enabled */
    SAM3X8_HAL_TIMER_RESOLVE(timer_num, bsptimer);
    if (bsptimer->tmr_enabled) {
        rc = EINVAL;
        goto err;
    }
    tmr_cfg = (struct sam3x8_timer_cfg *)cfg;

    rc = 0;
    switch (timer_num) {
    case 0:
        irq_isr = sam3x8_timer0_irq_handler;
        break;
    default:
        rc = -1;
        break;
    }

    if (rc) {
        goto err;
    }

    irq = tmr_cfg->irq;
    bsptimer->tmr_irq = irq;
    bsptimer->tc_mod = tmr_cfg->hwtimer;
    bsptimer->tmr_initialized = 1;
    bsptimer->tmr_channel = tmr_cfg->channel;

    /* Disable existing interupt config and configure new one */
    NVIC_DisableIRQ(irq);
    NVIC_SetPriority(irq, (1 << __NVIC_PRIO_BITS) - 1);
    NVIC_SetVector(irq, (uint32_t)irq_isr);
    tc_disable_interrupt(bsptimer->tc_mod, bsptimer->tmr_channel, 0xFFFF);

    return 0;

err:
    return rc;
}



/* Joshua - This code is from https://github.com/ivanseidel/DueTimer/ */
uint8_t best_clock(double frequency, uint32_t *retRC){
    /*
        Pick the best Clock, thanks to Ogle Basil Hall!
        Timer           Definition
        TIMER_CLOCK1    MCK /  2
        TIMER_CLOCK2    MCK /  8
        TIMER_CLOCK3    MCK / 32
        TIMER_CLOCK4    MCK /128
    */
    const struct {
        uint8_t flag;
        uint8_t divisor;
    } clockConfig[] = {
        { TC_CMR_TCCLKS_TIMER_CLOCK1,   2 },
        { TC_CMR_TCCLKS_TIMER_CLOCK2,   8 },
        { TC_CMR_TCCLKS_TIMER_CLOCK3,  32 },
        { TC_CMR_TCCLKS_TIMER_CLOCK4, 128 }
    };
    float ticks;
    float error;
    int clkId = 3;
    int bestClock = 3;
    float bestError = 9.999e99;
    do
    {
        ticks = (float) SystemCoreClock / frequency / (float) clockConfig[clkId].divisor;
        // error = abs(ticks - round(ticks));
        error = clockConfig[clkId].divisor * abs(ticks - round(ticks));	// Error comparison needs scaling
        if (error < bestError)
        {
            bestClock = clkId;
            bestError = error;
        }
    } while (clkId-- > 0);
    ticks = (float) SystemCoreClock / frequency / (float) clockConfig[bestClock].divisor;
    *retRC = (uint32_t) round(ticks);
    return clockConfig[bestClock].flag;
}


/**
 * hal timer config
 *
 * Configure a timer to run at the desired frequency. This starts the timer.
 *
 * @param timer_num
 * @param freq_hz
 *
 * @return int
 */
int
hal_timer_config(int timer_num, uint32_t freq_hz)
{
    int rc;
    uint32_t clk_freq;
    uint8_t clock;
    struct sam3x8_hal_timer *bsptimer;

    /* Get timer. Make sure not enabled */
    SAM3X8_HAL_TIMER_RESOLVE(timer_num, bsptimer);
    if (bsptimer->tmr_enabled || (bsptimer->tmr_initialized == 0) ||
        (freq_hz == 0)) {
        rc = EINVAL;
        goto err;
    }

    pmc_set_writeprotect(false);
    pmc_enable_periph_clk(bsptimer->tmr_irq);

    /* Select the clock source closest to the desired frequency */
    for(int i = 0; i < 4; i++) {
        switch(i) {
            case 0:
                clk_freq = SystemCoreClock / 128.0;
                if(clk_freq > freq_hz) {
                    bsptimer->tmr_freq = clk_freq;
                    clock = TC_CMR_TCCLKS_TIMER_CLOCK4;
                    break;
                }
            case 1:
                clk_freq = SystemCoreClock / 32.0;
                if(clk_freq > freq_hz) {
                    bsptimer->tmr_freq = clk_freq;
                    clock = TC_CMR_TCCLKS_TIMER_CLOCK3;
                    break;
                }
            case 2:
                clk_freq = SystemCoreClock / 8.0;
                if(clk_freq > freq_hz) {
                    bsptimer->tmr_freq = clk_freq;
                    clock = TC_CMR_TCCLKS_TIMER_CLOCK2;
                    break;
                }
            case 3:
                clk_freq = SystemCoreClock / 2.0;
                if(clk_freq > freq_hz) {
                    bsptimer->tmr_freq = clk_freq;
                    clock = TC_CMR_TCCLKS_TIMER_CLOCK1;
                    break;
                }
                /* No matching clock, error out */
                rc = -1;
                goto err;
        }
    }


    tc_init(bsptimer->tc_mod, bsptimer->tmr_channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | clock);
    /* Enable the RC Compare Interrupt and disable all others */
    bsptimer->tc_mod->TC_CHANNEL[bsptimer->tmr_channel].TC_IER=TC_IER_CPCS;
    bsptimer->tc_mod->TC_CHANNEL[bsptimer->tmr_channel].TC_IDR=~TC_IER_CPCS;

    bsptimer->tmr_enabled = 1;

    tc_start(bsptimer->tc_mod, bsptimer->tmr_channel);


    /* Set isr in vector table and enable interrupt */
    NVIC_EnableIRQ(bsptimer->tmr_irq);


    return 0;

err:
    return rc;
}

/**
 * hal timer deinit
 *
 * De-initialize a HW timer.
 *
 * @param timer_num
 *
 * @return int
 */
int
hal_timer_deinit(int timer_num)
{
    int rc;
    struct sam3x8_hal_timer *bsptimer;

    SAM3X8_HAL_TIMER_RESOLVE(timer_num, bsptimer);

    tc_stop(bsptimer->tc_mod, bsptimer->tmr_channel);
    bsptimer->tmr_enabled = 0;
    bsptimer->tmr_initialized = 0;

err:
    return rc;
}

/**
 * hal timer get resolution
 *
 * Get the resolution of the timer. This is the timer period, in nanoseconds
 *
 * @param timer_num
 *
 * @return uint32_t The resolution of the timer, in nanoseconds.
 */
uint32_t
hal_timer_get_resolution(int timer_num)
{
    int rc;
    uint32_t resolution;
    struct sam3x8_hal_timer *bsptimer;

    SAM3X8_HAL_TIMER_RESOLVE(timer_num, bsptimer);

    resolution = 1000000000 / bsptimer->tmr_freq;
    return resolution;

err:
    rc = 0;
    return rc;
}


/**
 * hal timer read
 *
 * Returns the timer counter. NOTE: if the timer is a 16-bit timer, only
 * the lower 16 bits are valid. If the timer is a 64-bit timer, only the
 * low 32-bits are returned.
 *
 * @return uint32_t The timer counter register.
 */
uint32_t
hal_timer_read(int timer_num)
{
    int rc;
    uint32_t tcntr;
    struct sam3x8_hal_timer *bsptimer;

    SAM3X8_HAL_TIMER_RESOLVE(timer_num, bsptimer);
    tcntr = hal_timer_read_bsptimer(bsptimer);
    return tcntr;

    /* Assert here since there is no invalid return code */
err:
    assert(0);
    rc = 0;
    return rc;
}

/**
 * hal timer delay
 *
 * Blocking delay for n ticks
 *
 * @param timer_num
 * @param ticks
 *
 * @return int 0 on success; error code otherwise.
 */
int
hal_timer_delay(int timer_num, uint32_t ticks)
{
    uint32_t until;

    until = hal_timer_read(timer_num) + ticks;
    while ((int32_t)(hal_timer_read(timer_num) - until) <= 0) {
        /* Loop here till finished */
    }

    return 0;
}

/**
 *
 * Initialize the HAL timer structure with the callback and the callback
 * argument. Also initializes the HW specific timer pointer.
 *
 * @param cb_func
 *
 * @return int
 */
int
hal_timer_set_cb(int timer_num, struct hal_timer *timer, hal_timer_cb cb_func,
                 void *arg)
{
    int rc;
    struct sam3x8_hal_timer *bsptimer;

    SAM3X8_HAL_TIMER_RESOLVE(timer_num, bsptimer);

    timer->cb_func = cb_func;
    timer->cb_arg = arg;
    timer->link.tqe_prev = NULL;
    timer->bsp_timer = bsptimer;

    rc = 0;

err:
    return rc;
}

int
hal_timer_start(struct hal_timer *timer, uint32_t ticks)
{
    int rc;
    uint32_t tick;
    struct sam3x8_hal_timer *bsptimer;

    /* Set the tick value at which the timer should expire */
    bsptimer = (struct sam3x8_hal_timer *)timer->bsp_timer;
    tick = hal_timer_read_bsptimer(bsptimer) + ticks;
    rc = hal_timer_start_at(timer, tick);
    return rc;
}

int
hal_timer_start_at(struct hal_timer *timer, uint32_t tick)
{
    struct hal_timer *entry;
    struct sam3x8_hal_timer *bsptimer;

    if ((timer == NULL) || (timer->link.tqe_prev != NULL) ||
        (timer->cb_func == NULL)) {
        return EINVAL;
    }
    bsptimer = (struct sam3x8_hal_timer *)timer->bsp_timer;
    timer->expiry = tick;

    cpu_irq_enter_critical();

    if (TAILQ_EMPTY(&bsptimer->hal_timer_q)) {
        TAILQ_INSERT_HEAD(&bsptimer->hal_timer_q, timer, link);
    } else {
        TAILQ_FOREACH(entry, &bsptimer->hal_timer_q, link) {
            if ((int32_t)(timer->expiry - entry->expiry) < 0) {
                TAILQ_INSERT_BEFORE(entry, timer, link);
                break;
            }
        }
        if (!entry) {
            TAILQ_INSERT_TAIL(&bsptimer->hal_timer_q, timer, link);
        }
    }

    /* If this is the head, we need to set new OCMP */
    if (timer == TAILQ_FIRST(&bsptimer->hal_timer_q)) {
        sam3x8_timer_set_ocmp(bsptimer, timer->expiry);
    }

    cpu_irq_leave_critical();

    return 0;
}

/**
 * hal timer stop
 *
 * Stop a timer.
 *
 * @param timer
 *
 * @return int
 */
int
hal_timer_stop(struct hal_timer *timer)
{
    int reset_ocmp;
    struct hal_timer *entry;
    struct sam3x8_hal_timer *bsptimer;

    if (timer == NULL) {
        return EINVAL;
    }

   bsptimer = (struct sam3x8_hal_timer *)timer->bsp_timer;

    cpu_irq_enter_critical();

    if (timer->link.tqe_prev != NULL) {
        reset_ocmp = 0;
        if (timer == TAILQ_FIRST(&bsptimer->hal_timer_q)) {
            /* If first on queue, we will need to reset OCMP */
            entry = TAILQ_NEXT(timer, link);
            reset_ocmp = 1;
        }
        TAILQ_REMOVE(&bsptimer->hal_timer_q, timer, link);
        timer->link.tqe_prev = NULL;
        if (reset_ocmp) {
            if (entry) {
                sam3x8_timer_set_ocmp((struct sam3x8_hal_timer *)entry->bsp_timer,
                                      entry->expiry);
            } else {
                sam3x8_timer_disable_ocmp(bsptimer);
            }
        }
    }

    cpu_irq_leave_critical();

    return 0;
}