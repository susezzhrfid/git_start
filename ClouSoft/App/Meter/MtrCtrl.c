/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֵ��ĳ������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��1��
 *********************************************************************************************************/
#include "FaAPI.h"
#include "MtrAPI.h"
#include "MtrStruct.h"
#include "MtrCtrl.h"
#include "MtrFmt.h"
#include "MtrHook.h"
#include "MtrCfg.h"
#include "LibDbAPI.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "MtrProAPI.h"
#include "FaAPI.h"
#include "MtrExc.h"
#include "CommonTask.h"
#include "DbAPI.h"
#include "FlashMgr.h"
#include "ExcTask.h"
#include "MtrAPIEx.h"
#include "ProEx.h"
#include "SysApi.h"
#include  "SearchMeter.h"
#include  "DbCfg.h"
#include "DrvCfg.h"
#include  "DbGbAPI.h"
#include "DbFmt.h"
#include "BatMtrTask.h"
#include "AutoReader.h"
#include "DoTask.h"
////////////////////////////////////////////////////////////////////////////////////////////
//MtrCtrl˽�к궨��


////////////////////////////////////////////////////////////////////////////////////////////
//MtrCtrl˽�����ݶ���
const WORD g_wMtrId[] = 
{
	0x9010, 0x901f, 0x9020, 0x902f, 0x907f, 0x908f, 0x9110, 0x911f, 0x9120, 0x912f, 
	0x913f, 0x914f, 0x915f, 0x916f, 0x917f, 0x918f, 0x941f, 0x942f, 0x951f, 0x952f,
	0x953f, 0x954f, 0x955f, 0x956f, 0x957f, 0x958f, 0xa01f, 0xa02f, 0xa11f, 0xa12f,
	0xa13f, 0xa14f, 0xa15f, 0xa16f, 0xa41f, 0xa42f, 0xa51f, 0xa52f, 0xa53f, 0xa54f,
	0xa55f, 0xa56f, 0xb01f, 0xb02f, 0xb11f, 0xb12f, 0xb13f, 0xb14f, 0xb15f, 0xb16f,
	0xb210, 0xb211, 0xb212, 0xb213, 0xb214, 0xb21f, 0xb310, 0xb311, 0xb312, 0xb313,
	0xb31f, 0xb320, 0xb321, 0xb322, 0xb323, 0xb32f, 0xb330, 0xb331, 0xb332, 0xb333,
	0xb33f, 0xb340, 0xb341, 0xb342, 0xb343, 0xb34f, 0xb41f, 0xb42f, 0xb51f, 0xb52f,
	0xb53f, 0xb54f, 0xb55f, 0xb56f, 0xb611, 0xb612, 0xb613, 0xb61f, 0xb621, 0xb622,
	0xb623, 0xb62f, 0xb630, 0xb631, 0xb632, 0xb633, 0xb63f, 0xb640, 0xb641, 0xb642,
	0xb643, 0xb64f, 0xb650, 0xb651, 0xb652, 0xb653, 0xb65f, 0xb670, 0xb671, 0xb672,
	0xb673, 0xb67f, 0xb6a0, 0xc010, 0xc011, 0xc01f, 0xc020, 0xc021, 0xc022, 0xc02f,
	0xc030, 0xc031, 0xc032, 0xc033, 0xc034, 0xc111, 0xc112, 0xc117, 0xc313, 0xc331,
	0xc332, 0xc333, 0xc334, 0xc335, 0xc336, 0xc337, 0xc338, 0xc33f, 0xc60f, 0xc700,
	0xc701, 0xc70f, 0xc800,

	0x9a00, 0x9a1f, 0x9a2f, 0x9b1f, 0x9b2f, 0x9b3f, 0x9b4f, 0x9b5f, 0x9b6f, 0x9c0f,
	0x9c1f, 0x9c2f, 0x9c3f, 0x9c8f, 0x9c9f, 0x9caf, 0x9cbf, 0xc810, 0xc811, 0xc812,
	0xc813, 0xc814, 0xc815, 0xc820, 0xc821, 0xc830, 0xc831, 0xc840, 0xc841, 0xc850,
	/*0xc851,*/ 0xc860, 0xc861, 0xc862, 0xc863, 0xc864, 0xc865, 0xc866, 0xc86f, 0xc870,
	0xc871, 0xc872, 0xc873, 0xc880, 0xc881, 0xc882, 0xc883, 0xc884, 0xc885, 0xc886,
	0xc88f, 0xc990, 0xc991, 0xc9a0, 0xc9a1, 0xc9a2, 0xc9a3, 0xc9a4, 0xc9a5, 0xc9b0,
	0xc9b1, 0xc9c0, 0xc9c1, 0xc9c2, 0xc9c3, 0xc9c4, 0xc9c5, 0xc9d0, 0xc9d1, 0xc9d2,
	0xc9e0, 0xc9e1, 0xc9e2, 0xd040, 0xea14, 0xea16, 0xea18, 0xea40, 0xea42, 0xea44,
    0xea60, 0xea62, 0xea64,
};	

#define MTR_UNSUP_ID_NUM	(sizeof(g_wMtrId)/sizeof(WORD))
#define MTR_UNSUP_ID_SIZE	((MTR_UNSUP_ID_NUM+7)/8)
BYTE g_bUnsupIdFlg[DYN_PN_NUM][MTR_UNSUP_ID_SIZE] = {0};	//���֧��ID��־λ

extern BYTE g_bSect3Data[DYN_PN_NUM][SECT3_DATA_SZ]; 		//��SEC����RAM����
extern DWORD g_dwSect3IntervTime[DYN_PN_NUM][2];			//��BANK�ĵ�ǰ���
extern BYTE g_bSect3TimeFlg[DYN_PN_NUM][SECT3_TMFLG_SZ];	//��BANK���ݵĸ���ʱ���־
WORD g_wPnDataChk[DYN_PN_NUM];	//����ֱ��1������ֻ������������ݲ���ʱУ����

DWORD g_dwComTaskRecTm[DYN_PN_NUM][64];				//��ͨ�����м�����
BYTE g_ProfFrzFlg[DYN_PN_NUM][30];					//���߶����־λ
TMtrSaveInf g_MtrSaveInf[DYN_PN_NUM];
BYTE g_bMtrAddr[DYN_PN_NUM][6];		//����ַ�����ڵ������������ú������ϵ磬ͨ������ַ���ж��Ƿ񻻱���
						//��û����������ͨ��IsMtrParaChg()�жϱ�־λ����
//BYTE g_bMtrRdTm[DYN_PN_NUM][3]; //���һ�γ���ʱ��������

BYTE g_bDayMtrStatV[DYN_PN_NUM][66+27];//������ƽ��ֵ�������ʱ����
BYTE g_bMonMtrStatV[DYN_PN_NUM][66+27];
TTime g_tDayMtrStatLtm[DYN_PN_NUM];
DWORD g_dwTotalV[DYN_PN_NUM][6];//��ƽ����ѹʱ���ۼƵ��ӵ�ѹ
WORD  g_wTotalTimes[DYN_PN_NUM][6];//��ƽ����ѹʱ���ۼƵ���ͳ�ƴ���

BYTE g_bDayUnbalanceVI[DYN_PN_NUM][14];
BYTE g_bDayDemandStat[DYN_PN_NUM][24];
BYTE g_bMonDemandStat[DYN_PN_NUM][24];
//TTime g_bMonMtrStatLtm[DYN_PN_NUM];
BYTE g_bLastRdMtrData[DYN_PN_NUM][14]; //���һ�γ���ɹ���ʱ���ʾֵ��������ʧ���¼�ʹ��
BYTE g_bCurIntvWrFlg[DYN_PN_NUM]; //��ǰ���д��־0:��һ��д1:ȫ����ȫд

#define PNTMP_NUM	20//11	//��������/��������

const TPnTmp g_PnTmp[DYN_PN_NUM][PNTMP_NUM] = 
{
	//������0
	{
		{g_bSect3Data[0], SECT3_DATA_SZ},		//����������
		{(BYTE* )g_dwSect3IntervTime[0], 8},	//������ʱ��
		{g_bSect3TimeFlg[0], SECT3_TMFLG_SZ},	//����ʱ���־
		{(BYTE* )&g_wPnDataChk[0], 2},			//����ֱ��1������ֻ������������ݲ���ʱУ����
	
		{(BYTE* )&g_dwComTaskRecTm[0], sizeof(g_dwComTaskRecTm)/DYN_PN_NUM},	//��ͨ�����м�����
		{g_ProfFrzFlg[0], sizeof(g_ProfFrzFlg)/DYN_PN_NUM},			//���߶����־λ
		{g_bUnsupIdFlg[0], sizeof(g_bUnsupIdFlg)/DYN_PN_NUM},			//���֧��ID��־λ
		{(BYTE *)&g_MtrSaveInf[0], sizeof(TMtrSaveInf)},	//�������Ϣ
		{(BYTE *)&g_MtrAlrCtrl[0], sizeof(TMtrAlrCtrl)},	//�澯�м�����
		{g_bMtrAddr[0], 6},				//����ַ
		/////////////////������ĵ�ѹͳ��//////////////////////////
		{g_bDayMtrStatV[0], sizeof(g_bDayMtrStatV)/DYN_PN_NUM},
		{g_bMonMtrStatV[0], sizeof(g_bMonMtrStatV)/DYN_PN_NUM},
		{(BYTE *)&g_tDayMtrStatLtm[0],sizeof(TTime)},
		{(BYTE *)g_dwTotalV[0], sizeof(g_dwTotalV)/DYN_PN_NUM},
		{(BYTE *)g_wTotalTimes[0], sizeof(g_wTotalTimes)/DYN_PN_NUM},
		
		/////////////////������ĵ�ѹ������ƽ��////////////////////
		{g_bDayUnbalanceVI[0], sizeof(g_bDayUnbalanceVI)/DYN_PN_NUM},
		/////////////////��������������////////////////////////////
		{g_bDayDemandStat[0], sizeof(g_bDayDemandStat)/DYN_PN_NUM},
		{g_bMonDemandStat[0], sizeof(g_bMonDemandStat)/DYN_PN_NUM},
		//{g_bMonMtrStatLtm[0],sizeof(TTime)},
		//{g_bMtrRdTm[0], 3},				//���һ�γ���ʱ��
		{g_bLastRdMtrData[0], sizeof(g_bLastRdMtrData)/DYN_PN_NUM},
		{&g_bCurIntvWrFlg[0], sizeof(g_bCurIntvWrFlg)/DYN_PN_NUM},
	},

	//������1
	{
		{g_bSect3Data[1], SECT3_DATA_SZ},		//����������
		{(BYTE* )g_dwSect3IntervTime[1], 8},	//������ʱ��
		{g_bSect3TimeFlg[1], SECT3_TMFLG_SZ},	//����ʱ���־
		{(BYTE* )&g_wPnDataChk[1], 2},			//����ֱ��1������ֻ������������ݲ���ʱУ����
	
		{(BYTE* )&g_dwComTaskRecTm[1], sizeof(g_dwComTaskRecTm)/DYN_PN_NUM},	//��ͨ�����м�����
		{g_ProfFrzFlg[1], sizeof(g_ProfFrzFlg)/DYN_PN_NUM},			//���߶����־λ
		{g_bUnsupIdFlg[1], sizeof(g_bUnsupIdFlg)/DYN_PN_NUM},			//���֧��ID��־λ
		{(BYTE *)&g_MtrSaveInf[1], sizeof(TMtrSaveInf)},	//�������Ϣ
		{(BYTE *)&g_MtrAlrCtrl[1], sizeof(TMtrAlrCtrl)},	//�澯�м�����
		{g_bMtrAddr[1], 6},				//����ַ
		{g_bDayMtrStatV[1], sizeof(g_bDayMtrStatV)/DYN_PN_NUM},
		{g_bMonMtrStatV[1], sizeof(g_bMonMtrStatV)/DYN_PN_NUM},
		{(BYTE *)&g_tDayMtrStatLtm[1],sizeof(TTime)},
		{(BYTE *)g_dwTotalV[1], sizeof(g_dwTotalV)/DYN_PN_NUM},
		{(BYTE *)g_wTotalTimes[1], sizeof(g_wTotalTimes)/DYN_PN_NUM},
		{g_bDayUnbalanceVI[1], sizeof(g_bDayUnbalanceVI)/DYN_PN_NUM},
		/////////////////��������������////////////////////////////
		{g_bDayDemandStat[1], sizeof(g_bDayDemandStat)/DYN_PN_NUM},
		{g_bMonDemandStat[1], sizeof(g_bMonDemandStat)/DYN_PN_NUM},
		//{g_bMonMtrStatLtm[1],sizeof(TTime)},
		//{g_bMtrRdTm[1], 3},				//���һ�γ���ʱ��
		{g_bLastRdMtrData[1], sizeof(g_bLastRdMtrData)/DYN_PN_NUM},
		{&g_bCurIntvWrFlg[1], sizeof(g_bCurIntvWrFlg)/DYN_PN_NUM},
	},

};

#define PNDAT_NUM	4		//ֱ��Ҫ����Ĳ�����������Ŀ����

WORD g_wCurPn[DYN_PN_NUM];	//��ǰ���뵽�ڴ�Ĳ�����
TSem g_semMtrCtrl;		//��������̼߳���ź���
TSem g_semRdMtr[DYN_PN_NUM];
bool g_fDirRd = false;	//ֱ����־
BYTE g_bDirRdStep = 0;	//1�����ݳ���״̬ 1�����ڳ���  0��û��
TMtrPara g_MtrPara[DYN_PN_NUM];
DWORD g_dwLastIntervSec[DYN_PN_NUM];
BYTE g_bPnFailCnt[REAL_PN_NUM];
BYTE g_bPnFailFlg[REAL_PN_MASK_SIZE];
BYTE g_bMtrRdStatus[REAL_PN_MASK_SIZE];
BYTE g_bMtrRdStep[DYN_PN_NUM];	//������������״̬�� 1���յ������������ 2�����ڳ��� 0���Ѿ�����

BYTE g_b485PortStatus;//485�����״̬:0����,1���Ϸ���,2���ϻָ�
bool g_f485FailHapFlg;
BYTE g_bPortOfThrd[DYN_PN_NUM];		//�̵߳�ǰʹ�õĳ���˿�

void  LockReader() { WaitSemaphore(g_semRdMtr[1], SYS_TO_INFINITE); };

void  UnLockReader() { SignalSemaphore(g_semRdMtr[1]); };

BYTE* GetPnTmpData(WORD wNum, BYTE bThrId)
{
	if(wNum == 11)
	{
		return g_bDayMtrStatV[bThrId];
	}
	else if( wNum == 12)
	{
		return g_bMonMtrStatV[bThrId];
	}
	else
	{
		return NULL;
	}
}


//����:�ж��������Ƿ��Ѿ�����
//����:���귵��1�����򷵻�0
int QueryItemDone(WORD wPn, WORD wID, DWORD dwStartTime, DWORD dwEndTime, BYTE* pbBuf)
{
	WORD wValidNum = 0;
	int iConfirmNum = 0;

	TBankItem BankItem;
	BankItem.wBn = BN0;	//BANK���ȶ�Ϊ�๦�ܱ�,���涳���ʱ��CctQueryItemTime()���Զ���̬����
	BankItem.wPn = wPn;
	BankItem.wID = wID;
	iConfirmNum = QueryItemTimeMbi(dwStartTime, dwEndTime, &BankItem, 1, pbBuf, &wValidNum);

	if (iConfirmNum>0 || iConfirmNum<0)
		return 1;
	else
		return 0;
}

//����:�����û����ʱ��պ÷����򷵻�0,����Ѿ��������򷵻�1,
//	   ���ʱ��û���򷵻�-1
int ReadTimeOut(WORD wPn, const TMtrRdCtrl* pRdCtrl, TTime* pTm, BYTE* pbBuf)
{
	DWORD dwCurTime;
	DWORD dwStartTime;
	DWORD dwEndTime;

	if (GetItemTimeScope(wPn, pRdCtrl, pTm, &dwStartTime, &dwEndTime)) //ȡ�ü����������ȡ��ʱ�䷶Χ(��ʼʱ��~����ʱ��)
	{
		dwCurTime = TimeToSeconds(pTm);
		if (dwCurTime < dwStartTime)
			return -1;

		return QueryItemDone(wPn, pRdCtrl->wID, dwStartTime, dwEndTime, pbBuf);
	}
	else
	{
		return 1;	//��֧�ֵĶ�������1��ʾ����,��÷����س�
	}
}

//����:��ѯ������ĳ���״̬
//����:@rCctRdCtrl ����ѯ������
//	   @tmNow ��ǰʱ��
// 	   @pbBuf ��ϵͳ����õĻ���
//����:�����������Ŀǰ��Ҫ������δ����ʱ��պ÷����򷵻�0,����Ѿ��������򷵻�1,�����Ҫ����ʱ�仹û���򷵻�-1,
//	   �������������ڸõ����˵����Ҫ���򷵻�-2�������������-3
int QueryItemReadStatus(WORD wPn, const TMtrRdCtrl* pRdCtrl, TTime* pTm, BYTE* pbBuf)
{
	//TTime time;
	int iRet = IsIdNeedToRead(wPn, pRdCtrl);
	
	if (iRet != 0)	//Ŀǰ����Ҫ���Ķ�����----�����������Ŀǰ��Ҫ������δ����ʱ��պ÷����򷵻�0
		return iRet;	//���ó���������س���

	//Ŀǰ07/97���Ҫ�����������MtrCfg.c�ļ���ֱ�����úã�������Ҫ���ڲ�ID���ⲿID��ת��

	//return ReadTimeOut(wPn, pRdCtrl, pTm, pbBuf);
	iRet = ReadTimeOut(wPn, pRdCtrl, pTm, pbBuf);
	//if (0 == iRet && TIME_UNIT_HOUR == pRdCtrl->bIntervU)
	//{
	//	GetCurTime(&time);
	//	DTRACE(DB_METER, ("IsIdNeedToRead: TIME_UNIT_HOUR  wPn=%d, wId=%2x, time= %d-%d-%d-%d-%d-%d \r\n", wPn, pRdCtrl->wID, time.nYear, time.nMonth, time.nDay, time.nHour, time.nMinute, time.nSecond));
	//}
	return iRet;
}



//�ж�ʱ���Ƿ�����ǰ������
DWORD GetCurIntervSec(WORD wPn, TTime* ptmNow)
{
	DWORD dwMin = TimeToMinutes(ptmNow);
	BYTE bInterv = GetMeterInterv(wPn);
    
    if (bInterv == 0)
        return 0;

	return dwMin / bInterv * bInterv * 60;
}

//���������ò����㲻֧�ָ�ID�ı�־
void SetMtrUnsupId(WORD wPn, WORD wID, BYTE bThrId)
{
	WORD i;
	BYTE bMask;

	for (i=0; i<MTR_UNSUP_ID_NUM; i++)
	{
		if (wID == g_wMtrId[i])
		{	
			bMask = 1 << (i&7);
			if ((g_bUnsupIdFlg[bThrId][i>>3] & bMask) == 0)
			{
				g_bUnsupIdFlg[bThrId][i>>3] |= bMask;
				DTRACE(DB_METER, ("SetMtrUnsupId: wPn=%d, wID=0x%04x \r\n", wPn, wID));
			}

			return;
		}
	}
}

//�������жϸò������Ƿ�֧�ָ�ID
bool IsMtrUnsupId(WORD wPn, WORD wID, BYTE bThrId)
{
	WORD i;

	for (i=0; i<MTR_UNSUP_ID_NUM; i++)
	{
		if (wID == g_wMtrId[i])
		{	
			if (g_bUnsupIdFlg[bThrId][i>>3] & (1<<(i&7)))
			{
				DTRACE(DB_MTRX, ("IsMtrUnsupId: wPn=%d, unSupport wID=0x%04x!\r\n", wPn, wID));
				return true;			
			}
			else			
			{
				return false;			
			}
		}
	}

	return false;
}

//����������ò���������ID�Ƿ�֧�ֵı�־
void ResetMtrUnsupIdFlg(WORD wPn)
{
	memset(g_bUnsupIdFlg, 0, MTR_UNSUP_ID_SIZE);
}


//�����������������Ƿ��Ѿ�������ڴ�
bool IsPnDataLoaded(WORD wPn)
{
	BYTE i;
	for (i=0; i<DYN_PN_NUM; i++)
	{
		if (wPn == g_wCurPn[i])
			return true;
	}

	return false;
}

BYTE g_bPortInUseFlg = 0;	//�˿�ռ�ñ�־λ
bool SwDebugPortFun()
{
	BYTE bThrId, bPortFun = PORT_FUN_DEBUG;
	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
	WaitSemaphore(g_semMtrCtrl, SYS_TO_INFINITE);
	for (bThrId=0; bThrId<DYN_PN_NUM; bThrId++)
	{
		if (LOGIC_PORT_MAX == g_bPortOfThrd[bThrId])
		{
			g_bPortOfThrd[bThrId] = 0xff;
			g_bPortInUseFlg &= ~(1<<(LOGIC_PORT_MAX-LOGIC_PORT_MIN));		//���ԭ����ռ�ñ�־
			break;
		}
	}
	InitDebug();
	if (bPortFun == PORT_FUN_DEBUG)
	{
		CommClose(COMM_DEBUG);
		CommOpen(COMM_DEBUG, 9600, 8, ONESTOPBIT, EVENPARITY);
	}
	SignalSemaphore(g_semMtrCtrl);

	return true;
}

//�������л������̵߳Ķ˿ڵ�һ��û����ʹ�õĶ˿���
//���أ��л����Ķ˿ں�
BYTE SwPortOfThrd(BYTE bThrId)
{
	BYTE i, bOldPort, bPortFun = PORT_FUN_RDMTR;
	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
	if (GetInfo(INFO_PORT_FUN))
	{
		SwDebugPortFun();
	}
	WaitSemaphore(g_semMtrCtrl, SYS_TO_INFINITE);
	bOldPort = g_bPortOfThrd[bThrId];

	for (i=0; i<LOGIC_PORT_NUM; i++)
	{
		if (i==LOGIC_PORT_NUM-1 && bPortFun==PORT_FUN_DEBUG)
			continue;
		if ((g_bPortInUseFlg & (1<<i)) == 0) //û��ʹ�õĶ˿�
		{
			g_bPortOfThrd[bThrId] = LOGIC_PORT_MIN + i;
			if (bOldPort != 0xff)
				g_bPortInUseFlg &= ~(1<<(bOldPort-LOGIC_PORT_MIN));		//���ԭ����ռ�ñ�־
			
			g_bPortInUseFlg |= 1<<i;	//�����µ�ռ�ñ�־
			
			break;
		}
	}

	if (i==LOGIC_PORT_NUM && bPortFun!=PORT_FUN_DEBUG)
		g_bPortOfThrd[bThrId] = 0xff;

	SignalSemaphore(g_semMtrCtrl);

	return g_bPortOfThrd[bThrId];
}


//������ȡ�ò��������ڵ��߳�
BYTE GetPnThread(WORD wPn)
{
	BYTE bThrId;
	BYTE bPort = GetPnPort(wPn);

	if (PORT_CCT_PLC == bPort || PORT_CCT_WIRELESS == bPort)
	{
		if (!g_fCctInitOk)
		{
			return 0xff;
		}

		return (DYN_PN_NUM-1);//�ز��߳� �̶���bThrId=1
	}

	if (GetInfo(INFO_PORT_FUN))
	{
		SwDebugPortFun();
	}

	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return 0xff;

	return 0;	//485�߳� �̶���bThrId=0

	/*WaitSemaphore(g_semMtrCtrl, SYS_TO_INFINITE);

	for (bThrId=0; bThrId<DYN_PN_NUM; bThrId++)
	{
		if (bPort == g_bPortOfThrd[bThrId])
		{
			SignalSemaphore(g_semMtrCtrl);
			return bThrId;
		}
	}
	
	SignalSemaphore(g_semMtrCtrl);

	return 0xff;*/
}

//��������ռһ���̣߳�����ֱ��������£����������Ķ˿ڻ�û�б�ʹ�ã���ǿ������һ���̵߳Ķ˿�
BYTE GrabPnThread(WORD wPn)
{
	BYTE bThrId, bOldPort;
	BYTE bPort = GetPnPort(wPn);
	if (PORT_CCT_PLC == bPort || PORT_CCT_WIRELESS == bPort)
	{
		if (!g_fCctInitOk)
		{
			return 0xff;
		}
		return DYN_PN_NUM - 1;//�ز��߳�
	}

	if (GetInfo(INFO_PORT_FUN))
	{
		SwDebugPortFun();
	}
	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return 0xff;

	WaitSemaphore(g_semMtrCtrl, SYS_TO_INFINITE);

	//�˿��Ƿ��Ѿ����߳�ʹ��
	/*for (bThrId=0; bThrId<DYN_PN_NUM; bThrId++)
	{
		if (bPort == g_bPortOfThrd[bThrId])
		{
			SignalSemaphore(g_semMtrCtrl);
			return bThrId;
		}
	}

	//���Ƿ��п���û�õ�
	for (bThrId=0; bThrId<DYN_PN_NUM; bThrId++)
	{
		if (g_bPortOfThrd[bThrId] == 0xff)
		{
			g_bPortOfThrd[bThrId] = bPort;
			g_bPortInUseFlg |= 1<<(bPort-LOGIC_PORT_MIN);	//�����µ�ռ�ñ�־

			SignalSemaphore(g_semMtrCtrl);
			return bThrId;
		}
	}*/

	//ǿ�Ƹ��߳�0
	bOldPort = g_bPortOfThrd[0];
	g_bPortOfThrd[0] = bPort;
	g_bPortInUseFlg &= ~(1<<(bOldPort-LOGIC_PORT_MIN));		//���ԭ����ռ�ñ�־
	g_bPortInUseFlg |= 1<<(bPort-LOGIC_PORT_MIN);	//�����µ�ռ�ñ�־

	SignalSemaphore(g_semMtrCtrl);
	return 0;	//�����߳�0
}


//���������������ֱ������
bool LoadPnDirData(WORD wPn)
{
	WORD wPnDataChk;
	BYTE bThrId = GrabPnThread(wPn);
	if (bThrId >= DYN_PN_NUM)
        return false;

	DTRACE(DB_METER, ("LoadPnDirData: wPn=%d\r\n", wPn));
	if (!LoadPnData(wPn, g_PnTmp[bThrId], PNDAT_NUM))
	{	
		DTRACE(DB_METER, ("LoadPnDirData: fail! use default. wPn=%d\r\n", wPn));
		ClrPnTmp(g_PnTmp[bThrId], PNDAT_NUM);
	}

	wPnDataChk = CheckPnData(g_PnTmp[bThrId], PNDAT_NUM-1);
	if (wPnDataChk != g_wPnDataChk[bThrId])
	{	
		DTRACE(DB_METER, ("LoadPnDirData: fail! use default. wPn=%d\r\n", wPn));
		ClrPnTmp(g_PnTmp[bThrId], PNDAT_NUM);
	}

	g_wCurPn[bThrId] = wPn;
	return true;
}


//������ȡ��ֱ���Ŀ���Ȩ
void GetDirRdCtrl(BYTE bPort)
{
	BYTE bThrId;
	if((bPort == PORT_CCT_PLC) || (bPort == PORT_CCT_WIRELESS))
		bThrId = 1;
	else
		bThrId = 0;

	
	g_fDirRd = true;
	WaitSemaphore(g_semRdMtr[bThrId], SYS_TO_INFINITE);

	DTRACE(DB_METER, ("GetDirRdCtrl: bThrId=%d---\r\n", bThrId));
}

//�������ͷ�ֱ���Ŀ���Ȩ
void ReleaseDirRdCtrl(BYTE bPort)
{
	BYTE bThrId;
	if((bPort == PORT_CCT_PLC) || (bPort == PORT_CCT_WIRELESS))
		bThrId = 1;
	else
		bThrId = 0;

	g_fDirRd = false;
	SignalSemaphore(g_semRdMtr[bThrId]);	
	DTRACE(DB_METER, ("ReleaseDirRdCtrl: bThrId=%d---\r\n", bThrId));
}

//�ն˳������ʵ��
void DoPortRdErr(bool fMtrFailHap)
{
    TTime tmNow;
    if (fMtrFailHap)   //��������ʧ��
    {
        if (g_b485PortStatus == 0)
            g_b485PortStatus = 1;
    }
    else
    {
        if (g_b485PortStatus == 1)
            g_b485PortStatus = 2;
    }
    
    if (g_b485PortStatus == 1 && !g_f485FailHapFlg)     //IsAllPnRdFail()
	{
//		BYTE bErr = 4;
		GetCurTime(&tmNow);
// 		SaveAlrData(ERC_TERMERR, tmNow, &bErr, 0, 0);
		g_f485FailHapFlg = true;
	}
	else if (g_b485PortStatus == 2)
	{
		g_b485PortStatus = 0;
		g_f485FailHapFlg = false;
	}
}

//�������Զ���һ��������Ҫ���������һ��
//���أ�����������
BYTE AutoReadPn(struct TMtrPro* pMtrPro, WORD wPn, DWORD dwCurIntervSec, BYTE bThrId, bool* pfModified, BYTE bRdCnt)
{
	int iIdx, iItemStatus, iRet;
	DWORD dwIntervSec;
	WORD i, wID, wTestId=0x9010, wItemNum;
	const TMtrRdCtrl* pRdCtrl = MtrGetRdCtrl(&wItemNum);
	TTime now;
	char str[10];
	bool fUnfinish = false;
	BYTE bPos, bMask;
	BYTE bBuf[160];
	BYTE bRdErNum = 3;

#if MTRPNMAP!=PNUNMAP
	iIdx = SearchPnMap(MTRPNMAP, wPn);
	if (iIdx < 0)
		return RD_ERR_PARACHG;		//���������
#else
	iIdx = wPn;
#endif

	bPos = iIdx>>3;
	bMask = 1<<(iIdx & 7);

	//if (IsPowerOff())	//ͣ��
		//return RD_ERR_PWROFF;

	if (g_bPnFailFlg[bPos] & bMask) //�����ͨ�Ź���
	{
		if (pMtrPro->pMtrPara->bProId != PROTOCOLNO_DLT645)
			wTestId = 0x901f;
		if (AskMtrItem(pMtrPro, wPn, wTestId, bBuf) > 0)	//��wTestId���Ա�����Ƿ�ָ�
		{
			g_bPnFailCnt[iIdx] = 0;	
			g_bPnFailFlg[bPos] &= ~bMask;
 			dwIntervSec = GetCurIntervSec(wPn, &now);
			iRet = AskMtrItem(pMtrPro, wPn, 0x9010, bBuf);
			if (iRet > 0)
				SaveMeterItem(wPn, 0x9010, bBuf, iRet, dwIntervSec, ERR_APP_OK);

			iRet = AskMtrItem(pMtrPro, wPn, 0x9110, bBuf);
			if (iRet > 0)
				SaveMeterItem(wPn, 0x9110, bBuf, iRet, dwIntervSec, ERR_APP_OK);

			OnMtrErrRecv(wPn);		//������ϻָ�
			DoPortRdErr(false);
		}
		else
		{
			return RD_ERR_485;	//��Ȼ����485�������
		}
	}

	//ֻ���ڵ��û�й��ϵ�ʱ�������ʵʱ����
	for (i=0; i<wItemNum; i++)
	{
	  	if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//ֱ����־
			return RD_ERR_DIR;		//����ֱ��
		if (g_fStopMtrRd)
  			return RD_ERR_STOPRD;		//����ֱ��

		//�ж�ʱ���Ƿ�����ǰ���������������˾ͽ���������2�����ݶ��ᡢ�澯�жϵ�������ִ��һ��
		GetCurTime(&now);

		if (IsMtrParaChg(wPn))	//�����仯
			return RD_ERR_PARACHG;		//���������

		dwIntervSec = GetCurIntervSec(wPn, &now);
		if (dwIntervSec != dwCurIntervSec)	//�����������ı�
			return RD_ERR_INTVCHG;		//���������

		iItemStatus = QueryItemReadStatus(wPn, pRdCtrl+i, &now, bBuf);
		if (iItemStatus == 0)	//�����������Ŀǰ��Ҫ������δ���򷵻�0
		{
			//if (IsPowerOff())	//ͣ������ܳ������������Ҫ����
				//return RD_ERR_PWROFF;
			DTRACE(DB_METER, ("AutoReadPn: Rd ---> Pn=%d, wID=%x, i=%d, bIntervU=%s\r\n", wPn, pRdCtrl[i].wID, i, IntervUToStr(pRdCtrl[i].bIntervU, str)));

			wID = pRdCtrl[i].wID;
			if (IsMtrUnsupId(wPn, wID, bThrId))
				iRet = -1;	//��֧�ֵ�������
			else
				iRet = AskMtrItem(pMtrPro, wPn, wID, bBuf);

			if (iRet > 0)	//��������
			{
				if (SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_APP_OK) >= 0) //9a00ʱ�겻�Բ������޸ı�־�����򲹳�9a00�ᵼ��һֱдFlash
					*pfModified = true; //�������������޸ģ���Ҫ���浽�ⲿFLASH
				g_bPnFailCnt[iIdx] = 0;
			}
			else if (iRet == -1) //��֧�ֵ�������
			{
				SetMtrUnsupId(wPn, wID, bThrId);
				SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
			}
			else if (iRet == -4) //07��645������ͨ�ŷ�����������Ŀǰ�޴˼�¼
			{
				SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
			}
			else if(!IsPowerOff()) //ûͣ�������³���ʧ�ܣ�Ҫͬʱ�ж�485����
			{
				if (IsCctPn(wPn))
					bRdErNum = 1;
				else
					bRdErNum = 3;

				g_bPnFailCnt[iIdx]++;
				if (g_bPnFailCnt[iIdx] >= bRdErNum)
				{
					g_bPnFailCnt[iIdx] = bRdErNum;
					if (pMtrPro->pMtrPara->bProId != PROTOCOLNO_DLT645)
						wTestId = 0x901f;

					if (IsCctPn(wPn))//�ز�����ֱ����Ϊ����
					{
						if ((g_bPnFailFlg[bPos] & bMask) == 0)
						{
							g_bPnFailFlg[bPos] |= bMask;
							OnMtrErrEstb(wPn, g_bLastRdMtrData[bThrId]);		//�������ȷ��
							DoPortRdErr(true);      //�ն˳������
						}

						return RD_ERR_485;	//����485�������
					}
					else//485������
					{
						if (pMtrPro->pfnAskItem(pMtrPro, wTestId, bBuf) <= 0) //����ʧ�ܻ�Ҫ��һ��9010ȷ��һ��,������ЩID�ĸ���
						{
							if ((g_bPnFailFlg[bPos] & bMask) == 0)
							{
								g_bPnFailFlg[bPos] |= bMask;
								OnMtrErrEstb(wPn, g_bLastRdMtrData[bThrId]);		//�������ȷ��
								DoPortRdErr(true);      //�ն˳������
							}

							return RD_ERR_485;	//����485�������
						}
						else
						{
							fUnfinish = true;
							g_bPnFailCnt[iIdx] = 0;
						}
					}
				}
				else
				{
					fUnfinish = true;
				}
			}
			else  //ͣ����,�������㱾��ʣ�µĲ�����
			{
				return RD_ERR_PWROFF;
			}
		}
		else if (iItemStatus == -1)	//�����Ҫ����ʱ�仹û���򷵻�-1,
		{
			fUnfinish = true;
		}
		//else ����Ѿ��������򷵻�1,�������������ڸõ����˵����Ҫ���򷵻�-2�������������-3
		//if (i==1 && bRdCnt==0)
		//	return RD_ERR_UNFIN;	//û���꣬�������ȳ���
	}

	if (fUnfinish)
		return RD_ERR_UNFIN;	//û����
	else
		return RD_ERR_OK;		//�޴�����ȫ����
}

/*
//�������ڳ������Զ���������������������1������
BYTE AutoReadRptClass1(struct TMtrPro* pMtrPro, WORD wPn, DWORD dwCurIntervSec, BYTE* pbFnFlg, BYTE bThrId, bool* pfModified)
{
	const WORD* pwSubID;
	int iIdx, iRet, iLen;
	DWORD dwIntervSec;
	WORD wBN, wPN, wID, wSubID;		//ת����ı�ʶ
	WORD wTestId = 0x9010;
	TTime now;
	BYTE bVarLen = false;	//�䳤
	BYTE bFn, fn, bPos, bMask;
	BYTE bBuf[120];

	if (IsAllAByte(pbFnFlg, 0, 32))
		return RD_ERR_OK;

#if MTRPNMAP!=PNUNMAP
	iIdx = SearchPnMap(MTRPNMAP, wPn);
	if (iIdx < 0)
		return RD_ERR_PARACHG;		//���������
#else
	iIdx = wPn;
#endif

	bPos = iIdx>>3;
	bMask = 1<<(iIdx & 7);

	//if (IsPowerOff())	//ͣ��
		//return RD_ERR_PWROFF;

	if (g_bPnFailFlg[bPos] & bMask) //�����ͨ�Ź���
	{
		if (pMtrPro->pMtrPara->bProId != PROTOCOLNO_DLT645)
			wTestId = 0x901f;
		if (AskMtrItem(pMtrPro, wPn, wTestId, bBuf) > 0)	//��wTestId���Ա�����Ƿ�ָ�
		{
			g_bPnFailCnt[iIdx] = 0;	
			g_bPnFailFlg[bPos] &= ~bMask;
			dwIntervSec = GetCurIntervSec(wPn, &now);
			iRet = AskMtrItem(pMtrPro, wPn, 0x9010, bBuf);
			if (iRet > 0)
				SaveMeterItem(wPn, 0x9010, bBuf, iRet, dwIntervSec, ERR_APP_OK);

			iRet = AskMtrItem(pMtrPro, wPn, 0x9011, bBuf);
			if (iRet > 0)
				SaveMeterItem(wPn, 0x9011, bBuf, iRet, dwIntervSec, ERR_APP_OK);

			OnMtrErrRecv(wPn);		//������ϻָ�
			DoPortRdErr(false);
		}
		else
		{
			return RD_ERR_485;	//��Ȼ����485�������
		}
	}

	//dwStartTime = GetCurSec() / 3600 * 3600;
	//dwEndTime = dwStartTime + 3600;

	for (bFn=1; bFn<181; bFn++)
	{
	
		fn = bFn - 1;
		if ((pbFnFlg[fn>>3] & (1<<(fn&7))) == 0)
			continue;

		if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//ֱ����־
			return RD_ERR_DIR;		//����ֱ��
		if (g_fStopMtrRd)
  			return RD_ERR_STOPRD;		//����ֱ��

		//�ж�ʱ���Ƿ�����ǰ���������������˾ͽ���������2�����ݶ��ᡢ�澯�жϵ�������ִ��һ��
		GetCurTime(&now);

		if (IsMtrParaChg(wPn))	//�����仯
			return RD_ERR_PARACHG;		//���������

		dwIntervSec = GetCurIntervSec(wPn, &now);
		if (dwIntervSec != dwCurIntervSec)	//�����������ı�
			return RD_ERR_INTVCHG;		//���������

		if (IfOKConvertFNtoID(GB_DATACLASS1, bFn, wPn, &wBN, &wPN, &wID, &bVarLen) == false)
			return RD_ERR_OK;
	
		iLen = GetItemLen(wBN, wID);
		if (iLen <= 0)
			return RD_ERR_OK;
	
		pwSubID = Bank0To645ID(wID);
		if (pwSubID == NULL)
		{	
			if (!QueryItemDone(wPn, wID, dwIntervSec, dwIntervSec+3600, bBuf))
			{
				DTRACE(DB_METER, ("AutoReadRptClass1: wPn=%d, wID=0x%04x\r\n", wPn, wID));
				iRet = AskMtrItem(pMtrPro, wPn, wID, bBuf);
				if (iRet > 0)	//��������	
				{
					SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
					*pfModified = true; //�������������޸ģ���Ҫ���浽�ⲿFLASH
				}
				else if (iRet == -1) //��֧�ֵ�������
				{
					SetMtrUnsupId(wPn, wID, bThrId);
					SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
					*pfModified = true; //�������������޸ģ���Ҫ���浽�ⲿFLASH
				}
			}
		}
		else
		{
			while ((wSubID=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
			{
				if (!QueryItemDone(wPn, wSubID, dwIntervSec, dwIntervSec+3600, bBuf))
				{
					DTRACE(DB_METER, ("AutoReadRptClass1: wPn=%d, wID=0x%04x\r\n", wPn, wSubID));
					iRet = AskMtrItem(pMtrPro, wPn, wSubID, bBuf);
					if (iRet > 0)	//��������	
					{
						SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
						*pfModified = true; //�������������޸ģ���Ҫ���浽�ⲿFLASH
					}
					else if (iRet == -1) //��֧�ֵ�������
					{
						SetMtrUnsupId(wPn, wSubID, bThrId);
						SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
						*pfModified = true; //�������������޸ģ���Ҫ���浽�ⲿFLASH
					}
				}
			}
		}
	}

	return RD_ERR_OK;
}*/

void UpdateMtrRdStep(BYTE bThrId)
{
	switch (g_bMtrRdStep[bThrId])
	{
		case 1:		//�յ�������������
			DTRACE(DB_METER, ("MtrRdThread: start to direct rd mtr.\r\n"));
			//memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//����ɱ�־���
			g_bMtrRdStep[bThrId] = 2;
			break;
		case 2:		//����״̬
			DTRACE(DB_METER, ("MtrRdThread: finish direct rd mtr.\r\n"));			
			g_bMtrRdStep[bThrId] = 0;
			break;
		default:	//����״̬
			g_bMtrRdStep[bThrId] = 0;
			break;
	}
}

void MtrCtrlInit()
{
	BYTE i;

	g_semMtrCtrl = NewSemaphore(1, 1);

	for (i=0; i<DYN_PN_NUM; i++)
	{
		g_semRdMtr[i] = NewSemaphore(1, 1);
	}

	memset(&g_bPortOfThrd, 0xff, sizeof(g_bPortOfThrd));		//�̵߳�ǰʹ�õĳ���˿�
	memset(&g_wCurPn, 0, sizeof(g_wCurPn));
	memset(&g_bMtrRdStep, 0, sizeof(g_bMtrRdStep));

	memset(g_dwLastIntervSec, 0, sizeof(g_dwLastIntervSec));
	memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));
	memset(g_bPnFailCnt, 0, sizeof(g_bPnFailCnt));
	memset(g_bPnFailFlg, 0, sizeof(g_bPnFailFlg));
  	memset(&g_b485PortStatus, 0, sizeof(g_b485PortStatus));
	g_f485FailHapFlg = false;

	for (i=0; i<LOGIC_PORT_NUM; i++)
		InitSearch(i, 0);
    //for (wPn=1; wPn<PN_NUM; wPn++)
        //InitMtrExc(wPn, &g_MtrAlrCtrl);
}

bool IsAutoSendAdmit(void)
{
	BYTE bAdmitRpt;
	ReadItemEx(BN0, PN0, 0x103f, &bAdmitRpt);
	if ((bAdmitRpt&0x03) == 1)	
		return true;
	else
		return false;
}

//��������Բ���������������̵߳ĵ�ǰ������
//���أ���������㲻���̵߳�ǰ����˿ڵĲ������򷵻�NULL,��ȷ���ض�Ӧ������ĵ��Э��
struct TMtrPro* SetupThrdForPn(BYTE bThrId, WORD wPn)
{
	if (!IsMtrPn(wPn))
		return NULL;

	if (GetPnThread(wPn) != bThrId)  //���Ǳ��˿ڵĵ��ܱ�
		return NULL;

	//SetDynPn(bThrId, wPn);	//����ϵͳ�⣬����ò����������
								//Ŀǰ�������漰��ϵͳ�⣬���Բ�������

	memset(g_bMtrAddr[bThrId], 0, 6);
	GetMeterPara(wPn, &g_MtrPara[bThrId]);

	memset(&g_MtrSaveInf[bThrId], 0, sizeof(g_MtrSaveInf[bThrId]));

	return CreateMtrPro(wPn, &g_MtrPara[bThrId], &g_MtrSaveInf[bThrId], false, bThrId);
}

//����:ȡ�õ�ǰ�ĳ���״̬
BYTE GetRdMtrState(BYTE bThrId)
{
	if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//ֱ����־
		return RD_ERR_DIR;		//����ֱ��
	if (g_fStopMtrRd)
		return RD_ERR_STOPRD;	//����ֱ��

	return RD_ERR_OK;
}

//�����������̣߳�ִ�г���2�����ݶ��ᡢ�澯�жϵ�����
TThreadRet MtrRdThread(void* pvPara)
{
	struct TMtrPro* pMtrPro;
	int iIdx;
	int Vip_Num;
	DWORD dwCurIntervSec/*, dwStartSchClick = 0*/;
	WORD i, wPn;
	TTime now;
	BYTE bThrId, bLastDay;
	BYTE bRdErr, bPos, bMask;
	bool fRes, fMtrChg, fModified, fNeedToSave, fInfo;
	static DWORD dwUpdFatClick = 0;
	BYTE fStart = 0;   //ÿBIT��ʾһ���˿�
	BYTE fStop = 0;
	BYTE bMode = 0;
	bool fHaveRd, fSuccOnce;		//���в������Ƿ���������
	BYTE bPortSn = 0, bSearch = 0;
	BYTE bRdCnt = 0, bReRdType = 0;
//	BYTE bFnFlg[32]; //1�������������������ڱ�������������Ҫ����Fn
	BYTE bInit[2] = {0, 0};//�ϵ��ʼ��
	BYTE bSchStep[MTR_PORT_NUM];
	BYTE bType, bVip;
	memset(bSchStep, 1, sizeof(bSchStep));
	GetCurTime(&now);
	bLastDay = now.nDay;
	bThrId = (BYTE ) pvPara;

	InitMtrReRd();
	SwPortOfThrd(bThrId);	//�л������̵߳Ķ˿ڵ�һ��û����ʹ�õĶ˿���
	DTRACE(DB_METER, ("MtrRdThread: bThrId=%d start.\r\n", bThrId));
	while (1)
	{
		Vip_Num = 0;
		fHaveRd = false;	//���в������Ƿ���������

		//DoTask(bThrId);//��ͨ������м�����

	//    WaitSemaphore(g_semRdMtr[bThrId], SYS_TO_INFINITE);
		for (wPn=1; wPn<PN_NUM; wPn++)
		{
			//NOTE��
			//�����п������������̻߳�ͬʱ���ã���UpdFat()�������ź���TdbWaitSemaphore()������
			//����������
			if (1 == bThrId)
				fInfo = GetInfo(INFO_TASK_PARA);	// || GetInfo(INFO_METER_PARA)
			else
				fInfo = false;

			//fInfo = GetInfo(INFO_TASK_PARA);	// || GetInfo(INFO_METER_PARA)
			if (fInfo || //2��F39���÷����ı�
				(!g_fUpdFatOk && GetClick()-dwUpdFatClick>60*60))
			{
				g_fUpdFatOk = UpdFat();		//����Flash��̬���䣬һ��Ҫ����InitDB֮��
				dwUpdFatClick = GetClick();
	
				if (fInfo)
				{
					SetInfo(INFO_TASK_INIT);	 //֪ͨ�����̣߳�F39���øı�
					memset(g_dwLastIntervSec, 0, sizeof(g_dwLastIntervSec));
					memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));
                    if (g_fStopMtrRd)
                    {
                        if (IsCctPn(PN9))	//���̨������޸�
                            StopMtrRd(12);      //12
                        else
                            StopMtrRd(18);      //12
                    }
                    //g_fStopMtrRd = false;   //������
				}
			}
			//Sleep(20);
			if (g_fDirRd || g_fStopMtrRd || g_bMtrRdStep[bThrId]==1)	//ֱ����־ �� ������������
				break;
			
			if(1==bThrId && !IsCctPn(wPn))	//�߳�1���ز��̣߳�������������õĲ����ز��˿ھ�continue
				continue;

			if(0==bThrId && IsCctPn(wPn))	//�߳�0��485�̣߳�������������õ����ز��˿ھ�continue
				continue;

			MtrRelayTask(wPn, &g_MtrAlrCtrl[bThrId].tMtrPwSta);			//����բ����ʧ��
          	SwPortOfThrd(bThrId);	//�л������̵߳Ķ˿ڵ�һ��û����ʹ�õĶ˿���
			if (!IsMtrPn(wPn))
				continue;

			if (GetPnThread(wPn) != bThrId)  //���Ǳ��˿ڵĵ��ܱ�
				continue;

			SetDynPn(bThrId, wPn);	//����ϵͳ�⣬����ò����������
		
	#if MTRPNMAP!=PNUNMAP
			iIdx = SearchPnMap(MTRPNMAP, wPn);
			if (iIdx < 0)
				continue;
	#else
			iIdx = wPn;
	#endif
	
			fMtrChg = false;
			bPos = iIdx>>3;
			bMask = 1 << (iIdx & 7);
	
			GetCurTime(&now);
			dwCurIntervSec = GetCurIntervSec(wPn, &now);
			//if (dwCurIntervSec!=g_dwLastIntervSec[bThrId] || GetInfo(INFO_C1FN_CFG) || GetInfo(INFO_C2FN_CFG))
			if (dwCurIntervSec!=g_dwLastIntervSec[bThrId])
			{	//�����������л�				|| F38���øı�			  || F39���øı�
				g_dwLastIntervSec[bThrId] = dwCurIntervSec;
				memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//����ɱ�־���
				DoMtrRdStat();
				DTRACE(DB_METER, ("MtrRdThread: clr rd flg for interval or para change, cur pn=%d's\r\n", wPn));
			}
	
			//if (IsAutoSendAdmit())   //�����ϱ��Ƿ�����
			//	DoAutoSend();	//�ϱ�һ������ʱ����ֱ����ʽ�����ȴ��ź���g_semRdMtr[bThrId]���������ͷ�g_semRdMtr[bThrId]��ִ��	***DoAutoSend����Ų�������̵߳���***
	
			//DoFaProtoEx(&g_ProEx);	//����������ݣ���չ645-F003Э��
	
			if (IsMtrParaChg(wPn)==false && (g_bMtrRdStatus[bPos] & bMask) && !g_bMtrRdStep[bThrId])	//�Ѿ�����
				continue;
	
			WaitSemaphore(g_semRdMtr[bThrId], SYS_TO_INFINITE);
			memset(g_bMtrAddr[bThrId], 0, 6);
			if (!GetMeterPara(wPn, &g_MtrPara[bThrId]))
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}
	
			//1��ȡ�õ��뵽RAM�Ĳ�����������ݣ� ���������ݡ������м����ݡ��澯�м����ݡ������м����ݵ�
			fRes = ReadPnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);	//���������SearchPnMap()
			if (fRes && !bInit[bThrId])//��һ�γ�ʼ��
			{
				InitMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);		//��ʼ���¼����ƽṹ
			}
	
			if (!fRes)
			{
				DTRACE(DB_METER, ("MtrRdThread: Read pn=%d's tmp fail, init\r\n", wPn));
				ClrPnTmp(g_PnTmp[bThrId], PNTMP_NUM);
				DbInitTimeData(SECT_PN_DATA, bThrId, dwCurIntervSec);
				InitMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);		//��ʼ���¼����ƽṹ
				WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);
			}
			else if (g_bMtrRdStep[bThrId] == 2)		//��������״̬
			{
				//if (!(g_bMtrRdStatus[bPos] & bMask))	//��δ����
				{
					DTRACE(DB_METER, ("MtrRdThread: clr pn=%d's tmp data for rx dir rd mtr cmd.\r\n", wPn));
					ClrPnTmp(g_PnTmp[bThrId], PNDAT_NUM);
					DbInitTimeData(SECT_PN_DATA, bThrId, dwCurIntervSec);
				}			
			}
	
			g_wCurPn[bThrId] = wPn;	//��ǰ���뵽�ڴ�Ĳ�����
	
			//2���жϼ�����ߵ������Ƿ����仯
			if (!fRes || IsMtrParaChg(wPn) || memcmp(g_MtrPara[bThrId].bAddr, g_bMtrAddr[bThrId], 6)!=0)	//�����仯
			{
				DTRACE(DB_METER, ("MtrRdThread: pn=%d para chg.\r\n", wPn));
	
				g_bPnFailCnt[iIdx] = 0;
				g_bPnFailFlg[bPos] &= ~bMask;
				g_bMtrRdStatus[bPos] &= ~bMask;
				
				ClrPnTmp(g_PnTmp[bThrId], PNTMP_NUM);
				DbInitTimeData(SECT_PN_DATA, bThrId, dwCurIntervSec);
				memcpy(g_bMtrAddr[bThrId], g_MtrPara[bThrId].bAddr, 6);
				WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);
	
				fMtrChg = true;
				ClrMtrChgFlg(wPn);
				InitMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);		//��ʼ���¼����ƽṹ
				DoMtrRdStat();
			}
			else if (dwCurIntervSec != DbGetCurInterv(SECT_PN_DATA, bThrId))	//����仯
			{
				g_bMtrRdStatus[bPos] &= ~bMask;
				g_bCurIntvWrFlg[bThrId] = 0;
				DbSetCurInterv(SECT_PN_DATA, bThrId, dwCurIntervSec);
				DoMtrRdStat();
			}
	
			//3���������Э��
			DTRACE(DB_METER, ("MtrRdThread: start rd mtr=%d at click=%ld...\r\n", wPn, GetClick()));
			
			pMtrPro = CreateMtrPro(wPn, &g_MtrPara[bThrId], &g_MtrSaveInf[bThrId], fMtrChg, bThrId);
			if (pMtrPro == NULL)
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}
	
			//4������ÿ��������
			fNeedToSave = false;
			for (i=0; i<2; i++)	//һ����������೭��2��
			{
				fModified = false;
				bRdErr = AutoReadPn(pMtrPro, wPn, dwCurIntervSec, bThrId, &fModified, g_bCurIntvWrFlg[bThrId]);
				//DTRACE(DB_METER, ("MtrRdThread: AutoReadPn wPn=%d, bRdErr=%d\r\n", wPn, bRdErr));
				if (bRdErr == RD_ERR_OK)
				{
					//if (GetClass1RptFn(wPn, bFnFlg))
					//	bRdErr = AutoReadRptClass1(pMtrPro, wPn, dwCurIntervSec, bFnFlg, bThrId, &fModified);
				}

				if (bRdErr == RD_ERR_485)
				{
					BYTE bLogicPort = GetPnPort(wPn);
					int iPort = MeterPortToPhy(bLogicPort);
					if (iPort >= 0)//������ڣ��������GPRS�����
					{
						DTRACE(DB_METER, ("MtrRdThread: CommClose Log bLogicPort=%d, Phy iPort=%d\r\n", bLogicPort, iPort));
						CommClose(iPort);
					}
				}

				if (fModified)
				{
					fNeedToSave = true;
					fHaveRd = true;		//���в����㷢��������
				}
	
				if (bRdErr != RD_ERR_UNFIN)		//û����
					break; //������for (i=0; i<2; i++)����ʹ�ã����������ź�������
			}
			
			
			if (bRdErr==RD_ERR_DIR && fNeedToSave)		//����ֱ��
			{
				g_wPnDataChk[bThrId] = CheckPnData(g_PnTmp[bThrId], PNDAT_NUM-1);
				WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}
	
			if (bRdErr!=RD_ERR_PARACHG && fNeedToSave)
			{
				//5��2�����ݶ���
                
                DTRACE(DB_FAPROTO, ("MtrRdThread: start do history task at click=%ld, bRdRet = %d.\r\n", GetClick(), bRdErr));
				if (GetUserTypeAndVip(wPn, &bVip, &bType))
				{
					if (1 == bVip)
					{
						Vip_Num++;
					}
					DoComTask(wPn, &now, g_dwComTaskRecTm[bThrId], Vip_Num, bVip, bType);
				}
				
				//6���澯
				//����ܵ����ֿռ䲻��(<128Bytes)�������̵߳ȴ��ܵ��ź���
                DTRACE(DB_FAPROTO, ("MtrRdThread: start do alr task at click=%ld.\r\n", GetClick()));
				DoMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);

				DTRACE(DB_FAPROTO, ("MtrRdThread: start do stat task at click=%ld.\r\n", GetClick()));
				//7��ͳ������
				DoDpMtrDemandStat(wPn,g_bDayDemandStat[bThrId], g_bMonDemandStat[bThrId], &now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUnBalanceStat(wPn, g_bDayUnbalanceVI[bThrId] ,&now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUStat(wPn, g_bDayMtrStatV[bThrId], g_bMonMtrStatV[bThrId], g_dwTotalV[bThrId], g_wTotalTimes[bThrId], &now,  &g_tDayMtrStatLtm[bThrId]);
				g_tDayMtrStatLtm[bThrId] = now;


				//8������������������ݣ� ���������ݡ������м����ݡ��澯�м����ݡ������м����ݵ�
				g_wPnDataChk[bThrId] = CheckPnData(g_PnTmp[bThrId], PNDAT_NUM-1);

				if (bRdErr == RD_ERR_OK)		//�޴�����ȫ����
				{	
					g_bMtrRdStatus[bPos] |= bMask;
					DoMtrRdStat();
					g_bCurIntvWrFlg[bThrId] = 100;
					if (!WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM))
					{
						DTRACE(DB_CRITICAL, ("MtrRdThread: angain WritePnTmp g_bCurIntvWrFlg 1 mtr=%d\r\n", wPn));
						WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);
					}
					DTRACE(DB_METER, ("MtrRdThread: mtr=%d rd ok!\r\n", wPn));
				}
						
				if (g_bCurIntvWrFlg[bThrId] < 100 && g_fStopMtrRd==false) //һ��������������ģʽ��2�α�����ᣬ����ģʽ��100�α�������㹻��
				{
					ReadItemEx(BN2, PN0, 0x2040, &bMode);
					if (bMode==0 && g_bCurIntvWrFlg[bThrId]==0)
						g_bCurIntvWrFlg[bThrId] = 99;
					else
						g_bCurIntvWrFlg[bThrId]++;
					if (!WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM))
					{
						DTRACE(DB_CRITICAL, ("MtrRdThread: angain WritePnTmp g_bCurIntvWrFlg 0 mtr=%d\r\n", wPn));
						WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);
					}
					DTRACE(DB_METER, ("MtrRdThread: mtr=%d rd data save!\r\n", wPn));
				}

				if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//ֱ����־
				{
					SignalSemaphore(g_semRdMtr[bThrId]);
					break;
				}
			}
			else if (IsDiffDay(&g_tDayMtrStatLtm[bThrId], &now))
			{
				//7��ͳ������
				DoDpMtrDemandStat(wPn,g_bDayDemandStat[bThrId], g_bMonDemandStat[bThrId], &now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUnBalanceStat(wPn, g_bDayUnbalanceVI[bThrId] ,&now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUStat(wPn, g_bDayMtrStatV[bThrId], g_bMonMtrStatV[bThrId], g_dwTotalV[bThrId], g_wTotalTimes[bThrId], &now,  &g_tDayMtrStatLtm[bThrId]);
				g_tDayMtrStatLtm[bThrId] = now;
				WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM); //д��ʱ��
				//g_bMtrRdStatus[bPos] &= ~bMask;//xzz ad ����״̬�������
			}

			if (!fNeedToSave && bRdErr==RD_ERR_OK && (g_bMtrRdStatus[bPos]&bMask)==0x00)		//�޴�����ȫ����
			{
				g_bMtrRdStatus[bPos] |= bMask;
				DoMtrRdStat();
			}

			SignalSemaphore(g_semRdMtr[bThrId]);
	
			//Sleep(50);
		}	//for (wPn=1; wPn<PN_NUM; wPn++)
		bInit[bThrId] = 1;
	
	//    SignalSemaphore(g_semRdMtr[bThrId]);

		if (g_fStopMtrRd)
		{
			if (GetClick()-g_dwLastStopMtrClick > g_wStopSec)  //��ͣ����30��� ���¿�ʼ����
				g_fStopMtrRd = false;
		}
	
		if (g_fDirRd == false)
			UpdateMtrRdStep(bThrId);

		//if (fHaveRd==false && g_fStopMtrRd==false && g_fDirRd==false)
		{
			//if (bThrId == 0)
					DoTask(bThrId);//��ͨ������м�����		
		}

		/*if (fHaveRd==false && g_fStopMtrRd==false && g_fDirRd==false) 	//���в�����û�з��������� && ����ֹͣ�����ֱ��״̬
		{	
			bRdCnt++;
			if (bRdCnt%2 == 0) //����2�ֶ�û����Ҫ��������ݣ�����Ϊ���ڴ��ڿ���״̬��
			{				 //�ڿ���״̬�£���������ȼ������ǣ��������˿��л����ѱ�
				WaitSemaphore(g_semRdMtr[bThrId], SYS_TO_INFINITE);

				//--����
				ReadItemEx(BN24, PN0, 0x5021, &bReRdType); //1:���� 2�����¶��� 3�����ߺ����¶���

				fSuccOnce = false;
				if (bReRdType & 2)
				{
					bRdErr = DoMtrReRd(bThrId, &fSuccOnce); //�������¶���
					if (bRdErr==RD_ERR_DIR || bRdErr==RD_ERR_STOPRD) 
						goto spec_end;
				}

				if (bReRdType & 1)
				{
					bRdErr = DoMtrCurveReRd(bThrId, &fSuccOnce); //��������
					if (bRdErr==RD_ERR_DIR || bRdErr==RD_ERR_STOPRD) 
						goto spec_end;
				}

				if (fSuccOnce)	//ֻҪ�����ɹ�һ�����������bRdCnt����Ҫ���������ѱ�״̬
					bRdCnt = 0;

				//--�ѱ�
				if (bRdCnt >= 2)
				{
					ReadItemEx(BN0, PN0, 0x07ef, &bSearch);
					GetCurTime(&now);
					if (bPort <= LOGIC_PORT_MAX)
						bPortSn = bPort - LOGIC_PORT_MIN;
					if(now.nHour>=20 && now.nHour<23 && (GetClick()>3*60) && ((fStart&(1<<bPortSn))==0) && bSearch!=0)
					{
						fStart |= 1<<bPortSn;
						fStop &= ~(1<<bPortSn);
						if (bPort == LOGIC_PORT_MIN)
							SetInfo(INFO_START_485I_MTRSCH);
						else if (bPort == LOGIC_PORT_MIN+1)
							SetInfo(INFO_START_485II_MTRSCH);
						else if (bPort == LOGIC_PORT_MIN+2)
							SetInfo(INFO_START_485III_MTRSCH);
					}

					if(now.nHour>=23 && (GetClick()>3*60) && fStart&(1<<bPortSn) && ((fStop&(1<<bPortSn))==0))
					{
						fStart &= ~(1<<bPortSn);
						fStop |= 1<<bPortSn;
						if (bPort == LOGIC_PORT_MIN)
							SetInfo(INFO_STOP_485I_MTRSCH);
						else if (bPort == LOGIC_PORT_MIN+1)
							SetInfo(INFO_STOP_485II_MTRSCH);
						else if (bPort == LOGIC_PORT_MIN+2)
							SetInfo(INFO_STOP_485III_MTRSCH);
					}

					if (now.nDay != bLastDay)
					{
						bLastDay = now.nDay;
						//DelReadFailPn();
					}

					if (bPort==LOGIC_PORT_MIN && GetInfo(INFO_START_485I_MTRSCH))
					{
						//if ((fStart&(1<<bPortSn))==0 && (fStop&(1<<bPortSn)))         //����ʲôʱ���յ������������
							//bSchStep[bPortSn] = 1; //�ڷ��ѱ�ʱ���յ�����������
						fStart |= 1<<bPortSn;
						fStop &= ~(1<<bPortSn);						
                        StartSearch(bPortSn);
						//dwStartSchClick = GetClick();
					}

					if (bPort==(LOGIC_PORT_MIN+1) && GetInfo(INFO_START_485II_MTRSCH))
					{
						//if ((fStart&(1<<bPortSn))==0 && (fStop&(1<<bPortSn)))
							//bSchStep[bPortSn] = 1; //�ڷ��ѱ�ʱ���յ�����������
						fStart |= 1<<bPortSn;
						fStop &= ~(1<<bPortSn);
                        StartSearch(bPortSn);
						//dwStartSchClick = GetClick();
					}

					if (bPort==(LOGIC_PORT_MIN+2) && GetInfo(INFO_START_485III_MTRSCH))
					{
						//if ((fStart&(1<<bPortSn))==0 && (fStop&(1<<bPortSn)))
							//bSchStep[bPortSn] = 1; //�ڷ��ѱ�ʱ���յ�����������
						fStart |= 1<<bPortSn;
						fStop &= ~(1<<bPortSn);						
                        StartSearch(bPortSn);
						//dwStartSchClick = GetClick();
					}

					if (bPort==LOGIC_PORT_MIN && GetInfo(INFO_STOP_485I_MTRSCH))
					{
						fStart &= ~(1<<bPortSn);
						fStop |= 1<<bPortSn;
                        //SaveSchMtrStaInfo();
						StopSearch(bPortSn);						
						//dwStartSchClick = 0;
					}

					if (bPort==(LOGIC_PORT_MIN+1) && GetInfo(INFO_STOP_485II_MTRSCH))
					{
						fStart &= ~(1<<bPortSn);
						fStop |= 1<<bPortSn;
                        //SaveSchMtrStaInfo();
						StopSearch(bPortSn);						
						//dwStartSchClick = 0;
					}

					if (bPort==(LOGIC_PORT_MIN+2) && GetInfo(INFO_STOP_485III_MTRSCH))
					{
						fStart &= ~(1<<bPortSn);
						fStop |= 1<<bPortSn;
                        //SaveSchMtrStaInfo();
						StopSearch(bPortSn);						
						//dwStartSchClick = 0;
					}

					DoSearch(bPortSn);
					bRdCnt = 0;
				}

				//--�л�������Ķ˿�
				bPort = SwPortOfThrd(bThrId);	//�л������̵߳Ķ˿ڵ�һ��û����ʹ�õĶ˿���
				DTRACE(DB_METER, ("MtrRdThread: bThrId=%d sw to bPort=%d\r\n", bThrId, bPort));
spec_end:
				SignalSemaphore(g_semRdMtr[bThrId]);
			}
		}*/
		//DoSearch(bThrId);

		Sleep(500);//1000
	}	//while(1)

	DTRACE(DB_METER, ("MtrRdThread: end at Click = %d!\r\n", GetClick()));

	return THREAD_RET_OK;
}

//1������ʵʱ����
int DoDirMtrRd(WORD wBn, WORD wPn, WORD wID, TTime now)
{
	int iRet;
	bool fRes, fMtrChg = false;
	DWORD dwIntervSec;
	struct TMtrPro* pMtrPro;	
	WORD wSubID=0;
	const WORD* pwSubID;
	int iThrId;	
	BYTE bBuf[120];
        
	if (!IsMtrPn(wPn))
		return -1;	
    
    iThrId = GrabPnThread(wPn);
	if ((iThrId >= DYN_PN_NUM) || (iThrId<0))
		return -1;

	SetDynPn(iThrId, wPn);	//����ϵͳ�⣬����ò����������
	memset(g_bMtrAddr[iThrId], 0, 6);
	if (!GetMeterPara(wPn, &g_MtrPara[iThrId]))
		return -1;

	fRes = ReadPnTmp(wPn, g_PnTmp[iThrId], PNTMP_NUM);	//���������SearchPnMap()
	if (!fRes || IsMtrParaChg(wPn) || memcmp(g_MtrPara[iThrId].bAddr, g_bMtrAddr[iThrId], 6)!=0)	//�����仯
		fMtrChg = true;

	g_wCurPn[iThrId] = wPn;	//��ǰ���뵽�ڴ�Ĳ�����
	pMtrPro = CreateMtrPro(wPn, &g_MtrPara[iThrId], &g_MtrSaveInf[iThrId], fMtrChg, iThrId);
	if (pMtrPro == NULL)
		return -1;	

	//GetCurTime(&now);
	dwIntervSec = GetCurIntervSec(wPn, &now);
	if (dwIntervSec != DbGetCurInterv(SECT_PN_DATA,  iThrId))	//����仯
		DbInitTimeData(SECT_PN_DATA, iThrId, dwIntervSec);	//���õ�ǰ���ʱ��
			
	pwSubID = Bank0To645ID(wID);
	if (pwSubID == NULL)
	{	
		iRet = AskMtrItem(pMtrPro, wPn, wID, bBuf);
		if (iRet > 0)	//��������	
		{
			SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
		}
		else if (iRet == -1) //��֧�ֵ�������
		{
			SetMtrUnsupId(wPn, wID, iThrId);
			SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
		}
	}
	else
	{
		while ((wSubID=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
		{
			iRet = AskMtrItem(pMtrPro, wPn, wSubID, bBuf);
			if (iRet > 0)	//��������	
			{
				SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
				if(wSubID == 0xc86f && false == g_fStopMtrRd)
				{
					g_wPnDataChk[iThrId] = CheckPnData(g_PnTmp[iThrId], PNDAT_NUM-1);
					WritePnTmp(wPn, g_PnTmp[iThrId], PNTMP_NUM);
				}
			}
			else if (iRet == -1) //��֧�ֵ�������
			{
				SetMtrUnsupId(wPn, wSubID, iThrId);
				SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
			}
		}
	}

	if(iRet <= 0 && -1 != iRet && -4 != iRet)//����ʧ��
	{
		if (IsCctPn(wPn))//�ز�����ʧ��ֱ����Ϊ����
		{
			if ((g_bPnFailFlg[wPn>>3] & (1<<(wPn & 7))) == 0)
			{
				g_bPnFailFlg[wPn>>3] |= 1<<(wPn & 7);
				OnMtrErrEstb(wPn, g_bLastRdMtrData[1]);		//�������ȷ��
				DoPortRdErr(true);      //�ն˳������
			}
		}
	}
	else
	{
		if (IsCctPn(wPn))//�ز�����ɹ�ֱ�ӻָ�
		{
			g_bPnFailFlg[wPn>>3] &= ~(1<<(wPn & 7));
			OnMtrErrRecv(wPn);		//������ϻָ�
			DoPortRdErr(false);
		}
	}

	return iThrId;
}

int DirReadFrz(BYTE bFrzTimes, BYTE* pbRxBuf, BYTE bFN, WORD wPN, WORD wIDs, TTime* time, struct TMtrPro* pMtrPro, BYTE bInterU)
{
	//TTime now;
	bool fSpec = false;
	TV07Tmp pTmpV07;	
	const WORD* pwID;	
	int iRet = 0, iRetLen = 0;
	WORD wItemLen = 0, wDataLen = 0;
	BYTE bRateNum = GetPnRateNum(wPN);
	BYTE *pbRxBuf0 = pbRxBuf;
	BYTE bRxBuf[128];

	if ( IsV07CurveId(wIDs) )
	{						
		pTmpV07.tRdLoadInfo.bNum = 1;//ÿ�γ�һ����
		//SecondsToTime((GetCurSec()-3600)/(15*60)*(15*60), &pTmpV07.tRdLoadInfo.tmStart);
		//pTmpV07.tRdLoadInfo.tmStart.nYear = 2011;
		//pTmpV07.tRdLoadInfo.tmStart.nMonth = 10;
		//pTmpV07.tRdLoadInfo.tmStart.nDay = 26;
		//pTmpV07.tRdLoadInfo.tmStart.nHour = 12;
		//pTmpV07.tRdLoadInfo.tmStart.nMinute = 0;
		memcpy(&pTmpV07.tRdLoadInfo.tmStart, time, sizeof(TTime));
		iRetLen = MtrReadFrz(pMtrPro, &pTmpV07, wIDs, pbRxBuf, 0);//��-4���ñʼ�¼������ -1����֧�ֵ�ID 0:�����쳣��
		
		return iRetLen;
	}
	if ((bFN >= 185 && bFN <= 188) || (bFN >= 193 && bFN <= 196)) //�����ʽ���⴦��
	{
		fSpec = true;
	}

	pwID = Bank0To645ID(wIDs);
	if (pwID == NULL)
	{	
		return -1001;
	}
	else
	{
		while (*pwID != 0x00)
		{
			if (*pwID == 0x9a00)
			{
				pwID ++;
				continue;
			}

			memset(bRxBuf, INVALID_DATA, sizeof(bRxBuf));
			wItemLen = GetItemLen(BN0, *pwID);

			memset(&pTmpV07, 0, sizeof(pTmpV07));
			
			if (*pwID==0xa11f || *pwID==0xa12f || *pwID==0xb11f || *pwID==0xb12f)//�������ID�������ǵ�ǰ������
			{
				//bFrzTimes = 0;
				iRetLen = -1;
			}
			else
			{
				iRetLen = MtrReadFrz(pMtrPro, &pTmpV07, *pwID, bRxBuf, bFrzTimes);
			}

			if (iRetLen > 0)
			{
				iRet = DirRdCommToProType(pbRxBuf0, *pwID, bRxBuf, iRetLen, fSpec);
				
				if (iRet == -1) //�����������ݲ���ת��ΪЭ���ʽ
				{
					memcpy(pbRxBuf0, bRxBuf, wItemLen);
				}
				else if (iRet == -2) //�����������ݺ�����Ч����
				{
					DTRACE(DB_COMPENMTR, ("DirReadFrz : C2F%d ,DirRead Failed, Data invalid!! \n",bFN));
					return -3;
				}
			}
			else if (iRetLen == -1)//��֧�ֵ�ID����û�е��������
			{
				memcpy(pbRxBuf0, bRxBuf, wItemLen);//�����Ч����
			}
			else //����ʧ��
			{
				DTRACE(DB_COMPENMTR, ("DirReadFrz : C2F%d ,DirRead Failed!! \n",bFN));
				return iRetLen; //��һ��IDʧ�ܣ�����ζ��FN����ʧ��
			}

			if(!fSpec)
				pbRxBuf0 += wItemLen; //ƫ�Ƴ���
			
			wDataLen += wItemLen;
			pwID++;
		}

	}

	return wDataLen;
}

//������ȡ�õ��ǰ3���ն��������
bool GetMtrDayFrzIdx(struct TMtrPro* pMtrPro, TTime tmStart, BYTE* pbDayFrzIdx)
{
	TV07Tmp pTmpV07;
	int iRetLen = 0;
	TTime time;
	int i, j;
	BYTE bRxBuf[32];	
	BYTE bRecTime[3];
	
	memset(&pTmpV07, 0, sizeof(pTmpV07));

	for (i=0; i<8; i++) //��Ѱ��8(3+5)���ʱ��
	{
		memset(bRxBuf, 0, sizeof(bRxBuf));
		iRetLen = MtrReadFrz(pMtrPro, &pTmpV07, 0x9a00, bRxBuf, i);
		if (iRetLen <= 0)	//����ʧ�ܣ��೭һ��
			iRetLen = MtrReadFrz(pMtrPro, &pTmpV07, 0x9a00, bRxBuf, i);
		
		if (iRetLen > 0)
		{
			for (j=0; j<3; j++)	//�Ƿ�������Ҫ��ǰ3���ʱ�꣺���죬ǰ�죬��ǰ��
			{
				time = tmStart;
				AddIntervs(&time, TIME_UNIT_DAY, -j);
				TimeToFmt20(&time, bRecTime);
				if (memcmp(bRxBuf+2, bRecTime, 3)==0 && pbDayFrzIdx[j]==INVALID_DATA)
				{										//�Ѿ��ҵ���Ӧ�����ŵľͲ��ٸ��£����ⱻ���ϵ������Ÿ���
					pbDayFrzIdx[j] = i;
				}

				if (pbDayFrzIdx[0]!=INVALID_DATA && 
					pbDayFrzIdx[1]!=INVALID_DATA && 
					pbDayFrzIdx[2]!=INVALID_DATA)
					return true;
			}
		}
	}
	if (pbDayFrzIdx[0]!=INVALID_DATA || pbDayFrzIdx[1]!=INVALID_DATA || pbDayFrzIdx[2]!=INVALID_DATA)//ֻҪ��1�������ݣ���ȥ����
		return true;
	else
		return false;
}

void SaveLastRdMtrData(WORD wID, WORD wPn, BYTE* pbBuf)
{
    TTime tmNow;
    BYTE bTmpBuf[5];
	GetCurTime(&tmNow);
	TimeToFmt15(&tmNow, bTmpBuf);
	if ((wID&0xfff0) == 0x9010)
	{
		BYTE bDynBn = GetBankOfDynPn(wPn);
		if (bDynBn < DYN_PN_NUM)
		{
			memcpy(&g_bLastRdMtrData[bDynBn][0], bTmpBuf, sizeof(bTmpBuf));
			memcpy(&g_bLastRdMtrData[bDynBn][5], pbBuf, 5);
		}
	}
	else if ((wID&0xfff0) == 0x9110)
	{
		BYTE bDynBn = GetBankOfDynPn(wPn);
		if (bDynBn < DYN_PN_NUM)
		{
			memcpy(&g_bLastRdMtrData[bDynBn][0], bTmpBuf, sizeof(bTmpBuf));
			memcpy(&g_bLastRdMtrData[bDynBn][10], pbBuf, 4);
		}
	}
}
