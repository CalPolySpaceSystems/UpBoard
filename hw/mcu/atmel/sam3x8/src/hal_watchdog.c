#include "hal/hal_watchdog.h"
#include "wdt.h"
#include "board.h"

#define WATCHDOG_MODE (WDT_MR_WDDBGHLT | WDT_MR_WDIDLEHLT)

static uint32_t timeout = 0;

int hal_watchdog_init(uint32_t expire_msecs)
{
    timeout = wdt_get_timeout_value(expire_msecs * 1000, BOARD_FREQ_SLCK_XTAL);
    return timeout == WDT_INVALID_ARGUMENT;
}

void hal_watchdog_enable(void)
{
    wdt_init(WDT, WATCHDOG_MODE, timeout, timeout);
}

void hal_watchdog_tickle(void)
{
    wdt_restart(WDT);
}
