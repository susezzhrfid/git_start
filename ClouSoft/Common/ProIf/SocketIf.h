/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：SocketIf.h
 * 摘    要：本文件实现了socket通信接口类,只有WINDOWS平台支持本接口, 终端协议栈支持本接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月

 *********************************************************************************************************/
#ifndef SOCKETIF_H
#define SOCKETIF_H
#include "ProIf.h"

typedef struct{
	//参数
	bool 	fSvr;	//是否是服务器模式
	bool 	fUdp;	//是否是UDP通信方式
	bool	fEnableFluxStat;	//是否允许流量控制,只有本socket用的是GPRS通道时才支持
    BYTE    bCnType;
    
	//数据
	int  iSocket;
	char cLastErr;		//最后的错误

	bool fBakIP;
	WORD wIPUseCnt;
	//WORD wConnectFailCnt;	//连接失败次数
	WORD wConnectNum;	//连接失败连续尝试的次数
}TSocketIf;	//Socket接口子类

////////////////////////////////////////////////////////////////////////////////////////////
//SocketIf公共函数定义
bool SocketIfInit(TSocketIf* pSocketIf);
bool SocketIfDisConnect(TSocketIf* pSocketIf);
bool SocketIfConnect(TSocketIf* pSocketIf);
void SocketIfOnConnectFail(TSocketIf* pSocketIf);
void SocketIfOnConnectOK(TSocketIf* pSocketIf);
void SocketIfOnLoginOK(TSocketIf* pSocketIf);
void SocketIfEnterDorman(TSocketIf* pSocketIf);
bool SocketIfClose(TSocketIf* pSocketIf);
bool SocketIfSend(TSocketIf* pSocketIf, BYTE* pbTxBuf, WORD wLen);
int SocketIfReceive(TSocketIf* pSocketIf, BYTE* pbRxBuf, WORD wBufSize);
void SocketIfTrans(TSocketIf* pSocketIf);

bool SocketIfListen(TSocketIf* pSocketIf);
bool SocketIfCloseListen(TSocketIf* pSocketIf);

bool SetSocketLed(bool fLight);

#endif  //SOCKETIF_H




