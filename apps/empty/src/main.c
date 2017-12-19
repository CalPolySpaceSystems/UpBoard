#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

/**
 * main
 *
 * This application does nothing, it's primary goal is to test compiling with the sam3x8 mcu
 *
 * @return int NOTE: this function should never return!
 */
int main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();
    /* Debug breakpoint for testing */
    __asm__("bkpt");
    assert(0);

    return rc;
}

