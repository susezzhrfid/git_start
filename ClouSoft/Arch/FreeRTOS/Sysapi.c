/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Sysapi.c
 * ժ    Ҫ��ϵͳ��ص�һЩAPI
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#include "Sysarch.h"
#include "Sysdebug.h"
#include "Sysapi.h"
//#include "RtcBios.h"
#include "FaCfg.h"
#include "ComAPI.h"
#include "Drivers.h"
#include "FaAPI.h"

static unsigned long g_dwBaseClick = 0;  //�ڸ�ʱ���ʱ�����������ʱ��ǰ���Ѿ��ߵ�click
static TSem  g_semTime;   

bool SetSoftTime(const TTime* pTime)
{    
	if (IsInvalidTime(pTime))
	{
		DTRACE(DB_CRITICAL, ("SetSysTime: ##### time invalid\n"));        
		return false;
	}
    WaitSemaphore(g_semTime, SYS_TO_INFINITE);
	g_dwBaseClick = TimeToSeconds(pTime) - GetClick();
    SignalSemaphore(g_semTime);
	return true;
}

bool GetSysTime(TTime* pTime)
{
    WaitSemaphore(g_semTime, SYS_TO_INFINITE);
	DWORD dwSysClick = g_dwBaseClick + GetClick();
    SignalSemaphore(g_semTime);
	SecondsToTime(dwSysClick, pTime);

	if (IsInvalidTime(pTime))
		return false;
	else
		return true;	
}

bool SetSysTime(const TTime* pTime)
{
    WaitSemaphore(g_semTime, SYS_TO_INFINITE);
    RtcSetTime(pTime);
    SignalSemaphore(g_semTime);
    SetSoftTime(pTime);
	return true;
}

//�����ͬ��ʱ��
/*
void SyncTimer()
{
    WORD i;
	static TTime tmLast = { 0 };
	TTime now;

   	GetCurTime(&now);
	if (IsTimeEmpty(&tmLast))
		GetCurTime(&tmLast);

	if (HoursPast(&tmLast, &now)>0 && now.nMinute==57 && now.nSecond>=45)//ÿ��Сʱ��57��30��ͬ��һ��ʱ��
	{
		for (i=0; i<3; i++)
		{			
            if (ReadInMtrTime(&g_ProEx))
            {
                SaveSysClockToDB(true); //ÿСʱ����һ�����ݿ�
                break;
			}
		}

		GetCurTime(&tmLast);
	}
}*/

void SyncTimer()
{
	static TTime tmLast = { 0 };
	TTime now;
	TTime tmOld;
	memset(&tmOld, 0, sizeof(tmOld));
	if (IsTimeEmpty(&tmLast))
    {
        WaitSemaphore(g_semTime, SYS_TO_INFINITE);
        RtcGetTime(&now);
        SignalSemaphore(g_semTime);
        SetSoftTime(&now);
		GetCurTime(&tmLast);
        DTRACE(DB_CRITICAL, ("SyncTimer: Current time %d/%02d/%02d %02d::%02d::%02d, click=%d.\r\n", 
					now.nYear, now.nMonth, now.nDay, 
			  		now.nHour, now.nMinute, now.nSecond, GetClick())); //���ϵ��ʱ���ӡ�µ�ǰ��ʱ��
    }
	GetCurTime(&now);
	if (HoursPast(&tmLast, &now)>0 && now.nMinute==57 && now.nSecond>=45)//ÿ��Сʱ��57��30��ͬ��һ��ʱ��
//  if (MinutesPast(tmLast, now) > 15)//ÿ15���Ӷ�һ��ʱ
	{
		for (WORD i=0; i<3; i++)
		{
			memset(&now, 0, sizeof(now));
            WaitSemaphore(g_semTime, SYS_TO_INFINITE);
			RtcGetTime(&now);
            SignalSemaphore(g_semTime);
			DTRACE(DB_CRITICAL, ("SyncTimer: NO.%d time %d/%02d/%02d %02d::%02d::%02d.\r\n", 
					i,
					now.nYear, now.nMonth, now.nDay, 
			  		now.nHour, now.nMinute, now.nSecond)); 
		
			if (!IsInvalidTime(&now))
			{
				if (now.nYear==tmOld.nYear && now.nMonth==tmOld.nMonth && 
					now.nDay==tmOld.nDay && now.nHour==tmOld.nHour)
				{
					if (SetSoftTime(&now))//ͬ��һ��ϵͳʱ��
					{
						GetCurTime(&tmLast);
						return;
					}
				}
				tmOld = now;
			}
			else
			{
				memset(&tmOld, 0, sizeof(tmOld));
				DTRACE(DB_CRITICAL, ("SyncTimer: ####### invalid time\n"));
			}
		}
		GetCurTime(&tmLast);
	}
}

//������ϵͳ��ʼ��
//��������
//���أ���
void SysInit()
{
	g_semTime = NewSemaphore(1, 1);
    InitInfo();
   	//SysDebugInit();
}

//������ϵͳ����
//��������
//���أ���
void SysExit()
{
    
}
