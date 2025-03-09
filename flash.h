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

#include <stdbool.h>

#include "hardware/spi.h"

// Status Register 1 Bit masks
#define WB_STATUS_REGISTER_1_BUSY_MASK 0x01
#define WB_STATUS_REGISTER_1_WEL_MASK 0x02
#define WB_STATUS_REGISTER_1_BP0_MASK 0x04
#define WB_STATUS_REGISTER_1_BP1_MASK 0x08
#define WB_STATUS_REGISTER_1_BP2_MASK 0x10
#define WB_STATUS_REGISTER_1_TB_MASK 0x20
#define WB_STATUS_REGISTER_1_SEC_MASK 0x40
#define WB_STATUS_REGISTER_1_SRP0_MASK 0x80
// Status Register 2 Bit masks
#define WB_STATUS_REGISTER_2_SRL_MASK 0x01
#define WB_STATUS_REGISTER_2_QE_MASK 0x02
#define WB_STATUS_REGISTER_2_LB1_MASK 0x08
#define WB_STATUS_REGISTER_2_LB2_MASK 0x10
#define WB_STATUS_REGISTER_2_LB3_MASK 0x20
#define WB_STATUS_REGISTER_2_CMP_MASK 0x40
#define WB_STATUS_REGISTER_2_SUS_MASK 0x80
// Status Register 3 Bit masks
#define WB_STATUS_REGISTER_3_WPS_MASK 0x04
#define WB_STATUS_REGISTER_3_DRV0_MASK 0x20
#define WB_STATUS_REGISTER_3_DRV1_MASK 0x40

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

/*
 * Initialise SPI to communicate with the flash IC.
 *
 * Other than setting some SIO pins to the correct state this function does not
 * perform any communication with the flash IC.
*/
uint flash_spi_init(flash_context_t *flash_context);

/*
 * Perform a software reset on the flash IC.
 *
 * This does not perform any reset operation on the data stored on the flash IC.
*/
void flash_reset(flash_context_t *flash_context);

/*
 * Perform a post reset test to ensure the flash IC is ready for use.
 *
 * This will check the following are as expected:
 * - The manufacturer ID is as expected.
 * - The device ID is as expected.
 * - The memory type is as expected.
 * - The capacity is as expected.
 * - The busy bit is clear.
 *
 * If any of these checks fail the function will return false.
*/
bool flash_post_reset_test(flash_context_t *flash_context);

/**
 * Load the device information from the flash device.
 */
void flash_load_device_info(flash_context_t *flash_context, flash_device_info_t *device_info);

/*
 * Reads status register 1 and tests if the busy bit is set.
 *
 * Can be used to detect if a clear or write operation is still in progress.
*/
bool flash_is_busy(flash_context_t *flash_context);

#endif // PICO_FLASH_H