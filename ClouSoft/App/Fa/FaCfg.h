/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FaCfg.h
 * 摘    要：本文件主要用来宏定义那些在各种终端中不一样的常量
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef FACFG_H
#define FACFG_H
#include "SysCfg.h"

#define FA_NAME		"CL818K5"

//#define PRO_GB2005	1
#define PRO_698		1

//部分软件功能的编译投退
#define EN_CCT			1	//是否允许集抄功能
//#define EN_CCT485		1	//是否允许集抄485口
//#define EN_CTRL			1	//是否允许控制功能
//#define EN_VARCPS		1	//是否允许无功补偿(VAR compensator)功能

#ifndef SYS_WIN
//#define EN_AC			1	//是否允许交采功能
#endif

//#define AC_DETECT_POWER_OFF    //交采检测掉电

//#define EN_INMTR		1	//是否允许内部DL645
#define EN_ESAM			1	//是否允许使用加密模块

 #define MAXPNMASK         0x7F

#define MAXDBMASK         (16*8)//20B*8bit

//逻辑抄表口定义
#define PORT_AC			1	//终端交流采样通信接口
#define PORT_GB485		2	//国标负控485抄表口
//#define PORT_CCT_485	3	//集中器485抄表口	
#define PORT_IN485		4	//内表485口
#define PORT_CCT_PLC	31	//集中器载波或无线抄表口
#define PORT_CCT_WIRELESS	32  //集中器无线抄表

#define MAX_VIP_METER	10
#define MAX_CCT_METER	32

//调试输出开关的序号定义
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


#define  DB_VBREAK         17   //电压断相
#define  DB_VMISS          18   //电压缺相
#define  DB_POLAR          19   //电流反极性
#define  DB_IOVER          20   //相电流过负荷
#define  DB_OVLOAD         21   //负荷过载
#define  DB_OVDEC          22   //超合同用电
#define  DB_IUNBAL         23   //三相负荷不平衡
#define  DB_METER_EXC      24   //电表异常类
#define  DB_OVCPS          25   //无功过补欠补类
#define  DB_DIFF           26   //差动
#define  DB_EXC1           27   //异常1
#define  DB_CPS            28   //无功补偿
#define  DB_EXC3           29   //异常3
#define  DB_EXC4           30   //异常4
#define  DB_EXC5           31   //异常5
#define  DB_LANDIS         32
#define  DB_OSTAR          33  //蜀达表
#define  DB_ZHEJ	       34
#define  DB_DLMS	       35	//爱拓利表
#define  DB_HND		       36	//浩宁达
#define  DB_WS		       37	//威盛表
#define  DB_COMPENSATE     38   //无功补偿
#define  DB_TASKDB         39	//任务数据库

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

#define DB_HT3A			91		//恒通
#define DB_HL645		91		//华隆
#define DB_AH645		91		//安徽
#define DB_HB645		91		//湖北
#define DB_TJ645		91		//天津
#define DB_DL645V07		91		//07版645协议
#define DB_NMG645		91		//内蒙古645表(带谐波)
#define DB_BJ645		91		//北京97版645表

#define DB_DP		 	92		//数据处理
#define DB_MSCHED	 	93		//电表调度
#define DB_CCT	 		94		//集抄
#define DB_CCTRXFRM		95		//集抄接收帧
#define DB_CCTTXFRM		96		//集抄发送帧
#define	DB_MULTITEMP    97		//多通道温度计协议
#define DB_FAPROTO645	97		//内部645
#define	DB_MTRX   	 	98		//抄表高级信息
#define DB_CCT_EXC		99		//集抄事件
#define DB_COMPENMTR	111
//跟上面重复的定义
#define DB_LANDLMS		DB_LANDIS		//兰吉尔DLMS
#define	DB_1107			DB_LANDIS		//山东A1700
#define DB_LANZMC		DB_LANDIS		//兰吉尔ZMC表

//跟上面重复的定义
#define DB_ABB2		DB_ABB		//ABB圆表
#define	DB_EMAIL	DB_ABB		//EMAIL电表

#define LED_MODE_OFF		0
#define LED_MODE_ON			1
#define LED_MODE_TOGGLE		2
#define LED_MODE_BURST		3

//应用层灯序号定义
typedef enum _LEDs
{
    LED_RUN = 0,//运行灯    
	LED_ONLINE,//在线灯    
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