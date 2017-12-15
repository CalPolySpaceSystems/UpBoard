/*
 * CPSS UpBoard X Main code
 *
 *
 *
 */

// Include necessary mynewt os + hal libraries
//#include <os/os.h>
//#include <bsp/bsp.h>

//hardware abstraction layer (HAL)
// this contains things like uart/twi drivers

//#include <assert.h>
//#include <sysinit/sysinit.h>

// Include our sensor/functionality libraries (location subject to change)
/*
#include <module/a3g4250d.h>
#include <module/lsm9ds1.h>
#include <module/ms56xx.h>
#include <module/ublox_gps>
#include <module/xbee_spi>
*/

// define task priorities and stack sizes
/*
#define GEN_TASK_PRIO		3
#define GEN_TASK_STACK_SZ	512
*/
// Pin and other definitions

// variables

// main function to run once
int  main(int argc, char **argv) {

	// initialize system
	sysinit();

	// initialize scheduled tasks (see os.h??)
	//init_tasks();

	while (1) {

	}
	assert(0);

}

/* initialize tasks 
void init_tasks(void) {

}




/* event callbacks*/

// sensor reads

// packet transmissions

// other