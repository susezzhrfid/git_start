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
//#include "stdafx.h"
#include "lwip/debug.h"
#include "lwip/arch.h"
#include "lwip/api_msg.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/ctrl.h"
#include "dhcp.h"
#include "ppp.h"
#include "etharp.h"

#include "ComAPI.h"
#include "LibDbAPI.h"

/*-----------------------------------------------------------------------------------*/
//����������һ��PPP�ӿ�,��Ӧ�ò����
err_t clr_timeout()
{
  struct ctrl_msg ctrlmsg;

  if((ctrlmsg.mbox = sys_mbox_new()) == SYS_MBOX_NULL) 
  {
    return ERR_MEM;
  }
  ctrlmsg.type = CTRL_MSG_CLRTIMEOUT;
  ctrlmsg.arg = (void* )NULL;
  tcpip_ctrlmsg(&ctrlmsg);
  sys_mbox_fetch(ctrlmsg.mbox, NULL);
  sys_mbox_free(ctrlmsg.mbox); 
  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
//����������һ��PPP�ӿ�,��TCP/IP�ں˵���
static void do_clr_timeout(void* arg, sys_mbox_t mbox)
{
	sys_timeout_clear();	
  	sys_mbox_post(mbox, NULL);  //�����һ������Ϣ��ͬ����ʹ��Ϣ���е�ͷָ���ƶ�
}


/*-----------------------------------------------------------------------------------*/
typedef void (* ctrl_msg_decode)(void* arg, sys_mbox_t mbox);
static ctrl_msg_decode decode[] = {    //API_MSG_MAXcode������API�Ľӿں���ָ��
  do_clr_timeout,
};



void ctrl_msg_input(struct ctrl_msg* msg)
{  
  decode[msg->type]((void* )(msg->arg), msg->mbox);
}


/* The MAC address used for demo */
//static uint8_t gMacAddress[6] = {0x00, 0x1e, 0x90, 0xaa, 0xb7, 0x1c};
//static const BYTE gMacAddress[6] = {0x7e,0xa4,0x49,0x6c, 0x01, 0x00};

/* The IP address used for demo (ping ...) */
//static const BYTE gIpAddress[4] = {192, 168, 1, 3};

/* Set the default router's IP address. */
//static const BYTE gGateWay[4] = {10, 98, 97, 7};

/* The NetMask address */
//static const BYTE gNetMask[4] = {255, 255, 255, 0};

//static const uint8_t gDNSSvr[4] = {10, 98, 94, 5};

static u8_t   dhcp_state = DHCP_INIT;
bool   g_fEnDHCP = false;  
//BYTE g_bNetIp[12] = {0};

struct netif gNetIf;
bool fUp = false;

void StatusCb(struct netif* netif)
{
	if (netif->flags & NETIF_FLAG_UP)
	{
		DTRACE(0, ("Netif Status Changed Into Up!\n"));
		fUp = true;
	}
	else
	{
		DTRACE(0, ("Netif Status Changed Into Dwon!\n"));
		fUp = false;
	}	
}

extern err_t ethernetif_init( struct netif *pxNetIf );
//extern void ethernet_set_mac(struct netif *pxNetIf, BYTE *pMacAddr);
/* Called from the TCP/IP thread. */
void NetIfInit( void *pvArgument )
{
	struct ip_addr ipaddr, netmask, gw;
	struct netif *netif;
	BYTE* bNetIp = (BYTE* )pvArgument;
    BYTE bNetContTye;

    bNetContTye = bNetIp[12]; //0-��̫����1-PPPOE
        
	if ((bNetIp[0]==0 && bNetIp[1]==0 && bNetIp[2]==0 && bNetIp[3]==0) && bNetContTye==0)  //PPPOE�Զ�Э��IP
		g_fEnDHCP = true;
	if (g_fEnDHCP)
	{
		IP4_ADDR(&gw, 0, 0, 0, 0);
		IP4_ADDR(&ipaddr, 0, 0, 0, 0);
		IP4_ADDR(&netmask, 0, 0, 0, 0);
	}
	else
	{
		IP4_ADDR(&gw, bNetIp[8], bNetIp[9], bNetIp[10], bNetIp[11]);
		IP4_ADDR(&ipaddr, bNetIp[0], bNetIp[1], bNetIp[2], bNetIp[3]);
		IP4_ADDR(&netmask, bNetIp[4], bNetIp[5], bNetIp[6], bNetIp[7]);
		//IP4_ADDR(&gw, gGateWay[0], gGateWay[1], gGateWay[2], gGateWay[3]);
		//IP4_ADDR(&ipaddr, gIpAddress[0], gIpAddress[1], gIpAddress[2], gIpAddress[3]);
		//IP4_ADDR(&netmask, gNetMask[0], gNetMask[1], gNetMask[2], gNetMask[3]);
		//IP4_ADDR(&dns_srv, gDNSSvr[0], gDNSSvr[1], gDNSSvr[2], gDNSSvr[3]);
		//dns_setserver(0, &dns_srv);
	}
    
    gNetIf.name[0] = 'e';
    gNetIf.name[1] = 't';//netif_set_addr ��ӡ�ӿ�ʱ��Ҫ
    
	netif = netif_add(&gNetIf, &ipaddr, &netmask, &gw, NULL, ethernetif_init, NULL);
//	ethernet_set_mac(&gNetIf, (BYTE *)gMacAddress);
    netif_set_default(netif);
	//netif_set_status_callback(netif, StatusCb);
    DTRACE(0, ("NetIfInit OK!\n"));
}

void pppInit(void);

bool InitTCPIP(void *arg)
{
    /* Sanity check user-configurable values */
    //lwip_sanity_check();
    BYTE bNetIp[13] = {0};
    
    /* Modules initialization */
    stats_init();
  
    //lwIP��ʼ��
    sys_init();
    mem_init();
    memp_init();
    pbuf_init();    
    netif_init();
#if LWIP_SOCKET
    lwip_socket_init();
#endif /* LWIP_SOCKET */
    ip_init();
#if LWIP_ARP
    etharp_init();
#endif /* LWIP_ARP */
#if LWIP_RAW
    raw_init();
#endif /* LWIP_RAW */
#if LWIP_UDP
  udp_init();
#endif /* LWIP_UDP */
#if LWIP_TCP
  tcp_init();
#endif /* LWIP_TCP */
#if LWIP_SNMP
  snmp_init();
#endif /* LWIP_SNMP */
#if LWIP_AUTOIP
  autoip_init();
#endif /* LWIP_AUTOIP */
#if LWIP_IGMP
  igmp_init();
#endif /* LWIP_IGMP */
#if LWIP_DNS
  dns_init();
#endif /* LWIP_DNS */

//#if LWIP_TIMERS
  //sys_timeouts_init();
//#endif /* LWIP_TIMERS */
    
	//memcpy(bNetIp, (BYTE* )arg, sizeof(bNetIp));

    tcpip_init(NULL, NULL); //ֻ��GPRS��û����̫��
    
    //tcpip_init(NetIfInit, (void* )bNetIp); //��̫��
    
   	pppInit();
    
   	return true;
}


void int_tcpip(void* pParam)
{
	//pppOpen(&g_commGPRS, NULL, NULL);
}

#if LWIP_DHCP
DWORD GetDhcpLeaseTime(void)
{
    struct netif *netif = &gNetIf;
    struct dhcp *dhcp = netif->dhcp;
    if (dhcp == NULL)
        return 0;
    
    return dhcp->offered_t0_lease;
}
#endif

void DumpDHCPAddr(void)
{
    /* Dump DHCP addresses */
    if (g_fEnDHCP && gNetIf.dhcp)
    {
        u8_t tmp_state = gNetIf.dhcp->state;
        if (tmp_state != dhcp_state)
        {
            if (tmp_state == DHCP_BOUND && dhcp_state != DHCP_RENEWING)
            {
                BYTE bAddr[4];
                BYTE bBuf[4];
                DTRACE(0,("\n\r"));
                DTRACE(0,("=== DHCP Configurations ===\n\r"));
                //pAddr = (u8_t*)&gNetIf.ip_addr;
                memcpy(bAddr, &gNetIf.ip_addr, 4);
                Swap(bAddr, 4);
                ReadItemEx(10, 0, 0xa151, bBuf);
                if (memcmp(bAddr, bBuf, 4) != 0)
                {
                    WriteItemEx(10, 0, 0xa151, bAddr);        //BN10������Ƭ��FLASH�������龭��дƬ��FLASH
                }
                DTRACE(0,("- IP   : %d.%d.%d.%d\n\r", bAddr[3], bAddr[2], bAddr[1], bAddr[0]));
                
                //pAddr = (u8_t*)&gNetIf.netmask;
                memcpy(bAddr, &gNetIf.netmask, 4);
                Swap(bAddr, 4);
                ReadItemEx(10, 0, 0xa152, bBuf);
                if (memcmp(bAddr, bBuf, 4) != 0)
                {
                    WriteItemEx(10, 0, 0xa152, bAddr);
                }
                DTRACE(0,("- Mask : %d.%d.%d.%d\n\r", bAddr[3], bAddr[2], bAddr[1], bAddr[0]));
                
                //pAddr = (u8_t*)&gNetIf.gw;
                memcpy(bAddr, &gNetIf.gw, 4);
                Swap(bAddr, 4);
                ReadItemEx(10, 0, 0xa153, bBuf);
                if (memcmp(bAddr, bBuf, 4) != 0)
                {
                    WriteItemEx(10, 0, 0xa153, bAddr);
                }
                DTRACE(0,("- GW   : %d.%d.%d.%d\n\r", bAddr[3], bAddr[2], bAddr[1], bAddr[0]));
                DTRACE(0,("===========================\n\r\n"));                               
            }
            dhcp_state = tmp_state;
        }
    }
}

//�����Ƿ���
bool ETLinkUp(void)
{
    return netif_is_up(&gNetIf);    
}

void SetNetIfDefaultET(void)
{
    netif_set_default(&gNetIf); //����Ĭ������Ϊ��̫��
}

void SetNetIfDefaultPPP(void)
{
    sifdefaultroute(0, 0, 0);  //����Ĭ������ΪPPP
}

struct netif *GetEthIf(void)
{
    return &gNetIf;
}

struct ip_addr GetEthGW(void)
{
    return gNetIf.gw;
}

err_t Rethernetif_init( struct netif *pxNetIf )
{
    return ERR_OK;
}

void LoadNetIfPara(BYTE *pbBuf)
{
    BYTE *pbNetIp = pbBuf;
    struct ip_addr ipaddr, netmask, gw;
	struct netif *netif;
    
    if (pbNetIp[0]==0 && pbNetIp[1]==0 && pbNetIp[2]==0 && pbNetIp[3]==0)
		g_fEnDHCP = true;
	if (g_fEnDHCP)
	{
		IP4_ADDR(&gw, 0, 0, 0, 0);
		IP4_ADDR(&ipaddr, 0, 0, 0, 0);
		IP4_ADDR(&netmask, 0, 0, 0, 0);
	}
	else
	{
		IP4_ADDR(&gw, pbNetIp[8], pbNetIp[9], pbNetIp[10], pbNetIp[11]);
		IP4_ADDR(&ipaddr, pbNetIp[0], pbNetIp[1], pbNetIp[2], pbNetIp[3]);
		IP4_ADDR(&netmask, pbNetIp[4], pbNetIp[5], pbNetIp[6], pbNetIp[7]);		
	}
    //ɾ��֮ǰ�Ľӿ�
    netif_remove(&gNetIf);
    
    netif = netif_add(&gNetIf, &ipaddr, &netmask, &gw, NULL, Rethernetif_init, NULL);

    netif_set_default(netif);
}

static void linkStatusCB(void *ctx, int errCode, void *arg)
{    
    if (arg != NULL) //��Ϊ�շ��ص��ǵ�ַ��ppp_addrs������ͨ�������ȡIP�����أ��������룬DNS
    {        
        struct ppp_addrs *pppaddrs = (struct ppp_addrs *)arg;
        //u8_t * pAddr;
        BYTE bAddr[4];
        BYTE bBuf[4];
        //pAddr = (u8_t*)&pppaddrs->our_ipaddr;
        memcpy(bAddr, &pppaddrs->our_ipaddr, 4);
        Swap(bAddr, 4);
        ReadItemEx(10, 0, 0xa151, bBuf);
        if (memcmp(bAddr, bBuf, 4) != 0)
        {
            WriteItemEx(10, 0, 0xa151, bAddr);
        }
        DTRACE(0,("- IP   : %d.%d.%d.%d\n\r", bAddr[3], bAddr[2], bAddr[1], bAddr[0]));
                
        //pAddr = (u8_t*)&pppaddrs->netmask;
        memcpy(bAddr, &pppaddrs->netmask, 4);
        Swap(bAddr, 4);
        ReadItemEx(10, 0, 0xa152, bBuf);
        if (memcmp(bAddr, bBuf, 4) != 0)
        {
            WriteItemEx(10, 0, 0xa152, bAddr);
        }
        DTRACE(0,("- Mask : %d.%d.%d.%d\n\r", bAddr[3], bAddr[2], bAddr[1], bAddr[0]));
                
        //pAddr = (u8_t*)&pppaddrs->his_ipaddr;
        memcpy(bAddr, &pppaddrs->his_ipaddr, 4);
        Swap(bAddr, 4);
        ReadItemEx(10, 0, 0xa153, bBuf);
        if (memcmp(bAddr, bBuf, 4) != 0)
        {
            WriteItemEx(10, 0, 0xa153, bAddr);
        }
        DTRACE(0,("- GW   : %d.%d.%d.%d\n\r", bAddr[3], bAddr[2], bAddr[1], bAddr[0]));
    }
    else //���ӶϿ�
    {
        if ((errCode == PPPERR_CONNECT) //PPPOE���ӱ��Ͽ� //todo�û������Ͽ������͸�λͨ����Ϣ
            /*|| (errCode == PPPERR_PROTOCOL)*/) //�������ò��ɹ���
        {            
            PppoeLinkdown();
        }
        DTRACE(0,("----->linkStatusCB : %d\n\r", errCode));
    }
    return;
}

//����pd
int OpenPppoe(void)
{
    return pppOverEthernetOpen(&gNetIf, NULL, NULL, linkStatusCB, NULL);
}

//����0 �ɹ����������ش������
int ClosePppoe(int pd)
{
    //return pppClose(pd);
    
    pppOverEthernetClose(pd);
    
    return 0;
}
