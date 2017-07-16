/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Sysarch.h
 * ժ    Ҫ������ϵͳ��ص�һЩ�ӿڣ����ź����ȣ�
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#ifndef SYSARCH_H
#define SYSARCH_H
#include <stdlib.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "Typedef.h"
#include "task.h"

#define SYS_ERR_OK       	0		//�޴���
#define SYS_ERR_INVALID  	1       //�������ݷǷ�
#define SYS_ERR_TIMEOUT  	0x20	//��ʱ
#define SYS_ERR_SYS      	3		//ϵͳ����

#define SYS_TO_INFINITE  	0		//��ʱ
#define SYS_GET_SEM_NO_WAIT 0xffffffffUL  //���ź�����ȡ��û���ź�������

#define THREAD_RET_OK	0
#define THREAD_RET_ERR	1

#define THREAD_PRIORITY_IDLE			0
#define THREAD_PRIORITY_LOWEST			1
#define THREAD_PRIORITY_BELOW_NORMAL    2
#define THREAD_PRIORITY_NORMAL          3
#define THREAD_PRIORITY_ABOVE_NORMAL    4      //���
//#define THREAD_PRIORITY_HIGHEST         5   
//#define THREAD_PRIORITY_TIME_CRITICAL   6

//typedef xQUEUE * TSem;
#define TSem xQueueHandle
typedef int TPrio;
typedef int TThreadRet;

//�����������ź���
//������@count - ��ʼ����
//      @max - ����������
//���أ�0 - ����, ����-�ź����ľ��
//#define NewSemaphore(count, max) xSemaphoreCreateCounting( max, count )
TSem NewSemaphore(WORD count, WORD max);    //����һ���ź�����Ҫ81���ֽڵ��ڴ�ռ�

//�����������ź���
//������@sem - ���ͷŵ��ź������
//���أ���
//#define FreeSemaphore(sem) vSemaphoreDelete((xQueueHandle) sem )
void FreeSemaphore(TSem sem);

//�������ȴ��ź���
//������@sem - �ź������
//      @dwMilliseconds - �ȴ���ʱʱ��, 0 - ��Զ�ȴ�������ʱ
//���أ�SYS_ERR_OK-�ȵ�ָ���ź���, SYS_ERR_TIMEOUT-��ʱ
//#define WaitSemaphore(sem, dwMilliseconds) xSemaphoreTake( sem, dwMilliseconds )
WORD WaitSemaphore(TSem sem, DWORD dwMilliseconds);

//�������ͷ��ź���
//������@sem - �ź������
//���أ���
//#define SignalSemaphore(sem) xSemaphoreGive(sem) //This macro must not be used from an ISR
void SignalSemaphore(TSem sem);

//���������ж����ͷ��ź���
//������@sem - �ź������
void SignalSemaphoreFromISR(TSem sem);

//���������߳�����һ��ʱ��
//������@dwMilliseconds - ���ߺ�����
//���أ���
//#define Sleep( dwMilliseconds ) vTaskDelay((portTickType) dwMilliseconds)
void Sleep(DWORD dwMilliseconds);

//�������������õ�ʱ����ִ���߳�
//������@pdwPreviousWakeTime - ��ʼ���ɵ�ǰʱ���TICK
//      @dwTimeIncrement - ִ�м����
void DelayUntil(DWORD * const pdwPreviousWakeTime, DWORD dwTimeIncrement );

//����������һ���߳�
//������@function - �̵߳�ִ�к���
//      @arg - �����߳�ִ�к����Ĳ���
//      @nStackSize - �߳�ջ�ռ�, ����Ϊ��λ��
//      @prio - �߳����ȼ�
//���أ���
void NewThread(const signed char *ThreadName, TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize, TPrio prio);


DWORD GetTick(void);

DWORD GetClick(void);

void EnterCritical();        //����֮�󣬽�ֹͣ�������жϣ���˲����ٳ��ֵ��ź���

void ExitCritical();

void OsEnterCritical();      //����֮�󣬽�ֹͣ�������жϣ���˲����ٳ��ֵ��ź���

void OsExitCritical();      

#ifndef NDEBUG
void StatMallocSize(DWORD dwSize);
#endif//NDEBUG
#endif
