/**
 ****************************************************************************************************
 * @file        flash.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-29
 * @brief       Flash驱动
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_FLASH_H
#define __MSS_FLASH_H

#include "./SYSTEM/sys/sys.h"

#define MSS_FLASH_BASE 0x08000000U                          /* 主存储块起始地址 */
#define MSS_FLASH_END 0x0807FFFFU                           /* 主存储块结束地址 */
#define MSS_FLASH_PAGE_NUM 0x00000100U                      /* 共256页, 页号 0 ~ 255 */
#define MSS_FLASH_PAGE_SIZE_1B 0x00000800U                  /* 按B为单位, 页大小2K */
#define MSS_FLASH_PAGE_SIZE_2B 0x00000400U                  /* 按2B为单位, 页大小1K */
#define MSS_FLASH_PAGE_SIZE_4B 0x00000200U                  /* 按4B为单位, 页大小512 */
#define MSS_FLASH_PAGE_SIZE_8B 0x00000100U                  /* 按8B为单位, 页大小256 */
#define MSS_FLASH_PERSIST_BASE 0x0807F800U                  /* 参数持久化起始地址 */
#define MSS_FLASH_PERSIST_END MSS_FLASH_END                 /* 参数持久化结束地址 */
#define MSS_FLASH_PERSIST_PAGE0_BASE MSS_FLASH_PERSIST_BASE /* 参数持久化第0页起始地址, 0x0807F800U */

void flash_read_1B(uint32_t addr, uint8_t *read_Data, uint16_t size);
void flash_read_2B(uint32_t addr, uint16_t *read_Data, uint16_t size);
void flash_read_4B(uint32_t addr, uint32_t *read_Data, uint16_t size);
void flash_read_page(uint8_t page_Index, uint8_t *read_Data);
void flash_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t size);
void flash_write_page(uint8_t page_Index, uint8_t *write_Data, uint32_t size);

#endif
