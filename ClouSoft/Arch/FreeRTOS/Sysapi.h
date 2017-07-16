/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Sysapi.h
 * ժ    Ҫ��ϵͳ��ص�һЩAPI
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#ifndef SYSAPI_H
#define SYSAPI_H
#include "Typedef.h"
#include "ComStruct.h"

bool GetSysTime(TTime* pTime);

bool SetSysTime(const TTime* pTime);

void SyncTimer();

//������ϵͳ��ʼ��
//��������
//���أ���
void SysInit();

//������ϵͳ����
//��������
//���أ���
void SysExit();

#endif
