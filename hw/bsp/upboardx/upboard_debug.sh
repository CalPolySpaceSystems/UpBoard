#!/bin/sh

. $CORE_PATH/hw/scripts/openocd.sh

FILE_NAME=$BIN_BASENAME.elf
CFG="-f $BSP_PATH/upboard.cfg"

openocd_debug
