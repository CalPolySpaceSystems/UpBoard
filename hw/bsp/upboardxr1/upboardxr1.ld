MEMORY
{
  FLASH    (rx)  : ORIGIN = 0x0000c000, LENGTH = 0x18000
  RAM      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000
}

/* This linker script is used for images and thus contains an image header */
_imghdr_size = 0x20;
