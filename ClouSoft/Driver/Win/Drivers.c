/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Drivers.c
 * 摘    要：驱动相关API
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
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

//获取LED的状态
//返回 true-点亮状态，false-息灭状态
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