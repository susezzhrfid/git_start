/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Drivers.c
 * ժ    Ҫ���������API
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#include "Sysdebug.h"
#include "FaConst.h"
#include "FaCfg.h"
#include "DrvConst.h"
#include "Drivers.h"

int DrvInit()
{
	EsamInit();

    DTRACE(DB_CRITICAL, ("DrvInit: "DRV_VER" OK.\r\n"));
    
    return 0;
}

void ClearWDG()
{

}

//��ȡLED��״̬
//���� true-����״̬��false-Ϣ��״̬
bool GetLed(BYTE bID)
{
	return true;
}

void RunLedToggle()
{

}

void AlertLedToggle()
{

}

bool ModemPowerOn()
{
	return true;
}

bool ModemPowerOff()
{
	return true;
}

bool ResetGC864()
{
	return true;
}

bool ResetME3000()
{
	return true;
}

bool ResetGL868()
{
	return true;
}

bool ResetM590()
{
	return true;
}

void SetLed(bool fOn, BYTE bID)
{
}

WORD GetYxInput()
{
	return 0;
}

void ResetCPU()
{
}