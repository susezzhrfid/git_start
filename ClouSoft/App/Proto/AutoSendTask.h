/*********************************************************************************************************
 * Copyright (c) 2014,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AutoSendTask.h
 * 摘    要：本文件主要实现主动上送任务
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2014年9月
*********************************************************************************************************/
#ifndef AUTOSENDTASK_H
#define AUTOSENDTASK_H
#include "GbPro.h"
#include "ComStruct.h"

#define	COMMON_TASK_TYPE	1
#define	FWD_TASK_TYPE		2

#define	MAX_COMMON_TASK		254
#define	MAX_FWD_TASK		254

#define BASE_TIME_DELAY		5

/////////////////////////////////////////////////////////////////
//普通任务参数数据结构
#pragma   pack(1)
typedef struct{
	BYTE   bComTaskValid;		//任务有效
	BYTE   bComRptBasTime[5];	//上报基准时间
	BYTE   bComRptIntervU;		//周期单位
	BYTE   bComRptIntervV;		//上报周期
	BYTE   bComDataStru;		//数据方式
	BYTE   bComSampBasTime[5];	//采样基准时间
	BYTE   bComSampIntervU;		//采样周期单位
	BYTE   bComSampIntervV;		//采样周期	
	BYTE   bComRatio;			//抽取倍率
	WORD   bComDoTaskTimes;		//任务执行次数
}TComTaskCfg;
#pragma   pack()

//中继任务参数数据结构
#pragma   pack(1)
typedef struct{
	BYTE   bFwdTaskValid;		//任务有效
	BYTE   bFwdRptBasTime[5];	//上报基准时间
	BYTE   bFwdRptIntervU;		//周期单位
	BYTE   bFwdRptIntervV;		//上报周期
//	BYTE   bDataStru;				//数据方式
	BYTE   bFwdSampBasTime[5];	//采样基准时间
	BYTE   bFwdSampIntervU;		//采样周期单位
	BYTE   bFwdSampIntervV;		//采样周期	
	BYTE   bFwdRatio;			//抽取倍率
	BYTE   bFwdFwdType;			//中继类型
	WORD   bFwdDoTaskTimes;		//任务执行次数
}TFwdTaskCfg;
#pragma   pack()

//typedef struct{
//	DWORD  dwLastClick;		//上次任务执行的秒
//	BYTE   bExcCnt;		//执行次数计数器
//}TRptCtrl;	//定时上报控制结构

void TraceSecsToTime(char *pStr, DWORD dwSecs);
bool IsTaskExist(BYTE bTaskType, BYTE bTaskNo);

DWORD GetComTaskRecNum(BYTE bTaskNo);
DWORD GetFwdTaskRecNum(BYTE bTaskNo);
bool  GetOneTaskDataPFnNum(BYTE bTaskNo, WORD* pwPnNum, WORD* pwFnNum);

int   GetComTaskPerDataLen(BYTE bTaskNo);
int   GetFwdTaskPerDataLen(BYTE bTaskNo);

BYTE  GetComTaskSmplIntervU(BYTE bTaskNo);
BYTE  GetFwdTaskSmplIntervU(BYTE bTaskNo);

BYTE  GetComTaskSmplInterV(BYTE bTaskNo);
BYTE  GetFwdTaskSmplInterV(BYTE bTaskNo);

bool  IsTaskValid(BYTE bTaskType, BYTE bTaskNo);
void  FwdToComTaskPara(TComTaskCfg* pComTaskRptCfg, TFwdTaskCfg* pFwdTaskRptCfg);

bool  DoRptTask(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro);

bool  GetRptTimeScope(TComTaskCfg* pCfg, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime);
void  DoRptTaskData(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro, TComTaskCfg* pCfg, DWORD dwStartTime, DWORD dwEndTime);

int   MakeComTaskReqRptFrm(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro, TComTaskCfg* pCfg, DWORD dwStepTime, DWORD dwStartTime);
bool  GetOneFrmStartEndTime(BYTE bTaskType, BYTE bTaskNo, DWORD* pdwStepTime, DWORD dwStartTime, DWORD dwEndTime, TComTaskCfg* pCfg, bool* pfOnce);

#endif //AUTOSEND_H
