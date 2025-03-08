/* Copyright 2025, Darran A Lofthouse
 *
 * This file is part of pico-flash.
 *
 * pico-flash is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * pico-flash is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with pico-flash.
 * If  not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include "flash.h"
#include "hardware/gpio.h"

uint flash_spi_init(flash_context_t *flash_context)
{
    uint speed = spi_init(flash_context->spi, 100 * 1000 * 1000);
    // Needed to set SPI_CPHA_1 as we were losing the first bit of data.
    spi_set_format(flash_context->spi, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    printf("SPI init at %d Hz\n", speed);

    gpio_set_function(flash_context->tx_pin, GPIO_FUNC_SPI);
    gpio_set_function(flash_context->clk_pin, GPIO_FUNC_SPI);
    gpio_set_function(flash_context->rx_pin, GPIO_FUNC_SPI);
    printf("SPI pins configured\n");

    uint32_t sio_pin_mask = (0b1 << flash_context->hold_pin) |
                                        (0b1 << flash_context->wp_pin) |
                                        (0b1 << flash_context->cs_pin);

    // Set the pins that will be manually controlled as SIO output pins
    gpio_init_mask(sio_pin_mask);
    gpio_set_dir_out_masked(sio_pin_mask);

    gpio_put(flash_context->cs_pin, 1); // Chip select active low
    gpio_put(flash_context->hold_pin, 1); // Hold active low
    gpio_put(flash_context->wp_pin, 1); // Write protect active low
    printf("SIO Pins Configured\n");

    return speed;
}