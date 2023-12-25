/**
 ****************************************************************************************************
 * @file        global.h
 * @author      电气组
 * @version     V1.0
 * @date        2023-04-23
 * @brief       串口驱动
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * 硬件图中,使用的是串口1和串口3
 * 串口1: PA8同步时钟, PA9TX, PA10RX
 * 串口3: PC12同步时钟, PC10TX, PC11RX
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
 * @brief   串口X初始化函数, 会自动使能对应的时钟和引脚, 并自动使能USARTx中断;
 * @param   baudrate: 波特率, 根据自己需要设置波特率值;
 * @param   itEnable: 是否使能串口中断, 0 = 屏蔽中断, 1 = 不屏蔽
 * @param   priority: USARTx_IRQn 的中断优先级
 */
void uart_enable(USART_TypeDef *Instance, uint32_t baudrate, uint8_t itEnable, uint32_t priority)
{
    UART_HandleTypeDef *uart_handle;
    /* UART 初始化设置 */
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
        /* 配置时钟和中断 */
        /* code */
    }
    else if (Instance == UART5)
    {
        uart_handle = &g_uart5_handle;
        /* 配置时钟和中断 */
        /* code */
    }
    uart_handle->Instance = Instance;
    uart_handle->Init.BaudRate = baudrate;             /* 波特率, 常用9600 */
    uart_handle->Init.WordLength = UART_WORDLENGTH_8B; /* 一个数据帧中有8位数据 */
    uart_handle->Init.StopBits = UART_STOPBITS_1;      /* 一个停止位 */
    uart_handle->Init.Parity = UART_PARITY_NONE;       /* FIXME:后续应当与操作面板统一, 这里先默认无校验位 */
    uart_handle->Init.HwFlowCtl = UART_HWCONTROL_NONE; /* 无 硬件 流控制 */
    uart_handle->Init.Mode = UART_MODE_TX_RX;          /* 支持收和发, 全双工 */
    HAL_UART_Init(uart_handle);

    /* 开启读中断, 此时接收DR非空时会触发中断, 需要在中断服务函数中手动读取DR寄存器到buffer */
    __HAL_UART_ENABLE_IT(uart_handle, UART_IT_RXNE); /* 使能接收非空中断 */
}
