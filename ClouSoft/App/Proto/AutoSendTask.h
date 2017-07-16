/*********************************************************************************************************
 * Copyright (c) 2014,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AutoSendTask.h
 * ժ    Ҫ�����ļ���Ҫʵ��������������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2014��9��
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
//��ͨ����������ݽṹ
#pragma   pack(1)
typedef struct{
	BYTE   bComTaskValid;		//������Ч
	BYTE   bComRptBasTime[5];	//�ϱ���׼ʱ��
	BYTE   bComRptIntervU;		//���ڵ�λ
	BYTE   bComRptIntervV;		//�ϱ�����
	BYTE   bComDataStru;		//���ݷ�ʽ
	BYTE   bComSampBasTime[5];	//������׼ʱ��
	BYTE   bComSampIntervU;		//�������ڵ�λ
	BYTE   bComSampIntervV;		//��������	
	BYTE   bComRatio;			//��ȡ����
	WORD   bComDoTaskTimes;		//����ִ�д���
}TComTaskCfg;
#pragma   pack()

//�м�����������ݽṹ
#pragma   pack(1)
typedef struct{
	BYTE   bFwdTaskValid;		//������Ч
	BYTE   bFwdRptBasTime[5];	//�ϱ���׼ʱ��
	BYTE   bFwdRptIntervU;		//���ڵ�λ
	BYTE   bFwdRptIntervV;		//�ϱ�����
//	BYTE   bDataStru;				//���ݷ�ʽ
	BYTE   bFwdSampBasTime[5];	//������׼ʱ��
	BYTE   bFwdSampIntervU;		//�������ڵ�λ
	BYTE   bFwdSampIntervV;		//��������	
	BYTE   bFwdRatio;			//��ȡ����
	BYTE   bFwdFwdType;			//�м�����
	WORD   bFwdDoTaskTimes;		//����ִ�д���
}TFwdTaskCfg;
#pragma   pack()

//typedef struct{
//	DWORD  dwLastClick;		//�ϴ�����ִ�е���
//	BYTE   bExcCnt;		//ִ�д���������
//}TRptCtrl;	//��ʱ�ϱ����ƽṹ

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
