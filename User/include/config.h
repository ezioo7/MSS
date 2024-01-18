/**
 ****************************************************************************************************
 * @file        config.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-01-08
 * @brief       ��������ģ��
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_Config_H
#define __MSS_Config_H

#include "stdint.h"



/* �ṹ��a.b     . �� -> �󶼲��ܼ�����, a.(b) �� a -> (b)������ */
/* MARK: ����ͨ�� */
#define __MSS_GET_CONFIG(__CONFIG_GROUP__, __CONFIG_ITEM__) ((mss_Config->__CONFIG_GROUP__).__CONFIG_ITEM__)
/* MARK: ����ͨ�� */
#define __MSS_SET_CONFIG(__CONFIG_GROUP__, __CONFIG_ITEM__, __CONFIG_VALUE__) ((mss_Config->__CONFIG_GROUP__).__CONFIG_ITEM__ = (__CONFIG_VALUE__))


/*TODO:����*/
void load_config_flash(void);
/*TODO:����*/
void store_config_flash(void);
/*TODO:����*/
void load_config_w25q64(void);
/*TODO:����*/
void store_config_w25q64(void);

#endif
