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
 * $Id: sys_arch.h,v 1.1 2001/12/12 10:00:57 adam Exp $
 */
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#define SYS_MBOX_NULL NULL
#define SYS_SEM_NULL  NULL

#include "lwip/err.h"

//#define sys_sem   HANDLE 
//#define sys_sem_t HANDLE 

typedef HANDLE sys_sem;
typedef HANDLE sys_sem_t;

struct sys_mbox_msg {
  struct sys_mbox_msg *next;
  void *msg;
};

#define SYS_MBOX_SIZE 100

struct sys_mbox {
  u16_t first, last;
  void *msgs[SYS_MBOX_SIZE];
  struct sys_sem* mail;
  struct sys_sem* space;
  struct sys_sem* mutex;
};


typedef struct sys_mbox *sys_mbox_t;

struct sys_thread;
typedef struct sys_thread * sys_thread_t;
void sys_mbox_get(sys_mbox_t mbox, void **msg);
void sys_mbox_drain(sys_mbox_t mbox, u8_t type);
u32_t sys_wait_multiple(u32_t count, sys_sem_t* sem, u8_t waitall, u32_t milliseconds);
sys_sem_t new_period_sem(u32_t milliseconds);
void sys_thread_new(void (* function)(void *arg), void *arg, u32_t nStackSize);
u8_t sys_mbox_msgnum(sys_mbox_t mbox);

err_t sys_sem_wait_t(sys_sem_t sem, u32_t timeout);
err_t sys_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout);

#define sys_sleep(ms)    Sleep(ms)     
#define sys_msleep(ms)   Sleep(ms)

inline u32_t sys_jiffies(void) { return GetTickCount(); }; /* since power up. */

#define MBOX_DRAINTYPE_NETCONN     0
#define MBOX_DRAINTYPE_PBUF        1
#define MBOX_DRAINTYPE_NETBUF      2 


#define SYS_INFINITE               INFINITE   //操作系统调用无穷等待


typedef HANDLE TSem;

TSem NewSemaphore(WORD count);
TSem NewSemaphore(WORD count, WORD max);
TSem NewPeriodicSemaphore(DWORD dwMilliseconds);
void FreeSemaphore(TSem sem);
void WaitSemaphore(TSem sem);
WORD WaitSemaphore(TSem sem, DWORD timeout);
void SignalSemaphore(TSem sem);
void NewThread(int (* function)(void *arg), void *arg, DWORD nStackSize);
inline DWORD GetTick() { return GetTickCount(); };

#endif /* __ARCH_SYS_ARCH_H__ */

