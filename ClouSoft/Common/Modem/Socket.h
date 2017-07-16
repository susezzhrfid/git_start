/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Socket.h
 * 摘    要：本文件实现了socket通信,支持GPRS终端协议栈
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月 
 *********************************************************************************************************/
#ifndef SOCKET_H
#define SOCKET_H
#include "ProIf.h"

typedef unsigned long ULONG;

#define INVALID_SOCKET -1

typedef struct{
	//参数
	bool 	fSvr;	//是否是服务器模式
	bool 	fUdp;	//是否是UDP通信方式
	bool	fEnableFluxStat;	//是否允许流量控制,只有本socket用的是GPRS通道时才支持
	BYTE 	bBeatMin;		//客户端心跳间隔,单位分钟
   
	//数据
	int  iSocket;
	DWORD dwBeatClick;
	BYTE bSubState;		//传输的子状态
	char cLastErr;		//最后的错误
}TSocket;	//Socket接口子类

////////////////////////////////////////////////////////////////////////////////////////////
//Socket公共函数定义
bool SocketInit(TSocket *pSocket);

bool SocketDisConnect(TSocket *pSocket);
bool SocketConnect(TSocket *pSocket);
bool SocketClose(TSocket *pSocket);
bool SocketSend(TSocket *pSocket, BYTE* pbTxBuf, WORD wLen);
int SocketReceive(TSocket *pSocket, BYTE* pbRxBuf, WORD wBufSize);
void SocketTrans(TSocket *pSocket);

//bool SetSocketLed(bool fLight);

extern TSocket g_tSock;

#endif  //SOCKET_H




