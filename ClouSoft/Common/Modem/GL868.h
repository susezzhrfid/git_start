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
#include "socket.h"

typedef struct{
	//BYTE bModuleVer;
	TSocket *pSocket;
    
	//����
	BYTE bSvrSock;	//�������˵�socket�ţ�ֻ��Ϊ0~1����ֵ0xff��ʾ��Ч��
	BYTE bCliSock;	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
	bool fPowOn;	//�Ƿ��Ǹ��ϵ�
	bool fCliUdp;	//�ͻ����Ƿ���UDP����
}TGL868;


////////////////////////////////////////////////////////////////////////////////////////////
//M590������������
void GL868Init(struct TModem* pModem);

////////////////////////////////////////////////////////////////////////////////////////////
//M590˽�к�������
int GL868ResetModem(struct TModem* pModem);
int GL868InitAPN(struct TModem* pModem);

bool GL868Connect(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort);
bool GL868Listen(struct TModem* pModem, bool fUdp, WORD wLocalPort);
bool GL868CloseCliSock(struct TModem* pModem);
bool GL868CloseSvrSock(struct TModem* pModem);
bool GL868CloseListen(struct TModem* pModem);

bool GL868ClosePpp(struct TModem* pModem);
bool GL868OpenPpp(struct TModem* pModem);

int GL868SvrReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize);
int GL868CliReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize);

bool GL868ChkCliStatus(struct TModem* pModem);
bool GL868ChkSvrStatus(struct TModem* pModem);	

int GL868CliSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen);
int GL868SvrSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen);

bool GL868SpecHandle(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize);
bool GL868IsSvrAcceptOne(struct TModem* pModem);

char* GL868IsRxIpFrm(struct TModem* pModem, char* pszRx, bool* pfSvr, WORD* pwLen);
extern DWORD GetLocalAddr();

#endif  //GC864_H









 