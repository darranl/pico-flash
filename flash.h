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

#ifndef PICO_FLASH_H
#define PICO_FLASH_H

#include "hardware/spi.h"

struct flash_context
{
    spi_inst_t *spi;
    uint tx_pin;
    uint clk_pin;
    uint rx_pin;
    uint hold_pin;
    uint wp_pin;
    uint cs_pin;
};

typedef struct flash_context flash_context_t;

uint flash_spi_init(flash_context_t *flash_context);

#endif // PICO_FLASH_H