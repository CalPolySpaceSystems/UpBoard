#include <stddef.h>
#include <inttypes.h>
#include <mcu/cortex_m3.h>

/**
 * Boots the image described by the supplied image header.
 *
 * @param hdr                   The header for the image to boot.
 */
void hal_system_start(void *img_start)
{
    typedef void jump_fn(void);

    uint32_t base0entry;
    uint32_t jump_addr;
    jump_fn *fn;

    /* First word contains initial MSP value. */
    __set_MSP(*(uint32_t *)img_start);
    __set_PSP(*(uint32_t *)img_start);

    /* Second word contains address of entry point (Reset_Handler). */
    base0entry = *(uint32_t *)(img_start + 4);
    jump_addr = base0entry;
    fn = (jump_fn *)jump_addr;

    /* Jump to image. */
    fn();
}
