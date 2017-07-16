/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrCtrl.cpp
 * 摘    要：本文件主要实现电表的抄表控制
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年1月
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
//MtrCtrl私有宏定义


////////////////////////////////////////////////////////////////////////////////////////////
//MtrCtrl私有数据定义
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
BYTE g_bUnsupIdFlg[DYN_PN_NUM][MTR_UNSUP_ID_SIZE] = {0};	//电表不支持ID标志位

extern BYTE g_bSect3Data[DYN_PN_NUM][SECT3_DATA_SZ]; 		//本SEC数据RAM缓存
extern DWORD g_dwSect3IntervTime[DYN_PN_NUM][2];			//本BANK的当前间隔
extern BYTE g_bSect3TimeFlg[DYN_PN_NUM][SECT3_TMFLG_SZ];	//本BANK数据的更新时间标志
WORD g_wPnDataChk[DYN_PN_NUM];	//用于直抄1类数据只导入测量点数据部分时校验用

DWORD g_dwComTaskRecTm[DYN_PN_NUM][64];				//普通任务中间数据
BYTE g_ProfFrzFlg[DYN_PN_NUM][30];					//曲线冻结标志位
TMtrSaveInf g_MtrSaveInf[DYN_PN_NUM];
BYTE g_bMtrAddr[DYN_PN_NUM][6];		//电表地址，用在电表参数重新配置后重新上电，通过电表地址来判断是否换表了
						//在没掉电的情况下通过IsMtrParaChg()判断标志位即可
//BYTE g_bMtrRdTm[DYN_PN_NUM][3]; //最近一次抄表时间年月日

BYTE g_bDayMtrStatV[DYN_PN_NUM][66+27];//不考虑平均值，在入库时再算
BYTE g_bMonMtrStatV[DYN_PN_NUM][66+27];
TTime g_tDayMtrStatLtm[DYN_PN_NUM];
DWORD g_dwTotalV[DYN_PN_NUM][6];//求平均电压时的累计叠加电压
WORD  g_wTotalTimes[DYN_PN_NUM][6];//求平均电压时的累计叠加统计次数

BYTE g_bDayUnbalanceVI[DYN_PN_NUM][14];
BYTE g_bDayDemandStat[DYN_PN_NUM][24];
BYTE g_bMonDemandStat[DYN_PN_NUM][24];
//TTime g_bMonMtrStatLtm[DYN_PN_NUM];
BYTE g_bLastRdMtrData[DYN_PN_NUM][14]; //最近一次抄表成功的时间和示值，供抄表失败事件使用
BYTE g_bCurIntvWrFlg[DYN_PN_NUM]; //当前间隔写标志0:第一次写1:全部抄全写

#define PNTMP_NUM	20//11	//正常导入/导出个数

const TPnTmp g_PnTmp[DYN_PN_NUM][PNTMP_NUM] = 
{
	//测量点0
	{
		{g_bSect3Data[0], SECT3_DATA_SZ},		//测量点数据
		{(BYTE* )g_dwSect3IntervTime[0], 8},	//抄表间隔时标
		{g_bSect3TimeFlg[0], SECT3_TMFLG_SZ},	//抄表时间标志
		{(BYTE* )&g_wPnDataChk[0], 2},			//用于直抄1类数据只导入测量点数据部分时校验用
	
		{(BYTE* )&g_dwComTaskRecTm[0], sizeof(g_dwComTaskRecTm)/DYN_PN_NUM},	//普通任务中间数据
		{g_ProfFrzFlg[0], sizeof(g_ProfFrzFlg)/DYN_PN_NUM},			//曲线冻结标志位
		{g_bUnsupIdFlg[0], sizeof(g_bUnsupIdFlg)/DYN_PN_NUM},			//电表不支持ID标志位
		{(BYTE *)&g_MtrSaveInf[0], sizeof(TMtrSaveInf)},	//电表保持信息
		{(BYTE *)&g_MtrAlrCtrl[0], sizeof(TMtrAlrCtrl)},	//告警中间数据
		{g_bMtrAddr[0], 6},				//电表地址
		/////////////////测量点的电压统计//////////////////////////
		{g_bDayMtrStatV[0], sizeof(g_bDayMtrStatV)/DYN_PN_NUM},
		{g_bMonMtrStatV[0], sizeof(g_bMonMtrStatV)/DYN_PN_NUM},
		{(BYTE *)&g_tDayMtrStatLtm[0],sizeof(TTime)},
		{(BYTE *)g_dwTotalV[0], sizeof(g_dwTotalV)/DYN_PN_NUM},
		{(BYTE *)g_wTotalTimes[0], sizeof(g_wTotalTimes)/DYN_PN_NUM},
		
		/////////////////测量点的电压电流不平衡////////////////////
		{g_bDayUnbalanceVI[0], sizeof(g_bDayUnbalanceVI)/DYN_PN_NUM},
		/////////////////测量点日月需量////////////////////////////
		{g_bDayDemandStat[0], sizeof(g_bDayDemandStat)/DYN_PN_NUM},
		{g_bMonDemandStat[0], sizeof(g_bMonDemandStat)/DYN_PN_NUM},
		//{g_bMonMtrStatLtm[0],sizeof(TTime)},
		//{g_bMtrRdTm[0], 3},				//最近一次抄表时间
		{g_bLastRdMtrData[0], sizeof(g_bLastRdMtrData)/DYN_PN_NUM},
		{&g_bCurIntvWrFlg[0], sizeof(g_bCurIntvWrFlg)/DYN_PN_NUM},
	},

	//测量点1
	{
		{g_bSect3Data[1], SECT3_DATA_SZ},		//测量点数据
		{(BYTE* )g_dwSect3IntervTime[1], 8},	//抄表间隔时标
		{g_bSect3TimeFlg[1], SECT3_TMFLG_SZ},	//抄表时间标志
		{(BYTE* )&g_wPnDataChk[1], 2},			//用于直抄1类数据只导入测量点数据部分时校验用
	
		{(BYTE* )&g_dwComTaskRecTm[1], sizeof(g_dwComTaskRecTm)/DYN_PN_NUM},	//普通任务中间数据
		{g_ProfFrzFlg[1], sizeof(g_ProfFrzFlg)/DYN_PN_NUM},			//曲线冻结标志位
		{g_bUnsupIdFlg[1], sizeof(g_bUnsupIdFlg)/DYN_PN_NUM},			//电表不支持ID标志位
		{(BYTE *)&g_MtrSaveInf[1], sizeof(TMtrSaveInf)},	//电表保持信息
		{(BYTE *)&g_MtrAlrCtrl[1], sizeof(TMtrAlrCtrl)},	//告警中间数据
		{g_bMtrAddr[1], 6},				//电表地址
		{g_bDayMtrStatV[1], sizeof(g_bDayMtrStatV)/DYN_PN_NUM},
		{g_bMonMtrStatV[1], sizeof(g_bMonMtrStatV)/DYN_PN_NUM},
		{(BYTE *)&g_tDayMtrStatLtm[1],sizeof(TTime)},
		{(BYTE *)g_dwTotalV[1], sizeof(g_dwTotalV)/DYN_PN_NUM},
		{(BYTE *)g_wTotalTimes[1], sizeof(g_wTotalTimes)/DYN_PN_NUM},
		{g_bDayUnbalanceVI[1], sizeof(g_bDayUnbalanceVI)/DYN_PN_NUM},
		/////////////////测量点日月需量////////////////////////////
		{g_bDayDemandStat[1], sizeof(g_bDayDemandStat)/DYN_PN_NUM},
		{g_bMonDemandStat[1], sizeof(g_bMonDemandStat)/DYN_PN_NUM},
		//{g_bMonMtrStatLtm[1],sizeof(TTime)},
		//{g_bMtrRdTm[1], 3},				//最近一次抄表时间
		{g_bLastRdMtrData[1], sizeof(g_bLastRdMtrData)/DYN_PN_NUM},
		{&g_bCurIntvWrFlg[1], sizeof(g_bCurIntvWrFlg)/DYN_PN_NUM},
	},

};

#define PNDAT_NUM	4		//直抄要导入的测量点数据项目个数

WORD g_wCurPn[DYN_PN_NUM];	//当前导入到内存的测量点
TSem g_semMtrCtrl;		//抄表控制线程间的信号量
TSem g_semRdMtr[DYN_PN_NUM];
bool g_fDirRd = false;	//直抄标志
BYTE g_bDirRdStep = 0;	//1类数据抄表状态 1：正在抄，  0：没抄
TMtrPara g_MtrPara[DYN_PN_NUM];
DWORD g_dwLastIntervSec[DYN_PN_NUM];
BYTE g_bPnFailCnt[REAL_PN_NUM];
BYTE g_bPnFailFlg[REAL_PN_MASK_SIZE];
BYTE g_bMtrRdStatus[REAL_PN_MASK_SIZE];
BYTE g_bMtrRdStep[DYN_PN_NUM];	//立即抄表命令状态机 1：收到立即抄表命令， 2：正在抄表， 0：已经抄完

BYTE g_b485PortStatus;//485抄表口状态:0正常,1故障发生,2故障恢复
bool g_f485FailHapFlg;
BYTE g_bPortOfThrd[DYN_PN_NUM];		//线程当前使用的抄表端口

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


//描述:判断数据项是否已经抄完
//返回:抄完返回1，否则返回0
int QueryItemDone(WORD wPn, WORD wID, DWORD dwStartTime, DWORD dwEndTime, BYTE* pbBuf)
{
	WORD wValidNum = 0;
	int iConfirmNum = 0;

	TBankItem BankItem;
	BankItem.wBn = BN0;	//BANK号先定为多功能表,后面冻结的时候CctQueryItemTime()会自动动态调整
	BankItem.wPn = wPn;
	BankItem.wID = wID;
	iConfirmNum = QueryItemTimeMbi(dwStartTime, dwEndTime, &BankItem, 1, pbBuf, &wValidNum);

	if (iConfirmNum>0 || iConfirmNum<0)
		return 1;
	else
		return 0;
}

//返回:如果还没抄且时间刚好符合则返回0,如果已经抄完了则返回1,
//	   如果时间没到则返回-1
int ReadTimeOut(WORD wPn, const TMtrRdCtrl* pRdCtrl, TTime* pTm, BYTE* pbBuf)
{
	DWORD dwCurTime;
	DWORD dwStartTime;
	DWORD dwEndTime;

	if (GetItemTimeScope(wPn, pRdCtrl, pTm, &dwStartTime, &dwEndTime)) //取得集抄数据项读取的时间范围(起始时间~结束时间)
	{
		dwCurTime = TimeToSeconds(pTm);
		if (dwCurTime < dwStartTime)
			return -1;

		return QueryItemDone(wPn, pRdCtrl->wID, dwStartTime, dwEndTime, pbBuf);
	}
	else
	{
		return 1;	//不支持的东西返回1表示抄完,免得反复地抄
	}
}

//描述:查询数据项的抄读状态
//参数:@rCctRdCtrl 待查询数据项
//	   @tmNow 当前时间
// 	   @pbBuf 给系统库借用的缓冲
//返回:如果该数据项目前需要抄但还未抄、时间刚好符合则返回0,如果已经抄完了则返回1,如果需要抄但时间还没到则返回-1,
//	   如果该数据项对于该电表来说不需要抄则返回-2，其它情况返回-3
int QueryItemReadStatus(WORD wPn, const TMtrRdCtrl* pRdCtrl, TTime* pTm, BYTE* pbBuf)
{
	//TTime time;
	int iRet = IsIdNeedToRead(wPn, pRdCtrl);
	
	if (iRet != 0)	//目前不需要抄的都返回----如果该数据项目前需要抄但还未抄、时间刚好符合则返回0
		return iRet;	//不用抄的数据项返回抄完

	//目前07/97电表要抄的数据项都在MtrCfg.c文件中直接配置好，还不需要做内部ID到外部ID的转换

	//return ReadTimeOut(wPn, pRdCtrl, pTm, pbBuf);
	iRet = ReadTimeOut(wPn, pRdCtrl, pTm, pbBuf);
	//if (0 == iRet && TIME_UNIT_HOUR == pRdCtrl->bIntervU)
	//{
	//	GetCurTime(&time);
	//	DTRACE(DB_METER, ("IsIdNeedToRead: TIME_UNIT_HOUR  wPn=%d, wId=%2x, time= %d-%d-%d-%d-%d-%d \r\n", wPn, pRdCtrl->wID, time.nYear, time.nMonth, time.nDay, time.nHour, time.nMinute, time.nSecond));
	//}
	return iRet;
}



//判断时间是否跨过当前抄表间隔
DWORD GetCurIntervSec(WORD wPn, TTime* ptmNow)
{
	DWORD dwMin = TimeToMinutes(ptmNow);
	BYTE bInterv = GetMeterInterv(wPn);
    
    if (bInterv == 0)
        return 0;

	return dwMin / bInterv * bInterv * 60;
}

//描述：设置测量点不支持该ID的标志
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

//描述：判断该测量点是否不支持该ID
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

//描述：清楚该测量点所有ID是否支持的标志
void ResetMtrUnsupIdFlg(WORD wPn)
{
	memset(g_bUnsupIdFlg, 0, MTR_UNSUP_ID_SIZE);
}


//描述：测量点数据是否已经导入进内存
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

BYTE g_bPortInUseFlg = 0;	//端口占用标志位
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
			g_bPortInUseFlg &= ~(1<<(LOGIC_PORT_MAX-LOGIC_PORT_MIN));		//清除原来的占用标志
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

//描述：切换抄表线程的端口到一个没有在使用的端口上
//返回：切换到的端口号
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
		if ((g_bPortInUseFlg & (1<<i)) == 0) //没被使用的端口
		{
			g_bPortOfThrd[bThrId] = LOGIC_PORT_MIN + i;
			if (bOldPort != 0xff)
				g_bPortInUseFlg &= ~(1<<(bOldPort-LOGIC_PORT_MIN));		//清除原来的占用标志
			
			g_bPortInUseFlg |= 1<<i;	//设置新的占用标志
			
			break;
		}
	}

	if (i==LOGIC_PORT_NUM && bPortFun!=PORT_FUN_DEBUG)
		g_bPortOfThrd[bThrId] = 0xff;

	SignalSemaphore(g_semMtrCtrl);

	return g_bPortOfThrd[bThrId];
}


//描述：取得测量点所在的线程
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

		return (DYN_PN_NUM-1);//载波线程 固定用bThrId=1
	}

	if (GetInfo(INFO_PORT_FUN))
	{
		SwDebugPortFun();
	}

	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return 0xff;

	return 0;	//485线程 固定用bThrId=0

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

//描述：抢占一个线程，用在直抄的情况下，如果测量点的端口还没有被使用，则强制设置一个线程的端口
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
		return DYN_PN_NUM - 1;//载波线程
	}

	if (GetInfo(INFO_PORT_FUN))
	{
		SwDebugPortFun();
	}
	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return 0xff;

	WaitSemaphore(g_semMtrCtrl, SYS_TO_INFINITE);

	//端口是否已经被线程使用
	/*for (bThrId=0; bThrId<DYN_PN_NUM; bThrId++)
	{
		if (bPort == g_bPortOfThrd[bThrId])
		{
			SignalSemaphore(g_semMtrCtrl);
			return bThrId;
		}
	}

	//看是否有空闲没用的
	for (bThrId=0; bThrId<DYN_PN_NUM; bThrId++)
	{
		if (g_bPortOfThrd[bThrId] == 0xff)
		{
			g_bPortOfThrd[bThrId] = bPort;
			g_bPortInUseFlg |= 1<<(bPort-LOGIC_PORT_MIN);	//设置新的占用标志

			SignalSemaphore(g_semMtrCtrl);
			return bThrId;
		}
	}*/

	//强制给线程0
	bOldPort = g_bPortOfThrd[0];
	g_bPortOfThrd[0] = bPort;
	g_bPortInUseFlg &= ~(1<<(bOldPort-LOGIC_PORT_MIN));		//清除原来的占用标志
	g_bPortInUseFlg |= 1<<(bPort-LOGIC_PORT_MIN);	//设置新的占用标志

	SignalSemaphore(g_semMtrCtrl);
	return 0;	//返回线程0
}


//描述：导入测量点直抄数据
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


//描述：取得直抄的控制权
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

//描述：释放直抄的控制权
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

//终端抄表故障实践
void DoPortRdErr(bool fMtrFailHap)
{
    TTime tmNow;
    if (fMtrFailHap)   //发生抄表失败
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

//描述：自动把一个测量点要抄的数据项抄一轮
//返回：抄读错误定义
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
		return RD_ERR_PARACHG;		//电表参数变更
#else
	iIdx = wPn;
#endif

	bPos = iIdx>>3;
	bMask = 1<<(iIdx & 7);

	//if (IsPowerOff())	//停电
		//return RD_ERR_PWROFF;

	if (g_bPnFailFlg[bPos] & bMask) //本电表通信故障
	{
		if (pMtrPro->pMtrPara->bProId != PROTOCOLNO_DLT645)
			wTestId = 0x901f;
		if (AskMtrItem(pMtrPro, wPn, wTestId, bBuf) > 0)	//用wTestId测试本电表是否恢复
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

			OnMtrErrRecv(wPn);		//抄表故障恢复
			DoPortRdErr(false);
		}
		else
		{
			return RD_ERR_485;	//依然存在485抄表故障
		}
	}

	//只有在电表没有故障的时候才下来实时抄表
	for (i=0; i<wItemNum; i++)
	{
	  	if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//直抄标志
			return RD_ERR_DIR;		//正在直抄
		if (g_fStopMtrRd)
  			return RD_ERR_STOPRD;		//正在直抄

		//判断时间是否跨过当前抄表间隔，如果跨过了就结束抄表，把2类数据冻结、告警判断等任务先执行一下
		GetCurTime(&now);

		if (IsMtrParaChg(wPn))	//参数变化
			return RD_ERR_PARACHG;		//电表参数变更

		dwIntervSec = GetCurIntervSec(wPn, &now);
		if (dwIntervSec != dwCurIntervSec)	//抄表间隔发生改变
			return RD_ERR_INTVCHG;		//抄表间隔变更

		iItemStatus = QueryItemReadStatus(wPn, pRdCtrl+i, &now, bBuf);
		if (iItemStatus == 0)	//如果该数据项目前需要抄但还未抄则返回0
		{
			//if (IsPowerOff())	//停电后电表还能抄的情况，还是要返回
				//return RD_ERR_PWROFF;
			DTRACE(DB_METER, ("AutoReadPn: Rd ---> Pn=%d, wID=%x, i=%d, bIntervU=%s\r\n", wPn, pRdCtrl[i].wID, i, IntervUToStr(pRdCtrl[i].bIntervU, str)));

			wID = pRdCtrl[i].wID;
			if (IsMtrUnsupId(wPn, wID, bThrId))
				iRet = -1;	//不支持的数据项
			else
				iRet = AskMtrItem(pMtrPro, wPn, wID, bBuf);

			if (iRet > 0)	//抄表正常
			{
				if (SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_APP_OK) >= 0) //9a00时标不对不能置修改标志，否则补抄9a00会导致一直写Flash
					*pfModified = true; //测量点数据已修改，需要保存到外部FLASH
				g_bPnFailCnt[iIdx] = 0;
			}
			else if (iRet == -1) //不支持的数据项
			{
				SetMtrUnsupId(wPn, wID, bThrId);
				SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
			}
			else if (iRet == -4) //07版645表曲线通信返回正常，但目前无此记录
			{
				SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
			}
			else if(!IsPowerOff()) //没停电的情况下抄表失败，要同时判断485故障
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

					if (IsCctPn(wPn))//载波抄表直接认为故障
					{
						if ((g_bPnFailFlg[bPos] & bMask) == 0)
						{
							g_bPnFailFlg[bPos] |= bMask;
							OnMtrErrEstb(wPn, g_bLastRdMtrData[bThrId]);		//抄表故障确认
							DoPortRdErr(true);      //终端抄表故障
						}

						return RD_ERR_485;	//存在485抄表故障
					}
					else//485抄表保持
					{
						if (pMtrPro->pfnAskItem(pMtrPro, wTestId, bBuf) <= 0) //抄表失败还要抄一下9010确认一下,避免有些ID的干扰
						{
							if ((g_bPnFailFlg[bPos] & bMask) == 0)
							{
								g_bPnFailFlg[bPos] |= bMask;
								OnMtrErrEstb(wPn, g_bLastRdMtrData[bThrId]);		//抄表故障确认
								DoPortRdErr(true);      //终端抄表故障
							}

							return RD_ERR_485;	//存在485抄表故障
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
			else  //停电了,本测量点本轮剩下的不抄了
			{
				return RD_ERR_PWROFF;
			}
		}
		else if (iItemStatus == -1)	//如果需要抄但时间还没到则返回-1,
		{
			fUnfinish = true;
		}
		//else 如果已经抄完了则返回1,如果该数据项对于该电表来说不需要抄则返回-2，其它情况返回-3
		//if (i==1 && bRdCnt==0)
		//	return RD_ERR_UNFIN;	//没抄完，电量优先抄读
	}

	if (fUnfinish)
		return RD_ERR_UNFIN;	//没抄完
	else
		return RD_ERR_OK;		//无错误，完全抄完
}

/*
//描述：在抄表间隔自动抄读电表主动上送任务的1类数据
BYTE AutoReadRptClass1(struct TMtrPro* pMtrPro, WORD wPn, DWORD dwCurIntervSec, BYTE* pbFnFlg, BYTE bThrId, bool* pfModified)
{
	const WORD* pwSubID;
	int iIdx, iRet, iLen;
	DWORD dwIntervSec;
	WORD wBN, wPN, wID, wSubID;		//转换后的标识
	WORD wTestId = 0x9010;
	TTime now;
	BYTE bVarLen = false;	//变长
	BYTE bFn, fn, bPos, bMask;
	BYTE bBuf[120];

	if (IsAllAByte(pbFnFlg, 0, 32))
		return RD_ERR_OK;

#if MTRPNMAP!=PNUNMAP
	iIdx = SearchPnMap(MTRPNMAP, wPn);
	if (iIdx < 0)
		return RD_ERR_PARACHG;		//电表参数变更
#else
	iIdx = wPn;
#endif

	bPos = iIdx>>3;
	bMask = 1<<(iIdx & 7);

	//if (IsPowerOff())	//停电
		//return RD_ERR_PWROFF;

	if (g_bPnFailFlg[bPos] & bMask) //本电表通信故障
	{
		if (pMtrPro->pMtrPara->bProId != PROTOCOLNO_DLT645)
			wTestId = 0x901f;
		if (AskMtrItem(pMtrPro, wPn, wTestId, bBuf) > 0)	//用wTestId测试本电表是否恢复
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

			OnMtrErrRecv(wPn);		//抄表故障恢复
			DoPortRdErr(false);
		}
		else
		{
			return RD_ERR_485;	//依然存在485抄表故障
		}
	}

	//dwStartTime = GetCurSec() / 3600 * 3600;
	//dwEndTime = dwStartTime + 3600;

	for (bFn=1; bFn<181; bFn++)
	{
	
		fn = bFn - 1;
		if ((pbFnFlg[fn>>3] & (1<<(fn&7))) == 0)
			continue;

		if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//直抄标志
			return RD_ERR_DIR;		//正在直抄
		if (g_fStopMtrRd)
  			return RD_ERR_STOPRD;		//正在直抄

		//判断时间是否跨过当前抄表间隔，如果跨过了就结束抄表，把2类数据冻结、告警判断等任务先执行一下
		GetCurTime(&now);

		if (IsMtrParaChg(wPn))	//参数变化
			return RD_ERR_PARACHG;		//电表参数变更

		dwIntervSec = GetCurIntervSec(wPn, &now);
		if (dwIntervSec != dwCurIntervSec)	//抄表间隔发生改变
			return RD_ERR_INTVCHG;		//抄表间隔变更

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
				if (iRet > 0)	//抄表正常	
				{
					SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
					*pfModified = true; //测量点数据已修改，需要保存到外部FLASH
				}
				else if (iRet == -1) //不支持的数据项
				{
					SetMtrUnsupId(wPn, wID, bThrId);
					SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
					*pfModified = true; //测量点数据已修改，需要保存到外部FLASH
				}
			}
		}
		else
		{
			while ((wSubID=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
			{
				if (!QueryItemDone(wPn, wSubID, dwIntervSec, dwIntervSec+3600, bBuf))
				{
					DTRACE(DB_METER, ("AutoReadRptClass1: wPn=%d, wID=0x%04x\r\n", wPn, wSubID));
					iRet = AskMtrItem(pMtrPro, wPn, wSubID, bBuf);
					if (iRet > 0)	//抄表正常	
					{
						SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
						*pfModified = true; //测量点数据已修改，需要保存到外部FLASH
					}
					else if (iRet == -1) //不支持的数据项
					{
						SetMtrUnsupId(wPn, wSubID, bThrId);
						SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
						*pfModified = true; //测量点数据已修改，需要保存到外部FLASH
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
		case 1:		//收到立即抄表命令
			DTRACE(DB_METER, ("MtrRdThread: start to direct rd mtr.\r\n"));
			//memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//把完成标志清除
			g_bMtrRdStep[bThrId] = 2;
			break;
		case 2:		//抄表状态
			DTRACE(DB_METER, ("MtrRdThread: finish direct rd mtr.\r\n"));			
			g_bMtrRdStep[bThrId] = 0;
			break;
		default:	//空闲状态
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

	memset(&g_bPortOfThrd, 0xff, sizeof(g_bPortOfThrd));		//线程当前使用的抄表端口
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

//描述：针对补抄等情况，设置线程的当前测量点
//返回：如果测量点不是线程当前抄表端口的测量点则返回NULL,正确返回对应测量点的电表协议
struct TMtrPro* SetupThrdForPn(BYTE bThrId, WORD wPn)
{
	if (!IsMtrPn(wPn))
		return NULL;

	if (GetPnThread(wPn) != bThrId)  //不是本端口的电能表
		return NULL;

	//SetDynPn(bThrId, wPn);	//设置系统库，缓存该测量点的数据
								//目前补抄不涉及到系统库，所以不用设置

	memset(g_bMtrAddr[bThrId], 0, 6);
	GetMeterPara(wPn, &g_MtrPara[bThrId]);

	memset(&g_MtrSaveInf[bThrId], 0, sizeof(g_MtrSaveInf[bThrId]));

	return CreateMtrPro(wPn, &g_MtrPara[bThrId], &g_MtrSaveInf[bThrId], false, bThrId);
}

//描述:取得当前的抄表状态
BYTE GetRdMtrState(BYTE bThrId)
{
	if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//直抄标志
		return RD_ERR_DIR;		//正在直抄
	if (g_fStopMtrRd)
		return RD_ERR_STOPRD;	//正在直抄

	return RD_ERR_OK;
}

//描述：抄表线程，执行抄表，2类数据冻结、告警判断等任务
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
	BYTE fStart = 0;   //每BIT表示一个端口
	BYTE fStop = 0;
	BYTE bMode = 0;
	bool fHaveRd, fSuccOnce;		//所有测量点是否发生过抄读
	BYTE bPortSn = 0, bSearch = 0;
	BYTE bRdCnt = 0, bReRdType = 0;
//	BYTE bFnFlg[32]; //1类数据主动上送任务，在本抄表周期内需要抄的Fn
	BYTE bInit[2] = {0, 0};//上电初始化
	BYTE bSchStep[MTR_PORT_NUM];
	BYTE bType, bVip;
	memset(bSchStep, 1, sizeof(bSchStep));
	GetCurTime(&now);
	bLastDay = now.nDay;
	bThrId = (BYTE ) pvPara;

	InitMtrReRd();
	SwPortOfThrd(bThrId);	//切换抄表线程的端口到一个没有在使用的端口上
	DTRACE(DB_METER, ("MtrRdThread: bThrId=%d start.\r\n", bThrId));
	while (1)
	{
		Vip_Num = 0;
		fHaveRd = false;	//所有测量点是否发生过抄读

		//DoTask(bThrId);//普通任务和中继任务

	//    WaitSemaphore(g_semRdMtr[bThrId], SYS_TO_INFINITE);
		for (wPn=1; wPn<PN_NUM; wPn++)
		{
			//NOTE：
			//本段有可能两个抄表线程会同时调用，在UpdFat()函数有信号量TdbWaitSemaphore()保护，
			//不会有问题
			if (1 == bThrId)
				fInfo = GetInfo(INFO_TASK_PARA);	// || GetInfo(INFO_METER_PARA)
			else
				fInfo = false;

			//fInfo = GetInfo(INFO_TASK_PARA);	// || GetInfo(INFO_METER_PARA)
			if (fInfo || //2类F39配置发生改变
				(!g_fUpdFatOk && GetClick()-dwUpdFatClick>60*60))
			{
				g_fUpdFatOk = UpdFat();		//更新Flash动态分配，一定要放在InitDB之后
				dwUpdFatClick = GetClick();
	
				if (fInfo)
				{
					SetInfo(INFO_TASK_INIT);	 //通知抄表线程，F39配置改变
					memset(g_dwLastIntervSec, 0, sizeof(g_dwLastIntervSec));
					memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));
                    if (g_fStopMtrRd)
                    {
                        if (IsCctPn(PN9))	//配合台体测试修改
                            StopMtrRd(12);      //12
                        else
                            StopMtrRd(18);      //12
                    }
                    //g_fStopMtrRd = false;   //允许抄表
				}
			}
			//Sleep(20);
			if (g_fDirRd || g_fStopMtrRd || g_bMtrRdStep[bThrId]==1)	//直抄标志 或 立即抄表命令
				break;
			
			if(1==bThrId && !IsCctPn(wPn))	//线程1是载波线程，如果测量点配置的不是载波端口就continue
				continue;

			if(0==bThrId && IsCctPn(wPn))	//线程0是485线程，如果测量点配置的是载波端口就continue
				continue;

			MtrRelayTask(wPn, &g_MtrAlrCtrl[bThrId].tMtrPwSta);			//拉合闸控制失败
          	SwPortOfThrd(bThrId);	//切换抄表线程的端口到一个没有在使用的端口上
			if (!IsMtrPn(wPn))
				continue;

			if (GetPnThread(wPn) != bThrId)  //不是本端口的电能表
				continue;

			SetDynPn(bThrId, wPn);	//设置系统库，缓存该测量点的数据
		
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
			{	//抄表间隔发生切换				|| F38配置改变			  || F39配置改变
				g_dwLastIntervSec[bThrId] = dwCurIntervSec;
				memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//把完成标志清除
				DoMtrRdStat();
				DTRACE(DB_METER, ("MtrRdThread: clr rd flg for interval or para change, cur pn=%d's\r\n", wPn));
			}
	
			//if (IsAutoSendAdmit())   //主动上报是否允许
			//	DoAutoSend();	//上报一类数据时采用直抄方式抄表会等待信号量g_semRdMtr[bThrId]，必须在释放g_semRdMtr[bThrId]后执行	***DoAutoSend可以挪到其他线程调用***
	
			//DoFaProtoEx(&g_ProEx);	//与基表交互数据，扩展645-F003协议
	
			if (IsMtrParaChg(wPn)==false && (g_bMtrRdStatus[bPos] & bMask) && !g_bMtrRdStep[bThrId])	//已经抄完
				continue;
	
			WaitSemaphore(g_semRdMtr[bThrId], SYS_TO_INFINITE);
			memset(g_bMtrAddr[bThrId], 0, 6);
			if (!GetMeterPara(wPn, &g_MtrPara[bThrId]))
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}
	
			//1、取得导入到RAM的测量点相关数据： 测量点数据、冻结中间数据、告警中间数据、电表库中间数据等
			fRes = ReadPnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);	//里面调用了SearchPnMap()
			if (fRes && !bInit[bThrId])//第一次初始化
			{
				InitMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);		//初始化事件控制结构
			}
	
			if (!fRes)
			{
				DTRACE(DB_METER, ("MtrRdThread: Read pn=%d's tmp fail, init\r\n", wPn));
				ClrPnTmp(g_PnTmp[bThrId], PNTMP_NUM);
				DbInitTimeData(SECT_PN_DATA, bThrId, dwCurIntervSec);
				InitMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);		//初始化事件控制结构
				WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);
			}
			else if (g_bMtrRdStep[bThrId] == 2)		//立即抄表状态
			{
				//if (!(g_bMtrRdStatus[bPos] & bMask))	//还未抄到
				{
					DTRACE(DB_METER, ("MtrRdThread: clr pn=%d's tmp data for rx dir rd mtr cmd.\r\n", wPn));
					ClrPnTmp(g_PnTmp[bThrId], PNDAT_NUM);
					DbInitTimeData(SECT_PN_DATA, bThrId, dwCurIntervSec);
				}			
			}
	
			g_wCurPn[bThrId] = wPn;	//当前导入到内存的测量点
	
			//2、判断间隔或者电表参数是否发生变化
			if (!fRes || IsMtrParaChg(wPn) || memcmp(g_MtrPara[bThrId].bAddr, g_bMtrAddr[bThrId], 6)!=0)	//参数变化
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
				InitMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);		//初始化事件控制结构
				DoMtrRdStat();
			}
			else if (dwCurIntervSec != DbGetCurInterv(SECT_PN_DATA, bThrId))	//间隔变化
			{
				g_bMtrRdStatus[bPos] &= ~bMask;
				g_bCurIntvWrFlg[bThrId] = 0;
				DbSetCurInterv(SECT_PN_DATA, bThrId, dwCurIntervSec);
				DoMtrRdStat();
			}
	
			//3、创建电表协议
			DTRACE(DB_METER, ("MtrRdThread: start rd mtr=%d at click=%ld...\r\n", wPn, GetClick()));
			
			pMtrPro = CreateMtrPro(wPn, &g_MtrPara[bThrId], &g_MtrSaveInf[bThrId], fMtrChg, bThrId);
			if (pMtrPro == NULL)
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}
	
			//4、抄读每个数据项
			fNeedToSave = false;
			for (i=0; i<2; i++)	//一个测量点最多抄读2轮
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
					if (iPort >= 0)//关物理口，不能误关GPRS与红外
					{
						DTRACE(DB_METER, ("MtrRdThread: CommClose Log bLogicPort=%d, Phy iPort=%d\r\n", bLogicPort, iPort));
						CommClose(iPort);
					}
				}

				if (fModified)
				{
					fNeedToSave = true;
					fHaveRd = true;		//所有测量点发生过抄读
				}
	
				if (bRdErr != RD_ERR_UNFIN)		//没抄完
					break; //必须与for (i=0; i<2; i++)配套使用，否则引起信号量死锁
			}
			
			
			if (bRdErr==RD_ERR_DIR && fNeedToSave)		//正在直抄
			{
				g_wPnDataChk[bThrId] = CheckPnData(g_PnTmp[bThrId], PNDAT_NUM-1);
				WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM);
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}
	
			if (bRdErr!=RD_ERR_PARACHG && fNeedToSave)
			{
				//5、2类数据冻结
                
                DTRACE(DB_FAPROTO, ("MtrRdThread: start do history task at click=%ld, bRdRet = %d.\r\n", GetClick(), bRdErr));
				if (GetUserTypeAndVip(wPn, &bVip, &bType))
				{
					if (1 == bVip)
					{
						Vip_Num++;
					}
					DoComTask(wPn, &now, g_dwComTaskRecTm[bThrId], Vip_Num, bVip, bType);
				}
				
				//6、告警
				//如果管道出现空间不够(<128Bytes)，抄表线程等待管道信号量
                DTRACE(DB_FAPROTO, ("MtrRdThread: start do alr task at click=%ld.\r\n", GetClick()));
				DoMtrExc(wPn, &g_MtrAlrCtrl[bThrId]);

				DTRACE(DB_FAPROTO, ("MtrRdThread: start do stat task at click=%ld.\r\n", GetClick()));
				//7、统计数据
				DoDpMtrDemandStat(wPn,g_bDayDemandStat[bThrId], g_bMonDemandStat[bThrId], &now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUnBalanceStat(wPn, g_bDayUnbalanceVI[bThrId] ,&now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUStat(wPn, g_bDayMtrStatV[bThrId], g_bMonMtrStatV[bThrId], g_dwTotalV[bThrId], g_wTotalTimes[bThrId], &now,  &g_tDayMtrStatLtm[bThrId]);
				g_tDayMtrStatLtm[bThrId] = now;


				//8、导出测量点相关数据： 测量点数据、冻结中间数据、告警中间数据、电表库中间数据等
				g_wPnDataChk[bThrId] = CheckPnData(g_PnTmp[bThrId], PNDAT_NUM-1);

				if (bRdErr == RD_ERR_OK)		//无错误，完全抄完
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
						
				if (g_bCurIntvWrFlg[bThrId] < 100 && g_fStopMtrRd==false) //一个抄表周期正常模式下2次保存机会，测试模式下100次保存机会足够了
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

				if (g_fDirRd || g_bMtrRdStep[bThrId]==1)	//直抄标志
				{
					SignalSemaphore(g_semRdMtr[bThrId]);
					break;
				}
			}
			else if (IsDiffDay(&g_tDayMtrStatLtm[bThrId], &now))
			{
				//7、统计数据
				DoDpMtrDemandStat(wPn,g_bDayDemandStat[bThrId], g_bMonDemandStat[bThrId], &now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUnBalanceStat(wPn, g_bDayUnbalanceVI[bThrId] ,&now, &g_tDayMtrStatLtm[bThrId]);
				DoDpMtrUStat(wPn, g_bDayMtrStatV[bThrId], g_bMonMtrStatV[bThrId], g_dwTotalV[bThrId], g_wTotalTimes[bThrId], &now,  &g_tDayMtrStatLtm[bThrId]);
				g_tDayMtrStatLtm[bThrId] = now;
				WritePnTmp(wPn, g_PnTmp[bThrId], PNTMP_NUM); //写入时间
				//g_bMtrRdStatus[bPos] &= ~bMask;//xzz ad 抄表状态按天计算
			}

			if (!fNeedToSave && bRdErr==RD_ERR_OK && (g_bMtrRdStatus[bPos]&bMask)==0x00)		//无错误，完全抄完
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
			if (GetClick()-g_dwLastStopMtrClick > g_wStopSec)  //暂停抄表30秒后 重新开始抄表
				g_fStopMtrRd = false;
		}
	
		if (g_fDirRd == false)
			UpdateMtrRdStep(bThrId);

		//if (fHaveRd==false && g_fStopMtrRd==false && g_fDirRd==false)
		{
			//if (bThrId == 0)
					DoTask(bThrId);//普通任务和中继任务		
		}

		/*if (fHaveRd==false && g_fStopMtrRd==false && g_fDirRd==false) 	//所有测量点没有发生过抄读 && 不在停止抄表和直抄状态
		{	
			bRdCnt++;
			if (bRdCnt%2 == 0) //连续2轮都没有需要保存的数据，就认为现在处于空闲状态，
			{				 //在空闲状态下，任务的优先级依次是：补抄、端口切换、搜表
				WaitSemaphore(g_semRdMtr[bThrId], SYS_TO_INFINITE);

				//--补抄
				ReadItemEx(BN24, PN0, 0x5021, &bReRdType); //1:曲线 2：日月冻结 3：曲线和日月冻结

				fSuccOnce = false;
				if (bReRdType & 2)
				{
					bRdErr = DoMtrReRd(bThrId, &fSuccOnce); //补抄日月冻结
					if (bRdErr==RD_ERR_DIR || bRdErr==RD_ERR_STOPRD) 
						goto spec_end;
				}

				if (bReRdType & 1)
				{
					bRdErr = DoMtrCurveReRd(bThrId, &fSuccOnce); //补抄曲线
					if (bRdErr==RD_ERR_DIR || bRdErr==RD_ERR_STOPRD) 
						goto spec_end;
				}

				if (fSuccOnce)	//只要补抄成功一个数据项，就清bRdCnt，不要让它进入搜表状态
					bRdCnt = 0;

				//--搜表
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
						//if ((fStart&(1<<bPortSn))==0 && (fStop&(1<<bPortSn)))         //不管什么时段收到命令就重新搜
							//bSchStep[bPortSn] = 1; //在非搜表时段收到命令重新搜
						fStart |= 1<<bPortSn;
						fStop &= ~(1<<bPortSn);						
                        StartSearch(bPortSn);
						//dwStartSchClick = GetClick();
					}

					if (bPort==(LOGIC_PORT_MIN+1) && GetInfo(INFO_START_485II_MTRSCH))
					{
						//if ((fStart&(1<<bPortSn))==0 && (fStop&(1<<bPortSn)))
							//bSchStep[bPortSn] = 1; //在非搜表时段收到命令重新搜
						fStart |= 1<<bPortSn;
						fStop &= ~(1<<bPortSn);
                        StartSearch(bPortSn);
						//dwStartSchClick = GetClick();
					}

					if (bPort==(LOGIC_PORT_MIN+2) && GetInfo(INFO_START_485III_MTRSCH))
					{
						//if ((fStart&(1<<bPortSn))==0 && (fStop&(1<<bPortSn)))
							//bSchStep[bPortSn] = 1; //在非搜表时段收到命令重新搜
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

				//--切换到另外的端口
				bPort = SwPortOfThrd(bThrId);	//切换抄表线程的端口到一个没有在使用的端口上
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

//1类数据实时抄表
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

	SetDynPn(iThrId, wPn);	//设置系统库，缓存该测量点的数据
	memset(g_bMtrAddr[iThrId], 0, 6);
	if (!GetMeterPara(wPn, &g_MtrPara[iThrId]))
		return -1;

	fRes = ReadPnTmp(wPn, g_PnTmp[iThrId], PNTMP_NUM);	//里面调用了SearchPnMap()
	if (!fRes || IsMtrParaChg(wPn) || memcmp(g_MtrPara[iThrId].bAddr, g_bMtrAddr[iThrId], 6)!=0)	//参数变化
		fMtrChg = true;

	g_wCurPn[iThrId] = wPn;	//当前导入到内存的测量点
	pMtrPro = CreateMtrPro(wPn, &g_MtrPara[iThrId], &g_MtrSaveInf[iThrId], fMtrChg, iThrId);
	if (pMtrPro == NULL)
		return -1;	

	//GetCurTime(&now);
	dwIntervSec = GetCurIntervSec(wPn, &now);
	if (dwIntervSec != DbGetCurInterv(SECT_PN_DATA,  iThrId))	//间隔变化
		DbInitTimeData(SECT_PN_DATA, iThrId, dwIntervSec);	//设置当前间隔时标
			
	pwSubID = Bank0To645ID(wID);
	if (pwSubID == NULL)
	{	
		iRet = AskMtrItem(pMtrPro, wPn, wID, bBuf);
		if (iRet > 0)	//抄表正常	
		{
			SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
		}
		else if (iRet == -1) //不支持的数据项
		{
			SetMtrUnsupId(wPn, wID, iThrId);
			SaveMeterItem(wPn, wID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
		}
	}
	else
	{
		while ((wSubID=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
		{
			iRet = AskMtrItem(pMtrPro, wPn, wSubID, bBuf);
			if (iRet > 0)	//抄表正常	
			{
				SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_APP_OK);
				if(wSubID == 0xc86f && false == g_fStopMtrRd)
				{
					g_wPnDataChk[iThrId] = CheckPnData(g_PnTmp[iThrId], PNDAT_NUM-1);
					WritePnTmp(wPn, g_PnTmp[iThrId], PNTMP_NUM);
				}
			}
			else if (iRet == -1) //不支持的数据项
			{
				SetMtrUnsupId(wPn, wSubID, iThrId);
				SaveMeterItem(wPn, wSubID, bBuf, iRet, dwIntervSec, ERR_UNSUP);
			}
		}
	}

	if(iRet <= 0 && -1 != iRet && -4 != iRet)//抄表失败
	{
		if (IsCctPn(wPn))//载波抄表失败直接认为故障
		{
			if ((g_bPnFailFlg[wPn>>3] & (1<<(wPn & 7))) == 0)
			{
				g_bPnFailFlg[wPn>>3] |= 1<<(wPn & 7);
				OnMtrErrEstb(wPn, g_bLastRdMtrData[1]);		//抄表故障确认
				DoPortRdErr(true);      //终端抄表故障
			}
		}
	}
	else
	{
		if (IsCctPn(wPn))//载波抄表成功直接恢复
		{
			g_bPnFailFlg[wPn>>3] &= ~(1<<(wPn & 7));
			OnMtrErrRecv(wPn);		//抄表故障恢复
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
		pTmpV07.tRdLoadInfo.bNum = 1;//每次抄一个点
		//SecondsToTime((GetCurSec()-3600)/(15*60)*(15*60), &pTmpV07.tRdLoadInfo.tmStart);
		//pTmpV07.tRdLoadInfo.tmStart.nYear = 2011;
		//pTmpV07.tRdLoadInfo.tmStart.nMonth = 10;
		//pTmpV07.tRdLoadInfo.tmStart.nDay = 26;
		//pTmpV07.tRdLoadInfo.tmStart.nHour = 12;
		//pTmpV07.tRdLoadInfo.tmStart.nMinute = 0;
		memcpy(&pTmpV07.tRdLoadInfo.tmStart, time, sizeof(TTime));
		iRetLen = MtrReadFrz(pMtrPro, &pTmpV07, wIDs, pbRxBuf, 0);//《-4：该笔记录不存在 -1：不支持的ID 0:串口异常》
		
		return iRetLen;
	}
	if ((bFN >= 185 && bFN <= 188) || (bFN >= 193 && bFN <= 196)) //特殊格式特殊处理
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
			
			if (*pwID==0xa11f || *pwID==0xa12f || *pwID==0xb11f || *pwID==0xb12f)//这个几个ID补抄的是当前的数据
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
				
				if (iRet == -1) //抄回来的数据不用转换为协议格式
				{
					memcpy(pbRxBuf0, bRxBuf, wItemLen);
				}
				else if (iRet == -2) //抄回来的数据含有无效数据
				{
					DTRACE(DB_COMPENMTR, ("DirReadFrz : C2F%d ,DirRead Failed, Data invalid!! \n",bFN));
					return -3;
				}
			}
			else if (iRetLen == -1)//不支持的ID或者没有当天的数据
			{
				memcpy(pbRxBuf0, bRxBuf, wItemLen);//填充无效数据
			}
			else //抄表失败
			{
				DTRACE(DB_COMPENMTR, ("DirReadFrz : C2F%d ,DirRead Failed!! \n",bFN));
				return iRetLen; //有一个ID失败，则意味该FN补抄失败
			}

			if(!fSpec)
				pbRxBuf0 += wItemLen; //偏移长度
			
			wDataLen += wItemLen;
			pwID++;
		}

	}

	return wDataLen;
}

//描述：取得电表前3天日冻结的索引
bool GetMtrDayFrzIdx(struct TMtrPro* pMtrPro, TTime tmStart, BYTE* pbDayFrzIdx)
{
	TV07Tmp pTmpV07;
	int iRetLen = 0;
	TTime time;
	int i, j;
	BYTE bRxBuf[32];	
	BYTE bRecTime[3];
	
	memset(&pTmpV07, 0, sizeof(pTmpV07));

	for (i=0; i<8; i++) //搜寻上8(3+5)天的时标
	{
		memset(bRxBuf, 0, sizeof(bRxBuf));
		iRetLen = MtrReadFrz(pMtrPro, &pTmpV07, 0x9a00, bRxBuf, i);
		if (iRetLen <= 0)	//抄表失败，多抄一次
			iRetLen = MtrReadFrz(pMtrPro, &pTmpV07, 0x9a00, bRxBuf, i);
		
		if (iRetLen > 0)
		{
			for (j=0; j<3; j++)	//是否是所需要的前3天的时标：今天，前天，大前天
			{
				time = tmStart;
				AddIntervs(&time, TIME_UNIT_DAY, -j);
				TimeToFmt20(&time, bRecTime);
				if (memcmp(bRxBuf+2, bRecTime, 3)==0 && pbDayFrzIdx[j]==INVALID_DATA)
				{										//已经找到相应索引号的就不再更新，避免被更老的索引号更新
					pbDayFrzIdx[j] = i;
				}

				if (pbDayFrzIdx[0]!=INVALID_DATA && 
					pbDayFrzIdx[1]!=INVALID_DATA && 
					pbDayFrzIdx[2]!=INVALID_DATA)
					return true;
			}
		}
	}
	if (pbDayFrzIdx[0]!=INVALID_DATA || pbDayFrzIdx[1]!=INVALID_DATA || pbDayFrzIdx[2]!=INVALID_DATA)//只要有1天有数据，都去补抄
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
