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
#include <time.h>
#include "TypeDef.h"
#include "SysArch.h"
#include "SysAPI.h"
#include "SysDebug.h"

//using namespace std;

static long g_iStartClick = 0;
static long g_iLastTime = 0;
static long g_iBaseClick = 0;  //在改时间的时候用来保存改时间前的已经走的click
static TSem  g_semTime;   


//描述：获取系统上电以来的毫秒数
//参数：无
//返回：系统上电以来的毫秒数
DWORD GetTick()
{
	return clock() / (CLOCKS_PER_SEC/1000);
}

//描述：获取系统上电以来的秒数
//参数：无
//返回：系统上电以来的秒数
DWORD GetClick()
{
	int64 tm;
	int64 click;
	WaitSemaphore(g_semTime, 0);

	tm = time(NULL);
	if (tm<g_iLastTime || tm-g_iLastTime>60)	//异常的时间调整
	{
		g_iBaseClick += g_iLastTime - g_iStartClick;
		g_iStartClick = (long)tm;
	}

	g_iLastTime = (long)tm;
	click = g_iBaseClick + tm - g_iStartClick;
	
	SignalSemaphore(g_semTime);
	return (DWORD)click;
}

bool GetSysTime(TTime* pTime)
{
	struct tm temptm;
	time_t temptime;

	time(&temptime);
	localtime_s(&temptm, &temptime);

	pTime->nYear = temptm.tm_year + 1900;
	pTime->nMonth = temptm.tm_mon + 1;
	pTime->nDay = temptm.tm_mday;
	pTime->nHour = temptm.tm_hour;
	pTime->nMinute = temptm.tm_min;
	pTime->nSecond = temptm.tm_sec;
	pTime->nWeek = temptm.tm_wday + 1;

	return true;	
}

bool SetSysTime(const TTime* pTime)
{
	SYSTEMTIME stTime;
	stTime.wYear = pTime->nYear;
	stTime.wMonth = pTime->nMonth;
	stTime.wDay = pTime->nDay;
	stTime.wHour = pTime->nHour;
	stTime.wMinute = pTime->nMinute;
	stTime.wSecond = pTime->nSecond;
	stTime.wMilliseconds = 0;
//	SetSystemTime(&stTime);
	SetLocalTime(&stTime);
	return true;
}

void SyncTimer()
{
}

//描述：系统初始化
//参数：无
//返回：无
void SysInit()
{
	WSADATA wsa;
	WORD wVersionRequested;

	SysDebugInit();
	g_semTime = NewSemaphore(1, 1);

	wVersionRequested = MAKEWORD( 2, 0 );
	if (WSAStartup(wVersionRequested , &wsa)!=0)
		DTRACE(1, ("CSocketIf::Winsock Initialization failed.\r\n"));

}

//描述：系统结束
//参数：无
//返回：无
void SysExit()
{
    
}

WORD g_wTicks = 0;
WORD GetMillSec(void)
{
	return (WORD)(g_wTicks%1000);
}

void SetMillSec(WORD wMs)
{
	g_wTicks = wMs;
}
