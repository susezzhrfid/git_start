/**
 * @file
 * Ping sender module
 *
 */

/*
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
 */

/** 
 * This is an example of a "ping" sender (with raw API and socket API).
 * It can be used as a start point to maintain opened a network connection, or
 * like a network "watchdog" for your device.
 *
 */

#include "lwip/opt.h"

#if LWIP_RAW && LWIP_ICMP /* don't build if not configured for use in lwipopts.h */

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
//#include "lwip/inet_chksum.h"
#include "ctrl.h"
#include "ping.h"
//#ifdef LWIP_SOCKET
//#undef LWIP_SOCKET    //选择原生PING，更省资源.另一种基于SOCKET.注意原生PING直接使用了硬件，没有受TCPIP线程保护,会有资源冲突
//#endif

/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DEBUG//LWIP_DBG_ON
#endif

/** ping target - should be a "struct ip_addr" */
#ifndef PING_TARGET
#define PING_TARGET   GetEthGW();//(netif_default?netif_default->gw:ip_addr_any)
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

/* ping variables */
static u16_t ping_seq_num;
static u32_t ping_time;
#if !LWIP_SOCKET
static struct raw_pcb *pcb;
#endif /* LWIP_SOCKET */

#if NO_SYS
/* port-defined functions used for timer execution */
void sys_msleep(u32_t ms);
u32_t sys_now();
#endif /* NO_SYS */


#define STATE_INIT      0
#define STATE_PING      2
#define STATE_TIMEOUT   3
#define STATE_WAIT      4

static BYTE bPingFailCnt = 1;

/** Prepare a echo ICMP request */
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = htons(++ping_seq_num);

  /* fill the additional data buffer with some data */
  for(i = 0; i < data_len; i++) {
    ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
  }

  iecho->chksum = inet_chksum(iecho, len);
}

#if LWIP_SOCKET

/* Ping using the socket ip */
static err_t
ping_send(int s, struct ip_addr *addr)
{
  int err;
  struct icmp_echo_hdr *iecho;
  struct sockaddr_in to;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
  LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

  if (!(iecho = mem_malloc((mem_size_t)ping_size))) {
    return ERR_MEM;
  }

  ping_prepare_echo(iecho, (u16_t)ping_size);

  to.sin_len = sizeof(to);
  to.sin_family = AF_INET;
  to.sin_addr.s_addr = addr->addr;

  err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

  mem_free(iecho);

  return (err ? ERR_OK : ERR_VAL);
}

static void
ping_recv(int s)//注意外部的PING也可以接收到，但是PING_ID与帧序号会不一样。
{
  char buf[64];
  int fromlen, len;
  struct sockaddr_in from;
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *iecho;

  while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0) {
    if (len >= (sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) {
      LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
      ip_addr_debug_print(PING_DEBUG, (struct ip_addr *)&(from.sin_addr));
      LWIP_DEBUGF( PING_DEBUG, (" %lu ms\n", (sys_now()-ping_time)));

      iphdr = (struct ip_hdr *)buf;
      iecho = (struct icmp_echo_hdr *)(buf+(IPH_HL(iphdr) * 4));
      if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
        /* do some ping result processing */
        PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
        return;
      } else {
        LWIP_DEBUGF( PING_DEBUG, ("ping: drop\n"));
      }
    }
  }

  if (len == 0) {
    LWIP_DEBUGF( PING_DEBUG, ("ping: recv - %lu ms - timeout\n", (sys_now()-ping_time)));
  }

  /* do some ping result processing */
  PING_RESULT(0);
}

static void
ping_thread(void *arg)
{
  int s;
  int timeout = PING_RCV_TIMEO;
  struct ip_addr ping_target;

  LWIP_UNUSED_ARG(arg);

  if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
    return;
  }

  lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  while (1) {
    ping_target = PING_TARGET;

    if (ping_send(s, &ping_target) == ERR_OK) {
      LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      LWIP_DEBUGF( PING_DEBUG, ("\n"));
      ping_time = sys_now();
      ping_recv(s);
    } else {
      LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      LWIP_DEBUGF( PING_DEBUG, (" - error\n"));
    }
    sys_msleep(PING_DELAY);
  }
}

static bool
my_ping_recv(int s)//注意外部的PING也可以接收到，但是PING_ID与帧序号会不一样。
{
  char buf[64];
  int fromlen, len;
  struct sockaddr_in from;
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *iecho;

  if((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0) 
  {
    if (len >= (sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) 
    {
        DTRACE(0, ("ping: recv "));
      ip_addr_debug_print(PING_DEBUG, (struct ip_addr *)&(from.sin_addr));
      DTRACE(0, (" %lu ms\n", (sys_now()-ping_time)));

      iphdr = (struct ip_hdr *)buf;
      iecho = (struct icmp_echo_hdr *)(buf+(IPH_HL(iphdr) * 4));
      if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) 
      {
        /* do some ping result processing */
        PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
        return true;
      } 
      else 
      {
          UpdPingTime();
          DTRACE(0, ("ping: drop\n"));
      }
    }
  }

  if (len == 0) {
    LWIP_DEBUGF( PING_DEBUG, ("ping: recv - %lu ms - timeout\n", (sys_now()-ping_time)));
  }

  /* do some ping result processing */
  PING_RESULT(0);
  return false;
}
/*
static void
my_ping_thread(void *arg)
{
  int s;
  int timeout = PING_RCV_TIMEO;
  struct ip_addr ping_target;

  LWIP_UNUSED_ARG(arg);

  if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
    return;
  }

  lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  while (1) {
    ping_target = PING_TARGET;

    if (ping_send(s, &ping_target) == ERR_OK) {
      //LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
        DTRACE(0, ("ping:send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      //LWIP_DEBUGF( PING_DEBUG, ("\n"));
        DTRACE(0, ("\n"));
      ping_time = sys_now();
      my_ping_recv(s);
    } else {
      //LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
        DTRACE(0, ("ping:send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      //LWIP_DEBUGF( PING_DEBUG, (" - error\n"));
        DTRACE(0, (" - error\n"));
    }
    sys_msleep(PING_DELAY);
  }
}*/
DWORD g_dwPingInter;
void UpdPingTime(void)
{
    g_dwPingInter = GetTick();
}

void CheckEthernet(void)
{
    static BYTE bState = STATE_INIT;
    static DWORD dwTick;
    struct netif *netif;
    netif = GetEthIf();
    
    static int s;
    //int timeout = PING_RCV_TIMEO;
    struct ip_addr ping_target = PING_TARGET;    
    switch(bState)
    {
    case STATE_INIT:  
        if (netif->flags & NETIF_FLAG_LINK_UP)//以太网初始化完成后才能PING
        {            
            if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) 
                break;

            //lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            unsigned int arg = 1;
		    ioctlsocket(s, FIONBIO,  (unsigned long* )&arg);//设置为非阻塞		
		    arg = 1;
		    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int));
            
            bState = STATE_PING;
            
            UpdPingTime();
        }        
        break;    
    case STATE_PING:
        if ((ip_addr_cmp(IP_ADDR_ANY, &(netif->ip_addr))) == true)//本机IP是0，说明DHCP还没有成功，先不PING
            break;
        //raw_bind(pcb, &netif->ip_addr);  //挷定一个本机IP地址,由于DHCP，IP不一定每次相同
        if (ping_send(s, &ping_target) == ERR_OK)
        {
            dwTick = GetTick();       
            ping_time = sys_now();
            bState = STATE_TIMEOUT;
        }
        break;    
    case STATE_TIMEOUT:
        if (my_ping_recv(s))
        {            
            //dwTick = GetTick();
            UpdPingTime();
            bState = STATE_WAIT;
            bPingFailCnt = 0;
        }
        else if (GetTick()-dwTick > 8000) //电脑的PING超时都是4S
        {                  
            if (bPingFailCnt < 255) //连续失败次数不超过255，超过就保持
                bPingFailCnt++;  
            UpdPingTime();
            bState = STATE_WAIT;
            DTRACE(PING_DEBUG, ("Ping request time out.\r\n"));
        }
        break;
    case STATE_WAIT:
        my_ping_recv(s);  //电脑PING终端时   todo:当电脑不停地PING终端时需不需要去取一下呢？
        if (GetTick()-g_dwPingInter > 30000)  //每30S Ping一次,以免影响GPRS
        {
            bState = STATE_PING;
        }
        break;
    default:
        break;
    }
}

#else /* LWIP_SOCKET */

static bool fPingOK;

/* Ping using the raw ip */
static u8_t
ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *addr)
{
  struct icmp_echo_hdr *iecho;
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(pcb);

  if (pbuf_header( p, -PBUF_IP_HLEN)==0) {
    iecho = p->payload;

    if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
      LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
      ip_addr_debug_print(PING_DEBUG, addr);
      LWIP_DEBUGF( PING_DEBUG, (" %lu ms\n", (sys_now()-ping_time)));

      /* do some ping result processing */
      PING_RESULT(1);  
      pbuf_free(p);
      
      fPingOK = true;
      return 1;  /* eat the event */
    }
    pbuf_header( p, PBUF_IP_HLEN); //不是我要的报文应该还原
  }
  return 0; 
}

/*static*/ void
ping_send(struct raw_pcb *raw, struct ip_addr *addr)
{
  struct pbuf *p;
  struct icmp_echo_hdr *iecho;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

  if (!(p = pbuf_alloc(PBUF_IP, ping_size, PBUF_RAM))) {
    return;
  }
  if ((p->len == p->tot_len) && (p->next == NULL)) {
    iecho = p->payload;

    ping_prepare_echo(iecho, ping_size);

    raw_sendto(raw, p, addr);
    ping_time = sys_now();
  }
  pbuf_free(p);
}

extern err_t my_raw_sendto(struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *ipaddr);

static void
my_ping_send(struct raw_pcb *raw, struct ip_addr *addr)
{
  struct pbuf *p;
  struct icmp_echo_hdr *iecho;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

  if (!(p = pbuf_alloc(PBUF_IP, ping_size, PBUF_RAM))) {
    return;
  }
  if ((p->len == p->tot_len) && (p->next == NULL)) {
    iecho = p->payload;

    ping_prepare_echo(iecho, ping_size);

    my_raw_sendto(raw, p, addr);
    ping_time = sys_now();
  }
  pbuf_free(p);
}

static void
ping_timeout(void *arg)
{
  struct raw_pcb *pcb = (struct raw_pcb*)arg;
  struct ip_addr ping_target = PING_TARGET;
  
  LWIP_ASSERT("ping_timeout: no pcb given!", pcb != NULL);

  LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
  ip_addr_debug_print(PING_DEBUG, &ping_target);
  LWIP_DEBUGF( PING_DEBUG, ("\n"));

  //ping_send(pcb, &ping_target);
  my_ping_send(pcb, &ping_target);

  sys_timeout(PING_DELAY, ping_timeout, pcb);
}

static void
ping_raw_init(void)
{
    struct netif *netif;
    netif = GetEthIf();
    
  if (!(pcb = raw_new(IP_PROTO_ICMP))) {
    return;
  }

  raw_recv(pcb, ping_recv, NULL);
  //raw_bind(pcb, IP_ADDR_ANY);
  raw_bind(pcb, &netif->ip_addr);  //挷定一个本机IP地址
  //sys_timeout(PING_DELAY, ping_timeout, pcb);
}

static void
my_ping(void)
{    
    struct ip_addr ping_target = PING_TARGET;
    struct netif *netif;
    netif = GetEthIf();
    ping_target = netif->gw;
    
    my_ping_send(pcb, &ping_target);
}

#if NO_SYS
void
ping_send_now()
{
  ping_timeout((void*)pcb);
}
#endif /* NO_SYS */

#endif /* LWIP_SOCKET */

void
ping_init(void)
{
#if LWIP_SOCKET
  sys_thread_new("ping_thread", ping_thread, NULL, 256, DEFAULT_THREAD_PRIO);//DEFAULT_THREAD_STACKSIZE
#else /* LWIP_SOCKET */
  ping_raw_init();
#endif /* LWIP_SOCKET */
}

#endif /* LWIP_RAW && LWIP_ICMP */

/*
void CheckEthernet(void)
{
    static BYTE bState = STATE_INIT;
    static DWORD dwTick;
    struct netif *netif;
    netif = GetEthIf();
    switch(bState)
    {
    case STATE_INIT:  
        if (netif->flags & NETIF_FLAG_LINK_UP)//以太网初始化完成后才能PING
        {
            ping_init();
            fEthPingOK = false;
            bState = STATE_PING;
        }        
        break;    
    case STATE_PING:
        if ((ip_addr_cmp(IP_ADDR_ANY, &(netif->ip_addr))) == true)//本机IP是0，说明DHCP还没有成功，先不PING
        {
            break;
        }
        raw_bind(pcb, &netif->ip_addr);  //挷定一个本机IP地址,由于DHCP，IP不一定每次相同
        my_ping();
        dwTick = GetTick();
        fPingOK = false;
        bState = STATE_TIMEOUT;
        break;    
    case STATE_TIMEOUT:
        if (fPingOK)
        {
            fEthPingOK = true;
            dwTick = GetTick();
            bState = STATE_WAIT;
            g_bPingFailCnt = 0;
        }
        else if (GetTick()-dwTick > 4000) //电脑的PING超时都是4S
        {
            g_bPingFailCnt++;            
            fEthPingOK = false;
            dwTick = GetTick();
            bState = STATE_WAIT;
            DTRACE(PING_DEBUG, ("Ping request time out.\r\n"));
        }
        break;
    case STATE_WAIT:
        if (GetTick()-dwTick > 10000)  //每10S Ping一次,以免影响GPRS
        {
            bState = STATE_PING;
        }
        break;
    default:
        break;
    }
}*/

BYTE GetEthPingFailCnt(void)
{
    return bPingFailCnt;
}

bool DhcpGetIpOver(void)
{
    struct netif *netif;
    netif = GetEthIf();
    if ((ip_addr_cmp(IP_ADDR_ANY, &(netif->ip_addr))) == true)
        return false;
    return true;
}