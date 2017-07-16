/****************************************************************************************************
* Copyright (c) 2007,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DpStat.cpp
* 摘    要: 本文件实现2类数据统计类
* 当前版本：1.0
* 作    者：杨 凡、李湘平
* 完成日期：2007年8月
* 备    注：
****************************************************************************************************/
//#include "stdafx.h"
#include "FaCfg.h"
#include "DpStat.h"
#include "FaStruct.h"
#include "DbConst.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "Trace.h"
#include <math.h>
#include <stdio.h>
#include "MtrAPI.h"
#include "MtrExc.h"
#include "DbStruct.h"
#include "DbGbAPI.h"
#include "ComAPI.h"
#include "SysDebug.h"
#include "DbFmt.h"


#define DAY			0
#define MONTH			1
#define MIN_VAL			(-0x7fffffff)
#define MIN_VAL64		(-0x7fffffffffffffff)
#define MAX_VAL			(0x7fffffff)
#define MAX_VAL64		(0x7fffffffffffffff)

//////////////////////////////////////////////////////////////////////////////////////////
//描述:初始化函数
void DpInit(TLimitVolt* pLimitVolt, WORD wPn)
{	
    LoadLimitPara(pLimitVolt, wPn);
}


//描述:装载参数
void  LoadLimitPara(TLimitVolt* pLimitVolt, WORD wPn)
{
	BYTE iLimitData[58] = {0};
	int iLen = 0;
	if(wPn == 0)
	{
      if(GetAcPn() > 0)
      {
		  wPn = GetAcPn();
	  }
	  else
	  {
		  return;
	  }
	}

    iLen = ReadItemEx(BN0, wPn, 0x01af, iLimitData);	//F26,测量点限值参数
	if(iLen > 0)
	{
		pLimitVolt->wUpLimitVolt     = (WORD)Fmt7ToVal(iLimitData, 2);			//电压合格上限
		pLimitVolt->wLowLimitVolt    = (WORD)Fmt7ToVal(iLimitData+2, 2);			//电压合格下限
		pLimitVolt->wUpUpLimitVolt   = (WORD)Fmt7ToVal(iLimitData+6, 2);			//过压门限
		pLimitVolt->wLowLowLimitVolt = (WORD)Fmt7ToVal(iLimitData+11, 2);			//欠压门限

		pLimitVolt->wUnbalanceVoltLimit = (WORD)Fmt5ToVal(iLimitData+46, 2);
		pLimitVolt->wUnbalanceCurLimit =  (WORD)Fmt5ToVal(iLimitData+51, 2);
	}
}



//描述:数据统计总函数
void DoDpDataStat(TTime* ptmNow, TVoltStat* pDayStat, TVoltStat* pMonStat)
{
	//int iRet = 0;
    BYTE bVolt[2];
	//DWORD dwCurIntervS = 0;
	//DWORD dwIntervM = 0;
	DWORD dwCurM;
	//DWORD dwCurMin;
    //WORD   wValidNum;
    //TTime tmNow;
    TLimitVolt tLimitVolt;
        
	if (IsTimeEmpty(&pDayStat->tmLastRun) || IsInvalidTime(&pDayStat->tmLastRun))	//第一次运行或者统计数据结构被清除过
	{
		memset(pDayStat, 0, sizeof(TVoltStat));
		pDayStat->tmLastRun = *ptmNow;	//更新最后一次的运行时间
		DTRACE(DB_DP, ("DoDpDataStat: pDayStat->tmLastRun IsTimeEmpty!.###########\r\n"));
	}

	if (IsTimeEmpty(&pMonStat->tmLastRun) || IsInvalidTime(&pMonStat->tmLastRun))	//第一次运行或者统计数据结构被清除过
	{
		memset(pMonStat, 0, sizeof(TVoltStat));
		pMonStat->tmLastRun = *ptmNow;	//更新最后一次的运行时间
		DTRACE(DB_DP, ("DoDpDataStat: pMonStat->tmLastRun IsTimeEmpty!.###########\r\n"));
	}

	//GetCurTime(&tmNow);//可以在StatMgr.c中调用时带入当前时间
    //dwCurMin = TimeToMinutes(ptmNow);
    DpInit(&tLimitVolt, 0);//交采赋测量点值0
        
	dwCurM    = GetCurMinute();
	//dwIntervM = 1;//GetMeterInterv(GetAcPn());
	//dwCurM    = dwCurM;///dwIntervM*dwIntervM;
	memset(bVolt, 0x00, sizeof(bVolt)); 
		
	if(ReadItemEx(BN2, PN0, 0x1032, bVolt) < 0)
	    return ;
	if (IsDiffDay(&pDayStat->tmLastRun, ptmNow))
	{
		DTRACE(DB_DP, ("CDpStat:day change from %d to %d\n", DaysFrom2000(&pDayStat->tmLastRun), DaysFrom2000(ptmNow)));
		//DayChange(ptmNow, pDayStat, bVolt, dwCurM);	
		DayChange(pDayStat, bVolt, dwCurM);
	}

	if ( MonthFrom2000(ptmNow) != MonthFrom2000(&pMonStat->tmLastRun))
	{
		DTRACE(DB_DP, ("CDpStat:month change from %d to %d\n",MonthFrom2000(&pMonStat->tmLastRun), MonthFrom2000(ptmNow)));
		//MonthChange(ptmNow, pMonStat, bVolt, dwCurM);
		MonthChange(pMonStat, bVolt, dwCurM);
	}

	/*if (MinutesPast(&pDayStat->tmLastRun, ptmNow) > 2) //往前对时加上对时跨过的时间
		DoVoltStat(pDayStat, pMonStat, &tLimitVolt, bVolt, dwCurM, MinutesPast(&pDayStat->tmLastRun, ptmNow));
	else*/
		DoVoltStat(pDayStat, pMonStat, &tLimitVolt, bVolt, dwCurM, 1);//dwIntervM
	pDayStat->tmLastRun = *ptmNow;
	pMonStat->tmLastRun = *ptmNow;
}

void DataPro(TVoltStat* pStat, BYTE *pData, DWORD dwTime)
{
	WORD	wVoltTime = 0;
	DWORD	dwRate = 0;
	////////////////越界时间/////////////////
	if(pStat->wVoltUpUpTime >= dwTime)
		pStat->wVoltUpUpTime = dwTime;
	WordToByte(pStat->wVoltUpUpTime, pData);


	if(pStat->wVoltLowLowTime >= dwTime)
		pStat->wVoltLowLowTime = dwTime;
	WordToByte(pStat->wVoltLowLowTime, pData+2);


	if(pStat->wVoltUpTime >= dwTime)
		pStat->wVoltUpTime = dwTime;
	WordToByte(pStat->wVoltUpTime, pData+4);


	if(pStat->wVoltLowTime >= dwTime)
		pStat->wVoltLowTime = dwTime;
	WordToByte(pStat->wVoltLowTime, pData+6);


	if(pStat->wVoltTime >= dwTime)
		pStat->wVoltTime = dwTime;
	WordToByte(pStat->wVoltTime, pData+8);

	wVoltTime  =	pStat->wVoltUpTime + pStat->wVoltLowTime + pStat->wVoltTime;
	if(wVoltTime > dwTime)
		wVoltTime = dwTime;

	memset(pData+10, 0x00, 20);	
	/////////////////最值及发生时间///////////////
	DWORDToBCD(pStat->wMaxVolt, pData+30, 2);
	memcpy(pData+32 ,pStat->bTime, 3);
	DWORDToBCD(pStat->wMinVolt, pData+35, 2);
	memcpy(pData+37, pStat->bTime+3, 3);
	memset(pData+40, 0xee, 2);
	memset(pData+42, 0x00, 3);
	memset(pData+45, 0xee, 2);
	memset(pData+47, 0x00, 3);
	memset(pData+50, 0xee, 2);
	memset(pData+52, 0x00, 3);
	memset(pData+55, 0xee, 2);
	memset(pData+57, 0x00, 3);
	/////////////////平均值//////////////////////////
	DWORDToBCD(pStat->wAvrVolt, pData+60, 2);
	memset(pData+62, 0x00, 2);
	memset(pData+64, 0x00, 2+27);

	//////////////越限率//////////////////////////////
	if(wVoltTime > 0)
	{
		dwRate	=	pStat->wVoltUpTime*1000*100/wVoltTime;
		DWORDToBCD(dwRate, pData+66, 3);

		dwRate	=	pStat->wVoltLowTime*1000*100/wVoltTime;
		DWORDToBCD(dwRate, pData+69, 3);

		dwRate	=	1000*100 -  pStat->wVoltUpTime*1000*100/wVoltTime - dwRate;
		DWORDToBCD(dwRate, pData+72, 3);
	}
}

//描述:
//void DayChange(TTime* ptmNow, TVoltStat* pDayStat, BYTE *piData, DWORD dwCurMin)
void DayChange(TVoltStat* pDayStat, BYTE *piData, DWORD dwCurMin)
{
		BYTE bData[70+27];
		WORD	wVoltTime = 0;
		DWORD	dwRate = 0;
		TimeToFmt20(&pDayStat->tmLastRun, bData);
		bData[3] = GetAcPn()&0xff;

		DataPro(pDayStat, bData+4, 1440);
		/////////////////------>C2F27入库
		PipeAppend(TYPE_FRZ_TASK, 27, bData, 97);
		
		//////////初始化日统计数据///////////
		memset((BYTE*)&pDayStat->wVoltUpUpTime, 0, sizeof(TVoltStat)-sizeof(TTime));
		//pDayStat->wVoltUpUpTime = 0;
		//pDayStat->wVoltLowLowTime = 0;
		//pDayStat->wVoltUpTime = 0;
		//pDayStat->wVoltLowTime = 0;
		//pDayStat->wVoltTime = 0;
		pDayStat->wMaxVolt = (WORD)Fmt7ToVal(piData, 2);
		pDayStat->wMinVolt = (WORD)Fmt7ToVal(piData, 2);
		MinToFmt(dwCurMin, pDayStat->bTime, FMT18);
		MinToFmt(dwCurMin, pDayStat->bTime+3, FMT18);
		pDayStat->wAvrVolt = (WORD)Fmt7ToVal(piData, 2);
}


//void MonthChange(TTime* ptmNow, TVoltStat* pMonStat, BYTE *piData, DWORD dwCurMin)
void MonthChange(TVoltStat* pMonStat, BYTE *piData, DWORD dwCurMin)
{	
		BYTE bData[69+27];
		BYTE bDay = 0;
		WORD	wVoltTime = 0;
		DWORD	dwRate = 0;
		bDay = pMonStat->tmLastRun.nDay;

		TimeToFmt21(&pMonStat->tmLastRun, bData);
		bData[2] = GetAcPn()&0xff;
		
		DataPro(pMonStat, bData+3, 1440*bDay);
		/////////////////------>C2F27入库
		PipeAppend(TYPE_FRZ_TASK, 35, bData, 96);
		
		//////////初始化月统计数据///////////
		memset((BYTE*)&pMonStat->wVoltUpUpTime, 0, sizeof(TVoltStat)-sizeof(TTime));
		//pMonStat->wVoltUpUpTime = 0;
		//pMonStat->wVoltLowLowTime = 0;
		//pMonStat->wVoltUpTime = 0;
		//pMonStat->wVoltLowTime = 0;
		//pMonStat->wVoltTime = 0;
		pMonStat->wMaxVolt = (WORD)Fmt7ToVal(piData, 2);
		pMonStat->wMinVolt = (WORD)Fmt7ToVal(piData, 2);
		MinToFmt(dwCurMin, pMonStat->bTime, FMT18);
		MinToFmt(dwCurMin, pMonStat->bTime+3, FMT18);
		pMonStat->wAvrVolt = (WORD)Fmt7ToVal(piData, 2);	
}

//描述:日/月电压统计数据 C2F27   C2F35
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:		
void DoVoltStat(TVoltStat* pDayStat, TVoltStat* pMonStat, TLimitVolt* pLimitVolt, BYTE *piData, DWORD  dwCurMin, DWORD dwIntervM)
{
	//int	iRet = 0;
	WORD     wCurVolt = 0;
	bool fSetTm = false;
	if (IsAllAByte(piData, INVALID_DATA, 2))
		return;

	wCurVolt = (WORD)Fmt7ToVal(piData, 2);
        	
	if (wCurVolt==INVALID_VAL)
        return;
	if (dwIntervM > 2)
		fSetTm = true;
	//////////////////////////////////////越限////////////////////////////////////
    //越上限
    if (!fSetTm && wCurVolt>=pLimitVolt->wUpLimitVolt && pLimitVolt->wUpLimitVolt>0)
    {			
		//日时间累计
		pDayStat->wVoltUpTime += dwIntervM;

		 //月时间累计
		pMonStat->wVoltUpTime += dwIntervM;

		//越上上限
		if (wCurVolt>=pLimitVolt->wUpUpLimitVolt && pLimitVolt->wUpUpLimitVolt>0)
		{
		    pDayStat->wVoltUpUpTime += dwIntervM;
			pMonStat->wVoltUpUpTime += dwIntervM;
		}
    }
    else if(!fSetTm && wCurVolt<=pLimitVolt->wLowLimitVolt && pLimitVolt->wLowLimitVolt>0)
    {
        pDayStat->wVoltLowTime += dwIntervM;
        pMonStat->wVoltLowTime += dwIntervM;

        //越下下限
        if (wCurVolt<=pLimitVolt->wLowLowLimitVolt && pLimitVolt->wLowLowLimitVolt>0)  
        {
            pDayStat->wVoltLowLowTime += dwIntervM;
            pMonStat->wVoltLowLowTime += dwIntervM;
        }
    }
    else
    {
        //电压合格 
        pDayStat->wVoltTime += dwIntervM;  
        pMonStat->wVoltTime += dwIntervM;
    }
    //////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////最值/////////////////////////////////////
    if( wCurVolt > pDayStat->wMaxVolt)
    {
        pDayStat->wMaxVolt = wCurVolt;
        MinToFmt(dwCurMin, pDayStat->bTime, FMT18);
    }
    if (wCurVolt < pDayStat->wMinVolt) //0值会不断刷新时间
    {
        pDayStat->wMinVolt = wCurVolt;
        MinToFmt(dwCurMin, pDayStat->bTime+3, FMT18);
    }
    if ( 0 >= wCurVolt)
        MinToFmt(dwCurMin, pDayStat->bTime+3, FMT18);
	
    if(wCurVolt > pMonStat->wMaxVolt)
    {
        pMonStat->wMaxVolt = wCurVolt;
        MinToFmt(dwCurMin, pMonStat->bTime, FMT18);
    }
    if (wCurVolt < pMonStat->wMinVolt) //0值会不断刷新时间
    {
        pMonStat->wMinVolt = wCurVolt;
        MinToFmt(dwCurMin, pMonStat->bTime+3, FMT18);
    }

    if ( 0 >= wCurVolt)
        MinToFmt(dwCurMin, pMonStat->bTime+3, FMT18);
	/////////////////////////////////////////////////////////////////////////////
        
    //////////////////////////////////平均值//////////////////////////////////////		
    pDayStat->dwTotalVolt += wCurVolt;
    pMonStat->dwTotalVolt += wCurVolt;
    pDayStat->wTotalTimes	+=	1;
    pMonStat->wTotalTimes	+=	1;

    if(pDayStat->wTotalTimes>0 && pMonStat->wTotalTimes>0)
    {
        pDayStat->wAvrVolt = pDayStat->dwTotalVolt / pDayStat->wTotalTimes;
        pMonStat->wAvrVolt = pMonStat->dwTotalVolt / pMonStat->wTotalTimes;
    }
    /////////////////////////////////////////////////////////////////////////////////
	return;
}

///////////////////////////////////////////////////////////////////////电表测量点统计数据/////////////////////////////////////////////////////////////////////////////////

void MtrDayChange(WORD wPn, BYTE* pStatdata, BYTE *pbCurData, DWORD dwCurMin, TTime* ptmLast)
{
	//TTime tmTemp;
	BYTE bData[97];
	BYTE i;
	TimeToFmt20(ptmLast, bData);
	bData[3] = wPn&0xff;
	/////////////////////
	memcpy(bData+4, pStatdata, 66+27);
	//memcpy(bData+64, pbAvrStat, 6);
	//memcpy(bData+70, pbRateStat, 27);
	/////////////////------>C2F27入库
	if(IsFnSupport(wPn, 27, 2))
	{
	    PipeAppend(TYPE_FRZ_TASK, 27, bData, 97);
	}

	//////////初始化日统计数据///////////
	memset(pStatdata, 0x00, 66+27);
	for(i=0; i<3; i++)
	{
		if(IsAllAByte(pbCurData+(i<<1), INVALID_DATA, 2) || (GetConnectType(wPn)==CONNECT_3P3W && i==1))
			continue;
		memcpy(pStatdata+30+i*10, pbCurData+(i<<1), 2);
		MinToFmt(dwCurMin, pStatdata+30+i*10+2, FMT18);
		memcpy(pStatdata+30+i*10+5, pbCurData+(i<<1), 2);
		MinToFmt(dwCurMin, pStatdata+30+i*10+7, FMT18);
	}
}


void MtrMonChange(WORD wPn, BYTE* pStatdata, BYTE *pbCurData, DWORD dwCurMin, TTime* ptmLast)
{
	//TTime tmTemp;
	BYTE bData[96];
	BYTE i;
	TimeToFmt21(ptmLast, bData);
	bData[2] = wPn&0xff;
	/////////////////////
	memcpy(bData+3, pStatdata, 66+27);
	//memcpy(bData+63, pbAvrStat, 6);
	//memcpy(bData+69, pbRateStat, 27);
	/////////////////------>C2F35入库
	if(IsFnSupport(wPn, 35, 2))
	{
	    PipeAppend(TYPE_FRZ_TASK, 35, bData, 96);
	}

	//////////初始化日统计数据///////////
	memset(pStatdata, 0x00, 66+27);
	for(i=0; i<3; i++)
	{
		if(IsAllAByte(pbCurData+(i<<1), INVALID_DATA, 2) || (GetConnectType(wPn)==CONNECT_3P3W && i==1))
			continue;
		memcpy(pStatdata+30+i*10, pbCurData+(i<<1), 2);
		MinToFmt(dwCurMin, pStatdata+30+i*10+2, FMT18);
		memcpy(pStatdata+30+i*10+5, pbCurData+(i<<1), 2);
		MinToFmt(dwCurMin, pStatdata+30+i*10+7, FMT18);
	}
}


void DoDpMtrUStat(WORD wPn, BYTE* pbDayData, BYTE* pbMonData, DWORD* pdwTotalV, WORD* pwStatCnt, TTime* ptmNow, TTime* ptmLast)
{
	BYTE bVolt[6];//0xb61f
	DWORD dwCurM, dwCurMStar, dwTimeOut, dwLastMStat;
	DWORD dwRate = 0;
	BYTE  bInterv = 0;//这里用两次进入函数的时间差值
	BYTE bVarData[6];
	BYTE bRate[27];
	WORD wTemp, wTemp2;//这两个参数用来倒换数据
	BYTE i,j;
	BYTE *pData;
	TLimitVolt tLimitVolt;
	bool	fIsReadData = true;

	bInterv = GetMeterInterv(wPn);
	if(!IsMtrPn(wPn))
	{
		return;
	}

	if (IsTimeEmpty(ptmLast) || IsInvalidTime(ptmLast))	//第一次运行或者统计数据结构被清除过
	{
		memset(pbDayData, 0, 93);
		memset(pbMonData, 0, 93);
		ptmLast = ptmNow;	//更新最后一次的运行时间
		DTRACE(DB_DP, ("DoDpDataStat: wPn=%d  tmLastRun IsTimeEmpty!.###########\r\n", wPn));
		return ;
	}

	DpInit(&tLimitVolt, wPn);
	dwCurM     = TimeToMinutes(ptmNow);//GetCurMinute();
	dwCurMStar = dwCurM/bInterv*bInterv;
	//if((dwCurM - TimeToMinutes(ptmLast)) <= bInterv)
		//bInterv = dwCurM - TimeToMinutes(ptmLast);
	dwLastMStat = TimeToMinutes(ptmLast)/bInterv*bInterv;
	if (dwCurMStar == dwLastMStat)
		return ; //本间隔统计过了

	memset(bVolt, 0xee, sizeof(bVolt)); 

	wTemp2 = 0xb61f;
	if (QueryItemTimeMid(dwCurMStar*60, 0, BN0, wPn, &wTemp2, 1, bVolt, &wTemp) == 1)
	{
	    if(ReadItemMid(BN0, wPn, &wTemp2, 1, bVolt, dwCurMStar*60, INVALID_TIME) < 0)///导入导出的都在PN0吧
		{
			fIsReadData = false;
		    memset(bVolt, 0xee, sizeof(bVolt));
		}
	}
	else
	{
		fIsReadData = false;
		memset(bVolt, 0xee, sizeof(bVolt));
	}

	if (IsDiffDay(ptmLast, ptmNow))//换日
	{
		DTRACE(DB_DP, ("DoDpMtrDataStat:day change from %d to %d\n", DaysFrom2000(ptmLast), DaysFrom2000(ptmNow)));	 
		MtrDayChange(wPn, pbDayData, bVolt, dwCurM, ptmLast);
		memset((BYTE *)pdwTotalV, 0, 12);
		memset((BYTE *)pwStatCnt, 0, 6);
	}

	if ( MonthFrom2000(ptmNow) != MonthFrom2000(ptmLast))//换月
	{
		DTRACE(DB_DP, ("DoDpMtrDataStat:month change from %d to %d\n",MonthFrom2000(ptmLast), MonthFrom2000(ptmNow)));
		MtrMonChange(wPn, pbMonData, bVolt, dwCurM, ptmLast);
		memset((BYTE *)(pdwTotalV+3), 0, 12);
		memset((BYTE *)(pwStatCnt+3), 0, 6);
	}

	if(!fIsReadData)
	{
		DTRACE(DB_DP, ("DoDpMtrDataStat:Read CurData Err...\r\n"));
		return;
	}
 
	for(i=0; i<3; i++)//A\B\C
	{
		wTemp = (WORD)Fmt7ToVal(bVolt+(i<<1), 2); 
		if(IsAllAByte(bVolt+(i<<1), INVALID_DATA, 2) || (GetConnectType(wPn)==CONNECT_3P3W && i==1))
		{
			//MinToFmt(0, pData+32+i*10, FMT18);
			//MinToFmt(0, pData+37+i*10, FMT18);
			//*(pdwTotalV+i) += 0;
			//*(pdwTotalV+3+i) += 0;
			continue;
		}
		for(j=0; j<2; j++)//日月
		{
			if(0==j)
			{
				pData = pbDayData;
				dwTimeOut = 1440;
			}
			else if(1==j)
			{
				pData = pbMonData;
				dwTimeOut = ptmLast->nDay*1440;
			}
			else
				break;

			//////////////////////////////////////越限////////////////////////////////////
			//越上限
			if (wTemp>=tLimitVolt.wUpLimitVolt && tLimitVolt.wUpLimitVolt>0 )
			{			
				//时间累计
				wTemp2 = ByteToWord(pData+4+i*10) + bInterv;
				memcpy(pData+4+i*10, &wTemp2, 2);

				//越上上限
				if (wTemp>=tLimitVolt.wUpUpLimitVolt && tLimitVolt.wUpUpLimitVolt>0)
				{
					//时间累计
					wTemp2 = ByteToWord(pData+i*10) + bInterv;
					memcpy(pData+i*10, &wTemp2, 2);
				}
			}
			else if(wTemp<=tLimitVolt.wLowLimitVolt && tLimitVolt.wLowLimitVolt>0)
			{
				//日时间累计
				wTemp2 = ByteToWord(pData+6+i*10) + bInterv;
				memcpy(pData+6+i*10, &wTemp2, 2);

				//越下下限
				if (wTemp<=tLimitVolt.wLowLowLimitVolt &&tLimitVolt.wLowLowLimitVolt>0)  
				{
					//时间累计
					wTemp2 = ByteToWord(pData+2+i*10) + bInterv;
					memcpy(pData+2+i*10, &wTemp2, 2);
				}
			}
			else
			{
				//时间累计
				wTemp2 = ByteToWord(pData+8+i*10) + bInterv;
				memcpy(pData+8+i*10, &wTemp2, 2);
			}
			//////////////////////////////////////////////////////////////////////////////////////

			////////////////////////////////////////////最值/////////////////////////////////////
			if( wTemp>BcdToDWORD(pData+30+i*10, 2) || IsAllAByte(pData+30+i*10, INVALID_DATA, 2))
			{
				DWORDToBCD(wTemp, pData+30+i*10, 2);
				MinToFmt(dwCurM, pData+32+i*10, FMT18);
			}
			if (wTemp < BcdToDWORD(pData+35+i*10, 2)  || IsAllAByte(pData+35+i*10, INVALID_DATA, 2)) //0值会不断刷新时间
			{
				DWORDToBCD(wTemp, pData+35+i*10, 2);
				MinToFmt(dwCurM, pData+37+i*10, FMT18);
			}
			if ( 0 >= wTemp)
				MinToFmt(dwCurM, pData+37+i*10, FMT18);

			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////累加值最后入库时计算平均值与越限值//////////////////	
			*(pdwTotalV+3*j+i) += wTemp;
			*(pwStatCnt+3*j+i) += 1; //统计次数累加
			/////////////////////////////////////////////////////////////////////////////
			wTemp2 = 0;
			wTemp2 = ByteToWord(pData+4+i*10) + ByteToWord(pData+6+i*10) + ByteToWord(pData+8+i*10);
			if(wTemp2 > dwTimeOut)
				wTemp2 = dwTimeOut;

			if(wTemp2 > 0)
			{
				dwRate = ByteToWord(pData+4+i*10)*1000*100/wTemp2;
				ValToFmt25(dwRate, pData+66+i*9, 3);

				dwRate = ByteToWord(pData+6+i*10)*1000*100/wTemp2;
				ValToFmt25(dwRate, pData+66+i*9+3, 3);

				dwRate = 1000*100 - ByteToWord(pData+4+i*10)*1000*100/wTemp2 - dwRate;
				ValToFmt25(dwRate, pData+66+i*9+6, 3);
			}

			wTemp2 = 0;
			if(*(pwStatCnt+3*j+i) > 0)
				wTemp2 = *(pdwTotalV+3*j+i)/(*(pwStatCnt+3*j+i));

			ValToFmt7(wTemp2, pData+60+2*i, 2);
		}
	}

	//ptmLast = ptmNow;不在此修改，在最后做完所有统计后修改时间
}

//描述:日电压电流不平衡度
void DoDpMtrUnBalanceStat(WORD wPn, BYTE* pbUnBalanceData,TTime* ptmNow, TTime* ptmLast)
{
	DWORD dwCurM;
	DWORD dwCurMStar, dwLastMStat;
	BYTE  bInterv = 0;
	BYTE bCalcType;
	BYTE  i;
	BYTE bData[18];
	BYTE bCurData[15];
	WORD wAvrV, wAvrI;
	WORD wTempA, wTempB, wTempC;
	WORD wID[2] = {0xb61f, 0xb62f};
	WORD wTemp;
	BYTE *pData;
	//TTime tm;
	TLimitVolt tLimitVolt;
	//bool	fIsReadData = true;

	bInterv = GetMeterInterv(wPn);

	if(!IsMtrPn(wPn))
	{
		return;
	}

	if (IsTimeEmpty(ptmLast) || IsInvalidTime(ptmLast))	//第一次运行或者统计数据结构被清除过
	{
		memset(pbUnBalanceData, 0, 14);
		//memset(pbMonData, 0, 60);
		//ptmLast = ptmNow;	//更新最后一次的运行时间
		DTRACE(DB_DP, ("DoDpMtrUnBalanceStat: wPn=%d  tmLastRun IsTimeEmpty!.###########\r\n", wPn));
		return ;
	}

	DpInit(&tLimitVolt, wPn);
	dwCurM     = TimeToMinutes(ptmNow);//GetCurMinute();
	dwCurMStar = dwCurM/bInterv*bInterv;
	//if((dwCurM - TimeToMinutes(ptmLast)) <= bInterv)
		//bInterv = dwCurM - TimeToMinutes(ptmLast);
	dwLastMStat = TimeToMinutes(ptmLast)/bInterv*bInterv;
	if (dwCurMStar == dwLastMStat)
		return ; //本间隔统计过了
 
	if (QueryItemTimeMid(dwCurMStar*60, 0, BN0, wPn, wID, 2, bCurData, &wTempA) == 2)
	{
		if(ReadItemMid(BN0, wPn, wID, 2, bCurData, dwCurMStar*60, INVALID_TIME) < 0)///导入导出的都在PN0吧
		{
			//fIsReadData = false;
			memset(bCurData, 0xee, sizeof(bCurData));
		}
	}
	else
	{
		//fIsReadData = false;
		memset(bCurData, 0xee, sizeof(bCurData));
	}


	if (IsDiffDay(ptmLast, ptmNow))//换日
	{
		DTRACE(DB_DP, ("DoDpMtrUnBalanceStat:day change from %d to %d\n", DaysFrom2000(ptmLast), DaysFrom2000(ptmNow)));
		//tm = *ptmNow;
		//AddIntervs(&tm, TIME_UNIT_DAY, -1);
		//TimeToFmt20(&tm, bData);
		TimeToFmt20(ptmLast, bData);
		bData[3] = wPn&0xff;
		/////////////////////
		memcpy(bData+4, pbUnBalanceData, 14);
		/////////////////------>C2F28入库
		if(IsFnSupport(wPn, 28, 2))
		{
			PipeAppend(TYPE_FRZ_TASK, 28, bData, 18);
		}
		memset(pbUnBalanceData, 0, 4);//前四个字节为累计时间
	}

	ReadItemEx(BN24, PN0, 0x5009, &bCalcType);
	wAvrI = 0;
	wAvrV = 0;
	for(i=0; i<3; i++)
	{
		if(GetConnectType(wPn)==CONNECT_3P3W) //台体测试判断电表的要算B相电压
		{
			if(1==i)
			{
				if((1==bCalcType) || IsAllAByte(bCurData+2*i, INVALID_DATA, 2))
				   wAvrV += 0;
				else
					wAvrV += (WORD )Fmt7ToVal(bCurData+2*i, 2);

				wAvrI += 0;
			}	
			else
			{
				if(IsAllAByte(bCurData+2*i, INVALID_DATA, 2))
				{
					wAvrV += 0;
				}
				else
					wAvrV += (WORD )Fmt7ToVal(bCurData+2*i, 2);

				if(IsAllAByte(bCurData+6+ILEN*i, INVALID_DATA, ILEN))
				{
					wAvrI += 0;
				}
				else
					wAvrI += (WORD )ABS(FmtCurToVal(bCurData+6+ILEN*i, ILEN)); //2个字节，2个小数位
			}
		}
		else
		{
			if(IsAllAByte(bCurData+2*i, INVALID_DATA, 2))
			{
				wAvrV += 0;
			}
			else
				wAvrV += (WORD )Fmt7ToVal(bCurData+2*i, 2);

			if(IsAllAByte(bCurData+6+ILEN*i, INVALID_DATA, ILEN))
			{
				wAvrI += 0;
			}
			else
				wAvrI += (WORD )ABS(FmtCurToVal(bCurData+6+ILEN*i, ILEN)); //2个字节，2个小数位
		}
	}

	if(GetConnectType(wPn) == CONNECT_3P3W)
	{
		if (1==bCalcType)
			wAvrV /= 2;
		else
			wAvrV /= 3;
		wAvrI /= 2;
	}
	else
	{
		wAvrV /= 3;
		wAvrI /= 3;  		
	}

	wTemp = 0;
	wTempA = 0;
	wTempB = 0;
	wTempC = 0;
	for(i=0; i<2; i++)//电压、电流
	{
		if(1 == i)
		{
			pData = bCurData;
			if(0 != wAvrV)//出现0值时全部赋值为0
			{
				wTemp = wAvrV;
				wTempA  = (WORD)(abs(Fmt7ToVal(pData, 2) - wTemp) * 1000/wTemp);
				if((GetConnectType(wPn)==CONNECT_3P3W) && 1==bCalcType) //台体测试判断电表的要算B相电压
				{
					wTempB  =  0;//(WORD)(abs(ByteToWord(pData+2) - wTemp)* 1000/wTemp);
				}	
				else
				{
					wTempB  =  (WORD)(abs(Fmt7ToVal(pData+2, 2) - wTemp)* 1000/wTemp);
				}
				wTempC  =  (WORD)(abs(Fmt7ToVal(pData+4, 2) - wTemp)* 1000/wTemp);
			}
		}
		else if(0 == i)
		{
			pData = bCurData+6;
			if(0 != wAvrI)
			{
				wTemp = wAvrI;
				wTempA  = (WORD)(abs(ABS(FmtCurToVal(pData, ILEN)) - wTemp) * 1000/wTemp);
				if(GetConnectType(wPn)==CONNECT_3P3W)
					 wTempB  =  0;
				else
					 wTempB  =  (WORD)(abs(ABS(FmtCurToVal(pData+ILEN, ILEN)) - wTemp)* 1000/wTemp);

				wTempC  =  (WORD)(abs(ABS(FmtCurToVal(pData+2*ILEN, ILEN)) - wTemp)* 1000/wTemp);
			}
		}
		else
		{
			break;
		}

		
		wTemp = (wTempA > wTempB)? ((wTempA > wTempC)? wTempA : wTempC) : ((wTempB > wTempC)? wTempB : wTempC); 
		if (IsDiffDay(ptmLast, ptmNow))//换日时初始化最大值
		{
			ValToFmt5(wTemp, pbUnBalanceData+4+i*5, 2);
			MinToFmt(dwCurM,  pbUnBalanceData+4+i*5+2, FMT18);
		}

		if(wTemp > Fmt5ToVal(pbUnBalanceData+4+i*5, 2))
		{
			ValToFmt5(wTemp, pbUnBalanceData+4+i*5, 2);
			MinToFmt(dwCurM,  pbUnBalanceData+4+i*5+2, FMT18);
		}
		if(1==i && wTemp>tLimitVolt.wUnbalanceVoltLimit && tLimitVolt.wUnbalanceVoltLimit>0)
		{
			wTempA = ByteToWord(pbUnBalanceData+2) + bInterv;
			memcpy(pbUnBalanceData+2, &wTempA, 2);
		}

		if(0==i && wTemp>tLimitVolt.wUnbalanceCurLimit && tLimitVolt.wUnbalanceCurLimit>0)
		{
			wTempA = ByteToWord(pbUnBalanceData) + bInterv;
			memcpy(pbUnBalanceData, &wTempA, 2);
		}
	}

}


//描述:日月最大需量及发生时间
void DoDpMtrDemandStat(WORD wPn, BYTE* pbDayData, BYTE* pbMonData, TTime* ptmNow, TTime* ptmLast)
{
	BYTE		i,j;
	DWORD		dwDemond	= 0;
	BYTE        bDemond[28];
	BYTE        bCurData[12];
	WORD  wID = 0xb63f;
	WORD  wVaildNum;
	DWORD dwCurM;
	DWORD dwCurMStar, dwLastMStat;
	BYTE  bInterv = 0;  
	BYTE* pData;

	if(!IsMtrPn(wPn))
	{
		return;
	}

	if (IsTimeEmpty(ptmLast) || IsInvalidTime(ptmLast))	//第一次运行或者统计数据结构被清除过
	{
		memset(pbDayData, 0, 24);
		memset(pbMonData, 0, 24);
		//ptmLast = ptmNow;	//更新最后一次的运行时间
		DTRACE(DB_DP, ("DoDpMtrDemandStat: wPn=%d  tmLastRun IsTimeEmpty!.###########\r\n", wPn));
		return ;
	}

	dwCurM    = TimeToMinutes(ptmNow);//GetCurMinute();
	bInterv   = GetMeterInterv(wPn);
	dwCurMStar = dwCurM/bInterv*bInterv;
	dwLastMStat = TimeToMinutes(ptmLast)/bInterv*bInterv;
	if (dwCurMStar == dwLastMStat)
		return ; //本间隔统计过了

	if (QueryItemTimeMid(dwCurMStar*60, 0, BN0, wPn, &wID, 1, bCurData, &wVaildNum) == 1)
	{
		if(ReadItemMid(BN0, wPn, &wID, 1, bCurData, dwCurMStar*60, INVALID_TIME) < 0)///导入导出的都在PN0吧
		{
			memset(bCurData, 0xee, sizeof(bCurData));
			//return ;
		}
	}
	else
	{
		memset(bCurData, 0xee, sizeof(bCurData));
		//return ;
	}

	if (IsDiffDay(ptmLast, ptmNow))//换日
	{
		DTRACE(DB_DP, ("DoDpMtrDemandStat:day change from %d to %d\n", DaysFrom2000(ptmLast), DaysFrom2000(ptmNow)));
		TimeToFmt20(ptmLast, bDemond);
		bDemond[3] = wPn&0xff;
		/////////////////////
		memcpy(bDemond+4, pbDayData, 24);
		/////////////////------>C2F26入库
		if(IsFnSupport(wPn, 26, 2))
		{
		   PipeAppend(TYPE_FRZ_TASK, 26, bDemond, 28);
		}
		//memset(pbDayData, 0, 24);
	}

	if ( MonthFrom2000(ptmNow) != MonthFrom2000(ptmLast))//换月
	{
		DTRACE(DB_DP, ("DoDpMtrDemandStat:month change from %d to %d\n",MonthFrom2000(ptmLast), MonthFrom2000(ptmNow)));
		TimeToFmt21(ptmLast, bDemond);
		bDemond[2] = wPn&0xff;
		/////////////////////
		memcpy(bDemond+3, pbMonData, 24);
		/////////////////------>C2F34入库
		if(IsFnSupport(wPn, 34, 2))
		{
		   PipeAppend(TYPE_FRZ_TASK, 34, bDemond, 27);
		}
		//memset(pbMonData, 0, 24);
	}


	for( i=0; i<4; i++ )
	{
		for(j=0; j<2; j++)
		{
			if(0==j)
			{
				pData = pbDayData;
			}
			else
			{
				pData = pbMonData;
			}

			if (0==j && IsDiffDay(ptmLast, ptmNow))//换日
			{
				memcpy(pData+6*i, bCurData+3*i, 3);
				MinToFmt(dwCurM, pData+6*i+3, FMT18);
			}
			else if(1==j && MonthFrom2000(ptmNow)!=MonthFrom2000(ptmLast))//换月
			{
				memcpy(pData+6*i, bCurData+3*i, 3);
				MinToFmt(dwCurM, pData+6*i+3, FMT18);
			}

			//当前值无效 本次不统计
			if (IsAllAByte(bCurData+3*i, INVALID_DATA, 3 ) || (GetConnectType(wPn)==CONNECT_3P3W && i==2))
			{
				//MinToFmt(dwCurM, pData+6*i+3, FMT18);
				continue;
			}
			dwDemond = BcdToDWORD(bCurData+3*i, 3);

			//日最大有功需量统计 上一次统计值如果为无效则直接用当前值替换（换日、月时可能将无效值填入）
			if(dwDemond>abs(Fmt23ToVal64(pData+6*i, 3)) || IsAllAByte(pData+6*i, INVALID_DATA, 3 ))
			{
				Val64ToFmt23(dwDemond, pData+6*i, 3);
				MinToFmt(dwCurM, pData+6*i+3, FMT18);
			}
		}
	}
}

