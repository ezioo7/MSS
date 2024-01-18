/**
 ****************************************************************************************************
 * @file        w25q64.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2024-01-06
 * @brief       �ⲿFlash����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_W25Q64_H
#define __MSS_W25Q64_H

#include "./SYSTEM/sys/sys.h"
#include "./globalE.h"

#define W25Q64_BASE 0x00000000               /* ����оƬ����ʼ��ַ */
#define W25Q64_END 0x007FFFFF                /* ����оƬ��ĩβ��ַ, �ܴ�С0x80-0000, 8M����ַ */
#define W25Q64_SECTOR_SIZE 0x1000            /* ��λΪB, ������С4KB */
#define W25Q64_SECTOR_NUM 0x0800             /* �� 2K ������ */
#define W25Q64_PAGE_SIZE 0x0100              /* ��λΪB, ҳ��С256B */
#define W25Q64_PAGE_NUM 0x0010               /* һ���������� 16��ҳ�� */
#define W25Q64_DEFAULT_DATA 0xFF             /* Ĭ�ϵ��������ֽ�, ������ʱ, �����ô����ݽ����ӻ����� */
#define W25Q64_TIMOUT 1000                   /* ��λms, ��ʱʱ�� */
#define W25Q64_PERSIST_BASE 0x007FF000       /* ���ڲ����־û����������׵�ַ */
#define W25Q64_PERSIST_PAGE0_BASE 0x007FF000 /* ���ڲ����־û��ĵ�һ��ҳ���׵�ַ */

#define W25Q64_ENABLE() HAL_GPIO_WritePin(CF_W25Q64_CS_GPIO, CF_W25Q64_CS_PIN, GPIO_PIN_RESET) /* ʹ��W25Q64Ƭѡ�ź� */
#define W25Q64_DISABLE() HAL_GPIO_WritePin(CF_W25Q64_CS_GPIO, CF_W25Q64_CS_PIN, GPIO_PIN_SET)  /* ʧ��W25Q64Ƭѡ�ź� */

void w25q64_init(void);
void w25q64_read_ID(uint8_t *read_Data);
void w25q64_read(uint32_t addr, uint8_t *read_Data, uint32_t len);
void w25q64_read_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *read_Data);
void w25q64_write_addr_withinSector(uint32_t addr, uint8_t *write_Data, uint16_t len);

/* TODO: ����, ��ʱû��Ҫʵ��; */
//void w25q64_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t len);
/* TODO: ����, ��ʱû��Ҫʵ��; */
//void w25q64_write_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *write_Data, uint32_t len);

#endif
