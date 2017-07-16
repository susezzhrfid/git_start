/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Socket.c
 * 摘    要：本文件实现了socket通信,支持GPRS终端协议栈
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#include "FaCfg.h"

#include <stdio.h>
#include "SysArch.h" 
#include "Socket.h"
#include "Trace.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "SysApi.h"
#include "SysDebug.h"
#include "ProStruct.h"
#include "ProPara.h"
#ifndef SYS_WIN
#include "sockets.h"
#endif


#define SOCKET_ERROR   -1

#define EWOULDBLOCK    11  /* Try again */   //in "arch.h"

//#define EWOULDBLOCK WSAEWOULDBLOCK

#ifndef SYS_WIN
extern unsigned long htonl(unsigned long n);
extern unsigned short htons(unsigned short n);
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//Socket私有宏定义

//客户端状态机
#define SK_STATE_IDLE		0   //空闲:有主动上报需求->连接状态
#define SK_STATE_CONNECT 	1	//连接:连接成功->登陆状态
#define SK_STATE_LOGIN		2   //登录:登录成功->传输状态
#define SK_STATE_TRANS		3   //传输:连续无通信规定时间->空闲状态

#define BEAT_TEST_TIMES 	2	//心跳测试次数,   为0,表示不自动掉线,只周期发心跳
#define BEAT_TEST_TO		30	//心跳超时时间,单位秒

////////////////////////////////////////////////////////////////////////////////////////////
//Socket私有成员变量
//因为可能会产生多个Socket连接，所以应尽量避免定义全局变量，可以把成员变量定义到TSocket

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Socket实现

TSocket g_tSock;

bool SocketInit(TSocket *pSocket)
{	
	//派生类具体初始化
	pSocket->iSocket = INVALID_SOCKET;
	pSocket->bSubState = SK_STATE_CONNECT;
    
    pSocket->fUdp = 0;         //TCP模式
	pSocket->fEnableFluxStat = true;	//是否允许流量控制,只有本socket用的是GPRS通道时才支持
	return true;
}

//描述:初始化服务器
void SocketInitSvr(TSocket *pSocket, int socket)
{
	pSocket->iSocket = socket;	
	pSocket->bSubState = SK_STATE_TRANS;	
}

bool SocketSend(TSocket *pSocket, BYTE* pbTxBuf, WORD wLen)
{
	int tolen;
	TMasterIp tMasterIp;	
	DTRACE(DB_FAPROTO, ("CSocket::Send: GetClick()=%d,wLen=%d.\r\n",GetClick(),wLen));
	//TraceFrm("<-- CSocket::Send:", pbTxBuf, wLen);	
	if (pSocket->iSocket != INVALID_SOCKET)
	{
		int iReLen = 0;
		if (pSocket->fUdp)
		{
			struct  sockaddr_in to;

			GetMasterIp(&tMasterIp);
			to.sin_addr.s_addr = htonl(tMasterIp.dwRemoteIP);
			to.sin_family = AF_INET;
			to.sin_port = htons(tMasterIp.wRemotePort);	
			tolen=sizeof(to);				
			iReLen = sendto(pSocket->iSocket, (char* )pbTxBuf, wLen, 0,(struct sockaddr *)&to,tolen);
		}
		else
		{
			iReLen = send(pSocket->iSocket, (char* )pbTxBuf, wLen, 0);
		}

		DoLedBurst(LED_REMOTE_TX);
		if (pSocket->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
			AddFlux(wLen);

		if (iReLen == wLen)
		{
			return true;
		}
		else
		{
			DTRACE(DB_FAPROTO, ("CSocket::Send : sock tx fail iReLen=%d, wLen=%d.\r\n", iReLen, wLen));
			return false;
		}
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CSocket::Send : sock tx fail due to invalid sock.\r\n"));
		return false;
	}
}

//描述:接收串口来的数据,如果接收循环缓冲区中还有数据,则返回循环缓冲区中的数据,
//     否则调用串口接收函数,直接等待串口的数据到来
//参数:@pbRxBuf 用来接收返回数据的缓冲区,
//     @wBufSize 接收缓冲区的大小
//返回:返回数据的长度,负数为错误 -1：socket正常断开
int SocketReceive(TSocket *pSocket, BYTE* pbRxBuf, WORD wBufSize)
{
	int len = 0, i=0; 
	struct  sockaddr_in from; 
	i = sizeof(from);

	if (pSocket->iSocket != INVALID_SOCKET)
	{
		//Sleep(100);//10
  		//SockSetLastError(pSocket->iSocket, 0);
  		if (pSocket->fUdp == 0x01)
			len =  recvfrom(pSocket->iSocket, (char*)pbRxBuf, wBufSize, 0,(struct sockaddr *)&from, &i);	//(socklen_t* )
		else
			len =  recv(pSocket->iSocket, (char *)pbRxBuf, wBufSize, 0);

	  	if (len == 0)
      	{
            Sleep(100);
  	    	DTRACE(DB_FAPROTO, ("CSocket::Receive: close socket due to rx len=0.\r\n"));//TCP链接时服务器关闭。
	    	SocketDisConnect(pSocket);
            pSocket->iSocket = INVALID_SOCKET;
	    	return -1;
      	}
      	else if (len == SOCKET_ERROR) 
	  	{
            Sleep(100);
#ifndef SYS_WIN
			int iLastErr =  SocketGetLastError(pSocket->iSocket);

	  		//DTRACE(DB_FAPROTO, ("CSocket::Receive: len==-1 errno=%s\n", strerror(errno)));
	  		//return 0;
			if (iLastErr == EWOULDBLOCK)   //TCP在socket层的超时，应用程序不应执行重传           //todo,注意是11还是-11
        	{               //应该坚信只要通信渠道还能传递数据，TCP就会把交付的数据给对方
				SocketSetLastError(pSocket->iSocket, 0);
          		return 0;
			}
			else if (iLastErr == 0)
			{
				DTRACE(DB_FAPROTO, ("CSocket::Receive: len==-1 errno=%s\n", strerror(iLastErr)));
				return 0;
			}
			else
			{
	  			DTRACE(DB_FAPROTO, ("CSocket::Receive: len==-1 errno=%ld, %s\n", iLastErr, strerror(iLastErr)));
	    		SocketDisConnect(pSocket);
				SocketSetLastError(pSocket->iSocket, 0);
                pSocket->iSocket = INVALID_SOCKET;
        		return -1;
			}
#else
			return 0;
#endif
      	}
      	else
      	{
            Sleep(50);
		  	DoLedBurst(LED_REMOTE_RX);
			if (pSocket->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
				AddFlux((DWORD )len);

      		DTRACE(DB_FAPROTO, ("CSocket::Receive:GetClick()=%d.\r\n",GetClick()));
      		//TraceFrm("--> CSocket::Receive:", pbRxBuf, len);
      	}
	}
	else
	{
		Sleep(200);
        return -1;
	}

	return len; 
}

//描述：建立连接
bool SocketConnect(TSocket *pSocket)
{
	TMasterIp tMasterIp;
	struct  sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	unsigned int arg;

	SocketDisConnect(pSocket);
	
	if (pSocket->fUdp == 0x01)
	{
		pSocket->iSocket = socket(PF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		pSocket->iSocket = socket(PF_INET, SOCK_STREAM, 0);
	}
		
	if (pSocket->iSocket == INVALID_SOCKET)
	{
	  	DTRACE(DB_FAPROTO, ("CSocket::Connect: failed to create socket.\r\n"));
		pSocket->cLastErr = GPRS_ERR_CON;
	  	return false;
	}

//	if (pSocket->fUdp)
	{
		local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		local_addr.sin_family = AF_INET;
        if (pSocket->fUdp)
    		local_addr.sin_port = htons(1024); //UDP挷定固定的端口
        else
            local_addr.sin_port = htons(0);    //TCP挷定0时，TCP协议会随机选择一个端口挷定，有利于通过防火墙
	
		if (bind(pSocket->iSocket, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocket::Connect: Error: bind error\n"));
		  	SocketClose(pSocket);
			pSocket->cLastErr = GPRS_ERR_CON;
		  	return false;
		}
	}
	
	GetMasterIp(&tMasterIp);
	
	remote_addr.sin_addr.s_addr = htonl(tMasterIp.dwRemoteIP);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(tMasterIp.wRemotePort);
	
	DTRACE(DB_FAPROTO, ("CSocket::Connect: connecting %d.%d.%d.%d %d.\r\n",
						 (tMasterIp.dwRemoteIP>>24)&0xff, (tMasterIp.dwRemoteIP>>16)&0xff, (tMasterIp.dwRemoteIP>>8)&0xff, tMasterIp.dwRemoteIP&0xff, 
						 tMasterIp.wRemotePort));
	
	if (pSocket->fUdp == false)
	{
		if (connect(pSocket->iSocket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) != 0)
    	{
    		DTRACE(DB_FAPROTO, ("CSocket::Connect: connect fail.\r\n"));
		  	SocketClose(pSocket);
	  		//UpdateErrRst(false);
			pSocket->cLastErr = GPRS_ERR_CON;
		  	return false;
    	}
	}
	
	//UpdateErrRst(true);
    DTRACE(DB_FAPROTO, ("CSocket::Connect: connect ok.\r\n"));

	//int timeout = 2000;
	//if (setsockopt(pSocket->iSocket, SOL_SOCKET, SO_RCVTIMEO, (char* )&timeout, sizeof(timeout)) != 0)
    //int optval = 1;
	//if (setsockopt(pSocket->iSocket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) != 0) 
	arg = 1;
	if (ioctlsocket(pSocket->iSocket, FIONBIO,  (ULONG* )&arg) != 0) //(u_long FAR*)
    {
    	DTRACE(DB_FAPROTO, ("CSocket::Connect: ioctl fail.\r\n"));
	  	SocketClose(pSocket);
		pSocket->cLastErr = GPRS_ERR_CON;
	  	return false;
    }

	pSocket->cLastErr = GPRS_ERR_OK;
    SetSocketLed(true);
  	return true;
}


//描述:接口保活探测
void SocketKeepAlive(TSocket *pSocket)
{	/*
	DWORD dwClick, dwBrokenTime;
	struct TPro* pPro = pProIf->pPro;	//通信协议
	DWORD dwBeatMinutes = pSocket->bBeatMin;		//客户端心跳间隔,单位分钟
	if (dwBeatMinutes == 0)
		return;
	
	dwClick = GetClick();
	dwBrokenTime = dwBeatMinutes*60 + BEAT_TEST_TO*BEAT_TEST_TIMES;
	if (dwClick-pProIf->dwRxClick > dwBrokenTime)
	{	
		SocketDisConnect(pProIf); //重新初始化
		DTRACE(DB_FAPROTO, ("SocketKeepAlive: DisConnect at click %ld\r\n", dwClick));
	}
	else if (dwClick-pProIf->dwRxClick > dwBeatMinutes*60)//40 20秒还没收到过一帧，则进行心跳检测
	{	//刚开始时dwBeatClick为0，能马上发
		if (dwClick-pSocket->dwBeatClick > BEAT_TEST_TO)
		{							//心跳超时时间,单位秒
			pPro->pfnBeat(pPro);
			pSocket->dwBeatClick = dwClick;
			DTRACE(DB_FAPROTO, ("SocketKeepAlive: heart beat test at click %ld\r\n", dwClick));
		}
	}*/
}


bool SocketClose(TSocket *pSocket)
{
	if (pSocket->iSocket != INVALID_SOCKET)
	{
#ifndef SYS_WIN
		unsigned int arg = 0;
		ioctlsocket(pSocket->iSocket, FIONBIO,  (ULONG* )&arg);//设置为非阻塞		
		arg = 1;
		setsockopt(pSocket->iSocket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int));
		//要已经处于连接状态的soket在调用closesocket后强制关闭，省掉CLOSE_WAIT的过程：
		struct linger so_linger;
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		setsockopt(pSocket->iSocket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
#endif
	  	//closesocket(pSocket->iSocket);
        close(pSocket->iSocket);
	  	pSocket->iSocket = INVALID_SOCKET;
	  	DTRACE(DB_FAPROTO, ("CSocket::Close: close socket at click %d\n", GetClick()));
	}
	
	return true;
}


//描述:在接口由连接转为断开的时候调用，不管是主动断开还是被动断开
bool SocketDisConnect(TSocket *pSocket)
{
	if (pSocket->iSocket != INVALID_SOCKET) //断开socket连接
	{
        SetSocketLed(false);
		SocketClose(pSocket);
	  	pSocket->iSocket = INVALID_SOCKET;
	}
	
	//if (pSocket->fSvr) //服务器模式下,socket断开时退出线程
		//pProIf->fExit = true;
	//else if (pSocket->bSubState > SK_STATE_CONNECT)	//客户端
    if (pSocket->bSubState > SK_STATE_CONNECT)	//客户端
		pSocket->bSubState = SK_STATE_CONNECT;
	
	return true;
}

/*
void SocketTrans(struct TSocket *pSocket)
{
	DWORD dwClick = GetClick();
	//TSocket* pSocket = (TSocket* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//通信协议

	switch (pSocket->bSubState)	//客户机状态机：连接->登陆->通信->主动断开->空闲
	{
	case SK_STATE_IDLE:  	 	//空闲:有主动上报需求->连接状态
		break;

	case SK_STATE_CONNECT:		//连接:连接成功->登陆状态
		if (SocketConnect(pProIf))
			pSocket->bSubState = SK_STATE_LOGIN;
		else
			Sleep(2000);

		break;

	case SK_STATE_LOGIN:   	//登录:登录成功->传输状态
		if (pPro->pfnLogin(pPro))
			pSocket->bSubState = SK_STATE_TRANS;
		else
			Sleep(2000);

		break;

	case SK_STATE_TRANS:   	//传输:连续无通信规定时间->空闲状态
		//接收帧处理
		pPro->pfnRcvFrm(pPro); //接收到的一帧,并已经对其进行处理

		if (!pSocket->fSvr) //客户端模式下
		{
			pPro->pfnAutoSend(pPro);	//主动上送处理
			SocketKeepAlive(pProIf);
		}

		break;
	}
}*/

/*
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
*/
//#endif 	//SYS_WIN
