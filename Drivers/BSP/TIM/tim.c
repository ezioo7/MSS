/**
 ****************************************************************************************************
 * @file        global.h
 * @author      电气组
 * @version     V1.0
 * @date        2023-04-23
 * @brief       全局变量A页
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
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
 * @brief   定时器初始化并以使能更新中断的方式计数;
 * @param   tim: 要初始化的定时器, MSS_TIM1 ~ MSS_TIM8;
 * @param   psc: 预分频系数, 可取 0 ~ 65535, 最终得到的计数器计数频率为 72M/(psc+1);
 *  @arg    71, 每 1us 计数一次;
 *  @arg    7199, 每 0.1ms 计数一次;
 * @param   arr: 可取 0 ~ 65535, 在向上计数和向下计数时, 计数 arr+1 次溢出;
 * @param   priority: 更新中断的优先级, 使用RTOS管理时可取 5 ~ 15, 数字越小优先级越高;
 * @param   start: 是否立即开始计数, FALSE = 否, TRUE = 是;
 */
void tim_update_enable(MSS_TIM *tim, uint32_t psc, uint32_t arr, uint32_t priority, uint8_t start)
{

    tim->handle->Instance = tim->Instance;
    tim->handle->Init.Prescaler = psc;
    tim->handle->Init.Period = arr;
    tim->handle->Init.CounterMode = TIM_COUNTERMODE_UP;                   /* 默认向上计数 */
    tim->handle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; /* 默认关闭预装载功能, 修改ARR立即生效 */

    /* 使能时钟, 添加一个读操作是为了等待时钟稳定 */
    __IO uint32_t tmpreg;
    if (tim->APBn == 1) /* 如果是挂载在APB1总线上, 进行相应的时钟使能 */
    {
        SET_BIT(RCC->APB1ENR, tim->RCC_en);
        tmpreg = READ_BIT(RCC->APB1ENR, tim->RCC_en);
    }
    else if (tim->APBn == 2) /* 如果是挂载在APB2总线上, 进行相应的时钟使能 */
    {
        SET_BIT(RCC->APB2ENR, tim->RCC_en);
        tmpreg = READ_BIT(RCC->APB2ENR, tim->RCC_en);
    }
    UNUSED(tmpreg);

    /* 配置并使能中断 */
    HAL_NVIC_SetPriority(tim->Update_IRQn, priority, 0);
    HAL_NVIC_EnableIRQ(tim->Update_IRQn);

    /* HAL_TIM_Base_Init会产生一个更新事件(用于装载psc), 进而导致更新中断, 需要手动清除这个中断 */
    HAL_TIM_Base_Init(tim->handle);
    __HAL_TIM_CLEAR_FLAG(tim->handle, TIM_FLAG_UPDATE);

    /* 使能更新中断但不开启定时器 */
    __HAL_TIM_ENABLE_IT(tim->handle, TIM_IT_UPDATE);

    /* 根据传参判断是否开启定时器 */
    if (start)
    {
        __HAL_TIM_SET_COUNTER(tim->handle, 0); /* 清0计数器 */
        __HAL_TIM_ENABLE(tim->handle);         /* 开始计数 */
    }
}
