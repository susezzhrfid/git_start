/****************************************************************************************************
* Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�DpStat.cpp
* ժ    Ҫ: ���ļ�ʵ��2������ͳ����
* ��ǰ�汾��1.0
* ��    �ߣ��� ��������ƽ
* ������ڣ�2007��8��
* ��    ע��
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
//����:��ʼ������
void DpInit(TLimitVolt* pLimitVolt, WORD wPn)
{	
    LoadLimitPara(pLimitVolt, wPn);
}


//����:װ�ز���
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

    iLen = ReadItemEx(BN0, wPn, 0x01af, iLimitData);	//F26,��������ֵ����
	if(iLen > 0)
	{
		pLimitVolt->wUpLimitVolt     = (WORD)Fmt7ToVal(iLimitData, 2);			//��ѹ�ϸ�����
		pLimitVolt->wLowLimitVolt    = (WORD)Fmt7ToVal(iLimitData+2, 2);			//��ѹ�ϸ�����
		pLimitVolt->wUpUpLimitVolt   = (WORD)Fmt7ToVal(iLimitData+6, 2);			//��ѹ����
		pLimitVolt->wLowLowLimitVolt = (WORD)Fmt7ToVal(iLimitData+11, 2);			//Ƿѹ����

		pLimitVolt->wUnbalanceVoltLimit = (WORD)Fmt5ToVal(iLimitData+46, 2);
		pLimitVolt->wUnbalanceCurLimit =  (WORD)Fmt5ToVal(iLimitData+51, 2);
	}
}



//����:����ͳ���ܺ���
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
        
	if (IsTimeEmpty(&pDayStat->tmLastRun) || IsInvalidTime(&pDayStat->tmLastRun))	//��һ�����л���ͳ�����ݽṹ�������
	{
		memset(pDayStat, 0, sizeof(TVoltStat));
		pDayStat->tmLastRun = *ptmNow;	//�������һ�ε�����ʱ��
		DTRACE(DB_DP, ("DoDpDataStat: pDayStat->tmLastRun IsTimeEmpty!.###########\r\n"));
	}

	if (IsTimeEmpty(&pMonStat->tmLastRun) || IsInvalidTime(&pMonStat->tmLastRun))	//��һ�����л���ͳ�����ݽṹ�������
	{
		memset(pMonStat, 0, sizeof(TVoltStat));
		pMonStat->tmLastRun = *ptmNow;	//�������һ�ε�����ʱ��
		DTRACE(DB_DP, ("DoDpDataStat: pMonStat->tmLastRun IsTimeEmpty!.###########\r\n"));
	}

	//GetCurTime(&tmNow);//������StatMgr.c�е���ʱ���뵱ǰʱ��
    //dwCurMin = TimeToMinutes(ptmNow);
    DpInit(&tLimitVolt, 0);//���ɸ�������ֵ0
        
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

	/*if (MinutesPast(&pDayStat->tmLastRun, ptmNow) > 2) //��ǰ��ʱ���϶�ʱ�����ʱ��
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
	////////////////Խ��ʱ��/////////////////
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
	/////////////////��ֵ������ʱ��///////////////
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
	/////////////////ƽ��ֵ//////////////////////////
	DWORDToBCD(pStat->wAvrVolt, pData+60, 2);
	memset(pData+62, 0x00, 2);
	memset(pData+64, 0x00, 2+27);

	//////////////Խ����//////////////////////////////
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

//����:
//void DayChange(TTime* ptmNow, TVoltStat* pDayStat, BYTE *piData, DWORD dwCurMin)
void DayChange(TVoltStat* pDayStat, BYTE *piData, DWORD dwCurMin)
{
		BYTE bData[70+27];
		WORD	wVoltTime = 0;
		DWORD	dwRate = 0;
		TimeToFmt20(&pDayStat->tmLastRun, bData);
		bData[3] = GetAcPn()&0xff;

		DataPro(pDayStat, bData+4, 1440);
		/////////////////------>C2F27���
		PipeAppend(TYPE_FRZ_TASK, 27, bData, 97);
		
		//////////��ʼ����ͳ������///////////
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
		/////////////////------>C2F27���
		PipeAppend(TYPE_FRZ_TASK, 35, bData, 96);
		
		//////////��ʼ����ͳ������///////////
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

//����:��/�µ�ѹͳ������ C2F27   C2F35
//����:@piData		ָ�����������������ָ��,
//	   @iLen		������ĸ���
//	   @dwCurMin	��ǰʱ�䣬��λ ����
//	   @dwIntervM	���
//����:��
//��ע:		
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
	//////////////////////////////////////Խ��////////////////////////////////////
    //Խ����
    if (!fSetTm && wCurVolt>=pLimitVolt->wUpLimitVolt && pLimitVolt->wUpLimitVolt>0)
    {			
		//��ʱ���ۼ�
		pDayStat->wVoltUpTime += dwIntervM;

		 //��ʱ���ۼ�
		pMonStat->wVoltUpTime += dwIntervM;

		//Խ������
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

        //Խ������
        if (wCurVolt<=pLimitVolt->wLowLowLimitVolt && pLimitVolt->wLowLowLimitVolt>0)  
        {
            pDayStat->wVoltLowLowTime += dwIntervM;
            pMonStat->wVoltLowLowTime += dwIntervM;
        }
    }
    else
    {
        //��ѹ�ϸ� 
        pDayStat->wVoltTime += dwIntervM;  
        pMonStat->wVoltTime += dwIntervM;
    }
    //////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////��ֵ/////////////////////////////////////
    if( wCurVolt > pDayStat->wMaxVolt)
    {
        pDayStat->wMaxVolt = wCurVolt;
        MinToFmt(dwCurMin, pDayStat->bTime, FMT18);
    }
    if (wCurVolt < pDayStat->wMinVolt) //0ֵ�᲻��ˢ��ʱ��
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
    if (wCurVolt < pMonStat->wMinVolt) //0ֵ�᲻��ˢ��ʱ��
    {
        pMonStat->wMinVolt = wCurVolt;
        MinToFmt(dwCurMin, pMonStat->bTime+3, FMT18);
    }

    if ( 0 >= wCurVolt)
        MinToFmt(dwCurMin, pMonStat->bTime+3, FMT18);
	/////////////////////////////////////////////////////////////////////////////
        
    //////////////////////////////////ƽ��ֵ//////////////////////////////////////		
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

///////////////////////////////////////////////////////////////////////��������ͳ������/////////////////////////////////////////////////////////////////////////////////

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
	/////////////////------>C2F27���
	if(IsFnSupport(wPn, 27, 2))
	{
	    PipeAppend(TYPE_FRZ_TASK, 27, bData, 97);
	}

	//////////��ʼ����ͳ������///////////
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
	/////////////////------>C2F35���
	if(IsFnSupport(wPn, 35, 2))
	{
	    PipeAppend(TYPE_FRZ_TASK, 35, bData, 96);
	}

	//////////��ʼ����ͳ������///////////
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
	BYTE  bInterv = 0;//���������ν��뺯����ʱ���ֵ
	BYTE bVarData[6];
	BYTE bRate[27];
	WORD wTemp, wTemp2;//����������������������
	BYTE i,j;
	BYTE *pData;
	TLimitVolt tLimitVolt;
	bool	fIsReadData = true;

	bInterv = GetMeterInterv(wPn);
	if(!IsMtrPn(wPn))
	{
		return;
	}

	if (IsTimeEmpty(ptmLast) || IsInvalidTime(ptmLast))	//��һ�����л���ͳ�����ݽṹ�������
	{
		memset(pbDayData, 0, 93);
		memset(pbMonData, 0, 93);
		ptmLast = ptmNow;	//�������һ�ε�����ʱ��
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
		return ; //�����ͳ�ƹ���

	memset(bVolt, 0xee, sizeof(bVolt)); 

	wTemp2 = 0xb61f;
	if (QueryItemTimeMid(dwCurMStar*60, 0, BN0, wPn, &wTemp2, 1, bVolt, &wTemp) == 1)
	{
	    if(ReadItemMid(BN0, wPn, &wTemp2, 1, bVolt, dwCurMStar*60, INVALID_TIME) < 0)///���뵼���Ķ���PN0��
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

	if (IsDiffDay(ptmLast, ptmNow))//����
	{
		DTRACE(DB_DP, ("DoDpMtrDataStat:day change from %d to %d\n", DaysFrom2000(ptmLast), DaysFrom2000(ptmNow)));	 
		MtrDayChange(wPn, pbDayData, bVolt, dwCurM, ptmLast);
		memset((BYTE *)pdwTotalV, 0, 12);
		memset((BYTE *)pwStatCnt, 0, 6);
	}

	if ( MonthFrom2000(ptmNow) != MonthFrom2000(ptmLast))//����
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
		for(j=0; j<2; j++)//����
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

			//////////////////////////////////////Խ��////////////////////////////////////
			//Խ����
			if (wTemp>=tLimitVolt.wUpLimitVolt && tLimitVolt.wUpLimitVolt>0 )
			{			
				//ʱ���ۼ�
				wTemp2 = ByteToWord(pData+4+i*10) + bInterv;
				memcpy(pData+4+i*10, &wTemp2, 2);

				//Խ������
				if (wTemp>=tLimitVolt.wUpUpLimitVolt && tLimitVolt.wUpUpLimitVolt>0)
				{
					//ʱ���ۼ�
					wTemp2 = ByteToWord(pData+i*10) + bInterv;
					memcpy(pData+i*10, &wTemp2, 2);
				}
			}
			else if(wTemp<=tLimitVolt.wLowLimitVolt && tLimitVolt.wLowLimitVolt>0)
			{
				//��ʱ���ۼ�
				wTemp2 = ByteToWord(pData+6+i*10) + bInterv;
				memcpy(pData+6+i*10, &wTemp2, 2);

				//Խ������
				if (wTemp<=tLimitVolt.wLowLowLimitVolt &&tLimitVolt.wLowLowLimitVolt>0)  
				{
					//ʱ���ۼ�
					wTemp2 = ByteToWord(pData+2+i*10) + bInterv;
					memcpy(pData+2+i*10, &wTemp2, 2);
				}
			}
			else
			{
				//ʱ���ۼ�
				wTemp2 = ByteToWord(pData+8+i*10) + bInterv;
				memcpy(pData+8+i*10, &wTemp2, 2);
			}
			//////////////////////////////////////////////////////////////////////////////////////

			////////////////////////////////////////////��ֵ/////////////////////////////////////
			if( wTemp>BcdToDWORD(pData+30+i*10, 2) || IsAllAByte(pData+30+i*10, INVALID_DATA, 2))
			{
				DWORDToBCD(wTemp, pData+30+i*10, 2);
				MinToFmt(dwCurM, pData+32+i*10, FMT18);
			}
			if (wTemp < BcdToDWORD(pData+35+i*10, 2)  || IsAllAByte(pData+35+i*10, INVALID_DATA, 2)) //0ֵ�᲻��ˢ��ʱ��
			{
				DWORDToBCD(wTemp, pData+35+i*10, 2);
				MinToFmt(dwCurM, pData+37+i*10, FMT18);
			}
			if ( 0 >= wTemp)
				MinToFmt(dwCurM, pData+37+i*10, FMT18);

			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////�ۼ�ֵ������ʱ����ƽ��ֵ��Խ��ֵ//////////////////	
			*(pdwTotalV+3*j+i) += wTemp;
			*(pwStatCnt+3*j+i) += 1; //ͳ�ƴ����ۼ�
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

	//ptmLast = ptmNow;���ڴ��޸ģ��������������ͳ�ƺ��޸�ʱ��
}

//����:�յ�ѹ������ƽ���
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

	if (IsTimeEmpty(ptmLast) || IsInvalidTime(ptmLast))	//��һ�����л���ͳ�����ݽṹ�������
	{
		memset(pbUnBalanceData, 0, 14);
		//memset(pbMonData, 0, 60);
		//ptmLast = ptmNow;	//�������һ�ε�����ʱ��
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
		return ; //�����ͳ�ƹ���
 
	if (QueryItemTimeMid(dwCurMStar*60, 0, BN0, wPn, wID, 2, bCurData, &wTempA) == 2)
	{
		if(ReadItemMid(BN0, wPn, wID, 2, bCurData, dwCurMStar*60, INVALID_TIME) < 0)///���뵼���Ķ���PN0��
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


	if (IsDiffDay(ptmLast, ptmNow))//����
	{
		DTRACE(DB_DP, ("DoDpMtrUnBalanceStat:day change from %d to %d\n", DaysFrom2000(ptmLast), DaysFrom2000(ptmNow)));
		//tm = *ptmNow;
		//AddIntervs(&tm, TIME_UNIT_DAY, -1);
		//TimeToFmt20(&tm, bData);
		TimeToFmt20(ptmLast, bData);
		bData[3] = wPn&0xff;
		/////////////////////
		memcpy(bData+4, pbUnBalanceData, 14);
		/////////////////------>C2F28���
		if(IsFnSupport(wPn, 28, 2))
		{
			PipeAppend(TYPE_FRZ_TASK, 28, bData, 18);
		}
		memset(pbUnBalanceData, 0, 4);//ǰ�ĸ��ֽ�Ϊ�ۼ�ʱ��
	}

	ReadItemEx(BN24, PN0, 0x5009, &bCalcType);
	wAvrI = 0;
	wAvrV = 0;
	for(i=0; i<3; i++)
	{
		if(GetConnectType(wPn)==CONNECT_3P3W) //̨������жϵ���Ҫ��B���ѹ
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
					wAvrI += (WORD )ABS(FmtCurToVal(bCurData+6+ILEN*i, ILEN)); //2���ֽڣ�2��С��λ
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
				wAvrI += (WORD )ABS(FmtCurToVal(bCurData+6+ILEN*i, ILEN)); //2���ֽڣ�2��С��λ
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
	for(i=0; i<2; i++)//��ѹ������
	{
		if(1 == i)
		{
			pData = bCurData;
			if(0 != wAvrV)//����0ֵʱȫ����ֵΪ0
			{
				wTemp = wAvrV;
				wTempA  = (WORD)(abs(Fmt7ToVal(pData, 2) - wTemp) * 1000/wTemp);
				if((GetConnectType(wPn)==CONNECT_3P3W) && 1==bCalcType) //̨������жϵ���Ҫ��B���ѹ
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
		if (IsDiffDay(ptmLast, ptmNow))//����ʱ��ʼ�����ֵ
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


//����:�����������������ʱ��
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

	if (IsTimeEmpty(ptmLast) || IsInvalidTime(ptmLast))	//��һ�����л���ͳ�����ݽṹ�������
	{
		memset(pbDayData, 0, 24);
		memset(pbMonData, 0, 24);
		//ptmLast = ptmNow;	//�������һ�ε�����ʱ��
		DTRACE(DB_DP, ("DoDpMtrDemandStat: wPn=%d  tmLastRun IsTimeEmpty!.###########\r\n", wPn));
		return ;
	}

	dwCurM    = TimeToMinutes(ptmNow);//GetCurMinute();
	bInterv   = GetMeterInterv(wPn);
	dwCurMStar = dwCurM/bInterv*bInterv;
	dwLastMStat = TimeToMinutes(ptmLast)/bInterv*bInterv;
	if (dwCurMStar == dwLastMStat)
		return ; //�����ͳ�ƹ���

	if (QueryItemTimeMid(dwCurMStar*60, 0, BN0, wPn, &wID, 1, bCurData, &wVaildNum) == 1)
	{
		if(ReadItemMid(BN0, wPn, &wID, 1, bCurData, dwCurMStar*60, INVALID_TIME) < 0)///���뵼���Ķ���PN0��
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

	if (IsDiffDay(ptmLast, ptmNow))//����
	{
		DTRACE(DB_DP, ("DoDpMtrDemandStat:day change from %d to %d\n", DaysFrom2000(ptmLast), DaysFrom2000(ptmNow)));
		TimeToFmt20(ptmLast, bDemond);
		bDemond[3] = wPn&0xff;
		/////////////////////
		memcpy(bDemond+4, pbDayData, 24);
		/////////////////------>C2F26���
		if(IsFnSupport(wPn, 26, 2))
		{
		   PipeAppend(TYPE_FRZ_TASK, 26, bDemond, 28);
		}
		//memset(pbDayData, 0, 24);
	}

	if ( MonthFrom2000(ptmNow) != MonthFrom2000(ptmLast))//����
	{
		DTRACE(DB_DP, ("DoDpMtrDemandStat:month change from %d to %d\n",MonthFrom2000(ptmLast), MonthFrom2000(ptmNow)));
		TimeToFmt21(ptmLast, bDemond);
		bDemond[2] = wPn&0xff;
		/////////////////////
		memcpy(bDemond+3, pbMonData, 24);
		/////////////////------>C2F34���
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

			if (0==j && IsDiffDay(ptmLast, ptmNow))//����
			{
				memcpy(pData+6*i, bCurData+3*i, 3);
				MinToFmt(dwCurM, pData+6*i+3, FMT18);
			}
			else if(1==j && MonthFrom2000(ptmNow)!=MonthFrom2000(ptmLast))//����
			{
				memcpy(pData+6*i, bCurData+3*i, 3);
				MinToFmt(dwCurM, pData+6*i+3, FMT18);
			}

			//��ǰֵ��Ч ���β�ͳ��
			if (IsAllAByte(bCurData+3*i, INVALID_DATA, 3 ) || (GetConnectType(wPn)==CONNECT_3P3W && i==2))
			{
				//MinToFmt(dwCurM, pData+6*i+3, FMT18);
				continue;
			}
			dwDemond = BcdToDWORD(bCurData+3*i, 3);

			//������й�����ͳ�� ��һ��ͳ��ֵ���Ϊ��Ч��ֱ���õ�ǰֵ�滻�����ա���ʱ���ܽ���Чֵ���룩
			if(dwDemond>abs(Fmt23ToVal64(pData+6*i, 3)) || IsAllAByte(pData+6*i, INVALID_DATA, 3 ))
			{
				Val64ToFmt23(dwDemond, pData+6*i, 3);
				MinToFmt(dwCurM, pData+6*i+3, FMT18);
			}
		}
	}
}

