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
#include "pico/stdlib.h"

/*
 * Constants
 */

// SPI Instruction Set
#define WB_WRITE_ENABLE 0x06
#define WB_VOLATILE_SR_WRITE_ENABLE 0x50
#define WB_WRITE_DISABLE 0x04
#define WB_RELEASE_POWER_DOWN_ID 0xAB

// Skipping as other calls load this.
#define WB_MANUFACTURER_DEVICE_ID 0x90

#define WB_JEDEC_ID 0x9F
#define WB_READ_UNIQUE_ID 0x4B
#define WB_READ_DATA 0x03
#define WB_FAST_READ 0x0B
#define WB_PAGE_PROGRAM 0x02
#define WB_SECTOR_ERASE 0x20
#define WB_BLOCK_ERASE_32K 0x52
#define WB_BLOCK_ERASE_64K 0xD8
#define WB_CHIP_ERASE 0x60
#define WB_READ_STATUS_REGISTER_1 0x05
#define WB_WRITE_STATUS_REGISTER_1 0x01
#define WB_READ_STATUS_REGISTER_2 0x35
#define WB_WRITE_STATUS_REGISTER_2 0x31
#define WB_READ_STATUS_REGISTER_3 0x15
#define WB_WRITE_STATUS_REGISTER_3 0x11

// Don't have complete documentation, so not implementing this yet.
#define WB_READ_SFDP_REGISTER 0x5A

#define WB_ERASE_SECURITY_REGISTER 0x44
#define WB_PROGRAM_SECURITY_REGISTER 0x42
#define WB_READ_SECURITY_REGISTER 0x48
#define WB_GLOBAL_BLOCK_LOCK 0x7E
#define WB_GLOBAL_BLOCK_UNLOCK 0x98
#define WB_READ_BLOCK_LOCK 0x3D
#define WB_INDIVIDUAL_BLOCK_LOCK 0x36
#define WB_INDIVIDUAL_BLOCK_UNLOCK 0x39
#define WB_ERASE_PROGRAM_SUSPEND 0x75
#define WB_ERASE_PROGRAM_RESUME 0x7A
#define WB_POWER_DOWN 0xB9
#define WB_ENABLE_RESET 0x66
#define WB_RESET 0x99


/*
 * Internal Functions
 */

static void _wb_enable_reset(flash_context_t *flash_context)
{
    gpio_put(flash_context->cs_pin, 0);
    uint8_t cmd = WB_ENABLE_RESET;
    spi_write_blocking(flash_context->spi, &cmd, 1);
    gpio_put(flash_context->cs_pin, 1);
}

static void _wb_reset(flash_context_t *flash_context)
{
    gpio_put(flash_context->cs_pin, 0);
    uint8_t cmd = WB_RESET;
    spi_write_blocking(flash_context->spi, &cmd, 1);
    gpio_put(flash_context->cs_pin, 1);
}

 static void _wb_enable_reset_reset(flash_context_t *flash_context)
 {
     // This is a pair of commands to reset the device whilst
     // adding some protection for an accidental reset hence the enable.

     printf("Starting software reset sequence \n");
     _wb_enable_reset(flash_context);
     printf("Enable reset sent\n");
     _wb_reset(flash_context);
     printf("Reset sent\n");
     sleep_us(30);
 }

 static uint8_t _wb_release_power_down_id(flash_context_t *flash_context)
{
    // DataSheet Page 44
    gpio_put(flash_context->cs_pin, 0);
    uint8_t cmdbuf[4] = {
            WB_RELEASE_POWER_DOWN_ID,
            0x00, 0x00, 0x00  // Three dummy bytes
    };
    spi_write_blocking(flash_context->spi, cmdbuf, 4);
    uint8_t data[1];
    spi_read_blocking(flash_context->spi, 0, data, 1);
    gpio_put(flash_context->cs_pin, 1);

    return data[0];
}

static void _wb_read_jdec_id(flash_context_t *flash_context, uint8_t *jdec_id)
{
    // DataSheet Page 49
    gpio_put(flash_context->cs_pin, 0);
    uint8_t cmd = WB_JEDEC_ID;
    spi_write_blocking(flash_context->spi, &cmd, 1);
    spi_read_blocking(flash_context->spi, 0, jdec_id, 3);
    gpio_put(flash_context->cs_pin, 1);
}

static void _wb_read_unique_id(flash_context_t *flash_context, uint8_t *unique_id)
{
    // DataSheet Page 48
    gpio_put(flash_context->cs_pin, 0);
    uint8_t cmdbuf[5] = {
            WB_READ_UNIQUE_ID,
            0x00, 0x00, 0x00, 0x00  // Four dummy bytes
    };
    spi_write_blocking(flash_context->spi, cmdbuf, 5);
    spi_read_blocking(flash_context->spi, 0, unique_id, 8);
    gpio_put(flash_context->cs_pin, 1);
}

static uint8_t _wb_read_status_register(flash_context_t *flash_context,  uint8_t cmd)
{
    // DataSheet Page 25
    gpio_put(flash_context->cs_pin, 0);
    spi_write_blocking(flash_context->spi, &cmd, 1);
    uint8_t data[1];
    spi_read_blocking(flash_context->spi, 0, data, 1);
    gpio_put(flash_context->cs_pin, 1);

    return data[0];
}

static uint8_t _wb_read_status_register_1(flash_context_t *flash_context)
{
    return _wb_read_status_register(flash_context,  WB_READ_STATUS_REGISTER_1);
}

static uint8_t _wb_read_status_register_2(flash_context_t *flash_context)
{
    return _wb_read_status_register(flash_context, WB_READ_STATUS_REGISTER_2);
}

static uint8_t _wb_read_status_register_3(flash_context_t *flash_context)
{
    return _wb_read_status_register(flash_context, WB_READ_STATUS_REGISTER_3);
}

/*
 * API Implementation
 */

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

void flash_reset(flash_context_t *flash_context)
{
    _wb_enable_reset_reset(flash_context);
    printf("Software Reset\n");
}

void flash_load_device_info(flash_context_t *flash_context, flash_device_info_t *device_info)
{
    device_info->manufacturer_id = _wb_release_power_down_id(flash_context);
    _wb_read_jdec_id(flash_context, device_info->jedec_id);
    _wb_read_unique_id(flash_context, device_info->unique_id);
    device_info->status_register_1 = _wb_read_status_register_1(flash_context);
    device_info->status_register_2 = _wb_read_status_register_2(flash_context);
    device_info->status_register_3 = _wb_read_status_register_3(flash_context);
}