/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Drivers.h
 * ժ    Ҫ���������API
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#ifndef DRIVERS_H
#define DRIVERS_H
#include "Typedef.h"

int DrvInit();
void ClearWDG();
void RunLedToggle();
bool ModemPowerOn();
bool ModemPowerOff();
bool ResetGC864();
bool ResetME3000();
bool ResetGL868();
bool ResetM590();
bool GetLed(BYTE bID);
void SetLed(bool fOn, BYTE bID);
WORD GetYxInput();
void ResetCPU();
void AlertLedToggle();
#endif
