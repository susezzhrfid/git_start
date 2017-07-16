/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrHook.cpp
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��4��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *			 $�����ﲻҪ�����inline,����Ϳ��ļ�һ�����ʱ�ض�λ
 *********************************************************************************************************/
#include "MtrHook.h"
#include "FaAPI.h"
#include "DbGbAPI.h"
#include "DbConst.h"
#include "SysDebug.h"
#include "ComAPI.h"
#include "LibDbAPI.h"
#include "DbAPI.h"
#include "MtrAPI.h"
#include "ComAPI.h"
#include "ExcTask.h"
#include "DbFmt.h"
#include "MtrCtrl.h"
#include "FlashMgr.h"

//������@iPhase 1���ࣻ3���࣬-1���ж�
bool IsCtrlFlgMismatch(DWORD dwFlg, BYTE bMtrPro,
					   BYTE bProfMode, BYTE bDayMode, BYTE bDayFlgMode, 
					   BYTE bDemDayMode, BYTE bDemDayFlgMode, BYTE bMonSettMode, BYTE bDemMonSettMode, BYTE bIntervU)
{
	if ((bMtrPro!=MTRPRO_07 && (dwFlg&M97)==0) || 
		(bMtrPro==MTRPRO_07 && (dwFlg&M07)==0) ||	//���Э�鲻����

		((dwFlg&PFCUR) && bMtrPro==MTRPRO_07 && bProfMode==1) ||		//��ǰ���������07�����ó����
		((dwFlg&PFMTR) && (bMtrPro!=MTRPRO_07 || bProfMode==0)) ||	//������������97���������óɳ���ǰ

		((dwFlg&DMCUR) && bIntervU==TIME_UNIT_DAY && bMtrPro==MTRPRO_07 && bDayMode==1) ||	//��ǰ�������07�����ó������
		((dwFlg&DMMTR) && bIntervU==TIME_UNIT_DAY && (bMtrPro!=MTRPRO_07 || bDayMode==0)) ||	//����ն��������97����

		((dwFlg&DMCUR) && bIntervU==TIME_UNIT_MONTH && bMtrPro==MTRPRO_07 && bMonSettMode!=0) ||	//��ǰ�¶��������07�����óɳ�����᲻��
  		((dwFlg&DMMTR) && bIntervU==TIME_UNIT_MONTH && (bMtrPro!=MTRPRO_07 || bMonSettMode!=1)) ||	//���ǰ/�����������¶��᳭ʵʱ������ղ���
		((dwFlg&MSETT) && bMonSettMode!=2) ||	//��������������ǳ�������ģʽ����

		((dwFlg&DFCUR) && bDayFlgMode==1) ||	//��ǰ����������������ó������
		((dwFlg&DFMTR) && bDayFlgMode==0) ||	//������¶�����������ó����ǰ

		((dwFlg&DEMDMCUR) && bIntervU==TIME_UNIT_DAY && bMtrPro==MTRPRO_07 && bDemDayMode==1) || 	//������ǰ�������07�����ó������
		((dwFlg&DEMDMMTR) && bIntervU==TIME_UNIT_DAY && (bMtrPro!=MTRPRO_07 || bDemDayMode==0)) ||	//��������ն��������97���������óɳ���ǰ

		((dwFlg&DEMDMCUR) && bIntervU==TIME_UNIT_MONTH && bMtrPro==MTRPRO_07 && bDemMonSettMode!=0) ||	//���ǰ/����������07�������¶���ǳ���ǰ����    
   		((dwFlg&DEMDMMTR) && bIntervU==TIME_UNIT_MONTH && (bMtrPro!=MTRPRO_07 || bDemMonSettMode!=1)) ||	//���ǰ/���������������¶��᳭ʵʱ������ղ���    
		((dwFlg&DEMMSETT) && bDemMonSettMode!=2) ||	//������������������ǳ�������ģʽ����

		((dwFlg&DEMDFCUR) && bDemDayFlgMode==1) ||	//������ǰ����������������ó������
		((dwFlg&DEMDFMTR) && bDemDayFlgMode==0))	//����������¶�����������ó����ǰ
	{
		return true;
	}
	else
	{
		return false;
	}
}

extern DWORD g_dwComTaskRecTm[DYN_PN_NUM][64];
//������������(��Ҫ���g_CctRdCtrl[]�����õ�)�Ƿ���Ҫ����
//����:�����������Ŀǰ��Ҫ������δ����ʱ��պ÷����򷵻�0,����Ѿ��������򷵻�1,�����Ҫ����ʱ�仹û���򷵻�-1,
//	   �������������ڸõ����˵����Ҫ���򷵻�-2�������������-3
int IsIdNeedToRead(WORD wPn, const TMtrRdCtrl* pRdCtrl)
{
	const BYTE* pbFn = pRdCtrl->bFn;
	const DWORD* pdwFnFlg = pRdCtrl->dwFnFlg;
	DWORD dwFlg = 0;
	WORD i;
	WORD wID = pRdCtrl->wID;
	TTime now;
	BYTE bClass, bMtrPro, bProfMode, bDayMode, bDayFlgMode, bDemDayMode, bDemDayFlgMode, bMonthSettMode, bDemMonthSettMode;
	BYTE bFn = 0;
//	BYTE bMode = 0;	
	BYTE bNum = *pbFn++;
	BYTE bBuf[8];
	pdwFnFlg++;	//��������

	bMtrPro = GetPnMtrPro(wPn);

	bProfMode = 0;
	ReadItemEx(BN24, PN0, 0x4110, &bProfMode);	//0x3003 1 ���߶���ģʽ��,0�����ǰ���ݣ�1���������
	if (wID==0xb61f && pRdCtrl->bIntervU==TIME_UNIT_HOUR)
		bProfMode = 0; //07�������ʾֵ����Ŀǰֻ֧�ֳ���ǰ

	bDayMode = 0;
	ReadItemEx(BN24, PN0, 0x4111, &bDayMode); //0x3004 1 �ն���ģʽ��,0�����ǰ���ݣ�1�������

	bDayFlgMode = 0;
	ReadItemEx(BN24, PN0, 0x4112, &bDayFlgMode); //0x3005 1 �����ն���ģʽ��,0�����ǰ���ݣ�1�������

	bDemDayMode = 0;
	ReadItemEx(BN24, PN0, 0x4113, &bDemDayMode); //0x3006 1 �����ն���ģʽ��,0�����ǰ���ݣ�1�������

	bDemDayFlgMode = 0;
	ReadItemEx(BN24, PN0, 0x4114, &bDemDayFlgMode); //0x3007 1 ���������ն���ģʽ��,0�����ǰ���ݣ�1�������
    
   	bMonthSettMode = 0;
	ReadItemEx(BN24, PN0, 0x4115, &bMonthSettMode); //0x3007 1 �¶���ģʽ��,0�����ǰ���ݣ�1������᣻2����������
    
	bDemMonthSettMode = 0;
	ReadItemEx(BN24, PN0, 0x4116, &bDemMonthSettMode); //0x3007 1 �����¶���ģʽ��,0�����ǰ���ݣ�1������᣻2����������

	GetCurTime(&now);

	for (i=0; i<bNum; i++)
	{
		dwFlg = *pdwFnFlg++;
		bFn = *pbFn++;
		bClass = pRdCtrl->bIntervU==TIME_UNIT_MTR ? 1 : 2;

		if (IsCtrlFlgMismatch(dwFlg, bMtrPro, bProfMode, 
							  bDayMode, bDayFlgMode,	
							  bDemDayMode, bDemDayFlgMode,
                              bMonthSettMode, bDemMonthSettMode, pRdCtrl->bIntervU))
		{
			continue;
		}

		if (bFn == ALLPN)		//���в�����������
		{
			//�����κζ���������Ϊ�˱������if (!IsFnSupport(wPn, bFn, 2))���ж�				
		}
		else if (bFn > FN_MAX)
		{
			if (GetErcType(bFn - ERCPN) == 0)
				continue;
		}
		else
		{
			BYTE bThrId = GetPnThread(wPn);
			if (IsComTaskDone(wPn, bFn, &now, g_dwComTaskRecTm[bThrId]))
				continue;
		}

		if (pRdCtrl->bIntervU == TIME_UNIT_DAY)		//�ն����޶������л���һ��Сʱ�ڳ���
		{
			if (now.nHour==0 && now.nMinute<pRdCtrl->tmStartTime.nMinute && bDayMode==1)	//���������ӳ�,��ʵʱ���ӳ�
				return -1;	//�����Ҫ����ʱ�仹û���򷵻�-1
		}

		/*if (pRdCtrl->bIntervU == TIME_UNIT_DAYFLG)
		{
			TTime tmRd = now;
			GetPnDate(wPn, bBuf);
			dwFlg = ByteToDWord(bBuf);
			if ((dwFlg & (1<<(now.nDay-1))) == 0) //���ǳ�����
				continue;	//��ǰʱ�䲻���ϳ���Ҫ��
			tmRd.nHour = BcdToByte(bBuf[5]);
			tmRd.nMinute = BcdToByte(bBuf[4]);
			tmRd.nSecond = 0; //��֤������ʼ������ִ��
			dwFlg = SecondsPast(&tmRd, &now);
			if (dwFlg==0 || dwFlg>3600*2)
				continue;	//��ǰʱ�䲻���ϳ���Ҫ��
		}*/

		if (pRdCtrl->bIntervU == TIME_UNIT_MONTH)
		{
			if (now.nDay!=1 || now.nHour!=0)
				continue;	//��ǰʱ�䲻���ϳ���Ҫ��
			if (now.nMinute < pRdCtrl->tmStartTime.nMinute)
				return -1;	//�����Ҫ����ʱ�仹û���򷵻�-1
		}

		//ǰ������˻����жϣ���ĿǰΪֹ��֧�֣��������һЩ����ID���ж� 
		if (bMtrPro == MTRPRO_07 &&
			(wID==0x9a1f || wID==0x9a2f || (wID>=0x9b1f && wID<=0x9b6f) || (wID>=0x9c0f && wID<=0x9cbf))) //�ն���
		{
			DWORD dwDays;
			WORD wValidNum = 0;	
	
			TBankItem BankItem;
			BankItem.wBn = BN0;	//BANK���ȶ�Ϊ�๦�ܱ�,���涳���ʱ��CctQueryItemTime()���Զ���̬����
			BankItem.wPn = wPn;
			BankItem.wID = 0x9a00; //�ϴ��ն���ʱ��
	
			dwDays = DaysFrom2000(&now);
			QueryItemTimeMbi(dwDays*24*60*60, (dwDays+1)*24*60*60, &BankItem, 1, bBuf, &wValidNum);
			if (wValidNum == 0)	//�ϴ��ն���ʱ�仹û����,����ʱ�䲻��,����������������Ȳ�����
				return -1;	//�����Ҫ����ʱ�仹û���򷵻�-1,
		}

		return 0;	//���ڵ�ǰFN��˵������������֧�ֵģ�������Ҫ����
	}

	return -2; //�������������ڸõ����˵����Ҫ���򷵻�-2
}


//����:ȡ�ü����������ȡ��ʱ�䷶Χ(��ʼʱ��~����ʱ��)
//��ע:������ЩЭ�鲻�ܼ򵥸���RdCtrl.tmStartTime��ȷ����ʼʱ��ͽ���ʱ��,������Ҫ���ݲ���������
// 	   ����Ӧ��case���޸ļ���
bool GetItemTimeScope(WORD wPn, const TMtrRdCtrl* pRdCtrl, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime)
{
	DWORD dwDayFlg;
	WORD wIntervV;
	TTime tmNow, tmStart, tmEnd;
	BYTE bMtrInterv;
	BYTE bBuf[10];

	tmNow = *pNow;
	tmNow.nSecond = 0;		//��������
	tmStart = tmNow;
	tmEnd = tmNow;

	switch (pRdCtrl->bIntervU)
	{
	case TIME_UNIT_MTR:		//������
	case TIME_UNIT_STA:
		bMtrInterv = GetMeterInterv(wPn);
		if (bMtrInterv < 60)	//��ͬ�ڷ���
		{
			wIntervV = bMtrInterv;
			if (wIntervV == 0)
				wIntervV = 15;

			tmStart.nMinute = tmStart.nMinute / wIntervV * wIntervV + pRdCtrl->tmStartTime.nMinute;	//�����Ӽ�������������ʼʱ��

			//�����ʱ��
			tmEnd = tmStart;
			AddIntervs(&tmEnd, TIME_UNIT_MINUTE, wIntervV);
		}
		else	//��ͬ��Сʱ
		{
			tmStart.nMinute = pRdCtrl->tmStartTime.nMinute;

			//�����ʱ��
			tmEnd = tmStart;
			tmEnd.nMinute = 0;
			tmEnd.nSecond = 0;
			AddIntervs(&tmEnd, TIME_UNIT_HOUR, 1);
		}

		break;

	case TIME_UNIT_MINUTE://���Ӷ���,��Сʱ���շ���������
		//��ʼʱ��
		wIntervV = pRdCtrl->bIntervV;
		if (wIntervV == 0)
			wIntervV = 15;

		tmStart.nMinute = tmStart.nMinute / wIntervV * wIntervV + pRdCtrl->tmStartTime.nMinute;	//�����Ӽ�������������ʼʱ��

		//�����ʱ��
		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_MINUTE, wIntervV);
		break;

	case TIME_UNIT_HOUR:	//Сʱ
		//��ʼʱ��
		wIntervV = pRdCtrl->bIntervV;
		if (wIntervV == 0)
			wIntervV = 1;

		tmStart.nMinute = pRdCtrl->tmStartTime.nMinute;
		tmStart.nHour = tmStart.nHour / wIntervV * wIntervV;	//��Сʱ��������������ʼʱ��

		//�����ʱ��
		tmEnd = tmStart;
		tmEnd.nMinute = 0;
		tmEnd.nSecond = 0;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, wIntervV);
		break;

	case TIME_UNIT_DAY:		//�ն����޶������л���һ��Сʱ�ڳ���
		//��ʼʱ��
		tmStart.nHour = 0;
		tmStart.nMinute = 0; //pRdCtrl->tmStartTime.nMinute; ��Ϊ�������ʱ���ǰ����������
		tmStart.nSecond = 0;

		//�����ʱ��
		tmEnd.nHour = 23;
		tmEnd.nMinute = 59;
		tmEnd.nSecond = 59;

		break;
	/*case TIME_UNIT_DAYFLG:		
		GetPnDate(wPn, bBuf);	//GetMtrDate(PORT_GB485, bBuf);
		
		dwDayFlg = ByteToDWord(bBuf);
		if ((dwDayFlg & (1<<(tmNow.nDay-1))) == 0) //���ǳ�����
			return false;	//��ǰʱ�䲻���ϳ���Ҫ��,����false����
				
		//�󳭱�Ŀ�ʼʱ��
		tmStart.nHour = BcdToByte(bBuf[5]);
		tmStart.nMinute = BcdToByte(bBuf[4]); 		
		tmStart.nSecond = 0;
		wIntervV = GetMeterInterv(wPn);
        if (wIntervV == 0)  //�����ֹ��0
            return false; 
		dwDayFlg = TimeToMinutes(&tmStart)/wIntervV*wIntervV; //��Ϊ�������ʱ���ǰ����������
		MinutesToTime(dwDayFlg, &tmStart);

		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, 2);
		//tmEnd.nHour = 23;
		//tmEnd.nMinute = 59;
		//tmEnd.nSecond = 59;
		break;	*/
	case TIME_UNIT_MONTH:
		if (tmNow.nDay != 1)
			return false;	//��ǰʱ�䲻���ϳ���Ҫ��,����false����

		//��ʼʱ��
		tmStart.nDay = 1;	//�¶���������1�Ŷ���,����Ҫ������ĩ���ж���
		tmStart.nHour = 0;		//pRdCtrl->tmStartTime.nHour;
		tmStart.nMinute = 0; //pRdCtrl->tmStartTime.nMinute; ��Ϊ�������ʱ���ǰ����������
		tmStart.nSecond = 0;

		//�����ʱ��:����1��23:59
		tmEnd.nDay = 1;
		tmEnd.nHour = 00;
		tmEnd.nMinute = 59;
		tmEnd.nSecond = 59;
		break;

	default:
		return false;
	}

/*	if (pRdCtrl->wID == 0x9a00) //�̶���ѯ��ǰ�������ģ�Ϊ��ÿ�������ȥ����һ��
	{
		wIntervV = GetMeterInterv(wPn);
		dwDayFlg = TimeToMinutes(pNow)/wIntervV*wIntervV; //��Ϊ�������ʱ���ǰ����������
		MinutesToTime(dwDayFlg, &tmStart);

		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_MINUTE, wIntervV);
	}*/

	*pdwStartTime = TimeToSeconds(&tmStart);
	*pdwEndTime = TimeToSeconds(&tmEnd);
	return true;
}


//����:�������ȷ��(����������),�ڹ��ϵķ���/�ָ�ʱ��Ļص�����,�������ɸ澯�¼�
void OnMtrErrEstb(WORD wPn, BYTE* pbData)	
{
	BYTE m = 0, n = 0;

	if (wPn > POINT0 && wPn < PN_NUM) 
	{
		m = (wPn)/8;
		n = (wPn)%8;

		g_bRdMtrAlr[m] |= 0x01<<n;

	}
}

//����:������ϻָ�(����������),�ڹ��ϵķ���/�ָ�ʱ��Ļص�����,�������ɸ澯�¼�
void OnMtrErrRecv(WORD wPn)	
{
	BYTE m = 0, n = 0;

	if (wPn > POINT0 && wPn < PN_NUM) 
	{
		m = (wPn)/8;
		n = (wPn)%8;

		g_bRdMtrAlr[m] &= ~(0x01<<n);

	}		

}

//����:ͨ������ַת��Ϊ�������
//����:@pb485Addr ����ַ,
//����:����ҵ�ƥ��Ĳ������򷵻ز������,���򷵻�0
WORD MtrAddrToPn(const BYTE* pb485Addr)
{
	BYTE bMtrAddr[6];
	WORD wPn = 1;

	while (1)
	{
		if (wPn >= POINT_NUM)
			break;

		if ( !IsMtrPn(wPn) )
		{
			wPn++;
			continue;
		}

		GetMeterAddr(wPn, bMtrAddr);
		if (memcmp(pb485Addr, bMtrAddr, 6) == 0)
		{
			return wPn;
		}

		wPn++;
	}

	return 0;
}

//����:ͳ�Ƽ���������Ϣ
void DoMtrRdStat()
{
	BYTE bBuf[16];
	BYTE bPort;
	TTime tNow;
	BYTE bStat;
	WORD wMtrNum = 0, wSuccNum = 0; //Ҫ���������&& ����ɹ�����
	//static DWORD dwCurIntervSec[LOGIC_PORT_NUM] = {0};
	static bool fIsClr = true;


	for(bPort=LOGIC_PORT_MIN; bPort<=LOGIC_PORT_MAX; bPort++)
		GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);


	bPort = 31;
	GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);

	//���
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
	if(IsAllAByte(bBuf, 0x00, sizeof(bBuf)))
		fIsClr = true;

	DWORDToBCD(wMtrNum, bBuf, 2); //Ҫ���������
	if (wMtrNum == 0)
		bStat = 0; //�ޱ�ɳ�ʱ����δ����
	else if (wMtrNum == wSuccNum)
		bStat = 2; //ȫ������
	else
		bStat = 1; //��ǰ������״̬��־:���ڳ���

	if(BcdToDWORD( &bBuf[2], 2) <= wSuccNum)
		DWORDToBCD(wSuccNum, &bBuf[2], 2); //����ɹ�����					

	if (bStat == 1)
	{
		if (wSuccNum>=0 && fIsClr)
		{
			fIsClr = false;
			GetCurTime(&tNow);
			TimeTo6Bcd(&tNow,bBuf+4);
            memset(bBuf+10, 0, 6); //����ʱ����0
		}
	}
	else if (bStat == 2)
	{
		GetCurTime(&tNow);
		TimeTo6Bcd(&tNow,bBuf+10);
	}
	else
		memset(bBuf+4, 0, 12);

	WriteItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
}

void ClrMtrRdStat()
{
	BYTE bBuf[16];
	//���
	memset(bBuf, 0, sizeof(bBuf));
	
	WriteItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
}
//ɾ����ʱ��Ҫ���¼���������Ϣ
void UpdRdStat()
{
    TTime tNow;
    BYTE bBuf[16];
	BYTE bPort;
	BYTE bStat;
	WORD wMtrNum, wSuccNum; //Ҫ���������&& ����ɹ�����
    

	for(bPort=LOGIC_PORT_MIN; bPort<=LOGIC_PORT_MAX; bPort++)
		GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);


	bPort = 31;
	GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);

	//���
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
	DWORDToBCD(wMtrNum, bBuf, 2); //Ҫ���������
	if (wMtrNum == 0)
		bStat = 0; //�ޱ�ɳ�ʱ����δ����
	else if (wMtrNum == wSuccNum)
		bStat = 2; //ȫ������
	else
		bStat = 1; //��ǰ������״̬��־:���ڳ���
	DWORDToBCD(wSuccNum, &bBuf[2], 2); //����ɹ�����					

	if (bStat == 2)
	{
		GetCurTime(&tNow);
		TimeToFmt1(&tNow,bBuf+10);
	}

	WriteItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
}

//����:�㲥ʱ���Ƿ񵽴�
bool IsBroadcastTime(BYTE bPort)
{
#ifdef PRO_698
	BYTE bBuf[120];
	TTime tmNow;	
	TTime tmTime;
	static DWORD dwLastMin[LOGIC_PORT_NUM] = {0};
	DWORD dwNow, dwMin;

	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return false;

	GetCurTime(&tmNow);
	dwNow = TimeToMinutes(&tmNow);
	if (dwLastMin[bPort - LOGIC_PORT_MIN] == dwNow)
		return false;

	dwLastMin[bPort - LOGIC_PORT_MIN] = dwNow; //�˿ںŴ�2��ʼ
	if (ReadItemEx(BN0, bPort-1, 0x021f, bBuf) > 0)	//F33
	{
		if (bBuf[1] & 0x8)  //�㲥Уʱʹ�� D3
		{
			GetCurTime(&tmTime);
			tmTime.nMinute = BcdToByte(bBuf[10]);
			tmTime.nHour = BcdToByte(bBuf[11]);            
			tmTime.nDay = BcdToByte(bBuf[12]);
			if (tmTime.nDay == 0)    //ÿ���ʱ
			{
				if (tmTime.nHour==tmNow.nHour && tmTime.nMinute==tmNow.nMinute)        
					return true;
			}
			else
			{
				dwMin = TimeToMinutes(&tmTime);
				if (dwNow == dwMin)
					return true;
			}
		}
	}
	return false;
#endif
}

//����:ȡ�㲥Уʱ����ʱ���֡
BYTE GetAdjTimeFrm(BYTE* pbFrm)
{
	TTime time;
	WORD i;

	pbFrm[0] = 0x68;
	memset(&pbFrm[1], 0x99, 6);
	pbFrm[7] = 0x68;
	pbFrm[8] = 0x08; //cmd
	pbFrm[9] = 0x06; //len
	GetCurTime(&time);
	pbFrm[10] = ByteToBcd(time.nSecond&0xff);
	pbFrm[11] = ByteToBcd(time.nMinute&0xff);
	pbFrm[12] = ByteToBcd(time.nHour&0xff);
	pbFrm[13] = ByteToBcd(time.nDay&0xff);
	pbFrm[14] = ByteToBcd(time.nMonth&0xff);
	pbFrm[15] = ByteToBcd((time.nYear-2000)&0xff);

    //+0x33
    for (i=10; i<16; i++)
	{
  	    pbFrm[i] += 0x33;
	}	 
	pbFrm[16] = CheckSum(pbFrm, 16);
	pbFrm[17] = 0x16;

	return 18;
}

//����:�ú���ʵ�ֱ�Ƽ�ز���ʱ645֡͸������
//����:�ɹ�����1,ʧ�ܷ���-1
int DoBroadcastFwdCmd(BYTE bPort, DWORD dwBaudRate)
{
	int iPort;
	TCommPara CommPara;
	char szBuf[128];
	BYTE bBuf[20];
	BYTE bFrmLen = GetAdjTimeFrm(bBuf);

	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return -1;

	iPort = MeterPortToPhy(bPort);
	if (iPort < 0)
		return -1;

	CommPara.wPort = (WORD)iPort;
	CommPara.dwBaudRate = dwBaudRate;	
	CommPara.bParity =  EVENPARITY;
	CommPara.bByteSize = 8;
	CommPara.bStopBits = ONESTOPBIT;

	GetDirRdCtrl(bPort);	//ȡ��ֱ���Ŀ���Ȩ
	if ( !MtrProOpenComm(&CommPara) )
	{
		ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
		return -1;
	}

	if (CommWrite(CommPara.wPort, bBuf, bFrmLen, 1000) != bFrmLen)
	{
		DTRACE(DB_FAPROTO, ("DoBroadcastFwdCmd: fail to write comm.\r\n")); 
		ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
		return -1;
	}
	ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
	sprintf(szBuf, "Port %d dwBaudRate %d DoBroadcastFwdCmd Adj Frm->", bPort, dwBaudRate);
	TraceBuf(DB_CRITICAL, szBuf, bBuf, bFrmLen);
	return 1;
}

void DoBroadcastMtrTime(BYTE bPort)
{
	DoBroadcastFwdCmd(bPort, CBR_1200);
	Sleep(500);
	DoBroadcastFwdCmd(bPort, CBR_2400);
	Sleep(500);
	DoBroadcastFwdCmd(bPort, CBR_4800);
	Sleep(500);
	DoBroadcastFwdCmd(bPort, CBR_9600);
}

void DoBrcastMtrTm()
{
	if (IsBroadcastTime(2))
		DoBroadcastMtrTime(2);
	if (IsBroadcastTime(3))
		DoBroadcastMtrTime(3);
	if (IsBroadcastTime(4))
		DoBroadcastMtrTime(4);
}
