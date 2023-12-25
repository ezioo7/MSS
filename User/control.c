/**
 ****************************************************************************************************
 * @file        control.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-18
 * @brief       �����ź��������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 *
 ****************************************************************************************************
 * @attention
 * GPIO������������ź�
 * PE9, ���, ���ƹ�������ͣ
 * ??????????????????????? PD6 PB7 �뷢�����ƿ������, ���������뻹�������ȷ�� ???????????
 * PE0 PB8 PB9  ��չ���̵������
 * PE4 PE5 PE6  X4�̵������
 * PB7 PE1 PE2 PE3  �̵������������ź�
 */
#include "./BSP/GPIO/gpio.h"
#include "globalE.h"
#include "control.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief   ��������ź�GPIO��ʼ��
 */
void control_input_init(void)
{
    /* PB7 PE1 PE2 PE3  �̵������������ź� */
    gpio_config(GPIOB, GPIO_PIN_7, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_1, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLDOWN);
}

/**
 * @brief   ��������ź�GPIO��ʼ��
 */
void control_output_init(void)
{
    /* PE9, ���, ���ƹ�������ͣ */
    gpio_config(GPIOE, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* PE0 PB8 PB9  ��չ���̵������ */
    gpio_config(GPIOE, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* PE4 PE5 PE6  X4�̵������ */
    gpio_config(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOE, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* FIXME: �ȸ��ߵ�ƽ, ��ֹLED1�� */
    gpio_config(GPIOE, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP);
}
