/*******************************************************************************
 * Copyright (c) 2012,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Sysarch.c
 * 摘    要：操作系统相关的一些接口（如信号量等）
 * 当前版本：1.0.0
 * 作    者：李焱
 * 完成日期：2012年2月
 * 备    注：
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

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );  //如果有优先级高的任务被唤醒，在中断退出后会触发一次SV中断，进行任务切换。注意这里不会中断嵌套。这里是M3的咬尾中断
/*    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        //taskYIELD();//在中断之后，强制进行任务切换 
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
    //Note:xTaskCreate之前不能关调度, xTaskCreate内部会更新当前任务控制块，并触发一次调度。如果新创建的线程优先级比目前执行的线程高，那么将导致系统出错。
    TskRet = xTaskCreate( (pdTASK_CODE)GeneralThread, ThreadName, nStackSize, pTskPara, prio, &xHandle );
    if (TskRet == 1)
    {//成功创建任务
        //g_iThreadCnt++;   
        pTskPara->tTaskHandle = xHandle;        
    }
    else if (TskRet == -1)
    {//任务创建失败，任务栈分配失败，任务栈使用malloc分配，需要将堆扩大  
        OsEnterCritical();	
        free(pTskPara);
        OsExitCritical();
        DTRACE(DB_CRITICAL, ("ThreadCreate : failed to create thread!\r\n"));//应该在调用OsExitCritical()之后
    }  
}

//描述：获取系统上电以来的毫秒数
//参数：无
//返回：系统上电以来的毫秒数
DWORD GetTick(void)
{    
    return xTaskGetTickCount();
}

//描述：获取系统上电以来的秒数
//参数：无
//返回：系统上电以来的秒数
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

//尽量不要使用。
//关掉所有中断
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

//关掉与操作系统API有关的中断，以及内核中断（PendSV与SYSTick)
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
//统计Malloc
void StatMallocSize(DWORD dwSize)
{
    g_dwMallocSize += dwSize;
}
#endif
