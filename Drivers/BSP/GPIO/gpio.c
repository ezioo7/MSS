/**
 ****************************************************************************************************
 * @file        gpio.c
 * @author      ������
 * @version     V1.0
 * @date        2023-04-23
 * @brief       GPIO�˿ڼ���Ų���, ����EXTI
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
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
 * @brief   ʹ������GPIO�˿ڵ�ʱ���ź�, AFIO��ʱ���ź�;
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
 * @brief   ��ʼ��ָ���˿ڵ�ָ������;
 * @param   GPIOx: GPIO�˿�,GPIOA ~ GPIOE;
 *
 * @param   pin: ���ű��, 0x0000 0001����PIN1, 0x0000 0002����PIN2, �Դ�����;
 *  @arg    ����ֱ��ʹ��GPIO_PIN_0 ~ GPIO_PIN_15;
 *
 * @param   mode: ģʽѡ��;
 *  @arg    GPIO_MODE_INPUT, ����;
 *  @arg    GPIO_MODE_OUTPUT_PP, �������;
 *  @arg    GPIO_MODE_OUTPUT_OD, ��©���;
 *  @arg    GPIO_MODE_AF_PP, �����������;
 *  @arg    GPIO_MODE_AF_OD, ���ÿ�©���;
 *  @arg    GPIO_MODE_AF_INPUT, ��������;
 *  @arg    GPIO_MODE_ANALOG, ģ������, ����ADC����;
 *  @arg    GPIO_MODE_IT_RISING, ����, �����źŵ������ش����ⲿ�ж�;
 *  @arg    GPIO_MODE_IT_FALLING, ����, �����ź��½��ش����ⲿ�ж�;
 *  @arg    GPIO_MODE_IT_RISING_FALLING, ����, �����ź�˫���ش����ⲿ�ж�;
 *
 * @param   pull: ������ģʽѡ��
 *  @arg    GPIO_PULLDOWN, ����, Ĭ�ϸ��͵�ƽ;
 *  @arg    GPIO_PULLUP, ����, Ĭ�ϸߵ�ƽ;
 *  @arg    GPIO_NOPULL, ������Ĭ�ϵ�ƽ;
 *
 * @param   speed, ����GPIO�˿ڵ��ٶ�
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
    /* ���ģʽ��û��Ĭ�������ƽ�Ĺ��ܵ�, ����Ҫ���ݲ���"pull"�ֶ����õ�ƽ, pull == GPIO_NOPULLʱ�������� */
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
 * @brief   ���ò�ʹ���ⲿ�ж�;
 * @param   IRQn, �ж����ͺ�, ��stm32f103xe.h;
 *  @arg    USART1_IRQn, TIM2_IRQn, TIM1_UP_IRQn, EXTI0_IRQn, ADC1_2_IRQn, DMA1_Channel1_IRQn;
 *
 * @param   priority, ��ռ���ȼ�; ��FreeRTOS��, ʹ���ж����ȼ�����4, ֻ����ռ���ȼ�, ����Ӧ���ȼ�;
 *  @arg    5 ~ 15, ����Խ�����ȼ�Խ��, �����޷ֱ���configLIBRARY_LOWEST_INTERRUPT_PRIORITY��configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITYȷ��;
 *
 */
void exti_enable(IRQn_Type IRQn, uint32_t priority)
{
    HAL_NVIC_SetPriority(IRQn, priority, 0); /* ����EXTI�ж����ȼ� */
    HAL_NVIC_EnableIRQ(IRQn);                /* ʹ���ж� */
}

/**
 * @brief       ��ʼ��LED0�����������Ϊ�˷������;
 * @attention   LED0Ϊ��ɫ�ƹ⣬ʹ��PE5����;
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
 * @brief led0��;
 */
void led0_on(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief led0��;
 */
void led0_off(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
 * @brief led0��ת״̬;
 */
void led0_toggle(void)
{
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
}
