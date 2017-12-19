#include "mcu/sam3x8.h"
#include "hal/hal_flash.h"

const struct hal_flash* hal_bsp_flash_dev(uint8_t id)
{
    /*
     * Internal flash mapped to id 0.
     */
    if (id != 0) {
        return NULL;
    }

    return &sam3x8_flash_dev_1;
}
