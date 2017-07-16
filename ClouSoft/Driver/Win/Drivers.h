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
