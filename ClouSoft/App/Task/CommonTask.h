/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CommonTask.h
 * ժ    Ҫ�����ļ���Ҫʵ���������ݿ���ͨ��������ݲɼ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef COMMONTASK_H
#define COMMONTASK_H
#include "TypeDef.h"
#include "TaskStruct.h"
#include "TaskConst.h"
#include "ComStruct.h"

////////////////////////////////////////////////////////////////////////////////////////////
////Sg////////////////
#define SG_TASK_COMMON			3
#define SG_TASK_FW				4
#define SG_TASK_ID				0xE0000000
#define SG_TASK_CONTROL		0xCB

#define SG_GET_TASK_TYPE			8
#define SG_DWORD_GET_LOW_BYTE		0x000000ff
#define SG_DWORD_GET_LOW_WORD	0x0000ffff
#define SG_WORD_GET_LOW_BYTE		0x00ff

#define SG_COMMTASK_CFG_FIXLEN	20
#define SG_FWTASK_CFG_FIXLEN	26
#define SG_COMMTASK_REC_FIXLEN	19
#define SG_FWTASK_REC_FIXLEN	17

#define SG_BYTE_PN				2
#define SG_BYTE_DI				4
#define SG_BYTE_DI_GROUP		1

#define SG_BYTE_DATE_TIME		5
#define SG_FRM_DATE_TIME		6

#define TASK_TYPE_COMMON   		  1
#define TASK_COMMON_GROUP 		  2
#define TASK_TYPE_FW       		  2
#define TASK_TYPE_EXC      		  4
#define SURVEY_INTERV 			 15

#define EXCTASK_CFG_FIXLEN     	  8
#define COMMTASK_CFG_FIXLEN      16
#define FWTASK_CFG_FIXLEN     	 21

#define COMMTASK_REC_TIME_LEN	  5
#define COMMTASK_REC_MAXLEN       512   //��ͨ�����ÿ�ʼ�¼����󳤶�

#define SG_TASK_RDMTR			2
#define SG_RDMTR_CFG_FIXLEN		7
#define SG_TASK_LEN				510

#define CURVE_PER_DAY			4	//�������ݰ�6��Сʱһ�ʼ�¼���棬һ���4�ʼ�¼

BYTE GetRdMtrTaskValidNum();
////Sg///////////////
//ComTask������������
bool DoComTask(WORD wPn, TTime* pTm, DWORD* pdwRecTime, int Vip_Num, BYTE bVip, BYTE bType);
const TCommTaskCtrl* ComTaskFnToCtrl(BYTE bFn);	//ͨ��FNȡ�ü�¼���ƽṹ
const TCommTaskCtrl* ComTaskIdToCtrl(DWORD dwID);	//ͨ��FNȡ�ü�¼���ƽṹ
WORD ComTaskGetRecSize(const TCommTaskCtrl* pTaskCtrl);	//ͨ����¼���ƽṹȡ�ü�¼�ĳ���
WORD ComTaskGetFnRecSize(BYTE bFn);	//ͨ��FNȡ�ü�¼�ĳ���	
WORD ComTaskGetFnRecNum(BYTE bFn);	//ͨ��FNȡ�ü�¼�ı���

WORD ComTaskGetDataLen(const TCommTaskCtrl* pTaskCtrl);
WORD ComTaskGetRecOffset(const TCommTaskCtrl* pTaskCtrl);
WORD ComTaskGetDataLen(const TCommTaskCtrl* pTaskCtrl);
WORD ComTaskGetRecNumPerPnMon(const TCommTaskCtrl* pTaskCtrl);
WORD ComTaskGetMonthNum(const TCommTaskCtrl* pTaskCtrl);
bool ComTaskGetCurRecTime(WORD wPn, const TCommTaskCtrl* pTaskCtrl, TTime* pRecTm, DWORD* pdwStartTm, DWORD* pdwEndTm);
bool ComTaskIsNeedToDo(WORD wPn, const TCommTaskCtrl* pTaskCtrl);
bool IsComTaskDone(WORD wPn, BYTE bFn, TTime* pTm, DWORD* pdwRecTime);

bool IsFnSupByPn(BYTE bFn);
extern void DoAutoSend();
/***��������**/
extern int DirRdCommToProType(BYTE* pbDst, WORD wSrcId, BYTE* pbSrc, WORD wSrcLen, bool fSpec);
void InitMtrReRd();
BYTE DoMtrReRd(BYTE bThrId, bool* pfSuccOnce);
BYTE DoMtrCurveReRd(BYTE bThrId, bool* pfSuccOnce);
//BYTE ReRdCurveFrz(WORD wPN, struct TMtrPro* pMtrPro, BYTE bThrId, bool* pfSuccOnce);
//BYTE ReRdDayMonFrz(WORD wPN, struct TMtrPro* pMtrPro, BYTE bThrId, bool* pfSuccOnce);
//int DirReadFrz(BYTE bFrzTimes, BYTE *pRxBuf, BYTE bFN, WORD wPN, WORD wIDs, TTime *time);
bool IsSupPnType(BYTE bPnType);
bool SaveNewFrzRec(TTime tmCur, BYTE* pbRec, BYTE bFN, WORD wPN, BYTE bTimeLen, WORD wRecLen);
bool SaveNewMeterTask(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen,  TTime *tSaveTime);
int SchComTaskRec(BYTE bFn, BYTE bPn, TTime tTime, BYTE *pbBuf, BYTE bInterU);
DWORD IntervsToMinutes(BYTE bInterU,BYTE bInterV);
int ReadTaskData(BYTE* pbRx, BYTE* pbTx, DWORD* rdwRecCnt, int iMaxFrmSize, bool fReadData);
BYTE FindFn(DWORD dwID);
/***��������**/
#endif  //COMMONTASK_H


