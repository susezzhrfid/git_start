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

typedef struct{	
		
	//数据
	BYTE bSvrSock;	//服务器端的socket号，只能为0~1，赋值0xff表示无效。
	BYTE bCliSock;	//客户端的socket号，目前固定只使用0
	bool fPowOn;	//是否是刚上电
	bool fCliUdp;	//客户端是否是UDP连接
    
    bool fDataUpload;  //有数据上报
    bool fLinkErr;     //网络断开
    bool fInitiativeErr;   //主动判断模块网络断开
}TM590;


////////////////////////////////////////////////////////////////////////////////////////////
//M590公共函数定义
void M590Init(struct TModem* pModem);

////////////////////////////////////////////////////////////////////////////////////////////
//M590私有函数定义
int M590ResetModem(struct TModem* pModem);
int M590InitAPN(struct TModem* pModem);

bool M590Connect(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort);
bool M590Listen(struct TModem* pModem, bool fUdp, WORD wLocalPort);
bool M590CloseCliSock(struct TModem* pModem);
bool M590CloseSvrSock(struct TModem* pModem);
bool M590CloseListen(struct TModem* pModem);

bool M590ClosePpp(struct TModem* pModem);
bool M590OpenPpp(struct TModem* pModem);

int M590SvrReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize);
int M590CliReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize);

bool M590ChkCliStatus(struct TModem* pModem);
bool M590ChkSvrStatus(struct TModem* pModem);	

int M590CliSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen);
int M590SvrSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen);

bool M590SpecHandle(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize);
bool M590IsSvrAcceptOne(struct TModem* pModem);

char* M590IsRxIpFrm(struct TModem* pModem, char* pszRx, bool* pfSvr, WORD* pwLen);
extern DWORD GetLocalAddr();

#endif  //GC864_H









 