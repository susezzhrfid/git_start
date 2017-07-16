/*******************************************************************************
 * Copyright (c) 2012,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Sysarch.c
 * ժ    Ҫ������ϵͳ��ص�һЩ�ӿڣ����ź����ȣ�
 * ��ǰ�汾��1.0.0
 * ��    �ߣ�����
 * ������ڣ�2012��2��
 * ��    ע��
 ******************************************************************************/
#include "Sysarch.h"
#include "task.h"
#include "SysDebug.h"
#include <intrinsics.h>
#include "FaCfg.h"

#include "Drivers.h"

//xTaskHandle g_tTaskHandle[10] = { 0 };

TSem NewSemaphore(WORD count, WORD max)
{    
    return xSemaphoreCreateCounting(max, count); 
}

void FreeSemaphore(TSem sem)
{    
    if (sem == NULL)
        return;
    vSemaphoreDelete( sem );
}

WORD WaitSemaphore(TSem sem, DWORD dwMilliseconds)
{
    WORD wRet = 0;
    if (sem == NULL)
        return SYS_ERR_SYS;
    if (dwMilliseconds == 0)
        dwMilliseconds = portMAX_DELAY;    //block indefinitely
    else if (dwMilliseconds == SYS_GET_SEM_NO_WAIT)
        dwMilliseconds = 0;                //poll the semaphore
    wRet = xSemaphoreTake( sem, dwMilliseconds );
    if (wRet == 1)
        return SYS_ERR_OK;
    else 
        return SYS_ERR_TIMEOUT;
}

void SignalSemaphore(TSem sem)
{
    if (sem == NULL)
        return;
    xSemaphoreGive(sem);
}

void SignalSemaphoreFromISR(TSem sem)
{
    static signed portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    if (sem == NULL)
        return;
    xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );  //��������ȼ��ߵ����񱻻��ѣ����ж��˳���ᴥ��һ��SV�жϣ����������л���ע�����ﲻ���ж�Ƕ�ס�������M3��ҧβ�ж�
/*    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        //taskYIELD();//���ж�֮��ǿ�ƽ��������л� 
    }*/
}

void Sleep(DWORD dwMilliseconds)
{
    vTaskDelay((portTickType) dwMilliseconds);
}

void DelayUntil(DWORD * const pdwPreviousWakeTime, DWORD dwTimeIncrement )
{
    vTaskDelayUntil((portTickType *)pdwPreviousWakeTime, (portTickType)dwTimeIncrement );   
}

typedef void (*TSKFunc)(void* pvArg);
typedef struct 
{
	void *pvArg;
	TSKFunc pFunc;
	xTaskHandle tTaskHandle;
} TTSKPara;

TThreadRet GeneralThread(void* pvPara)
{
	TTSKPara* pTskPara = (TTSKPara *)pvPara;
	
	pTskPara->pFunc(pTskPara->pvArg);    
       
	vTaskDelete(pTskPara->tTaskHandle);
    
    OsEnterCritical();	
    free(pvPara);    
	OsExitCritical();
    
    DTRACE(DB_CRITICAL, ("Thread exit!\r\n"));
    
	return 0;
}

//int g_iThreadCnt = 0;

//volatile static DWORD g_mallocCnt1 = 0;
//volatile static DWORD g_mallocNum1 = 0;

void NewThread(const signed char *ThreadName, TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize, TPrio prio)
{   
    xTaskHandle xHandle;
    signed long TskRet = 0;
        
    TTSKPara *pTskPara = NULL;
    
    OsEnterCritical();	
    pTskPara = (TTSKPara *)malloc(sizeof(TTSKPara));
    OsExitCritical();
#ifndef NDEBUG    
    StatMallocSize(sizeof(TTSKPara));
#endif
    //g_mallocCnt1++;
    //g_mallocNum1 += sizeof(TTSKPara);
    
    if (pTskPara == NULL)
    {        
        DTRACE(DB_CRITICAL, ("ThreadCreate : failed to malloc pTskPara!\r\n"));
        return;
    }
    pTskPara->pvArg = arg;
    pTskPara->pFunc = (TSKFunc)function;
    //Note:xTaskCreate֮ǰ���ܹص���, xTaskCreate�ڲ�����µ�ǰ������ƿ飬������һ�ε��ȡ�����´������߳����ȼ���Ŀǰִ�е��̸߳ߣ���ô������ϵͳ����
    TskRet = xTaskCreate( (pdTASK_CODE)GeneralThread, ThreadName, nStackSize, pTskPara, prio, &xHandle );
    if (TskRet == 1)
    {//�ɹ���������
        //g_iThreadCnt++;   
        pTskPara->tTaskHandle = xHandle;        
    }
    else if (TskRet == -1)
    {//���񴴽�ʧ�ܣ�����ջ����ʧ�ܣ�����ջʹ��malloc���䣬��Ҫ��������  
        OsEnterCritical();	
        free(pTskPara);
        OsExitCritical();
        DTRACE(DB_CRITICAL, ("ThreadCreate : failed to create thread!\r\n"));//Ӧ���ڵ���OsExitCritical()֮��
    }  
}

//��������ȡϵͳ�ϵ������ĺ�����
//��������
//���أ�ϵͳ�ϵ������ĺ�����
DWORD GetTick(void)
{    
    return xTaskGetTickCount();
}

//��������ȡϵͳ�ϵ�����������
//��������
//���أ�ϵͳ�ϵ�����������
DWORD GetClick(void)
{
    DWORD dwClick;    
    dwClick = xTaskGetTickCount()/1000;
    return dwClick;
}

/*WORD g_wTicks = 0;
WORD GetMillSec(void)
{
	return (WORD)(g_wTicks%1000);
}

void SetMillSec(WORD wMs)
{
    g_wTicks = wMs;
}*/

//������Ҫʹ�á�
//�ص������ж�
void EnterCritical()
{
    ClearWDG();
    __disable_interrupt();    
}

void ExitCritical()
{
    ClearWDG();
    __enable_interrupt();
}

//�ص������ϵͳAPI�йص��жϣ��Լ��ں��жϣ�PendSV��SYSTick)
void OsEnterCritical()
{
    ClearWDG();
    taskENTER_CRITICAL();
}

void OsExitCritical()
{    
    ClearWDG();
    taskEXIT_CRITICAL();
}

#ifndef NDEBUG
static DWORD g_dwMallocSize = 0;
//ͳ��Malloc
void StatMallocSize(DWORD dwSize)
{
    g_dwMallocSize += dwSize;
}
#endif
