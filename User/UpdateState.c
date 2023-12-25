
/**
 ****************************************************************************************************
 * @file        UpdateState.c
 * @author      电气组，zhuowenwei
 * @version     V1.0
 * @date        2024-12-06
 * @brief       系统状态变化
 * @licence     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
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

uint16_t SV_MSSSatus;       /* 1-表示默认状态*/
#define STATE_STANDY 1      /*1-表示Standy状态，*/
#define STATE_TRANSITION 2  /*2-表示Transition状态*/
#define STATE_RUNNING 3     /*3-表示Running状态*/
#define STATE_SYSTEMLOCK 10 /*10-SystemLock状态*/
#define STATE_TEST 11       /*11-TEST状态*/
uint16_t CI_Command;        /*用户外部输入命令*/
#define CMD_RUNING 1        /*运行指令*/
#define CMD_ESTOP 2         /*急停指令*/
#define CMD_PAUSED 3        /*暂停指令*/
#define CMD_STOP 4          /*停止指令*/
#define CMD_TESTING 5       /*测试指令*/
extern uint32_t SV_Fault1;  /*一级错误，严重错误，系统进入锁死状态*/
extern uint32_t SV_Fault2;  /*二级错误，引起系统降低功耗运行*/
extern uint32_t SV_Fault3;  /*三级错误，警告*/

float CI_DesireVoltage; /*用户输入期望相电压变量，用于输入*/
float SV_DesireVoltage; /*期望相电压变量，系统变量*/
float AM_OutUO_Voltage; /*输出U相电压平均值*/
float AM_OutVO_Voltage; /*输出V相电压平均值*/
float AM_OutWO_Voltage; /*输出W相电压平均值*/
float AM_OutUV_Voltage; /*输出UV相间电压平均值*/
float AM_OutUW_Voltage; /*输出UW相间电压平均值*/
float AM_OutWV_Voltage; /*输出WV相间电压平均值*/

float CI_AirPreMax;   /*空压机负载压力范围最大值*/
float CI_AirPreMin;   /*空压机负载压力范围最小值*/
float AM_AirPreValue; /*空压机负载压力值*/

void UpdateState_task(void *pvParameters)
{
    while (1)
    {
        UpdateState();
        vTaskDelay(10); /* 频率500HZ - 1KHZ */
    }
}

/**
 * @brief MSS系统各种不同状态变化
 * @param 无
 *   @arg 无
 * @note 定时任务，频率为1KHz
 * @retval 无
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
 * @brief 等待状态Standy调用的运行函数
 * @param 无
 *   @arg 无
 * @note 无
 * @retval 系统状态
 */
uint16_t Standy(void)
{
    /*1.处理用户输入指令*/
    switch (CI_Command)
    {
    case CMD_RUNING: /*用户输入运行指令*/
        return STATE_TRANSITION;

    case CMD_TESTING: /*用户输入测试指令*/
        return STATE_TEST;

    default:
        break;
    }

    return STATE_STANDY;
}

/**
 * @brief 过渡状态Transition调用的运行函数
 * @param 无
 *   @arg 无
 * @note 无
 * @retval 系统状态
 */
uint16_t Transition(void)
{
    if (SV_Fault1 == FAT1_NO) /*系统无一级错误*/
    {
        switch (CI_Command)
        {
        case CMD_RUNING: /*接收到运行指令*/
            /*1.空压机负载压力小于CI_AirPreMin*/
            if (AM_AirPreValue <= CI_AirPreMin)
            {
                /*空压机加载压力过低，启动发波接口*/

                return STATE_RUNNING;
            }
            else if (AM_AirPreValue > CI_AirPreMin && AM_AirPreValue <= CI_AirPreMax)
            {
                /*空压机加载压力正常，无需操作*/
                return STATE_TRANSITION;
            }
            else
            {
                /*空压机加载压力过大，启动空压机泄压接口*/
                return STATE_TRANSITION;
            }

        case CMD_STOP:
            /*1.停止发波接口*/

            return STATE_STANDY;
        default:
            return STATE_TRANSITION;
        }
    }
    else
    {
        /*1.停止发波接口*/

        return STATE_SYSTEMLOCK;
    }
}

/**
 * @brief 运行状态Running调用的运行函数
 * @param 无
 *   @arg 无
 * @note 无
 * @retval 系统状态
 */
uint16_t Running(void)
{
    if (SV_Fault1 == FAT1_NO) /*无一级系统错误*/
    {
        switch (CI_Command)
        {
        case CMD_STOP:
            /*进行系统停车操作*/
            CI_DesireVoltage = 0;
            SV_DesireVoltage = 0;

            if (abs(AM_OutUO_Voltage) < 10)
            {
                return STATE_TRANSITION;
            }
            break;
        case CMD_RUNING:
            /*1.空压机压力处于正常范围，进入TRANSITION状态，需停止发波*/
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
        /*停止发波接口*/

        return STATE_SYSTEMLOCK;
    }
}

/**
 * @brief 系统锁定状态SystemLock调用的运行函数
 * @param 无
 *   @arg 无
 * @note 无
 * @retval 系统状态
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
 * @brief 系统测试状态Testing调用的运行函数
 * @param 无
 *   @arg 无
 * @note 无
 * @retval 系统状态
 */
uint16_t Testing(void)
{
    if (SV_Fault1 != FAT1_NO)
    {
        return STATE_SYSTEMLOCK;
    }

    return STATE_TEST;
}
