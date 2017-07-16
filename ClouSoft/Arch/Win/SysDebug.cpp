/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysdebug.cpp
 * ժ    Ҫ�����ļ���Ҫ������ϵͳ�µ��ԽӿڵĶ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��6��
 * ��    ע��
 *********************************************************************************************************/
#include <stdio.h>
#include <ctype.h> 
#include <stdlib.h>
#include <stdarg.h> 
#include <string>
#include <afx.h>
//#include "SysArch.h"

using namespace std;

typedef HANDLE TSem;

extern "C"
{
	TSem   g_semDebug;
bool SysDebugInit()
{
	g_semDebug = CreateSemaphore(NULL, 1, 1, NULL);

	return true; 
}

void DTRACEOUT(char* s, int len)
{	
	printf(s);
	TRACE(s);
}

void TraceOut(const char *fmt, ...)
{
	va_list ap;
	char szBuf[40960];
	va_start(ap, fmt);
	vsprintf(szBuf, fmt, ap);
	va_end(ap);

	printf(szBuf);
	TRACE(szBuf);
}
}