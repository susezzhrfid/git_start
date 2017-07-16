/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Sysapi.c
 * 摘    要：系统相关的一些API
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#include "Sysarch.h"
#include "Sysdebug.h"
#include "Sysapi.h"
//#include "RtcBios.h"
#include "FaCfg.h"
#include "ComAPI.h"
#include "Drivers.h"
#include "FaAPI.h"

static unsigned long g_dwBaseClick = 0;  //在改时间的时候用来保存改时间前的已经走的click
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

//与基表同步时间
/*
void SyncTimer()
{
    WORD i;
	static TTime tmLast = { 0 };
	TTime now;

   	GetCurTime(&now);
	if (IsTimeEmpty(&tmLast))
		GetCurTime(&tmLast);

	if (HoursPast(&tmLast, &now)>0 && now.nMinute==57 && now.nSecond>=45)//每个小时的57分30秒同步一下时钟
	{
		for (i=0; i<3; i++)
		{			
            if (ReadInMtrTime(&g_ProEx))
            {
                SaveSysClockToDB(true); //每小时保存一次数据库
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
			  		now.nHour, now.nMinute, now.nSecond, GetClick())); //刚上电的时候打印下当前的时间
    }
	GetCurTime(&now);
	if (HoursPast(&tmLast, &now)>0 && now.nMinute==57 && now.nSecond>=45)//每个小时的57分30秒同步一下时钟
//  if (MinutesPast(tmLast, now) > 15)//每15分钟对一次时
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
					if (SetSoftTime(&now))//同步一下系统时间
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

//描述：系统初始化
//参数：无
//返回：无
void SysInit()
{
	g_semTime = NewSemaphore(1, 1);
    InitInfo();
   	//SysDebugInit();
}

//描述：系统结束
//参数：无
//返回：无
void SysExit()
{
    
}
