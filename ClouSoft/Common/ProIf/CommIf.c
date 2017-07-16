/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CommIf.c
 * ժ    Ҫ�����ļ�ʵ���˴���ͨ�Žӿ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
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
//CommIf˽�г�Ա����
//��Ϊ���ܻ�������CommIf�ӿڣ�����Ӧ�������ⶨ��ȫ�ֱ��������԰ѳ�Ա�������嵽TCommIf


////////////////////////////////////////////////////////////////////////////////////////////
//CommIf˽�к�������
bool CommIfClose(struct TProIf* pProIf);
bool CommIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen);
int CommIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize);

////////////////////////////////////////////////////////////////////////////////////////////
//CommIfʵ��

bool CommIfInit(struct TProIf* pProIf)
{
	TCommIf* pCommIf = (TCommIf* )pProIf->pvIf;
	
	//����ṹ��Ա��ʼ��
	ProIfInit(pProIf);
	
	//������
	pProIf->pszName = "CommIf";					//�ӿ�����
	pProIf->wMaxFrmBytes = COMM_MAX_BYTES;		//���֡����
	pProIf->bIfType = IF_COMM;					//�ӿ�����

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pProIf->pfnSend = CommIfSend;				//���ͺ���
	pProIf->pfnReceive = CommIfReceive;			//���պ���

	//����������ʼ��
	pProIf->bState = IF_STATE_RST;
	if (pCommIf->fDebug == false)	//�������ڣ�û����������ڹ���
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
	if (pCommIf->fDebug == false)	//�������ڣ�û����������ڹ���
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
    iLen = (int)(CommRead(pCommIf->wPort, pbRxBuf, wBufSize, 200)); //UART ������δ����Ӧ������һ��,�벨�����뻺������С�й�
    
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


