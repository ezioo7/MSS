/**
 ****************************************************************************************************
 * @file        SystemSafety.c
 * @author      �����飬zhuowenwei
 * @version     V1.0
 * @date        2024-12-09
 * @brief       ϵͳ����
 * @licence     Copyright (c) 2020-2032, �����г����������޹�˾
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
    // #define STATE_STANDY        1       /*1-��ʾStandy״̬��*/
    // #define STATE_TRANSITION    2       /*2-��ʾTransition״̬*/
    // #define STATE_RUNNING       3       /*3-��ʾRunning״̬*/
    // #define STATE_SYSTEMLOCK    10      /*10-SystemLock״̬*/
    // #define STATE_TEST          11      /*11-TEST״̬*/


uint32_t SV_Fault1;                     /*һ���������ش���ϵͳ��������״̬*/
    // #define FAT1_NO             (1 << 0)       /*��ʾ�޴���*/
    // #define FAT1_CANOUT         (1 << 1)       /*CANͨ�ų�ʱ*/
    // #define FAT1_OVERCURRENT    (1 << 2)       /*�������������󣬴���2�������*/
    // #define FAT1_OUTLOSSPHU     (1 << 3)       /*���ȱ��U*/
    // #define FAT1_OUTLOSSPHV     (1 << 4)       /*���ȱ��V*/
    // #define FAT1_OUTLOSSPHW     (1 << 5)       /*���ȱ��W*/
    // #define FAT1_INTLOSSPHA     (1 << 6)       /*����ȱ��A*/
    // #define FAT1_INTLOSSPHB     (1 << 7)       /*����ȱ��B*/
    // #define FAT1_INTLOSSPHC     (1 << 8)       /*����ȱ��C*/
    
uint32_t SV_Fault2;                     /*������������ϵͳ���͹�������*/
uint32_t SV_Fault3;                     /*�������󣬾���*/


uint8_t  SV_Sequence_Detect_Flag = 1;   /*��־�Ƿ���Ҫ����������, 1 = ��Ҫ, 0 = ����Ҫ; TODO:��CheckPhaseSequence�޸�Ϊ 0*/
uint32_t SV_UPWM_Count;                 /*���������� U�������ŵ�ͨ������ÿ�����ڵĳ�ʼΪ0*/
uint32_t SV_VPWM_Count;                 /*���������� V�������ŵ�ͨ������ÿ�����ڵĳ�ʼΪ0*/
uint32_t SV_WPWM_Count;                 /*���������� W�������ŵ�ͨ������ÿ�����ڵĳ�ʼΪ0*/

/**
* @brief ���ϵͳUVW�������
* @param ��
*   @arg ��
* @note ���������ڣ����ർͨʱ��U>V>WΪ�޴��󣬷����������
* @retval 0-��ʾ���������1-��ʾ�����������
*/
uint16_t CheckPhaseSequence(void)
{
    /*�������߼�*/
    if ((SV_UPWM_Count >= SV_VPWM_Count) && (SV_VPWM_Count >= SV_WPWM_Count))
    {
        if (SV_MSSSatus == STATE_STANDY)                        /*��Standy״̬���������*/
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

uint16_t CF_OverPhI_Config;                  /*ϵͳ���ط�ʱ���趨*/
    // #define OVERPHI_L0      0                /*0-��ʾ�޷�ʱ���趨*/
    // #define OVERPHI_L1      1               /*1-��ʾ����1���޷�ʱ���趨��1.05���������ʱ�ޱ���*/
    // #define OVERPHI_L2      2               /*2-��ʾ����2���޷�ʱ���趨��1.2���������ʱ�ޱ���*/
    // #define OVERPHI_L3      3               /*3-��ʾ����3���޷�ʱ���趨��1.5���������ʱ�ޱ���*/
    // #define OVERPHI_L4      4               /*4-��ʾ����4���޷�ʱ���趨��1.8���������ʱ�ޱ���*/
    // #define OVERPHI_L5      5               /*5-��ʾ����5���޷�ʱ���趨��2���������ʱ�ޱ���*/
    // #define OVERPHI_L6      6               /*6-��ʾ����6���޷�ʱ���趨��3���������ʱ�ޱ���*/


/* TODO:�ڴ���ɼ�����������ʱ���й��ط�ʱ�޼��, ���������ݴ����Ƶ���й�, ��CF_HIGH_LEVEL_DATA_TRANS_PERIOD����, ��ǰΪ250us */
float SV_OVERPHI_L1Cnt;                     /*1.05�������������ʱ�䣬��λ������*/
float SV_OVERPHI_L2Cnt;                     /*1.2�������������ʱ�䣬��λ������*/
float SV_OVERPHI_L3Cnt;                     /*1.5�������������ʱ�䣬��λ������*/
float SV_OVERPHI_L4Cnt;                     /*1.8�������������ʱ�䣬��λ������*/
float SV_OVERPHI_L5Cnt;                     /*2�������������ʱ�䣬��λ������*/
float SV_OVERPHI_L6Cnt;                     /*3�������������ʱ�䣬��λ������*/

float CF_RatedPhI;                          /*����Ķ����,32λ��Ч,��λA,����0.01A*/

float   AM_OutU_PhaseI;                     /*���U�����ƽ��ֵ,32λ��Ч,��λA,����0.01A*/
float   AM_OutV_PhaseI;                     /*���V�����ƽ��ֵ,32λ��Ч,��λA,����0.01A*/
float   AM_OutW_PhaseI;                     /*���W�����ƽ��ֵ,32λ��Ч,��λA,����0.01A*/



/**
* @brief ���ϵͳUVW�������������ʱ
* @param ��
*   @arg ��
* @note ���������ڣ����ർͨʱ��U>V>WΪ�޴��󣬷����������
* @retval 0-��ʾ���������1-��ʾ�����������
*/
void OverLoadCurrent(void)
{   
    switch (CF_OverPhI_Config)
    {
        case OVERPHI_L6:
            /*6�����ط���ʱ�ж�,3������*/
            if ( !OverCurrent_Factor(SV_OVERPHI_L6Cnt, 23, 3) )
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L5:
            /*5�����ط���ʱ�ж�,2������*/
            if ( !OverCurrent_Factor(SV_OVERPHI_L5Cnt, 23, 2) )
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L4:
            /*4�����ط���ʱ�ж�,1.8������*/
            if (!OverCurrent_Factor(SV_OVERPHI_L4Cnt, 23, 1.8))
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L3:
            /*3�����ط���ʱ�ж�,1.5������*/
            if (!OverCurrent_Factor(SV_OVERPHI_L3Cnt, 23, 1.5) )
            {
                if (SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L2:
            /*2�����ط���ʱ�ж�,1.2������*/
            if (!OverCurrent_Factor(SV_OVERPHI_L2Cnt, 23, 1.2))
            {
                if ( SV_MSSSatus == STATE_STANDY && BIT_TEST(SV_Fault1, FAT1_OVERCURRENT))
                {
                    BIT_CLEAR(SV_Fault1, FAT1_OVERCURRENT);
                }
            }
        case OVERPHI_L1:
            /*1�����ط���ʱ�ж�*/
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
* @brief ���ϵͳUVW�����������
* @param ��
*   @arg OverPhITime-ʵ�����г�ʱʱ��
*   @arg LimitTime-������ʱ��
*   @arg OverFactor-������������1.05/1.2/1.5/1.8/2.0/3.0
* @note 1.���ؾ�������FA2����2.���س�ʱ������FA1����
* @retval 0-��ʾ���������1-��ʾ�����������
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
