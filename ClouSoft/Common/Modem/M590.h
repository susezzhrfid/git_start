/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�M590.h
 * ժ    Ҫ�����ļ�ʵ����GC864MODEM����
 * ��ǰ�汾��1.0
 * ��    �ߣ������᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef GC864_H
#define GC864_H
#include "Modem.h"

typedef struct{	
		
	//����
	BYTE bSvrSock;	//�������˵�socket�ţ�ֻ��Ϊ0~1����ֵ0xff��ʾ��Ч��
	BYTE bCliSock;	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
	bool fPowOn;	//�Ƿ��Ǹ��ϵ�
	bool fCliUdp;	//�ͻ����Ƿ���UDP����
    
    bool fDataUpload;  //�������ϱ�
    bool fLinkErr;     //����Ͽ�
    bool fInitiativeErr;   //�����ж�ģ������Ͽ�
}TM590;


////////////////////////////////////////////////////////////////////////////////////////////
//M590������������
void M590Init(struct TModem* pModem);

////////////////////////////////////////////////////////////////////////////////////////////
//M590˽�к�������
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









 