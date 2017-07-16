
#include "FaCfg.h"
#include "sysarch.h" 
#include "stdio.h"
#include "CommIf.h"
#include "FaConst.h"
#include "ComAPI.h"
#include "drivers.h"
#include "Trace.h"
#include "SysDebug.h"
#include "WlIf.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CommIf私有函数定义
bool WlIfOpen(struct TProIf* pProIf);
bool WlIfClose(struct TProIf* pProIf);
bool WlIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen);
int WlIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize);

////////////////////////////////////////////////////////////////////////////////////////////
//CommIf实现

bool WlIfInit(struct TProIf* pProIf)
{
	TWLIf* pWlIf = (TWLIf* )pProIf->pvIf;
	
	//基类结构成员初始化
	ProIfInit(pProIf);
	
	//类派生
	pProIf->pszName = "WlIf";					//接口名称
	pProIf->wMaxFrmBytes = COMM_WIREL_FRMSIZE;	//最大帧长度
	pProIf->bIfType = IF_WIRELESS;				//接口类型

	//虚函数，需要实例化为具体接口的对应函数
	pProIf->pfnSend = WlIfSend;				    //发送函数
	pProIf->pfnReceive = WlIfReceive;			//接收函数
    
	//派生类具体初始化
	pProIf->bState = IF_STATE_RST;
	if (!CommOpen(pWlIf->wPort, pWlIf->dwBaudRate,
				  pWlIf->bByteSize, pWlIf->bStopBits, pWlIf->bParity))
	{
		DTRACE(DB_FAPROTO, ("WlIfInit : failed to open COM%d\n", pWlIf->wPort));
		return false;
	}
	
	pProIf->bState = IF_STATE_TRANS; 
	
  	return true;
}

bool WlIfOpen(struct TProIf* pProIf) 
{ 
	return true;
}

bool WlIfClose(struct TProIf* pProIf) 
{ 
	TWLIf* pWlIf = (TWLIf* )pProIf->pvIf;
	if (CommIsOpen(pWlIf->wPort))
		CommClose(pWlIf->wPort);
	return true;
}


bool WlIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen)
{
	TWLIf* pWlIf = (TWLIf* )pProIf->pvIf;	
#ifdef SYS_WIN
    if (IsDebugOn(DB_FAPROTO))
    {
        char szBuf[48];	
		sprintf(szBuf, "--> CWlIf(%d)::Send:", pProIf->bIfType);
		TraceFrm(szBuf, pbTxBuf, wLen);
    }
#endif
    //DTRACE(DB_FAPROTO,("WlIfSend:%d\r\n", wLen));
	return CommWrite(pWlIf->wPort, pbTxBuf, wLen, 2000) == wLen;
}


int WlIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize)
{
	TWLIf* pWlIf = (TWLIf* )pProIf->pvIf;
	int iLen = 0;

#ifndef SYS_WIN
    //SIRecvPacket();
   	//SetLedStatus(WIRELESS_LED,LED_SLOW);
#endif

    iLen = CommRead(pWlIf->wPort, pbRxBuf, wBufSize, 1000);
    /*if (iLen>0)
    {
		DTRACE(DB_FAPROTO,("WLIfReceive:%d\r\n", iLen));
    }*/
    
#ifdef SYS_WIN
	if (iLen>0 && IsDebugOn(DB_FAPROTO))
	{	
	    char szBuf[48];
		sprintf(szBuf, "<-- CWlIf(%d)::Receive:", pProIf->bIfType);
		TraceFrm(szBuf, pbRxBuf, iLen);
	}
#endif		
	return iLen;
}
