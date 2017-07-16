/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysarch.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֲ���ϵͳ�ӿڵķ�װ,���ź���,�̵߳�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��7��
 *********************************************************************************************************/
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__
#include "Typedef.h"
//#include "Sysdebug.h"
//#include "Sysapi.h"

#define SYS_ERR_OK       	0		//�޴���
#define SYS_ERR_INVALID  	1       //�������ݷǷ�
#define SYS_ERR_TIMEOUT  	0x20	//��ʱ
#define SYS_ERR_SYS      	3		//ϵͳ����

#define SYS_TO_INFINITE  	INFINITE		//��ʱ

typedef HANDLE TSem;

#define THREAD_RET_OK	0
#define THREAD_RET_ERR	1
typedef int TThreadRet;

typedef int TPrio;


TSem NewSemaphore(WORD count, WORD max);
void FreeSemaphore(TSem sem);
WORD WaitSemaphore(TSem sem, DWORD timeout);
void SignalSemaphore(TSem sem);
void NewThread(const signed char *ThreadName, TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize, TPrio prio);
//void Sleep(DWORD dwMilliseconds);

#endif //__ARCH_SYS_ARCH_H__ 

