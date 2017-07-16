/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Socket.c
 * ժ    Ҫ�����ļ�ʵ����socketͨ��,֧��GPRS�ն�Э��ջ
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
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
//Socket˽�к궨��

//�ͻ���״̬��
#define SK_STATE_IDLE		0   //����:�������ϱ�����->����״̬
#define SK_STATE_CONNECT 	1	//����:���ӳɹ�->��½״̬
#define SK_STATE_LOGIN		2   //��¼:��¼�ɹ�->����״̬
#define SK_STATE_TRANS		3   //����:������ͨ�Ź涨ʱ��->����״̬

#define BEAT_TEST_TIMES 	2	//�������Դ���,   Ϊ0,��ʾ���Զ�����,ֻ���ڷ�����
#define BEAT_TEST_TO		30	//������ʱʱ��,��λ��

////////////////////////////////////////////////////////////////////////////////////////////
//Socket˽�г�Ա����
//��Ϊ���ܻ�������Socket���ӣ�����Ӧ�������ⶨ��ȫ�ֱ��������԰ѳ�Ա�������嵽TSocket

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Socketʵ��

TSocket g_tSock;

bool SocketInit(TSocket *pSocket)
{	
	//����������ʼ��
	pSocket->iSocket = INVALID_SOCKET;
	pSocket->bSubState = SK_STATE_CONNECT;
    
    pSocket->fUdp = 0;         //TCPģʽ
	pSocket->fEnableFluxStat = true;	//�Ƿ�������������,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
	return true;
}

//����:��ʼ��������
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
		if (pSocket->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
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

//����:���մ�����������,�������ѭ���������л�������,�򷵻�ѭ���������е�����,
//     ������ô��ڽ��պ���,ֱ�ӵȴ����ڵ����ݵ���
//����:@pbRxBuf �������շ������ݵĻ�����,
//     @wBufSize ���ջ������Ĵ�С
//����:�������ݵĳ���,����Ϊ���� -1��socket�����Ͽ�
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
  	    	DTRACE(DB_FAPROTO, ("CSocket::Receive: close socket due to rx len=0.\r\n"));//TCP����ʱ�������رա�
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
			if (iLastErr == EWOULDBLOCK)   //TCP��socket��ĳ�ʱ��Ӧ�ó���Ӧִ���ش�           //todo,ע����11����-11
        	{               //Ӧ�ü���ֻҪͨ���������ܴ������ݣ�TCP�ͻ�ѽ��������ݸ��Է�
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
			if (pSocket->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
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

//��������������
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
    		local_addr.sin_port = htons(1024); //UDP�����̶��Ķ˿�
        else
            local_addr.sin_port = htons(0);    //TCP����0ʱ��TCPЭ������ѡ��һ���˿ڒ�����������ͨ������ǽ
	
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


//����:�ӿڱ���̽��
void SocketKeepAlive(TSocket *pSocket)
{	/*
	DWORD dwClick, dwBrokenTime;
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��
	DWORD dwBeatMinutes = pSocket->bBeatMin;		//�ͻ����������,��λ����
	if (dwBeatMinutes == 0)
		return;
	
	dwClick = GetClick();
	dwBrokenTime = dwBeatMinutes*60 + BEAT_TEST_TO*BEAT_TEST_TIMES;
	if (dwClick-pProIf->dwRxClick > dwBrokenTime)
	{	
		SocketDisConnect(pProIf); //���³�ʼ��
		DTRACE(DB_FAPROTO, ("SocketKeepAlive: DisConnect at click %ld\r\n", dwClick));
	}
	else if (dwClick-pProIf->dwRxClick > dwBeatMinutes*60)//40 20�뻹û�յ���һ֡��������������
	{	//�տ�ʼʱdwBeatClickΪ0�������Ϸ�
		if (dwClick-pSocket->dwBeatClick > BEAT_TEST_TO)
		{							//������ʱʱ��,��λ��
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
		ioctlsocket(pSocket->iSocket, FIONBIO,  (ULONG* )&arg);//����Ϊ������		
		arg = 1;
		setsockopt(pSocket->iSocket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int));
		//Ҫ�Ѿ���������״̬��soket�ڵ���closesocket��ǿ�ƹرգ�ʡ��CLOSE_WAIT�Ĺ��̣�
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


//����:�ڽӿ�������תΪ�Ͽ���ʱ����ã������������Ͽ����Ǳ����Ͽ�
bool SocketDisConnect(TSocket *pSocket)
{
	if (pSocket->iSocket != INVALID_SOCKET) //�Ͽ�socket����
	{
        SetSocketLed(false);
		SocketClose(pSocket);
	  	pSocket->iSocket = INVALID_SOCKET;
	}
	
	//if (pSocket->fSvr) //������ģʽ��,socket�Ͽ�ʱ�˳��߳�
		//pProIf->fExit = true;
	//else if (pSocket->bSubState > SK_STATE_CONNECT)	//�ͻ���
    if (pSocket->bSubState > SK_STATE_CONNECT)	//�ͻ���
		pSocket->bSubState = SK_STATE_CONNECT;
	
	return true;
}

/*
void SocketTrans(struct TSocket *pSocket)
{
	DWORD dwClick = GetClick();
	//TSocket* pSocket = (TSocket* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��

	switch (pSocket->bSubState)	//�ͻ���״̬��������->��½->ͨ��->�����Ͽ�->����
	{
	case SK_STATE_IDLE:  	 	//����:�������ϱ�����->����״̬
		break;

	case SK_STATE_CONNECT:		//����:���ӳɹ�->��½״̬
		if (SocketConnect(pProIf))
			pSocket->bSubState = SK_STATE_LOGIN;
		else
			Sleep(2000);

		break;

	case SK_STATE_LOGIN:   	//��¼:��¼�ɹ�->����״̬
		if (pPro->pfnLogin(pPro))
			pSocket->bSubState = SK_STATE_TRANS;
		else
			Sleep(2000);

		break;

	case SK_STATE_TRANS:   	//����:������ͨ�Ź涨ʱ��->����״̬
		//����֡����
		pPro->pfnRcvFrm(pPro); //���յ���һ֡,���Ѿ�������д���

		if (!pSocket->fSvr) //�ͻ���ģʽ��
		{
			pPro->pfnAutoSend(pPro);	//�������ʹ���
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

	CommRead(COMM_GPRS, NULL, 0, 10);    //��AT����ǰ�����崮�ڻ�����
	wLen = strlen(pszCmd);
    if (CommWrite(COMM_GPRS, (BYTE *)pszCmd, wLen, 1000) != wLen)
		return -2;
    
	do
	{
		if (i!=0 && i!=1)   //����nWaitSeconds==0�����,Read()����,��Sleep()
			Sleep(1000);
        
		wLen = CommRead(COMM_GPRS, bBuf+wAllLen, sizeof(bBuf)-wAllLen, 1000);
        wAllLen += wLen;
        if (wAllLen >= sizeof(bBuf))
            wAllLen = sizeof(bBuf)-1;
		bBuf[wAllLen] = 0;
        
		//2.������AT�����
		if (strstr((char* )bBuf, pszAnsOK) != NULL)   //���յ���ȷ�ش�
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
    		return wLen;  
		}

		if (pszAnsErr!=NULL && strstr((char* )bBuf, pszAnsErr)!=NULL)  //���յ�����ش�1
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -1;
		}
		
        if (wAllLen == sizeof(bBuf)-1) //Buf ��
            wAllLen = 0;  //��֮ǰ�յ����ӵ�

		i++;
		if (i == 1)    //����nWaitSeconds==0�����,���ٻ�������һ��
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
	//���ƵĿ�������	
	char* pszLight = "AT$MYSOCKETLED=1\r\n";
	char* pszDark = "AT$MYSOCKETLED=0\r\n";

    //�˳�����ģʽ
    for ( i=0; i<5; i++)
    {
        if (ATCmd("+++", "OK", NULL, 1) > 0)//Note:+++���ܼ��κη���,������ͨ��ʱ�������ʧ��
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
			
    //��������ģʽ    
    ATCmd("ATO\r\n", "CONNECT", NULL, 0);
	
	return fRet;
}
*/
//#endif 	//SYS_WIN
