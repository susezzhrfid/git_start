/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Sysarch.h
 * 摘    要：操作系统相关的一些接口（如信号量等）
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#ifndef SYSARCH_H
#define SYSARCH_H
#include <stdlib.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "Typedef.h"
#include "task.h"

#define SYS_ERR_OK       	0		//无错误
#define SYS_ERR_INVALID  	1       //设置内容非法
#define SYS_ERR_TIMEOUT  	0x20	//超时
#define SYS_ERR_SYS      	3		//系统出错

#define SYS_TO_INFINITE  	0		//超时
#define SYS_GET_SEM_NO_WAIT 0xffffffffUL  //有信号量则取，没有信号量不等

#define THREAD_RET_OK	0
#define THREAD_RET_ERR	1

#define THREAD_PRIORITY_IDLE			0
#define THREAD_PRIORITY_LOWEST			1
#define THREAD_PRIORITY_BELOW_NORMAL    2
#define THREAD_PRIORITY_NORMAL          3
#define THREAD_PRIORITY_ABOVE_NORMAL    4      //最高
//#define THREAD_PRIORITY_HIGHEST         5   
//#define THREAD_PRIORITY_TIME_CRITICAL   6

//typedef xQUEUE * TSem;
#define TSem xQueueHandle
typedef int TPrio;
typedef int TThreadRet;

//描述：创建信号量
//参数：@count - 初始数量
//      @max - 最大可用数量
//返回：0 - 错误, 其它-信号量的句柄
//#define NewSemaphore(count, max) xSemaphoreCreateCounting( max, count )
TSem NewSemaphore(WORD count, WORD max);    //创建一个信号量需要81个字节的内存空间

//描述：销毁信号量
//参数：@sem - 欲释放的信号量句柄
//返回：无
//#define FreeSemaphore(sem) vSemaphoreDelete((xQueueHandle) sem )
void FreeSemaphore(TSem sem);

//描述：等待信号量
//参数：@sem - 信号量句柄
//      @dwMilliseconds - 等待超时时间, 0 - 永远等待，不超时
//返回：SYS_ERR_OK-等到指定信号量, SYS_ERR_TIMEOUT-超时
//#define WaitSemaphore(sem, dwMilliseconds) xSemaphoreTake( sem, dwMilliseconds )
WORD WaitSemaphore(TSem sem, DWORD dwMilliseconds);

//描述：释放信号量
//参数：@sem - 信号量句柄
//返回：无
//#define SignalSemaphore(sem) xSemaphoreGive(sem) //This macro must not be used from an ISR
void SignalSemaphore(TSem sem);

//描述：从中断中释放信号量
//参数：@sem - 信号量句柄
void SignalSemaphoreFromISR(TSem sem);

//描述：本线程休眠一段时间
//参数：@dwMilliseconds - 休眠毫秒数
//返回：无
//#define Sleep( dwMilliseconds ) vTaskDelay((portTickType) dwMilliseconds)
void Sleep(DWORD dwMilliseconds);

//描述：按照设置的时间间隔执行线程
//参数：@pdwPreviousWakeTime - 初始化成当前时间的TICK
//      @dwTimeIncrement - 执行间隔。
void DelayUntil(DWORD * const pdwPreviousWakeTime, DWORD dwTimeIncrement );

//描述：创建一个线程
//参数：@function - 线程的执行函数
//      @arg - 传给线程执行函数的参数
//      @nStackSize - 线程栈空间, 以字为单位。
//      @prio - 线程优先级
//返回：无
void NewThread(const signed char *ThreadName, TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize, TPrio prio);


DWORD GetTick(void);

DWORD GetClick(void);

void EnterCritical();        //调用之后，将停止调度与中断，因此不能再出现等信号量

void ExitCritical();

void OsEnterCritical();      //调用之后，将停止调度与中断，因此不能再出现等信号量

void OsExitCritical();      

#ifndef NDEBUG
void StatMallocSize(DWORD dwSize);
#endif//NDEBUG
#endif
