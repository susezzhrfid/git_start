 /*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrExc.c
 * ժ    Ҫ��ʵ�ָ澯�¼����жϣ��洢��
 *
 * ��    ��: 1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
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

TMtrAlrCtrl g_MtrAlrCtrl[DYN_PN_NUM];	//���澯���ƽṹ


//�������жϱ��ַ�Ƿ���ȷ�����ַ����Ϊ0����
//���أ������ַ��ֵΪ0�� ����false;��֮������true;
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
//****************�������¼�*****************************
//==================================���ܱ�ʧѹ(bAlrID = 0xE2000016~0xE2000018)========================================================
static const WORD g_wMeterVoltageID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//���������ܱ�ʧѹ�澯�ĳ�ʼ����
void MtrCVMissTimesChgInit(WORD wPn, TMtrCVLess* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//���������ܱ�ʧѹ�澯�жϡ�
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
	//�б仯
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
//==================================���ܱ�ʧ��(bAlrID = 0xE2000013~0xE2000015)========================================================
static const WORD  g_wCurrentID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//���������ܱ�ʧ���澯�ĳ�ʼ����
void MtrCIMissTimesChgInit(WORD wPn, TMtrCIMisss* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//���������ܱ�ʧ���澯�жϡ�
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
	//�б仯
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
//==================================���ܱ�������(bAlrID = 0xE200000D~0xE200000F)========================================================
static const WORD  g_wIRevsChgID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//���������ܱ�������澯�ĳ�ʼ����
void MtrCIRevsTimesChgInit(WORD wPn, TMtrCIRevs* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//���������ܱ�������澯�жϡ�
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
	//�б仯
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
//==================================���ܱ���ʱ�����(bAlrID = 0xE2000035)========================================================
static const WORD  g_wPrgTmChgID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//���������ܱ���ʱ����ĸ澯�ĳ�ʼ����
void MtrPrgTmChgInit(WORD wPn, TMtrPrgTm* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//���������ܱ���ʱ����ĸ澯�жϡ�
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
	//�б仯
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
//==================================���ܱ�̵�����λ(bAlrID = 0xE200003B)========================================================
static const WORD  g_wPwStaChgID[] = {0xc862};

//���������ܱ�̵�����λ�澯�ĳ�ʼ����
void MtrPwStaChgInit(WORD wPn, TMtrPwSta* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//���������ܱ�̵�����λ�澯�жϡ�
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
	//�б仯
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
//==================================���ܱ�����բ����ʧ��(bAlrID = 0xE200003C)========================================================
//============�����¼���¼============
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

		dwRdSec = GetCurIntervSec(wPn, &time);   //���ϵ�ǰ������ʱ��
		ReadItemTm(BN0, wPn, wTeminalCtrEvnID[n], bBuf+iLen, dwRdSec, INVALID_TIME);	//û���������ݶ��Ѿ��ó���Ч����
		iLen += GetItemLen(BN0,wTeminalCtrEvnID[n]);
	}

	pbAlr = bAlrBuf + 7;//1���ֽڵ�CRC + 2���ֽڵ�Pn + 4���ֽڵ�ID
	pbAlr += TimeTo6Bcd(&time,pbAlr);
	*pbAlr++ = bEventType;
	memcpy(pbAlr,bBuf,wLen-BYTE_DATE_TIME-SG_BYTE_TYPE_EVENT);

	SaveAlrData(0xE201000E, wPn, bAlrBuf, wLen);
	DTRACE(DB_EXC4, ("MtrRelayControlEventRec: ########## Event happen, ID=0xE201000E\r\n"));

	return true;
}

//���������ܱ�����բ����ʧ�ܸ澯�жϡ�
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
	TYkCtrl tYkCtrl;//����������Ŀ���
	TTime now = { 0 };
	GetCurTime(&now);

	ExFlashReadPnCtrlPara(wPn, &tYkCtrl);
	DTRACE(DB_EXC4,("MtrRelayTask : P%d function running and tYkCtrl.wYkPn = %d\r\n", wPn, tYkCtrl.wYkPn));
	if (wPn != tYkCtrl.wYkPn)//�������Ƿ���Ч
		return true;

	ReadItemEx(BN0,wPn,0x8902,bCtrlMtrAddr);
	if(memcmp(tYkCtrl.bYkMtrAddr,bCtrlMtrAddr,6) != 0)//���������仯
	{
		DTRACE(DB_EXC4,("MtrRelayTask : P%d meter address change\r\n", wPn, tYkCtrl.wYkPn));
		TraceBuf(DB_EXC4, "MtrRelayTask : current address -> ", bCtrlMtrAddr,6);
		TraceBuf(DB_EXC4, "MtrRelayTask : Yk address -> ", tYkCtrl.bYkMtrAddr,6);
		return true;
	}

	memcpy(bData, tYkCtrl.bYkOptPwd, 4);
	memcpy(bData+4, tYkCtrl.bYKOptCode, 4);
	bData[8] = tYkCtrl.bYKCtrlType;
	bData[9] = 0;	//N2����
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
			memcpy(bRetFrm, &bRetFrm[bPos], bRetLen-bPos); 	//ȥ��ǰ���ַ�

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

	GetDirRdCtrl(bPort);	//ȡ��ֱ���Ŀ���Ȩ
	DoDirMtrRd(BN0, wPn, wMeterTimesIds[0], now);
	ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ

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
		HandleAlr(0xE200003B, wPn, tDataID, bDataIdNum, 0, now, NULL, 0);//�̵�����λ
		ReadItemEx(BN0,wPn,wMeterTimesIds[0],bBuf);
		TraceBuf(DB_EXC4, "MtrRelayTask : 0xE200003B current data -> ", bBuf,bLenth);
		TraceBuf(DB_EXC4, "MtrRelayTask : 0xE200003B last data -> ", pCtrl->bBackBuf,bLenth);
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
	}
	else
	{
		DTRACE(DB_EXC4, ("MtrRelayTask : ############# P%d exception 0xE200003C confirmed.\r\n",wPn));
		HandleAlr(0xE200003C, wPn, tDataID, bDataIdNum, 0, now, NULL, 0);//����բʧ��
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
	tYkCtrl.wYkPn = 0;//�������Ч������
	ExFlashWritePnCtrlPara(wPn, &tYkCtrl, sizeof(TYkCtrl));
	
	return true;
}
//==================================���ܱ�ʱ���쳣(bAlrID = 0xE200003E)========================================================
static const WORD  g_wClockErrChgID[] = {0xc010, 0xc011, 0x8030};

//���������ܱ�ʱ���쳣�澯�ĳ�ʼ����
void MtrClockErrChgInit(WORD wPn, TMtrClockErr* pCtrl)
{
	pCtrl->fExcValid = false;
	GetCurTime(&pCtrl->tmOccur);
	pCtrl->dwLastMin = 0;
}

//���������ܱ�ʱ���쳣�澯�жϡ�
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

	if(!IsBcdCode(bBuf,bLenth))//��Ч����
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
	bClock = bBuf[0];		//ʱ�����ʱ��
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

	if (dwErr > bClock*60)   //������10����, 5
	{
		if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && true==pCtrl->fExcValid)//ÿ���ϱ�һ��
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
//==================================���ܱ�ʱ�λ���ʸ���(bAlrID = 0xE2000036)========================================================
static const WORD  g_wTouChgID[] = {0xc33f, 0xc32f};

//���������ܱ�ʱ�λ���ʸ��ĸ澯�ĳ�ʼ����
void MtrTouChgInit(WORD wPn, TMtrTou* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
}

//���������ܱ�ʱ�λ���ʸ��ĸ澯�жϡ�
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

	if(!IsBcdCode(bBuf,bLenth))//��Ч����
		return false;

	DTRACE(DB_CPS,("MtrTouChgTask : P%d data effective\r\n", wPn));
	if (IsInvalidData(pCtrl->bBackBuf, bLenth))
	{
		DTRACE(DB_CPS,("MtrTouChgTask : P%d data initial\r\n", wPn));
		memcpy(pCtrl->bBackBuf, bBuf, bLenth);
	}
	TraceBuf(DB_CPS, "MtrTouChgTask : current data -> ", bBuf,bLenth);
	TraceBuf(DB_CPS, "MtrTouChgTask : last    data -> ", pCtrl->bBackBuf,bLenth);
	//�б仯
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
//==================================���ܱ�ʣ���Ѳ���(bAlrID = 0xE200002B)========================================================
static const WORD  g_wBuyEngLackID[] = {0x9010, 0x9020,0x9110,0x9120,0xc9b0};

//���������ܱ�ʣ���Ѳ���澯�ĳ�ʼ����
void MtrBuyEngLackInit(WORD wPn, TMtrBuyEngLack* pCtrl)
{
	pCtrl->fExcValid = false;
	GetCurTime(&pCtrl->tmOccur);
}

//���������ܱ�ʣ���Ѳ���澯�жϡ�
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

	dwEngRemain = BcdToDWORD(bBuf, 4);//�ӵ���������ʱ���Ѿ��뷧ֵ��������С�����ˣ�������Ͳ����ٳ���100��
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
		if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true)//ÿ���ϱ�һ��
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
//==================================���ܱ�ͣ��(bAlrID = 0xE200002E)========================================================
static const WORD  g_wMeterStopID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};

//���������ܱ�ͣ�߸澯�ĳ�ʼ����
void MtrMeterStopInit(WORD wPn, TMtrMeterStop* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
	pCtrl->fExcValid = false;
	pCtrl->dwPLastClick = GetClick();
	pCtrl->dwLastMin = 0;
}

//���������ܱ�ͣ�߸澯�жϡ�
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

	if (!IsBcdCode(bBuf, bLenth))//��Ч����
		return false;
	if(pCtrl->dwLastMin == dwCurMin)
	{
		DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d dwCurMin=%d current minute already done\r\n", wPn, dwCurMin));
		return true;
	}
	pCtrl->dwLastMin = dwCurMin;

	DTRACE(DB_OVLOAD,("MtrMeterStopTask : P%d data effective\r\n", wPn));
	dwCurClick = GetClick();

	if (!IsBcdCode(pCtrl->bBackBuf, bLenth))//��Ч����
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
//==================================���ܱ�ʾ���½�(bAlrID = 0xE200002C)========================================================
static const WORD  g_wEnergyDecID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f};

//���������ܱ�ʾ���½��澯�ĳ�ʼ����
void MtrEnergyDecInit(WORD wPn, TMtrEnergyDec* pCtrl)
{
	memset(pCtrl->bBackBuf, INVALID_DATA, sizeof(pCtrl->bBackBuf));
	pCtrl->fExcValid = false;
	memset(pCtrl->bPreBuf, 0, sizeof(pCtrl->bPreBuf));
	pCtrl->wDataLen = 0;
}

//���������ܱ�ʾ���½��澯�жϡ�
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

	if (dwPMeterAE < dwBacPMeterAE)//�����й�
	{
		if (dwBacPMeterAE>99999000 && dwPMeterAE<1000)//��������ߵ����̶ȵ�ͷ��
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
	if (dwNMeterAE < dwBacNMeterAE)//�����й�
	{
		if (dwBacNMeterAE>99999000 && dwNMeterAE<1000)//��������ߵ����̶ȵ�ͷ��
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

	if (dwPMeterAE >= dwBacPMeterAE && dwNMeterAE >= dwBacNMeterAE)//���������ûָ�
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
//==================================���ܱ�ʱ�ӵ�ص�ѹ��(bAlrID = 0xE2000032)========================================================
//���������ܱ�ʱ�ӵ�ص�ѹ�͸澯�ĳ�ʼ����
void MtrBatteryLowInit(WORD wPn, TMtrBatteryLow* pCtrl)
{
	pCtrl->fExcValid = false;
	memset(&pCtrl->tmOccur, 0, sizeof(pCtrl->tmOccur));
}

//���������ܱ�ʱ�ӵ�ص�ѹ�͸澯�жϡ�
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
	if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true)//ÿ���ϱ�һ��
	{
		DTRACE(DB_DIFF,("MtrBatteryLowTask : P%d time for day is differ\r\n", wPn));
		pCtrl->tmOccur = now;
		pCtrl->fExcValid = false;
	}

	DTRACE(DB_DIFF,("MtrBatteryLowTask : P%d data effective and (bBuf[0]>>2)&0x01 = 0x%02x\r\n", wPn, (bBuf[0]>>2)&0x01));
	//�б仯

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

//============�ն˲��������¼=================
bool IsTerminalParaChgRec(BYTE *pdwDI, DWORD dwLen, TTime tmTime)
{
	DWORD dwStartClick=0;
	BYTE bAlrBuf[ALRSECT_SIZE];
	BYTE* pbAlr = NULL;

	memset(bAlrBuf, 0, sizeof(bAlrBuf));

	if(pdwDI == NULL)
		return false;

	pbAlr = bAlrBuf + 7;//1���ֽڵ�CRC + 2���ֽڵ�Pn + 4���ֽڵ�ID
	pbAlr += TimeTo6Bcd(&tmTime,pbAlr);
	memcpy(pbAlr,pdwDI,dwLen);

	dwStartClick = GetClick();
	DTRACE(DB_FAPROTO, ("IsTerminalParaChgRec: ########## Start Save Event happen at Click=%ld, ID=0xE2010014.\r\n", dwStartClick));
	SaveAlrData(0xE2010014,PN0,bAlrBuf,dwLen+BYTE_DATE_TIME);
	DTRACE(DB_FAPROTO, ("IsTerminalParaChgRec: ########## End Save EventEvent at Click=%ld, wDeltaClick=%ld.\r\n", GetClick(), GetClick()-dwStartClick));

	return true;
}

//���������澯�¼����ƽṹ�ĳ�ʼ����
//		���ϵ����߲����������ú���ñ�����������ÿ�ζ�����
void InitMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl)
{
	MtrCVMissTimesChgInit(wPn, &pCtrl->tMtrCVLess);	//ʧѹ

	MtrCIMissTimesChgInit(wPn, &pCtrl->tMtrCIMisss);//ʧ��

	MtrCIRevsTimesChgInit(wPn, &pCtrl->tMtrCIRevs);	//��������

	MtrPrgTmChgInit(wPn, &pCtrl->tMtrPrgTm);		//���ʱ�����

	MtrPwStaChgInit(wPn, &pCtrl->tMtrPwSta);		//�̵�����λ

	MtrClockErrChgInit(wPn, &pCtrl->tMtrClockErr);	//ʱ���쳣

	MtrTouChgInit(wPn, &pCtrl->tMtrTou);			//ʱ�λ���ʸ���

	MtrBuyEngLackInit(wPn, &pCtrl->tMtrBuyEngLack);	//ʣ���Ѳ���

	MtrMeterStopInit(wPn, &pCtrl->tMtrMeterStop);	//���ܱ�ͣ��

	MtrEnergyDecInit(wPn, &pCtrl->tMtrEnergyDec);	//ʾ���½�

	MtrBatteryLowInit(wPn, &pCtrl->tMtrBatteryLow);	//ʱ�ӵ�ص�ѹ��
}


bool DoMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl)
{
// 	if(GetErcType(ERC_VMISS) != 0)
		MtrCVMissTimesChgTask(wPn, &pCtrl->tMtrCVLess);	//ʧѹ

// 	if(GetErcType(ERC_IMISS) != 0)
		MtrCIMissTimesChgTask(wPn, &pCtrl->tMtrCIMisss);//ʧ��

// 	if(GetErcType(ERC_FLOW_REVERSE) != 0)
		MtrCIRevsTimesChgTask(wPn, &pCtrl->tMtrCIRevs);	//��������

// 	if(GetErcType(ERC_SETTIME) != 0)
		MtrPrgTmChgTask(wPn, &pCtrl->tMtrPrgTm);		//���ʱ�����

// 	if(GetErcType(ERC_MTRSTATUSCHG) != 0)
		MtrPwStaChgTask(wPn, &pCtrl->tMtrPwSta);		//�̵�����λ

// 	if(GetErcType(ERC_MTRTIME) != 0)
		MtrClockErrChgTask(wPn, &pCtrl->tMtrClockErr);	//ʱ���쳣

// 	if(GetErcType(ERC_ENGDEC) != 0)
		MtrTouChgTask(wPn, &pCtrl->tMtrTou);			//ʱ�λ���ʸ���

// 	if(GetErcType(ERC_ENGOVER) != 0)
		MtrBuyEngLackTask(wPn, &pCtrl->tMtrBuyEngLack);	//ʣ���Ѳ���

// 	if(GetErcType(ERC_MTRSTOP) != 0)
		MtrMeterStopTask(wPn, &pCtrl->tMtrMeterStop);	//���ܱ�ͣ��

// 	if(GetErcType(ERC_MTRFLEW) != 0)
		MtrEnergyDecTask(wPn, &pCtrl->tMtrEnergyDec);	//ʾ���½�

// 	if(GetErcType(ERC_ENGDEC) != 0)
		MtrBatteryLowTask(wPn, &pCtrl->tMtrBatteryLow);	//ʱ�ӵ�ص�ѹ��

	return true;
}
