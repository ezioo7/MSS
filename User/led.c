/**
 ****************************************************************************************************
 * @file        led.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       LED灯应用层
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 * 
 ****************************************************************************************************
 *
 */
#include "./BSP/GPIO/gpio.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"


void led0_task(void *pvParameters)
{
    led0_config();
    while (1)
    {
        vTaskDelay(500);
        led0_toggle();
    }
}
