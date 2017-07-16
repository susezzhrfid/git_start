/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysdebug.h
 * 摘    要：本文件主要包含本系统下调试接口的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef SYSDEBUG_H
#define SYSDEBUG_H
#include <stdlib.h>
#include <stdio.h>
#include "TypeDef.h"

//调试输出接口的定义
extern void TraceOut(fmt, ...);
extern bool IsDebugOn(BYTE bType);

//描述：初始化调试输出功能
//参数：无
//返回：true-成功,false-失败
bool SysDebugInit();

#define DTRACE(debug, x) do { if (IsDebugOn(debug)){ TraceOut x;}} while(0)	
#define STRACE(debug, s, len) do { if (IsDebugOn(debug)){ DTRACEOUT(s, len);}} while(0)
void DTRACEOUT(char* s, int len);

#endif //SYSDEBUG_H