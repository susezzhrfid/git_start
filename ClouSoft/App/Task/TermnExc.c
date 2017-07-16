 /*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TermnExc.cpp
 * ժ    Ҫ��ʵ�ָ澯�¼����жϣ��洢��
 *
 * ��    ��: 1.0 1
 * ��    �ߣ�����
 * ������ڣ�2008-04-25
 *
 * ȡ���汾��1.0 0
 * ԭ �� ��:
 * ������ڣ� 
 * ��    ע: 
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

//==================================�ն˳���ʧ��(bAlrID = 0xE200003D)========================================================
//�������ն˳���ʧ�ܸ澯�ĳ�ʼ����
void MtrRdErrInit(TMtrRdErr* pCtrl)
{
	TTime now;
	GetCurTime(&now);

	memset(&pCtrl->bRdFailHapFlg, 0, sizeof(pCtrl->bRdFailHapFlg));
	memset(&pCtrl->tmOccur, 0, sizeof(TTime));
	pCtrl->tmBackTime = now;
}

//�������ն˱�ʧ�ܸ澯�жϡ�
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

		if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->bRdFailHapFlg[m]&(0x01<<n))//ÿ���ϱ�һ��
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
				if (abs(DaysFrom2000(&now) - DaysFrom2000(&pCtrl->tmBackTime))>=3)//3��û�г�����
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

//==================================�ն�ʱ�ӵ�ص�ѹ��(bAlrID = 0xE2000032)========================================================
//�������ն�ʱ�ӵ�ص�ѹ�͸澯�ĳ�ʼ����
void TermBatteryLowInit(TTermBatteryLow* pCtrl)
{
	pCtrl->fExcValid = false;
	memset(&pCtrl->tmOccur, 0, sizeof(pCtrl->tmOccur));
}

//�������ն�ʱ�ӵ�ص�ѹ�͸澯�жϡ�
bool TermBatteryLowTask(TTermBatteryLow* pCtrl)
{	
	BYTE bBuf[2] = {0};
	TBankItem tDataID[1] = { 0 };
	BYTE bDataIdNum = 0;
	WORD wBatterVol = 0;
	TTime now;
	GetCurTime(&now);

	if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true)//ÿ���ϱ�һ��
	{
		pCtrl->tmOccur = now;
		pCtrl->fExcValid = false;
	}

	ReadItemEx(BN0, PN0, 0x8875, bBuf);//��ص�ѹ  2���ֽ�  NN.NN
	wBatterVol = BcdToDWORD(bBuf, 2);
	DTRACE(DB_IOVER,("TermBatteryLowTask : P0  wBatterVol = %d\r\n", wBatterVol));

	if(!IsAlrEnable(0xE2000032))
	{
		DTRACE(DB_IOVER,("TermBatteryLowTask : Alr 0xE2000032 disable\r\n"));
		return false;
	}

	if(wBatterVol<220)//����2.2Vʱ�澯
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

//==================================�ն�ͣ�ϵ�(bAlrID = 0xE2000033~0xE2000034)========================================================
void PwrOffExcInit(TPwrOffCtrl* pCtrl)
{
    pCtrl->fPowerOff = IsPowerOff();
	if (g_PowerOffTmp.fAlrPowerOff)			//����ǰ�ϱ���ͣ��澯,����������е�������澯
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

	pbData = bAlrBuf + 7;//1���ֽڵ�CRC + 2���ֽڵ�Pn + 4���ֽڵ�ID
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

//������ͣ���ϵ��¼��жϣ�
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

	if (fPowerOff)  //�쳣������ȥ��
		pCtrl->fPowerOff = true;
	else			//�쳣�ָ���ȥ��
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
    if (pCtrl->fPowerOff != pCtrl->fOldPowerOff)   //�����ı�
    {
		bDataIdNum = sizeof(wPowerOffID)/sizeof(wPowerOffID[0]);
		for(n=0; n<bDataIdNum; n++)
		{
			tDataID[n].wBn = BN0;
			tDataID[n].wID = wPowerOffID[n];
			tDataID[n].wPn = PN0;
		}

		if (pCtrl->fPowerOff)   //�ն�ͣ��
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
		else  //�ն�����
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

//==================================�ն���ͨ������Խ��(bAlrID = 0xE200003A)========================================================
//�������ն���ͨ������Խ�޸澯�ĳ�ʼ����
void FluxOverInit(TFluxOver* pCtrl)
{
	pCtrl->fExcValid = false;
	memset(&pCtrl->tmOccur, 0, sizeof(TTime));
}

//�������ն���ͨ������Խ�޸澯�жϡ�
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
	DWORD dwMonFlux = 0;//����ͨ������
	DWORD dwMonLimit = 0;//����ͨ����������ֵ
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
	//###��ȡ��ͨ����������ֵ��0xE000018C��
	ReadItemEx(BN0, PN0, 0x804C, bBuf);
	dwMonLimit = BcdToDWORD(bBuf, 3);

	DTRACE(DB_IUNBAL, ("FluxOverTask : dwMonFlux = %ld dwMonLimit = %ld pCtrl->fExcValid = %d.\r\n",dwMonFlux,dwMonLimit,pCtrl->fExcValid));
	if(0 == dwMonLimit)
	{
		pCtrl->fExcValid = false;
		return true;
	}
	if (DaysFrom2000(&now)!=DaysFrom2000(&pCtrl->tmOccur) && pCtrl->fExcValid==true && now.nMonth==pCtrl->tmOccur.nMonth)//ÿ���ϱ�һ��,ֱ������Ϊֹ
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
	MtrRdErrInit(&pCtrl->tMtrRdErr);			//7�ն˳���ʧ��
	TermBatteryLowInit(&pCtrl->tTermBatteryLow);//13ʱ�ӵ�ص�ѹ��
    PwrOffExcInit(&pCtrl->tPwrOffCtrl);			//14�ն�ͣ�ϵ�
	FluxOverInit(&pCtrl->tFluxOver);			//15��ͨ������Խ��
}


bool DoTermExc(TTermExcCtrl* pCtrl)
{
	MtrRdErrTask(&pCtrl->tMtrRdErr);			//�ն˳���ʧ��
	TermBatteryLowTask(&pCtrl->tTermBatteryLow);//ʱ�ӵ�ص�ѹ��
	DoPowerExcTask(&pCtrl->tPwrOffCtrl);		//�ն�ͣ�ϵ�
	FluxOverTask(&pCtrl->tFluxOver);			//��ͨ������Խ��

    return true;
}

//ͨ�Ų��������¼� ERC=60
bool DoTestConnectExc(BYTE *pbBuf)
{
  	TTime time;
  	GetCurTime(&time);
// 	SaveAlrData(ERC_TEST_CONNECT, time, pbBuf, 0, 0);
    return true;
}
