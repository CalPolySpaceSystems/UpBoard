/**
 * Modified by Cal Poly Space Systems, 2018
 *
 * California Polytechnic State University
 * San Luis Obispo, California
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hal/hal_gpio.h"

#include <bsp/cmsis_nvic.h>

#include <assert.h>
#include <compiler.h>
#include "sam3x8.h"
#include "pio.h"
#include "pio_handler.h"
#include "parts.h"
#include <stdint.h>


// Input and output modes that you can't put as input arguments to PIO init in/out
#define PIO_OUTPUT_MODE PIO_OPENDRAIN
#define PIO_INPUT_MODE	0

 /*
 * GPIO pin mapping
 *
 * The sam3x8e has 144 pins and 104 GPIO. (A.K.A. PIO)
 *
 * They are split among 4 ports A (PA0-PA29),  B (PB0-PB31),
 * C (PC0-PC30), and D (PD0-PD9)
 *
 * We map from 0 to 143, where Port A is pins 0 - 31, Port B
 * is 32 - 63, port C is 64 - 95, and port D is 96 - 143;
 */

#define PIO_PORT(pin)           (pin/32)
#define PIO_PIN(pin)            (pin % 144)

#define GPIO_MAX_PORT   (3)

 /*
 * This defines which pins are valid for the ports
 * Whatever input you enter must end up being less than these values
 */
static const int valid_pins[GPIO_MAX_PORT + 1] =
{
    0x4fffffff, // Port A
    0xffffffff, // Port B
    0x8fffffff, // Port C
    0x000004ff, // Port D
};

int hal_gpio_init_out(int pin, int val)
{

    int port = PIO_PORT(pin);
    int port_pin = PIO_PIN(pin);

    if(port > GPIO_MAX_PORT) {
        return -1;
    }

    if((port_pin & valid_pins[port]) == 0) {
        return -1;
    }

    // Define input pin as bitmask of a pin
    uint32_t pio_mask = pio_get_pin_group_mask(pin);

    // Use arg "val" to decide if output is intitialized low or high
    int pio_init_lvl;

    if (val) {
        pio_init_lvl = PIO_OUTPUT_1;
    }
    else {
        pio_init_lvl = PIO_OUTPUT_0;
    }

    // Get the pointer to pio instance depending on the port
    switch (port){
    case 0:
        pio_configure(PIOA, pio_init_lvl, pio_mask, PIO_DEFAULT);
    case 1:
        pio_configure(PIOB, pio_init_lvl, pio_mask, PIO_DEFAULT);
    case 2:
        pio_configure(PIOC, pio_init_lvl, pio_mask, PIO_DEFAULT);
    case 3:
        pio_configure(PIOD, pio_init_lvl, pio_mask, PIO_DEFAULT);
    default:
        return -1;
    }

    return 0;
}

int hal_gpio_init_in(int pin, hal_gpio_pull_t pull)
{
    // Make sure pin is valid (uwu)
    int port = PIO_PORT(pin);
    int port_pin = PIO_PIN(pin);

    if (port > GPIO_MAX_PORT) {
        return -1;
    }

    if ((port_pin & valid_pins[port]) == 0) {
        return -1;
    }

    // Pull type of input pin
    int pio_pull;

    switch (pull) {
    case HAL_GPIO_PULL_NONE:
        pio_pull = PIO_DEFAULT;
        break;
    case HAL_GPIO_PULL_UP:
        pio_pull = PIO_PULLUP;
        break;
    case HAL_GPIO_PULL_DOWN:
        pio_pull = PIO_OPENDRAIN;
        break;
    default:
        return -1;
    }

    // Define input pin as bitmask of a pin
    uint32_t pio_mask = pio_get_pin_group_mask(pin);

    // Get the pointer to pio instance depending on the port
    switch (port) {
    case 0:
        pio_configure(PIOA, PIO_INPUT, pio_mask, pio_pull);
    case 1:
        pio_configure(PIOB, PIO_INPUT, pio_mask, pio_pull);
    case 2:
        pio_configure(PIOC, PIO_INPUT, pio_mask, pio_pull);
    case 3:
        pio_configure(PIOD, PIO_INPUT, pio_mask, pio_pull);
    default:
        return -1;
    }

    return 0;
}

int sam_get_gpio_dir(Pio *p_pio, uint32_t pio_mask) {
    if (p_pio->PIO_OSR & pio_mask){
        return PIO_OUTPUT_0;
    }
    return PIO_INPUT;
}

/**
 * gpio read
 *
 * Reads the specified pin.
 *
 * @param pin Pin number to read
 *
 * @return int 0: low, 1: high
 */
int hal_gpio_read(int pin){
    // Make sure pin is valid
    int port = PIO_PORT(pin);
    int port_pin = PIO_PIN(pin);

    assert(port <= GPIO_MAX_PORT);
    assert(((1 << port_pin) & valid_pins[port]) != 0);

    // Define input pin as bitmask of a pin
    uint32_t pio_mask = pio_get_pin_group_mask(pin);

    // Read from the pin with the correct settings
    switch (port) {
    case 0:
        return pio_get(PIOA, sam_get_gpio_dir(PIOA, pio_mask), pio_mask);
    case 1:
        return pio_get(PIOB, sam_get_gpio_dir(PIOB, pio_mask), pio_mask);
    case 2:
        return pio_get(PIOC, sam_get_gpio_dir(PIOC, pio_mask), pio_mask);
    case 3:
        return pio_get(PIOD, sam_get_gpio_dir(PIOD, pio_mask), pio_mask);
    default:
        return -1;
    }
}

/**
 * gpio write
 *
 * Write a value (either high or low) to the specified pin.
 *
 * @param pin Pin to set
 * @param val Value to set pin (0:low 1:high)
 */
void hal_gpio_write(int pin, int val)
{
    // Get port
    int port = PIO_PORT(pin);
    int port_pin = PIO_PIN(pin);

    // Get pinmask
    uint32_t pio_mask = pio_get_pin_group_mask(port_pin);

    if (val) {
        switch (port) {
        case 0:
            pio_set(PIOA, pio_mask);
            return;
        case 1:
            pio_set(PIOB, pio_mask);
            return;
        case 2:
            pio_set(PIOC, pio_mask);
            return;
        case 3:
            pio_set(PIOD, pio_mask);
            return;
        }
    }
    else {
        switch (port) {
        case 0:
            pio_clear(PIOA, pio_mask);
            return;
        case 1:
            pio_clear(PIOB, pio_mask);
            return;
        case 2:
            pio_clear(PIOC, pio_mask);
            return;
        case 3:
            pio_clear(PIOD, pio_mask);
            return;
        }
    }
}

/**
 * gpio toggle
 *
 * Toggles the specified pin
 *
 * @param pin Pin number to toggle
 */
int hal_gpio_toggle(int pin)
{
    int pin_state;

    pin_state = (hal_gpio_read(pin) == 0);
    hal_gpio_write(pin, pin_state);
    return hal_gpio_read(pin);
}
