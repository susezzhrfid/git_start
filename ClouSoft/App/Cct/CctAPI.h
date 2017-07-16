/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctAPI.h
 * 摘    要：本文件主要实现低压集抄的公共接口
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2009年1月
 *********************************************************************************************************/
#ifndef CCTAPI_H
#define CCTAPI_H

#include "sysarch.h"
#include "CctHook.h"
#include "AutoReader.h"
#include "FaConst.h"
#include "StdReader.h"

bool IsCctOK(TStdReader* ptStdReader);

//描述:初始化
bool InitCct();
int CctDirectTransmit645Cmd(BYTE* pbCmdBuf, BYTE bCmdLen, BYTE* pbRet, BYTE* pbRetLen, BYTE bTimeOut);
bool CctReadFactoryCode();
bool InitCctPlc();
void GetCctAutoReader();

TThreadRet AutoReaderPlcThread(void* pvArg);
//描述:新建抄表线程
void NewCctThread();

#endif //CCTAPI_H
