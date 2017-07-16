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
#include "SAM3XA.h"
#include "ComStruct.h"


//���ջ���������
/*typedef struct
{
	BYTE bRx[UART_RECV_BUFSIZE];
	int iHead;
	int iTail;
} TUartRx;
*/

int DrvInit(void);
void ResetCPU();
void RunLedToggle();
void ClearWDG();
bool ModemPowerOn();
bool ModemPowerOff();
bool ResetGC864();
bool ResetME3000();
bool ResetM590(void);
bool ResetGL868(void);

void SetRTS();
void ResetRTS();
bool IsPwrOff();
void BatOnCtrl();
void BatOffCtrl();
void SetLed(bool fOn, BYTE bID);
bool GetLed(BYTE bID);
void ToggleLed(BYTE bID);
void AlertLedToggle();
void GetYxInput(BYTE *pbYxVal);

void RtcSetTime(const TTime* pTime);
void RtcGetTime(TTime* pTime);

BYTE GetModeState(void);
void InitRandom(void);
bool GetRandom(DWORD *pdwRandom);

bool AcPowerOff(void);
bool PowerLow(void);
void PowerOffProtect(void);

//��λ�ز�ģ��
void PlcReset(void);

#endif
