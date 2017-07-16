/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ThreadMonitor.h
 * ժ    Ҫ�����ļ���Ҫʵ���̼߳��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef THREADMONITRO_H
#define THREADMONITRO_H
#include "TypeDef.h"
#include "Sysarch.h"

#define THRD_MNTR_NUM			6	//������߳�����,������Ҫ�����
#define THRD_NAME_LEN			4   //32 ����ȡ̫�����ڴ�,����ʵ��Ҳûʲô��

////////////////////////////////////////////////////////////////////////////////////////////
//ThreadMonitor������������
bool InitThreadMonitor();
int ReqThreadMonitorID(char* pszName, DWORD dwUdpInterv);
void ReleaseThreadMonitorID(int iID);
void UpdThreadRunClick(int iID);
int DoThreadMonitor();
bool GetMonitorThreadName(int iID, char* pszName);

#endif //THREADMONITRO_H