/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DL645V07.h
 * ժ    Ҫ�����ļ�����07��645����Э�����ض���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef DL645V07_H
#define DL645V07_H
#include "MeterPro.h"

typedef struct
{
	WORD wDL645Id;
	DWORD dwProId;
	WORD wDL645Len;
	WORD wProLen;	
}TItemList;

typedef struct
{
	TTime tmStart;
	BYTE bNum;	
}TRdLoadInfo;

typedef struct 
{
	BYTE bSubPro; //��Э�����ú�			
}TV07Priv;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;
	BYTE bRdNextSeq;
	bool fRdNext;
	TRdLoadInfo tRdLoadInfo;
}TV07Tmp;


//07��645Э�����ӿں�������
bool Mtr645V07Init(struct TMtrPro* pMtrPro, bool fInit, BYTE bThrId);
int DL645V07AskItem(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);//�����ݽӿ�
bool DL645V07RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //�������պ���
void DL645V07GetProPrintType(BYTE* pbPrintPro, char* pszProName);//��ȡ��ӡЭ������

int MtrReadFrz(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, WORD wID, BYTE* pbBuf, WORD wSubIdx);

#endif //DL645V07_H


