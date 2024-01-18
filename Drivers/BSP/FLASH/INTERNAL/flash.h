/**
 ****************************************************************************************************
 * @file        flash.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-29
 * @brief       Flash����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_FLASH_H
#define __MSS_FLASH_H

#include "./SYSTEM/sys/sys.h"

#define MSS_FLASH_BASE 0x08000000U                          /* ���洢����ʼ��ַ */
#define MSS_FLASH_END 0x0807FFFFU                           /* ���洢�������ַ */
#define MSS_FLASH_PAGE_NUM 0x00000100U                      /* ��256ҳ, ҳ�� 0 ~ 255 */
#define MSS_FLASH_PAGE_SIZE_1B 0x00000800U                  /* ��BΪ��λ, ҳ��С2K */
#define MSS_FLASH_PAGE_SIZE_2B 0x00000400U                  /* ��2BΪ��λ, ҳ��С1K */
#define MSS_FLASH_PAGE_SIZE_4B 0x00000200U                  /* ��4BΪ��λ, ҳ��С512 */
#define MSS_FLASH_PAGE_SIZE_8B 0x00000100U                  /* ��8BΪ��λ, ҳ��С256 */
#define MSS_FLASH_PERSIST_BASE 0x0807F800U                  /* �����־û���ʼ��ַ */
#define MSS_FLASH_PERSIST_END MSS_FLASH_END                 /* �����־û�������ַ */
#define MSS_FLASH_PERSIST_PAGE0_BASE MSS_FLASH_PERSIST_BASE /* �����־û���0ҳ��ʼ��ַ, 0x0807F800U */

void flash_read_1B(uint32_t addr, uint8_t *read_Data, uint16_t size);
void flash_read_2B(uint32_t addr, uint16_t *read_Data, uint16_t size);
void flash_read_4B(uint32_t addr, uint32_t *read_Data, uint16_t size);
void flash_read_page(uint8_t page_Index, uint8_t *read_Data);
void flash_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t size);
void flash_write_page(uint8_t page_Index, uint8_t *write_Data, uint32_t size);

#endif
