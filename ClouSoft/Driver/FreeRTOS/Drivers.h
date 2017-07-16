/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Drivers.h
 * 摘    要：驱动相关API
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#ifndef DRIVERS_H
#define DRIVERS_H
#include "Typedef.h"
#include "SAM3XA.h"
#include "ComStruct.h"


//接收缓冲区定义
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

//复位载波模块
void PlcReset(void);

#endif
