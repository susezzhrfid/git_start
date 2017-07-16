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
 * $Id: LWIPOPTS.H,v 1.3 2006/08/02 01:38:34 cvs_admin Exp $
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "SysCfg.h"

/* ---------- Memory options ---------- */
#define MEM_ALIGNMENT           4

#define MEM_SIZE                5*1024//3*1024//4096//2048    //0x800     总的内存池大小sys_mbox（176），PPP_MRU（1500）

#define MEM_RECLAIM             1
#define MEMP_RECLAIM            1

#define MEMP_NUM_PBUF           16//20//4 //PBUF_ROM类型pbuf结构(没数据区)的个数
#define MEMP_NUM_UDP_PCB        4//2
#define MEMP_NUM_TCP_PCB        2//4//3//2
#define MEMP_NUM_TCP_PCB_LISTEN 2
#define MEMP_NUM_TCP_SEG        9//16//10
#define MEMP_NUM_NETBUF         2//8//2
#define MEMP_NUM_NETCONN        4//2 
#define MEMP_NUM_API_MSG        8//2
#define MEMP_NUM_TCPIP_MSG      8

#define MEMP_NUM_SYS_TIMEOUT    6   //使用sys_timeout的个数

#define MEMP_NUM_PPPOE_INTERFACES  1//0 //PPPOE接口数,不使用PPPOE时定义为0可以节省内存占用

/* ---------- Pbuf options ---------- */
#define PBUF_POOL_SIZE          18//16//8//12//5           //份数          //收发共可同时缓存的帧数
#define PBUF_POOL_BUFSIZE       128//256         //每份的大小

#define PBUF_LINK_HLEN          16

/* ---------- TCP options ---------- */
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP Maximum segment size. */
#define TCP_MSS                 1500

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             2150

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
//#define TCP_SND_QUEUELEN        2 * TCP_SND_BUF/TCP_MSS
#define TCP_SND_QUEUELEN        ((6 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))//可以支持将一个大包折分成多少个包发送

/* TCP receive window. */
#define TCP_WND                 1500//1024

/* Maximum number of retransmissions of data segments. */
//#define TCP_MAXRTX              12

/* Maximum number of retransmissions of SYN segments. */
//#define TCP_SYNMAXRTX           6//4

/* ---------- ARP options ---------- */
#define ARP_TABLE_SIZE 10//2

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD              0

/* If defined to 1, IP options are allowed (but not parsed). If
   defined to 0, all packets with IP options are dropped. */
#define IP_OPTIONS              1

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255


/* ---------- UDP options ---------- */
#define UDP_TTL                 255


#define LWIP_STATS                      0
/*
// ---------- Statistics options ---------- 
#define STATS

#ifdef STATS
//#define LINK_STATS
#define IP_STATS
#define ICMP_STATS
#define UDP_STATS
#define TCP_STATS
#define MEM_STATS
#define MEMP_STATS
#define PBUF_STATS
#define SYS_STATS
#endif // STATS */

#define PPP_SUPPORT         1
#define PPPOE_SUPPORT       1
#define PPPOS_SUPPORT       1
#define mem_ptr_t (unsigned long)
#define LWIP_PROVIDE_ERRNO  1
#define LWIP_DEBUG          1
#define LWIP_DHCP           1
#define LWIP_NOASSERT
#define SO_REUSE            1  //


#define PAP_SUPPORT      1      /* Set for PAP. */

#ifndef SYS_LWIP_DEBUG
//#define PPP_DEBUG          	40
#define ETHARP_DEBUG       	0//41
#define NETIF_DEBUG        	0//42
#define PBUF_DEBUG         	0//43
#define API_LIB_DEBUG      	0//44
#define API_MSG_DEBUG      	0//45
#define SOCKETS_DEBUG      	0//46
#define ICMP_DEBUG         	0//47
#define INET_DEBUG         	0//48
#define IP_DEBUG           	0//49
#define IP_REASS_DEBUG      0//50
#define RAW_DEBUG           0//51
#define MEM_DEBUG           0//52
#define MEMP_DEBUG          0//53
#define SYS_DEBUG           0//54
#define TCP_DEBUG           55
#define TCP_INPUT_DEBUG     56
#define TCP_FR_DEBUG        57
#define TCP_RTO_DEBUG       58
#define TCP_REXMIT_DEBUG    59
#define TCP_CWND_DEBUG      60
#define TCP_WND_DEBUG       61
#define TCP_OUTPUT_DEBUG    0//62
#define TCP_RST_DEBUG       63
#define TCP_QLEN_DEBUG      64
#define UDP_DEBUG           0//65
#define TCPIP_DEBUG         66
#define PPP_DEBUG           0//67
#define SLIP_DEBUG          0//68
#define DHCP_DEBUG          0//69
#define ASSERT_DEBUG		0//70
#endif

#endif /* __LWIPOPTS_H__ */
