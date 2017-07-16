/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�PoIf.h
 * ժ    Ҫ�����ļ�ʵ����ͨ�Žӿڻ��ඨ��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef PROTOIF_H
#define PROTOIF_H

#include "TypeDef.h"
#include "sysarch.h"
#include "Pro.h"
#include "ProIfCfg.h"

#define GPRS_MAX_BYTES     	1024       
#define SOCK_MAX_BYTES      GPRS_MAX_BYTES	//��GPRS��ͬ���÷�������
#define COMM_MAX_BYTES      512	
#define SMS_MAX_BYTES       140
#define ETHER_MAX_BYTES     1024
#define PPP_MAX_BYTES       1024
#define WIRELESS_MAX_BYTES  256	

//�ӿ�����
#define IF_UNKNOWN		0
#define IF_GPRS         1
#define IF_COMM     	2
#define IF_SOCKET       3	//���ڲ���ϵͳsocket�׽���
#define IF_R230M		4	//230M��̨
#define IF_WIRELESS    	5   //С����

#define IF_RST_OK  		0  	//��λ�ɹ�
#define IF_RST_HARDFAIL 1	//Ӳ��λʧ��
#define IF_RST_SOFTFAIL 2	//��λʧ��(Э���)

//�ӿ�״̬��,�ӿڵ�״̬�л�: (����)->(��λ)->(����)->(��¼)->(����)
#define IF_STATE_DORMAN  	0 //����
#define IF_STATE_RST  		1 //��λ
//#define IF_STATE_CONNECT 	2 //����
//#define IF_STATE_LOGIN  	3 //��¼
#define IF_STATE_TRANS  	4 //����	

#define IF_DEBUG_INTERV		(2*60)	//��������ļ��,��λ��

struct TProIf{
	//�ⲿ�������õĲ���
	void* pvIf;				//ͨ�Žӿ���������,����bIfType�Ĳ�ͬ��ָ��TGprsIf��TCommIf��ʵ��
	struct TPro* pPro;		//ͨ��Э��

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	bool (* pfnDorman)(struct TProIf* pProIf);			//����
	bool (* pfnReset)(struct TProIf* pProIf);			//��λ�ӿ�
	bool (* pfnSend)(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen);			//���ͺ���
	int (* pfnReceive)(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize);	//���պ���,int֧�ִ��󷵻�
	void (* pfnTrans)(struct TProIf* pProIf);			//����״̬����
	void (* pfnOnResetOK)(struct TProIf* pProIf);	//�ӿڸ�λ�ɹ�ʱ�Ļص�����
	void (* pfnOnResetFail)(struct TProIf* pProIf);	//�ӿڸ�λʧ��ʱ�Ļص�����
	void (* pfnOnRcvFrm)(struct TProIf* pProIf);	//�յ�����֡ʱ�Ļص�����:��Ҫ������·״̬,����������
	void (* pfnDoIfRelated)(struct TProIf* pProIf);	//�ӿ�������⴦����
   	void (* LoadUnrstPara)(struct TProIf* pProIf);	//�ӿ���طǸ�λ��������

	//����
	char* pszName;		//�ӿ����ƣ�***�����Ҫ�ı�Ĭ��ֵ�����ڵ�����Ӧ�ӿڵĳ�ʼ��������ֱ�Ӹ�ֵ�޸�****
	WORD wMaxFrmBytes;	//���֡���ȣ�***�����Ҫ�ı�Ĭ��ֵ�����ڵ�����Ӧ�ӿڵĳ�ʼ��������ֱ�Ӹ�ֵ�޸�****
	BYTE bIfType;		//�ӿ�����
	DWORD dwRxClick;	//���һ�ν��յ����ĵ�ʱ��
	BYTE  bState;   	//�ӿ�״̬��:����->��λ(MODEM��λ��PPP����)->����(�ͻ���״̬��������->��½->ͨ��->�����Ͽ�->����)
    bool fExit;			//�Ƿ�Ҫ�˳�
    bool fExitDone;		//�˳��Ƿ����    
};	//ͨ�Žӿڻ���ṹ


////////////////////////////////////////////////////////////////////////////////////////////
//ProIf������������
void ProIfInit(struct TProIf* pProIf);

void SetMaxFrmBytes(struct TProIf* pProIf, WORD wMaxFrmBytes);

#endif //PROTOIF_H
 
