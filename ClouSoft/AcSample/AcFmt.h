/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcFmt.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֽ����������ֵ����ʽ��ת���ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 * ��    ע: ���ļ���Ҫ�������θ��汾������Ĳ�����
 *********************************************************************************************************/
#ifndef ACFMT_H
#define ACFMT_H
#include "Typedef.h"

bool InitAcValToDb(WORD wPn);
void AcValToDb(int* piVal);
void AcHarmonicToDb(WORD* pwHarPercent, WORD* pwHarVal);
bool InitPulseValToDb(BYTE bPnIndex);
void PulseValToDb(BYTE bPnIndex, int* piVal);

//���ɵ������ݸ�ʽ��ת��
WORD AcEpToFmt(int64 val, BYTE* pbBuf);
WORD AcFmtToEp(BYTE* pbBuf, int64* piVal);
WORD AcEqToFmt(int64 val, BYTE* pbBuf);
WORD AcFmtToEq(BYTE* pbBuf, int64* piVal);
WORD AcDemandToFmt(DWORD dwDemand, BYTE* pbBuf);
WORD AcFmtToDemand(BYTE* pbBuf, DWORD* pdwDemand);

#endif //ACFMT_H
