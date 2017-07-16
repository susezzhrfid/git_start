/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�StatMgr.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���ն�ͳ����Ϣ������ͳ����Ĺ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��������ƽ
 * ������ڣ�2008��7��
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

//����:�ն�ͳ������
void DoTermStat(TTime* ptmNow, TTermStat* pTermStat)
{
	//WORD    wRstNum = 0;
	BYTE bData[4] = {0};
	
	if (IsTimeEmpty(&pTermStat->tmLastRun))	//��һ�����л���ͳ�����ݽṹ�������
	{
	    memset(pTermStat, 0, sizeof(TTermStat));
		pTermStat->tmLastRun = *ptmNow;	//�������һ�ε�����ʱ��
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
		g_PowerOffTmp.wRstNum = 0;	//���¸�λ����ͳ����0
		SavePoweroffTmp(false);
	}
	pTermStat->tmLastRun = *ptmNow;	//�������һ�ε�����ʱ��
	return;
}


//������ͨ����
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