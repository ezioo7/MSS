/**
 ****************************************************************************************************
 * @file        led.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       LED��Ӧ�ò�
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
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
