/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-07     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include "board.h"

#include "fal.h"


#define APP_VERSION                      1L              /**< major version number */
#define APP_SUBVERSION                   0L              /**< minor version number */
#define APP_REVISION                     7L              /**< revise version number */

int main(void)
{
    fal_init( ); /*Tang Huimin add comments  for tesing pull request*/

    LOG_I(" Application Software %d.%d.%d build %s\n",
            APP_VERSION, APP_SUBVERSION, APP_REVISION, __DATE__);

    return RT_EOK;
}


/* 将中断向量表起始地址重新设置为 application 分区的起始地址 */
static int rt_hw_app_vector_reconfig(void)
{
    #define NVIC_VECTOR_MASK   0xFFFFFF80
    #define RT_APP_PART_ADDR 0x08020000
    /* 重新设置中断向量表 */
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VECTOR_MASK;

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_app_vector_reconfig);
