/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProAPI.h
 * ժ    Ҫ�����ļ���Ҫ����FaProtoĿ¼��API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef FROAPI_H
#define FROAPI_H
#include "ProStruct.h"
#include "Sysarch.h"

TThreadRet StdProThread(void* pvArg);
//TThreadRet LocalThread(void* pvArg);
void LocalThread();
bool GetMasterAddr(TMasterIp* pMasterIp);
void NewFaProThread();

void InitProto();
void NewProThread();

//extern BYTE g_bRxBuf[1024];
//extern BYTE g_bTxBuf[1024];

void NewPppThread();
BYTE GetGprsIfState(void);

#endif //FROAPI_H


