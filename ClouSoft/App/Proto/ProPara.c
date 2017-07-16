/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProPara.c
 * 摘    要：本文件主要用来把各协议不同的参数装载到相同的参数结构中去,
 *			 如TSocket,TGprs等,使共用的通信代码不用直接面对各种
 *			 协议的差异
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "ProStruct.h"
#include "Pro.h"
#include "DrvCfg.h"
#include "ProIf.h"
//#include "GbPro.h"
#include "Modem.h"
#include "M590.h"
#include "CommIf.h"
#include "GprsIf.h"
#include "DbFmt.h"
#include "ComAPI.h"
#include "DbAPI.h"
#include "SysDebug.h"
#include "SocketIf.h"
#include "ProPara.h"
#include "ctrl.h"

#define FAP_TYPE_OFFSET  		0
#define FAP_IP_OFFSET  			2
#define FAP_DNS_OFFSET  		33
#define FAP_FRONT_PORT_OFFSET  	0
#define FAP_CONCENT_PORT_OFFSET	39
#define FAP_APN_OFFSET  		24
#define FAP_APN_USER_OFFSET  	0
#define FAP_APN_PSW_OFFSET  	16
#define FAP_NETMAST_OFFSET  	89
#define FAP_GATEWAY_OFFSET  	93

void LoadGprsPara(TGprsIf* pGprs)
{
    static bool first = true;
	int iScale;
	BYTE bMode;
	//BYTE bType = 1;
	BYTE bBuf[64];
    WORD wSvrBeatMin;
	bMode = 1;
    
    ReadItemEx(BN1, PN0, 0x2032, bBuf);//TCP/IP协议库：0模块自带,1终端
    if (bBuf[0] == 0)
        pGprs->bCnMode = CN_MODE_EMBED;
    else
        pGprs->bCnMode = CN_MODE_SOCKET;
    
#ifdef SYS_WIN
	pGprs->bCnType = CN_TYPE_ET;
#else
	pGprs->bCnType = CN_TYPE_GPRS;
#endif
    WriteItemEx(BN2, PN0, 0x10d3, &pGprs->bCnType); //本参数用于GPRS，以太网切换,标识当前通道类型
    
    if (first)
    {
        pGprs->ipd = -1;  //重载参数时，不能重新给ipd赋值。
        first = false;
    }
    
    LoadSockPara(pGprs->pSocketIf);
    
    pGprs->dwNoRxRstAppInterv = 0;
    ReadItemEx(BN10, PN0, 0xa142, (BYTE* )&pGprs->dwNoRxRstAppInterv); //无通讯复位终端时间,单位分钟,HEX,默认4天
    if (pGprs->dwNoRxRstAppInterv == 0) //等于0会引起反复复位
        pGprs->dwNoRxRstAppInterv = 5760;
	pGprs->dwNoRxRstAppInterv *= 60;

	//一般参数
	pGprs->bRstToDormanNum = 2;	//复位到休眠的次数:任何情况的失败会导致模块复位，当模块复位次数到达这个次数后，进入休眠状态
	pGprs->bConnectNum = 2;		//连接失败连续尝试的次数
	pGprs->bLoginNum = 2; 		//登录失败连续尝试的次数

	ReadItemEx(BN0, PN0, 0x8017, bBuf);
	if (bBuf[0] == 0) 
		bBuf[0] = 0x15;
	pGprs->bCliBeatMin = BcdToByte(bBuf[0]);
    
	pGprs->wDormanInterv = 0;   //休眠时间间隔, 单位秒, ,0表示禁止休眠模
    
    pGprs->bSignStrength = 0;  //信号强度初始化成0
	
	//模式参数
	if (bMode == 0)	//如果是混合模式，就是按需上报
	{
		DTRACE(DB_FAPROTO, ("LoadGprsPara:: GPRS is in mix-mode\r\n"));
		pGprs->bOnlineMode = ONLINE_M_MIX;
		pGprs->fEnableAutoSendActive = true; 	//允许主动上报激活
		pGprs->wActiveDropSec = 60; 	 //非连续在线模式的自动掉线时间,单位秒

   		ReadItemEx(BN24, PN0, 0x4108, bBuf);
		iScale = Fmt6ToVal(bBuf, 2);
		wSvrBeatMin = iScale * pGprs->bCliBeatMin / 100;	//Fmt6 format as SNN.NN
        pGprs->bSvrBeatMin = (wSvrBeatMin > 255)? 255:wSvrBeatMin;
		return;
	}
	else //if (bType == 1)	//永久在线
	{
		DTRACE(DB_FAPROTO, ("LoadGprsPara:: GPRS is in persisit-mode\r\n"));

		pGprs->bOnlineMode = ONLINE_M_PERSIST;
		pGprs->fEnableAutoSendActive = false; 	//允许主动上报激活
		pGprs->wActiveDropSec = 0; 	 //非连续在线模式的自动掉线时间,单位秒
	}
}

void LoadSockPara(TSocketIf* pSocket)
{
    BYTE bBuf[1];
	//参数
	pSocket->fSvr = false;	//是否是服务器模式
	//pSocket->fUdp = false;	//是否是UDP通信方式

	ReadItemEx(BN0, PN0, 0x801a, bBuf);
	if (bBuf[0] == 1)
		pSocket->fUdp = true;
	else
		pSocket->fUdp = false;
    
	pSocket->fEnableFluxStat = true;	//是否允许流量控制,只有本socket用的是GPRS通道时才支持	
	pSocket->bCnType = CN_TYPE_GPRS;
	pSocket->fBakIP = false;
	pSocket->wIPUseCnt = 0;
	//pSocket->wConnectFailCnt = 0;
	pSocket->wConnectNum = 2;
}

bool GetMasterIp(TMasterIp* pMasterIp)
{
	BYTE bBuf[32];

	memset(pMasterIp, 0, sizeof(TMasterIp));
	ReadItemEx(BN0, PN0, 0x801a, bBuf);
	if (bBuf[0] == 1)
		pMasterIp->fUdp = true;
	else
		pMasterIp->fUdp = false;

	//ReadItemEx(BN0, PN0, 0x003f, bBuf);
	ReadItemEx(BN0, PN0, 0x8010, bBuf);

#ifdef SYS_WIN
    bBuf[2] = 1;	//IP地址
	bBuf[3] = 0;
	bBuf[4] = 0;
	bBuf[5] = 127;

	WordToByte(9200, bBuf);	//端口号
#endif

	pMasterIp->dwRemoteIP = ((DWORD )bBuf[FAP_IP_OFFSET]) + ((DWORD )bBuf[FAP_IP_OFFSET+1]<<8) + 
							((DWORD )bBuf[FAP_IP_OFFSET+2]<<16) + (bBuf[FAP_IP_OFFSET+3]<<24);

	pMasterIp->wRemotePort = ByteToWord(&bBuf[FAP_FRONT_PORT_OFFSET]);
	//pPara->dwLocalIP = 0; //INADDR_ANY;
	//pPara->wLocalPort = 8000;

	ReadItemEx(BN0, PN0, 0x8011, bBuf);
	if (bBuf[8] == 0x02 || bBuf[8] == 0x04)//备用通道
	{
		pMasterIp->dwBakIP = ((DWORD )bBuf[FAP_IP_OFFSET]) + ((DWORD )bBuf[FAP_IP_OFFSET+1]<<8) + 
			((DWORD )bBuf[FAP_IP_OFFSET+2]<<16) + (bBuf[FAP_IP_OFFSET+3]<<24);

		pMasterIp->wBakPort = ByteToWord(&bBuf[FAP_FRONT_PORT_OFFSET]);
	}
	else
	{
		pMasterIp->dwBakIP = 0;
		pMasterIp->wBakPort = 0;
	}

	if (pMasterIp->dwBakIP == 0 || bBuf[8] == 0xff)
	{
		ReadItemEx(BN0, PN0, 0x8012, bBuf);
		if (bBuf[8] == 0x02 || bBuf[8] == 0x04)//备用通道
		{
			pMasterIp->dwBakIP = ((DWORD )bBuf[FAP_IP_OFFSET]) + ((DWORD )bBuf[FAP_IP_OFFSET+1]<<8) + 
				((DWORD )bBuf[FAP_IP_OFFSET+2]<<16) + (bBuf[FAP_IP_OFFSET+3]<<24);

			pMasterIp->wBakPort = ByteToWord(&bBuf[FAP_FRONT_PORT_OFFSET]);
		}
		else
		{
			pMasterIp->dwBakIP = 0;
			pMasterIp->wBakPort = 0;
		}
	}

//	pPara->dwBakIP = 0;
//	pPara->wBakPort = 0;
	/*DTRACE(DB_FAPROTO, ("SocketLoadPara:remote ip=%d.%d.%d.%d port=%d, local port=%d.\n",
						(pPara->dwRemoteIP>>24)&0xff, (pPara->dwRemoteIP>>16)&0xff, (pPara->dwRemoteIP>>8)&0xff, pPara->dwRemoteIP&0xff, 
						pPara->wRemotePort, pPara->wLocalPort)); */
	return true;
}

//描述：取得终端自身服务器参数
bool GetSvrPara(TSvrPara* pSvrPara)
{
	
    
	pSvrPara->fUdp = false;
	
	pSvrPara->wLocalPort = 5100;

    DTRACE(DB_FAPROTO, ("GetSvrPara:: Svr wLocalPort=%d.\r\n", pSvrPara->wLocalPort));
//	pSvrPara->wLocalPort = 8000;		//绑定的本地端口号
	return true;
}

bool GetApn(char* pszApn)
{
	BYTE bBuf[16];
	ReadItemEx(BN0, PN0, 0x8014, bBuf);	
	if(IsAllAByte(bBuf,0x00,16))
	{
		strcpy(pszApn, "CMNET");
	}
	else
	{
		memcpy(pszApn,bBuf,16);
	}
	pszApn[16] = 0;
	return true;
}

bool GetUserAndPsw(char *psUser, char *psPsw)
{
	BYTE bBuf[32];
	ReadItemEx(BN0, PN0, 0x8015, bBuf);
	memcpy(psUser,bBuf,32);

	psUser[32] = 0;
	ReadItemEx(BN0, PN0, 0x8016, bBuf);
	memcpy(psPsw,bBuf,32);
	psPsw[32] = 0;
		
	if (psUser[0]=='\0' && psPsw[0]=='\0')
	{
		strcpy(psUser, "CARD");
		strcpy(psPsw, "CARD");
	}   
    return true;
}

//本地232维护口初始化
void LoadLocal232Para(TCommIf* pCommIf)
{
	pCommIf->wPort = COMM_TEST;
	pCommIf->dwBaudRate = CBR_9600;
	pCommIf->bByteSize = 8;
	pCommIf->bStopBits = ONESTOPBIT;
	pCommIf->bParity = EVENPARITY;
	pCommIf->fDebug = true;
}

//本地红外维护口初始化
void LoadLocalIrPara(TCommIf* pCommIf)
{
	pCommIf->wPort = COMM_LOCAL;
	pCommIf->dwBaudRate = CBR_1200;
	pCommIf->bByteSize = 8;
	pCommIf->bStopBits = ONESTOPBIT;
	pCommIf->bParity = EVENPARITY;
	pCommIf->fDebug = false;
}
