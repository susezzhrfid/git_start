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
#include <time.h>
#include "TypeDef.h"
#include "SysArch.h"
#include "SysAPI.h"
#include "SysDebug.h"

//using namespace std;

static long g_iStartClick = 0;
static long g_iLastTime = 0;
static long g_iBaseClick = 0;  //�ڸ�ʱ���ʱ�����������ʱ��ǰ���Ѿ��ߵ�click
static TSem  g_semTime;   


//��������ȡϵͳ�ϵ������ĺ�����
//��������
//���أ�ϵͳ�ϵ������ĺ�����
DWORD GetTick()
{
	return clock() / (CLOCKS_PER_SEC/1000);
}

//��������ȡϵͳ�ϵ�����������
//��������
//���أ�ϵͳ�ϵ�����������
DWORD GetClick()
{
	int64 tm;
	int64 click;
	WaitSemaphore(g_semTime, 0);

	tm = time(NULL);
	if (tm<g_iLastTime || tm-g_iLastTime>60)	//�쳣��ʱ�����
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

//������ϵͳ��ʼ��
//��������
//���أ���
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

//������ϵͳ����
//��������
//���أ���
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
