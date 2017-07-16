/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�SocketIf.h
 * ժ    Ҫ�����ļ�ʵ����socketͨ�Žӿ���,ֻ��WINDOWSƽ̨֧�ֱ��ӿ�, �ն�Э��ջ֧�ֱ��ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��

 *********************************************************************************************************/
#ifndef SOCKETIF_H
#define SOCKETIF_H
#include "ProIf.h"

typedef struct{
	//����
	bool 	fSvr;	//�Ƿ��Ƿ�����ģʽ
	bool 	fUdp;	//�Ƿ���UDPͨ�ŷ�ʽ
	bool	fEnableFluxStat;	//�Ƿ�������������,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
    BYTE    bCnType;
    
	//����
	int  iSocket;
	char cLastErr;		//���Ĵ���

	bool fBakIP;
	WORD wIPUseCnt;
	//WORD wConnectFailCnt;	//����ʧ�ܴ���
	WORD wConnectNum;	//����ʧ���������ԵĴ���
}TSocketIf;	//Socket�ӿ�����

////////////////////////////////////////////////////////////////////////////////////////////
//SocketIf������������
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




