/**
 ****************************************************************************************************
 * @file        global.h
 * @author      ������
 * @version     V1.0
 * @date        2023-04-23
 * @brief       ��������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * Ӳ��ͼ��,ʹ�õ��Ǵ���1�ʹ���3
 * ����1: PA8ͬ��ʱ��, PA9TX, PA10RX
 * ����3: PC12ͬ��ʱ��, PC10TX, PC11RX
 *
 ****************************************************************************************************
 *
 */
#include "globalE.h"
#include "./BSP/UART/uart.h"
#include "./SYSTEM/sys/sys.h"
#include "./BSP/GPIO/gpio.h"

UART_HandleTypeDef g_uart1_handle = {0};
UART_HandleTypeDef g_uart2_handle = {0};
UART_HandleTypeDef g_uart3_handle = {0};
UART_HandleTypeDef g_uart4_handle = {0};
UART_HandleTypeDef g_uart5_handle = {0};

/**
 * @brief   ����X��ʼ������, ���Զ�ʹ�ܶ�Ӧ��ʱ�Ӻ�����, ���Զ�ʹ��USARTx�ж�;
 * @param   baudrate: ������, �����Լ���Ҫ���ò�����ֵ;
 * @param   itEnable: �Ƿ�ʹ�ܴ����ж�, 0 = �����ж�, 1 = ������
 * @param   priority: USARTx_IRQn ���ж����ȼ�
 */
void uart_enable(USART_TypeDef *Instance, uint32_t baudrate, uint8_t itEnable, uint32_t priority)
{
    UART_HandleTypeDef *uart_handle;
    /* UART ��ʼ������ */
    if (Instance == USART1)
    {
        uart_handle = &g_uart1_handle;
        __HAL_RCC_USART1_CLK_ENABLE();
        /* PA9-TX PA10-RX */
        gpio_config(GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_PULLUP);
        gpio_config(GPIOA, GPIO_PIN_10, GPIO_MODE_AF_INPUT, GPIO_PULLUP);
        if (itEnable)
        {
            HAL_NVIC_SetPriority(USART1_IRQn, priority, 0);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
        }
    }
    else if (Instance == USART2)
    {
        uart_handle = &g_uart2_handle;
        __HAL_RCC_USART2_CLK_ENABLE();
        /* PA2-TX, PA3_RX */
        gpio_config(GPIOA, GPIO_PIN_2, GPIO_MODE_AF_PP, GPIO_PULLUP);
        gpio_config(GPIOA, GPIO_PIN_3, GPIO_MODE_AF_INPUT, GPIO_PULLUP);
        if (itEnable)
        {
            HAL_NVIC_SetPriority(USART2_IRQn, priority, 0);
            HAL_NVIC_EnableIRQ(USART2_IRQn);
        }
    }
    else if (Instance == USART3)
    {
        uart_handle = &g_uart3_handle;
        __HAL_RCC_USART3_CLK_ENABLE();
        /* PC10-TX, PC11-RX */
        gpio_config(GPIOC, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_PULLUP);
        gpio_config(GPIOC, GPIO_PIN_11, GPIO_MODE_AF_INPUT, GPIO_PULLUP);
        if (itEnable)
        {
            HAL_NVIC_SetPriority(USART3_IRQn, priority, 0);
            HAL_NVIC_EnableIRQ(USART3_IRQn);
        }
    }
    else if (Instance == UART4)
    {
        uart_handle = &g_uart4_handle;
        /* ����ʱ�Ӻ��ж� */
        /* code */
    }
    else if (Instance == UART5)
    {
        uart_handle = &g_uart5_handle;
        /* ����ʱ�Ӻ��ж� */
        /* code */
    }
    uart_handle->Instance = Instance;
    uart_handle->Init.BaudRate = baudrate;             /* ������, ����9600 */
    uart_handle->Init.WordLength = UART_WORDLENGTH_8B; /* һ������֡����8λ���� */
    uart_handle->Init.StopBits = UART_STOPBITS_1;      /* һ��ֹͣλ */
    uart_handle->Init.Parity = UART_PARITY_NONE;       /* FIXME:����Ӧ����������ͳһ, ������Ĭ����У��λ */
    uart_handle->Init.HwFlowCtl = UART_HWCONTROL_NONE; /* �� Ӳ�� ������ */
    uart_handle->Init.Mode = UART_MODE_TX_RX;          /* ֧���պͷ�, ȫ˫�� */
    HAL_UART_Init(uart_handle);

    /* �������ж�, ��ʱ����DR�ǿ�ʱ�ᴥ���ж�, ��Ҫ���жϷ��������ֶ���ȡDR�Ĵ�����buffer */
    __HAL_UART_ENABLE_IT(uart_handle, UART_IT_RXNE); /* ʹ�ܽ��շǿ��ж� */
}
