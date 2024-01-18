/**
 ****************************************************************************************************
 * @file        SystemSafety.c
 * @author      电气组，zhuowenwei
 * @version     V1.0
 * @date        2024-12-09
 * @brief       系统错误
 * @licence     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 
 * 
 *
 *
 ****************************************************************************************************
 */

#include "SystemSafety.h"

extern uint16_t SV_MSSSatus;
    // #define STATE_STANDY        1       /*1-表示Standy状态，*/
    // #define STATE_TRANSITION    2       /*2-表示Transition状态*/
    // #define STATE_RUNNING       3       /*3-表示Running状态*/
    // #define STATE_SYSTEMLOCK    10      /*10-SystemLock状态*/
    // #define STATE_TEST          11      /*11-TEST状态*/


uint32_t SV_Fault1;                     /*一级错误，严重错误，系统进入锁死状态*/
    // #define FAT1_NO             (1 << 0)       /*表示无错误*/
    // #define FAT1_CANOUT         (1 << 1)       /*CAN通信超时*/
    // #define FAT1_OVERCURRENT    (1 << 2)       /*输出三相电流过大，大于2倍额定电流*/
    // #define FAT1_OUTLOSSPHU     (1 << 3)       /*输出缺相U*/
    // #define FAT1_OUTLOSSPHV     (1 << 4)       /*输出缺相V*/
    // #define FAT1_OUTLOSSPHW     (1 << 5)       /*输出缺相W*/
    // #define FAT1_INTLOSSPHA     (1 << 6)       /*输入缺相A*/
    // #define FAT1_INTLOSSPHB     (1 << 7)       /*输入缺相B*/
    // #define FAT1_INTLOSSPHC     (1 << 8)       /*输入缺相C*/
    
uint32_t SV_Fault2;                     /*二级错误，引起系统降低功耗运行*/
uint32_t SV_Fault3;                     /*三级错误，警告*/


uint8_t  SV_Sequence_Detect_Flag = 1;   /*标志是否需要进行相序检测, 1 = 需要, 0 = 不需要; TODO:在CheckPhaseSequence修改为 0*/
uint32_t SV_UPWM_Count;                 /*单个周期内 U相上下桥导通计数，每个周期的初始为0*/
uint32_t SV_VPWM_Count;                 /*单个周期内 V相上下桥导通计数，每个周期的初始为0*/
uint32_t SV_WPWM_Count;                 /*单个周期内 W相上下桥导通计数，每个周期的初始为0*/

/**
* @brief 检查系统UVW相序错误
* @param 无
*   @arg 无
* @note 单个周期内，三相导通时间U>V>W为无错误，否则相序错误。
* @retval 0-表示无相序错误；1-表示存在相序错误。
*/
uint16_t CheckPhaseSequence(void)
{
    /*相序监测逻辑*/
    if ((SV_UPWM_Count >= SV_VPWM_Count) && (SV_VPWM_Count >= SV_WPWM_Count))
    {
        if (SV_MSSSatus == STATE_STANDY)                        /*仅Standy状态能清除错误*/
        {
            BIT_CLEAR(SV_Fault1, FAT1_PHASESEQUENCE);
        }
        return SUCCESS_OK;
    }else
    {    
        BIT_SET(SV_Fault1, FAT1_PHASESEQUENCE);
        return FAIL_ERR;
    }
}

uint16_t CF_OverPhI_Config;                  /*系统过载反时限设定*/
    // #define OVERPHI_L0      0                /*0-表示无反时限设定*/
    // #define OVERPHI_L1      1               /*1-表示启用1级无反时限设定，1.05倍额定电流反时限保护*/
    // #define OVERPHI_L2      2               /*2-表示启用2级无反时限设定，1.2倍额定电流反时限保护*/
    // #define OVERPHI_L3      3               /*3-表示启用3级无反时限设定，1.5倍额定电流反时限保护*/
    // #define OVERPHI_L4      4               /*4-表示启用4级无反时限设定，1.8倍额定电流反时限保护*/
    // #define OVERPHI_L5      5               /*5-表示启用5级无反时限设定，2倍额定电流反时限保护*/
    // #define OVERPHI_L6      6               /*6-表示启用6级无反时限设定，3倍额定电流反时限保护*/


/* TODO:在处理采集到电流数据时进行过载反时限检查, 所以与数据处理的频率有关, 由CF_HIGH_LEVEL_DATA_TRANS_PERIOD决定, 当前为250us */
float SV_OVERPHI_L1Cnt;                     /*1.05倍额定电流，运行时间，单位，精度*/
float SV_OVERPHI_L2Cnt;                     /*1.2倍额定电流，运行时间，单位，精度*/
float SV_OVERPHI_L3Cnt;                     /*1.5倍额定电流，运行时间，单位，精度*/
float SV_OVERPHI_L4Cnt;                     /*1.8倍额定电流，运行时间，单位，精度*/
float SV_OVERPHI_L5Cnt;                     /*2倍额定电流，运行时间，单位，精度*/
float SV_OVERPHI_L6Cnt;                     /*3倍额定电流，运行时间，单位，精度*/

float CF_RatedPhI;                          /*电机的额定电流,32位有效,单位A,精度0.01A*/

float   AM_OutU_PhaseI;                     /*输出U相电流平均值,32位有效,单位A,精度0.01A*/
float   AM_OutV_PhaseI;                     /*输出V相电流平均值,32位有效,单位A,精度0.01A*/
float   AM_OutW_PhaseI;                     /*输出W相电流平均值,32位有效,单位A,精度0.01A*/



/**
* @brief 检查系统UVW三相输出过载限时
* @param 无
*   @arg 无
* @note 单个周期内，三相导通时间U>V>W为无错误，否则相序错误。
* @retval 0-表示无相序错误；1-表示存在相序错误。
*/
void OverLoadCurrent(void)
{   
    switch (CF_OverPhI_Config)
    {
        case OVERPHI_L6:
            /*6级过载反限时判定,3倍过载*/
            if ( !OverCurrent_Factor(SV_OVERPHI_L6Cnt, 23, 3) )
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L5:
            /*5级过载反限时判定,2倍过载*/
            if ( !OverCurrent_Factor(SV_OVERPHI_L5Cnt, 23, 2) )
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L4:
            /*4级过载反限时判定,1.8倍过载*/
            if (!OverCurrent_Factor(SV_OVERPHI_L4Cnt, 23, 1.8))
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L3:
            /*3级过载反限时判定,1.5倍过载*/
            if (!OverCurrent_Factor(SV_OVERPHI_L3Cnt, 23, 1.5) )
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L2:
            /*2级过载反限时判定,1.2倍过载*/
            if (!OverCurrent_Factor(SV_OVERPHI_L2Cnt, 23, 1.2))
            {
                if ( SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L1:
            /*1级过载反限时判定*/
            if (!OverCurrent_Factor(SV_OVERPHI_L1Cnt, 23, 1.05) )
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L0:
        
        default:
            break;
    }
}

/**
* @brief 检查系统UVW三相输出过载
* @param 无
*   @arg OverPhITime-实际运行超时时间
*   @arg LimitTime-过流限时，
*   @arg OverFactor-过电流倍数，1.05/1.2/1.5/1.8/2.0/3.0
* @note 1.过载警告设置FA2错误，2.过载超时限设置FA1错误。
* @retval 0-表示无相序错误；1-表示存在相序错误。
*/
uint16_t OverCurrent_Factor(float OverPhITime, float LimitTime, float OverFactor)
{
    if (AM_OutU_PhaseI > (OverFactor * CF_RatedPhI))
    {
        BIT_SET(SV_Fault2, FAT2_OVERPHI);
    }else
    {
        BIT_CLEAR(SV_Fault2, FAT2_OVERPHI);
    }
    if (OverPhITime >= LimitTime)
    {   
        BIT_SET(SV_Fault1, FAT1_OVERCURRENT);
        return FAIL_ERR;
    }
    return SUCCESS_OK;
}
