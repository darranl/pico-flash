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

struct flash_device_info {
    uint8_t manufacturer_id;
    uint8_t device_id;
    uint8_t jedec_id[3];
    uint8_t unique_id[8];
    uint8_t status_register_1;
    uint8_t status_register_2;
    uint8_t status_register_3;
};

typedef struct flash_context flash_context_t;
typedef struct flash_device_info flash_device_info_t;

uint flash_spi_init(flash_context_t *flash_context);

void flash_reset(flash_context_t *flash_context);

/**
 * Load the device information from the flash device.
 */
void flash_load_device_info(flash_context_t *flash_context, flash_device_info_t *device_info);

#endif // PICO_FLASH_H