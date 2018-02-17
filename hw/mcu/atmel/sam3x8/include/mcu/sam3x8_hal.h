#ifndef _H_SAM3Xi_HAL_
#define _H_SAM3X8_HAL_

#ifdef __cplusplus
 extern "C" {
#endif

struct sam3x8_timer_cfg
{
    IRQn_Type irq;
    IRQn_Type irq_ps;
    Tc *hwtimer;
};

#ifdef __cplusplus
}
#endif

#endif
