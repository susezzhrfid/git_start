/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Dl645.h
 * ժ    Ҫ�����ļ�����97��645����Э�����ض���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef DL645_H
#define DL645_H

#include "MeterPro.h"

//97��645Э������ṹ����
typedef struct 
{
	//BYTE bSubPro; //��Э�����ú�		
	bool fRd901f;
	bool fRd9010;
	bool fRdSID;
	BYTE bAddrByte; //��ַ����6�ֽ�(12λ)������ֽ�	
}T645Priv;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;		
}T645Tmp;

//97��645Э�����ӿں�������
bool Mtr645Init(struct TMtrPro* pMtrPro, bool fInit, BYTE bThrId);
int DL645AskItem(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);//�����ݽӿ�
bool DL645RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //�������պ���
void DL645GetProPrintType(BYTE* pbPrintPro, char* pszProName);//��ȡ��ӡЭ������


#endif //DL645_H


