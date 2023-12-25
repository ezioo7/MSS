
/**
 ****************************************************************************************************
 * @file        UpdateState.c
 * @author      �����飬zhuowenwei
 * @version     V1.0
 * @date        2024-12-06
 * @brief       ϵͳ״̬�仯
 * @licence     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 *
 *
 ****************************************************************************************************
 */

#include "UpdateState.h"
#include "SystemSafety.h"
#include "FreeRTOS.h"
#include "task.h"

uint16_t SV_MSSSatus;       /* 1-��ʾĬ��״̬*/
#define STATE_STANDY 1      /*1-��ʾStandy״̬��*/
#define STATE_TRANSITION 2  /*2-��ʾTransition״̬*/
#define STATE_RUNNING 3     /*3-��ʾRunning״̬*/
#define STATE_SYSTEMLOCK 10 /*10-SystemLock״̬*/
#define STATE_TEST 11       /*11-TEST״̬*/
uint16_t CI_Command;        /*�û��ⲿ��������*/
#define CMD_RUNING 1        /*����ָ��*/
#define CMD_ESTOP 2         /*��ָͣ��*/
#define CMD_PAUSED 3        /*��ָͣ��*/
#define CMD_STOP 4          /*ָֹͣ��*/
#define CMD_TESTING 5       /*����ָ��*/
extern uint32_t SV_Fault1;  /*һ���������ش���ϵͳ��������״̬*/
extern uint32_t SV_Fault2;  /*������������ϵͳ���͹�������*/
extern uint32_t SV_Fault3;  /*�������󣬾���*/

float CI_DesireVoltage; /*�û������������ѹ��������������*/
float SV_DesireVoltage; /*�������ѹ������ϵͳ����*/
float AM_OutUO_Voltage; /*���U���ѹƽ��ֵ*/
float AM_OutVO_Voltage; /*���V���ѹƽ��ֵ*/
float AM_OutWO_Voltage; /*���W���ѹƽ��ֵ*/
float AM_OutUV_Voltage; /*���UV����ѹƽ��ֵ*/
float AM_OutUW_Voltage; /*���UW����ѹƽ��ֵ*/
float AM_OutWV_Voltage; /*���WV����ѹƽ��ֵ*/

float CI_AirPreMax;   /*��ѹ������ѹ����Χ���ֵ*/
float CI_AirPreMin;   /*��ѹ������ѹ����Χ��Сֵ*/
float AM_AirPreValue; /*��ѹ������ѹ��ֵ*/

void UpdateState_task(void *pvParameters)
{
    while (1)
    {
        UpdateState();
        vTaskDelay(10); /* Ƶ��500HZ - 1KHZ */
    }
}

/**
 * @brief MSSϵͳ���ֲ�ͬ״̬�仯
 * @param ��
 *   @arg ��
 * @note ��ʱ����Ƶ��Ϊ1KHz
 * @retval ��
 */
void UpdateState(void)
{
    uint16_t tmpstate = SV_MSSSatus;
    switch (tmpstate)
    {
    case STATE_STANDY:
        SV_MSSSatus = Standy();
        break;

    case STATE_TRANSITION:
        SV_MSSSatus = Transition();
        break;

    case STATE_RUNNING:
        SV_MSSSatus = Running();
        break;

    case STATE_SYSTEMLOCK:
        SV_MSSSatus = SystemLock();
        break;
    case STATE_TEST:
        SV_MSSSatus = Testing();

    default:
        SV_MSSSatus = STATE_STANDY;
        break;
    }
}

/**
 * @brief �ȴ�״̬Standy���õ����к���
 * @param ��
 *   @arg ��
 * @note ��
 * @retval ϵͳ״̬
 */
uint16_t Standy(void)
{
    /*1.�����û�����ָ��*/
    switch (CI_Command)
    {
    case CMD_RUNING: /*�û���������ָ��*/
        return STATE_TRANSITION;

    case CMD_TESTING: /*�û��������ָ��*/
        return STATE_TEST;

    default:
        break;
    }

    return STATE_STANDY;
}

/**
 * @brief ����״̬Transition���õ����к���
 * @param ��
 *   @arg ��
 * @note ��
 * @retval ϵͳ״̬
 */
uint16_t Transition(void)
{
    if (SV_Fault1 == FAT1_NO) /*ϵͳ��һ������*/
    {
        switch (CI_Command)
        {
        case CMD_RUNING: /*���յ�����ָ��*/
            /*1.��ѹ������ѹ��С��CI_AirPreMin*/
            if (AM_AirPreValue <= CI_AirPreMin)
            {
                /*��ѹ������ѹ�����ͣ����������ӿ�*/

                return STATE_RUNNING;
            }
            else if (AM_AirPreValue > CI_AirPreMin && AM_AirPreValue <= CI_AirPreMax)
            {
                /*��ѹ������ѹ���������������*/
                return STATE_TRANSITION;
            }
            else
            {
                /*��ѹ������ѹ������������ѹ��йѹ�ӿ�*/
                return STATE_TRANSITION;
            }

        case CMD_STOP:
            /*1.ֹͣ�����ӿ�*/

            return STATE_STANDY;
        default:
            return STATE_TRANSITION;
        }
    }
    else
    {
        /*1.ֹͣ�����ӿ�*/

        return STATE_SYSTEMLOCK;
    }
}

/**
 * @brief ����״̬Running���õ����к���
 * @param ��
 *   @arg ��
 * @note ��
 * @retval ϵͳ״̬
 */
uint16_t Running(void)
{
    if (SV_Fault1 == FAT1_NO) /*��һ��ϵͳ����*/
    {
        switch (CI_Command)
        {
        case CMD_STOP:
            /*����ϵͳͣ������*/
            CI_DesireVoltage = 0;
            SV_DesireVoltage = 0;

            if (abs(AM_OutUO_Voltage) < 10)
            {
                return STATE_TRANSITION;
            }
            break;
        case CMD_RUNING:
            /*1.��ѹ��ѹ������������Χ������TRANSITION״̬����ֹͣ����*/
            if (AM_AirPreValue > CI_AirPreMin)
            {
                return STATE_TRANSITION;
            }

        default:
            return STATE_RUNNING;
        }
    }
    else
    {
        /*ֹͣ�����ӿ�*/

        return STATE_SYSTEMLOCK;
    }
}

/**
 * @brief ϵͳ����״̬SystemLock���õ����к���
 * @param ��
 *   @arg ��
 * @note ��
 * @retval ϵͳ״̬
 */
uint16_t SystemLock(void)
{
    if (CI_Command == CMD_STOP)
    {
        return STATE_STANDY;
    }

    return STATE_SYSTEMLOCK;
}

/**
 * @brief ϵͳ����״̬Testing���õ����к���
 * @param ��
 *   @arg ��
 * @note ��
 * @retval ϵͳ״̬
 */
uint16_t Testing(void)
{
    if (SV_Fault1 != FAT1_NO)
    {
        return STATE_SYSTEMLOCK;
    }

    return STATE_TEST;
}
