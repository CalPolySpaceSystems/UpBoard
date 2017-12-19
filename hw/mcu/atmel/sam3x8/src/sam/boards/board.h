/*
 * The ASF expects a board.h file to define some characteristics.
 * For mynewt purposes, we're just going to create a generic board.h file with the defaults.
 * TODO: Look into using the syscfg.yml file to make this configurable.
 */

#define BOARD_FREQ_SLCK_XTAL      (32768U)
#define BOARD_FREQ_SLCK_BYPASS    (32768U)
#define BOARD_FREQ_MAINCK_XTAL    (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS  (12000000U)
#define BOARD_OSC_STARTUP_US      15625
