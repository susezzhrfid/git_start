/*********************************************************************************************************
 * Copyright (c) 2003,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�pppif.cpp
 * ժ    Ҫ�����ļ���Ҫ������PPP�ӿڵ����ݶ���ͺ���ʵ�֣���ѭlwIP�ӿڲ��������ʽ��д����
 *
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2003��4��25��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 **********************************************************************************************************/
#ifndef __LWIP_CTRL_H__
#define __LWIP_CTRL_H__

#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "lwip/ip.h"

#include "lwip/udp.h"
#include "lwip/tcp.h"

#include "lwip/api.h"
#include "ppp.h"

enum ctrl_msg_type 
{
  CTRL_MSG_CLRTIMEOUT,
  CTRL_MSG_MAX
};


struct ctrl_msg
{
  enum ctrl_msg_type type;    //�������������
  void* arg;                  //��������Ĳ���
  sys_mbox_t mbox;            //ִ�н����������
};

void ctrl_msg_input(struct ctrl_msg* msg);
err_t clr_timeout();

bool InitTCPIP(void *arg);

bool ETLinkUp(void);
void SetNetIfDefaultET(void);
void SetNetIfDefaultPPP(void);
struct netif *GetEthIf(void);
struct ip_addr GetEthGW(void);

#endif //__LWIP_CTRL_H__