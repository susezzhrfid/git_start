/*********************************************************************************************************
 * Copyright (c) 2013,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AutoSend.c
 * 摘    要：本文件主要实现主动上送任务
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2013年3月
*********************************************************************************************************/
#include "GbPro.h"
#include "SysDebug.h"
#include "DbGbAPI.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "ExcTask.h"
#include "ProAPI.h"
#include "MtrCtrl.h"
#include "LibDbStruct.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "DbHook.h"
#include "DbFmt.h"
#include "CommonTask.h"
#include "SysApi.h"
#include "FileMgr.h"
#include "FlashMgr.h"
#include "EsamCmd.h"
#include "AutoSend.h"
#include "AutoSendTask.h"
#include "DbSgAPI.h"
//////////////////////////////////////////////////////////////////
/*
告警主动上报屏蔽字，其二进制值从
最低位0到最高位255分别对应
报警编号E2000001H~E20000FFH
的报警，0屏蔽，1不屏蔽。
*/
bool IsAlrReport(DWORD dwAlrID)
{
	BYTE bAlrMask[32];
	BYTE bModeTest;
	WORD n;
	if (dwAlrID<0xe200001 || dwAlrID>0xe200004e)
		return false;

	n = dwAlrID - 0xe2000001;

	ReadItemEx(BN0, PN0, 0x8033, bAlrMask);

	if (bAlrMask[n/8] & (1<<n%8))   //1不屏蔽
	{
		// 		DTRACE(DB_TASK, ("%s : alr 0x%04x report enable.\r\n", __FUNCTION__, dwAlrID));
		return true;
	}
	else
	{
		// 		DTRACE(DB_TASK, ("%s : alr 0x%04x report disable.\r\n", __FUNCTION__, dwAlrID));
		return false;
	}
}

int MakeClass3Frm(struct TPro* pPro, BYTE bEc)
{
	WORD wTxPtr;
	BYTE bCmd, bAFN;
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	//int iLen;	

	bAFN = SG_AFUN_ASKCLASS3;
	//FnPnToBytes(1, 0, pbTx);
	pbTx += 4;
	*pbTx++ = bEc; //起始指针
	*pbTx++ = bEc+1; //结束指针
	wTxPtr = SG_LOC_DA1 + 6;

	bCmd = SG_LFUN_ASKN2DATA;
	bCmd |= SG_CTL_FCV;

	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;
	SgMakeMasterReqFrm(pPro, true, bCmd, bAFN, wTxPtr);

	wTxPtr += 2;	//校验位和结束符
	return wTxPtr;
}

//描述：告警上报
bool DoRptAlarm(struct TPro* pPro)
{
	//检测读写指针并组帧
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = &pSgPro->pbTxBuf[SG_LOC_DA1];
	int iRet;
	WORD wNewAlarmNo, wLastAlarmNo, wAlarmNum;
	WORD i, j = 0;
	BYTE bTmpBuf[280];
	DWORD dwClcik, dwLastClick;

	//举例：如果总告警数为6，那么wAlarmNum为6，告警指针为0~5，如果存了6笔都存了，新告警指针指向6
	ExFlashRdData(FADDR_ALRREC+2, NULL, (BYTE* )&wNewAlarmNo, 2);			//取新的告警指针，存入wNewAlarmNo
	ReadItemEx(BN24, PN0, 0x5030, (BYTE* )&wLastAlarmNo);					//取上次读告警指针，存入wLastAlarmNo
	ExFlashRdData(FADDR_ALRREC, NULL, (BYTE* )&wAlarmNum, 2);				//取总共存入的告警数，存入wAlarmNum

	if (wNewAlarmNo > wAlarmNum)									//如果新的告警指针超过总告警数，返回false
		return false;

	if ((wAlarmNum>500)||(wNewAlarmNo>500))
		return false;

	while (wNewAlarmNo != wLastAlarmNo)
	{
		pSgPro->fRptState = true;									//告警数据主动上送状态

		ReadOneAlr(FADDR_ALRREC, bTmpBuf, wLastAlarmNo);			//读出wLastAlarmNo对应的记录到bTmpBuf

		wLastAlarmNo += 1;
		if ((wLastAlarmNo >= 500))									//超过最大告警数500，回到第一条告警
			wLastAlarmNo = 0;
		WriteItemEx(BN24, PN0, 0x5030, (BYTE* )&wLastAlarmNo);		//保存这次读告警指针

		if (!IsAlrReport(ByteToDWORD(bTmpBuf+3, 4)))				//该告警屏蔽字为为0，则不上报，继续搜索下一条
		{
			pSgPro->fRptState = false;								//告警数据主动上送状态调整回false
			continue;
		}
		
   		pbTx -= j;
		PnToBytes(ByteToWord(bTmpBuf+1), bTmpBuf+1);
		j = 2+4+GetFmtARDLen(ByteToDWORD(bTmpBuf+3, 4));
		memcpy(pbTx, bTmpBuf+1, j);
		pbTx += j;

		i = 3;
REPORT_ALARM_FRM:
		dwLastClick = GetClick();
		pSgPro->pbTxBuf[SG_LOC_SEQ] = (pSgPro->pbTxBuf[SG_LOC_SEQ]++) & SG_SEQ_SEQ | SG_SEQ_CON | SG_SEQ_FIN | SG_SEQ_FIR;//单帧需确认
		if (!((ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) >= 0xE2000001) && (ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) <= 0xE200004E)))
		{
			DTRACE(DB_FAPROTO, ("DoRptAlarm auotosend id=%x wrong, exit!\r\n", ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4)));
			pSgPro->fRptState = false;									//告警数据主动上送状态调整回false
			return false;
		}

		SgMakeTxFrm(pPro, true, SG_LFUN_ASKNOREPLY, SG_AFUN_ASKALARM, (WORD)(pbTx - pSgPro->pbTxBuf));
        SetLedCtrlMode(LED_ALARM, LED_MODE_OFF);   //告警灯        
		pSgPro->fRptState = false;									//告警数据主动上送状态调整回false
		i -= 1;

		do{
			iRet = pPro->pfnRcvFrm(pPro);
			if (iRet > 0)
			{
				if (pSgPro->pbRxBuf[SG_LOC_AFN]==SG_AFUN_CONFIRM)
				{
					i = 3;
					break;
				}
				else
					dwClcik = GetClick();
			}
			else
				dwClcik = GetClick();

		}while (dwClcik-dwLastClick < 20);
		
		if (i==0)													//上报三次都没收到回确认帧，保存一下此告警，上报下一个告警帧
		{
			WriteAlrRec(FADDR_RPTFAILREC, bTmpBuf+1, j);
			break;													//一次进来处理一个告警，给联网失败处理时间
		}
		else if (i<3)												//告警上报不够3次，继续上报
		{
			pSgPro->fRptState = true;								//告警数据主动上送状态
			goto REPORT_ALARM_FRM;
		}
	}

	return true;
}

static BYTE g_bTaskNo = 0;
static BYTE g_bFwdTaskNo = 0;
//备注：本函数在通信线程调用，
//		1、对于2类数据，直接查询任务库，对通信线程的阻塞问题不大；
//		2、对于1类数据的主动上送任务，抄表数据应该在MtrRdThread()中自动完成，缓存在每个测量点的当前数据中
//		   上送任务执行的时候，把每个测量点的数据导入即可
bool GbAutoSend(struct TPro* pPro)
{
	static DWORD dwLastClick = 0;
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE bCommonTaskNum = 0, bFwdTaskNum = 0;

	//检测总开关,判断是否允许上报
	/*if ( !IsAdmitReport() )
		return false;*/

	if (GetClick()-dwLastClick < 3)	 //经常访问外部FLASH，数据量比较大，不要频繁执行
		return true;

	if (pGbPro->wRxStep!=0 && GetClick()-pGbPro->dwRxClick<3)	//之前的接收帧还在收，不便插入上送
		return true;

	//DoRptAlr(pPro);
	DoRptAlarm(pPro);

	if (GetInfo(INFO_TASK_INIT))
		UpdTaskInfo();

	ReadItemEx(BN0, POINT0, 0x0b10, &bCommonTaskNum);	//当前的普通任务总数
	ReadItemEx(BN0, POINT0, 0x0b20, &bFwdTaskNum);		//当前的中继任务总数
	if (bCommonTaskNum == 0 || bFwdTaskNum == 0)
		DTRACE(DB_TASK, ("GbAutoSend bCommonTaskNum=%d, bFwdTaskNum=%d\n", bCommonTaskNum, bFwdTaskNum));

	if (bCommonTaskNum != 0)
	{
		while (g_bTaskNo < MAX_COMMON_TASK)//普通任务数
		{
			//if (!IsTaskExist(MAX_COMMON_TASK, i))
			//	continue;
				
			if (IsTaskValid(COMMON_TASK_TYPE, g_bTaskNo))
			{
				DoRptTask(COMMON_TASK_TYPE, g_bTaskNo, pPro);

				g_bTaskNo++;
				if (g_bTaskNo >= MAX_COMMON_TASK)
					g_bTaskNo=0;
				break;
			}

			g_bTaskNo++;
			if (g_bTaskNo >= MAX_COMMON_TASK)
			{
				g_bTaskNo=0;
				break;
			}
		}
	}

	
	if (bFwdTaskNum != 0)
	{
		while (g_bFwdTaskNo < MAX_FWD_TASK)//中继任务数
		{
			//if (!IsTaskExist(MAX_COMMON_TASK, i))
			//	continue;
					
			if (IsTaskValid(FWD_TASK_TYPE, g_bFwdTaskNo))
			{
				DoRptTask(FWD_TASK_TYPE, g_bFwdTaskNo, pPro);

				g_bFwdTaskNo++;
				if (g_bFwdTaskNo >= MAX_FWD_TASK)
					g_bFwdTaskNo=0;
				break;
			}

			g_bFwdTaskNo++;
			if (g_bFwdTaskNo >= MAX_FWD_TASK)
			{
				g_bFwdTaskNo=0;
				break;
			}
		}
	}
	
	dwLastClick = GetClick();
	return true;
}
