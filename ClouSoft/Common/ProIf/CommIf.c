/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CommIf.c
 * 摘    要：本文件实现了串口通信接口类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#include "FaCfg.h"
#include "sysarch.h" 
#include <stdio.h>
#include "CommIf.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "drivers.h"
#include "DrvCfg.h"
#include "SysDebug.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CommIf私有成员变量
//因为可能会产生多个CommIf接口，所以应尽量避免定义全局变量，可以把成员变量定义到TCommIf


////////////////////////////////////////////////////////////////////////////////////////////
//CommIf私有函数定义
bool CommIfClose(struct TProIf* pProIf);
bool CommIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen);
int CommIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize);

////////////////////////////////////////////////////////////////////////////////////////////
//CommIf实现

bool CommIfInit(struct TProIf* pProIf)
{
	TCommIf* pCommIf = (TCommIf* )pProIf->pvIf;
	
	//基类结构成员初始化
	ProIfInit(pProIf);
	
	//类派生
	pProIf->pszName = "CommIf";					//接口名称
	pProIf->wMaxFrmBytes = COMM_MAX_BYTES;		//最大帧长度
	pProIf->bIfType = IF_COMM;					//接口类型

	//虚函数，需要实例化为具体接口的对应函数
	pProIf->pfnSend = CommIfSend;				//发送函数
	pProIf->pfnReceive = CommIfReceive;			//接收函数

	//派生类具体初始化
	pProIf->bState = IF_STATE_RST;
	if (pCommIf->fDebug == false)	//单独串口，没跟调试输出口共用
	{	
		if (!CommOpen(pCommIf->wPort, pCommIf->dwBaudRate,
					  pCommIf->bByteSize, pCommIf->bStopBits, pCommIf->bParity))
		{
			DTRACE(DB_FAPROTO, ("CommIfInit : failed to open COM%d\n", pCommIf->wPort));
			return false;
		}
	}
	
	pProIf->bState = IF_STATE_TRANS; 
	
  	return true;
}

bool CommIfClose(struct TProIf* pProIf) 
{ 
	TCommIf* pCommIf = (TCommIf* )pProIf->pvIf;
	if (pCommIf->fDebug == false)	//单独串口，没跟调试输出口共用
	{	
		if (CommIsOpen(pCommIf->wPort))
			CommClose(pCommIf->wPort);
	}
			
	return true;
}


bool CommIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen)
{
	TCommIf* pCommIf = (TCommIf* )pProIf->pvIf;	
//#ifdef SYS_WIN
    if (IsDebugOn(DB_FAPROTO))
    {
        char szBuf[48];	
		//sprintf(szBuf, "--> CCommIf(%d)::Send:", pProIf->bIfType);
		sprintf(szBuf, "--> CCommIf(%d)::Send:", pCommIf->wPort);
		//TraceFrm(szBuf, pbTxBuf, wLen);
    }
//#endif
    
    /*if (wLen>0)
    {
        DTRACE(DB_FAPROTO,("CommIfSend:%d\r\n", wLen));
    }*/
    
	return CommWrite(pCommIf->wPort, pbTxBuf, wLen, 2500) == wLen;
}


int CommIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize)
{
	TCommIf* pCommIf = (TCommIf* )pProIf->pvIf;
	int iLen = 0;

	if (pCommIf->wPort == COMM_DEBUG)
	{
		BYTE bRS232Mode = 0, bPortFun = 0;
		ReadItemEx(BN2, PN0, 0x2110, &bRS232Mode);
		ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
		if (bRS232Mode==1 && bPortFun==PORT_FUN_DEBUG)
			return 0;
	}
    iLen = (int)(CommRead(pCommIf->wPort, pbRxBuf, wBufSize, 200)); //UART 缓存区未满就应该来读一次,与波特率与缓存区大小有关
    
    /*if (iLen>0)
    {
        DTRACE(DB_FAPROTO,("CommIfReceive:%d\r\n", iLen));
    }*/
       
//#ifdef SYS_WIN
	if (iLen>0 && IsDebugOn(DB_FAPROTO))
	{	
	    char szBuf[48];
		//sprintf(szBuf, "<-- CCommIf(%d)::Receive:", pProIf->bIfType);
		sprintf(szBuf, "<-- CCommIf(%d)::Receive:", pCommIf->wPort);
		//TraceFrm(szBuf, pbRxBuf, iLen);
	}
//#endif		
	return iLen;
}


