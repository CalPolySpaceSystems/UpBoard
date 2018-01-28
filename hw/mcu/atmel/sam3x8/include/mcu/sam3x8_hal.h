#ifndef _H_SAM3Xi_HAL_
#define _H_SAM3X8_HAL_

#ifdef __cplusplus
 extern "C" {
#endif

struct sam3x8_timer_cfg
{
    IRQn_Type irq;
    int channel; /* 3 channels per HW Timer */
    Tc *hwtimer;
};

#ifdef __cplusplus
}
#endif

#endif
