/**
 ****************************************************************************************************
 * @file        UpdateState.h
 * @author      �����飬zhuowenwei
 * @version     V1.0
 * @date        2024-12-06
 * @brief       ϵͳ״̬�仯
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_UpdateState_H
#define __MSS_UpdateState_H

typedef unsigned short int uint16_t;


uint16_t Standy(void);
uint16_t Transition(void);
uint16_t Running(void);
uint16_t SystemLock(void);
uint16_t Testing(void);

void UpdateState(void);

#endif
