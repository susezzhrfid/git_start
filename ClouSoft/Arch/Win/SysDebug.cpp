/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysdebug.cpp
 * 摘    要：本文件主要包含本系统下调试接口的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年6月
 * 备    注：
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