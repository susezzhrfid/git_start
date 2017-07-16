 /*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TermnExc.cpp
 * 摘    要：实现告警事件的判断，存储；
 *
 * 版    本: 1.0 1
 * 作    者：陈曦
 * 完成日期：2008-04-25
 *
 * 取代版本：1.0 0
 * 原 作 者:
 * 完成日期： 
 * 备    注: 
************************************************************************************************************/
#include "FaAPI.h"
#include "ComAPI.h"
#include "ExcTask.h"
#include "DbAPI.h"
#include "SysDebug.h"
#include "TermnExc.h"
#include "TypeDef.h"
#include "FileMgr.h"

TTermExcCtrl g_TermAlrCtrl;

//==================================终端抄表失败(bAlrID = 0xE200003D)========================================================
//描述：终端抄表失败告警的初始化。
void MtrRdErrInit(TMtrRdErr* pCtrl)
{
	TTime now;
	GetCurTime(&now);

	memset(&pCtrl->bRdFailHapFlg, 0, sizeof(pCtrl->bRdFailHapFlg));
	memset(&pCtrl->tmOccur, 0, sizeof(TTime));
	pCtrl->tmBackTime = now;
}

//描述：终端表失败告警判断。
bool MtrRdErrTask(TMtrRdErr* pCtrl)
{	
	WORD i = 0;
	BYTE m = 0, n = 0;
	TTime now;
	GetCurTime(&now);

	for(i = 1; i<PN_NUM; i++)
	{
		if (!IsPnValid(i))
			continue;

		m = i/8;
		n = i%8;

		if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->bRdFailHapFlg[m]&(0x01<<n))//每天上报一次
		{
			pCtrl->bRdFailHapFlg[m] &=  ~(0x01<<n);
			pCtrl->tmOccur = now;
		}

		DTRACE(DB_VBREAK, ("MtrRdErrTask : g_bRdMtrAlr=0x%x  P%d DaysFrom2000(&now) - DaysFrom2000(&pCtrl->tmBackTime)=%08d,in alr 0xE200003D.\r\n",g_bRdMtrAlr[m]&(0x01<<n),i,abs(DaysFrom2000(&now) - DaysFrom2000(&pCtrl->tmBackTime))));

		if(!IsAlrEnable(0xE200003D))
		{
			DTRACE(DB_VBREAK, ("MtrRdErrTask : Alr 0xE200003D disable\r\n"));
			return false;
		}

		if (g_bRdMtrAlr[m]&(0x01<<n))
		{
			if (!(pCtrl->bRdFailHapFlg[m]&(0x01<<n)))
			{
				if (abs(DaysFrom2000(&now) - DaysFrom2000(&pCtrl->tmBackTime))>=3)//3天没有抄到表
				{
					DTRACE(DB_VBREAK, ("MtrRdErrTask : ############# P%d alr 0xE200003D happen.\r\n",i));

					g_bRdMtrAlrStatus[m] |= 0x01<<n;

					pCtrl->tmOccur = now;

					HandleAlr(0xE200003D, PN0, NULL, 0, 0, now, NULL, 0);
					pCtrl->bRdFailHapFlg[m] |= 0x01<<n;
				}
			}
		}
		else
		{
			pCtrl->tmBackTime = now;
			g_bRdMtrAlrStatus[m] &= ~(0x01<<n);
			pCtrl->bRdFailHapFlg[m] &=  ~(0x01<<n);
		}
	}

	return true;
}

//==================================终端时钟电池电压低(bAlrID = 0xE2000032)========================================================
//描述：终端时钟电池电压低告警的初始化。
void TermBatteryLowInit(TTermBatteryLow* pCtrl)
{
	pCtrl->fExcValid = false;
	memset(&pCtrl->tmOccur, 0, sizeof(pCtrl->tmOccur));
}

//描述：终端时钟电池电压低告警判断。
bool TermBatteryLowTask(TTermBatteryLow* pCtrl)
{	
	BYTE bBuf[2] = {0};
	TBankItem tDataID[1] = { 0 };
	BYTE bDataIdNum = 0;
	WORD wBatterVol = 0;
	TTime now;
	GetCurTime(&now);

	if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true)//每天上报一次
	{
		pCtrl->tmOccur = now;
		pCtrl->fExcValid = false;
	}

	ReadItemEx(BN0, PN0, 0x8875, bBuf);//电池电压  2个字节  NN.NN
	wBatterVol = BcdToDWORD(bBuf, 2);
	DTRACE(DB_IOVER,("TermBatteryLowTask : P0  wBatterVol = %d\r\n", wBatterVol));

	if(!IsAlrEnable(0xE2000032))
	{
		DTRACE(DB_IOVER,("TermBatteryLowTask : Alr 0xE2000032 disable\r\n"));
		return false;
	}

	if(wBatterVol<220)//低于2.2V时告警
	{
		if (!pCtrl->fExcValid)
		{
			DTRACE(DB_IOVER, ("TermBatteryLowTask : ############# P0 alr 0xE2000032 happen.\r\n"));
			HandleAlr(0xE2000032, PN0, tDataID, bDataIdNum, 0, now, NULL, 0);
			pCtrl->tmOccur = now;
			pCtrl->fExcValid = true;
		}
	}
	else
		pCtrl->fExcValid = false;

	return true;
}

//==================================终端停上电(bAlrID = 0xE2000033~0xE2000034)========================================================
void PwrOffExcInit(TPwrOffCtrl* pCtrl)
{
    pCtrl->fPowerOff = IsPowerOff();
	if (g_PowerOffTmp.fAlrPowerOff)			//掉电前上报了停电告警,则如果现在有电则报来电告警
	{
		DTRACE(DB_FA, ("PowerOnInit::g_fOldPowerOff = true\r\n"));
		pCtrl->fOldPowerOff = true;
	}
	else        
	{
		pCtrl->fOldPowerOff = pCtrl->fPowerOff;
	}
    
    pCtrl->bRecvCnt = pCtrl->bEstbCnt = 0;
	pCtrl->dwEstbTime = 0;
}

void PwrOffEventTask(TTime* TmTimeStart, TTime* tmTimeStop)
{
	BYTE bAlrBuf[ALRSECT_SIZE] = {0};
	BYTE* pbData = NULL;

	pbData = bAlrBuf + 7;//1个字节的CRC + 2个字节的Pn + 4个字节的ID
	if(TmTimeStart == NULL)
		memset(pbData, INVALID_DATA, BYTE_DATE_TIME);
	else
		TimeTo6Bcd(TmTimeStart,pbData);
	pbData += BYTE_DATE_TIME;
	
	if(tmTimeStop == NULL)
		memset(pbData, INVALID_DATA, BYTE_DATE_TIME);
	else
		TimeTo6Bcd(tmTimeStop,pbData+7);
	pbData += BYTE_DATE_TIME;

	DTRACE(DB_FA, ("DoPowerExcTask : ############# P0 event 0xE201000A hanpend.\r\n"));
	SaveAlrData(0xE201000A,PN0,bAlrBuf,2*BYTE_DATE_TIME);
}

//描述：停、上电事件判断；
bool DoPowerExcTask(TPwrOffCtrl* pCtrl)
{
	WORD  wPowerOffID[] = {0x9010,0x9020,0x9110,0x9120,0xb61f,0x2011,0x2012,0x2013,0x2014,0x2015,0x2016,0xb62f,0x2021,0x2022,0x2023,0xb63f,0xb64f,0xb65f};
	TBankItem tDataID[18] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	DWORD dwClick = 0;
	TTime now;
	bool fPowerOff = IsPowerOff();
	GetCurTime(&now);
	dwClick = GetClick();

	if (dwClick < 15)
		return false;

	if (fPowerOff)  //异常成立的去抖
		pCtrl->fPowerOff = true;
	else			//异常恢复的去抖
		pCtrl->fPowerOff = false;

	DTRACE(DB_FA, ("DoPowerExcTask : fPowerOff = %d pCtrl->fPowerOff = %d pCtrl->fOldPowerOff = %d.\r\n",fPowerOff,pCtrl->fPowerOff,pCtrl->fOldPowerOff));

	if(!IsAlrEnable(0xE2000033))
	{
		DTRACE(DB_FA, ("DoPowerExcTask : Alr 0xE2000033 disable\r\n"));
		return false;
	}
	if(!IsAlrEnable(0xE2000034))
	{
		DTRACE(DB_FA, ("DoPowerExcTask : Alr 0xE2000034 disable\r\n"));
		return false;
	}
    if (pCtrl->fPowerOff != pCtrl->fOldPowerOff)   //发生改变
    {
		bDataIdNum = sizeof(wPowerOffID)/sizeof(wPowerOffID[0]);
		for(n=0; n<bDataIdNum; n++)
		{
			tDataID[n].wBn = BN0;
			tDataID[n].wID = wPowerOffID[n];
			tDataID[n].wPn = PN0;
		}

		if (pCtrl->fPowerOff)   //终端停电
		{
			if(0==pCtrl->dwEstbTime)
			{
				pCtrl->dwEstbTime = GetClick();
				return false;//20140628-3
			}
			else if(GetClick() - pCtrl->dwEstbTime < 3)
				return false;

			DTRACE(DB_FA, ("DoPowerExcTask : ############# P0 exception 0xE2000033 confirmed.\r\n"));
			if (HandleAlr(0xE2000033, PN0, tDataID, bDataIdNum, 0, now, NULL, 0))
				pCtrl->fOldPowerOff = pCtrl->fPowerOff;

			g_PowerOffTmp.fAlrPowerOff = true;
			memcpy(&g_PowerOffTmp.tPoweroff, &now,sizeof(now)); 
			SavePoweroffTmp(true);

			PwrOffEventTask(&g_PowerOffTmp.tPoweroff, NULL);
		}
		else  //终端来电
		{
			DTRACE(DB_FA, ("DoPowerExcTask : ############# P0 exception 0xE2000034 confirmed.\r\n"));
			pCtrl->dwEstbTime = 0;
			if (HandleAlr(0xE2000034, PN0, tDataID, bDataIdNum, 0, now, NULL, 0))
				pCtrl->fOldPowerOff = pCtrl->fPowerOff;

    	 	g_PowerOffTmp.fAlrPowerOff = false;
    	 	SavePoweroffTmp(true);

			PwrOffEventTask(&g_PowerOffTmp.tPoweroff, &now);
		}
    }
	else
		pCtrl->dwEstbTime = 0;

	return true;
}

//==================================终端月通信流量越限(bAlrID = 0xE200003A)========================================================
//描述：终端月通信流量越限告警的初始化。
void FluxOverInit(TFluxOver* pCtrl)
{
	pCtrl->fExcValid = false;
	memset(&pCtrl->tmOccur, 0, sizeof(TTime));
}

//描述：终端月通信流量越限告警判断。
bool FluxOverTask(TFluxOver* pCtrl)
{	
	WORD wIdNum;
	int nRead;
	WORD *wIds ;
	WORD wMeterTimesIds[] = {0x886c};
	WORD bLenth = GetItemsLenId(wMeterTimesIds, sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]));
	BYTE bBuf[3] = {0};
	TBankItem tDataID[1] = { 0 };
	BYTE bDataIdNum = 0;
	BYTE n = 0;
	DWORD dwMonFlux = 0;//当月通信流量
	DWORD dwMonLimit = 0;//当月通信流量门限值
	TTime now;
	GetCurTime(&now);


	wIds = (WORD *)wMeterTimesIds;
	wIdNum = sizeof(wMeterTimesIds)/sizeof(WORD);

	nRead = ReadItemMid(BN0, PN0, wIds, wIdNum, bBuf, 0, INVALID_TIME);
	if (nRead <= 0)
		return false;

	if(IsAllAByte(bBuf, INVALID_DATA, bLenth))
		return false;

	dwMonFlux = BcdToDWORD(bBuf, bLenth);
	//###获取月通信流量门限值（0xE000018C）
	ReadItemEx(BN0, PN0, 0x804C, bBuf);
	dwMonLimit = BcdToDWORD(bBuf, 3);

	DTRACE(DB_IUNBAL, ("FluxOverTask : dwMonFlux = %ld dwMonLimit = %ld pCtrl->fExcValid = %d.\r\n",dwMonFlux,dwMonLimit,pCtrl->fExcValid));
	if(0 == dwMonLimit)
	{
		pCtrl->fExcValid = false;
		return true;
	}
	if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true && now.nMonth==pCtrl->tmOccur.nMonth)//每天上报一次,直到过月为止
	{
		pCtrl->fExcValid = false;
		pCtrl->tmOccur = now;
	}

	if(!IsAlrEnable(0xE200003A))
	{
		DTRACE(DB_IUNBAL, ("FluxOverTask : Alr 0xE200003A disable\r\n"));
		return false;
	}

	if (dwMonFlux > dwMonLimit)
	{
		if (!pCtrl->fExcValid)
		{
			DTRACE(DB_IUNBAL, ("FluxOverTask : ############# P0 Exception 0xE200003A confirmed.\r\n"));

			bDataIdNum = sizeof(wMeterTimesIds)/sizeof(wMeterTimesIds[0]);
			for(n=0; n<bDataIdNum; n++)
			{
				tDataID[n].wBn = BN0;
				tDataID[n].wID = wMeterTimesIds[n];
				tDataID[n].wPn = PN0;
			}
			HandleAlr(0xE200003A, PN0, tDataID, bDataIdNum, 0, now, NULL, 0);
			pCtrl->fExcValid = true;
			pCtrl->tmOccur = now;
		}
	}
	else
	{
		if (pCtrl->fExcValid)
		{
			DTRACE(DB_IUNBAL, ("FluxOverTask : ############# P0 Exception 0xE200003A recover.\r\n"));
			pCtrl->fExcValid = false;
		}
	}

	return true;
}

void InitTermExc(TTermExcCtrl* pCtrl)
{
	MtrRdErrInit(&pCtrl->tMtrRdErr);			//7终端抄表失败
	TermBatteryLowInit(&pCtrl->tTermBatteryLow);//13时钟电池电压低
    PwrOffExcInit(&pCtrl->tPwrOffCtrl);			//14终端停上电
	FluxOverInit(&pCtrl->tFluxOver);			//15月通信流量越限
}


bool DoTermExc(TTermExcCtrl* pCtrl)
{
	MtrRdErrTask(&pCtrl->tMtrRdErr);			//终端抄表失败
	TermBatteryLowTask(&pCtrl->tTermBatteryLow);//时钟电池电压低
	DoPowerExcTask(&pCtrl->tPwrOffCtrl);		//终端停上电
	FluxOverTask(&pCtrl->tFluxOver);			//月通信流量越限

    return true;
}

//通信测试请求事件 ERC=60
bool DoTestConnectExc(BYTE *pbBuf)
{
  	TTime time;
  	GetCurTime(&time);
// 	SaveAlrData(ERC_TEST_CONNECT, time, pbBuf, 0, 0);
    return true;
}
