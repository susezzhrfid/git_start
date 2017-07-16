
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
//CommIf˽�к�������
bool WlIfOpen(struct TProIf* pProIf);
bool WlIfClose(struct TProIf* pProIf);
bool WlIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen);
int WlIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize);

////////////////////////////////////////////////////////////////////////////////////////////
//CommIfʵ��

bool WlIfInit(struct TProIf* pProIf)
{
	TWLIf* pWlIf = (TWLIf* )pProIf->pvIf;
	
	//����ṹ��Ա��ʼ��
	ProIfInit(pProIf);
	
	//������
	pProIf->pszName = "WlIf";					//�ӿ�����
	pProIf->wMaxFrmBytes = COMM_WIREL_FRMSIZE;	//���֡����
	pProIf->bIfType = IF_WIRELESS;				//�ӿ�����

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pProIf->pfnSend = WlIfSend;				    //���ͺ���
	pProIf->pfnReceive = WlIfReceive;			//���պ���
    
	//����������ʼ��
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
