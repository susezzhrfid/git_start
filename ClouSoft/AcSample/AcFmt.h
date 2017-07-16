/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcFmt.h
 * 摘    要：本文件主要实现交采数据项的值到格式的转换接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
#ifndef ACFMT_H
#define ACFMT_H
#include "Typedef.h"

bool InitAcValToDb(WORD wPn);
void AcValToDb(int* piVal);
void AcHarmonicToDb(WORD* pwHarPercent, WORD* pwHarVal);
bool InitPulseValToDb(BYTE bPnIndex);
void PulseValToDb(BYTE bPnIndex, int* piVal);

//交采电能数据格式的转换
WORD AcEpToFmt(int64 val, BYTE* pbBuf);
WORD AcFmtToEp(BYTE* pbBuf, int64* piVal);
WORD AcEqToFmt(int64 val, BYTE* pbBuf);
WORD AcFmtToEq(BYTE* pbBuf, int64* piVal);
WORD AcDemandToFmt(DWORD dwDemand, BYTE* pbBuf);
WORD AcFmtToDemand(BYTE* pbBuf, DWORD* pdwDemand);

#endif //ACFMT_H
