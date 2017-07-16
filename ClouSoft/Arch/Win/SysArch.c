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
#include <time.h>
#include <process.h>
#include <stdio.h>
#include "Sysarch.h"


TSem NewSemaphore(WORD count, WORD max)
{
    return CreateSemaphore(NULL, count, max, NULL);
}


void FreeSemaphore(TSem sem)
{
    CloseHandle(sem);
}

WORD WaitSemaphore(TSem sem, DWORD timeout)
{
  DWORD dw = WaitForSingleObject(sem, timeout);
  if (dw == WAIT_OBJECT_0)
    return SYS_ERR_OK;
  if (dw == WAIT_TIMEOUT)
    return SYS_ERR_TIMEOUT;
  if (dw == WAIT_FAILED)
    return SYS_ERR_SYS;	

  return SYS_ERR_SYS;
}

void SignalSemaphore(TSem sem)
{
    ReleaseSemaphore(sem, 1, NULL);
}


void NewThread(const signed char *ThreadName, TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize, TPrio prio)
{
	/*
     if (AfxBeginThread((AFX_THREADPROC)function,    //pfnThreadProc
                        (LPVOID)arg,                 //pParam
		                prio,						 //nPriority 
		                nStackSize,                  //nStackSize  
		                0,                           //dwCreateFlags 
		                NULL) 
         == NULL)               //lpSecurityAttrs
	 {
        // AfxMessageBox("Fail to create thread!");
         return;
	 }
	 */
	if (_beginthread(function, nStackSize, arg) == -1)
	{
		printf("Fail to create thread!\n");
		return ;
	}
}
