/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֵ�ѹ�����Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2009��1��
 *********************************************************************************************************/
#ifndef CCTAPI_H
#define CCTAPI_H

#include "sysarch.h"
#include "CctHook.h"
#include "AutoReader.h"
#include "FaConst.h"
#include "StdReader.h"

bool IsCctOK(TStdReader* ptStdReader);

//����:��ʼ��
bool InitCct();
int CctDirectTransmit645Cmd(BYTE* pbCmdBuf, BYTE bCmdLen, BYTE* pbRet, BYTE* pbRetLen, BYTE bTimeOut);
bool CctReadFactoryCode();
bool InitCctPlc();
void GetCctAutoReader();

TThreadRet AutoReaderPlcThread(void* pvArg);
//����:�½������߳�
void NewCctThread();

#endif //CCTAPI_H
