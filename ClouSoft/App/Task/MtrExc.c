 /*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrExc.c
 * 摘    要：实现告警事件的判断，存储；
 *
 * 版    本: 1.0
 * 作    者：
 * 完成日期：2011年3月
************************************************************************************************************/
#include "MtrExc.h"
#include "syscfg.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "sysarch.h"
#include "ProAPI.h"
#include "MtrAPI.h"
#include "ComAPI.h"
#include "DbFmt.h"
#include "DbConst.h"
#include <math.h>
#include "SysDebug.h"
#include "DbAPI.h"
#include "ExcTask.h"
#include "TaskDB.h"
#include "GbPro.h"
#include "MtrCtrl.h"

TMtrAlrCtrl g_MtrAlrCtrl[DYN_PN_NUM];	//电表告警控制结构


//描述：判断表地址是否正确（表地址不能为0）；
//返回：若表地址的值为0， 返回false;反之，返回true;
bool IsMtrAddrOK(BYTE *bBuf)
{
	BYTE b;
	for (b=0; b<6; b++)
	{
		if (*(bBuf+b) > 0)
			return true;
	}
	
	DTRACE(DB_METER_EXC, ("IsMtrAddrOK: Meter Address is 0, Addr Wrong!********\r\n"));
	return false;
}

//
//****************各子类事件*****************************
//==================================电能表失压(bAlrID = 0xE2000016~0xE2000018)========================================================
static const WORD g_wMeterVoltageID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//描述：电能表失压告警的初始化。
void MtrCVMissTimesChgInit(WORD wPn, TMtrCVLess* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//描述：电能表失压告警判断。
bool MtrCVMissTimesChgTask(WORD wPn, TMtrCVLess* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead,i;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	bool fChangeOk = true;
	WORD wMeterTimesIds[] = {0xea14,0xea16,0xea18};
	BYTE bLenth = GetItemLen(BN0, wMeterTimesIds[0]);
	BYTE bBuf[9] = {0};
	TBankItem tDataID[18] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now = { 0 };
// 	DWORD dwReTime = 0;
	GetCurTime(&now);

	DTRACE(DB_VMISS,("MtrCVMissTimesChgTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_VMISS,("MtrCVMissTimesChgTask : P%d data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	DTRACE(DB_VMISS,("MtrCVMissTimesChgTask : P%d data effective\r\n", wPn));
	//有变化
	for(i=0; i<3; i++)
	{
		if(!IsAlrEnable(0xE2000016+i))
		{
			DTRACE(DB_VMISS,("MtrCVMissTimesChgTask : Alr 0x%08X disable\r\n", 0xE2000016+i));
			continue;
		}

		if (!IsBcdCode(bBuf+i*bLenth, bLenth))
		{
			DTRACE(DB_VMISS,("MtrCVMissTimesChgTask : P%d %c data invalid\r\n", wPn, 'A'+i));
			continue;
		}
		if(!IsBcdCode(pCtrl->bBackBuf+i*bLenth, bLenth))
		{
			DTRACE(DB_VMISS,("MtrCVMissTimesChgTask : P%d %c data initial\r\n", wPn, 'A'+i));
			memcpy(pCtrl->bBackBuf+i*bLenth, bBuf+i*bLenth, bLenth);
		}

		TraceBuf(DB_VMISS, "MtrCVMissTimesChgTask : current data -> ", bBuf+i*bLenth,bLenth);
		TraceBuf(DB_VMISS, "MtrCVMissTimesChgTask : last data -> ", pCtrl->bBackBuf+i*bLenth,bLenth);
		if (memcmp(bBuf+i*bLenth, pCtrl->bBackBuf+i*bLenth, bLenth) != 0)
		{
			DTRACE(DB_VMISS, ("MtrCVMissTimesChgTask : ############# P%d exception 0x%08X confirmed.\r\n",wPn, 0xE2000016+i));
			bDataIdNum = sizeof(g_wMeterVoltageID)/sizeof(g_wMeterVoltageID[0]);
			for(n=0; n<bDataIdNum; n++)
			{
				tDataID[n].wBn = BN0;
				tDataID[n].wID = g_wMeterVoltageID[n];
				tDataID[n].wPn = wPn;
			}
// 			ReadItemGetTime(BN0, wPn, wMeterTimesIds[i], bBuf+i*bLenth, &dwReTime);
// 			SecondsToTime(dwReTime, &now);
			if (HandleAlr(0xE2000016+i, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
				memcpy(pCtrl->bBackBuf+i*bLenth, bBuf+i*bLenth, bLenth);
			else
				fChangeOk = false;
		}
	}

	if (!fChangeOk)
	{
		return false;
	}

	return true;
}
//==================================电能表失流(bAlrID = 0xE2000013~0xE2000015)========================================================
static const WORD  g_wCurrentID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//描述：电能表失流告警的初始化。
void MtrCIMissTimesChgInit(WORD wPn, TMtrCIMisss* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//描述：电能表失流告警判断。
bool MtrCIMissTimesChgTask(WORD wPn, TMtrCIMisss* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead,i;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	bool fChangeOk = true;
	WORD wMeterTimesIds[] = {0xea40, 0xea42, 0xea44};
	WORD bLenth = GetItemLen(BN0, wMeterTimesIds[0]);
	BYTE bBuf[9] = {0};
	TBankItem tDataID[18] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_EXC1,("MtrCIMissTimesChgTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_EXC1,("MtrCIMissTimesChgTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	DTRACE(DB_EXC1,("MtrCIMissTimesChgTask : P%d data effective\r\n", wPn));
	//有变化
	for(i=0; i<3; i++)
	{
		if(!IsAlrEnable(0xE2000013+i))
		{
			DTRACE(DB_EXC1,("MtrCIMissTimesChgTask : Alr 0x%08X disable\r\n",0xE2000013+i));
			continue;
		}

		if (!IsBcdCode(bBuf+i*bLenth, bLenth))
		{
			DTRACE(DB_EXC1,("MtrCIMissTimesChgTask : P%d %c data invalid\r\n", wPn, 'A'+i));
			continue;
		}
		if(!IsBcdCode(pCtrl->bBackBuf+i*bLenth, bLenth))
		{
			DTRACE(DB_EXC1,("MtrCIMissTimesChgTask : P%d %c data initial\r\n", wPn, 'A'+i));
			memcpy(pCtrl->bBackBuf+i*bLenth, bBuf+i*bLenth, bLenth);
		}

		TraceBuf(DB_EXC1, "MtrCIMissTimesChgTask : current data -> ", bBuf+i*bLenth,bLenth);
		TraceBuf(DB_EXC1, "MtrCIMissTimesChgTask : last data -> ", pCtrl->bBackBuf+i*bLenth,bLenth);
		if (memcmp(bBuf+i*bLenth, pCtrl->bBackBuf+i*bLenth, bLenth) != 0)
		{
			DTRACE(DB_EXC1, ("MtrCIMissTimesChgTask : ############# P%d exception 0x%08X confirmed.\r\n",wPn, 0xE2000013+i));
			bDataIdNum = sizeof(g_wCurrentID)/sizeof(g_wCurrentID[0]);
			for(n=0; n<bDataIdNum; n++)
			{
				tDataID[n].wBn = BN0;
				tDataID[n].wID = g_wCurrentID[n];
				tDataID[n].wPn = wPn;
			}
			if (HandleAlr(0xE2000013+i, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
				memcpy(pCtrl->bBackBuf+i*bLenth, bBuf+i*bLenth, bLenth);
			else
				fChangeOk = false;
		}
	}

	if (!fChangeOk)
	{
		return false;
	}

	return true;
}
//==================================电能表潮流反向(bAlrID = 0xE200000D~0xE200000F)========================================================
static const WORD  g_wIRevsChgID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//描述：电能表潮流反向告警的初始化。
void MtrCIRevsTimesChgInit(WORD wPn, TMtrCIRevs* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//描述：电能表潮流反向告警判断。
bool MtrCIRevsTimesChgTask(WORD wPn, TMtrCIRevs* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead,i;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	bool fChangeOk = true;
	WORD wMeterTimesIds[] = {0xea60, 0xea62, 0xea64};
	WORD bLenth = GetItemLen(BN0, wMeterTimesIds[0]);
	BYTE bBuf[9] = {0};
	TBankItem tDataID[18] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_POLAR,("MtrCIRevsTimesChgTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_POLAR,("MtrCIRevsTimesChgTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	DTRACE(DB_POLAR,("MtrCIRevsTimesChgTask : P%d data effective\r\n", wPn));
	//有变化
	for(i=0; i<3; i++)
	{
		if(!IsAlrEnable(0xE200000D+i))
		{
			DTRACE(DB_POLAR,("MtrCIRevsTimesChgTask : Alr 0x%08X disable\r\n", 0xE200000D+i));
			continue;
		}

		if (!IsBcdCode(bBuf+i*bLenth, bLenth))
		{
			DTRACE(DB_POLAR,("MtrCIRevsTimesChgTask : P%d %c data invalid\r\n", wPn, 'A'+i));
			continue;
		}
		if (!IsBcdCode(pCtrl->bBackBuf+i*bLenth, bLenth))
		{
			DTRACE(DB_POLAR,("MtrCIRevsTimesChgTask : P%d %c data initial\r\n", wPn, 'A'+i));
			memcpy(pCtrl->bBackBuf+i*bLenth, bBuf+i*bLenth, bLenth);
		}

		TraceBuf(DB_POLAR, "MtrCIRevsTimesChgTask : current data -> ", bBuf+i*bLenth,bLenth);
		TraceBuf(DB_POLAR, "MtrCIRevsTimesChgTask : last data -> ", pCtrl->bBackBuf+i*bLenth,bLenth);
		if (memcmp(bBuf+i*bLenth, pCtrl->bBackBuf+i*bLenth, bLenth) != 0)
		{
			DTRACE(DB_POLAR, ("MtrCIRevsTimesChgTask : ############# P%d exception 0x%08X confirmed.\r\n",wPn, 0xE200000D+i));
			bDataIdNum = sizeof(g_wIRevsChgID)/sizeof(g_wIRevsChgID[0]);
			for(n=0; n<bDataIdNum; n++)
			{
				tDataID[n].wBn = BN0;
				tDataID[n].wID = g_wIRevsChgID[n];
				tDataID[n].wPn = wPn;
			}
			if (HandleAlr(0xE200000D+i, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
				memcpy(pCtrl->bBackBuf+i*bLenth, bBuf+i*bLenth, bLenth);
			else
				fChangeOk = false;
		}
	}

	if (!fChangeOk)
	{
		return false;
	}

	return true;
}
//==================================电能表编程时间更改(bAlrID = 0xE2000035)========================================================
static const WORD  g_wPrgTmChgID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//描述：电能表编程时间更改告警的初始化。
void MtrPrgTmChgInit(WORD wPn, TMtrPrgTm* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//描述：电能表编程时间更改告警判断。
bool MtrPrgTmChgTask(WORD wPn, TMtrPrgTm* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	bool fChangeOk = true;
	WORD wMeterTimesIds[] = {0xc811};
	WORD bLenth = GetItemLen(BN0, wMeterTimesIds[0]);
	BYTE bBuf[6] = {0};
	TBankItem tDataID[18] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_METER_EXC,("MtrPrgTmChgTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_METER_EXC,("MtrPrgTmChgTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	if (!IsBcdCode(bBuf, bLenth) || IsAllAByte(bBuf, 0, bLenth))
		return false;

	DTRACE(DB_METER_EXC,("MtrPrgTmChgTask : P%d data effective\r\n", wPn));
	if(!IsBcdCode(pCtrl->bBackBuf, bLenth) || IsAllAByte(pCtrl->bBackBuf, 0, bLenth))
	{
		DTRACE(DB_METER_EXC,("MtrPrgTmChgTask : P%d data initial\r\n", wPn));
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
	}
	TraceBuf(DB_METER_EXC, "MtrPrgTmChgTask : current data -> ", bBuf,bLenth);
	TraceBuf(DB_METER_EXC, "MtrPrgTmChgTask : last data -> ", pCtrl->bBackBuf,bLenth);
	//有变化
	if(!IsAlrEnable(0xE2000035))
	{
		DTRACE(DB_METER_EXC,("MtrPrgTmChgTask : Alr 0xE2000035 disable\r\n"));
		return false;
	}

	if (memcmp(bBuf, pCtrl->bBackBuf, bLenth) != 0)
	{
		DTRACE(DB_METER_EXC, ("MtrPrgTmChgTask : ############# P%d exception 0xE2000035 confirmed.\r\n",wPn));
		bDataIdNum = sizeof(g_wPrgTmChgID)/sizeof(g_wPrgTmChgID[0]);
		for(n=0; n<bDataIdNum; n++)
		{
			tDataID[n].wBn = BN0;
			tDataID[n].wID = g_wPrgTmChgID[n];
			tDataID[n].wPn = wPn;
		}
		if (HandleAlr(0xE2000035, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
			memcpy(pCtrl->bBackBuf, bBuf, bLenth);
		else
			fChangeOk = false;
	}

	if (!fChangeOk)
	{
		return false;
	}

	return true;
}
//==================================电能表继电器变位(bAlrID = 0xE200003B)========================================================
static const WORD  g_wPwStaChgID[] = {0xc862};

//描述：电能表继电器变位告警的初始化。
void MtrPwStaChgInit(WORD wPn, TMtrPwSta* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//描述：电能表继电器变位告警判断。
bool MtrPwStaChgTask(WORD wPn, TMtrPwSta* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	bool fChangeOk = true;
	WORD wMeterTimesIds[] = {0xc862};
	WORD bLenth = GetItemLen(BN0, wMeterTimesIds[0]);
	BYTE bBuf[2] = {0};
	TBankItem tDataID[1] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_EXC3,("MtrPwStaChgTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_EXC3,("MtrPrgTmChgTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	if (IsInvalidData(bBuf, bLenth))
		return false;

	DTRACE(DB_EXC3,("MtrPwStaChgTask : P%d data effective and statue is bBuf[0]&0x10 = 0x%02x\r\n", wPn, bBuf[0]&0x10));
	if(IsInvalidData(pCtrl->bBackBuf, bLenth))
	{
		DTRACE(DB_EXC3,("MtrPwStaChgTask : P%d data initial\r\n", wPn));
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
	}
	TraceBuf(DB_EXC3, "MtrPwStaChgTask : current data -> ", bBuf,bLenth);
	TraceBuf(DB_EXC3, "MtrPwStaChgTask : last data -> ", pCtrl->bBackBuf,bLenth);
	//有变化
	if(!IsAlrEnable(0xE200003B))
	{
		DTRACE(DB_EXC3,("MtrPwStaChgTask : Alr 0xE200003B disable\r\n"));
		return false;
	}

	if ((bBuf[0]&0x10)!=(pCtrl->bBackBuf[0]&0x10))
	{
		DTRACE(DB_EXC3, ("MtrPwStaChgTask : ############# P%d exception 0xE200003B confirmed.\r\n",wPn));
		bDataIdNum = sizeof(g_wPwStaChgID)/sizeof(g_wPwStaChgID[0]);
		for(n=0; n<bDataIdNum; n++)
		{
			tDataID[n].wBn = BN0;
			tDataID[n].wID = g_wPwStaChgID[n];
			tDataID[n].wPn = wPn;
		}
		if (HandleAlr(0xE200003B, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
			memcpy(pCtrl->bBackBuf, bBuf, bLenth);
		else
			fChangeOk = false;
	}

	if (!fChangeOk)
	{
		return false;
	}

	return true;
}
//==================================电能表拉合闸控制失败(bAlrID = 0xE200003C)========================================================
//============控制事件记录============
bool  MtrRelayControlEventRec(BYTE bEventType, WORD wPn, TTime time)
{
	BYTE bBuf[8] = {0};
	BYTE bAlrBuf[ALRSECT_SIZE] = {0};
	BYTE* pbAlr = NULL;
	WORD wTeminalCtrEvnID[] = {0x9010, 0x9020};
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	DWORD dwRdSec = 0;
	int iLen = 0;
	WORD wLen = BYTE_DATE_TIME+SG_BYTE_TYPE_EVENT+SG_BYTE_POSITIVE_POWER+SG_BYTE_NEGATIVE_POWER;

	bDataIdNum = sizeof(wTeminalCtrEvnID)/sizeof(wTeminalCtrEvnID[0]);
	for(n=0; n<bDataIdNum; n++)
	{
		DoDirMtrRd(BN0, wPn, wTeminalCtrEvnID[n], time);

		dwRdSec = GetCurIntervSec(wPn, &time);   //贴上当前抄表间隔时标
		ReadItemTm(BN0, wPn, wTeminalCtrEvnID[n], bBuf+iLen, dwRdSec, INVALID_TIME);	//没读到的数据都已经置成无效数据
		iLen += GetItemLen(BN0,wTeminalCtrEvnID[n]);
	}

	pbAlr = bAlrBuf + 7;//1个字节的CRC + 2个字节的Pn + 4个字节的ID
	pbAlr += TimeTo6Bcd(&time,pbAlr);
	*pbAlr++ = bEventType;
	memcpy(pbAlr,bBuf,wLen-BYTE_DATE_TIME-SG_BYTE_TYPE_EVENT);

	SaveAlrData(0xE201000E, wPn, bAlrBuf, wLen);
	DTRACE(DB_EXC4, ("MtrRelayControlEventRec: ########## Event happen, ID=0xE201000E\r\n"));

	return true;
}

//描述：电能表拉合闸控制失败告警判断。
bool MtrRelayTask(WORD wPn, TMtrPwSta* pCtrl)
{
	BYTE bCtrlMtrAddr[6] = { 0 };
	BYTE bData[16] = { 0 };
	BYTE bCmdFrm[256] = { 0 };
	DWORD dwTime = 0;
	BYTE bRetLen = 0;
	BYTE bRetFrm[256] = { 0 };
	BYTE bPort = 0;
	int iRet = 0;
	BYTE bPos = 0;
	bool fCtrlOk = false;
	WORD wMeterTimesIds[] = {0xc862};
	WORD bLenth = GetItemLen(BN0, wMeterTimesIds[0]);
	BYTE bBuf[2] = {0};
	BYTE bEventType = 0;
	TBankItem tDataID[1] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TYkCtrl tYkCtrl;//集抄测量点的控制
	TTime now = { 0 };
	GetCurTime(&now);

	ExFlashReadPnCtrlPara(wPn, &tYkCtrl);
	DTRACE(DB_EXC4,("MtrRelayTask : P%d function running and tYkCtrl.wYkPn = %d\r\n", wPn, tYkCtrl.wYkPn));
	if (wPn != tYkCtrl.wYkPn)//检查控制是否有效
		return true;

	ReadItemEx(BN0,wPn,0x8902,bCtrlMtrAddr);
	if(memcmp(tYkCtrl.bYkMtrAddr,bCtrlMtrAddr,6) != 0)//表档案发生变化
	{
		DTRACE(DB_EXC4,("MtrRelayTask : P%d meter address change\r\n", wPn, tYkCtrl.wYkPn));
		TraceBuf(DB_EXC4, "MtrRelayTask : current address -> ", bCtrlMtrAddr,6);
		TraceBuf(DB_EXC4, "MtrRelayTask : Yk address -> ", tYkCtrl.bYkMtrAddr,6);
		return true;
	}

	memcpy(bData, tYkCtrl.bYkOptPwd, 4);
	memcpy(bData+4, tYkCtrl.bYKOptCode, 4);
	bData[8] = tYkCtrl.bYKCtrlType;
	bData[9] = 0;	//N2保留
	dwTime = TimeToSeconds(&now);
	dwTime += tYkCtrl.bYKValDly*60;
	SecondsToTime(dwTime, &now);
	bData[10] = ByteToBcd(now.nSecond);
	bData[11] = ByteToBcd(now.nMinute);
	bData[12] = ByteToBcd(now.nHour);
	bData[13] = ByteToBcd(now.nDay);
	bData[14] = ByteToBcd(now.nMonth);
	bData[15] = ByteToBcd(now.nYear%100) ;

	Make645Frm(bCmdFrm,tYkCtrl.bYkMtrAddr,SCHED_07_CMD_ALARM,bData,SCHED_07_REQUEST_FRAME_LENGTH);

	ReadItemEx(BN0,wPn,0x890a,&bPort);
	iRet = DirectTransmit645Cmd(wPn, bPort, bCmdFrm, bCmdFrm[SCHED_07_FRM_LEN_BIT]+SCHED_07_FRM_PERMANENT_BYTE, bRetFrm, &bRetLen, tYkCtrl.bYKValDly);
	DTRACE(DB_EXC4,("MtrRelayTask : P%d meter port = 0x%02x iRet = %d\r\n", wPn, bPort, iRet));
	if (iRet > 0)
	{
		TraceBuf(DB_EXC4, "MtrRelayTask : Yk return data  bRetFrm -> ", bRetFrm, bRetLen);
		for (bPos=0; bPos<bRetLen; bPos++)
		{
			if (bRetFrm[bPos] == 0x68)
				break;
		}
		if (bPos < bRetLen)
			memcpy(bRetFrm, &bRetFrm[bPos], bRetLen-bPos); 	//去掉前导字符

		if (bRetFrm[SCHED_07_RESPONSE_BIT]==SCHED_07_RESPONSE_OK)	
		{
			fCtrlOk = true;
		}
		else if (bRetFrm[SCHED_07_RESPONSE_BIT]==SCHED_07_RESPONSE_ABNORMITY )	
		{
			fCtrlOk = false;
		}
		else
		{
			fCtrlOk = false;
		}
	}
	else
	{
		fCtrlOk = false;
	}

	DTRACE(DB_EXC4,("MtrRelayTask : P%d Yk fCtrlOk = %d\r\n", wPn, fCtrlOk));

	if(!IsAlrEnable(0xE200003C))
	{
		DTRACE(DB_EXC4,("MtrRelayTask : Alr 0xE200003C disable\r\n"));
		return false;
	}

    if (ReadItemEx(BN0,wPn,0x890a,&bPort) < 0)
    	return false;

	GetDirRdCtrl(bPort);	//取得直抄的控制权
	DoDirMtrRd(BN0, wPn, wMeterTimesIds[0], now);
	ReleaseDirRdCtrl(bPort); //释放直抄的控制权

	bDataIdNum = sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]);
	for(n=0; n<bDataIdNum; n++)
	{
		tDataID[n].wBn = BN0;
		tDataID[n].wID = wMeterTimesIds[n];
		tDataID[n].wPn = wPn;
	}
	if (fCtrlOk)
	{
		if(!IsAlrEnable(0xE200003B))
		{
			DTRACE(DB_EXC4,("MtrRelayTask : Alr 0xE200003B disable\r\n"));
			return false;
		}
		DTRACE(DB_EXC4, ("MtrRelayTask : ############# P%d exception 0xE200003B confirmed.\r\n",wPn));
		HandleAlr(0xE200003B, wPn, tDataID, bDataIdNum, 0, now, NULL, 0);//继电器变位
		ReadItemEx(BN0,wPn,wMeterTimesIds[0],bBuf);
		TraceBuf(DB_EXC4, "MtrRelayTask : 0xE200003B current data -> ", bBuf,bLenth);
		TraceBuf(DB_EXC4, "MtrRelayTask : 0xE200003B last data -> ", pCtrl->bBackBuf,bLenth);
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
	}
	else
	{
		DTRACE(DB_EXC4, ("MtrRelayTask : ############# P%d exception 0xE200003C confirmed.\r\n",wPn));
		HandleAlr(0xE200003C, wPn, tDataID, bDataIdNum, 0, now, NULL, 0);//拉合闸失败
	}

	if (0x1A == tYkCtrl.bYKCtrlType)
		bEventType = 1;
	else if (0x1B == tYkCtrl.bYKCtrlType || 0x1C == tYkCtrl.bYKCtrlType)
		bEventType = 2;
	else if (0x3A == tYkCtrl.bYKCtrlType)
		bEventType = 3;
	else if (0x3B == tYkCtrl.bYKCtrlType)
		bEventType = 4;
	MtrRelayControlEventRec(bEventType, wPn, now);
	tYkCtrl.wYkPn = 0;//清控制有效测量点
	ExFlashWritePnCtrlPara(wPn, &tYkCtrl, sizeof(TYkCtrl));
	
	return true;
}
//==================================电能表时钟异常(bAlrID = 0xE200003E)========================================================
static const WORD  g_wClockErrChgID[] = {0xc010, 0xc011, 0x8030};

//描述：电能表时钟异常告警的初始化。
void MtrClockErrChgInit(WORD wPn, TMtrClockErr* pCtrl)
{
	pCtrl->fExcValid = false;
	GetCurTime(&pCtrl->tmOccur);
	pCtrl->dwLastMin = 0;
}

//描述：电能表时钟异常告警判断。
bool MtrClockErrChgTask(WORD wPn, TMtrClockErr* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	WORD wMeterTimesIds[] = {0xc010, 0xc011};
	WORD bLenth = GetItemsLenId(wMeterTimesIds, sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]));
	BYTE bBuf[7] = {0};
	long int dwMeterSeconds = 0;
	long int dwTermnSeconds = 0;
	DWORD dwErr = 0;
	BYTE bClock = 0;
	TTime tmMeter = { 0 };
	TBankItem tDataID[3] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_EXC5,("MtrClockErrChgTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_EXC5,("MtrClockErrChgTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	if(!IsBcdCode(bBuf,bLenth))//无效数据
	{
		DTRACE(DB_EXC5,("MtrClockErrChgTask : P%d data is't BCD code\r\n", wPn));
		return false;
	}
	if(pCtrl->dwLastMin == dwCurMin)
	{
		DTRACE(DB_EXC5,("MtrClockErrChgTask : P%d dwCurMin=%d current minute already done\r\n", wPn, dwCurMin));
		return true;
	}
	pCtrl->dwLastMin = dwCurMin;

	tmMeter.nYear = 2000 + BcdToByte(bBuf[3]);
	tmMeter.nMonth = BcdToByte(bBuf[2]);
	tmMeter.nDay = BcdToByte(bBuf[1]);
	tmMeter.nHour = BcdToByte(bBuf[6]);
	tmMeter.nMinute = BcdToByte(bBuf[5]);
	tmMeter.nSecond = BcdToByte(bBuf[4]);
	dwMeterSeconds = TimeToSeconds(&tmMeter);

	TraceBuf(DB_EXC5,"MtrClockErrChgTask : Meter time is -> ", bBuf, bLenth);

	dwTermnSeconds = GetCurSec();
	dwErr = abs(dwTermnSeconds - dwMeterSeconds);

	ReadItemEx(BN0, PN0, 0x8048, bBuf);
	bClock = bBuf[0];		//时钟误差时间
	if (bClock == 0)
		bClock = 10;
	else 
		bClock = BcdToByte(bClock);

	DTRACE(DB_EXC5,("MtrClockErrChgTask : P%d data effective and dwErr = %d bClock = %d pCtrl->fExcValid = %d\r\n", wPn, dwErr, bClock, pCtrl->fExcValid));

	if(!IsAlrEnable(0xE200003E))
	{
		DTRACE(DB_EXC5,("MtrClockErrChgTask : Alr 0xE200003E disable\r\n"));
		return false;
	}

	if (dwErr > bClock*60)   //误差大于10分钟, 5
	{
		if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && true==pCtrl->fExcValid)//每天上报一次
		{
			DTRACE(DB_EXC5,("MtrClockErrChgTask : P%d time for day is differ\r\n", wPn));
			pCtrl->tmOccur = now;
			pCtrl->fExcValid = false;
		}

		if (pCtrl->fExcValid == false)
		{
			DTRACE(DB_EXC5, ("MtrClockErrChgTask : ############# P%d exception 0xE200003E confirmed.\r\n",wPn));
			bDataIdNum = sizeof(g_wClockErrChgID)/sizeof(g_wClockErrChgID[0]);
			for(n=0; n<bDataIdNum-1; n++)
			{
				tDataID[n].wBn = BN0;
				tDataID[n].wID = g_wClockErrChgID[n];
				tDataID[n].wPn = wPn;
			}
			tDataID[n].wBn = BN0;
			tDataID[n].wID = g_wClockErrChgID[n];
			tDataID[n].wPn = PN0;

			if (HandleAlr(0xE200003E, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
			{
				pCtrl->fExcValid = true;
				pCtrl->tmOccur = now;
				return true;
			}
			else
			{
				DTRACE(DB_EXC5,("MtrClockErrChgTask : P%d Alr data store error!\r\n", wPn));
				return false;
			}
		}  
	}
	else
	{
		pCtrl->fExcValid = false;
	}	

	return true;
}
//==================================电能表时段或费率更改(bAlrID = 0xE2000036)========================================================
static const WORD  g_wTouChgID[] = {0xc33f, 0xc32f};

//描述：电能表时段或费率更改告警的初始化。
void MtrTouChgInit(WORD wPn, TMtrTou* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//描述：电能表时段或费率更改告警判断。
bool MtrTouChgTask(WORD wPn, TMtrTou* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	bool fChangeOk = true;
	WORD wMeterTimesIds[] = {0xc33f, 0xc32f};
	WORD bLenth = GetItemsLenId(wMeterTimesIds, sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]));
	BYTE bBuf[84] = {0};
	TBankItem tDataID[2] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_CPS,("MtrTouChgTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_CPS,("MtrTouChgTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	if(!IsBcdCode(bBuf,bLenth))//无效数据
		return false;

	DTRACE(DB_CPS,("MtrTouChgTask : P%d data effective\r\n", wPn));
	if (IsInvalidData(pCtrl->bBackBuf, bLenth))
	{
		DTRACE(DB_CPS,("MtrTouChgTask : P%d data initial\r\n", wPn));
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
	}
	TraceBuf(DB_CPS, "MtrTouChgTask : current data -> ", bBuf,bLenth);
	TraceBuf(DB_CPS, "MtrTouChgTask : last    data -> ", pCtrl->bBackBuf,bLenth);
	//有变化
	if(!IsAlrEnable(0xE2000036))
	{
		DTRACE(DB_CPS,("MtrTouChgTask : Alr 0xE2000036 disable\r\n"));
		return false;
	}

	if (memcmp(bBuf, pCtrl->bBackBuf, bLenth) != 0)
	{
		DTRACE(DB_CPS, ("MtrTouChgTask : ############# P%d exception 0xE2000036 confirmed.\r\n",wPn));
		bDataIdNum = sizeof(g_wTouChgID)/sizeof(g_wTouChgID[0]);
		for(n=0; n<bDataIdNum; n++)
		{
			tDataID[n].wBn = BN0;
			tDataID[n].wID = g_wTouChgID[n];
			tDataID[n].wPn = wPn;
		}
		if (HandleAlr(0xE2000036, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
			memcpy(pCtrl->bBackBuf, bBuf, bLenth);
		else
			fChangeOk = false;
	}

	if (!fChangeOk)
	{
		return false;
	}

	return true;
}
//==================================电能表剩余电费不足(bAlrID = 0xE200002B)========================================================
static const WORD  g_wBuyEngLackID[] = {0x9010, 0x9020,0x9110,0x9120,0xc9b0};

//描述：电能表剩余电费不足告警的初始化。
void MtrBuyEngLackInit(WORD wPn, TMtrBuyEngLack* pCtrl)
{
	pCtrl->fExcValid = false;
	GetCurTime(&pCtrl->tmOccur);
}

//描述：电能表剩余电费不足告警判断。
bool MtrBuyEngLackTask(WORD wPn, TMtrBuyEngLack* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	WORD wMeterTimesIds[] = {0xc9b0};
	WORD bLenth = GetItemsLenId(wMeterTimesIds, sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]));
	BYTE bBuf[4] = {0};
	DWORD dwEngRemain = 0;
	DWORD dwEngLimit = 0;
	TBankItem tDataID[5] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_OVDEC,("MtrBuyEngLackTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_OVDEC,("MtrBuyEngLackTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	if(!IsBcdCode(bBuf,bLenth))
		return false;

	dwEngRemain = BcdToDWORD(bBuf, 4);//从电表读回来的时候已经与阀值参数对齐小数点了，在这里就不用再乘以100了
	ReadItemEx(BN0, PN0, 0x804B, bBuf);
	dwEngLimit = BcdToDWORD(bBuf, 3);

	DTRACE(DB_OVDEC,("MtrBuyEngLackTask : P%d data effective and dwEngRemain = %d dwEngLimit = %d\r\n", wPn, dwEngRemain, dwEngLimit));
	if(!IsAlrEnable(0xE200002B))
	{
		DTRACE(DB_OVDEC,("MtrBuyEngLackTask : Alr 0xE200002B disable\r\n"));
		return false;
	}

	if (dwEngRemain < dwEngLimit)
	{
		if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true)//每天上报一次
		{
			DTRACE(DB_OVDEC,("MtrBuyEngLackTask : P%d time for day is differ\r\n", wPn));
			pCtrl->tmOccur = now;
			pCtrl->fExcValid = false;
		}
		if (pCtrl->fExcValid == false)
		{
			DTRACE(DB_OVDEC, ("MtrBuyEngLackTask : ############# P%d exception 0xE200002B confirmed.\r\n",wPn));
			bDataIdNum = sizeof(g_wBuyEngLackID)/sizeof(g_wBuyEngLackID[0]);
			for(n=0; n<bDataIdNum; n++)
			{
				tDataID[n].wBn = BN0;
				tDataID[n].wID = g_wBuyEngLackID[n];
				tDataID[n].wPn = wPn;
			}
			if (HandleAlr(0xE200002B, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
			{
				pCtrl->fExcValid = true;
				pCtrl->tmOccur = now;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		pCtrl->fExcValid = false;
	}

	return true;
}
//==================================电能表停走(bAlrID = 0xE200002E)========================================================
static const WORD  g_wMeterStopID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//描述：电能表停走告警的初始化。
void MtrMeterStopInit(WORD wPn, TMtrMeterStop* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
	pCtrl->fExcValid = false;
	pCtrl->dwPLastClick = GetClick();
	pCtrl->dwLastMin = 0;
}

//描述：电能表停走告警判断。
bool MtrMeterStopTask(WORD wPn, TMtrMeterStop* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	WORD wMeterTimesIds[] = {0x9010, 0x9020, 0xb630};
	WORD bLenth = GetItemsLenId(wMeterTimesIds, sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]));
	BYTE bBuf[11] = {0};
	DWORD dwIncre = 0;
	DWORD dwCurClick = 0;
	DWORD dwPMeterAE, dwNMeterAE, dwBacPMeterAE, dwBacNMeterAE;
	DWORD dwDeltaPE = 0;
	BYTE bTmpBuf[2] = {0};
	TBankItem tDataID[18] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	ReadItemEx(BN0, PN0, 0x804A, bTmpBuf);
	dwIncre = BcdToDWORD(bTmpBuf, 2);
	if (dwIncre == 0)
		dwIncre = 10;

	if (!IsBcdCode(bBuf, bLenth))//无效数据
		return false;
	if(pCtrl->dwLastMin == dwCurMin)
	{
		DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d dwCurMin=%d current minute already done\r\n", wPn, dwCurMin));
		return true;
	}
	pCtrl->dwLastMin = dwCurMin;

	DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d data effective\r\n", wPn));
	dwCurClick = GetClick();

	if (!IsBcdCode(pCtrl->bBackBuf, bLenth))//无效数据
	{
		DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d data initial\r\n", wPn));
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
		pCtrl->dwPLastClick = dwCurClick;
		return true;
	}

	dwPMeterAE = BcdToDWORD(bBuf, 4);
	dwNMeterAE = BcdToDWORD(bBuf+4, 4);
	dwDeltaPE = BcdToDWORD(bBuf+8, 3) * (dwCurClick - pCtrl->dwPLastClick);

	dwBacPMeterAE = BcdToDWORD(pCtrl->bBackBuf, 4);
	dwBacNMeterAE = BcdToDWORD(pCtrl->bBackBuf+4, 4);

	DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d data effective and dwDeltaPE = %d dwIncre = %d \r\n", wPn, dwDeltaPE, dwIncre*100*3600));
	DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d data dwPMeterAE = %d dwBacPMeterAE = %d dwNMeterAE = %d dwBacNMeterAE = %d\r\n", wPn, dwPMeterAE, dwBacPMeterAE, dwNMeterAE, dwBacNMeterAE));

	if(!IsAlrEnable(0xE200002E))
	{
		DTRACE(DB_OVLOAD,("MtrMeterStopTask : Alr 0xE200002E disable\r\n"));
		return false;
	}

	if (dwDeltaPE>dwIncre*100*3600 && dwPMeterAE==dwBacPMeterAE && dwNMeterAE==dwBacNMeterAE)
	{
		if (pCtrl->fExcValid == false)
		{
			DTRACE(DB_OVLOAD, ("MtrMeterStopTask : ############# P%d exception 0xE200002E confirmed.\r\n",wPn));
			bDataIdNum = sizeof(g_wMeterStopID)/sizeof(g_wMeterStopID[0]);
			for(n=0; n<bDataIdNum; n++)
			{
				tDataID[n].wBn = BN0;
				tDataID[n].wID = g_wMeterStopID[n];
				tDataID[n].wPn = wPn;
			}
			if (HandleAlr(0xE200002E, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0))
			{
				memcpy(pCtrl->bBackBuf, bBuf, bLenth);
				pCtrl->fExcValid = true;
				pCtrl->dwPLastClick = dwCurClick;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else if(dwPMeterAE!=dwBacPMeterAE || dwNMeterAE!=dwBacNMeterAE)
	{
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
		pCtrl->fExcValid = false;
		pCtrl->dwPLastClick = dwCurClick;
	}

	return true;
}
//==================================电能表示度下降(bAlrID = 0xE200002C)========================================================
static const WORD  g_wEnergyDecID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f};

//描述：电能表示度下降告警的初始化。
void MtrEnergyDecInit(WORD wPn, TMtrEnergyDec* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
	pCtrl->fExcValid = false;
	memset(pCtrl->bPreBuf, 0, sizeof(pCtrl->bPreBuf));
	pCtrl->wDataLen = 0;
}

//描述：电能表示度下降告警判断。
bool MtrEnergyDecTask(WORD wPn, TMtrEnergyDec* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	WORD wEnergyDecPreID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f};
	WORD wMeterTimesIds[] = {0x9010, 0x9020};
	WORD bLenth = GetItemsLenId(wMeterTimesIds, sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]));
	BYTE bBuf[8] = {0};
	DWORD dwPMeterAE, dwNMeterAE, dwBacPMeterAE, dwBacNMeterAE;
	TBankItem tDataID[17] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_OVCPS,("MtrEnergyDecTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_OVCPS,("MtrEnergyDecTask : P%d  data arrive\r\n", wPn));
	TraceBuf(DB_OVCPS, "MtrEnergyDecTask : read data -> ", bBuf, sizeof(bBuf));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;

	if(!IsBcdCode(bBuf, bLenth))
		return false;

	if(!IsBcdCode(pCtrl->bBackBuf, bLenth))
	{
		DTRACE(DB_OVCPS,("MtrEnergyDecTask : P%d data initial  \r\n", wPn));
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
	}

	dwPMeterAE = BcdToDWORD(bBuf, 4);
	dwNMeterAE = BcdToDWORD(bBuf+4, 4);

	dwBacPMeterAE = BcdToDWORD(pCtrl->bBackBuf, 4);
	dwBacNMeterAE = BcdToDWORD(pCtrl->bBackBuf+4, 4);

	DTRACE(DB_OVCPS,("MtrEnergyDecTask : P%d data effective and dwPMeterAE = %d dwBacPMeterAE = %d \r\n", wPn, dwPMeterAE, dwBacPMeterAE));

	if(!IsAlrEnable(0xE200002C))
	{
		DTRACE(DB_OVCPS,("MtrEnergyDecTask : Alr 0xE200002C disable\r\n"));
		return false;
	}

	if (dwPMeterAE < dwBacPMeterAE)//正向有功
	{
		if (dwBacPMeterAE>99999000 && dwPMeterAE<1000)//电表正常走到满刻度掉头了
		{
			DTRACE(DB_OVCPS, ("MtrEnergyDecTask : wPn=%d,reach measuring range, last pae %ld, pae %ld.\r\n",wPn, dwBacPMeterAE, dwPMeterAE));
		}
		else
		{
			DTRACE(DB_OVCPS,("MtrEnergyDecTask : P%d data effective and pCtrl->fExcValid = %d\r\n", wPn, pCtrl->fExcValid));
			if (pCtrl->fExcValid == false)
			{
				DTRACE(DB_OVCPS, ("MtrEnergyDecTask : ############# P%d exception 0xE200002C-P confirmed.\r\n",wPn));
				bDataIdNum = sizeof(g_wEnergyDecID)/sizeof(g_wEnergyDecID[0]);
				for(n=0; n<bDataIdNum; n++)
				{
					tDataID[n].wBn = BN0;
					tDataID[n].wID = g_wEnergyDecID[n];
					tDataID[n].wPn = wPn;
				}
				HandleAlr(0xE200002C, wPn, tDataID, bDataIdNum, dwCurMin, now, pCtrl->bPreBuf, pCtrl->wDataLen);
				memcpy(pCtrl->bBackBuf, bBuf, bLenth);
				pCtrl->fExcValid = true;
			}
		}
	}

	DTRACE(DB_OVCPS,("MtrEnergyDecTask : P%d data effective and dwNMeterAE = %d dwBacNMeterAE = %d \r\n", wPn, dwNMeterAE, dwBacNMeterAE));
	if (dwNMeterAE < dwBacNMeterAE)//反向有功
	{
		if (dwBacNMeterAE>99999000 && dwNMeterAE<1000)//电表正常走到满刻度掉头了
		{
			DTRACE(DB_OVCPS, ("MtrEnergyDecTask : wPn=%d,reach measuring range, last nae %ld, nae %ld.\r\n",wPn, dwBacNMeterAE, dwNMeterAE));
		}
		else
		{
			DTRACE(DB_OVCPS,("MtrEnergyDecTask : P%d data effective and pCtrl->fExcValid = %d\r\n", wPn, pCtrl->fExcValid));
			if (pCtrl->fExcValid == false)
			{
				DTRACE(DB_OVCPS, ("MtrEnergyDecTask : ############# P%d exception 0xE200002C-N confirmed.\r\n",wPn));
				bDataIdNum = sizeof(g_wEnergyDecID)/sizeof(g_wEnergyDecID[0]);
				for(n=0; n<bDataIdNum; n++)
				{
					tDataID[n].wBn = BN0;
					tDataID[n].wID = g_wEnergyDecID[n];
					tDataID[n].wPn = wPn;
				}
				HandleAlr(0xE200002C, wPn, tDataID, bDataIdNum, dwCurMin, now, pCtrl->bPreBuf, pCtrl->wDataLen);
				memcpy(pCtrl->bBackBuf, bBuf, bLenth);
				pCtrl->fExcValid = true;
			}
		}
	}

	if (dwPMeterAE >= dwBacPMeterAE && dwNMeterAE >= dwBacNMeterAE)//都正常，置恢复
	{
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
		pCtrl->fExcValid = false;
	}

	bDataIdNum = sizeof(wEnergyDecPreID)/sizeof(wEnergyDecPreID[0]);
	for(n=0; n<bDataIdNum; n++)
	{
		tDataID[n].wBn = BN0;
		tDataID[n].wID = wEnergyDecPreID[n];
		tDataID[n].wPn = wPn;
	}
	pCtrl->wDataLen = ReadItemMbi(tDataID, bDataIdNum, pCtrl->bPreBuf, dwCurMin*60, INVALID_TIME);

	return true;
}
//==================================电能表时钟电池电压低(bAlrID = 0xE2000032)========================================================
//描述：电能表时钟电池电压低告警的初始化。
void MtrBatteryLowInit(WORD wPn, TMtrBatteryLow* pCtrl)
{
	pCtrl->fExcValid = false;
	memset(&pCtrl->tmOccur, 0, sizeof(pCtrl->tmOccur));
}

//描述：电能表时钟电池电压低告警判断。
bool MtrBatteryLowTask(WORD wPn, TMtrBatteryLow* pCtrl)
{	
	WORD wIdNum, wValidNum;
	int nRead;
	WORD *wIds ;
	DWORD dwCurMin;
	BYTE bMtrInterv;
	WORD wMeterTimesIds[] = {0xc860};
	WORD bLenth = GetItemLen(BN0, wMeterTimesIds[0]);
	BYTE bBuf[2] = {0};
	TBankItem tDataID[1] = { 0 };
	BYTE bDataIdNum = 0;
	TTime now;
	GetCurTime(&now);

	DTRACE(DB_DIFF,("MtrBatteryLowTask : P%d function running......\r\n", wPn));
	dwCurMin = GetCurMinute();
	bMtrInterv = GetMeterInterv(wPn);
	if (bMtrInterv == 0)
		return false;
	dwCurMin = dwCurMin / bMtrInterv * bMtrInterv;

	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	if (QueryItemTimeMid(dwCurMin*60, 0, BN0, wPn, wIds, wIdNum, bBuf, &wValidNum) != wIdNum)
		return true;

	DTRACE(DB_DIFF,("MtrBatteryLowTask : P%d  data arrive\r\n", wPn));
	nRead = ReadItemMid(BN0, wPn, wIds, wIdNum, bBuf, dwCurMin*60, INVALID_TIME);
	if (nRead <= 0)
		return false;
	if(IsInvalidData(bBuf, bLenth))
		return false;
	if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true)//每天上报一次
	{
		DTRACE(DB_DIFF,("MtrBatteryLowTask : P%d time for day is differ\r\n", wPn));
		pCtrl->tmOccur = now;
		pCtrl->fExcValid = false;
	}

	DTRACE(DB_DIFF,("MtrBatteryLowTask : P%d data effective and (bBuf[0]>>2)&0x01 = 0x%02x\r\n", wPn, (bBuf[0]>>2)&0x01));
	//有变化

	if(!IsAlrEnable(0xE2000032))
	{
		DTRACE(DB_DIFF,("MtrBatteryLowTask : Alr 0xE2000032 disable\r\n"));
		return false;
	}

	if ((bBuf[0]>>2)&0x01)
	{
		if (!pCtrl->fExcValid)
		{
			DTRACE(DB_DIFF, ("MtrBatteryLowTask : ############# P%d exception 0xE2000032 confirmed.\r\n",wPn));
			HandleAlr(0xE2000032, wPn, tDataID, bDataIdNum, dwCurMin, now, NULL, 0);
			pCtrl->tmOccur = now;
			pCtrl->fExcValid = true;
		}
	}
	else
		pCtrl->fExcValid = false;

	return true;
}

//============终端参数变更记录=================
bool IsTerminalParaChgRec(BYTE *pdwDI, DWORD dwLen, TTime tmTime)
{
	DWORD dwStartClick=0;
	BYTE bAlrBuf[ALRSECT_SIZE];
	BYTE* pbAlr = NULL;

	memset(bAlrBuf, 0, sizeof(bAlrBuf));

	if(pdwDI == NULL)
		return false;

	pbAlr = bAlrBuf + 7;//1个字节的CRC + 2个字节的Pn + 4个字节的ID
	pbAlr += TimeTo6Bcd(&tmTime,pbAlr);
	memcpy(pbAlr,pdwDI,dwLen);

	dwStartClick = GetClick();
	DTRACE(DB_FAPROTO, ("IsTerminalParaChgRec: ########## Start Save Event happen at Click=%ld, ID=0xE2010014.\r\n", dwStartClick));
	SaveAlrData(0xE2010014,PN0,bAlrBuf,dwLen+BYTE_DATE_TIME);
	DTRACE(DB_FAPROTO, ("IsTerminalParaChgRec: ########## End Save EventEvent at Click=%ld, wDeltaClick=%ld.\r\n", GetClick(), GetClick()-dwStartClick));

	return true;
}

//描述：各告警事件控制结构的初始化，
//		在上电后或者参数重新配置后调用本函数，不用每次都调用
void InitMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl)
{
	MtrCVMissTimesChgInit(wPn, &pCtrl->tMtrCVLess);	//失压

	MtrCIMissTimesChgInit(wPn, &pCtrl->tMtrCIMisss);//失流

	MtrCIRevsTimesChgInit(wPn, &pCtrl->tMtrCIRevs);	//潮流反向

	MtrPrgTmChgInit(wPn, &pCtrl->tMtrPrgTm);		//编程时间更改

	MtrPwStaChgInit(wPn, &pCtrl->tMtrPwSta);		//继电器变位

	MtrClockErrChgInit(wPn, &pCtrl->tMtrClockErr);	//时钟异常

	MtrTouChgInit(wPn, &pCtrl->tMtrTou);			//时段或费率更改

	MtrBuyEngLackInit(wPn, &pCtrl->tMtrBuyEngLack);	//剩余电费不足

	MtrMeterStopInit(wPn, &pCtrl->tMtrMeterStop);	//电能表停走

	MtrEnergyDecInit(wPn, &pCtrl->tMtrEnergyDec);	//示度下降

	MtrBatteryLowInit(wPn, &pCtrl->tMtrBatteryLow);	//时钟电池电压低
}


bool DoMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl)
{
// 	if(GetErcType(ERC_VMISS) != 0)
		MtrCVMissTimesChgTask(wPn, &pCtrl->tMtrCVLess);	//失压

// 	if(GetErcType(ERC_IMISS) != 0)
		MtrCIMissTimesChgTask(wPn, &pCtrl->tMtrCIMisss);//失流

// 	if(GetErcType(ERC_FLOW_REVERSE) != 0)
		MtrCIRevsTimesChgTask(wPn, &pCtrl->tMtrCIRevs);	//潮流反向

// 	if(GetErcType(ERC_SETTIME) != 0)
		MtrPrgTmChgTask(wPn, &pCtrl->tMtrPrgTm);		//编程时间更改

// 	if(GetErcType(ERC_MTRSTATUSCHG) != 0)
		MtrPwStaChgTask(wPn, &pCtrl->tMtrPwSta);		//继电器变位

// 	if(GetErcType(ERC_MTRTIME) != 0)
		MtrClockErrChgTask(wPn, &pCtrl->tMtrClockErr);	//时钟异常

// 	if(GetErcType(ERC_ENGDEC) != 0)
		MtrTouChgTask(wPn, &pCtrl->tMtrTou);			//时段或费率更改

// 	if(GetErcType(ERC_ENGOVER) != 0)
		MtrBuyEngLackTask(wPn, &pCtrl->tMtrBuyEngLack);	//剩余电费不足

// 	if(GetErcType(ERC_MTRSTOP) != 0)
		MtrMeterStopTask(wPn, &pCtrl->tMtrMeterStop);	//电能表停走

// 	if(GetErcType(ERC_MTRFLEW) != 0)
		MtrEnergyDecTask(wPn, &pCtrl->tMtrEnergyDec);	//示度下降

// 	if(GetErcType(ERC_ENGDEC) != 0)
		MtrBatteryLowTask(wPn, &pCtrl->tMtrBatteryLow);	//时钟电池电压低

	return true;
}
