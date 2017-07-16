/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: sys_arch.c,v 1.3 2002/01/02 17:35:42 adam Exp $
 */
#include "stdafx.h"

//#include "lwip/debug.h"

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include "lwip/stats.h"
#include "lwip/api.h"

struct sys_timeouts timeouts;
//u32_t g_jiffies = 0;
//sys_sem_t g_semJiffies;
//void jiffies_thread(void* pvPara);

/*-----------------------------------------------------------------------------------*/
sys_sem_t
sys_sem_new(u8_t count)
{
  //BOOL fInitialState = count > 0 ? TRUE : FALSE;
  //return CreateEvent(NULL, FALSE, fInitialState, NULL);
  return CreateSemaphore(NULL, count, 1, NULL);
}

sys_sem_t
sys_sem_new_m(u8_t count, u8_t max)
{
  //BOOL fInitialState = count > 0 ? TRUE : FALSE;
  //return CreateEvent(NULL, FALSE, fInitialState, NULL);
  return CreateSemaphore(NULL, count, max, NULL);
}


/*-----------------------------------------------------------------------------------*/
void
sys_sem_free(sys_sem_t sem)
{
  CloseHandle(sem);
}

/*-----------------------------------------------------------------------------------*/
void
sys_sem_wait(sys_sem_t sem)
{
  WaitForSingleObject(sem, INFINITE);
}

err_t sys_sem_wait_t(sys_sem_t sem, u32_t timeout)
{
  DWORD dw = WaitForSingleObject(sem, timeout);
  if (dw == WAIT_OBJECT_0)
    return SYS_ERR_OK;
  if (dw == WAIT_TIMEOUT)
    return SYS_ERR_TIMEOUT;
  if (dw == WAIT_FAILED)
    return SYS_ERR_SYS;	
}

/*-----------------------------------------------------------------------------------*/
void
sys_sem_signal(sys_sem_t sem)
{
  //SetEvent(sem);
  ReleaseSemaphore(sem, 1, NULL);
}


/*-----------------------------------------------------------------------------------*/
void sys_init(void)
{
  timeouts.next = NULL;
  timeouts.lasttime = 0;

  //g_semJiffies = sys_sem_new(1);
  //sys_thread_new(jiffies_thread, NULL, 0);
  return;
}

/*-----------------------------------------------------------------------------------*/
struct sys_timeouts *
sys_arch_timeouts(void)
{
  return &timeouts;
}
/*-----------------------------------------------------------------------------------*/
void
sys_thread_new(void (* function)(void *arg), void *arg, u32_t nStackSize)
{
  if (AfxBeginThread((AFX_THREADPROC)function,    //pfnThreadProc
                     (LPVOID)arg,                 //pParam
		     THREAD_PRIORITY_NORMAL,      //nPriority 
		     nStackSize,                  //nStackSize  
		     0,                           //dwCreateFlags 
		     NULL) 
       == NULL)               //lpSecurityAttrs
  {
    AfxMessageBox("Fail to create thread!");
    return;
  }
}
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
sys_mbox_t sys_mbox_new(void)
{
  struct sys_mbox *mbox;

  mbox = (struct sys_mbox* )malloc(sizeof(struct sys_mbox));
  mbox->first = mbox->last = 0;
  mbox->mail = (struct sys_sem* )sys_sem_new_m(0, SYS_MBOX_SIZE);
  mbox->space = (struct sys_sem* )sys_sem_new_m(SYS_MBOX_SIZE, 
                                                SYS_MBOX_SIZE);
  mbox->mutex = (struct sys_sem* )sys_sem_new(1);
  
#ifdef SYS_STATS
  stats.sys.mbox.used++;
  if(stats.sys.mbox.used > stats.sys.mbox.max) 
  {
    stats.sys.mbox.max = stats.sys.mbox.used;
  }
#endif /* SYS_STATS */
  
  return mbox;
}

/*-----------------------------------------------------------------------------------*/
void
sys_mbox_free(struct sys_mbox *mbox)
{
  if(mbox != SYS_MBOX_NULL) 
  {
#ifdef SYS_STATS
    stats.sys.mbox.used--;
#endif /* SYS_STATS */
    sys_sem_wait(mbox->mutex);
    
    sys_sem_free(mbox->mail);
    sys_sem_free(mbox->space);
    sys_sem_free(mbox->mutex);
    mbox->mail = mbox->mutex = NULL;
    /*  DEBUGF("sys_mbox_free: mbox 0x%lx\r", mbox);*/
    free(mbox);
  }
}



/*-----------------------------------------------------------------------------------*/
//����������ȡ�ö������ռ�ã�Ȼ�����Ϣ������Ϣ���У�����ͷŶ������ռ��
//     ����Ϣ������Ϣ���У�ʹ������֪����û������Ϣ����ͨ��������ź��������ǵ��������ͷָ��ʹ��
//     ������βָ�롣������ź���ֻ�Ǳ�֤����������Ļ��⡣
//     �μ�sys_arch_mbox_fetch()
void sys_mbox_post(struct sys_mbox *mbox, void *msg)
{
  sys_sem_wait(mbox->space);      //�ȵȴ��������пռ���ʼ�
  sys_sem_wait(mbox->mutex);      //ȡ�ö������ռ��

  DEBUGF(SYS_DEBUG, ("sys_mbox_post: mbox %p msg %p first %d last %d\r\n", 
                     mbox, msg, mbox->first, mbox->last));

  mbox->msgs[mbox->last] = msg;   //������Ϣ���е�ͷ
                                  //���msgΪNULL�������һ������Ϣ
  
  mbox->last++;                   //��Ϣ���е�ͷָ��++
  if(mbox->last == SYS_MBOX_SIZE) //Խ��ѭ�����еı߽�
  {
    mbox->last = 0;
  }

  sys_sem_signal(mbox->mail);     //��һ���µ���Ϣ�ˣ�
  
  sys_sem_signal(mbox->mutex);    //�ͷŶ������ռ��
}


/*-----------------------------------------------------------------------------------*/
//������������mbox����û��Ϣ���������ŵ�msg�������NULL�ŵ�msg
//      ������mbox->mail��������Եȴ������ȵ�mail�ź������Ѿ������ѣ������
//������@mboxΪ����,
//      @msg��������ָ��ȡ�õ���Ϣ��ָ�룬������Ϊmsg����Ϊ��
//���أ���
void sys_mbox_get(sys_mbox_t mbox, void **msg)
{
  sys_sem_wait(mbox->mutex);         //ȡ�ö������ռ��

  DEBUGF(SYS_DEBUG, ("sys_mbox_fetch: mbox %p msg %p first %d last %d\r", 
                      mbox, mbox->msgs[mbox->first], mbox->first, mbox->last));
  
  if(msg != NULL) 
  {
    *msg = mbox->msgs[mbox->first];   //������Ϣ
  }
  
  mbox->first++;                      //βָ��++����ʹ����msg==NULL��++��ʾ����
  if(mbox->first == SYS_MBOX_SIZE)    //Խ��ѭ�����еı߽�
  {
    mbox->first = 0;
  }    
  
  sys_sem_signal(mbox->space);        //���µĿռ��ˣ�

  sys_sem_signal(mbox->mutex);        //�ͷŶ������ռ��
}

/*-----------------------------------------------------------------------------------*/
//������������mbox��ȡһ����Ϣ��������msg��
//������@mboxΪ����,
//      @msg��������ָ��ȡ�õ���Ϣ��ָ�룬
//���أ���
void sys_mbox_fetch(sys_mbox_t mbox, void **msg)
{
  sys_sem_wait(mbox->mail);          //�ȵȴ����������ʼ�
  sys_mbox_get(mbox, msg);
}


/*-----------------------------------------------------------------------------------*/
//������������mbox��ȡһ����Ϣ�������޶��ȴ���ʱ�䣬��Ϣ������msg��
//������@mboxΪ����,
//      @msg��������ָ��ȡ�õ���Ϣ��ָ�룬
//      @timeout�ȴ���Ϣ��ʱ��
//���أ������ȷȡ��һ����Ϣ�򷵻�ERR_OK�������ʱ�򷵻�ERR_TIMEOUT��
//      ������ֵȴ������򷵻�ERR_SYS
err_t sys_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout)
{
  err_t err = sys_sem_wait_t(mbox->mail, timeout);       //�ȵȴ����������ʼ�
  if (err == ERR_OK)
  {
    sys_mbox_get(mbox, msg);
    return ERR_OK;
  }
  else
  {
    return err;
  }
  return ERR_OK; 
}


u32_t sys_wait_multiple(u32_t count, sys_sem_t* sem, u8_t waitall, u32_t milliseconds)
{
  return WaitForMultipleObjects(count, sem, waitall, milliseconds);
}



void sys_mbox_drain(sys_mbox_t mbox, u8_t type)
{
  void* msg;

  /* Drain the recvmbox. */
  if(mbox != SYS_MBOX_NULL) 
  {
    sys_sem_wait(mbox->mutex);          //ȡ�ö������ռ��
    while(mbox->first != mbox->last)    //û����Ϣ
    {
      msg = mbox->msgs[mbox->first++];  //������Ϣ,βָ��++
      if(mbox->first == SYS_MBOX_SIZE)  //Խ��ѭ�����еı߽�
      {
        mbox->first = 0;
      }    

      if (type == MBOX_DRAINTYPE_NETCONN)
      {
        netconn_delete((struct netconn *)msg);
      }
      else if(type == MBOX_DRAINTYPE_PBUF)
      {
	pbuf_free((struct pbuf *)msg);
      } 
      else   //MBOX_DRAINTYPE_NETBUF
      {
	netbuf_delete((struct netbuf *)msg);
      }
    }
    sys_sem_signal(mbox->mutex);    //�ͷŶ������ռ�ã�
           //��Ϊ���ͷ�����֮ǰ�����Ѿ������PCB����API�Ľӿں��������Ե��ͷ�������һ��
	       //��û�еط�����������䷢����Ϣ
    sys_mbox_free(mbox);
  }
}

u8_t sys_mbox_msgnum(sys_mbox_t mbox)
{
  u16_t num;
  sys_sem_wait(mbox->mutex);         //ȡ�ö������ռ��

  if (mbox->last >= mbox->first)     //��mbox->last = mbox->first��ʾ��ʱ����ò���
  {
    num = mbox->last - mbox->first;  
  }
  else
  {
    num = mbox->last + SYS_MBOX_SIZE - mbox->first;  
  }
 
  sys_sem_signal(mbox->mutex);       //�ͷŶ������ռ��

  return num;
}

sys_sem_t new_period_sem(u32_t milliseconds)
{
  sys_sem_t sem = CreateWaitableTimer(NULL, FALSE, NULL);  //���˹�����
  LARGE_INTEGER li;
  li.QuadPart = -(5 * 10000000);
  SetWaitableTimer(sem, &li, milliseconds, NULL, NULL, FALSE);
  return sem;
}


/*
void jiffies_thread(void* pvPara)
{
	while (1)
	{
        sys_sem_wait(g_semJiffies);
	    g_jiffies += 100;
        sys_sem_signal(g_semJiffies);
		sys_sleep(100);
		//DTRACE(DB_DATAMANAGER, ("ClickThread: Click = 0x%08x.\r\n", dwClick));
	}
}


u32_t sys_jiffies(void)
{ 
	u32_t t;
	sys_sem_wait(g_semJiffies);
    t = g_jiffies;
    sys_sem_signal(g_semJiffies);

	return t;
};	
*/

void PrintTime()
{
    SYSTEMTIME SystemTime;
    GetSystemTime(&SystemTime);
	DEBUGF(1, ("now: %d:%d:%d:%d\r\n",
                SystemTime.wHour, SystemTime.wMinute,
				SystemTime.wSecond, SystemTime.wMilliseconds));
}



/*-----------------------------------------------------------------------------------*/
TSem NewSemaphore(WORD count)
{
    return CreateSemaphore(NULL, count, 1, NULL);
}

/*-----------------------------------------------------------------------------------*/
TSem NewSemaphore(WORD count, WORD max)
{
    return CreateSemaphore(NULL, count, max, NULL);
}


TSem NewPeriodicSemaphore(DWORD dwMilliseconds)
{
    TSem sem = CreateWaitableTimer(NULL, FALSE, NULL);  //���˹�����
    LARGE_INTEGER li;
    li.QuadPart = -(5 * 10000000);
    SetWaitableTimer(sem, &li, dwMilliseconds, NULL, NULL, FALSE);
    return sem;
}


/*-----------------------------------------------------------------------------------*/
void FreeSemaphore(TSem sem)
{
    CloseHandle(sem);
}

/*-----------------------------------------------------------------------------------*/
void WaitSemaphore(TSem sem)
{
    WaitForSingleObject(sem, INFINITE);
}


/*-----------------------------------------------------------------------------------*/
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


/*-----------------------------------------------------------------------------------*/
void SignalSemaphore(TSem sem)
{
    ReleaseSemaphore(sem, 1, NULL);
}


/*-----------------------------------------------------------------------------------*/
void NewThread(int (* function)(void *arg), void *arg, DWORD nStackSize)
{
     if (AfxBeginThread((AFX_THREADPROC)function,    //pfnThreadProc
                        (LPVOID)arg,                 //pParam
		                THREAD_PRIORITY_NORMAL,      //nPriority 
		                nStackSize,                  //nStackSize  
		                0,                           //dwCreateFlags 
		                NULL) 
         == NULL)               //lpSecurityAttrs
	 {
         AfxMessageBox("Fail to create thread!");
         return;
	 }
}



