/**
 ****************************************************************************************************
 * @file        stm32f1xx_it.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       ���е��жϷ��������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#include "globalE.h"
#include "dataCollect.h"
#include "pwmSoft.h"
#include "pwm3TIM.h"
#include "SystemSafety.h"
#include "./SYSTEM/sys/sys.h"
#include "./BSP/TIM/tim.h"
#include "./BSP/UART/uart.h"
#include "./SYSTEM/usart/usart.h"
#include "stm32f1xx_it.h"
#include "FreeRTOS.h"

/******************************************************************************/
/*                          TIM Interrupt Handlers                            */
/******************************************************************************/
extern void prvvUARTTxReadyISR(void);                       /* modbusͨѶ����ṩ�Ļص����� */
static void tim_IT_callback(TIM_HandleTypeDef *tim_handle); /* tim�Ĺ��������� */
static void tim_IT_callback(TIM_HandleTypeDef *tim_handle)
{
    __HAL_TIM_CLEAR_FLAG(tim_handle, TIM_FLAG_UPDATE); /* ����жϱ�־ */

    if (0)
        ; /* ���������Ƿ�CF_PWM_USE_3TIM��������� */

#ifdef CF_PWM_USE_3TIM
    else if (tim_handle == &CF_TIM_PWM_PHASE_A_HANDLE)
    {
        pwm_phaseA_IT_callback();
    }
    else if (tim_handle == &CF_TIM_PWM_PULLUP_HANDLE)
    {
        pwm_pullup_IT_callback();
    }
    else if (tim_handle == &CF_TIM_PWM_PULLDOWN_HANDLE)
    {
        pwm_pulldown_IT_callback();
    }
#endif

#ifdef CF_PWM_USE_SOFT
    else if (tim_handle == &CF_TIM_PWM_HANDLE) /* �������pwm����, ����pwm�Ļص����� */
    {
        pwm_update_IT_callback(tim_handle);
    }
#endif

    else if (tim_handle == &CF_DATA_TRANS_TIM_HANDLE) /* ������ڸ�ʵʱ��ADC�ɼ����ݴ���, ����dataCollect�Ļص�����*/
    {
        highLevel_dataTrans_callback();
    }
    else if (tim_handle == &CF_TIM_MB_HANDLE) /* �������modbusͨ��, ����modbus�ṩ�Ļص����� */
    {
        if (__HAL_TIM_GET_FLAG(&CF_TIM_MB_HANDLE, TIM_FLAG_UPDATE))
        {
            prvvTIMERExpiredISR();
        }
    }
}
/**
 * @brief   TIM2�жϴ��������
 */
void TIM2_IRQHandler(void)
{
    tim_IT_callback(&g_tim2_handle);
}

void TIM3_IRQHandler(void)
{
    tim_IT_callback(&g_tim3_handle);
}

void TIM4_IRQHandler(void)
{
    tim_IT_callback(&g_tim4_handle);
}

void TIM5_IRQHandler(void)
{
    tim_IT_callback(&g_tim5_handle);
}

void TIM6_IRQHandler(void)
{
    tim_IT_callback(&g_tim6_handle);
}

void TIM7_IRQHandler(void)
{
    tim_IT_callback(&g_tim7_handle);
}

/******************************************************************************/
/*                          USART Interrupt Handlers                          */
/******************************************************************************/
extern void prvvUARTRxISR(void);
extern void prvvUARTTxReadyISR(void);
static void uart_it_callback(UART_HandleTypeDef *uart_handle);
/**
 * @brief �жϻ�������Ƿ�Ϊ��, 1 = ��, 0 = �ǿ�;
 */
inline static uint8_t buffer_empty()
{
    return g_printf_head == g_printf_end;
}
static void uart_it_callback(UART_HandleTypeDef *uart_handle)
{
    if (uart_handle->Instance == CF_USART_PRINTF) /* �����ǰ�����жϵĴ����������ض��� printf ��*/
    {
        if (((CF_USART_PRINTF->CR1 & USART_CR1_TXEIE) != 0) && ((CF_USART_PRINTF->SR & USART_SR_TXE) != 0))
        {
            /**
             * ���������зǿ�
             *    ȡ����ͷ�ŵ�DR
             * ������п�
             *    ʧ��TXE,�ȴ���һ��fputc����
             */
            if (buffer_empty())
            {
                __HAL_UART_DISABLE_IT(&CF_USART_PRINTF_HANDLE, UART_IT_TXE);
            }
            else
            {
                CF_USART_PRINTF->DR = g_printf_buffer[g_printf_head];
                g_printf_head = (g_printf_head + 1) % CF_PRINTF_BUFFER_SIZE;
            }
        }
    }
    else if (uart_handle->Instance == CF_USART_MB) /* �����ǰ�����жϵĴ��������� FreeModbus �� */
    {
        /**
         * @brief     USART1 ISR, �����������շ�����Modbus����;
         * @attention ����������ǿ��ж�ʱ, ֪ͨmodbus�����ݵ���, ��modbus����ڲ���ɶ����ݵ�ת��
         * @attention ����дDRΪ���ж�ʱ, ֪ͨmodbus���Է�����һ������, ��modbus����ڲ���ɶ�DR��д����
         */
        if (((CF_USART_MB->CR1 & USART_CR1_RXNEIE) != 0) && ((CF_USART_MB->SR & USART_SR_RXNE) != 0))
        {
            prvvUARTRxISR(); /* ֪ͨmodbus�����ݵ��� */
        }
        if (((CF_USART_MB->CR1 & USART_CR1_TXEIE) != 0) && ((CF_USART_MB->SR & USART_SR_TXE) != 0)) // ����Ϊ���жϱ�Ǳ���λ
        {
            prvvUARTTxReadyISR(); /* ֪ͨmodbus���ݿ��Է��� */
        }
    }
}

/**
 * @brief USART1�ж����
 * @note  Ĭ��ʵ��Ϊ���ù����Ĵ����жϷ���ص�������������ض����FreeModbus�Ĵ���,
 * @note  �����������Ҫ, �����޸��ж����
 */
void USART1_IRQHandler(void)
{
    uart_it_callback(&g_uart1_handle);
}

/**
 * @brief USART2�ж����
 * @note  Ĭ��ʵ��Ϊ���ù����Ĵ����жϷ���ص�������������ض����FreeModbus�Ĵ���,
 * @note  �����������Ҫ, �����޸��ж����
 */
void USART2_IRQHandler(void)
{
    uart_it_callback(&g_uart2_handle);
}

/**
 * @brief USART3�ж����
 * @note  Ĭ��ʵ��Ϊ���ù����Ĵ����жϷ���ص�������������ض����FreeModbus�Ĵ���,
 * @note  �����������Ҫ, �����޸��ж����
 */
void USART3_IRQHandler(void)
{
    uart_it_callback(&g_uart3_handle);
}

/**
 * @brief USART4�ж����
 * @note  Ĭ��ʵ��Ϊ���ù����Ĵ����жϷ���ص�������������ض����FreeModbus�Ĵ���,
 * @note  �����������Ҫ, �����޸��ж����
 */
void USART4_IRQHandler(void)
{
    uart_it_callback(&g_uart4_handle);
}

/**
 * @brief USART5�ж����
 * @note  Ĭ��ʵ��Ϊ���ù����Ĵ����жϷ���ص�������������ض����FreeModbus�Ĵ���,
 * @note  �����������Ҫ, �����޸��ж����
 */
void USART5_IRQHandler(void)
{
    uart_it_callback(&g_uart5_handle);
}

/******************************************************************************/
/*                          EXTI Interrupt Handlers                           */
/******************************************************************************/
/**
 * @brief EXTI15-5�Ĺ���������
 */
static void EXTIx_callback(uint32_t GPIO_Pin)
{
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin); /* ����ж� */
    switch (GPIO_Pin)
    {
    case GPIO_PIN_5:                          /* EXTI5��ӦPD5, ��W�ྦྷբ�ܵĵ�ͨ���� */
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5); /* ����ж� */
        if (SV_Sequence_Detect_Flag)          /* �����Ҫ������� */
        {
            SV_WPWM_Count++; /* W�����++ */
        }
        else
        {
            SV_WPWM_Count = 0; /* ������� */
                               // HAL_NVIC_DisableIRQ(EXTI9_5_IRQn); /* FIXME: ���������, Ӧ������Ӧ���жϹر�, ������Ϊ5-9����ͬһ��EXTI, ���������Ȳ���, ����ѱ�������ж�Ҳ�ر��� */
        }
        break;
    }
}

/**
 * @brief EXTI0�жϷ�����
 * @note  ��ӦPD0, U���ѹ��������
 */
static uint8_t SV_U_Edge = 1; /* 0 = ���½��ش����ı����ж�, 1 = �����ش��� */
void EXTI0_IRQHandler(void)
{
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0); /* ����ж� */
    if (SV_U_Edge)                        /* ����������� */
    {
        /* �����ر�ʾU��һ�����ڿ�ʼ, ��ʱ����������0, ��ʼ��¼һ�������ڵ����ֵ */
        SV_U_MaxI = 0;
        SV_U_Edge = 0;
    }
    else
    {
        /* �½���˵�����������, ���������ڵ��������ֵ���Ը���2����������Чֵ */
        AM_EffectiveU_I = SV_U_MaxI / 1.4142;
        SV_U_Edge = 1;
    }
}

/**
 * @brief EXTI1�жϷ�����
 * @note  ��ӦPD1, V���ѹ��������
 */
static uint8_t SV_V_Edge = 1; /* 0 = ���½��ش����ı����ж�, 1 = �����ش��� */
void EXTI1_IRQHandler(void)
{
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1); /* ����ж� */
    if (SV_V_Edge)                        /* ����������� */
    {
        /* �����ر�ʾU��һ�����ڿ�ʼ, ��ʱ����������0, ��ʼ��¼һ�������ڵ����ֵ */
        SV_V_MaxI = 0;
        SV_V_Edge = 0;
    }
    else
    {
        /* �½���˵�����������, ���������ڵ��������ֵ���Ը���2����������Чֵ */
        AM_EffectiveV_I = SV_V_MaxI / 1.4142;
        SV_V_Edge = 1;
    }
}

/**
 * @brief EXTI2�жϷ�����
 * @note  ��ӦPD2, W���ѹ��������
 */
static uint8_t SV_W_Edge = 1; /* 0 = ���½��ش����ı����ж�, 1 = �����ش��� */
void EXTI2_IRQHandler(void)
{
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2); /* ����ж� */
    if (SV_W_Edge)                        /* ����������� */
    {
        /* �����ر�ʾU��һ�����ڿ�ʼ, ��ʱ����������0, ��ʼ��¼һ�������ڵ����ֵ */
        SV_W_MaxI = 0;
        SV_W_Edge = 0;
    }
    else
    {
        /* �½���˵�����������, ���������ڵ��������ֵ���Ը���2����������Чֵ */
        AM_EffectiveW_I = SV_W_MaxI / 1.4142;
        SV_W_Edge = 1;
    }
}

/**
 * @brief EXTI3�жϷ�����
 * @note  1.����ж�;
 * @note  2.���ж�ΪPD3��ӦV�ྦྷբ�ܵ�ͨ����, ��V�������һ;
 */
void EXTI3_IRQHandler(void)
{
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3); /* ����ж� */
    if (SV_Sequence_Detect_Flag)          /* �����Ҫ������� */
    {
        SV_VPWM_Count++; /* V�����++ */
    }
    else
    {
        SV_VPWM_Count = 0;               /* ���ü��� */
        HAL_NVIC_DisableIRQ(EXTI3_IRQn); /* ���������, Ӧ������Ӧ���жϹر�, ������Դ���� */
    }
}
/**
 * @brief EXTI4�жϷ�����
 * @note  1.����ж�;
 * @note  2.���ж�ΪPD4��ӦU�ྦྷբ�ܵ�ͨ����, ��U�������һ;
 */
void EXTI4_IRQHandler(void)
{
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4); /* ����ж� */
    if (SV_Sequence_Detect_Flag)          /* �����Ҫ������� */
    {
        SV_UPWM_Count++; /* U�����++ */
    }
    else
    {
        SV_UPWM_Count = 0;               /* ���ü��� */
        HAL_NVIC_DisableIRQ(EXTI4_IRQn); /* ���������, Ӧ������Ӧ���жϹر�, ������Դ���� */
    }
}

/**
 * @brief   EXTI15_10_IRQn�жϵ����
 */
void EXTI9_5_IRQHandler(void)
{
    uint32_t pin_i = 0;
    for (uint8_t i = 5; i <= 9; i++) /* �������� 5 - 9, ������������������ж�, ��ȥ���� */
    {
        pin_i = 1 << i;
        if ((EXTI->PR & pin_i) != 0) /* PR�Ĵ����������жϱ�־, ĳλΪ1��־��λ��Ӧ����Դ�������ж� */
        {
            EXTIx_callback(pin_i);
        }
    }
}

/**
 * @brief       EXTI15_10_IRQn�жϵ����
 * @attention   GPIO���ſ�������Ϊ�ⲿ�жϵ���Դ,�������� Mode ����Ϊ GPIO_MODE_IT_RISINGʱ,һ�������ؽ������ж�
 * @attention   ��ͬ�˿ڵ�ͬ������(����PA11��PB11)ֻ��ѡ��һ����Ϊ�ж�Դ
 */
void EXTI15_10_IRQHandler(void)
{
    uint32_t pin_i = 0;
    for (uint8_t i = 10; i <= 15; i++) /* ��������10-15, ������������������ж�, ��ȥ���� */
    {
        pin_i = 1 << i;
        if ((EXTI->PR & pin_i) != 0) /* PR�Ĵ����������жϱ�־, ĳλΪ1��־��λ��Ӧ����Դ�������ж� */
        {
            EXTIx_callback(pin_i);
        }
    }
}

/******************************************************************************/
/*                          Other Interrupt Handlers                          */
/******************************************************************************/
/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}
