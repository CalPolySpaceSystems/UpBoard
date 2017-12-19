#include <string.h>
#include <assert.h>
#include <hal/hal_flash_int.h>
#include <flash_efc.h>

static int sam3x8_flash_read(const struct hal_flash *dev, uint32_t address,
        void *dst, uint32_t num_bytes);
static int sam3x8_flash_write(const struct hal_flash *dev, uint32_t address,
        const void *src, uint32_t num_bytes);
static int sam3x8_flash_erase_sector(const struct hal_flash *dev,
        uint32_t sector_address);
static int sam3x8_flash_sector_info(const struct hal_flash *dev, int idx,
        uint32_t *addr, uint32_t *sz);
static int sam3x8_flash_init(const struct hal_flash *dev);

static const struct hal_flash_funcs sam3x8_flash_funcs = {
    .hff_read = sam3x8_flash_read,
    .hff_write = sam3x8_flash_write,
    .hff_erase_sector = sam3x8_flash_erase_sector,
    .hff_sector_info = sam3x8_flash_sector_info,
    .hff_init = sam3x8_flash_init,
};

/* TODO: Add second memory bank to flash */
struct hal_flash sam3x8_flash_dev_1 = {
    .hf_itf = &sam3x8_flash_funcs,
    .hf_size = 1024 * 256
};

static int sam3x8_flash_read(const struct hal_flash *dev, uint32_t address,
    void *dst, uint32_t num_bytes)
{
    memcpy(dst, (void *) address, num_bytes);
    return 0;
}

static int sam3x8_flash_write(const struct hal_flash *dev, uint32_t address,
    const void *src, uint32_t len)
{
    return flash_write(address, src, len, 1);
}

static int sam3x8_flash_erase_sector(const struct hal_flash *dev, uint32_t sector_address)
{
    return flash_erase_all(sector_address);
}

static int sam3x8_flash_sector_info(const struct hal_flash *dev, int idx,
        uint32_t *addr, uint32_t *sz) {
    return dev->hf_size;
}

static int sam3x8_flash_init(const struct hal_flash *dev)
{
    /*
     * Initialize flash. We're using 128 bit mode for read speed,
     * and 6 wait states because Atmel goofed and requires it.
     */
    if(flash_init(FLASH_ACCESS_MODE_128, 6)) {
        return -1;
    }

    sam3x8_flash_dev_1.hf_itf = &sam3x8_flash_funcs;
    sam3x8_flash_dev_1.hf_sector_cnt = 1;
    sam3x8_flash_dev_1.hf_align = 1;

    return 0;
}
