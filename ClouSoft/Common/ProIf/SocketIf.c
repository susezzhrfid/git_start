/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�SocketIf.c
 * ժ    Ҫ�����ļ�ʵ����socketͨ�Žӿ���,ֻ��WINDOWSƽ̨֧�ֱ��ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#include "FaCfg.h"
//#ifdef SYS_WIN
#include <stdio.h>
#include "SysArch.h" 
#include "SocketIf.h"
#include "Trace.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "SysApi.h"
#include "SysDebug.h"
#include "ProStruct.h"
#include "ProPara.h"
#include "sockets.h"
#include "ctrl.h"
#include "SysCfg.h"
#include "FaAPI.h"

typedef u_long ULONG;
#define INVALID_SOCKET   -1
#define SOCKET_ERROR   -1

//#define EWOULDBLOCK WSAEWOULDBLOCK
#define EWOULDBLOCK    11

#ifdef SYS_WIN
int SocketGetLastError(int s)
{
	return WSAGetLastError();
}

void SocketSetLastError(int s, int iError)
{
	WSASetLastError(iError);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//SocketIf˽�к궨��

//�ͻ���״̬��
#define SK_STATE_IDLE		0   //����:�������ϱ�����->����״̬
#define SK_STATE_CONNECT 	1	//����:���ӳɹ�->��½״̬
#define SK_STATE_LOGIN		2   //��¼:��¼�ɹ�->����״̬
#define SK_STATE_TRANS		3   //����:������ͨ�Ź涨ʱ��->����״̬

////////////////////////////////////////////////////////////////////////////////////////////
//SocketIf˽�г�Ա����
//��Ϊ���ܻ�������Socket���ӣ�����Ӧ�������ⶨ��ȫ�ֱ��������԰ѳ�Ա�������嵽TSocketIf
/////////////////////////////////////////////////////////////////////////////////////////////////////
//SocketIfʵ��

bool SocketIfInit(TSocketIf* pSocketIf)
{
/*	//����ṹ��Ա��ʼ��
	ProIfInit(pProIf);

	//������
	pProIf->pszName = "SocketIf";				//�ӿ�����
	pProIf->wMaxFrmBytes = SOCK_MAX_BYTES;		//���֡����
	pProIf->bIfType = IF_SOCKET;				//�ӿ�����

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pProIf->pfnSend = SocketIfSend;				//���ͺ���
	pProIf->pfnReceive = SocketIfReceive;		//���պ���
	pProIf->pfnTrans = SocketIfTrans;			//����״̬����*/
    
	//����������ʼ��
	pSocketIf->iSocket = INVALID_SOCKET;
	//pProIf->bState = IF_STATE_TRANS;
	//pSocketIf->bSubState = SK_STATE_CONNECT;

	return true;
}

//����:��ʼ��������
void SocketIfInitSvr(TSocketIf* pSocketIf, int socket)
{	
	pSocketIf->iSocket = socket;
	//pProIf->bState = IF_STATE_TRANS;
	//pSocketIf->bSubState = SK_STATE_TRANS;

	//pProIf->fExit = false;
	//pProIf->fExitDone = false;
}

bool SocketIfSend(TSocketIf* pSocketIf, BYTE* pbTxBuf, WORD wLen)
{
	int tolen;
	TMasterIp tMasterIp;	
	DTRACE(DB_FAPROTO, ("CSocketIf::Send: GetClick()=%d,wLen=%d.\r\n",GetClick(),wLen));
#ifdef SYS_WIN
	TraceFrm("<-- CSocketIf::Send:", pbTxBuf, wLen);	
#endif
	if (pSocketIf->iSocket != INVALID_SOCKET)
	{
		int iReLen = 0;
		if (pSocketIf->fUdp)
		{
			struct  sockaddr_in to;

			GetMasterIp(&tMasterIp);
            if (!pSocketIf->fBakIP)  //��IP
            {
			    to.sin_addr.s_addr = htonl(tMasterIp.dwRemoteIP);
    			to.sin_family = AF_INET;
    			to.sin_port = htons(tMasterIp.wRemotePort);	
            }
            else  //��IP
            {
                to.sin_addr.s_addr = htonl(tMasterIp.dwBakIP);
    			to.sin_family = AF_INET;
    			to.sin_port = htons(tMasterIp.wBakPort);	
            }
			tolen=sizeof(to);				
			iReLen = sendto(pSocketIf->iSocket, (char* )pbTxBuf, wLen, 0,(struct sockaddr *)&to,tolen);
		}
		else
		{
			iReLen = send(pSocketIf->iSocket, (char* )pbTxBuf, wLen, 0);
		}

		DoLedBurst(LED_REMOTE_TX);
		if (pSocketIf->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
			AddFlux(wLen);

		if (iReLen == wLen)
		{
			return true;
		}
		else
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail iReLen=%d, wLen=%d.\r\n", iReLen, wLen));
			return false;
		}
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail due to invalid sock.\r\n"));
		return false;
	}
}

//����:���մ�����������,�������ѭ���������л�������,�򷵻�ѭ���������е�����,
//     ������ô��ڽ��պ���,ֱ�ӵȴ����ڵ����ݵ���
//����:@pbRxBuf �������շ������ݵĻ�����,
//     @wBufSize ���ջ������Ĵ�С
//����:�������ݵĳ���
int SocketIfReceive(TSocketIf* pSocketIf, BYTE* pbRxBuf, WORD wBufSize)
{	
	int len = 0, i=0; 
	struct  sockaddr_in from; 
	i = sizeof(from);

	if (pSocketIf->iSocket != INVALID_SOCKET)
	{
		//Sleep(10);   //todo:Ӱ�����������ٶ�
  		//SockSetLastError(pSocketIf->iSocket, 0);
  		if (pSocketIf->fUdp == 0x01)
			len =  recvfrom(pSocketIf->iSocket, (char*)pbRxBuf, wBufSize, 0,(struct sockaddr *)&from, &i);	//(socklen_t* )
		else
			len =  recv(pSocketIf->iSocket, (char *)pbRxBuf, wBufSize, 0);

	  	if (len == 0)//�Է����˸�λ��·ָ��������ƶ����羭�����������
      	{
            Sleep(100);
  	    	DTRACE(DB_FAPROTO, ("CSocketIf::Receive: close socket due to rx len=0.\r\n"));
	    	SocketIfDisConnect(pSocketIf);            
	    	return -1;
      	}
      	else if (len == SOCKET_ERROR) 
	  	{
            Sleep(100);
#ifndef SYS_WIN
			int iLastErr =  SocketGetLastError(pSocketIf->iSocket);

	  		//DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(errno)));
	  		//return 0;
			if (iLastErr == EWOULDBLOCK)   //TCP��socket��ĳ�ʱ��Ӧ�ó���Ӧִ���ش�
        	{               //Ӧ�ü���ֻҪͨ���������ܴ������ݣ�TCP�ͻ�ѽ��������ݸ��Է�
				SocketSetLastError(pSocketIf->iSocket, 0);
          		return 0;
			}
			else if (iLastErr == 0)
			{
				DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(iLastErr)));
				return 0;
			}
			else
			{
	  			DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%ld, %s\n", iLastErr, strerror(iLastErr)));
	    		SocketIfDisConnect(pSocketIf);
				SocketSetLastError(pSocketIf->iSocket, 0);
        		return -1;
			}
#else
			return 0;
#endif
      	}
      	else
      	{
            Sleep(50);   //̫Ƶ������Ӱ�����߳�
		  	DoLedBurst(LED_REMOTE_RX);
			if (pSocketIf->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
				AddFlux((DWORD )len);

      		DTRACE(DB_FAPROTO, ("CSocketIf::Receive:GetClick()=%d.\r\n",GetClick()));
	#ifdef SYS_WIN
      		TraceFrm("--> CSocketIf::Receive:", pbRxBuf, len);
	#endif
      	}
	}
	else
	{
		Sleep(200);
        return -1;
	}

	return len; 
}

//����:ȡ���Ӵ���,GPRS��socket���ӵ�ʱ��,����б���IP�˿ڵĻ�,���Ӵ�����2
WORD SocketIfGetConnectNum(TSocketIf* pSocketIf)
{
	//TMasterIp tMasterIp;
	WORD wConnectNum = pSocketIf->wConnectNum;

	//GetMasterIp(&tMasterIp);
	
	//if (tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)
		//wConnectNum *= 2;

	return wConnectNum;
}

//��������������
bool SocketIfConnect(TSocketIf* pSocketIf)
{
	DWORD dwRemoteIP;
	WORD wRemotePort;
	TMasterIp tMasterIp;
	struct  sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	unsigned int arg;
    DWORD dwPort = 1024;

	SocketIfDisConnect(pSocketIf);
	
	if (pSocketIf->fUdp == 0x01)
	{
		pSocketIf->iSocket = socket(PF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		pSocketIf->iSocket = socket(PF_INET, SOCK_STREAM, 0);
	}
		
	if (pSocketIf->iSocket == INVALID_SOCKET)
	{
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: failed to create socket.\r\n"));
		pSocketIf->cLastErr = GPRS_ERR_CON;
	  	return false;
	}

	//if (pSocketIf->fUdp)  
	{
		local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		local_addr.sin_family = AF_INET;
        if (pSocketIf->fUdp)
    		local_addr.sin_port = htons(1024); //UDP�����̶��Ķ˿�
        else
        {
			//local_addr.sin_port = htons(0);    //TCP����0ʱ��TCPЭ������ѡ��һ���˿ڒ�����������ͨ������ǽ
		#ifdef SYS_WIN
			srand(GetTick()); //�������������
			dwPort = rand();
		#else
			GetRandom(&dwPort);
		#endif
			dwPort = dwPort%(9200-4097) + 4097;
            local_addr.sin_port = htons(dwPort);
            /*if ((g_PowerOffTmp.wLocalPort < 4097) || (g_PowerOffTmp.wLocalPort > 9200))
            {
                g_PowerOffTmp.wLocalPort = 4097;
            }
            local_addr.sin_port = htons(g_PowerOffTmp.wLocalPort++);  */
        }
	
		if (bind(pSocketIf->iSocket, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Connect: Error: bind error\n"));
		  	SocketIfClose(pSocketIf);
			pSocketIf->cLastErr = GPRS_ERR_CON;
		  	return false;
		}
	}
	
	GetMasterIp(&tMasterIp);

    //����IP�л���
	//1����û�гɹ���½��վ������£�����ʹ������IP
	//2���ڵ�½OK������£��嵱ǰIP��ʹ�ü������������Լ���ʹ�ã�
	if (pSocketIf->fBakIP	//��ǰҪʹ�ñ��õ�ַ
		&& tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)//����IP�˿���Ч
	{		//����IP�˿���Ч
		pSocketIf->wIPUseCnt++;
        if (pSocketIf->wIPUseCnt > pSocketIf->wConnectNum)
        {
			dwRemoteIP = tMasterIp.dwRemoteIP;
			wRemotePort = tMasterIp.wRemotePort;        	
            pSocketIf->wIPUseCnt = 1;   //�����Ѿ�ʹ�ù�һ����
            pSocketIf->fBakIP = false;  //�����л�����IP
        }
        else
        {
        	dwRemoteIP = tMasterIp.dwBakIP;
			wRemotePort = tMasterIp.wBakPort;
	    }
	}
	else
	{		
        pSocketIf->wIPUseCnt++;
        if (pSocketIf->wIPUseCnt>pSocketIf->wConnectNum &&
        	tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)
        {
            pSocketIf->wIPUseCnt = 1; //�����Ѿ�ʹ�ù�һ����
           	pSocketIf->fBakIP = true; //���л�������IP
        	dwRemoteIP = tMasterIp.dwBakIP;
			wRemotePort = tMasterIp.wBakPort;
        }
        else
        {
        	pSocketIf->fBakIP = false;  //��������IP
			dwRemoteIP = tMasterIp.dwRemoteIP;
			wRemotePort = tMasterIp.wRemotePort;
        }
	}

	remote_addr.sin_addr.s_addr = htonl(dwRemoteIP);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(wRemotePort);
	
	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connecting %d.%d.%d.%d %d, local port %d\r\n",
						 (dwRemoteIP>>24)&0xff, (dwRemoteIP>>16)&0xff, (dwRemoteIP>>8)&0xff, dwRemoteIP&0xff, 
						 wRemotePort, dwPort));
	
	if (pSocketIf->fUdp == false)
	{
		if (connect(pSocketIf->iSocket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) != 0)
    	{
    		DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect fail.\r\n"));
		  	SocketIfClose(pSocketIf);
	  		//UpdateErrRst(false);
			pSocketIf->cLastErr = GPRS_ERR_CON;
		  	return false;
    	}
	}
	
	//UpdateErrRst(true);
    DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect ok.\r\n"));
//	pProIf->dwRxClick = GetClick();//��ֹ����֮��û��ͨ�ţ�û�и��¸�ֵ��֮���keepalive�жϿ�

	//int timeout = 2000;
	//if (setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_RCVTIMEO, (char* )&timeout, sizeof(timeout)) != 0)
    //int optval = 1;
	//if (setsockopt(pSocketIf->iSocket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) != 0) 
#ifdef MY_TCP_KEEPALIVE     //todo:TCP����̽��  �ͼ�
    int keepAlive = SOF_KEEPALIVE; // ����keepalive����
    setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));//����TCP�㱣��̽��
    int keepIdle = 60000; // ���������60����û���κ���������,�����̽�� 
    setsockopt(pSocketIf->iSocket, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&keepIdle, sizeof(keepIdle));
#endif    
	arg = 1;
	if (ioctlsocket(pSocketIf->iSocket, FIONBIO,  (ULONG* )&arg) != 0) //(u_long FAR*)//����Ϊ������
    {
    	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: ioctl fail.\r\n"));
	  	SocketIfClose(pSocketIf);
		pSocketIf->cLastErr = GPRS_ERR_CON;
	  	return false;
    }

	pSocketIf->cLastErr = GPRS_ERR_OK;
    if (pSocketIf->bCnType == CN_TYPE_GPRS)
        SetSocketLed(true);
  	return true;
}

void SocketIfOnConnectFail(TSocketIf* pSocketIf)
{	
	//TMasterIp tMasterIp;

	//GetMasterIp(&tMasterIp);

	//�������
	/*pSocketIf->wConnectFailCnt++;
	if (pSocketIf->wConnectFailCnt > SocketIfGetConnectNum(pSocketIf))
	{
		pSocketIf->wConnectFailCnt = 0;
	}
	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: fail cnt = %d.\r\n", pSocketIf->wConnectFailCnt));*/
}

void SocketIfOnConnectOK(TSocketIf* pSocketIf)
{
	//pSocketIf->wConnectFailCnt = 0;
}

void SocketIfOnLoginOK(TSocketIf* pSocketIf)
{
    pSocketIf->wIPUseCnt = 0;
}

void SocketIfEnterDorman(TSocketIf* pSocketIf)
{
	//ShiftIP(pSocketIf);
	//DTRACE(DB_FAPROTO, ("CSocketIf::EnterDorman\n"));
}

bool SocketIfClose(TSocketIf* pSocketIf)
{
	if (pSocketIf->iSocket != INVALID_SOCKET)
	{
#ifndef SYS_WIN
		unsigned int arg = 0;
		ioctlsocket(pSocketIf->iSocket, FIONBIO,  (ULONG* )&arg);//����Ϊ����		
		arg = 1;
		setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int));
		//Ҫ�Ѿ���������״̬��soket�ڵ���closesocket��ǿ�ƹرգ�ʡ��CLOSE_WAIT�Ĺ��̣�
		struct linger so_linger;
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		setsockopt(pSocketIf->iSocket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

		close(pSocketIf->iSocket);
#else
		closesocket(pSocketIf->iSocket);
#endif	  	
        
	  	pSocketIf->iSocket = INVALID_SOCKET;
//	  	DTRACE(DB_FAPROTO, ("CSocketIf::Close: close socket at click %d\n", GetClick()));
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Close: close socket !!\r\n"));
	}
	
	return true;
}

//����:�ڽӿ�������תΪ�Ͽ���ʱ����ã������������Ͽ����Ǳ����Ͽ�
bool SocketIfDisConnect(TSocketIf* pSocketIf)
{	
	if (pSocketIf->iSocket != INVALID_SOCKET) //�Ͽ�socket����
	{
        if (pSocketIf->bCnType == CN_TYPE_GPRS)
            SetSocketLed(false);
		SocketIfClose(pSocketIf);
	  	//pSocketIf->iSocket = INVALID_SOCKET;
	}
	
/*	if (pSocketIf->fSvr) //������ģʽ��,socket�Ͽ�ʱ�˳��߳�
		pProIf->fExit = true;
	else if (pSocketIf->bSubState > SK_STATE_CONNECT)	//�ͻ���
		pSocketIf->bSubState = SK_STATE_CONNECT;*/
	
	return true;
}

//#endif 	//SYS_WIN

bool SocketIfListen(TSocketIf* pSocketIf)
{
    return true;
}

bool SocketIfCloseListen(TSocketIf* pSocketIf)
{
    return true;
}

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
    
    Sleep(2000);   //����������ʱ����Ϊ�п���TCPIP����ͨ������л�ģʽ���ܵ���ͨ�Ź���

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

