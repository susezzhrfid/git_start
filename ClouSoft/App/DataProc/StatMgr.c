/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：StatMgr.cpp
 * 摘    要：本文件主要实现终端统计信息及数据统计类的管理
 * 当前版本：1.0
 * 作    者：杨凡、李湘平
 * 完成日期：2008年7月
 *********************************************************************************************************/
//#include "stdafx.h"
#include "StatMgr.h"
#include "ComAPI.h"
#include "FaAPI.h"
//#include <string.h>
//#include <stdio.h>
//#include "SysFs.h"
#include "Info.h"
#include "MtrAPI.h"
#include "LibDbAPI.h"
#include "DbFmt.h"

//描述:终端统计数据
void DoTermStat(TTime* ptmNow, TTermStat* pTermStat)
{
	//WORD    wRstNum = 0;
	BYTE bData[4] = {0};
	
	if (IsTimeEmpty(&pTermStat->tmLastRun))	//第一次运行或者统计数据结构被清除过
	{
	    memset(pTermStat, 0, sizeof(TTermStat));
		pTermStat->tmLastRun = *ptmNow;	//更新最后一次的运行时间
		DTRACE(DB_DP, ("DoTermStat: pTermStat->tmLastRun IsTimeEmpty!.###########\r\n"));
	}
		
	if (IsDiffDay(&pTermStat->tmLastRun, ptmNow))
	{	               
		ClrMtrRdStat();
		ReadItemDw(BN0, PN0, 0xE1800011, bData);
		WriteItemDw(BN0, PN0, 0xE1800012, bData);
		pTermStat->dwDayFlux = 0;
		memset(bData, 0x00, sizeof(bData));
		WriteItemDw(BN0, PN0, 0xE1800011, bData);
	}

	if (MonthFrom2000(&pTermStat->tmLastRun) != MonthFrom2000(ptmNow))
	{	
		memset(bData, 0x00, sizeof(bData));
		ReadItemDw(BN0, PN0, 0xE1800013, bData);
		WriteItemDw(BN0, PN0, 0xE1800014, bData);
		ReadItemDw(BN0, PN0, 0xE1800016, bData);
		WriteItemDw(BN0, PN0, 0xE1800017, bData);	
		
		memset(bData, 0x00, sizeof(bData));		
		WriteItemDw(BN0, PN0, 0xE1800016, bData);
		WriteItemDw(BN0, PN0, 0xE1800013, bData);

		pTermStat->dwMonFlux = 0;
		g_PowerOffTmp.wRstNum = 0;	//当月复位次数统计清0
		SavePoweroffTmp(false);
	}
	pTermStat->tmLastRun = *ptmNow;	//更新最后一次的运行时间
	return;
}


//运行普通任务
bool DoMgrDataStat(TTime* ptmNow)
{
	//TTime now;
	//GetCurTime(&now);
        
	if (ptmNow->nMinute != g_PowerOffTmp.tTermStat.tmLastRun.nMinute)
	{
       DoTermStat(ptmNow, &g_PowerOffTmp.tTermStat);
	}
	if(GetAcPn() > 0)
	{
	   DoDpDataStat(ptmNow, &g_PowerOffTmp.tDayVoltStat, &g_PowerOffTmp.tMonVoltStat);
	}
	return true;
}