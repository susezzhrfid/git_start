/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProAPI.h
 * 摘    要：本文件主要包含FaProto目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：
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


