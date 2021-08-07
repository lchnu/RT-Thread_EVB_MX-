/*
 * File      : fal_flash_sfud_port.c
 * This file is part of FAL (Flash Abstraction Layer) package
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <fal.h>
#include <sfud.h>
#include "drv_qspi.h"

#ifdef FAL_USING_SFUD_PORT
#ifdef RT_USING_SFUD
#include <spi_flash_sfud.h>
#endif

#ifndef FAL_USING_NOR_FLASH_DEV_NAME
#define FAL_USING_NOR_FLASH_DEV_NAME             "norflash0"
#endif

static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);

static sfud_flash_t sfud_dev = NULL;
struct fal_flash_dev nor_flash0 =
{
    .name       = FAL_USING_NOR_FLASH_DEV_NAME,
    .addr       = 0,
    .len        = 8 * 1024 * 1024,
    .blk_size   = 4096,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};

#define QSPI_BUS_NAME       "qspi1"
#define QSPI_DEVICE_NAME    "qspi1_0"
#define QSPI_CS_PIN         GET_PIN(B, 11)   //此处注意引脚区分, 使用实际电路NCS引脚


static int rt_hw_qspi_flash_with_sfud_init(void)
{
    if (stm32_qspi_bus_attach_device(QSPI_BUS_NAME, QSPI_DEVICE_NAME, (rt_uint32_t)QSPI_CS_PIN, 1, RT_NULL, RT_NULL) != RT_EOK)
            return -RT_ERROR;

#ifdef  RT_USING_SFUD
    /* 使用 SFUD 探测  QSPI_DEVICE_NAME 从设备， 将 QSPI_DEVICE_NAME 连接的 FAL_USING_NOR_FLASH_DEV_NAME 初始化为块设备 */
    if (rt_sfud_flash_probe(FAL_USING_NOR_FLASH_DEV_NAME, QSPI_DEVICE_NAME) == RT_NULL)
        return -RT_ERROR;
#endif

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_qspi_flash_with_sfud_init);

static int init(void)
{

#ifdef RT_USING_SFUD
    /* RT-Thread RTOS platform */
    sfud_dev = rt_sfud_flash_find_by_dev_name(FAL_USING_NOR_FLASH_DEV_NAME);
#else
    /* bare metal platform */
    extern sfud_flash sfud_norflash0;
    sfud_dev = &sfud_norflash0;
#endif

    if (NULL == sfud_dev)
    {
        return -1;
    }

    /* update the flash chip information */
    nor_flash0.blk_size = sfud_dev->chip.erase_gran;
    nor_flash0.len = sfud_dev->chip.capacity;

    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    sfud_read(sfud_dev, nor_flash0.addr + offset, size, buf);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    if (sfud_write(sfud_dev, nor_flash0.addr + offset, size, buf) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

static int erase(long offset, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    if (sfud_erase(sfud_dev, nor_flash0.addr + offset, size) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}
#endif /* FAL_USING_SFUD_PORT */

