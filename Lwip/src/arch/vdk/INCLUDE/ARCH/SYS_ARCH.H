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
#include "vdk.h"
#include "lwip/err.h"

#define SYS_MBOX_NULL NULL
#define SYS_SEM_NULL  (VDK::SemaphoreID)UINT_MAX

//struct sys_sem;
//typedef struct sys_sem * sys_sem_t;

//#define sys_sem      VDK::SemaphoreID
//#define sys_sem_t    VDK::SemaphoreID
DWORD GetClick();
DWORD GetTick();
inline u32_t sys_jiffies(void) { return GetTick(); }; /* since power up. */

typedef VDK::SemaphoreID sys_sem;
typedef VDK::SemaphoreID sys_sem_t;

/* Semaphore functions. */
#define SYS_INFINITE               0x7ffffff0    //0

#ifndef NOWAIT
#define NOWAIT      		   0  
#define INFINITE 			   0x7ffffff0
#endif //NOWAIT

typedef VDK::Priority  TPrio;
#define THREAD_PRIORITY_IDLE			(VDK::kPriority8)
#define THREAD_PRIORITY_LOWEST			(VDK::kPriority7)
#define THREAD_PRIORITY_BELOW_NORMAL    (VDK::kPriority6)
#define THREAD_PRIORITY_NORMAL          (VDK::kPriority5)
#define THREAD_PRIORITY_ABOVE_NORMAL    (VDK::kPriority4)
#define THREAD_PRIORITY_HIGHEST         (VDK::kPriority3)
#define THREAD_PRIORITY_TIME_CRITICAL   (VDK::kPriority2)


//#define PPP_THREAD_PRIO         THREAD_PRIORITY_NORMAL
void sys_thread_new(int (* function)(void *arg), void *arg, DWORD dwStackSize, TPrio prio);

#define WAIT_OBJECT_0          0
sys_sem_t new_period_sem(u32_t milliseconds);
#define sys_sem_getval(sem)       VDK::GetSemaphoreValue(sem)

void Sleep(DWORD dwMilliseconds);
#define sys_sleep(timeout)        Sleep(timeout)
#define sys_msleep(ms)   		  Sleep(ms)


#define SYS_MBOX_SIZE 100
struct sys_mbox {
  u16_t first, last;
  void *msgs[SYS_MBOX_SIZE];
  sys_sem mail;
  sys_sem space;
  sys_sem mutex;
};

typedef struct sys_mbox *sys_mbox_t;

struct sys_thread;
typedef struct sys_thread * sys_thread_t;

void sys_mbox_fetch(sys_mbox_t mbox, void **msg);
void sys_mbox_get(sys_mbox_t mbox, void **msg);
err_t sys_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout);

u32_t sys_wait_multiple(u8_t nsema, sys_sem sema[], int fwaitall, u32_t timeout);

u16_t sys_mbox_msgnum(sys_mbox_t mbox);

err_t sys_sem_wait_t(sys_sem_t sem, u32_t timeout);
#define sys_sem_wait_timeout sys_sem_wait_t

#define sys_arch_mbox_fetch sys_mbox_fetch

void sys_mbox_drain(sys_mbox_t mbox, u8_t type);
#define MBOX_DRAINTYPE_NETCONN     0
#define MBOX_DRAINTYPE_PBUF        1
#define MBOX_DRAINTYPE_NETBUF      2 

extern sys_sem_t sem_sys_prot;
#define SYS_ARCH_DECL_PROTECT(lev)		
#define SYS_ARCH_PROTECT(lev)			sys_sem_wait(sem_sys_prot)
#define SYS_ARCH_UNPROTECT(lev)			sys_sem_signal(sem_sys_prot)
#endif /* __ARCH_SYS_ARCH_H__ */

