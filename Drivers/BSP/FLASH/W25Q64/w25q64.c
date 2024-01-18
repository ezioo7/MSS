/**
 ****************************************************************************************************
 * @file        w25q64.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2024-01-06
 * @brief       �ⲿFlash����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * Flash�洢���ڲ����ⲿFLASH;
 * �ڲ�Flashֱ�Ӷ�д, �ⲿFlash��SPIͨ�Ž��ж�д, ������API;
 * �������ļ������ⲿ W25Q64 Flash����
 ****************************************************************************************************
 * @attention
 * �ⲿFlash:
 * ��֯�ṹΪ ��-����-ҳ, ����4KB, ҳ256B; ��С�����û�����;
 * д����С��λ��1B, ��󳤶���һҳ; ������С��λ������4KB;
 * �����շ����� "����", ͨ����W25Q64������������ɷ��������;
 * д��Ͳ����ᵼ�½���æ״̬
 ****************************************************************************************************
 * @attention
 * ����д��������ѯ�ķ�ʽ����, Ϊ�˱�֤��ȫ��;
 * ����ʱ, �Ƚ���������, ��ȫ��ȡ��ɺ���ܽ�����һ���Ĺ���, ���Բ�����ѯ��ʽ;
 * �û�����������߹ػ�ʱ {TODO:�ػ�ʱҪ���������ϴα����Ƿ����޸�(ͨ�����ñ�ʾλʵ��), �����, ��ʾ�û�ѡ���Ƿ񱣴��²���} д��;
 * д�����(ֻ��д����Flash��RAM��)��Ҫ��FLASH�̻�(RAMд��FLASH��Ԫ)��ʱ��, �����ǲ�ѯBUSY, BUSY=0����Թػ�
 * �������ʱ�������޸Ĳ����������, ���Բ�����ѯ��ʽд�벻������������ܲ���Ӱ��
 ****************************************************************************************************
 */
#include "./BSP/FLASH/W25Q64/w25q64.h"
#include "./BSP/SPI/spi.h"
#include "stdio.h"

/* static variables */
static uint8_t cmd_Read_Status1[1] = {0x05};
static uint8_t cmd_Write_Enable[1] = {0x06};
static uint8_t cmd_Erase_Sector[1] = {0x20};
static uint8_t cmd_Write[1] = {0x02};
static uint8_t cmd_Read[1] = {0x03};
static uint8_t cmd_Read_ID[1] = {0x9F};

/* static functions */
static void w25q64_wait(void);
static void w25q64_write_enable(void);
static void w25q64_erase_sector(uint32_t addr);
static inline uint32_t get_sectorBase(uint32_t addr);
static inline uint32_t get_pageBase(uint32_t addr);
static inline void set_addr(uint32_t addr, uint8_t *cmd_Arr);

/**
 * @brief   ʹW25Q64���ڿ���״̬;
 * @note    ����涨W25Q64�̶�ʹ��SPI1;
 */
void w25q64_init(void)
{
    spi1_init(SPI_BAUDRATEPRESCALER_2);
}

/**
 * @brief   ��ȡW25Q64оƬ������Ϣ;
 * @param   read_Data: Я������������;
 * @note    ������Ϣ�������ֽ�, ���ݲ��ֵ�һ���ֽ� = ������ID, �ڶ����ֽ� = �洢����, �������ֽ� = оƬ����;
 */
void w25q64_read_ID(uint8_t *read_Data)
{
    printf("W25Q64��ȡID\r\n");
    w25q64_wait();                                                                            /* �ȴ����� */
    W25Q64_ENABLE();                                                                          /* Ƭѡ */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Read_ID, 1, W25Q64_TIMOUT);                   /* ���Ͷ����� */
    HAL_SPI_TransmitReceive(&CF_W25Q64_SPI_HANDLE, cmd_Read_ID, read_Data, 3, W25Q64_TIMOUT); /* ��������3B���� */
    W25Q64_DISABLE();                                                                         /* ʧ��оƬ */
}

/**
 * @brief   ����ַ�����ⳤ������;
 * @param   addr: Ҫ��ȡ�ĵ�ַ;
 * @param   read_Data: ��Я������������;
 * @param   len: ��ȡ�ĳ���, �޴�С����;
 */
void w25q64_read(uint32_t addr, uint8_t *read_Data, uint32_t len)
{
    /* �жϵ�ַ�ͳ����Ƿ�Ϸ� */
    if (addr + len - 1 > W25Q64_END)
    {
        printf("W25Q64��ȡʧ��, ԭ��: ��ַ�򳤶ȷǷ�\r\n");
        return;
    }
    printf("W25Q64��ȡ0x%06X\r\n", addr);
    uint8_t addr_arr[3] = {0};
    set_addr(addr, addr_arr);                                                   /* �����ַ */
    w25q64_wait();                                                              /* �ȴ����� */
    W25Q64_ENABLE();                                                            /* Ƭѡ */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Read, 1, W25Q64_TIMOUT);        /* ���Ͷ����� */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, addr_arr, 3, W25Q64_TIMOUT);        /* ���Ͷ���ַ */
    HAL_SPI_Receive(&CF_W25Q64_SPI_HANDLE, read_Data, len, 10 * W25Q64_TIMOUT); /* �������� */
    W25Q64_DISABLE();                                                           /* ʧ��оƬ */
}

/**
 * @brief   �������ź�ҳ�Ŷ�ȡ����;
 * @param   sector_Index: ������, 0 ~ 2K-1;
 * @param   page_Index: ҳ��, 0 ~ 15;
 * @param   read_Data: ��Я������������;
 * @note    �̶���һ��ҳ����;
 */
void w25q64_read_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *read_Data)
{
    if (sector_Index > W25Q64_SECTOR_NUM - 1)
    {
        printf("W25Q64��ȡʧ��, ԭ��: �����ŷǷ�\r\n");
        return;
    }
    if (page_Index > W25Q64_PAGE_NUM - 1)
    {
        printf("W25Q64��ȡʧ��, ԭ��: ҳ�ŷǷ�\r\n");
        return;
    }

    uint32_t read_Addr = sector_Index * W25Q64_SECTOR_SIZE + page_Index * W25Q64_PAGE_SIZE;
    w25q64_read(read_Addr, read_Data, W25Q64_PAGE_SIZE);
}

/**
 * @brief   д���ⳤ�����ݵ�ָ��λ��, ���Կ�����;
 * @param   addr: Ҫд��ĵ�ַ;
 * @param   write_Data: Ҫд������ݵ���ָ��;
 * @param   len: Ҫд������ݵĳ���, ��λΪB;
 * @note    д����������ڵ������������������ݽ�������, ���ᱻ������;
 */
void w25q64_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t len)
{
}

/**
 * @brief   д���ⳤ�����ݵ�ָ��������ָ��ҳ��ʼ��λ��, ���Կ�����;
 * @param   sector_Index: ������, 0 ~ 2K-1;
 * @param   page_Index: ҳ��, 0 ~ 15;
 * @param   write_Data: Ҫд������ݵ���ָ��;
 * @param   len: Ҫд������ݵĳ���, ��λΪ 1B;
 * @note    д����������ڵ������������������ݽ�������, ���ᱻ������;
 * @note    д����ʼλ�ù̶�Ϊָ��ҳ��ҳ��;
 */
void w25q64_write_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *write_Data, uint32_t len)
{
    uint32_t write_Addr = sector_Index * W25Q64_SECTOR_SIZE + page_Index * W25Q64_PAGE_SIZE;
}

/**
 * @brief   д���ݵ�ָ��λ��, Ҫ��д�����ʼλ�úͽ���λ��λ��ͬһ����֮��;
 * @param   addr: Ҫд��ĵ�ַ;
 * @param   write_Data: Ҫд������ݵ���ָ��;
 * @param   len: Ҫд������ݵĳ���, ��λΪB;
 * @note    �ᱣ������������λ�õ����ݲ���;
 */
void w25q64_write_addr_withinSector(uint32_t addr, uint8_t *write_Data, uint16_t len)
{
    /**
     * 1.��ȫ���, �����ַ;
     * 2.��д��λ���������������ڴ�, ���ڴ����޸�;
     * 3.����;
     * 4.д��;
     */
    /* ��ȫ���, �����ַ */
    uint32_t sector_Base = get_sectorBase(addr);               /* ��ȡд�뿪ʼλ�������������׵�ַ */
    uint32_t end_Sector_Base = get_sectorBase(addr + len - 1); /* д�����λ�����������׵�ַ */
    if (end_Sector_Base != sector_Base)                        /* �������λ�����������Ϳ�ʼλ������������һ��, �ܾ�д�� */
    {
        printf("W25Q64д��ʧ��, ԭ��: ������\r\n");
        return;
    }
    printf("W25Q64д��0x%06X\r\n", addr);

    /* ��д��λ���������������ڴ�, ���ڴ����޸� */
    /* MARK: ����̶���write_Data���Ƶ�buffer, ���write_Data�ܳ�, �����Ƚϴ�; */
    /* MARK: ���: ��Ȼ���� 4K ����, ����д�뿪ʼλ��֮ǰ�����ݳ��Ⱥͽ���λ��֮������ݳ���, �������buffer��������������д��; ʵ��̫�鷳 */
    uint8_t buffer[W25Q64_SECTOR_SIZE] = {0};
    w25q64_read(sector_Base, buffer, W25Q64_SECTOR_SIZE);
    uint16_t offset = addr % W25Q64_SECTOR_SIZE; /* ����д��λ������������׵�ַ��ƫ���� */
    for (uint16_t i = 0; i < len; i++)
    {
        buffer[i + offset] = write_Data[i];
    }

    /* ���� */
    w25q64_write_enable();            /* дʹ�� */
    w25q64_wait();                    /* �ȴ�оƬ���� */
    w25q64_erase_sector(sector_Base); /* ���� */
    /* NOTE: ������, ��Ҫ�ȴ�������д��, ���ȱ�ٵȴ�����, ��оƬæʱֱ�ӷ���д����, д��������� */
    /* ALERT: ���ȴ������¶��������Ǳ��������ֵ, ��Ϊд��ָ�����û�б�ִ�� */
    w25q64_wait(); /* �ȴ�оƬ���� */

    /* ��ҳд�� */
    uint8_t addr_arr[3] = {0};
    for (uint8_t page_Index = 0; page_Index < W25Q64_PAGE_NUM; page_Index++)
    {
        set_addr(sector_Base + page_Index * W25Q64_PAGE_SIZE, addr_arr);                                                      /* �����ַ */
        w25q64_wait();                                                                                                        /* �ȴ�оƬ���� */
        w25q64_write_enable();                                                                                                /* дʹ�� */
        w25q64_wait();                                                                                                        /* �ȴ�оƬ���� */
        W25Q64_ENABLE();                                                                                                      /* Ƭѡ */
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Write, 1, W25Q64_TIMOUT);                                                 /* ����дָ�� */
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, addr_arr, 3, W25Q64_TIMOUT);                                                  /* ���͵�ַ */
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, buffer + page_Index * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE, 5 * W25Q64_TIMOUT); /* ����д������� */
        W25Q64_DISABLE();                                                                                                     /* ʧ��оƬ */
    }
}

/**
 * @brief   ����ָ����ַ���ڵ�4KB����;
 * @param   addr: Ҫ�����ĵ�ַ;
 */
static void w25q64_erase_sector(uint32_t addr)
{
    uint8_t addr_arr[3] = {0};
    addr = get_sectorBase(addr);                                                 /* �ҵ� addr���������׵�ַ */
    set_addr(addr, addr_arr);                                                    /* �����ַ */
    W25Q64_ENABLE();                                                             /*Ƭѡ*/
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Erase_Sector, 1, W25Q64_TIMOUT); /* ���Ͳ������� */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, addr_arr, 3, W25Q64_TIMOUT);         /* Я����ַ, ��������������æ״̬, HAL_SPI_Transmitֻ��ע����, ���Խ��� */
    W25Q64_DISABLE();                                                            /* ���Ƭѡ */
}

/**
 * @brief   �ȴ�w25q64������״̬, æ��ѭ���ȴ�;
 * @note    ע��ʹ��֮ǰ��Ҫ���� W25Q64_ENABLE()ʹ��Flash; ������״̬�Ĵ�������FLASH�洢��Ԫ, ����FLASHæ��ʱ��Ҳ�ܶ�;
 */
static void w25q64_wait(void)
{
    uint8_t status1 = 0;
    W25Q64_ENABLE();
    do
    {
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Read_Status1, 1, W25Q64_TIMOUT); /* ���Ͷ�status1�Ĵ��������� */
        HAL_SPI_Receive(&CF_W25Q64_SPI_HANDLE, &status1, 1, W25Q64_TIMOUT);          /* �����õ�status1��ֵ(8bit) */

    } while ((status1 & 0x01) == 0x01);
    W25Q64_DISABLE();
}

/**
 * @brief   дʹ��
 * @note    ע��ʹ��ǰ��Ҫ���� W25Q64_ENABLE()ʹ��Flash��ȷ��оƬ����
 */
static void w25q64_write_enable(void)
{
    W25Q64_ENABLE();
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Write_Enable, 1, W25Q64_TIMOUT); /* HAL_SPI_Transmitֻ��ע����, ���Խ��� */
    W25Q64_DISABLE();
}

/**
 * @brief   ����һ����ַ���ڵ��������׵�ַ;
 * @note    ������С4KB, 16���Ʊ�ʾΪ 0x1000 B, �������׵�ַ����Ϊ 0x0000, 0x1000, 0x2000...
 */
static inline uint32_t get_sectorBase(uint32_t addr)
{
    return addr - (addr % W25Q64_SECTOR_SIZE);
}

/**
 * @brief   ����һ����ַ����ҳ���׵�ַ
 * @note    ҳ��С256B, 16���Ʊ�ʾΪ 0x100B, ��ҳ�׵�ַ����Ϊ 0x000, 0x100, 0x200....
 */
static inline uint32_t get_pageBase(uint32_t addr)
{
    return addr - (addr % W25Q64_PAGE_SIZE);
}

/**
 * @brief   32λ��ַת��Ϊ24λ(������ʵ����ֻ��23λ��Ч), ���ŵ�����ĵ�2,3,4��λ��(��һ��λ�÷�����);
 * @param   addr: Ҫ��ʽ���ĵ�ַ;
 * @param   cmd_Arr: ת������ŵ���������;
 * @note    �����λԪ�طŵ�ַ��λ;
 */
static inline void set_addr(uint32_t addr, uint8_t *addr_Arr)
{
    addr_Arr[0] = (addr >> 16); /* ȡ��ַ��λ[22:16] */
    addr_Arr[1] = (addr >> 8);  /* ��ַ��λ[15:8] */
    addr_Arr[2] = (addr >> 0);  /* ��ַ��λ[7:0] */
    // printf("addr[]:%02X, %02X, %02X\r\n", addr_Arr[0], addr_Arr[1], addr_Arr[2]);
}
