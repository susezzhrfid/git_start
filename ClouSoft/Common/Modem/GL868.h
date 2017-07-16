/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：M590.h
 * 摘    要：本文件实现了GC864MODEM子类
 * 当前版本：1.0
 * 作    者：杨进、岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef GC864_H
#define GC864_H
#include "Modem.h"
#include "socket.h"

typedef struct{
	//BYTE bModuleVer;
	TSocket *pSocket;
    
	//数据
	BYTE bSvrSock;	//服务器端的socket号，只能为0~1，赋值0xff表示无效。
	BYTE bCliSock;	//客户端的socket号，目前固定只使用0
	bool fPowOn;	//是否是刚上电
	bool fCliUdp;	//客户端是否是UDP连接
}TGL868;


////////////////////////////////////////////////////////////////////////////////////////////
//M590公共函数定义
void GL868Init(struct TModem* pModem);

////////////////////////////////////////////////////////////////////////////////////////////
//M590私有函数定义
int GL868ResetModem(struct TModem* pModem);
int GL868InitAPN(struct TModem* pModem);

bool GL868Connect(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort);
bool GL868Listen(struct TModem* pModem, bool fUdp, WORD wLocalPort);
bool GL868CloseCliSock(struct TModem* pModem);
bool GL868CloseSvrSock(struct TModem* pModem);
bool GL868CloseListen(struct TModem* pModem);

bool GL868ClosePpp(struct TModem* pModem);
bool GL868OpenPpp(struct TModem* pModem);

int GL868SvrReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize);
int GL868CliReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize);

bool GL868ChkCliStatus(struct TModem* pModem);
bool GL868ChkSvrStatus(struct TModem* pModem);	

int GL868CliSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen);
int GL868SvrSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen);

bool GL868SpecHandle(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize);
bool GL868IsSvrAcceptOne(struct TModem* pModem);

char* GL868IsRxIpFrm(struct TModem* pModem, char* pszRx, bool* pfSvr, WORD* pwLen);
extern DWORD GetLocalAddr();

#endif  //GC864_H









 