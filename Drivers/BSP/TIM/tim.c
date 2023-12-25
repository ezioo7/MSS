/**
 ****************************************************************************************************
 * @file        global.h
 * @author      ������
 * @version     V1.0
 * @date        2023-04-23
 * @brief       ȫ�ֱ���Aҳ
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#include "globalE.h"
#include "./BSP/TIM/tim.h"
#include "./SYSTEM/sys/sys.h"

TIM_HandleTypeDef g_tim1_handle = {0};
TIM_HandleTypeDef g_tim2_handle = {0};
TIM_HandleTypeDef g_tim3_handle = {0};
TIM_HandleTypeDef g_tim4_handle = {0};
TIM_HandleTypeDef g_tim5_handle = {0};
TIM_HandleTypeDef g_tim6_handle = {0};
TIM_HandleTypeDef g_tim7_handle = {0};
TIM_HandleTypeDef g_tim8_handle = {0};

MSS_TIM mss_tim[8] =
    {
        {&g_tim1_handle, TIM1, 2, RCC_APB2ENR_TIM1EN, TIM1_UP_IRQn},
        {&g_tim2_handle, TIM2, 1, RCC_APB1ENR_TIM2EN, TIM2_IRQn},
        {&g_tim3_handle, TIM3, 1, RCC_APB1ENR_TIM3EN, TIM3_IRQn},
        {&g_tim4_handle, TIM4, 1, RCC_APB1ENR_TIM4EN, TIM4_IRQn},
        {&g_tim5_handle, TIM5, 1, RCC_APB1ENR_TIM5EN, TIM5_IRQn},
        {&g_tim6_handle, TIM6, 1, RCC_APB1ENR_TIM6EN, TIM6_IRQn},
        {&g_tim7_handle, TIM7, 1, RCC_APB1ENR_TIM7EN, TIM7_IRQn},
        {&g_tim8_handle, TIM8, 2, RCC_APB2ENR_TIM1EN, TIM8_UP_IRQn}};

/**
 * @brief   ��ʱ����ʼ������ʹ�ܸ����жϵķ�ʽ����;
 * @param   tim: Ҫ��ʼ���Ķ�ʱ��, MSS_TIM1 ~ MSS_TIM8;
 * @param   psc: Ԥ��Ƶϵ��, ��ȡ 0 ~ 65535, ���յõ��ļ���������Ƶ��Ϊ 72M/(psc+1);
 *  @arg    71, ÿ 1us ����һ��;
 *  @arg    7199, ÿ 0.1ms ����һ��;
 * @param   arr: ��ȡ 0 ~ 65535, �����ϼ��������¼���ʱ, ���� arr+1 �����;
 * @param   priority: �����жϵ����ȼ�, ʹ��RTOS����ʱ��ȡ 5 ~ 15, ����ԽС���ȼ�Խ��;
 * @param   start: �Ƿ�������ʼ����, FALSE = ��, TRUE = ��;
 */
void tim_update_enable(MSS_TIM *tim, uint32_t psc, uint32_t arr, uint32_t priority, uint8_t start)
{

    tim->handle->Instance = tim->Instance;
    tim->handle->Init.Prescaler = psc;
    tim->handle->Init.Period = arr;
    tim->handle->Init.CounterMode = TIM_COUNTERMODE_UP;                   /* Ĭ�����ϼ��� */
    tim->handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; /* Ĭ�Ϲر�Ԥװ�ع���, �޸�ARR������Ч */

    /* ʹ��ʱ��, ���һ����������Ϊ�˵ȴ�ʱ���ȶ� */
    __IO uint32_t tmpreg;
    if (tim->APBn == 1) /* ����ǹ�����APB1������, ������Ӧ��ʱ��ʹ�� */
    {
        SET_BIT(RCC->APB1ENR, tim->RCC_en);
        tmpreg = READ_BIT(RCC->APB1ENR, tim->RCC_en);
    }
    else if (tim->APBn == 2) /* ����ǹ�����APB2������, ������Ӧ��ʱ��ʹ�� */
    {
        SET_BIT(RCC->APB2ENR, tim->RCC_en);
        tmpreg = READ_BIT(RCC->APB2ENR, tim->RCC_en);
    }
    UNUSED(tmpreg);

    /* ���ò�ʹ���ж� */
    HAL_NVIC_SetPriority(tim->Update_IRQn, priority, 0);
    HAL_NVIC_EnableIRQ(tim->Update_IRQn);

    /* HAL_TIM_Base_Init�����һ�������¼�(����װ��psc), �������¸����ж�, ��Ҫ�ֶ��������ж� */
    HAL_TIM_Base_Init(tim->handle);
    __HAL_TIM_CLEAR_FLAG(tim->handle, TIM_FLAG_UPDATE);

    /* ʹ�ܸ����жϵ���������ʱ�� */
    __HAL_TIM_ENABLE_IT(tim->handle, TIM_IT_UPDATE);

    /* ���ݴ����ж��Ƿ�����ʱ�� */
    if (start)
    {
        __HAL_TIM_SET_COUNTER(tim->handle, 0); /* ��0������ */
        __HAL_TIM_ENABLE(tim->handle);         /* ��ʼ���� */
    }
}
