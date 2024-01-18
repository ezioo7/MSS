/**
 ****************************************************************************************************
 * @file        config.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-01-08
 * @brief       ��������ģ��
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * 1. �־û������ɱ���������FLASH��, ͨ��SPIͨ�Ŷ�д; W25Q64����2K������, һ��������СΪ4KB, �ɷ� 1K ��uint32_t����, �϶�����;
 * 2. Լ��ѡ�����һ��������Ϊ��������λ��, ��������������־��¼;
 * 3. ��Ҫ�궨���: ������ʼ��ַ
 * 4. �ṩ������ȡ�ӿ�, �ýӿڽ���һ���������еĲ��������ڴ�һ���ṹ����
 * 5. �ṩ��������ӿ�, �ýӿڼ��ϵͳ��ǰ״̬, �����ǰ״̬����洢, �򽫲����ṹ���Ա��˳��д��FLASH
 ****************************************************************************************************
 * @attention
 * 1. �־û������ɱ������ڲ�FLASH��, ֱ�Ӷ�д; �ڲ�Flashһҳ 2KB, �ɷ� 512 �� uint32_t, ����;
 * 2. Լ��ѡ�����һҳ��Ϊ��������λ��;
 * 3. ��Ҫ�궨���: ������ʼ��ַ
 * 4. �ṩ������ȡ�ӿ�, �ýӿڽ���һ���������еĲ��������ڴ�һ���ṹ����
 * 5. �ṩ��������ӿ�, �ýӿڼ��ϵͳ��ǰ״̬, �����ǰ״̬����洢, �򽫲����ṹ���Ա��˳��д��FLASH
 ****************************************************************************************************
 */
#include "config.h"
#include "globalE.h"
#include "./BSP/FLASH/W25Q64/w25q64.h"
#include "./BSP/FLASH/INTERNAL/flash.h"

MSS_Config g_Mss_Config = {0};
MSS_Config *mss_Config = &g_Mss_Config;

/**
 * @brief
 */
void load_config_flash(void)
{
    flash_read_1B(MSS_FLASH_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}

/**
 * @brief
 */
void store_config_flash(void)
{
    flash_write_addr(MSS_FLASH_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}

/**
 * @brief   ��W25Q64�������õ��ڴ�
 * @note    �ڵ��ô˺���֮ǰ��Ҫ��ʼ��W25Q64��Ӧ��SPI
 */
void load_config_w25q64(void)
{
    w25q64_read(W25Q64_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}

/**
 * @brief   �־û����õ� W25Q64
 * @note    W25Q64һ��дָ�����д256B, ��������������ڴ�ֵ, ��Ҫ��ҳд��;
 */
void store_config_w25q64(void)
{
    w25q64_write_addr_withinSector(W25Q64_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}
