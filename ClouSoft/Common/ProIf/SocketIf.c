/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：SocketIf.c
 * 摘    要：本文件实现了socket通信接口类,只有WINDOWS平台支持本接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#include "FaCfg.h"
//#ifdef SYS_WIN
#include <stdio.h>
#include "SysArch.h" 
#include "SocketIf.h"
#include "Trace.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "SysApi.h"
#include "SysDebug.h"
#include "ProStruct.h"
#include "ProPara.h"
#include "sockets.h"
#include "ctrl.h"
#include "SysCfg.h"
#include "FaAPI.h"

typedef u_long ULONG;
#define INVALID_SOCKET   -1
#define SOCKET_ERROR   -1

//#define EWOULDBLOCK WSAEWOULDBLOCK
#define EWOULDBLOCK    11

#ifdef SYS_WIN
int SocketGetLastError(int s)
{
	return WSAGetLastError();
}

void SocketSetLastError(int s, int iError)
{
	WSASetLastError(iError);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//SocketIf私有宏定义

//客户端状态机
#define SK_STATE_IDLE		0   //空闲:有主动上报需求->连接状态
#define SK_STATE_CONNECT 	1	//连接:连接成功->登陆状态
#define SK_STATE_LOGIN		2   //登录:登录成功->传输状态
#define SK_STATE_TRANS		3   //传输:连续无通信规定时间->空闲状态

////////////////////////////////////////////////////////////////////////////////////////////
//SocketIf私有成员变量
//因为可能会产生多个Socket连接，所以应尽量避免定义全局变量，可以把成员变量定义到TSocketIf
/////////////////////////////////////////////////////////////////////////////////////////////////////
//SocketIf实现

bool SocketIfInit(TSocketIf* pSocketIf)
{
/*	//基类结构成员初始化
	ProIfInit(pProIf);

	//类派生
	pProIf->pszName = "SocketIf";				//接口名称
	pProIf->wMaxFrmBytes = SOCK_MAX_BYTES;		//最大帧长度
	pProIf->bIfType = IF_SOCKET;				//接口类型

	//虚函数，需要实例化为具体接口的对应函数
	pProIf->pfnSend = SocketIfSend;				//发送函数
	pProIf->pfnReceive = SocketIfReceive;		//接收函数
	pProIf->pfnTrans = SocketIfTrans;			//传输状态函数*/
    
	//派生类具体初始化
	pSocketIf->iSocket = INVALID_SOCKET;
	//pProIf->bState = IF_STATE_TRANS;
	//pSocketIf->bSubState = SK_STATE_CONNECT;

	return true;
}

//描述:初始化服务器
void SocketIfInitSvr(TSocketIf* pSocketIf, int socket)
{	
	pSocketIf->iSocket = socket;
	//pProIf->bState = IF_STATE_TRANS;
	//pSocketIf->bSubState = SK_STATE_TRANS;

	//pProIf->fExit = false;
	//pProIf->fExitDone = false;
}

bool SocketIfSend(TSocketIf* pSocketIf, BYTE* pbTxBuf, WORD wLen)
{
	int tolen;
	TMasterIp tMasterIp;	
	DTRACE(DB_FAPROTO, ("CSocketIf::Send: GetClick()=%d,wLen=%d.\r\n",GetClick(),wLen));
#ifdef SYS_WIN
	TraceFrm("<-- CSocketIf::Send:", pbTxBuf, wLen);	
#endif
	if (pSocketIf->iSocket != INVALID_SOCKET)
	{
		int iReLen = 0;
		if (pSocketIf->fUdp)
		{
			struct  sockaddr_in to;

			GetMasterIp(&tMasterIp);
            if (!pSocketIf->fBakIP)  //主IP
            {
			    to.sin_addr.s_addr = htonl(tMasterIp.dwRemoteIP);
    			to.sin_family = AF_INET;
    			to.sin_port = htons(tMasterIp.wRemotePort);	
            }
            else  //从IP
            {
                to.sin_addr.s_addr = htonl(tMasterIp.dwBakIP);
    			to.sin_family = AF_INET;
    			to.sin_port = htons(tMasterIp.wBakPort);	
            }
			tolen=sizeof(to);				
			iReLen = sendto(pSocketIf->iSocket, (char* )pbTxBuf, wLen, 0,(struct sockaddr *)&to,tolen);
		}
		else
		{
			iReLen = send(pSocketIf->iSocket, (char* )pbTxBuf, wLen, 0);
		}

		DoLedBurst(LED_REMOTE_TX);
		if (pSocketIf->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
			AddFlux(wLen);

		if (iReLen == wLen)
		{
			return true;
		}
		else
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail iReLen=%d, wLen=%d.\r\n", iReLen, wLen));
			return false;
		}
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail due to invalid sock.\r\n"));
		return false;
	}
}

//描述:接收串口来的数据,如果接收循环缓冲区中还有数据,则返回循环缓冲区中的数据,
//     否则调用串口接收函数,直接等待串口的数据到来
//参数:@pbRxBuf 用来接收返回数据的缓冲区,
//     @wBufSize 接收缓冲区的大小
//返回:返回数据的长度
int SocketIfReceive(TSocketIf* pSocketIf, BYTE* pbRxBuf, WORD wBufSize)
{	
	int len = 0, i=0; 
	struct  sockaddr_in from; 
	i = sizeof(from);

	if (pSocketIf->iSocket != INVALID_SOCKET)
	{
		//Sleep(10);   //todo:影响下载升级速度
  		//SockSetLastError(pSocketIf->iSocket, 0);
  		if (pSocketIf->fUdp == 0x01)
			len =  recvfrom(pSocketIf->iSocket, (char*)pbRxBuf, wBufSize, 0,(struct sockaddr *)&from, &i);	//(socklen_t* )
		else
			len =  recv(pSocketIf->iSocket, (char *)pbRxBuf, wBufSize, 0);

	  	if (len == 0)//对方发了复位链路指令过来，移动网络经常有这个问题
      	{
            Sleep(100);
  	    	DTRACE(DB_FAPROTO, ("CSocketIf::Receive: close socket due to rx len=0.\r\n"));
	    	SocketIfDisConnect(pSocketIf);            
	    	return -1;
      	}
      	else if (len == SOCKET_ERROR) 
	  	{
            Sleep(100);
#ifndef SYS_WIN
			int iLastErr =  SocketGetLastError(pSocketIf->iSocket);

	  		//DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(errno)));
	  		//return 0;
			if (iLastErr == EWOULDBLOCK)   //TCP在socket层的超时，应用程序不应执行重传
        	{               //应该坚信只要通信渠道还能传递数据，TCP就会把交付的数据给对方
				SocketSetLastError(pSocketIf->iSocket, 0);
          		return 0;
			}
			else if (iLastErr == 0)
			{
				DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(iLastErr)));
				return 0;
			}
			else
			{
	  			DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%ld, %s\n", iLastErr, strerror(iLastErr)));
	    		SocketIfDisConnect(pSocketIf);
				SocketSetLastError(pSocketIf->iSocket, 0);
        		return -1;
			}
#else
			return 0;
#endif
      	}
      	else
      	{
            Sleep(50);   //太频繁，响影其它线程
		  	DoLedBurst(LED_REMOTE_RX);
			if (pSocketIf->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
				AddFlux((DWORD )len);

      		DTRACE(DB_FAPROTO, ("CSocketIf::Receive:GetClick()=%d.\r\n",GetClick()));
	#ifdef SYS_WIN
      		TraceFrm("--> CSocketIf::Receive:", pbRxBuf, len);
	#endif
      	}
	}
	else
	{
		Sleep(200);
        return -1;
	}

	return len; 
}

//描述:取连接次数,GPRS或socket连接的时候,如果有备用IP端口的话,连接次数乘2
WORD SocketIfGetConnectNum(TSocketIf* pSocketIf)
{
	//TMasterIp tMasterIp;
	WORD wConnectNum = pSocketIf->wConnectNum;

	//GetMasterIp(&tMasterIp);
	
	//if (tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)
		//wConnectNum *= 2;

	return wConnectNum;
}

//描述：建立连接
bool SocketIfConnect(TSocketIf* pSocketIf)
{
	DWORD dwRemoteIP;
	WORD wRemotePort;
	TMasterIp tMasterIp;
	struct  sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	unsigned int arg;
    DWORD dwPort = 1024;

	SocketIfDisConnect(pSocketIf);
	
	if (pSocketIf->fUdp == 0x01)
	{
		pSocketIf->iSocket = socket(PF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		pSocketIf->iSocket = socket(PF_INET, SOCK_STREAM, 0);
	}
		
	if (pSocketIf->iSocket == INVALID_SOCKET)
	{
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: failed to create socket.\r\n"));
		pSocketIf->cLastErr = GPRS_ERR_CON;
	  	return false;
	}

	//if (pSocketIf->fUdp)  
	{
		local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		local_addr.sin_family = AF_INET;
        if (pSocketIf->fUdp)
    		local_addr.sin_port = htons(1024); //UDP挷定固定的端口
        else
        {
			//local_addr.sin_port = htons(0);    //TCP挷定0时，TCP协议会随机选择一个端口挷定，有利于通过防火墙
		#ifdef SYS_WIN
			srand(GetTick()); //产生随机数种子
			dwPort = rand();
		#else
			GetRandom(&dwPort);
		#endif
			dwPort = dwPort%(9200-4097) + 4097;
            local_addr.sin_port = htons(dwPort);
            /*if ((g_PowerOffTmp.wLocalPort < 4097) || (g_PowerOffTmp.wLocalPort > 9200))
            {
                g_PowerOffTmp.wLocalPort = 4097;
            }
            local_addr.sin_port = htons(g_PowerOffTmp.wLocalPort++);  */
        }
	
		if (bind(pSocketIf->iSocket, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Connect: Error: bind error\n"));
		  	SocketIfClose(pSocketIf);
			pSocketIf->cLastErr = GPRS_ERR_CON;
		  	return false;
		}
	}
	
	GetMasterIp(&tMasterIp);

    //主备IP切换：
	//1、在没有成功登陆主站的情况下，轮流使用主备IP
	//2、在登陆OK的情况下，清当前IP的使用计数，让它可以继续使用，
	if (pSocketIf->fBakIP	//当前要使用备用地址
		&& tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)//备用IP端口有效
	{		//备用IP端口有效
		pSocketIf->wIPUseCnt++;
        if (pSocketIf->wIPUseCnt > pSocketIf->wConnectNum)
        {
			dwRemoteIP = tMasterIp.dwRemoteIP;
			wRemotePort = tMasterIp.wRemotePort;        	
            pSocketIf->wIPUseCnt = 1;   //本次已经使用过一次了
            pSocketIf->fBakIP = false;  //本回切换到主IP
        }
        else
        {
        	dwRemoteIP = tMasterIp.dwBakIP;
			wRemotePort = tMasterIp.wBakPort;
	    }
	}
	else
	{		
        pSocketIf->wIPUseCnt++;
        if (pSocketIf->wIPUseCnt>pSocketIf->wConnectNum &&
        	tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)
        {
            pSocketIf->wIPUseCnt = 1; //本次已经使用过一次了
           	pSocketIf->fBakIP = true; //本切换到备用IP
        	dwRemoteIP = tMasterIp.dwBakIP;
			wRemotePort = tMasterIp.wBakPort;
        }
        else
        {
        	pSocketIf->fBakIP = false;  //本回用主IP
			dwRemoteIP = tMasterIp.dwRemoteIP;
			wRemotePort = tMasterIp.wRemotePort;
        }
	}

	remote_addr.sin_addr.s_addr = htonl(dwRemoteIP);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(wRemotePort);
	
	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connecting %d.%d.%d.%d %d, local port %d\r\n",
						 (dwRemoteIP>>24)&0xff, (dwRemoteIP>>16)&0xff, (dwRemoteIP>>8)&0xff, dwRemoteIP&0xff, 
						 wRemotePort, dwPort));
	
	if (pSocketIf->fUdp == false)
	{
		if (connect(pSocketIf->iSocket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) != 0)
    	{
    		DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect fail.\r\n"));
		  	SocketIfClose(pSocketIf);
	  		//UpdateErrRst(false);
			pSocketIf->cLastErr = GPRS_ERR_CON;
		  	return false;
    	}
	}
	
	//UpdateErrRst(true);
    DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect ok.\r\n"));
//	pProIf->dwRxClick = GetClick();//防止连上之后还没有通信，没有更新该值，之后的keepalive中断开

	//int timeout = 2000;
	//if (setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_RCVTIMEO, (char* )&timeout, sizeof(timeout)) != 0)
    //int optval = 1;
	//if (setsockopt(pSocketIf->iSocket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) != 0) 
#ifdef MY_TCP_KEEPALIVE     //todo:TCP保活探测  送检
    int keepAlive = SOF_KEEPALIVE; // 开启keepalive属性
    setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));//设置TCP层保活探测
    int keepIdle = 60000; // 如该连接在60秒内没有任何数据往来,则进行探测 
    setsockopt(pSocketIf->iSocket, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&keepIdle, sizeof(keepIdle));
#endif    
	arg = 1;
	if (ioctlsocket(pSocketIf->iSocket, FIONBIO,  (ULONG* )&arg) != 0) //(u_long FAR*)//设置为非阻塞
    {
    	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: ioctl fail.\r\n"));
	  	SocketIfClose(pSocketIf);
		pSocketIf->cLastErr = GPRS_ERR_CON;
	  	return false;
    }

	pSocketIf->cLastErr = GPRS_ERR_OK;
    if (pSocketIf->bCnType == CN_TYPE_GPRS)
        SetSocketLed(true);
  	return true;
}

void SocketIfOnConnectFail(TSocketIf* pSocketIf)
{	
	//TMasterIp tMasterIp;

	//GetMasterIp(&tMasterIp);

	//连接相关
	/*pSocketIf->wConnectFailCnt++;
	if (pSocketIf->wConnectFailCnt > SocketIfGetConnectNum(pSocketIf))
	{
		pSocketIf->wConnectFailCnt = 0;
	}
	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: fail cnt = %d.\r\n", pSocketIf->wConnectFailCnt));*/
}

void SocketIfOnConnectOK(TSocketIf* pSocketIf)
{
	//pSocketIf->wConnectFailCnt = 0;
}

void SocketIfOnLoginOK(TSocketIf* pSocketIf)
{
    pSocketIf->wIPUseCnt = 0;
}

void SocketIfEnterDorman(TSocketIf* pSocketIf)
{
	//ShiftIP(pSocketIf);
	//DTRACE(DB_FAPROTO, ("CSocketIf::EnterDorman\n"));
}

bool SocketIfClose(TSocketIf* pSocketIf)
{
	if (pSocketIf->iSocket != INVALID_SOCKET)
	{
#ifndef SYS_WIN
		unsigned int arg = 0;
		ioctlsocket(pSocketIf->iSocket, FIONBIO,  (ULONG* )&arg);//设置为阻塞		
		arg = 1;
		setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int));
		//要已经处于连接状态的soket在调用closesocket后强制关闭，省掉CLOSE_WAIT的过程：
		struct linger so_linger;
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

		close(pSocketIf->iSocket);
#else
		closesocket(pSocketIf->iSocket);
#endif	  	
        
	  	pSocketIf->iSocket = INVALID_SOCKET;
//	  	DTRACE(DB_FAPROTO, ("CSocketIf::Close: close socket at click %d\n", GetClick()));
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Close: close socket !!\r\n"));
	}
	
	return true;
}

//描述:在接口由连接转为断开的时候调用，不管是主动断开还是被动断开
bool SocketIfDisConnect(TSocketIf* pSocketIf)
{	
	if (pSocketIf->iSocket != INVALID_SOCKET) //断开socket连接
	{
        if (pSocketIf->bCnType == CN_TYPE_GPRS)
            SetSocketLed(false);
		SocketIfClose(pSocketIf);
	  	//pSocketIf->iSocket = INVALID_SOCKET;
	}
	
/*	if (pSocketIf->fSvr) //服务器模式下,socket断开时退出线程
		pProIf->fExit = true;
	else if (pSocketIf->bSubState > SK_STATE_CONNECT)	//客户端
		pSocketIf->bSubState = SK_STATE_CONNECT;*/
	
	return true;
}

//#endif 	//SYS_WIN

bool SocketIfListen(TSocketIf* pSocketIf)
{
    return true;
}

bool SocketIfCloseListen(TSocketIf* pSocketIf)
{
    return true;
}

int ATCmd(char* pszCmd, char* pszAnsOK, char* pszAnsErr, WORD nWaitSeconds)
{           
    BYTE bBuf[128];
	WORD i = 0;
	WORD wLen;
    WORD wAllLen = 0;
    
	DTRACE(DB_FAPROTO, ("ATCmd : tx %s\r\n", pszCmd));

	CommRead(COMM_GPRS, NULL, 0, 10);    //发AT命令前，先清串口缓冲区
	wLen = strlen(pszCmd);
    if (CommWrite(COMM_GPRS, (BYTE *)pszCmd, wLen, 1000) != wLen)
		return -2;
    
	do
	{
		if (i!=0 && i!=1)   //对于nWaitSeconds==0的情况,Read()两回,不Sleep()
			Sleep(1000);
        
		wLen = CommRead(COMM_GPRS, bBuf+wAllLen, sizeof(bBuf)-wAllLen, 1000);
        wAllLen += wLen;
        if (wAllLen >= sizeof(bBuf))
            wAllLen = sizeof(bBuf)-1;
		bBuf[wAllLen] = 0;
        
		//2.正常的AT命令处理
		if (strstr((char* )bBuf, pszAnsOK) != NULL)   //接收到正确回答
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
    		return wLen;  
		}

		if (pszAnsErr!=NULL && strstr((char* )bBuf, pszAnsErr)!=NULL)  //接收到错误回答1
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -1;
		}
		
        if (wAllLen == sizeof(bBuf)-1) //Buf 满
            wAllLen = 0;  //将之前收到的扔掉

		i++;
		if (i == 1)    //对于nWaitSeconds==0的情况,至少还得再来一次
		{
			continue;
		}
		else if (i >= nWaitSeconds)
		{
			break;
		}
	} while(1);

	if (wAllLen != 0)
	{
		bBuf[wAllLen] = 0;
		DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
	}
	else
	{
		DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx no answer.\r\n", bBuf));
	}

	return 0;
}

bool SetSocketLed(bool fLight)
{
	char* p;	
	bool fRet = false;
	BYTE i;

	//发灯的控制命令	
	char* pszLight = "AT$MYSOCKETLED=1\r\n";
	char* pszDark = "AT$MYSOCKETLED=0\r\n";
    
    Sleep(2000);   //这里必须加延时，因为有可能TCPIP还在通，如果切换模式可能导致通信故障

	//退出数据模式
    for ( i=0; i<5; i++)
    {
        if (ATCmd("+++", "OK", NULL, 1) > 0)//Note:+++后不能加任何符号,有数据通信时本命令会失败
            break; 
    }
    if (i >= 5)  
        return fRet;    
    
	if (fLight)
		p = pszLight;
	else
		p = pszDark;

    if (ATCmd(p, "OK", "ERROR", 0) > 0)
        fRet = true;
			
    //返回数据模式    
    ATCmd("ATO\r\n", "CONNECT", NULL, 0);
	
	return fRet;
}

