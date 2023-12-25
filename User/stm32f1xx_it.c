/**
 ****************************************************************************************************
 * @file        stm32f1xx_it.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       所有的中断服务函数入口
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#include "globalE.h"
#include "dataCollect.h"
#include "pwm.h"
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
extern void prvvUARTTxReadyISR(void);                       /* modbus通讯框架提供的回调函数 */
static void tim_it_callback(TIM_HandleTypeDef *tim_handle); /* tim的公共服务函数 */
static void tim_it_callback(TIM_HandleTypeDef *tim_handle)
{
  if (tim_handle == &CF_TIM_PWM_HANDLE) /* 如果用于pwm发波, 调用pwm的回调函数 */
  {
    pwm_update_callback(tim_handle);
  }
  else if (tim_handle == &CF_DATA_TRANS_TIM_HANDLE) /* 如果用于高实时性ADC采集数据处理, 调用dataCollect的回调函数*/
  {
    highLevel_dataTrans_callback();
  }
  else if (tim_handle == &CF_TIM_MB_HANDLE) /* 如果用于modbus通信, 调用modbus提供的回调函数 */
  {
    if (__HAL_TIM_GET_FLAG(&CF_TIM_MB_HANDLE, TIM_FLAG_UPDATE))
    {
      prvvTIMERExpiredISR();
    }
  }
  __HAL_TIM_CLEAR_FLAG(tim_handle, TIM_FLAG_UPDATE); /* 清除中断标志 */
}
/**
 * @brief   TIM2中断处理函数入口
 */
void TIM2_IRQHandler(void)
{
  tim_it_callback(&g_tim2_handle);
}

void TIM3_IRQHandler(void)
{
  tim_it_callback(&g_tim3_handle);
}

void TIM6_IRQHandler(void)
{
  tim_it_callback(&g_tim6_handle);
}

void TIM7_IRQHandler(void)
{
  tim_it_callback(&g_tim7_handle);
}

/******************************************************************************/
/*                          USART Interrupt Handlers                          */
/******************************************************************************/
extern void prvvUARTRxISR(void);
extern void prvvUARTTxReadyISR(void);
static void uart_it_callback(UART_HandleTypeDef *uart_handle);
/**
 * @brief 判断缓冲队列是否为空, 1 = 空, 0 = 非空;
 */
inline static uint8_t buffer_empty()
{
  return g_printf_head == g_printf_end;
}
static void uart_it_callback(UART_HandleTypeDef *uart_handle)
{
  if (uart_handle->Instance == CF_USART_PRINTF) /* 如果当前触发中断的串口是用于重定向 printf 的*/
  {
    if (((CF_USART_PRINTF->CR1 & USART_CR1_TXEIE) != 0) && ((CF_USART_PRINTF->SR & USART_SR_TXE) != 0))
    {
      /**
       * 如果缓冲队列非空
       *    取出队头放到DR
       * 如果队列空
       *    失能TXE,等待下一次fputc唤醒
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
  else if (uart_handle->Instance == CF_USART_MB) /* 如果当前触发中断的串口是用于 FreeModbus 的 */
  {
    /**
     * @brief     USART1 ISR, 负责处理数据收发并与Modbus交互;
     * @attention 发生读缓冲非空中断时, 通知modbus有数据到达, 在modbus框架内部完成对数据的转移
     * @attention 发生写DR为空中断时, 通知modbus可以发送下一个数据, 在modbus框架内部完成对DR的写操作
     */
    if (((CF_USART_MB->CR1 & USART_CR1_RXNEIE) != 0) && ((CF_USART_MB->SR & USART_SR_RXNE) != 0))
    {
      prvvUARTRxISR(); /* 通知modbus有数据到达 */
    }
    if (((CF_USART_MB->CR1 & USART_CR1_TXEIE) != 0) && ((CF_USART_MB->SR & USART_SR_TXE) != 0)) // 发送为空中断标记被置位
    {
      prvvUARTTxReadyISR(); /* 通知modbus数据可以发送 */
    }
  }
}

/**
 * @brief USART1中断入口
 * @note  默认实现为调用公共的串口中断服务回调函数进行输出重定向和FreeModbus的处理,
 * @note  如果有其它需要, 可以修改中断入口
 */
void USART1_IRQHandler(void)
{
  uart_it_callback(&g_uart1_handle);
}

/**
 * @brief USART2中断入口
 * @note  默认实现为调用公共的串口中断服务回调函数进行输出重定向和FreeModbus的处理,
 * @note  如果有其它需要, 可以修改中断入口
 */
void USART2_IRQHandler(void)
{
  uart_it_callback(&g_uart2_handle);
}

/**
 * @brief USART3中断入口
 * @note  默认实现为调用公共的串口中断服务回调函数进行输出重定向和FreeModbus的处理,
 * @note  如果有其它需要, 可以修改中断入口
 */
void USART3_IRQHandler(void)
{
  uart_it_callback(&g_uart3_handle);
}

/**
 * @brief USART4中断入口
 * @note  默认实现为调用公共的串口中断服务回调函数进行输出重定向和FreeModbus的处理,
 * @note  如果有其它需要, 可以修改中断入口
 */
void USART4_IRQHandler(void)
{
  uart_it_callback(&g_uart4_handle);
}

/**
 * @brief USART5中断入口
 * @note  默认实现为调用公共的串口中断服务回调函数进行输出重定向和FreeModbus的处理,
 * @note  如果有其它需要, 可以修改中断入口
 */
void USART5_IRQHandler(void)
{
  uart_it_callback(&g_uart5_handle);
}

/******************************************************************************/
/*                          EXTI Interrupt Handlers                           */
/******************************************************************************/
/**
 * @brief EXTI15-5的公共处理函数
 */
static void EXTIx_callback(uint32_t GPIO_Pin)
{
  BaseType_t yeild = pdFALSE;         /* 用于最后决定是否需要立即切换任务 */
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin); /* 清除中断 */
  switch (GPIO_Pin)
  {
  case GPIO_PIN_5:                        /* EXTI5对应PD5, 是W相晶闸管的导通反馈 */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5); /* 清除中断 */
    if (SV_Sequence_Detect_Flag)          /* 如果需要检测相序 */
    {
      SV_WPWM_Count++; /* W相计数++ */
    }
    else
    {
      // HAL_NVIC_DisableIRQ(EXTI9_5_IRQn); /* FIXME: 相序检测完毕, 应当将对应的中断关闭, 但是因为5-9公用同一个EXTI, 所以这里先不关, 避免把别的引脚中断也关闭了 */
    }
    break;
  default:
    // nothing
  }

  /* 检测是否需要立即切换任务*/
  if (yeild == pdTRUE)
  {
    portYIELD_FROM_ISR(yeild);
    yeild = pdFALSE;
  }
}

/**
 * @brief EXTI3中断服务函数
 * @note  1.清除中断;
 * @note  2.本中断为PD3对应V相晶闸管导通反馈, 将V相计数加一;
 */
void EXTI3_IRQHandler(void)
{
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3); /* 清除中断 */
  if (SV_Sequence_Detect_Flag)          /* 如果需要检测相序 */
  {
    SV_VPWM_Count++; /* V相计数++ */
  }
  else
  {
    HAL_NVIC_DisableIRQ(EXTI3_IRQn); /* 相序检测完毕, 应当将对应的中断关闭, 减少资源消耗 */
  }
}
/**
 * @brief EXTI4中断服务函数
 * @note  1.清除中断;
 * @note  2.本中断为PD4对应U相晶闸管导通反馈, 将U相计数加一;
 */
void EXTI4_IRQHandler(void)
{
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4); /* 清除中断 */
  if (SV_Sequence_Detect_Flag)          /* 如果需要检测相序 */
  {
    SV_UPWM_Count++; /* U相计数++ */
  }
  else
  {
    HAL_NVIC_DisableIRQ(EXTI4_IRQn); /* 相序检测完毕, 应当将对应的中断关闭, 减少资源消耗 */
  }
}

/**
 * @brief   EXTI15_10_IRQn中断的入口
*/
void EXTI9_5_IRQHandler(void)
{
  uint32_t pin_i = 0;
  for (uint8_t i = 5; i <= 9; i++) /* 遍历引脚 5 - 9, 如果遍历到的引脚有中断, 就去处理 */
  {
    pin_i = 1 << i;
    if ((EXTI->PR & pin_i) != 0) /* PR寄存器保存了中断标志, 某位为1标志该位对应的来源产生了中断 */
    {
      EXTIx_callback(pin_i);
    }
  }
}

/**
 * @brief       EXTI15_10_IRQn中断的入口
 * @attention   GPIO引脚可以配置为外部中断的来源,例如引脚 Mode 配置为 GPIO_MODE_IT_RISING时,一个上升沿将触发中断
 * @attention   不同端口的同号引脚(例如PA11和PB11)只能选择一个作为中断源
 */
void EXTI15_10_IRQHandler(void)
{
  uint32_t pin_i = 0;
  for (uint8_t i = 10; i <= 15; i++) /* 遍历引脚10-15, 如果遍历到的引脚有中断, 就去处理 */
  {
    pin_i = 1 << i;
    if ((EXTI->PR & pin_i) != 0) /* PR寄存器保存了中断标志, 某位为1标志该位对应的来源产生了中断 */
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
