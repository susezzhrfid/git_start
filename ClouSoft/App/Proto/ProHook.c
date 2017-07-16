/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProHook.c
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *********************************************************************************************************/
#include "FaAPI.h"
#include "ProAPI.h"
#include "FaCfg.h"
#include "ExcTask.h"
#include "ProHook.h"
#include "LibDbAPI.h"

//����:�ۼ������Ľӿں���
//��ע:����ת�����ͳ�������غ���,
// 	   ���ֻ������ʱ����ͳ����ĳ�Ա����,����ÿ�ν��շ��Ͷ�дϵͳ��,��Ӱ��ͨ��Ч��
void AddFlux(DWORD dwLen)
{
	BYTE bKbData[4] = {0};
 #ifdef PRO_698
//  DWORD dwDayFlux = 0;
//  DWORD dwMonFlux = 0;
  if (!IsDownSoft())
  {
	//WaitSemaphore(m_semTermLog);			//ͳ�Ƶ��������̶�Ҫ���б���,
	
//	ReadItemEx(BN0, PN0, 0x1500, (BYTE*)&dwDayFlux); //698 C1F10
//	ReadItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);
	
//	dwDayFlux += dwLen;		//�ն�GPRS������
//	dwMonFlux += dwLen;		//�ն�GPRS������

//	WriteItemEx(BN0, PN0, 0x1500, (BYTE*)&dwDayFlux); //698 C1F10
//	WriteItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);

	g_PowerOffTmp.tTermStat.dwDayFlux += dwLen;		//�ն�GPRS������
	g_PowerOffTmp.tTermStat.dwMonFlux += dwLen;		//�ն�GPRS������

	DWORDToBCD(g_PowerOffTmp.tTermStat.dwDayFlux/1024,  bKbData, 2);
	WriteItemEx(BN0, PN0, 0x886a, bKbData); //698 C1F10
	DWORDToBCD(g_PowerOffTmp.tTermStat.dwMonFlux/1024,  bKbData, 3);
	WriteItemEx(BN0, PN0, 0x886c, bKbData);
  }
 #endif
}

//����:�ۼ������Ľӿں���
bool IsFluxOver()	
{
	DWORD dwMonFlux = 0;
	DWORD dwFluxLimit = 0;
#ifdef PRO_698
    if (IsDownSoft())
    	return false;
	//DWORD dwMonFlux = 0;
    ReadItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);

	//DWORD dwFluxLimit = 0;
	ReadItemEx(BN0, PN0, 0x024f, (BYTE*)&dwFluxLimit); //F36���ն�����ͨ��������������,��ͨ����������Ϊ0����ʾϵͳ����Ҫ�ն˽�����������

	if (dwFluxLimit>0 && dwMonFlux>dwFluxLimit)
		return true;
	else
		return false;
#else
	return false;
#endif
}

//����:�ص�����,�������ɸ澯��¼����;
//��ע:ֻ���ڵ�һ�γ�������ʱ�����ɸ澯��¼,����ɱ������������ĵ��ú������ж�
void GprsOnFluxOver()	
{
#ifdef PRO_698
	BYTE bAlrBuf[16];
	
	TTime now;
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	GetCurTime(&now);

	ReadItemEx(BN0, PN0, 0x1501, &bAlrBuf[0]); //�����ѷ�����ͨ������
	ReadItemEx(BN0, PN0, 0x024f, &bAlrBuf[4]); //��ͨ����������,F36���ն�����ͨ��������������

// 	SaveAlrData(ERC_FLUXOVER, now, bAlrBuf,0,0);
	DTRACE(DB_METER_EXC, ("GprsOnFluxOver: ########## \n"));
#endif
}


//����:GPRS�Ƿ�������ʱ��
bool GprsIsInPeriod()
{	
  	BYTE bBuf[16];
#ifdef PRO_698
	if (ReadItemEx(BN0, PN0, 0x008f, bBuf) > 0) //C1F8
	{
        //ʱ������ģʽ��������ʱ�α�־��D0~D23��λ˳���Ӧ��ʾ0~23��
		//��"1"��ʾ��������ʱ�Σ���"0"��ʾ��ֹ����ʱ�Σ�������ʱ�ε��趨ֵ��ͬʱ���ϲ�Ϊһ����ʱ�Ρ�

		TTime now;
		GetCurTime(&now);
		if (bBuf[5+now.nHour>>3] & (1<<(now.nHour&7)))
			return true;
	}

	return false;
#else
	return true;
#endif
}

//����:GPRSͨ���ĸ澯���������������Ƿ�����,������Ҫ���ߵ�ʱ��,�ȴ��澯��������������
//	   ȫ�������ٵ���.������õȴ��澯����������ȫ������Ϳ��Ե���,ֱ�ӷ���true
//����:@dwStartClick �����������ʼʱ��,���������ڶ��ٷ�����û����Ҳ�������
//����:����澯����������ȫ�������򷵻�true,���򷵻�false
bool GprsIsTxComplete(DWORD dwStartClick) 
{
	return false;
}

//��������ģ��汾��CCID����Ϣ���µ�ϵͳ�⣬��MODEM��Ļص��ӿڣ��ڸ����������Ϣ�����
void UpdModemInfo(TModemInfo* pModemInfo)
{
	WriteItemEx(BN0, PN0, 0x21Ef, (BYTE *)pModemInfo);
}
