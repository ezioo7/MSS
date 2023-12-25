/**
 ****************************************************************************************************
 * @file        control.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-18
 * @brief       控制信号输入输出
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 *
 ****************************************************************************************************
 * @attention
 * GPIO输入输出控制信号
 * PE9, 输出, 控制柜体风机启停
 * ??????????????????????? PD6 PB7 与发波控制开关相关, 具体是输入还是输出不确定 ???????????
 * PE0 PB8 PB9  扩展卡继电器输出
 * PE4 PE5 PE6  X4继电器输出
 * PB7 PE1 PE2 PE3  继电器输入数字信号
 */
#include "./BSP/GPIO/gpio.h"
#include "globalE.h"
#include "control.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief   输入控制信号GPIO初始化
 */
void control_input_init(void)
{
    /* PB7 PE1 PE2 PE3  继电器输入数字信号 */
    gpio_config(GPIOB, GPIO_PIN_7, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_1, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLDOWN);
}

/**
 * @brief   输出控制信号GPIO初始化
 */
void control_output_init(void)
{
    /* PE9, 输出, 控制柜体风机启停 */
    gpio_config(GPIOE, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* PE0 PB8 PB9  扩展卡继电器输出 */
    gpio_config(GPIOE, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* PE4 PE5 PE6  X4继电器输出 */
    gpio_config(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* FIXME: 先给高电平, 防止LED1亮 */
    gpio_config(GPIOE, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP);
}
