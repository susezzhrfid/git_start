/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysarch.h
 * 摘    要：本文件主要实现操作系统接口的封装,如信号量,线程等
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年7月
 *********************************************************************************************************/
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__
#include "Typedef.h"
//#include "Sysdebug.h"
//#include "Sysapi.h"

#define SYS_ERR_OK       	0		//无错误
#define SYS_ERR_INVALID  	1       //设置内容非法
#define SYS_ERR_TIMEOUT  	0x20	//超时
#define SYS_ERR_SYS      	3		//系统出错

#define SYS_TO_INFINITE  	INFINITE		//超时

typedef HANDLE TSem;

#define THREAD_RET_OK	0
#define THREAD_RET_ERR	1
typedef int TThreadRet;

typedef int TPrio;


TSem NewSemaphore(WORD count, WORD max);
void FreeSemaphore(TSem sem);
WORD WaitSemaphore(TSem sem, DWORD timeout);
void SignalSemaphore(TSem sem);
void NewThread(const signed char *ThreadName, TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize, TPrio prio);
//void Sleep(DWORD dwMilliseconds);

#endif //__ARCH_SYS_ARCH_H__ 

