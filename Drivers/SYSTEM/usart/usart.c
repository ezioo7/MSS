/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-11
 * @brief       ���ڳ�ʼ�����룬֧��printf
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 * �޸�˵��
 * V1.0 20211103
 * ��һ�η���
 *
 ****************************************************************************************************
 */
#include "./globalA.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/UART/uart.h"

/******************************************************************************************/
/* �������´���, ֧��printf����, ������Ҫѡ��use MicroLIB */

#if 1

#if (__ARMCC_VERSION >= 6010050)           /* ʹ��AC6������ʱ */
__asm(".global __use_no_semihosting\n\t"); /* ������ʹ�ð�����ģʽ */
__asm(".global __ARM_use_no_argv \n\t");   /* AC6����Ҫ����main����Ϊ�޲�����ʽ�����򲿷����̿��ܳ��ְ�����ģʽ */

#else
/* ʹ��AC5������ʱ, Ҫ�����ﶨ��__FILE �� ��ʹ�ð�����ģʽ */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* ��ʹ�ð�����ģʽ��������Ҫ�ض���_ttywrch\_sys_exit\_sys_command_string����,��ͬʱ����AC6��AC5ģʽ */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE �� stdio.h���涨��. */
FILE __stdout;

/******************************************************************************/
/*                             ��Ҫ��ע�Ĳ���                                  */
/******************************************************************************/
uint8_t g_printf_buffer[CF_PRINTF_BUFFER_SIZE] = {0}; /* printf ��������ݻ��Ȼ��浽��ѭ��������, Ȼ������жϷ�ʽ���ͳ�ȥ, ����ʱ�����������ַ� */
uint16_t g_printf_head = 0;                            /* ��ͷ, ָ����ڵ�һ��Ԫ�� */
uint16_t g_printf_end = 0;                             /* ��β, ָ����һ��Ԫ�صĲ���λ��, �� head == end ʱ���п�, (end+1)%size == head ʱ����*/
/**
 * @brief �жϻ��������Ƿ��пռ�, 0 = ��������, 1 = ���пռ�;
 */
inline static uint8_t buffer_available()
{
    return !(((g_printf_end + 1) % CF_PRINTF_BUFFER_SIZE) == g_printf_head);
}

/* MDK����Ҫ�ض���fputc����, printf�������ջ�ͨ������fputc����ַ��������� */
/* TODO: �޸�Ϊ�ж�ʵ��, ���ڵ�ʵ�ַ�ʽ��ʵʱ�����º����� */
int fputc(int ch, FILE *f)
{
    /**
     * ������в���
     *      ��chд����������, �޸�end, ʹ��TXR
     * �������, �޶���, �൱�ڶ����������ַ�
     */
    if (buffer_available())
    {
        g_printf_buffer[g_printf_end] = ch;
        g_printf_end = (g_printf_end + 1) % CF_PRINTF_BUFFER_SIZE;
        __HAL_UART_ENABLE_IT(&CF_USART_PRINTF_HANDLE, UART_IT_TXE);
    }
    return ch;
}
#endif

/**
 * @brief       ����X��ʼ������
 * @param       baudrate: ������, �����Լ���Ҫ���ò�����ֵ
 * @param       priority: �����жϵ����ȼ�
 * @note        ע��: ����������ȷ��ʱ��Դ, ���򴮿ڲ����ʾͻ������쳣.
 *              �����USART��ʱ��Դ��sys_stm32_clock_init()�������Ѿ����ù���.
 * @retval      ��
 */
void printf_init(uint32_t baudrate, uint32_t priority)
{
    uart_enable(CF_USART_PRINTF, baudrate, 1, priority);
}
