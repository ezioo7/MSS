/**
 ****************************************************************************************************
 * @file        flash.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-29
 * @brief       Flash����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * Flash�洢���ڲ����ⲿFLASH;
 * �ڲ�Flashֱ�Ӷ�д, �ⲿFlash��SPIͨ�Ž��ж�д, ������API;
 * �������ļ������ڲ�Flash����
 ****************************************************************************************************
 * @attention
 * �ڲ�Flash:
 * �ܴ�С512KB, ��ҳ���, һҳ2KB, ��256ҳ, д��Codeʱ�ӵ͵�ַ��ʼд, ����С��ģʽ;
 * ���Ҫ���Ķ����ַ������, ��������ҳ(��ҳ)����, Ȼ�����write��������д��������;
 * Flashдһ����ַ��Ҫ����, ֻҪ�õ�ַ�ϴν��й���������, ����Ҫ��ÿ��д����֮ǰ������һ����������;
 * д�����С��λ��2B, ��������С��λ��ҳ, һҳ2KB, ����д��������2BΪ������λ;
 * ��Ƚ�w25q64,ʡȥ��Ƭѡ��дʹ�ܵĲ���, ����Ҫ������� volatile �ؼ��ֲ��ҽ��н���;
 * ��Ƚ�w25q64,ͬ��Ҫ��д�����λ����ҳβ֮ǰ, ��������Ƿ�ؾ��ݲ�֪;
 * д������������ᵼ��æ״̬, ���в�����д����֮ǰҪ�ȴ���һ���������, ����HAL�⺯���ڲ������˴˲���;
 ****************************************************************************************************
 * @attention
 * ���ڲ����־û�, һҳ 2KB ���Է� 512 �� 32λ����, ֱ��ѡȡ���һҳ��Ϊ��������ҳ��
 */
#include "./BSP/FLASH/INTERNAL/flash.h"
#include "stdio.h"

/* static functions */
/* �����Ĳ���, ���ɶȵ�, ����¶��Ӧ�ò� */
static uint32_t flash_erase_page(uint32_t addr);
static uint32_t flash_write8_addr_withinPage(uint32_t addr, uint8_t *write_Data, uint16_t size);
static uint32_t flash_write16_addr_withinPage(uint32_t addr, uint16_t *write_Data, uint16_t size);
static uint32_t flash_write8_page_withinPage(uint8_t page_Index, uint8_t *write_Data, uint16_t size);

/**
 * @brief   ��1BΪ��λ����������;
 * @param   addr: Ҫ��ȡ�ĵ�ַ;
 * @param   read_Data: �����ݶ�ȡ����ָ��ָ��ĵ�ַ��ʼ����������;
 * @param   size: Ҫ��ȡ�����ݳ���, ��λΪ8bit;
 */
void flash_read_1B(uint32_t addr, uint8_t *read_Data, uint16_t size)
{
    if (size == 0)
    {
        printf("Flash��ȡʧ��, ԭ��: size == 0\r\n");
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        read_Data[i] = *((__IO uint8_t *)(addr + i));
    }
}

/**
 * @brief   ��2BΪ��λ����������;
 * @param   addr: Ҫ��ȡ�ĵ�ַ;
 * @param   read_Data: �����ݶ�ȡ����ָ��ָ��ĵ�ַ��ʼ����������;
 * @param   size: Ҫ��ȡ�����ݳ���, ��λΪ16bit;
 */
void flash_read_2B(uint32_t addr, uint16_t *read_Data, uint16_t size)
{
    if (size == 0)
    {
        printf("Flash��ȡʧ��, ԭ��: size == 0\r\n");
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        read_Data[i] = *((__IO uint16_t *)(addr + 2 * i));
    }
}

/**
 * @brief   ��4BΪ��λ����������;
 * @param   addr: Ҫ��ȡ�ĵ�ַ;
 * @param   read_Data: �����ݶ�ȡ����ָ��ָ��ĵ�ַ��ʼ����������;
 * @param   size: Ҫ��ȡ�����ݳ���, ��λΪ32bit;
 */
void flash_read_4B(uint32_t addr, uint32_t *read_Data, uint16_t size)
{
    if (size == 0)
    {
        printf("Flash��ȡʧ��, ԭ��: size == 0\r\n");
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        read_Data[i] = *((__IO uint32_t *)(addr + 4 * i));
    }
}

/**
 * @brief   ��1BΪ��λ��һ��ҳ����;
 * @param   page_Index: ҳ��, 0 ~ 255;
 * @param   read_Data: �����ݶ�ȡ����ָ��ָ��ĵ�ַ��ʼ����������;
 */
void flash_read_page(uint8_t page_Index, uint8_t *read_Data)
{
    if (page_Index > MSS_FLASH_PAGE_NUM - 1)
    {
        printf("Flash����ҳʧ��, ԭ��: ҳ�ŷǷ�\r\n");
        return;
    }
    flash_read_1B(page_Index * MSS_FLASH_PAGE_SIZE_1B, read_Data, MSS_FLASH_PAGE_SIZE_1B);
}

/**
 * @brief   д���ⳤ�ȵ����ݵ�ָ��λ��, ֧�ֿ�ҳ;
 * @param   addr: Ҫд��ĵ�ַ;
 * @param   write_Data: ����ָ��ָ����ڴ��ַ��ʼ�����������е�����д��Flash��;
 * @param   size: Ҫд������ݳ���, ��λΪ 1B;
 * @note    ����������ʵ��д��ǰ�Զ�ִ�в�������, �����ֶ�����;
 * @note    Ҫд���ҳ�ڵ�����λ�õ����ݽ�������;
 */
void flash_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t size)
{
    /**
     * 1.��addr����ҳΪ��ʼҳ, ��д����ʼҳ, ��סwrite_data��ƫ������ʣ�����ݵ�size;
     * 2.ʣ�µ�����ֱ�ӵ���flash_write_page;
     */
    /* ���Խ�� */
    if (size == 0)
    {
        printf("Flashд��ʧ��, ԭ��: size == 0\r\n");
        return;
    }
    if ((addr + size - 1) > MSS_FLASH_END)
    {
        printf("Flashд��ʧ��, ԭ��: size����\r\n");
        return;
    }
    /* ������ʼҳ */
    /* ��һҳӦ��д��������� */
    uint16_t first_Size = MSS_FLASH_PAGE_SIZE_1B - addr % MSS_FLASH_PAGE_SIZE_1B;
    flash_write8_addr_withinPage(addr, write_Data, first_Size);
    /* ����ʣ���ҳ */
    uint8_t first_Page_Index = addr / MSS_FLASH_PAGE_SIZE_1B;
    flash_write_page(first_Page_Index + 1, write_Data + first_Size, size - first_Size);
}

/**
 * @brief   д���ⳤ�ȵ����ݵ�ָ��ҳ, ֧�ֿ�ҳ;
 * @param   first_Page_Index: Ҫд���ҳ��ҳ��;
 * @param   write_Data: ����ָ��ָ����ڴ��ַ��ʼ�����������е�����д��Flash��;
 * @param   size: Ҫд������ݳ���, ��λΪ 1B;
 * @note    ����������ʵ��д��ǰ�Զ�ִ�в�������, �����ֶ�����;
 * @note    Ҫд���ҳ�ڵ�����λ�õ����ݽ�������;
 */
void flash_write_page(uint8_t first_Page_Index, uint8_t *write_Data, uint32_t size)
{
    /**
     * 1.���㹲д����ҳ;
     * 2.������һҳҳ���Ƿ�Ϸ�;
     * 3.����, ÿ�ζ�ȡһ��ҳ, ���ڴ����޸�, �޸ĺ�д��;
     */
    if (size == 0)
    {
        printf("Flashд��ʧ��, ԭ��: size == 0\r\n");
        return;
    }
    uint16_t total_Page = (size - 1) / MSS_FLASH_PAGE_SIZE_1B + 1; /* ����Ҫд�������ܹ�ռ����ҳ */
    uint8_t end_Page_Index = first_Page_Index + total_Page - 1;
    /* ������һҳ��ҳ���Ƿ��� 0 ~ 255 ֮�� */
    if (end_Page_Index > MSS_FLASH_PAGE_NUM - 1)
    {
        printf("Flashд��ʧ��, ԭ��: size����\r\n");
        return;
    }
    /* ����ÿһҳ, д�� */
    uint16_t current_size = MSS_FLASH_PAGE_SIZE_1B;
    for (uint8_t i = 0; i < total_Page; i++)
    {
        /* ���һҳ��sizeҪ���⴦�� */
        if (i == (total_Page - 1))
        {
            current_size = size - (total_Page - 1) * MSS_FLASH_PAGE_SIZE_1B; /* ALERT: ������ current_size = size % MSS_FLASH_PAGE_SIZE_1B ���������һҳ��size, ������һҳǡ��Ҫд��, ȡģ��õ� 0 */
        }
        flash_write8_page_withinPage(first_Page_Index + i, write_Data + i * MSS_FLASH_PAGE_SIZE_1B, current_size);
    }
}

/**
 * @brief   д����С�ڵ���FLASH_PAGE_SIZE�����ݵ�ָ��λ��, ���ɿ�ҳ;
 * @param   TODO: addr: Ҫд��ĵ�ַ;
 * @param   data: ����ָ��ָ����ڴ��ַ��ʼ�����������е�����д��Flash��;
 * @param   size: Ҫд������ݳ���, ��λΪ 1B;
 * @note    ����������ʵ��д��ǰ�Զ�ִ�в�������, �����ֶ�����;
 * @note    Ҫд���ҳ�ڵ�����λ�õ����ݽ�������;
 */
static uint32_t flash_write8_addr_withinPage(uint32_t addr, uint8_t *write_Data, uint16_t size)
{
    /**
     * 1.��ȡ����ҳ�������ڴ�
     * 2.�޸��ڴ�
     * 3.��������ҳ
     * 4.д��
     */
    if (size == 0)
    {
        printf("Flashд��ʧ��, ԭ��: size == 0\r\n");
        return 1;
    }
    /* �ж�д�����λ���Ƿ���ͬһҳ�� */
    uint32_t page_Base = addr - addr % MSS_FLASH_PAGE_SIZE_1B; /* ҳ�׵�ַ */
    uint32_t write_End = addr + size - 1;                      /* д�����λ�� */
    if ((write_End - write_End % MSS_FLASH_PAGE_SIZE_1B) != page_Base)
    {
        printf("Flashд��ʧ��, ԭ��: ��ҳ\r\n");
        return 1;
    }

    /* ��ȡ���� */
    uint8_t buffer[MSS_FLASH_PAGE_SIZE_1B] = {0};
    flash_read_1B(page_Base, buffer, MSS_FLASH_PAGE_SIZE_1B);
    /* ���ڴ����޸� */
    uint16_t offset = addr - page_Base; /* д��λ�������ҳ�׵�ƫ����, ��1BΪ��λ */
    for (uint16_t i = 0; i < size; i++)
    {
        buffer[offset + i] = write_Data[i];
    }
    /* ���� */
    uint32_t page_Error = flash_erase_page(page_Base);

    /**
     * NOTE: ת��Ϊ����2BΪ������λд��;
     * Ϊʲô��������? RAM��Flash������С�˴洢, �Ա��� uint8_t 0x11, 0x12, 0x10Ϊ��, ������߱�ʾ����͵�ַ;
     * ��RAM��, �� | 11 | 12 | 10 | FF(������) |; ת��Ϊ 16λָ��, ����RAM����С��ģʽ, �������ַ��������Ϊ���� uint16_t 0x1211, 0xFF10;
     * ��ʱ��HALFWORD���� HAL_FLASH_Program, ����FLASHд�� 0x1211, 0xFF10;
     * FLASHͬ������С��ģʽ, д��0x1211ʱ, ��д 11, ��д12; 0xFF10ͬ��, ����FLASH���� | 11 | 12 | 10 | FF |;
     * ��ʱ�ٰ� 8bit �� FLASH, �����ľ��� 0x11, 0x12, 0x10;
     * ������Ȼ��һ������, ������������ֽ�, ��ôת�� 16λָ���, ����1B, ������һ��������, ����������Ƚ�FLASHһ��ҳ�����ݶ����ڴ�;
     * ������� 1B ����ԭ��Flash�е�����, ����������;
     */
    HAL_FLASH_Unlock();
    uint16_t *ptr = (uint16_t *)buffer;
    for (uint16_t i = 0; i < MSS_FLASH_PAGE_SIZE_2B; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_Base + 2 * i, ptr[i]);
    }
    HAL_FLASH_Lock();
    return page_Error;
}

/**
 * @brief   д����С�ڵ���FLASH_PAGE_SIZE�����ݵ�ָ��λ��, ���ɿ�ҳ;
 * @param   TODO: addr: Ҫд��ĵ�ַ, ������2B��������
 * @param   data: ����ָ��ָ����ڴ��ַ��ʼ�����������е�����д��Flash��;
 * @param   size: Ҫд������ݳ���, ��λΪ 2B;
 * @note    ����������ʵ��д��ǰ�Զ�ִ�в�������, �����ֶ�����;
 * @note    Ҫд���ҳ�ڵ�����λ�õ����ݽ�������;
 */
static uint32_t flash_write16_addr_withinPage(uint32_t addr, uint16_t *write_Data, uint16_t size)
{
    /**
     * 1.��ȡ����ҳ�������ڴ�
     * 2.�޸��ڴ�
     * 3.��������ҳ
     * 4.д��
     */
    if (size == 0)
    {
        printf("Flashд��ʧ��, ԭ��: size == 0\r\n");
        return 1;
    }
    /* �ж�д�����λ���Ƿ���ͬһҳ�� */
    uint32_t page_Base = addr - addr % MSS_FLASH_PAGE_SIZE_1B; /* ҳ�׵�ַ */
    uint32_t write_End = addr + 2 * size - 1;                  /* д�����λ�� */
    if ((write_End - write_End % MSS_FLASH_PAGE_SIZE_1B) != page_Base)
    {
        printf("Flashд��ʧ��, ԭ��: ��ҳ\r\n");
        return 1;
    }
    /* �жϴ����ַ�Ƿ���2B�������� */
    if (addr % 2 != 0)
    {
        printf("Flashд��ʧ��, ԭ��: ��ַ��2B������\r\n");
        return 1;
    }

    /* ��ȡ���� */
    uint16_t buffer[MSS_FLASH_PAGE_SIZE_2B] = {0};
    flash_read_2B(page_Base, buffer, MSS_FLASH_PAGE_SIZE_2B);
    /* ���ڴ����޸� */
    uint16_t offset = (addr - page_Base) / 2; /* д��λ�������ҳ�׵�ƫ����, ��2BΪ��λ */
    for (uint16_t i = 0; i < size; i++)
    {
        buffer[offset + i] = write_Data[i];
    }
    /* ���� */
    uint32_t page_Error = flash_erase_page(page_Base);
    /* д�� */
    HAL_FLASH_Unlock();
    for (uint16_t i = 0; i < MSS_FLASH_PAGE_SIZE_2B; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_Base + 2 * i, buffer[i]);
    }
    HAL_FLASH_Lock();
    return page_Error;
}

/**
 * @brief   д���ⳤ��С�ڵ���FLASH_PAGE_SIZE�����ݵ�ָ��ҳ�Ŀ�ʼλ��
 * @param   page_Index: Ҫд���ҳ��ҳ��, �ڲ� Flash ��256ҳ, ҳ��0 ~ 255;
 * @param   data: ����ָ��ָ����ڴ��ַ��ʼ�����������е�����д�� Flash ��;
 * @param   size: Ҫд������ݳ���, ��λΪ 1B;
 * @note    ����������ʵ��д��ǰ�Զ�ִ�в�������, �����ֶ�����;
 * @note    ��������֧�ִ�ҳ�׵�ַ��ʼд;
 * @note    Ҫд���ҳ�ڵ�����λ�õ����ݽ�������;
 */
static uint32_t flash_write8_page_withinPage(uint8_t page_Index, uint8_t *write_Data, uint16_t size)
{
    /**
     * 1.��ȡҳ�Ŷ�Ӧҳ���ڴ�
     * 2.�޸��ڴ�
     * 3.��������ҳ
     * 4.д��
     */
    /* �ж�size�͵�ַ�Ƿ�Ϸ� */
    if (page_Index > MSS_FLASH_PAGE_NUM - 1)
    {
        printf("Flashд��ʧ��, ԭ��: ҳ�ŷǷ�\r\n");
        return 1;
    }
    
    if (size > MSS_FLASH_PAGE_SIZE_1B)
    {
        printf("Flashд��ʧ��, ԭ��: ��ҳ\r\n");
        return 1;
    }
    uint32_t page_Base = page_Index * MSS_FLASH_PAGE_SIZE_1B; /* ҳ�׵�ַ */

    /* ��ȡ���� */
    uint8_t buffer[MSS_FLASH_PAGE_SIZE_1B] = {0};
    flash_read_1B(page_Base, buffer, MSS_FLASH_PAGE_SIZE_1B);
    /* ���ڴ����޸� */
    for (uint16_t i = 0; i < size; i++)
    {
        buffer[i] = write_Data[i];
    }
    /* ���� */
    uint32_t page_Error = flash_erase_page(page_Base);
    /* д�� */
    HAL_FLASH_Unlock();
    uint16_t *ptr = (uint16_t *)buffer;
    for (uint16_t i = 0; i < MSS_FLASH_PAGE_SIZE_2B; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_Base + 2 * i, ptr[i]);
    }
    HAL_FLASH_Lock();
    return page_Error;
}

/**
 * @brief   ����ָ����ַ���ڵĵ�ҳ
 * @param   addr: ��������ĵ�ַ���ڵ�ҳ
 */
static uint32_t flash_erase_page(uint32_t addr)
{
    /* �ж� addr �Ƿ�Ϸ� */
    if (addr > MSS_FLASH_END)
    {
        printf("Flash����ʧ��, ԭ��: ��ַ�Ƿ�\r\n");
        return 1;
    }
    
    HAL_FLASH_Unlock(); /*Flash����*/
    uint32_t page_Error = 0;
    FLASH_EraseInitTypeDef erase_handle = {0};                       /* NOTE:��Ҫ��static, ���̻߳���� */
    erase_handle.PageAddress = addr - addr % MSS_FLASH_PAGE_SIZE_1B; /* �õ� addr ����ҳ����ʼ��ַ */
    erase_handle.NbPages = 1;
    erase_handle.TypeErase = FLASH_TYPEERASE_PAGES;
    HAL_FLASHEx_Erase(&erase_handle, &page_Error); /* ԭ�Ӵ���, �ڲ�ͨ��__HAL_LOCK��֤ԭ���� */
    HAL_FLASH_Lock();                              /* Flash���� */
    return page_Error;
}
