/**
 ****************************************************************************************************
 * @file        delay.c
 * @author      dx
 * @version     V1.1
 * @date        2023-02-25
 * @brief       ʹ��SysTick����ͨ����ģʽ���ӳٽ��й���(֧��ucosii)
 *              �ṩdelay_init��ʼ�������� delay_us��delay_ms����ʱ����
 ****************************************************************************************************
 * @attention
 *
 * �޸�˵��
 * V1.0 20230206
 * ��һ�η���
 * V1.1 20230225
 * �޸�SYS_SUPPORT_OS���ִ���, Ĭ�Ͻ�֧��UCOSII 2.93.01�汾, ����OS��ο�ʵ��
 * �޸�delay_init����ʹ��8��Ƶ,ȫ��ͳһʹ��MCUʱ��
 * �޸�delay_usʹ��ʱ��ժȡ����ʱ, ����OS
 * �޸�delay_msֱ��ʹ��delay_us��ʱʵ��.
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"


static uint32_t g_fac_us = 0;       /* us��ʱ������ */

/* ���SYS_SUPPORT_OS������,˵��Ҫ֧��OS��(������UCOS) */
#if SYS_SUPPORT_OS

/* ��ӹ���ͷ�ļ� (FreeRTOS ��Ҫ�õ�) */
#include "FreeRTOS.h"
#include "task.h"

extern void xPortSysTickHandler(void);

/**
 * @brief     systick�жϷ�����,ʹ��OSʱ�õ�
 * @param     ticks : ��ʱ�Ľ�����  
 * @retval    ��
 */  
void SysTick_Handler(void)
{
    /* OS ��ʼ����,��ִ�������ĵ��ȴ��� */
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
    HAL_IncTick();
}
#endif

/**
 * @brief     ��ʼ���ӳٺ���
 * @param     sysclk: ϵͳʱ��Ƶ��, ��CPUƵ��(rcc_c_ck), 168MHz
 * @retval    ��
 */  
void delay_init(uint16_t sysclk)
{
#if SYS_SUPPORT_OS                                      /* �����Ҫ֧��OS */
    uint32_t reload;
#endif
    g_fac_us = sysclk;                                  /* ������HAL_Init���Ѷ�systick�������ã��������������������� */
#if SYS_SUPPORT_OS                                      /* �����Ҫ֧��OS. */
    reload = sysclk;                                    /* ÿ���ӵļ������� ��λΪM */
    reload *= 1000000 / configTICK_RATE_HZ;             /* ����configTICK_RATE_HZ�趨���ʱ��,reloadΪ24λ
                                                         * �Ĵ���,���ֵ:16777216,��168M��,Լ��0.09986s����
                                                         */
    SysTick->CTRL |= 1 << 1;                            /* ����SYSTICK�ж� */
    SysTick->LOAD = reload;                             /* ÿ1/delay_ostickspersec���ж�һ�� */
    SysTick->CTRL |= 1 << 0;                            /* ����SYSTICK */
#endif 
}

/**
 * @brief     ��ʱnus
 * @note      �����Ƿ�ʹ��OS, ������ʱ��ժȡ������us��ʱ
 * @param     nus: Ҫ��ʱ��us��
 * @note      nusȡֵ��Χ: 0 ~ (2^32 / fac_us) (fac_usһ�����ϵͳ��Ƶ, �����������)
 * @retval    ��
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        /* LOAD��ֵ */
    ticks = nus * g_fac_us;                 /* ��Ҫ�Ľ����� */

    told = SysTick->VAL;                    /* �ս���ʱ�ļ�����ֵ */
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;        /* ����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ����� */
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) 
            {
                break;                      /* ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳� */
            }
        }
    }

}

/**
 * @brief     ��ʱnms
 * @param     nms: Ҫ��ʱ��ms�� (0< nms <= (2^32 / fac_us / 1000))(fac_usһ�����ϵͳ��Ƶ, �����������)
 * @retval    ��
 */
void delay_ms(uint16_t nms)
{
    uint16_t index;
    
    for (index=0; index<nms; index++)
    {
        delay_us(1000);
    }
}










