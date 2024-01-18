/**
 ****************************************************************************************************
 * @file        gpio.c
 * @author      电气组
 * @version     V1.0
 * @date        2023-04-23
 * @brief       GPIO端口及针脚操作, 包括EXTI
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#include "./SYSTEM/sys/sys.h"
#include "./BSP/GPIO/gpio.h"
#include "globalE.h"
#include "FreeRTOS.h"
#include "task.h"

/* static functions */

/**
 * @brief   使能所有GPIO端口的时钟信号, AFIO的时钟信号;
 */
void gpio_clk_enable(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
}

/**
 * @brief   初始化指定端口的指定引脚;
 * @param   GPIOx: GPIO端口,GPIOA ~ GPIOE;
 *
 * @param   pin: 引脚编号, 0x0000 0001代表PIN1, 0x0000 0002代表PIN2, 以此类推;
 *  @arg    建议直接使用GPIO_PIN_0 ~ GPIO_PIN_15;
 *
 * @param   mode: 模式选择;
 *  @arg    GPIO_MODE_INPUT, 输入;
 *  @arg    GPIO_MODE_OUTPUT_PP, 推挽输出;
 *  @arg    GPIO_MODE_OUTPUT_OD, 开漏输出;
 *  @arg    GPIO_MODE_AF_PP, 复用推挽输出;
 *  @arg    GPIO_MODE_AF_OD, 复用开漏输出;
 *  @arg    GPIO_MODE_AF_INPUT, 复用输入;
 *  @arg    GPIO_MODE_ANALOG, 模拟输入, 用于ADC输入;
 *  @arg    GPIO_MODE_IT_RISING, 输入, 输入信号的上升沿触发外部中断;
 *  @arg    GPIO_MODE_IT_FALLING, 输入, 输入信号下降沿触发外部中断;
 *  @arg    GPIO_MODE_IT_RISING_FALLING, 输入, 输入信号双边沿触发外部中断;
 *
 * @param   pull: 上下拉模式选择
 *  @arg    GPIO_PULLDOWN, 下拉, 默认给低电平;
 *  @arg    GPIO_PULLUP, 上拉, 默认高电平;
 *  @arg    GPIO_NOPULL, 不设置默认电平;
 *
 * @param   speed, 配置GPIO端口的速度
 *  @arg    GPIO_SPEED_FREQ_HIGH, 50MHz;
 *  @arg    GPIO_SPEED_FREQ_MEDIUM, 10MHz;
 *  @arg    GPIO_SPEED_FREQ_LOW, 2MHz;
 *
 */
void gpio_init(GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t mode, uint32_t pull)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    gpio_init_struct.Pin = pin;
    gpio_init_struct.Mode = mode;
    gpio_init_struct.Pull = pull;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    /* 输出模式是没有默认输出电平的功能的, 所以要根据参数"pull"手动设置电平, pull == GPIO_NOPULL时不用设置 */
    if ((mode == GPIO_MODE_OUTPUT_PP) || (mode == GPIO_MODE_OUTPUT_OD) || (mode == GPIO_MODE_AF_PP) || (mode == GPIO_MODE_AF_OD))
    {
        if (pull == GPIO_PULLUP)
        {
            HAL_GPIO_WritePin(GPIOx, pin, GPIO_PIN_SET);
        }
        else if (pull == GPIO_PULLDOWN)
        {
            HAL_GPIO_WritePin(GPIOx, pin, GPIO_PIN_RESET);
        }
    }
    HAL_GPIO_Init(GPIOx, &gpio_init_struct);
}

/**
 * @brief   配置并使能外部中断;
 * @param   IRQn, 中断类型号, 见stm32f103xe.h;
 *  @arg    USART1_IRQn, TIM2_IRQn, TIM1_UP_IRQn, EXTI0_IRQn, ADC1_2_IRQn, DMA1_Channel1_IRQn;
 *
 * @param   priority, 抢占优先级; 在FreeRTOS中, 使用中断优先级分组4, 只有抢占优先级, 无响应优先级;
 *  @arg    5 ~ 15, 数字越低优先级越高, 上下限分别由configLIBRARY_LOWEST_INTERRUPT_PRIORITY和configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY确定;
 *
 */
void exti_enable(IRQn_Type IRQn, uint32_t priority)
{
    HAL_NVIC_SetPriority(IRQn, priority, 0); /* 配置EXTI中断优先级 */
    HAL_NVIC_EnableIRQ(IRQn);                /* 使能中断 */
}

/**
 * @brief       初始化LED0，单独拎出来为了方便测试;
 * @attention   LED0为红色灯光，使用PE5引脚;
 */
void led0_config(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    gpio_init_struct.Pin = GPIO_PIN_5;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);
    HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief led0亮;
 */
void led0_on(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief led0灭;
 */
void led0_off(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief led0翻转状态;
 */
void led0_toggle(void)
{
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
}
