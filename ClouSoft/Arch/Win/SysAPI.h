 /*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysapi.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԲ�ͬϵͳ��API�ӿں����ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��9��
*********************************************************************************************************/
#ifndef SYSAPI_H
#define SYSAPI_H
#include "ComStruct.h"

DWORD GetTick();
DWORD GetClick();
bool GetSysTime(TTime* pTime);
bool SetSysTime(const TTime* pTime);
void SyncTimer();
void SysInit();
void SysExit();

WORD GetMillSec(void);
void SetMillSec(WORD wMs);

#endif //SYSAPI_H

