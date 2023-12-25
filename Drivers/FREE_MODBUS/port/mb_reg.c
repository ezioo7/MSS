/**
 ****************************************************************************************************
 * @file        mb_reg.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-11
 * @brief       FreeModbusͨѶ�����ļĴ�������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 *
 ****************************************************************************************************
 */
#include "mb.h"
#include "mbport.h"
#include "globalE.h"


/* ʮ·����Ĵ��� */
#define REG_INPUT_SIZE 10
uint16_t REG_INPUT_BUF[REG_INPUT_SIZE];

// ʮ·���ּĴ���
#define REG_HOLD_SIZE 10
uint16_t REG_HOLD_BUF[REG_HOLD_SIZE];

// ʮ·��Ȧ
#define REG_COILS_SIZE 10
uint8_t REG_COILS_BUF[REG_COILS_SIZE] = {1, 1, 1, 1, 0, 0, 0, 0, 1, 1};

// ʮ·��ɢ��
#define REG_DISC_SIZE 10
uint8_t REG_DISC_BUF[REG_DISC_SIZE] = {1, 1, 1, 1, 0, 0, 0, 0, 1, 1};

/// CMD4�����ص�����
eMBErrorCode eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
    USHORT usRegIndex = usAddress - 1;

    // �Ƿ����
    if ((usRegIndex + usNRegs) > REG_INPUT_SIZE)
    {
        return MB_ENOREG;
    }

    // ѭ����ȡ
    while (usNRegs > 0)
    {
        *pucRegBuffer++ = (unsigned char)(REG_INPUT_BUF[usRegIndex] >> 8);
        *pucRegBuffer++ = (unsigned char)(REG_INPUT_BUF[usRegIndex] & 0xFF);
        usRegIndex++;
        usNRegs--;
    }

    // ģ������Ĵ������ı�
    for (usRegIndex = 0; usRegIndex < REG_INPUT_SIZE; usRegIndex++)
    {
        REG_INPUT_BUF[usRegIndex]++;
    }

    return MB_ENOERR;
}

/**
 * @brief CMD6��3��16�����ص�����,�����ɿ���Զ����
 * @param pucRegBuffer: ��������
 * @param usAddress: �Ĵ�����ַ
 * @param usNRegs: Ҫ�����ļĴ����ĸ���
 * @param eMode: ����ģʽ
 */
eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    USHORT usRegIndex = usAddress - 1;

    // �Ƿ����
    if ((usRegIndex + usNRegs) > REG_HOLD_SIZE)
    {
        return MB_ENOREG;
    }

    // д�Ĵ���
    if (eMode == MB_REG_WRITE)
    {
        while (usNRegs > 0)
        {
            REG_HOLD_BUF[usRegIndex] = (pucRegBuffer[0] << 8) | pucRegBuffer[1];
            pucRegBuffer += 2;
            usRegIndex++;
            usNRegs--;
        }
    }

    // ���Ĵ���
    else
    {
        while (usNRegs > 0)
        {
            *pucRegBuffer++ = (unsigned char)(REG_HOLD_BUF[usRegIndex] >> 8);
            *pucRegBuffer++ = (unsigned char)(REG_HOLD_BUF[usRegIndex] & 0xFF);
            usRegIndex++;
            usNRegs--;
        }
    }

    return MB_ENOERR;
}

/// CMD1��5��15�����ص�����
eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{
    USHORT usRegIndex = usAddress - 1;
    UCHAR ucBits = 0;
    UCHAR ucState = 0;
    UCHAR ucLoops = 0;

    // �Ƿ����
    if ((usRegIndex + usNCoils) > REG_COILS_SIZE)
    {
        return MB_ENOREG;
    }

    if (eMode == MB_REG_WRITE)
    {
        ucLoops = (usNCoils - 1) / 8 + 1;
        while (ucLoops != 0)
        {
            ucState = *pucRegBuffer++;
            ucBits = 0;
            while (usNCoils != 0 && ucBits < 8)
            {
                REG_COILS_BUF[usRegIndex++] = (ucState >> ucBits) & 0X01;
                usNCoils--;
                ucBits++;
            }
            ucLoops--;
        }
    }
    else
    {
        ucLoops = (usNCoils - 1) / 8 + 1;
        while (ucLoops != 0)
        {
            ucState = 0;
            ucBits = 0;
            while (usNCoils != 0 && ucBits < 8)
            {
                if (REG_COILS_BUF[usRegIndex])
                {
                    ucState |= (1 << ucBits);
                }
                usNCoils--;
                usRegIndex++;
                ucBits++;
            }
            *pucRegBuffer++ = ucState;
            ucLoops--;
        }
    }

    return MB_ENOERR;
}

/// CMD2�����ص�����
eMBErrorCode eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
    USHORT usRegIndex = usAddress - 1;
    UCHAR ucBits = 0;
    UCHAR ucState = 0;
    UCHAR ucLoops = 0;

    // �Ƿ����
    if ((usRegIndex + usNDiscrete) > REG_DISC_SIZE)
    {
        return MB_ENOREG;
    }

    ucLoops = (usNDiscrete - 1) / 8 + 1;
    while (ucLoops != 0)
    {
        ucState = 0;
        ucBits = 0;
        while (usNDiscrete != 0 && ucBits < 8)
        {
            if (REG_DISC_BUF[usRegIndex])
            {
                ucState |= (1 << ucBits);
            }
            usNDiscrete--;
            usRegIndex++;
            ucBits++;
        }
        *pucRegBuffer++ = ucState;
        ucLoops--;
    }

    // ģ����ɢ�����뱻�ı�
    for (usRegIndex = 0; usRegIndex < REG_DISC_SIZE; usRegIndex++)
    {
        REG_DISC_BUF[usRegIndex] = !REG_DISC_BUF[usRegIndex];
    }

    return MB_ENOERR;
}

/**
 * @brief FreeModbus��ѯ��������
 */
void mb_task(void *pvParameters)
{
    eMBInit(MB_RTU, 0x01, 1, 9600, MB_PAR_NONE); /* RTU��ʽ,�ӻ���ַ1,ʹ�ô���1,������9600,��У�� */
    eMBEnable();
    while (1)
    {
        eMBPoll();
    }
}
