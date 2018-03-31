/* Memory Spaces Definitions */
MEMORY
{
  FLASH    (rx)  : ORIGIN = 0x00000000, LENGTH = 0x0000c000
  RAM      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000
}

/* The bootloader does not contain an image header */
_imghdr_size = 0x0;
