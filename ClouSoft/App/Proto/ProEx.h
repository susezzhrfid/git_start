#ifndef PROEX_H
#define PROEX_H

#include "DbConst.h"
#include "Comm.h"

#ifdef EN_PROEX

#define MAX_645_BUF				256
#define MAX_ADD_TYPE			14
#define FAP_FRM_SIZE_645 		MAX_645_BUF

#define ADD645MAXEVT			57
#define MAX_MTR_EXC_ITEM		5

#define EXC_DOOR_IDX			53		//�˸��¼����
#define EXC_REAR_IDX			54		//β���¼����

#define FAP_CMD_645				8
#define FAP_LEN_645				9
#define FAP_DATA_645			10


#define FAP_CMD_GET_645			0x3f
#define FAP_CMD_UP_645			0x80
#define FAP_CMD_DIR_645 		0x80
#define FAP_CMD_DOWN_645		0x00
#define FAP_FIX_LEN_645			12

#define	MAX_PARA_ID					13		//sizeof(g_ModuleParaDesc)/sizeof(TModuleParaDesc);   10
#define	Module_PARA_LEN				173		//�洢�ڵ���е�ģ���������   80


#define ADDTYPEID_READ				12//��ģ�����
#define ADDTYPEID_WRITE				13//дģ�����
#define ADDTYPEID_PHASE				14//���

typedef struct 
{
	BYTE fInit;
	BYTE fNeedSend;
	BYTE SendErr;
}TSendInfo;

typedef struct{
	WORD wBn;
	WORD wID;
	WORD wLen;
	WORD wOffset;	//�ڵ���д洢��ƫ��λ��
	BYTE bFn;		//�ò�����Ӧ��Fn��Ϊ0��ʾ��ЧFN
	BYTE bPn;       //�������
}TModuleParaDesc;

typedef struct{
	BYTE bMStep;
	BYTE bGprsSignal;
	BYTE bGprsConnectOld;
	DWORD dwSignalClick;
	BYTE bInMtrAddr[6];
	DWORD dwNeedSendFlag;
	TCommPara* pCommPara;
	TSendInfo SendInfo[MAX_ADD_TYPE+1];
	TSendInfo SendPara[MAX_ADD_TYPE+1];
	BYTE bEvtNum[ADD645MAXEVT];
	BYTE bNewEvtNum[ADD645MAXEVT];
	BYTE bRxBuf[MAX_645_BUF];
	BYTE bTxBuf[MAX_645_BUF];	
}TPro645Ex;

typedef struct{
	BYTE bErc;
	BYTE bItemNum;
	WORD wIDs[MAX_MTR_EXC_ITEM];
}TMtrExcDesc;


extern void SetInMtrParaChg();
void DoFaProtoEx(TPro645Ex* pProEx);
bool ReadInMtrTime(TPro645Ex* pProEx);
extern void SetSendParaFlag(TPro645Ex* pProEx, BYTE bIdx);
#endif
#endif		//PROEX_H