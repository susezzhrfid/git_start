/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FaCfg.h
 * ժ    Ҫ�����ļ���Ҫ�����궨����Щ�ڸ����ն��в�һ���ĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef FACFG_H
#define FACFG_H
#include "SysCfg.h"

#define FA_NAME		"CL818K5"

//#define PRO_GB2005	1
#define PRO_698		1

//����������ܵı���Ͷ��
#define EN_CCT			1	//�Ƿ�����������
//#define EN_CCT485		1	//�Ƿ�������485��
//#define EN_CTRL			1	//�Ƿ�������ƹ���
//#define EN_VARCPS		1	//�Ƿ������޹�����(VAR compensator)����

#ifndef SYS_WIN
//#define EN_AC			1	//�Ƿ������ɹ���
#endif

//#define AC_DETECT_POWER_OFF    //���ɼ�����

//#define EN_INMTR		1	//�Ƿ������ڲ�DL645
#define EN_ESAM			1	//�Ƿ�����ʹ�ü���ģ��

 #define MAXPNMASK         0x7F

#define MAXDBMASK         (16*8)//20B*8bit

//�߼�����ڶ���
#define PORT_AC			1	//�ն˽�������ͨ�Žӿ�
#define PORT_GB485		2	//���긺��485�����
//#define PORT_CCT_485	3	//������485�����	
#define PORT_IN485		4	//�ڱ�485��
#define PORT_CCT_PLC	31	//�������ز������߳����
#define PORT_CCT_WIRELESS	32  //���������߳���

#define MAX_VIP_METER	10
#define MAX_CCT_METER	32

//����������ص���Ŷ���
#define  DB_CRITICAL	   0	
#define  DB_DB			   1
#define  DB_LOADCTRL       2
#define  DB_FAPROTO        3
#define  DB_FAFRM     	   4	
#define  DB_POINT          5  
#define  DB_FA             6 
#define  DB_TASK           7
#define  DB_645            8
#define  DB_645FRM         9
#define  DB_QUEUE          10
#define  DB_SYS            11
#define  DB_GLOBAL         12
#define  DB_ABB            13
#define  DB_FUJ			   14
#define  DB_EDMI		   15
#define  DB_FS  		   16


#define  DB_VBREAK         17   //��ѹ����
#define  DB_VMISS          18   //��ѹȱ��
#define  DB_POLAR          19   //����������
#define  DB_IOVER          20   //�����������
#define  DB_OVLOAD         21   //���ɹ���
#define  DB_OVDEC          22   //����ͬ�õ�
#define  DB_IUNBAL         23   //���ฺ�ɲ�ƽ��
#define  DB_METER_EXC      24   //����쳣��
#define  DB_OVCPS          25   //�޹�����Ƿ����
#define  DB_DIFF           26   //�
#define  DB_EXC1           27   //�쳣1
#define  DB_CPS            28   //�޹�����
#define  DB_EXC3           29   //�쳣3
#define  DB_EXC4           30   //�쳣4
#define  DB_EXC5           31   //�쳣5
#define  DB_LANDIS         32
#define  DB_OSTAR          33  //����
#define  DB_ZHEJ	       34
#define  DB_DLMS	       35	//��������
#define  DB_HND		       36	//������
#define  DB_WS		       37	//��ʢ��
#define  DB_COMPENSATE     38   //�޹�����
#define  DB_TASKDB         39	//�������ݿ�

#ifdef SYS_LWIP_DEBUG
#define PPP_DEBUG          	40
#define ETHARP_DEBUG       	41
#define NETIF_DEBUG        	42
#define PBUF_DEBUG         	43
#define API_LIB_DEBUG      	44
#define API_MSG_DEBUG      	45
#define SOCKETS_DEBUG      	46
#define ICMP_DEBUG         	47
#define INET_DEBUG         	48
#define IP_DEBUG           	49
#define IP_REASS_DEBUG      50
#define RAW_DEBUG           51
#define MEM_DEBUG           52
#define MEMP_DEBUG          53
#define SYS_DEBUG           54
#define TCP_DEBUG           55
#define TCP_INPUT_DEBUG     56
#define TCP_FR_DEBUG        57
#define TCP_RTO_DEBUG       58
#define TCP_REXMIT_DEBUG    59
#define TCP_CWND_DEBUG      60
#define TCP_WND_DEBUG       61
#define TCP_OUTPUT_DEBUG    62
#define TCP_RST_DEBUG       63
#define TCP_QLEN_DEBUG      64
#define UDP_DEBUG           65
#define TCPIP_DEBUG         66
//#define PPP_DEBUG           67
#define SLIP_DEBUG          68
#define DHCP_DEBUG          69
#endif
/*
#define	 LOG_CRITICAL    80
#define	 LOG_ERR         81
#define	 LOG_NOTICE      82
#define	 LOG_WARNING     83
#define	 LOG_INFO        84
#define	 LOG_DETAIL      85
#define	 LOG_DEBUG       86
*/
#define  PING_DEBUG      87
#define  DRIVER_DEBUG    88

#define DB_METER		90

#define DB_HT3A			91		//��ͨ
#define DB_HL645		91		//��¡
#define DB_AH645		91		//����
#define DB_HB645		91		//����
#define DB_TJ645		91		//���
#define DB_DL645V07		91		//07��645Э��
#define DB_NMG645		91		//���ɹ�645��(��г��)
#define DB_BJ645		91		//����97��645��

#define DB_DP		 	92		//���ݴ���
#define DB_MSCHED	 	93		//������
#define DB_CCT	 		94		//����
#define DB_CCTRXFRM		95		//��������֡
#define DB_CCTTXFRM		96		//��������֡
#define	DB_MULTITEMP    97		//��ͨ���¶ȼ�Э��
#define DB_FAPROTO645	97		//�ڲ�645
#define	DB_MTRX   	 	98		//����߼���Ϣ
#define DB_CCT_EXC		99		//�����¼�
#define DB_COMPENMTR	111
//�������ظ��Ķ���
#define DB_LANDLMS		DB_LANDIS		//������DLMS
#define	DB_1107			DB_LANDIS		//ɽ��A1700
#define DB_LANZMC		DB_LANDIS		//������ZMC��

//�������ظ��Ķ���
#define DB_ABB2		DB_ABB		//ABBԲ��
#define	DB_EMAIL	DB_ABB		//EMAIL���

#define LED_MODE_OFF		0
#define LED_MODE_ON			1
#define LED_MODE_TOGGLE		2
#define LED_MODE_BURST		3

//Ӧ�ò����Ŷ���
typedef enum _LEDs
{
    LED_RUN = 0,//���е�    
	LED_ONLINE,//���ߵ�    
    LED_ALARM,    
    LED_SIGNAL1G,
    LED_SIGNAL2R,
    LED_LOCAL_TX,
    LED_LOCAL_RX,
    LED_REMOTE_TX,
    LED_REMOTE_RX,
	MAX_LED_NUM,
}eLEDs;


#endif //FACFG_H