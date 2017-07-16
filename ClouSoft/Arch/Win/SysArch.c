/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysarch.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֲ���ϵͳ�ӿڵķ�װ,���ź���,�̵߳�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��7��
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
