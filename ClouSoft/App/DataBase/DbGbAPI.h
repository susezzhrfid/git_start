/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbGbAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�����ݽṹ����
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef DBGBAPI_H      
#define DBGBAPI_H      
#include "TypeDef.h"
#include "FaCfg.h"
#include "LibDbStruct.h"

#define GB_RDERR_FAIL		1		//��ʧ��
#define GB_RDERR_NOROOM		2		//�ռ䲻��,һ������������󣬶�������û���κ���Ч������

int SgReadClass1(DWORD dwId, WORD wPn, BYTE* pbTx, int iTxBufSize, bool fRptState);
int SgReadClass2(DWORD dwID, WORD wPn, BYTE* pbRx, BYTE* pbTx, int iTxBufSize, WORD* pwStart, bool fRptState);
//int SgReadClass3(DWORD dwId, WORD wPn, DWORD dwStartm, DWORD dwEndtm, BYTE* pbBuf, BYTE* bRdNum, int iTxBufLeft);
int SgReadClass3(DWORD dwId, WORD wPn, DWORD dwStartm, DWORD dwEndtm, BYTE* pbBuf, WORD* wRdNum, WORD wTotal, int iTxBufLeft);

#endif //DBGBAPI_H