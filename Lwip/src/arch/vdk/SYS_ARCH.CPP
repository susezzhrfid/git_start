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

//#include "lwip/debug.h"

#define WIN32_LEAN_AND_MEAN

#include "Const.h"
#include "lwip/stats.h"
#include "lwip/api.h"
#include "VDK.h"
#include "tGeneral.h"
#include "sysarch.h"

struct sys_timeouts timeouts;
sys_sem_t sem_sys_prot;


/*-----------------------------------------------------------------------------------*/
sys_sem_t sys_sem_new(u8_t count)
{
    return VDK::CreateSemaphore(count,1,0,0);  //0x7ffffff0
}

sys_sem_t sys_sem_new_m(u8_t count, u8_t max)
{
    return VDK::CreateSemaphore(count,max,0,0);
}

sys_sem_t new_period_sem(u32_t milliseconds)
{
	return VDK::CreateSemaphore(0, 1, milliseconds*TICKPERMS, milliseconds*TICKPERMS);
}

/*-----------------------------------------------------------------------------------*/
void sys_sem_free(sys_sem_t sem)
{
    VDK::DestroySemaphore(sem);
}

/*-----------------------------------------------------------------------------------*/
void sys_sem_wait(sys_sem_t sem)
{
   VDK::PendSemaphore(sem,0x7ffffff0);
   VDK::SystemError Error;
   Error=VDK::GetLastThreadError();
   if (Error==VDK::kSemaphoreTimeout)
   {
     VDK::DispatchThreadError((VDK::SystemError)0,0);
   }   
}

err_t sys_sem_wait_t(sys_sem_t sem, u32_t timeout)
{
	DWORD dw;
    VDK::DispatchThreadError((VDK::SystemError)0,0);
  
    if (timeout == NOWAIT)
    	VDK::PendSemaphore(sem, 1);
    else
   		VDK::PendSemaphore(sem, timeout*TICKPERMS);
   	
   	VDK::SystemError Error;
   	Error=VDK::GetLastThreadError();
   	dw = DWORD(Error);
   	if (Error==VDK::kSemaphoreTimeout)
   	{
     	VDK::DispatchThreadError((VDK::SystemError)0,0);
   	}   
   	
  	if (dw == 0)
   	{   	
    	return SYS_ERR_OK;
   	}
  	else
  	{
     	return SYS_ERR_TIMEOUT;
  	}
}


/*-----------------------------------------------------------------------------------*/
void sys_sem_signal(sys_sem_t sem)
{
	VDK::PostSemaphore(sem);
}

/*-----------------------------------------------------------------------------------*/
struct sys_timeouts *
sys_arch_timeouts(void)
{
  return &timeouts;
}


/*-----------------------------------------------------------------------------------*/
void sys_init(void)
{
  timeouts.next = NULL;
  timeouts.lasttime = 0;

  sem_sys_prot = sys_sem_new(1);
  
  return;
}

/*-----------------------------------------------------------------------------------*/
void sys_thread_new(int (* function)(void *arg), void *arg, DWORD dwStackSize, TPrio prio)
{
	TThreadData ThreadData;
	ThreadData.function = function;
	ThreadData.arg = arg;

  	VDK::ThreadCreationBlock inOutTCB;
  	inOutTCB.template_id = ktGeneral;  //a ThreadType defined in the vdk.h and vdk.cpp 
  	//inOutTCB.thread_id = 0;  //an output only field
  	inOutTCB.thread_stack_size = dwStackSize;
  	inOutTCB.thread_priority = prio;
  	inOutTCB.user_data_ptr = &ThreadData;
  	//inOutTCB.pTemplate = NULL;  //is a member used by VDK internally and does not need to be intilialised
  
  	VDK::CreateThreadEx(&inOutTCB);   //VDK::ThreadID id = 
}



/*-----------------------------------------------------------------------------------*/
sys_mbox_t sys_mbox_new(void)
{
  struct sys_mbox *mbox;

  mbox = (struct sys_mbox* )mem_malloc(sizeof(struct sys_mbox));
  mbox->first = mbox->last = 0;
  mbox->mail = (sys_sem )sys_sem_new_m(0, SYS_MBOX_SIZE);
  mbox->space = (sys_sem )sys_sem_new_m(SYS_MBOX_SIZE, 
                                                SYS_MBOX_SIZE);
  mbox->mutex = (sys_sem )sys_sem_new(1);
  
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
    mbox->mail = mbox->mutex = SYS_SEM_NULL;
    /*  DEBUGF("sys_mbox_free: mbox 0x%lx\r", mbox);*/
    mem_free(mbox);
  }
}



/*-----------------------------------------------------------------------------------*/
//描述：首先取得对邮箱的占用，然后把消息挂在消息队列，最后释放对邮箱的占用
//     把消息挂在消息队列，使消费者知道有没有新消息并不通过邮箱的信号量，而是递增邮箱的头指针使它
//     不等于尾指针。邮箱的信号量只是保证对邮箱操作的互斥。
//     参见sys_arch_mbox_fetch()
void sys_mbox_post(struct sys_mbox *mbox, void *msg)
{
  sys_sem_wait(mbox->space);      //先等待邮箱里有空间放邮件
  sys_sem_wait(mbox->mutex);      //取得对邮箱的占用

  DEBUGF(SYS_DEBUG, ("sys_mbox_post: mbox %p msg %p first %d last %d\r", 
                     mbox, msg, mbox->first, mbox->last));

  mbox->msgs[mbox->last] = msg;   //挂在消息队列的头
                                  //如果msg为NULL，则挂了一条空消息
  
  mbox->last++;                   //消息队列的头指针++
  if(mbox->last == SYS_MBOX_SIZE) //越过循环队列的边界
  {
    mbox->last = 0;
  }

  sys_sem_signal(mbox->mail);     //加一条新的消息了！
  
  sys_sem_signal(mbox->mutex);    //释放对邮箱的占用
}


/*-----------------------------------------------------------------------------------*/
//描述：看邮箱mbox里有没消息，如果有则放到msg，否则把NULL放到msg
//      不消费mbox->mail，这是针对等待函数等到mail信号量（已经先消费）的情况
//参数：@mbox为邮箱,
//      @msg用来返回指向取得的消息的指针，这里认为msg不能为空
//返回：无
void sys_mbox_get(sys_mbox_t mbox, void **msg)
{
  sys_sem_wait(mbox->mutex);         //取得对邮箱的占用

  DEBUGF(SYS_DEBUG, ("sys_mbox_get: mbox %p msg %p first %d last %d\r", 
                      mbox, mbox->msgs[mbox->first], mbox->first, mbox->last));
  
  if(msg != NULL) 
  {
    *msg = mbox->msgs[mbox->first];   //返回消息
  }
  
  mbox->first++;                      //尾指针++，即使上面msg==NULL，++表示消费
  if(mbox->first == SYS_MBOX_SIZE)    //越过循环队列的边界
  {
    mbox->first = 0;
  }    
  
  sys_sem_signal(mbox->space);        //有新的空间了！

  sys_sem_signal(mbox->mutex);        //释放对邮箱的占用
}

/*-----------------------------------------------------------------------------------*/
//描述：从邮箱mbox里取一条消息，返回在msg。
//     有新消息的标准是邮箱的头指针不等于尾指针。取得对邮箱的信号量的占用并不代表有新消息，而只是保证
//     对邮箱操作的互斥。
//     参见sys_mbox_post()
//参数：@mbox为邮箱,@msg用来返回指向取得的消息的指针，
//     @timeout等待消息的时间，=0无穷等待，!=0等待的时间
//返回：无
void sys_mbox_fetch(sys_mbox_t mbox, void **msg)
{
  sys_sem_wait(mbox->mail);          //先等待邮箱里有邮件
  sys_mbox_get(mbox, msg);
}


err_t sys_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout)
{
  err_t err = sys_sem_wait_t(mbox->mail, timeout*TICKPERMS);       //先等待邮箱里有邮件
  if (err == ERR_OK)
  {
    sys_mbox_get(mbox, msg);
    return ERR_OK;
  }
  else
  {
    return err;
  }
}

void sys_mbox_drain(sys_mbox_t mbox, u8_t type)
{
  void* msg;

  /* Drain the recvmbox. */
  if(mbox != SYS_MBOX_NULL) 
  {
    sys_sem_wait(mbox->mutex);          //取得对邮箱的占用
    while(mbox->first != mbox->last)    //没有消息
    {
      msg = mbox->msgs[mbox->first++];  //返回消息,尾指针++
      if(mbox->first == SYS_MBOX_SIZE)  //越过循环队列的边界
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
    sys_sem_signal(mbox->mutex);    //释放对邮箱的占用，
    sys_mbox_free(mbox);
  }
}


u16_t sys_mbox_msgnum(sys_mbox_t mbox)
{
  u16_t num;
  sys_sem_wait(mbox->mutex);         //取得对邮箱的占用
  
  num = sys_sem_getval(mbox->mail);
 
  sys_sem_signal(mbox->mutex);       //释放对邮箱的占用

  return num;
}

u32_t sys_wait_multiple(u8_t nsema, sys_sem sema[], int fwaitall, u32_t timeout)
{
	u8_t i;
	
	i=0;
	while (1)
	{
		VDK::PendSemaphore(sema[i],20);
    volatile VDK::SystemError Error;
    Error = VDK::GetLastThreadError();
   	if (Error != VDK::kNoError)
   	  VDK::DispatchThreadError((VDK::SystemError)0,0);
    else
		{
		  return i;
		}
		i++;
		if (i==nsema) i=0;
	}
}



