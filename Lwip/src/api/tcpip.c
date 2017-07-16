/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"

#include "lwip/sys.h"

#include "lwip/memp.h"
#include "lwip/pbuf.h"

#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "lwip/tcpip.h"

static void (* tcpip_init_done)(void *arg) = NULL;
static void *tcpip_init_done_arg;
static sys_mbox_t mbox;


int tcpip_state = -1;   //用于控制和反映TCP/IP内核线程状态的变量
                        //-1 关闭； 0 运行；1 待关闭
bool is_tcpip_on(void)
{
  return (tcpip_state == 0);
}

bool is_tcpip_close(void)
{
  return (tcpip_state == -1);
}

void close_tcpip_thread(void)
{
  tcpip_state = 1;   //待关闭
}


#if LWIP_TCP
//static int tcpip_tcp_timer_active = 0;

static void
tcpip_tcp_timer(void *arg)
{
  (void)arg;

  /* call TCP timer handler */
  tcp_tmr();
  /* timer still needed? */
  if (tcp_active_pcbs || tcp_tw_pcbs) {
    /* restart timer */
    sys_timeout(TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
  } else {
    /* disable timer */
    //tcpip_tcp_timer_active = 0;
  }
}

#if !NO_SYS
void
tcp_timer_needed(void)
{
/*	
  // timer is off but needed again? 
  if (!tcpip_tcp_timer_active && (tcp_active_pcbs || tcp_tw_pcbs)) {
    // enable and start timer 
    tcpip_tcp_timer_active = 1;
    sys_timeout(TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
  }
*/
}
#endif /* !NO_SYS */
#endif /* LWIP_TCP */

extern WORD g_wTcpipRunCnt;
extern void DumpDHCPAddr(void);
#if LWIP_DHCP
extern DWORD GetDhcpLeaseTime(void);
#endif
/*
sys_sem_t g_Sem250Timer = NULL;

void Timer250Init(void)
{
    g_Sem250Timer = sys_sem_new_m(0, 1);
}

void TimerOut250(void)
{
    static BYTE bTickCnt = 0;
    bTickCnt++;
    if (bTickCnt >= 250)
    {
        bTickCnt = 0;
        if (g_Sem250Timer != NULL)
            SignalSemaphoreFromISR(g_Sem250Timer);
    }
}*/

extern bool g_fDhcpStart;
//extern bool g_fDhcpRelease;
extern bool g_fDhcpStop;

static void
tcpip_thread(void *arg)
{
  struct tcpip_msg *msg;
  sys_sem_t sem[2];
  u32_t result;

  (void)arg;
  
  static BYTE bDhcpFineTime = 0;         //500ms
  static DWORD dwDhcpCoarseTime = 60*4;       //60S

  ip_init();
#if LWIP_UDP  
  udp_init();
#endif
#if LWIP_TCP
  tcp_init();
#endif
  if (tcpip_init_done != NULL) {
    tcpip_init_done(tcpip_init_done_arg);
  }
  
  sem[0] = mbox->mail;
  //sem[1] = new_period_sem(250);	 //100
  sem[1] =  /*g_Sem250Timer;*/sys_sem_new_m(0, 1);      //初始化为0个，每次等待250ms
                                      //向sys_timeout注册一个超时函数
  struct netif *netif;
  netif = GetEthIf();
    
  while (1) 
  {                          /* MAIN Loop */
  	g_wTcpipRunCnt++;
   	sys_timeout_callback();
   	result = sys_wait_multiple(2, sem, 0, INFINITE); //等待两个信号量中的任一个
    if (result == WAIT_OBJECT_0)
    {
		sys_mbox_get(mbox, (void **)&msg);   //读取全局邮箱mbox的消息
    	switch (msg->type) {
    		case TCPIP_MSG_API:
      			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: API message %p\n", (void *)msg));
      			api_msg_input(msg->msg.apimsg);
      			memp_free(MEMP_API_MSG, msg->msg.apimsg);
      			break;
    		case TCPIP_MSG_INPUT:
      			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: IP packet %p\n", (void *)msg));
      			ip_input(msg->msg.inp.p, msg->msg.inp.netif);
      			break;
    		case TCPIP_MSG_CALLBACK:
      			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: CALLBACK %p\n", (void *)msg));
      			msg->msg.cb.f(msg->msg.cb.ctx);
      			break;
        	case TCPIP_MSG_CTRL:
          		LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: ctrl message %p\r", msg));
          		ctrl_msg_input(msg->msg.ctrlmsg);
          		break;
    		default:
      			break;
    	}
    	memp_free(MEMP_TCPIP_MSG, msg);
    }
    else if (result == WAIT_OBJECT_0+1)
    {
      tcp_tmr();
      //pppif_tmr();
        //tcpip_tcp_timer(arg);
            
      if (g_fDhcpStop)
      {
          dhcp_release(netif);
          dhcp_stop(netif);
          g_fDhcpStop = false;
      }
      if (g_fDhcpStart)
      {
          if (dhcp_start(netif) == ERR_OK)
              g_fDhcpStart = false;
      }      
      
      bDhcpFineTime++;
      if (bDhcpFineTime >= 2)
      {
          bDhcpFineTime = 0;
          dhcp_fine_tmr();
      }
      if (dwDhcpCoarseTime > 0)
      {
          dwDhcpCoarseTime--;
          if (dwDhcpCoarseTime == 0)//todo：是不是应该提前续租，而不是等到到期后才去租。
          {
              dhcp_coarse_tmr();
              dwDhcpCoarseTime = GetDhcpLeaseTime()*4/2; //租期过半就续租
              if (dwDhcpCoarseTime == 0)
                  dwDhcpCoarseTime = 60*4;
          }
      }
      
      DumpDHCPAddr();
    }     
  }  
  //return;
}

err_t
tcpip_input(struct pbuf *p, struct netif *inp)
{
  struct tcpip_msg *msg;
  
  msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG);
  if (msg == NULL) {
    pbuf_free(p);    
    return ERR_MEM;  
  }
  
  msg->type = TCPIP_MSG_INPUT;
  msg->msg.inp.p = p;
  msg->msg.inp.netif = inp;
  sys_mbox_post(mbox, msg);
  return ERR_OK;
}

err_t
tcpip_callback(void (*f)(void *ctx), void *ctx)
{
  struct tcpip_msg *msg;
  
  msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG);
  if (msg == NULL) {
    return ERR_MEM;  
  }
  
  msg->type = TCPIP_MSG_CALLBACK;
  msg->msg.cb.f = f;
  msg->msg.cb.ctx = ctx;
  sys_mbox_post(mbox, msg);
  return ERR_OK;
}

void
tcpip_apimsg(struct api_msg *apimsg)
{
  struct tcpip_msg *msg;
  msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG);
  if (msg == NULL) {
    memp_free(MEMP_API_MSG, apimsg);
    return;
  }
  msg->type = TCPIP_MSG_API;
  msg->msg.apimsg = apimsg;
  sys_mbox_post(mbox, msg);
}


/*-----------------------------------------------------------------------------------*/
//描述：分配tcpip_msg结构，把传过来的ctrl_msg挂到ctrlmsg成员，
//      调用sys_mbox_post(mbox, msg);
void
tcpip_ctrlmsg(struct ctrl_msg* ctrlmsg)
{
  struct tcpip_msg *msg;
  msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG);
  if(msg == NULL) 
  {
    memp_free(MEMP_TCPIP_MSG, msg);
    return;
  }
  msg->type = TCPIP_MSG_CTRL;
  msg->msg.ctrlmsg = ctrlmsg;
  sys_mbox_post(mbox, msg);    //消息挂在全局邮箱mbox的消息队列
}

void
tcpip_init(void (* initfunc)(void *), void *arg)
{
  tcpip_init_done = initfunc;
  tcpip_init_done_arg = arg;
  mbox = sys_mbox_new();  //256
  sys_thread_new("TCPIP", tcpip_thread, NULL, 224/*192,256*/, THREAD_PRIORITY_NORMAL);
}
