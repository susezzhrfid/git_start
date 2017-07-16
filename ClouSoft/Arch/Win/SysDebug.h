/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysdebug.h
 * ժ    Ҫ�����ļ���Ҫ������ϵͳ�µ��ԽӿڵĶ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע��
 *********************************************************************************************************/
#ifndef SYSDEBUG_H
#define SYSDEBUG_H
#include <stdlib.h>
#include <stdio.h>
#include "TypeDef.h"

//��������ӿڵĶ���
extern void TraceOut(fmt, ...);
extern bool IsDebugOn(BYTE bType);

//��������ʼ�������������
//��������
//���أ�true-�ɹ�,false-ʧ��
bool SysDebugInit();

#define DTRACE(debug, x) do { if (IsDebugOn(debug)){ TraceOut x;}} while(0)	
#define STRACE(debug, s, len) do { if (IsDebugOn(debug)){ DTRACEOUT(s, len);}} while(0)
void DTRACEOUT(char* s, int len);

#endif //SYSDEBUG_H