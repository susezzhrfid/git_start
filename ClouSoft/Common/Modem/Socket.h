/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Socket.h
 * ժ    Ҫ�����ļ�ʵ����socketͨ��,֧��GPRS�ն�Э��ջ
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3�� 
 *********************************************************************************************************/
#ifndef SOCKET_H
#define SOCKET_H
#include "ProIf.h"

typedef unsigned long ULONG;

#define INVALID_SOCKET -1

typedef struct{
	//����
	bool 	fSvr;	//�Ƿ��Ƿ�����ģʽ
	bool 	fUdp;	//�Ƿ���UDPͨ�ŷ�ʽ
	bool	fEnableFluxStat;	//�Ƿ�������������,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
	BYTE 	bBeatMin;		//�ͻ����������,��λ����
   
	//����
	int  iSocket;
	DWORD dwBeatClick;
	BYTE bSubState;		//�������״̬
	char cLastErr;		//���Ĵ���
}TSocket;	//Socket�ӿ�����

////////////////////////////////////////////////////////////////////////////////////////////
//Socket������������
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




