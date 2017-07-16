/*********************************************************************************************************
 * Copyright (c) 2014,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AutoSend.c
 * 摘    要：本文件主要实现主动上送任务
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2014年9月
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
#include "AutoSendTask.h"
#include "DbSgAPI.h"
//////////////////////////////////////////////////////////////////
//TRptCtrl g_ComRptCtrl[MAX_COMMON_TASK];	//普通任务
//TRptCtrl g_FwdCtrl[MAX_FWD_TASK];		//中继任务

void TraceSecsToTime(char *pStr, DWORD dwSecs)
{
	TTime t;
	SecondsToTime(dwSecs, &t);
	DTRACE(DB_TASK, ("%s",pStr));
	DTRACE(DB_TASK, (" %d-%02d-%02d %02d:%02d:%02d\r\n",t.nYear,t.nMonth,t.nDay,t.nHour,t.nMinute,t.nSecond));
}

bool IsTaskExist(BYTE bTaskType, BYTE bTaskNo)
{
	BYTE bCfgFlag[64];
	memset(bCfgFlag, 0, sizeof(bCfgFlag));
	ReadItemEx(BN0, PN0, 0x0b40, bCfgFlag);

	if (bTaskType==COMMON_TASK_TYPE)
		if (bCfgFlag[bTaskNo/8]&(1<<(bTaskNo%8)))
			return true;

	if (bTaskType==FWD_TASK_TYPE)
		if (bCfgFlag[bTaskNo/8+32]&(1<<(bTaskNo%8)))
			return true;
			
	return false;
}

//描述：计算单个普通任务每笔记录的长度
//参数：@bTaskNo普通任务号
//返回：每笔记录的长度
int GetComTaskPerDataLen(BYTE bTaskNo)
{
	BYTE wPn;
	WORD wID = 0x0b11;
	DWORD dwLen = 0;
	//BYTE bPnMask[256];
	WORD i, j, wPointNum = 0;
	//BYTE bTaskParaData[512];

	BYTE bNum;
	DWORD dwID;
	BYTE bFnNum;
	WORD wDataLen;

	bool fMtrPn = false;
	//BYTE bPort = 0;

	if (bTaskNo > 0xfe)
		return 0;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, 512);
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	if (IsAllAByte(g_ExFlashBuf, 0x00, 512))
	{
		SignalSemaphore(g_semExFlashBuf);		
		return 0;
	}

	if (g_ExFlashBuf[0] == 0)// || (g_ExFlashBuf[8] == 0))//任务无效或者无需上报
	{
		SignalSemaphore(g_semExFlashBuf);		
		return 0;
	}

	//memset(bPnMask, 0, sizeof(bPnMask));
	if ((g_ExFlashBuf[20] == 0xff) && (g_ExFlashBuf[21] == 0xff))//全部测量点
	{
		
		for (wPn = 1; wPn < PN_NUM; wPn++)
		{
			fMtrPn = IsPnValid(wPn);
			if (!fMtrPn)
				continue;
			
			if (!GetPnProp(wPn))
				continue;
			
			//bPort = GetPnPort(wPn);
			//if (bPort > 30)//非485表不支持任务
				//continue;
			
			wPointNum++;
		}
	}
	else//单个或按位组测量点
	{
		for (i = 0; i < g_ExFlashBuf[19]; i++)
		{
			if (g_ExFlashBuf[20+2*i] != 0 && g_ExFlashBuf[20+2*i+1] != 0)
			{
				for (j=0; j<8; j++)
				{
					
					if (g_ExFlashBuf[20+2*i] & (1<<j))
					{
						wPn = (g_ExFlashBuf[20+2*i+1]-1)*8+j+1;

						fMtrPn = IsPnValid(wPn);
						if (!fMtrPn)
							continue;
						
						if (!GetPnProp(wPn))
							continue;
						
						//bPort = GetPnPort(wPn);
						//if (bPort > 30)//非485表不支持任务
							//continue;
						wPointNum++;
					}
				}
			}
		}
	}

	//TraceBuf(DB_FAPROTO, "g_ExFlashBuf:", g_ExFlashBuf, 50);
	bFnNum = (BYTE)*(g_ExFlashBuf+20+g_ExFlashBuf[19]*2);
	for (bNum=0; bNum < bFnNum; bNum++)
	{
		dwID = ByteToDWORD(g_ExFlashBuf+20+g_ExFlashBuf[19]*2+1+bNum*4, 4);
		wDataLen = GetItemLenDw(BN0, dwID); 
		dwLen += wPointNum*wDataLen;
	}

	SignalSemaphore(g_semExFlashBuf);

	return dwLen;

}


//描述：计算单个普通任务每笔记录包含的PN FN个数
//参数：@bTaskNo普通任务号
//返回：每笔记录中的PN FN个数
bool GetOneTaskDataPFnNum(BYTE bTaskNo, WORD* pwPnNum, WORD* pwFnNum)
{
	BYTE wPn;
	WORD wID = 0x0b11;
	DWORD dwLen = 0;
	WORD i, j, wPointNum = 0;

	BYTE bFnNum;
	WORD wDataLen;
	bool fMtrPn = false;
	
	if (bTaskNo > 0xfe)
		return false;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, sizeof(g_ExFlashBuf));
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	if (IsAllAByte(g_ExFlashBuf, 0x00, 512))
	{
		SignalSemaphore(g_semExFlashBuf);		
		return false;
	}

	if (g_ExFlashBuf[0] == 0)// || (g_ExFlashBuf[8] == 0))//任务无效或者无需上报
	{
		SignalSemaphore(g_semExFlashBuf);		
		return false;
	}

	if ((g_ExFlashBuf[20] == 0xff) && (g_ExFlashBuf[21] == 0xff))//全部测量点
	{
		
		for (wPn = 1; wPn < PN_NUM; wPn++)
		{
			fMtrPn = IsPnValid(wPn);
			if (!fMtrPn)
				continue;
			
			if (!GetPnProp(wPn))
				continue;
						
			wPointNum++;
		}
	}
	else//单个或按位组测量点
	{
		for (i = 0; i < g_ExFlashBuf[19]; i++)
		{
			if (g_ExFlashBuf[20+2*i] != 0 && g_ExFlashBuf[20+2*i+1] != 0)
			{
				for (j=0; j<8; j++)
				{
					
					if (g_ExFlashBuf[20+2*i] & (1<<j))
					{
						wPn = (g_ExFlashBuf[20+2*i+1]-1)*8+j+1;

						fMtrPn = IsPnValid(wPn);
						if (!fMtrPn)
							continue;
						
						if (!GetPnProp(wPn))
							continue;
						
						wPointNum++;
					}
				}
			}
		}
	}

	//TraceBuf(DB_FAPROTO, "g_ExFlashBuf:", g_ExFlashBuf, 50);
	*pwFnNum = (BYTE)*(g_ExFlashBuf+20+g_ExFlashBuf[19]*2);
	*pwPnNum = wPointNum;
	SignalSemaphore(g_semExFlashBuf);

	return true;
}


//描述：计算单个中继任务每笔记录的长度
//参数：@bTaskNo普通任务号
//返回：每笔记录的长度
int GetFwdTaskPerDataLen(BYTE bTaskNo)
{
	return 255;
}

//描述：计算单个普通任务需要存储的记录笔数
//参数：@bTaskNo普通任务号
//返回：需要存储的记录笔数(0表示一直记录，非0表示需要记录的笔数)
DWORD GetComTaskRecNum(BYTE bTaskNo)
{
	BYTE bSmplIntervU,bSmplIntervV;
	DWORD dwRecNum = 0;
	//BYTE bTaskParaData[512];

	if (bTaskNo > 0xfe)
		return -1;

	//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	//memset(g_ExFlashBuf, 0x00, 512);
	//ReadItemEx(BN0, bTaskNo, 0x0b11, g_ExFlashBuf);	
	//dwRecNum = ByteToWord(&g_ExFlashBuf[17]);
	//SignalSemaphore(g_semExFlashBuf);

	//if (0 == dwRecNum)//永远执行 就返回最大值
	{
		bSmplIntervU = GetComTaskSmplIntervU(bTaskNo);
		bSmplIntervV = GetComTaskSmplInterV(bTaskNo);
		if (bSmplIntervV == 0)
			bSmplIntervV = 1;
		if (TIME_UNIT_MINUTE == bSmplIntervU) return 7*1440/bSmplIntervV;
		if (TIME_UNIT_HOUR == bSmplIntervU)   return 15*24;
		if (TIME_UNIT_DAY == bSmplIntervU)    return 31;
		if (TIME_UNIT_MONTH == bSmplIntervU)  return 12;
	}

	return dwRecNum;
}


//描述：计算单个中继任务需要存储的记录笔数
//参数：@bTaskNo普通任务号
//返回：需要存储的记录笔数(0表示一直记录，非0表示需要记录的笔数)
DWORD GetFwdTaskRecNum(BYTE bTaskNo)
{
	DWORD dwRecNum = 0;
	BYTE bSmplIntervU = 0, bSmplIntervV;	
	//BYTE bTaskParaData[512];
	if (bTaskNo > 0xfe)
		return -1;


	//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	//memset(g_ExFlashBuf, 0x00, 512);
	//ReadItemEx(BN0, bTaskNo, 0x0b21, g_ExFlashBuf);	
	//dwRecNum = ByteToWord(&g_ExFlashBuf[17]);
	//SignalSemaphore(g_semExFlashBuf);

	//if (0 == dwRecNum)//永远执行 就返回最大值
	{
		bSmplIntervU = GetFwdTaskSmplIntervU(bTaskNo);
		bSmplIntervV = GetFwdTaskSmplInterV(bTaskNo);
		if (bSmplIntervV == 0)
			bSmplIntervV = 1;
		if (TIME_UNIT_MINUTE == bSmplIntervU) return 7*1440/bSmplIntervV;
		if (TIME_UNIT_HOUR == bSmplIntervU)   return 15*24;
		if (TIME_UNIT_DAY == bSmplIntervU)    return 31;
		if (TIME_UNIT_MONTH == bSmplIntervU)  return 12;
	}
		
	return dwRecNum;
}

//描述：获取普通任务采集间隔单位
BYTE  GetComTaskSmplIntervU(BYTE bTaskNo)
{
	TComTaskCfg tComTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b11, g_ExFlashBuf);
	memcpy(&tComTaskRptCfg, g_ExFlashBuf, sizeof(TComTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_MINUTE_TASK) return TIME_UNIT_MINUTE;
	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_HOUR_TASK)   return TIME_UNIT_HOUR;
	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_DAY_TASK)    return TIME_UNIT_DAY;
	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)	 return TIME_UNIT_MONTH;		
	
}

//描述：获取中继任务采集间隔单位
BYTE  GetFwdTaskSmplIntervU(BYTE bTaskNo)
{
	TFwdTaskCfg tFwdTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b21, g_ExFlashBuf);
	memcpy(&tFwdTaskRptCfg, g_ExFlashBuf, sizeof(TFwdTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_MINUTE_TASK) return TIME_UNIT_MINUTE;
	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_HOUR_TASK)   return TIME_UNIT_HOUR;
	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_DAY_TASK)    return TIME_UNIT_DAY;
	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_MONTH_TASK)	 return TIME_UNIT_MONTH;		
}

//描述：获取普通任务采集周期
BYTE GetComTaskSmplInterV(BYTE bTaskNo)
{
	TComTaskCfg tComTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b11, g_ExFlashBuf);
	memcpy(&tComTaskRptCfg, g_ExFlashBuf, sizeof(TComTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	return tComTaskRptCfg.bComSampIntervV;
}


//描述：获取中继任务采集周期
BYTE GetFwdTaskSmplInterV(BYTE bTaskNo)
{
	TFwdTaskCfg tFwdTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b21, g_ExFlashBuf);
	memcpy(&tFwdTaskRptCfg, g_ExFlashBuf, sizeof(TFwdTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	return tFwdTaskRptCfg.bFwdSampIntervV;
}

//描述：判断普通任务是否有效
//参数：@bTaskNo普通任务号
//返回：有效则返回true;反之返回false;
bool IsTaskValid(BYTE bTaskType, BYTE bTaskNo)
{	
	WORD wID;
	bool fRet = true;
	//BYTE bTaskParaData[512];
	//char str[16];

	if (bTaskNo > 0xfe)
		return false;

	if (bTaskType == COMMON_TASK_TYPE)
		wID = 0x0b11;
	if (bTaskType == FWD_TASK_TYPE)
		wID = 0x0b21;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, 512);
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	//TraceBuf(DB_FAPROTO, "g_ExFlashBuf:", g_ExFlashBuf, 100);
	if (IsAllAByte(g_ExFlashBuf, 0x00, 512) || IsAllAByte(g_ExFlashBuf, 0xff, 512))
	{
		SignalSemaphore(g_semExFlashBuf);		
		//DTRACE(DB_TASK, ("bTaskType=%d, bTaskNo=%d Para InValidate.\n", bTaskType, bTaskNo));
		return false;
	}

	if (g_ExFlashBuf[0] == 0)// || g_ExFlashBuf[8] == 0)//任务无效或者无需上报
	{
		fRet = false;
		//DTRACE(DB_TASK, ("bTaskType=%d, bTaskNo=%d is InValidate.\n", bTaskType, bTaskNo));
	}

	SignalSemaphore(g_semExFlashBuf);


	return fRet;

}


//描述：中继任务参数转换为普通任务参数格式
//便获取数据读取接口以统一的方式处理数据读取
void FwdToComTaskPara(TComTaskCfg* pComTaskRptCfg, TFwdTaskCfg* pFwdTaskRptCfg)
{
	pComTaskRptCfg->bComTaskValid = pFwdTaskRptCfg->bFwdTaskValid;				//任务有效
	memcpy(pComTaskRptCfg->bComRptBasTime, pFwdTaskRptCfg->bFwdRptBasTime, 5);	//上报基准时间

	pComTaskRptCfg->bComRptIntervU = pFwdTaskRptCfg->bFwdRptIntervU;			//周期单位
	pComTaskRptCfg->bComRptIntervV = pFwdTaskRptCfg->bFwdRptIntervV;			//上报周期

	pComTaskRptCfg->bComDataStru = pFwdTaskRptCfg->bFwdFwdType;					//中继类型存到数据方式 
	memcpy(pComTaskRptCfg->bComSampBasTime, pFwdTaskRptCfg->bFwdSampBasTime, 5);//采样基准时间

	pComTaskRptCfg->bComSampIntervU = pFwdTaskRptCfg->bFwdSampIntervU;			//采样周期单位
	pComTaskRptCfg->bComSampIntervV = pFwdTaskRptCfg->bFwdSampIntervV;			//采样周期	

	pComTaskRptCfg->bComRatio = pFwdTaskRptCfg->bFwdRatio;						//抽取倍率
	pComTaskRptCfg->bComDoTaskTimes = pFwdTaskRptCfg->bFwdDoTaskTimes;			//任务执行次数
}


//将一个上送间隔内存在的数据的分段读取
//控制一次读取[dwStepTime, dwStartTime)区间内存在的数据最大不超过1KB
bool GetOneFrmStartEndTime(BYTE bTaskType, BYTE bTaskNo, DWORD* pdwStepTime, DWORD dwStartTime, DWORD dwEndTime, TComTaskCfg* pCfg, bool* pfOnce)
{
	DWORD dwRptSecondsU = 0;
	DWORD dwSampleSecondsU = 0;
	TTime tStepTime, tLastRptTime;
	WORD  wDataLen = 0, bRecNum = 0, bRecTmpNum = 0;
	WORD  wPnNum=0, wFnNum=0;

	*pfOnce = false;
	*pdwStepTime = dwStartTime;
	
	if((pCfg == NULL) || (pCfg->bComTaskValid == 0) || 
		(pCfg->bComRptIntervV == 0) || (pCfg->bComSampIntervV == 0) ||
		(dwStartTime > dwEndTime) || (pdwStepTime == NULL))
		return false;

	if (pCfg->bComRptIntervU == TIME_UNIT_MINUTE_TASK)dwRptSecondsU = 60;
	if (pCfg->bComRptIntervU == TIME_UNIT_HOUR_TASK)  dwRptSecondsU = 60*60;
	if (pCfg->bComRptIntervU == TIME_UNIT_DAY_TASK)   dwRptSecondsU = 60*60*24;
	if (pCfg->bComRptIntervU == TIME_UNIT_MONTH_TASK)   dwRptSecondsU = 60*60*24*30;		
	
	if (pCfg->bComSampIntervU == TIME_UNIT_MINUTE_TASK)dwSampleSecondsU = 60;
	if (pCfg->bComSampIntervU == TIME_UNIT_HOUR_TASK)  dwSampleSecondsU = 60*60;
	if (pCfg->bComSampIntervU == TIME_UNIT_DAY_TASK)   dwSampleSecondsU = 60*60*24;
	if (pCfg->bComSampIntervU == TIME_UNIT_MONTH_TASK)   dwSampleSecondsU = 60*60*24*30;		

	//采集间隔比上报间隔大
	if (dwRptSecondsU*pCfg->bComRptIntervV < dwSampleSecondsU*pCfg->bComSampIntervV)
	{
		SecondsToTime(*pdwStepTime, &tStepTime);
		if(dwSampleSecondsU == 60*60*24*30)//月     此处的30没有问题Don't worry
		{
			tStepTime.nDay = 1;
			tStepTime.nHour = 0;
			tStepTime.nMinute = 0;
			tStepTime.nSecond = 0;
		}
		else if(dwSampleSecondsU == 60*60*24)//日
		{
			tStepTime.nHour = 0;
			tStepTime.nMinute = 0;
			tStepTime.nSecond = 0;
		}
		else if(dwSampleSecondsU == 60*60)	//时
		{
			tStepTime.nMinute = 0;
			tStepTime.nSecond = 0;
		}
		else if (dwSampleSecondsU == 60)	//分
		{
			tStepTime.nSecond = 0;
		}

		//间隔单位相等时，报最近一个采集间隔的数据，此时可以不用考虑抽取倍率了
		if (pCfg->bComRptIntervU == pCfg->bComSampIntervU)
		{
				AddIntervsInTask(&tStepTime , pCfg->bComSampIntervU, -(pCfg->bComSampIntervV));
		}
		
		*pfOnce = true;
		*pdwStepTime = TimeToSeconds(&tStepTime);
		wDataLen = GetComTaskPerDataLen(bTaskNo);

		if (wDataLen == 0)
			return false;
	}
	else
	{
		//(1)计算出上一个上报间隔的起始时间,用dwStartTime  和上一个上报间隔起始时间
		//计算出这两个间隔冻结的数据点数bRecNum。
		//(2)计算tStepTime的值，计算方式为:tStepTime的初始值为本间隔上报起始时间，然后
		//tStepTime向以前退N 个采集间隔(N <= bRecNum)，只要N  个点的数据总长度超过1000Byte  就暂停，带
		//回的[pdwStepTime, dwStartTime)  区间就可用于读取任务中不超过1000Byte的数据。
		//(3)数据读取完成后将dwStartTime  步进到dwStepTime 后， 重复步骤(1)。

		SecondsToTime(dwEndTime, &tLastRptTime);		
		AddIntervsInTask(&tLastRptTime , pCfg->bComRptIntervU, -(pCfg->bComRptIntervV));
		//SecondsToTime(dwEndTime - BASE_TIME_DELAY*60, &tLastRptTime);		
		AddIntervsInTask(&tLastRptTime , pCfg->bComRptIntervU, -(pCfg->bComRptIntervV));

		if (dwRptSecondsU*pCfg->bComRptIntervV == dwSampleSecondsU*pCfg->bComSampIntervV)
			bRecNum = 1;
		else if(dwRptSecondsU*pCfg->bComRptIntervV > dwSampleSecondsU*pCfg->bComSampIntervV)
			bRecNum = (dwStartTime - TimeToSeconds(&tLastRptTime))/(dwSampleSecondsU*pCfg->bComSampIntervV*pCfg->bComRatio);

		if (dwStartTime <= TimeToSeconds(&tLastRptTime))
			return false;

		SecondsToTime(*pdwStepTime, &tStepTime);
		while ((wDataLen < 1000) && (bRecTmpNum < bRecNum))
		{
			bRecTmpNum++;
			if (bTaskType == FWD_TASK_TYPE)
				wDataLen += 255;//中继任务暂时 固定为255
			else if (bTaskType == COMMON_TASK_TYPE)
			{				
				wDataLen += GetComTaskPerDataLen(bTaskNo);				
				if (!GetOneTaskDataPFnNum(bTaskNo, &wPnNum, &wFnNum))
					return false;
				if (pCfg->bComDataStru == 0)			//自描述方式
					wDataLen += (3+wPnNum*wFnNum*11+6);	//描述方式(1Byte), 数据个数(2Byte), PN(2Byte)+dwID(4Byte)+Time(5Byte)=11Byte, Time(6Byte)
				else if (pCfg->bComDataStru == 1)		//按任务参数方式
					wDataLen += (3+wFnNum*2+6);			//描述方式(1Byte), PN(2Byte), Time(6Byte)
			}

			AddIntervsInTask(&tStepTime , pCfg->bComSampIntervU, -(pCfg->bComSampIntervV*pCfg->bComRatio));
			if(TimeToSeconds(&tStepTime) <= TimeToSeconds(&tLastRptTime))
				break;
		}

		//总长度大于1000Byte，每笔数据小于1000Byte  时dwStepTime 向以后推一个抽取点
		if (bTaskType == COMMON_TASK_TYPE)
			if((GetComTaskPerDataLen(bTaskNo) < 1000) && (bRecNum != 1) && (wDataLen > 1000))
				AddIntervsInTask(&tStepTime , pCfg->bComSampIntervU, pCfg->bComSampIntervV*pCfg->bComRatio);


	}
	
	if (wDataLen != 0)
	{
		*pdwStepTime = TimeToSeconds(&tStepTime);
		return true;
	}
		
	return false;
}

//模拟主站，组请求一般任务数据帧
int MakeComTaskReqRptFrm(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro, TComTaskCfg* pCfg, DWORD dwStepTime, DWORD dwStartTime)
{
	DWORD dwDI;
	BYTE bTime1[6], bTime2[6];

	TTime tStepTime;
	TTime tStartTime;

	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;

	SecondsToTime(dwStepTime, &tStepTime);	
	SecondsToTime(dwStartTime, &tStartTime);

	TimeToBcd(&tStepTime, bTime1);
	TimeToBcd(&tStartTime, bTime2);

	*pbTx++ = 0;			//DA1
	*pbTx++ = 0;			//DA2	
	pGbPro->wTxPtr += 2;

	if (bTaskType == COMMON_TASK_TYPE)
		dwDI = 0xe0000300;
	else if (bTaskType == FWD_TASK_TYPE)
		dwDI = 0xe0000400;

	dwDI += (bTaskNo+1);
	memcpy(pbTx, &dwDI, 4);	//DI
	pbTx += 4;
	pGbPro->wTxPtr += 4;

	memcpy(pbTx, bTime1, 6);//起始时间
	pbTx += 6;
	memcpy(pbTx, bTime2, 6);//结束时间
	pbTx += 6;
	pGbPro->wTxPtr += 12;

	*pbTx++ = 0;			//数据密度(0-表示按照任务配置的采样周期和抽取倍率抽取数据)
	pGbPro->wTxPtr++;
	SgMakeMasterReqFrm(pPro, true, SG_LFUN_DATAREPLY, SG_AFUN_ASKTASK, pGbPro->wTxPtr);
	TraceBuf(DB_FAPROTO, "MakeTaskReqFrm -->", pGbPro->pbTxBuf, pGbPro->wTxPtr+2);

	pGbPro->wTxPtr += 2;
	return pGbPro->wTxPtr;
}

void DoRptTaskData(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro, TComTaskCfg* pCfg, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen;
	bool fRet=true, fOnce = false;
	DWORD dwStepTime;
	DWORD dwStartTime1=dwStartTime;
	while(fRet)
	{		
		//以[dwStepTime, dwStartTime)为区间从前向后读取，每次读取的数据控制在1KB之内
		//读完后dwStartTime更新为上一次dwStepTime的时间，重新获取新的dwStepTime
		//然后读取新的[dwStepTime, dwStartTime)区间内的数据，fRet返回fasle  无数据可读
		fRet = GetOneFrmStartEndTime(bTaskType, bTaskNo, &dwStepTime, dwStartTime1, dwEndTime, pCfg, &fOnce);

		if (!fRet)
			break;

		//组一帧请求读普通任务数据的帧来模拟主站下发
		iLen = MakeComTaskReqRptFrm(bTaskType, bTaskNo, pPro, pCfg, dwStepTime, dwStartTime1);
		if (iLen > 0)
		{
			TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
			pGbPro->wRxStep = 0;		//把之前的接收清0

			pGbPro->fRptState = true;	//数据主动上送状态
			pPro->pfnRxFrm(pPro, pGbPro->pbTxBuf, iLen);
			pGbPro->fRptState = false;
			pGbPro->bMySEQ = (pGbPro->bMySEQ+1) & 0x0f; 
		}
		else
			break;

		if (fOnce)
			return;

		dwStartTime1 = dwStepTime;
	}
}


//返回：true:表示当前时间间隔到达， pdwStartTime返回本间隔起始时间， pdwEndTime返回本间隔结束时间；
//		false:表示当前时间间隔未到达
bool GetRptTimeScope(TComTaskCfg* pCfg, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime)
{
	int iPast;
	DWORD dwCurSec, dwStartSec;
	TTime tmStart, tmEnd;

	Fmt15ToTime(pCfg->bComRptBasTime, &tmStart);
	tmStart.nSecond = 0;

	iPast = TaskIntervsPast(&tmStart, pNow, pCfg->bComRptIntervU, pCfg->bComRptIntervV);	//从基准时间开始，经历的整数个周期个数iPast	
	if (iPast < 0)		//比基准时间早
		return false;

	//tmStart = pCfg->tmStart;	//基准时间
	AddIntervsInTask(&tmStart, pCfg->bComRptIntervU, pCfg->bComRptIntervV*iPast);		//加iPast个周期-〉当前间隔起始时间

	dwCurSec = TimeToSeconds(pNow);	//当前时间所在间隔的起始时标
	dwStartSec = TimeToSeconds(&tmStart);	//当前时间所在间隔的起始时标
	if (dwCurSec < dwStartSec)	//还没到当前间隔起始时间
		return false;
	

	//求结束时间
	tmEnd = tmStart;
	//AddIntervsInTask(&tmEnd, TIME_UNIT_MINUTE_TASK, BASE_TIME_DELAY);		//控制在从基准时间开始的整数倍周期+5分钟内上报
	//AddIntervs(&tmEnd, pCfg->bIntervU, pCfg->bIntervV);		//加1个周期时间-〉当前间隔结束时间
	AddIntervsInTask(&tmEnd, pCfg->bComRptIntervU, pCfg->bComRptIntervV);		//改为整个间隔周期内都可进入

	*pdwStartTime = TimeToSeconds(&tmStart);
	*pdwEndTime = TimeToSeconds(&tmEnd);
	return true;
}


bool DoRptTask(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro)
{
	WORD wID, wTaskTimesID, wTaskTimes=0, wTaskDoneTimeID;
	TTime tmNow;
	DWORD dwCurSec, dwStartTime, dwEndTime, dwRptDoneSecond;
	TComTaskCfg tComTaskRptCfg;
	TFwdTaskCfg tFwdTaskRptCfg;
	char str[16];

	if (bTaskType == COMMON_TASK_TYPE)
	{
		wID = 0x0b11;
		wTaskDoneTimeID = 0x0b35;
	}
	if (bTaskType == FWD_TASK_TYPE)
	{
		wID = 0x0b21;
		wTaskDoneTimeID = 0x0b36;
	}
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	if (bTaskType == COMMON_TASK_TYPE)
		memcpy(&tComTaskRptCfg, g_ExFlashBuf, sizeof(TComTaskCfg));
	else if (bTaskType == FWD_TASK_TYPE)
		memcpy(&tFwdTaskRptCfg, g_ExFlashBuf, sizeof(TFwdTaskCfg));
	SignalSemaphore(g_semExFlashBuf);
	
	if (bTaskType == FWD_TASK_TYPE)
		FwdToComTaskPara(&tComTaskRptCfg, &tFwdTaskRptCfg);//参数转换
	
	//WaitSemaphore(g_semEsam, SYS_TO_INFINITE);	

	GetCurTime(&tmNow);
	if (GetRptTimeScope(&tComTaskRptCfg, &tmNow, &dwStartTime, &dwEndTime))	//找到本时间间隔起止时间
	{
		dwCurSec = TimeToSeconds(&tmNow);
		if (dwCurSec >= dwStartTime && dwCurSec<dwEndTime)	//当前时间间隔到达且当前间隔未执行过
		{	
			ReadItemEx(BN0, bTaskNo, wTaskDoneTimeID, (BYTE *)&dwRptDoneSecond);			
			
			if (dwStartTime != dwRptDoneSecond)			//每个间隔只进入一次，一次报完上一个间隔所有数据
			{				
				//ReadItemEx(BN0, bTaskNo, wTaskTimesID, (BYTE*)&wTaskTimes);
				//if (wTaskTimes != 0)//任务执行次数
				//{
					//if(wTaskTimes > tComTaskRptCfg.bComDoTaskTimes)
					//	return true;
					//else
					//{
					//	wTaskTimes++;
					//	WriteItemEx(BN0, bTaskNo, wTaskTimesID, (BYTE*)&wTaskTimes);
					//}
				//}
			
				DoRptTaskData(bTaskType, bTaskNo, pPro, &tComTaskRptCfg, dwStartTime, dwEndTime);
				
				sprintf(str, (bTaskType == COMMON_TASK_TYPE)?("COM_TASK"):("FWD_TASK"));
				DTRACE(DB_TASK, ("\nAutoSendTask, %s, bTaskNo=%d\n", str, bTaskNo+1));
				TraceSecsToTime("ThisTime dwStartRptTime= ",dwStartTime);					
				TraceSecsToTime("LastTime dwDoneRptSecond=",dwRptDoneSecond);						
				
				dwRptDoneSecond = dwStartTime;
				WriteItemEx(BN0, bTaskNo, wTaskDoneTimeID, (BYTE *)&dwRptDoneSecond);
				
			}
		}
	}

	//SignalSemaphore(g_semEsam);
	return true;
}

