/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CommonTask.c
 * ժ    Ҫ�����ļ���Ҫʵ���������ݿ���ͨ��������ݲɼ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
*********************************************************************************************************/
#include <stdio.h>
#include "SysCfg.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "ComConst.h"
#include "MtrAPI.h"
#include "DbAPI.h"
#include "DbConst.h"
#include "SysDebug.h"
#include "ComAPI.h"
#include "DbFmt.h"
#include "TaskDB.h"
#include "FlashMgr.h"
#include "CommonTask.h"
#include "Trace.h"
#include "DbGbAPI.h"
#include "ProAPI.h"
#include "Pro.h"
#include "GbPro.h"
#include "SocketIf.h"
#include "MtrCtrl.h"
#include "MeterPro.h"
#include "MtrFmt.h"
#include "AutoSendTask.h"

//#define	CLASS1_FN_NUM	32				//һ��Сʱ��������ӳ�䵽�����������ݵ�����
#define SINGL_METER  1
#define MULTI_METER  2
#define OTHER_METER  3

#define NOM_METER  0
#define VIP_METER  1

BYTE g_bReadData[1024]; 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ComTaskȫ�����ݶ���
//��ͨ����Ŀ��ƽṹ:ÿ����һ��
static const TCommTaskCtrl g_taskCtrl[] =
{//	bFN		IDCnt		wID				bLen		����ͨ��ID			�����λ			��¼����		������������
	//DayFrozen    ����������ն���
	{1,		1,			{0x3761},		21,			0x050601ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն��������й�����ʾֵ���ܣ������ʣ�
	{2,		1,			{0x3762},		21,			0x050603ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն��������޹�����ʾֵ���ܣ������ʣ�
	{3,		1,			{0x3763},		21,			0x050602ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն��ᷴ���й�����ʾֵ���ܣ������ʣ�
	{4,		1,			{0x3764},		21,			0x050604ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն��ᷴ���޹�����ʾֵ���ܣ������ʣ�
	{5,		1,			{0x3765},		21,			0x050605ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն���һ�����޹�����ʾֵ���ܣ������ʣ�
	{6,		1,			{0x3766},		21,			0x050606ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն���������޹�����ʾֵ���ܣ������ʣ�
	{7,		1,			{0x3767},		21,			0x050607ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն����������޹�����ʾֵ���ܣ������ʣ�
	{8,		1,			{0x3768},		21,			0x050608ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�ն����������޹�����ʾֵ���ܣ������ʣ�
	//DayFrozen    �����ǰ������Ϊ�ն���
	{1,		1,			{0x166f},		21,			0x050601ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն��������й�����ʾֵ���ܣ������ʣ�
	{2,		1,			{0x167f},		21,			0x050603ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն��������޹�����ʾֵ���ܣ������ʣ�
	{3,		1,			{0x168f},		21,			0x050602ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն��ᷴ���й�����ʾֵ���ܣ������ʣ�
	{4,		1,			{0x169f},		21,			0x050604ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն��ᷴ���޹�����ʾֵ���ܣ������ʣ�
	{5,		1,			{0x16af},		21,			0x050605ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն���һ�����޹�����ʾֵ���ܣ������ʣ�
	{6,		1,			{0x16bf},		21,			0x050606ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն���������޹�����ʾֵ���ܣ������ʣ�
	{7,		1,			{0x16cf},		21,			0x050607ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն����������޹�����ʾֵ���ܣ������ʣ�
	{8,		1,			{0x16df},		21,			0x050608ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�ն����������޹�����ʾֵ���ܣ������ʣ�

	//���ܱ�״̬�֡��ն���ͨ�����������ܡ������ǰ���򡮳�����ᡯ������ͬһ��ID��
	{9,		1,			{0xc86f},		14,			0x040005ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //���״̬�����ݿ�
	{9,		1,			{0xc86f},		14,			0x040005ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //���״̬�����ݿ�

	{10,	1,			{0x886b},		2,			0xe1800012,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_P0,	TSK_DMCUR}}, //��ͨ������
	{10,	1,			{0x886b},		2,			0xe1800012,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_P0,	TSK_DMMTR}}, //��ͨ������

	//MonthFrozen  ����������¶���
	{11,	1,			{0x3793},		41,		0x0101ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMMTR}}, //�������й��������������ʱ��
	{12,	1,			{0x3795},		41,		0x0102ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMMTR}}, //�·����й��������������ʱ��
	{13,	1,			{0x3777},		21,		0x0001ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶��������й�����ʾֵ���ܣ������ʣ�
	{14,	1,			{0x3778},		21,		0x0003ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶��������޹�����ʾֵ���ܣ������ʣ�
	{15,	1,			{0x3779},		21,		0x0002ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶��ᷴ���й�����ʾֵ���ܣ������ʣ�
	{16,	1,			{0x3780},		21,		0x0004ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶��ᷴ���޹�����ʾֵ���ܣ������ʣ�
	{17,	1,			{0x3781},		21,		0x0005ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶���һ�����޹�����ʾֵ���ܣ������ʣ�
	{18,	1,			{0x3782},		21,		0x0006ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶���������޹�����ʾֵ���ܣ������ʣ�
	{19,	1,			{0x3783},		21,		0x0007ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶����������޹�����ʾֵ���ܣ������ʣ�
	{20,	1,			{0x3784},		21,		0x0008ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�¶����������޹�����ʾֵ���ܣ������ʣ�

	//MonthFrozen  �����ǰ��Ϊ�¶���
	{11,	1,			{0x20cf},		41,		0x0101ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMCUR}}, //�������й��������������ʱ��
	{12,	1,			{0x20df},		41,		0x0102ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMCUR}}, //�·����й��������������ʱ��
	{13,	1,			{0x166f},		21,		0x0001ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶��������й�����ʾֵ���ܣ������ʣ�
	{14,	1,			{0x167f},		21,		0x0003ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶��������޹�����ʾֵ���ܣ������ʣ�
	{15,	1,			{0x168f},		21,		0x0002ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶��ᷴ���й�����ʾֵ���ܣ������ʣ�
	{16,	1,			{0x169f},		21,		0x0004ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶��ᷴ���޹�����ʾֵ���ܣ������ʣ�
	{17,	1,			{0x16af},		21,		0x0005ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶���һ�����޹�����ʾֵ���ܣ������ʣ�
	{18,	1,			{0x16bf},		21,		0x0006ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶���������޹�����ʾֵ���ܣ������ʣ�
	{19,	1,			{0x16cf},		21,		0x0007ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶����������޹�����ʾֵ���ܣ������ʣ�
	{20,	1,			{0x16df},		21,		0x0008ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�¶����������޹�����ʾֵ���ܣ������ʣ�


	//MonthFrozen  �������һ������
	{11,	1,			{0x3789},		41,		0x0101ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMMSETT}}, //�������й��������������ʱ��
	{12,	1,			{0x3791},		41,		0x0102ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMMSETT}}, //�·����й��������������ʱ��
	{13,	1,			{0x3877},		21,		0x0001ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶��������й�����ʾֵ���ܣ������ʣ�
	{14,	1,			{0x3878},		21,		0x0003ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶��������޹�����ʾֵ���ܣ������ʣ�
	{15,	1,			{0x3879},		21,		0x0002ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶��ᷴ���й�����ʾֵ���ܣ������ʣ�
	{16,	1,			{0x3880},		21,		0x0004ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶��ᷴ���޹�����ʾֵ���ܣ������ʣ�
	{17,	1,			{0x3881},		21,		0x0005ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶���һ�����޹�����ʾֵ���ܣ������ʣ�
	{18,	1,			{0x3882},		21,		0x0006ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶���������޹�����ʾֵ���ܣ������ʣ�
	{19,	1,			{0x3883},		21,		0x0007ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶����������޹�����ʾֵ���ܣ������ʣ�
	{20,	1,			{0x3884},		21,		0x0008ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�¶����������޹�����ʾֵ���ܣ������ʣ�

	{21,	1,			{0x2200},		27,		0xe100c010,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //�µ�ѹԽ��ͳ������
	{21,	1,			{0x2200},		27,		0xe100c010,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //�µ�ѹԽ��ͳ������
	{21,	1,			{0x2200},		27,		0xe100c010,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //�µ�ѹԽ��ͳ������

	{22,	1,			{0x886d},		3,		0xe1800014,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_P0,	TSK_DMMTR}}, //��ͨ������
	{22,	1,			{0x886d},		3,		0xe1800014,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_P0,	TSK_DMCUR}}, //��ͨ������
	{22,	1,			{0x886d},		3,		0xe1800014,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_P0,	TSK_MSETT}}, //��ͨ������

/*
	//��������   �����������   
	//��ʱ�Ȳ��������������
	{23,	1,			{0x3701},		21,		0x0001ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //���������й�����ʾֵ���ܣ������ʣ�
	{24,	1,			{0x3702},		21,		0x0003ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //���������޹�����ʾֵ���ܣ������ʣ�
	{25,	1,			{0x3703},		21,		0x0002ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //���㷴���й�����ʾֵ���ܣ������ʣ�
	{26,	1,			{0x3704},		21,		0x0004ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //���㷴���޹�����ʾֵ���ܣ������ʣ�
	{27,	1,			{0xb61f},		18,		0x0201ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //�����ѹ
	{28,	1,			{0xb62f},		18,		0x0202ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //�������
	{29,	1,			{0xb65f},		2,		0x02060000,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //�����ܹ�������ʾֵ
*/	

	//��������   �����ǰ����
	{23,	1,			{0x166f},		21,		0x0001ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //���������й�����ʾֵ���ܣ������ʣ�
	{24,	1,			{0x167f},		21,		0x0003ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //���������޹�����ʾֵ���ܣ������ʣ�
	{25,	1,			{0x168f},		21,		0x0002ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //���㷴���й�����ʾֵ���ܣ������ʣ�
	{26,	1,			{0x169f},		21,		0x0004ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //���㷴���޹�����ʾֵ���ܣ������ʣ�
	{27,	1,			{0xb61f},		18,		0x0201ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //�����ѹ
	{28,	1,			{0xb62f},		18,		0x0202ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //�������
	{29,	1,			{0xb65f},		2,		0x02060000,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //�����ܹ�������ʾֵ
};
#define COMM_TASK_NUM (sizeof(g_taskCtrl)/sizeof(TCommTaskCtrl))


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ComTaskʵ��

//������ͨ��FNȡ�ü�¼���ƽṹ
const TCommTaskCtrl* ComTaskFnToCtrl(BYTE bFn)
{
	WORD i;
	for (i=0; i<COMM_TASK_NUM; i++)
	{
		if (bFn == g_taskCtrl[i].bFN)
			return &g_taskCtrl[i];
	}

	return NULL;
}

//������ͨ��FNȡ�ü�¼���ƽṹ
const TCommTaskCtrl* ComTaskIdToCtrl(DWORD dwID)
{
	WORD i;
	for (i=0; i<COMM_TASK_NUM; i++)
	{
		//if (dwID == g_taskCtrl[i].dwID)
		if ((dwID == g_taskCtrl[i].dwID) || ((dwID | 0xff) == g_taskCtrl[i].dwID) || ((dwID | 0xff00) == g_taskCtrl[i].dwID)) 
			return &g_taskCtrl[i];
	}

	return NULL;
}

WORD ComTaskGetRecOffset(const TCommTaskCtrl* pTaskCtrl)
{	
	if (pTaskCtrl == NULL)
		return 0;

 	switch(pTaskCtrl->bIntervU)
 	{
 	case TIME_UNIT_DAY:
 		return 4;	//����ʱ��(������)�Ͳ������(BYTE)
 	case TIME_UNIT_MONTH:
 		return 3;	//����ʱ��(����)�Ͳ������(BYTE)

 	case TIME_UNIT_HOUR:
 		return  4;	//����ʱ��(������)�Ͳ������(BYTE)
 	}
}

//������ͨ����¼���ƽṹȡ�ü�¼���ݲ��ֵĳ���
WORD ComTaskGetDataLen(const TCommTaskCtrl* pTaskCtrl)
{
	int iItemLen = 0;
	BYTE bIDCnt = 0;

	if (pTaskCtrl == NULL)
		return 0;
	iItemLen = pTaskCtrl->bLen;

	if (iItemLen <= 0)
	{
		DTRACE(DB_TASK, ("ComTaskGetDataLen: fail due to unknown ID=%04x\r\n", pTaskCtrl->wID));
		return 0;
	}



	return (WORD )iItemLen;
}

//������ͨ����¼���ƽṹȡ�ü�¼�ĳ���
WORD ComTaskGetRecSize(const TCommTaskCtrl* pTaskCtrl)
{
	WORD wRecSize;

	if (pTaskCtrl == NULL)
		return 0;
	
	wRecSize = ComTaskGetDataLen(pTaskCtrl);

	wRecSize += ComTaskGetRecOffset(pTaskCtrl);

	return wRecSize;
}

//������ͨ����¼���ƽṹȡ�ü�¼�ı���
WORD ComTaskGetRecNumPerPnMon(const TCommTaskCtrl* pTaskCtrl)
{
	if (pTaskCtrl == NULL)
		return 0;

	switch(pTaskCtrl->bIntervU)
	{
	case TIME_UNIT_DAY:
		return 31;

	case TIME_UNIT_DAYFLG:
		return 31;
		
	case TIME_UNIT_MONTH:
		return 1;

	case TIME_UNIT_HOUR:
	case TIME_UNIT_MINUTE:
		return 31;	//*24
	}
	
	return 0;
}


//������ͨ����¼���ƽṹȡ�ü�¼������·���
WORD ComTaskGetMonthNum(const TCommTaskCtrl* pTaskCtrl)
{
	if (pTaskCtrl == NULL)
		return 0;

	switch(pTaskCtrl->bIntervU)
	{
	case TIME_UNIT_DAY:
		return 1;

	//case TIME_UNIT_DAYFLG:
		//return 1;
		
	case TIME_UNIT_MONTH:
		return 12;

	case TIME_UNIT_HOUR:
	case TIME_UNIT_MINUTE:
		return 1;	//*24
	}
	
	return 0;
}


//������ͨ��FNȡ�ü�¼�ĳ���
WORD ComTaskGetFnRecSize(BYTE bFn)
{
	const TCommTaskCtrl* pTaskCtrl = ComTaskFnToCtrl(bFn);

	return ComTaskGetRecSize(pTaskCtrl);
}

bool ComTaskGetCurRecTime(WORD wPn, const TCommTaskCtrl* pTaskCtrl, TTime* pRecTm, DWORD* pdwStartTm, DWORD* pdwEndTm)
{
	DWORD dwDayFlg;
	TTime tmStart, tmEnd;
	BYTE bBuf[6];
	WORD wIntervV;

	pRecTm->nSecond = 0;	//��������
	tmStart = *pRecTm;
	tmEnd = *pRecTm;

	switch (pTaskCtrl->bIntervU)
	{
	case TIME_UNIT_HOUR:	//Сʱ
		pRecTm->nMinute = 0;
		
		//��ʼʱ��
		tmStart.nMinute = 0;

		//�����ʱ��
		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, 1);
		break;

	case TIME_UNIT_DAY:		//�ն����޶������л���һ��Сʱ�ڳ���
		//if (pRecTm->nHour != 0)
		//	return false;

		//pRecTm->nHour = 0;
		//pRecTm->nMinute = 0;

		//��ʼʱ��
		tmStart.nHour = 0;
		tmStart.nMinute = 0;
		tmStart.nSecond = 0;

		//�����ʱ��
		tmEnd.nHour = 23;
		tmEnd.nMinute = 59;
		tmEnd.nSecond = 59;

		break;
	/*case TIME_UNIT_DAYFLG:		
		GetPnDate(wPn, bBuf);	 //GetMtrDate(PORT_GB485, bBuf);

		dwDayFlg = ByteToDWord(bBuf);
		if ((dwDayFlg & (1<<(pRecTm->nDay-1))) == 0) //���ǳ�����
			return false;	//��ǰʱ�䲻���ϳ���Ҫ��,����false����

		tmEnd.nHour = BcdToByte(bBuf[5]);
		tmEnd.nMinute = BcdToByte(bBuf[4]);
		pRecTm->nSecond = 30; //��֤������ʼ������ִ��
		dwDayFlg = SecondsPast(&tmEnd, pRecTm);
		if (dwDayFlg==0 || dwDayFlg>3600*2)
			return false;	//��ǰʱ�䲻���ϳ���Ҫ��
		//�󳭱�Ŀ�ʼʱ��
		tmStart.nHour = BcdToByte(bBuf[5]);		//��Сʱ�������ƣ��ٹ涨Сʱִ��
		tmStart.nMinute = BcdToByte(bBuf[4]);		//����
		tmStart.nSecond = 0;

		wIntervV = GetMeterInterv(wPn);
        if (wIntervV == 0)  //�����ֹ��0
            return false; 
		dwDayFlg = TimeToMinutes(&tmStart)/wIntervV*wIntervV;
		MinutesToTime(dwDayFlg, &tmStart);
		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, 2);
		break;	*/
	case TIME_UNIT_MONTH:
		if (pRecTm->nDay!=1 || pRecTm->nHour!=0)
			return false;	//��ǰʱ�䲻���ϳ���Ҫ��,����false����

		//��ʼʱ��
		tmStart.nDay = 1;	//�¶���������1�Ŷ���,����Ҫ������ĩ���ж���
		tmStart.nHour = 0;		//pRdCtrl->tmStartTime.nHour;
		tmStart.nMinute = 0;

		//�����ʱ��:����1��23:59
		tmEnd.nDay = 1;
		tmEnd.nHour = 1;
		tmEnd.nMinute = 0;
		break;

	default:
		return false;
	}

	*pdwStartTm = TimeToSeconds(&tmStart);
	*pdwEndTm = TimeToSeconds(&tmEnd);
	return true;
}


//�������ж���ͨ�����Ƿ���Ҫִ��
bool ComTaskIsNeedToDo(WORD wPn, const TCommTaskCtrl* pTaskCtrl)
{
	BYTE bProfMode, bDayMode, bDayFlgMode, bDemDayMode, bDemDayFlgMode, bMonSettMode, bDemMonSettMode;
	BYTE bProp = GetPnProp(wPn);

	if (wPn == PN0)
	{
		if (pTaskCtrl->bPnChar[0] == TASK_PN_TYPE_P0)
			return true;
	}
	else if (bProp == PN_PROP_METER || bProp == PN_PROP_CCT)
	{
		if ((pTaskCtrl->bPnChar[0]&TASK_PN_TYPE_MTR) == 0)
			return false;
		
		bProfMode = 0;
		ReadItemEx(BN24, PN0, 0x4110, &bProfMode);	//0x4110 1 ���߶���ģʽ��,0�����ǰ���ݣ�1���������
// 		if (pTaskCtrl->bFN<101 || pTaskCtrl->bFN>104)
// 			bProfMode = 0; //07�������ʾֵ����Ŀǰֻ֧�ֳ���ǰ

		bDayMode = 0;
		ReadItemEx(BN24, PN0, 0x4111, &bDayMode); //0x4111 1 �ն���ģʽ��,0�����ǰ���ݣ�1�������

		bDayFlgMode = 0;
		ReadItemEx(BN24, PN0, 0x4112, &bDayFlgMode); //0x4112 1 �����ն���ģʽ��,0�����ǰ���ݣ�1�������

		bDemDayMode = 0;
		ReadItemEx(BN24, PN0, 0x4113, &bDemDayMode); //0x4113 1 �������¶���ģʽ��,0�����ǰ���ݣ�1�������

		bDemDayFlgMode = 0;
		ReadItemEx(BN24, PN0, 0x4114, &bDemDayFlgMode); //0x4114 1 �����������¶���ģʽ��,0�����ǰ���ݣ�1�������
        
   		bMonSettMode = 0;
		ReadItemEx(BN24, PN0, 0x4115, &bMonSettMode); //0x4115 1 �¶���ģʽ��,0�����ǰ���ݣ�1�������
        
   		bDemMonSettMode = 0;
		ReadItemEx(BN24, PN0, 0x4116, &bDemMonSettMode); //0x4116 1 �����¶���ģʽ��,0�����ǰ���ݣ�1������ն��᣻2����������

		switch(pTaskCtrl->bPnChar[1])
		{
		case TSK_DMCUR:		//DAY&MONTH FROZEN CURRENT �����¶������óɳ����ǰ����ʱ����
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //�¶��᳭������
			{
				if (bMonSettMode==0 || (bMonSettMode!=2 && !IsV07Mtr(wPn)))
					return true;
				else
					return false;
			}

			if (bDayMode==0 || !IsV07Mtr(wPn))	//0x3004 1 ���¶���ģʽ��,0�����ǰ���ݣ�1�������
				return true;
			else
				return false;

		case TSK_DMMTR:		//DAY&MONTH FROZEN METER FRZ �����¶������óɳ������ʱ����
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //�¶��᳭������
			{
				if (bMonSettMode==1 && IsV07Mtr(wPn))
					return true;
				else
					return false;
			}

			if (bDayMode==1 && IsV07Mtr(wPn))	//0x3004 1 ���¶���ģʽ��,0�����ǰ���ݣ�1�������
				return true;
			else
				return false;
            
		case TSK_MSETT:            
	            if (pTaskCtrl->bIntervU==TIME_UNIT_MONTH && bMonSettMode==2)    //�¶��᳭������
				return true;
	            else
				return false;


		case TSK_PFCUR:		//PROFILE FROZEN CURRENT ���������óɳ����ǰ����ʱ����
//			if (bProfMode==0 || !IsV07Mtr(wPn))	//0x3003 1 ���߶���ģʽ��,0�����ǰ���ݣ�1���������
				return true;
// 			else
// 				return false;


		case TSK_DEMDMCUR:	//DEMAND DAY&MONTH FROZEN CURRENT ���������¶������óɳ����ǰ����ʱ����
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //�¶��᳭������
			{
				if (bDemMonSettMode==0 || !IsV07Mtr(wPn))	//0x3006 1 �������¶���ģʽ��,0�����ǰ���ݣ�1�������
					return true;
				else
					return false;
			}
            


		case TSK_DEMDMMTR:		//DEMAND DAY&MONTH FROZEN METER FRZ ���������¶������óɳ������ʱ����
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //�¶��᳭������
			{
				if (bDemMonSettMode==1 && IsV07Mtr(wPn))	//0x3006 1 �������¶���ģʽ��,0�����ǰ���ݣ�1�������
					return true;
				else
					return false;
			}

        
		case TSK_DEMMSETT:
			if (pTaskCtrl->bIntervU==TIME_UNIT_MONTH && bDemMonSettMode==2)    //�����¶��᳭������
				return true;
			else
				return false;
		}
	}

	return false;
}

bool ComTaskSaveRec(WORD wPn, const TCommTaskCtrl* pTaskCtrl, TTime* pRecTm, BYTE* pbRec)
{
	WORD i;
	TTime t;
	const WORD *pwSubID;
	BYTE* pbRec0 = pbRec;
	t = *pRecTm;

	switch(pTaskCtrl->bIntervU)
	{
	case TIME_UNIT_DAY:
        //AddIntervs(pRecTm, TIME_UNIT_DAY, -1);	//����20���ն�������Ϊ19��23��59��59������
   		TimeTo4Bcd(pRecTm, pbRec);
		pbRec += 4;
		*pbRec++ = wPn&0xff;
		break;		//����ʱ��(������ʱ��)�Ͳ������(BYTE)
	case TIME_UNIT_MONTH:
        //AddIntervs(pRecTm, TIME_UNIT_MONTH, -1);	//����20���ն�������Ϊ19��23��59��59������
		TimeTo4Bcd(pRecTm, pbRec);
		pbRec += 4;
		*pbRec++ = wPn&0xff;
		break;		//����ʱ��(����)�Ͳ������(BYTE)
	case TIME_UNIT_HOUR:
		TimeTo4Bcd(pRecTm, pbRec);
		pbRec += 4;
		*pbRec++ = wPn&0xff;
		break;	//����ʱ��(FMT15)�Ͳ������(BYTE)
	}

	#ifdef SYS_WIN
	if (IsDebugOn(DB_TASK))  
	{
		char szBuf[64];
		sprintf(szBuf, "ComTaskSaveRec: pn=%d, fn=%d append rec --> ", wPn, pTaskCtrl->bFN);
		TraceBuf(DB_TASK, szBuf, pbRec0, ComTaskGetDataLen(pTaskCtrl)+5);
	}
	#endif

	return PipeAppend(TYPE_FRZ_TASK, pTaskCtrl->bFN, pbRec0, ComTaskGetRecSize(pTaskCtrl));
}


//������ִ�в�����Pn��������ͨ����
//������@pTm ��ǰ�����ʱ��
//		@pdwRecTime �ɵ��뵼���߳��ṩ�ĵ�ǰ���������ͨ�����¼ʱ�䣬
//					������64�����������Ǹò�������Ҫִ�е��������ţ�����F39�򶳽�ģʽ�ֵĸı���ſ��ܻ�ı䣬
//					��3�ֽڱ�ʾ��BASETIME������Сʱ�����ֽڱ�ʾ��Ӧ��FN��������ֹ��ŵı䶯
bool DoComTask(WORD wPn, TTime* pTm, DWORD* pdwRecTime, int Vip_Num, BYTE bVip, BYTE bType)
{
	DWORD dwStartTm, dwEndTm, dwRecHour; //��ѯϵͳ���õ�ʱ��
	WORD i, wValidNum, wRecOff;
    WORD *pwID=NULL;
	TTime tmRec;
	BYTE bRecBuf[96];
	BYTE bFn;
// 	BYTE bType, bVip;
// 	if (!GetUserTypeAndVip(wPn, &bVip, &bType))
// 	{
// 		return false;
// 	}

	for(i=0; i<COMM_TASK_NUM; i++)   //���ĳ�������� �����е����񶼱���һ��
	{
		const TCommTaskCtrl* pTaskCtrl = &g_taskCtrl[i];
		if (!ComTaskIsNeedToDo(wPn, pTaskCtrl))		//�ж���ͨ������Ҫִ��
			continue;

		bFn = pTaskCtrl->bFN;
		//���������жϣ����ص㻧��ִ�ж�Ӧ�����񣬷�֮�������ص㻧������δ�ṩ�ӿڣ���������
		if (bFn >= 23 && bFn <= 29)
			if ((bVip != VIP_METER) || (Vip_Num > MAX_VIP_METER))	//�����ص㻧���߳���10���ص㻧,��ִ��
				continue;

		if (bType == SINGL_METER)
			if (bFn != 1 && bFn != 3 && bFn != 9 && bFn != 13 && bFn != 15 && bFn != 23 && bFn != 25 )
				continue;

		tmRec = *pTm;
		if (ComTaskGetCurRecTime(wPn, pTaskCtrl, &tmRec, &dwStartTm, &dwEndTm)) //��ȡ���������ֹʱ��
		{
			if (pTaskCtrl->bIntervU==TIME_UNIT_DAY)
				dwRecHour = DaysFrom2000(&tmRec);
			else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
				dwRecHour = MonthFrom2000(&tmRec);
			else
				dwRecHour = TimeToHours(&tmRec);
			if (dwRecHour!=(*pdwRecTime&0xffffff) || pTaskCtrl->bFN!=(*pdwRecTime>>24))	//��¼��û���ɹ�
			{
				pwID = (WORD* )pTaskCtrl->wID;
				if (QueryItemTimeMid(dwStartTm, dwEndTm, BN0, wPn, pwID, pTaskCtrl->bIDCnt, bRecBuf, &wValidNum) > 0) //�ж������Ƿ񵽴�
				{
					memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));
					wRecOff = ComTaskGetRecOffset(pTaskCtrl);	//��¼ͷƫ��
					if (ReadItemMid(BN0, wPn, pwID, pTaskCtrl->bIDCnt, bRecBuf+5, dwStartTm, dwEndTm) > 0) //�����ID
					{
						ComTaskSaveRec(wPn, pTaskCtrl, &tmRec, bRecBuf);
						*pdwRecTime = dwRecHour + ((DWORD )pTaskCtrl->bFN<<24);
					}
				}
			}
		}

		pdwRecTime++;
	}

	return true;
}

//�����������Ƿ�ִ��
//������@pTm ��ǰ�����ʱ��
//		@pdwRecTime �ɵ��뵼���߳��ṩ�ĵ�ǰ���������ͨ�����¼ʱ�䣬
//					������64�����������Ǹò�������Ҫִ�е��������ţ�����F39�򶳽�ģʽ�ֵĸı���ſ��ܻ�ı䣬
//					��3�ֽڱ�ʾ��BASETIME������Сʱ�����ֽڱ�ʾ��Ӧ��FN��������ֹ��ŵı䶯
bool IsComTaskDone(WORD wPn, BYTE bFn, TTime* pTm, DWORD* pdwRecTime)
{
	DWORD dwRecHour; //��ѯϵͳ���õ�ʱ��
	WORD i;
	TTime tmRec = *pTm;

	for(i=0; i<COMM_TASK_NUM; i++)
	{
		const TCommTaskCtrl* pTaskCtrl = &g_taskCtrl[i];
		if (!ComTaskIsNeedToDo(wPn, pTaskCtrl))		//�ж���ͨ������Ҫִ��
			continue;
		if (pTaskCtrl->bFN == bFn)
		{
			if (pTaskCtrl->bIntervU==TIME_UNIT_DAY || pTaskCtrl->bIntervU==TIME_UNIT_DAYFLG)
				dwRecHour = DaysFrom2000(&tmRec);
			else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
				dwRecHour = MonthFrom2000(&tmRec);
			else
				dwRecHour = TimeToHours(&tmRec);
			if (dwRecHour==(*pdwRecTime&0xffffff) && pTaskCtrl->bFN==(*pdwRecTime>>24))	//��¼���ɹ�
			{
				return true;
			}
		}

		pdwRecTime++;
	}

	return false;
}

//������������FN�����¶�̬����Ҫ����or��Ҫ�ͷ�
//������@bFN ��Ҫ������FN
bool IsFnSupByPn(BYTE bFn)
{
	WORD wPn;
	//��/��/���߶�������
	if (bFn < FN_COMSTAT)
		return true;

	//��ͨ����
	if (bFn < FN_FWDSTAT)
	{
		//�ж������Ƿ�����
		return IsTaskValid(COMMON_TASK_TYPE, bFn - FN_COMSTAT);
	}
	//�м�����
	if (bFn < FN_MAX)
	{
		//�ж������Ƿ�����
		return IsTaskValid(FWD_TASK_TYPE, bFn - FN_FWDSTAT);
	}

	return false;
}



/*************************************�������֡���Start*********************************************/
TTime g_tmOld;
BYTE g_bRdTimes[(PN_NUM+3)/4+1];	//Ϊ�˽�ʡ�ڴ棬ÿ��������ռ2bits


//�������������
//������pfSuccOnce ��������ɹ���һ��������򷵻�true,���򷵻�false
BYTE ReRdCurveFrz(WORD wPN, struct TMtrPro* pMtrPro, BYTE bThrId, bool* pfSuccOnce)
{
	WORD wTask = 0, wIDs = 0;
	int i = 0;
	TTime tmNow;
	BYTE bRdState;
	BYTE bReRdTimes = 0, bFN = 0;
	BYTE bRecBuf[128];
	int iDirRet = 0, iRet = 0;

	GetCurTime(&tmNow);

	tmNow.nMinute = 0;
	tmNow.nSecond = 0;

	bReRdTimes = tmNow.nHour;

	for (i=bReRdTimes; i>=0; i--) //��������
	{
		tmNow.nHour = (BYTE )i;
		for (wTask=0; wTask<COMM_TASK_NUM; wTask++)
		{
			bRdState = GetRdMtrState(bThrId);	//ȡ�õ�ǰ�ĳ���״̬
			if (bRdState != RD_ERR_OK)  //��ǰ����ֹͣ�������ֱ��״̬
				return bRdState;	

			if (!IsFnSupport(wPN, g_taskCtrl[wTask].bFN, 2))	//�ò����㲻֧��FN)		//�ж���ͨ������Ҫִ��
				continue;

			if (!IsSupPnType(g_taskCtrl[wTask].bPnChar[1]))
				continue;

			if (g_taskCtrl[wTask].bIntervU != TIME_UNIT_HOUR) 
				continue;

			bFN = g_taskCtrl[wTask].bFN;
			wIDs = g_taskCtrl[wTask].dwID;
			
			if ((bFN>81 && bFN<=88) || (bFN>89 && bFN<=94) || 
				(bFN>101 && bFN<=104) || (bFN>105 && bFN<=108) || 
				(bFN>145 && bFN<=148))	//�������ߵ�ID��ͬ��ֻ����һ��
				continue;

			iRet = SchComTaskRec(bFN, wPN, tmNow, bRecBuf, TIME_UNIT_HOUR);
			if (iRet > 0)	//�ñʼ�¼�Ѿ�����
				continue;
			
			memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));
			iDirRet = DirReadFrz(0, bRecBuf, bFN, wPN, wIDs, &tmNow, pMtrPro, TIME_UNIT_HOUR); 
			if (iDirRet >0)//����ɹ�
			{
				if (IsBcdCode(bRecBuf, iDirRet))
				{
					SaveNewMeterTask(wPN, wIDs, bRecBuf, iDirRet, &tmNow);
 					*pfSuccOnce = true;
				}
			}
		}
	}

	return RD_ERR_OK;
}

//������pfSuccOnce ��������ɹ���һ��������򷵻�true,���򷵻�false
BYTE ReRdDayMonFrz(WORD wPN, struct TMtrPro* pMtrPro, BYTE bThrId, bool* pfSuccOnce)
{
	WORD wRecLen=0;
	WORD wTask = 0;
	WORD wIDs =  0;

	bool fPnStatus = true; //��ǰ������Ĳ���״̬
	int iRet = 0;
	int iDirRet = 0;

	BYTE bRdState;
	BYTE bTimeLen = 0;
	BYTE bFN = 0, bInterU = 0, bRdTimes = 0;//, bNoRdTimes;
	BYTE j = 0;
	BYTE bRecBuf[128];
	BYTE bDayFrzStatus = 0;
	BYTE bMonFrzStatus = 0;
	BYTE bMonthSettMode = 0;
	BYTE bDayFrzIdx[3];

	BYTE *pbRec;
	TTime tmStart,tmNow, now, tmRec;

	memset(bDayFrzIdx, INVALID_DATA, sizeof(bDayFrzIdx));

	ReadItemEx(BN24, PN0, 0x4111, &bDayFrzStatus); //07�����ն���ģʽ��,��97�治������0:����ȡ�����,�ն����ж��ᡢ1:��ȡ���������
	ReadItemEx(BN24, PN0, 0x4115, &bMonthSettMode); //�¶���ģʽ <ʾֵ����>
	ReadItemEx(BN24, PN0, 0x4116, &bMonFrzStatus); //�����¶���ģʽ��  
	
												   

	GetCurTime(&now);
	now.nSecond = 0;
	now.nMinute = 0;
	now.nHour = 0;

	if (bDayFrzStatus == 1) //ֻ���ն�����Ҫ����ʱ��
	{
		if (!GetMtrDayFrzIdx(pMtrPro, now, bDayFrzIdx))	//ȡ�õ��ǰ3���ն��������
		{
			if (bMonFrzStatus!=2 && bMonthSettMode!=2)
			{
				return RD_ERR_RDFAIL;	//����ʧ��
			}
		}
			
	}

	for (wTask=0; wTask<COMM_TASK_NUM; wTask++)
	{
		tmNow = now;
		
		if (!IsFnSupport(wPN, g_taskCtrl[wTask].bFN, 2))	//�ò����㲻֧��FN)		
			continue;

/*		if (!IsSupPnType(g_taskCtrl[wTask].bPnChar[1]))
		{
			continue;
		}
*/
		if (g_taskCtrl[wTask].bIntervU==TIME_UNIT_MONTH )//�¶���
		{
			if ((bMonFrzStatus==2 && g_taskCtrl[wTask].bPnChar[1]==TSK_DEMMSETT) || 
					(bMonthSettMode==2 && g_taskCtrl[wTask].bPnChar[1]==TSK_MSETT) )
			{
				bInterU = TIME_UNIT_MONTH;
				bTimeLen = 2;
				bRdTimes = 1; //��1������
				tmNow.nDay = 0;
			}
			else
			{
				continue;
			}
			
		}
		else if (g_taskCtrl[wTask].bIntervU==TIME_UNIT_DAY && IsSupPnType(g_taskCtrl[wTask].bPnChar[1]))//�ն���
		{
			if (bDayFrzStatus != 1)
				continue;

			bInterU = TIME_UNIT_DAY;
			bTimeLen = 3;
			bRdTimes = 3; //��3��
		}
		else
		{
			continue; //�������͵����񲻽��в���
		}

		for (j=0; j<bRdTimes; j++) 
		{
			bRdState = GetRdMtrState(bThrId);	//ȡ�õ�ǰ�ĳ���״̬
			if (bRdState != RD_ERR_OK)  //��ǰ����ֹͣ�������ֱ��״̬
				return bRdState;	

			memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));

			pbRec = bRecBuf;
			pbRec += 7; //��һ�ʼ�¼�Ķ���ʱ��(5)�Ͳ������(2)����7���ֽڵĿռ�

			tmStart = tmNow;
			AddIntervs(&tmStart, bInterU, -(j+1));

			tmRec = tmStart;
			if (bDayFrzIdx[j]==INVALID_DATA && bInterU==TIME_UNIT_DAY)//û�е���Ķ�������
			{
				continue;
			}

			bFN = g_taskCtrl[wTask].bFN;
			wIDs = g_taskCtrl[wTask].dwID;
			wRecLen = ComTaskGetFnRecSize(bFN);
			
			iRet = SchComTaskRec(bFN, wPN, tmRec, bRecBuf, bInterU);
			if (iRet>0 || iRet==-2)	//����/�µļ�¼��������д���
				continue;

			memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));
			

			//bFrzTimes = DaysPast(&tmStart,&tmNow);//������������в���
			if (bInterU == TIME_UNIT_MONTH)
				iDirRet = DirReadFrz(j, bRecBuf+bTimeLen+1+5+1, 
									 bFN, wPN, wIDs, &tmRec, pMtrPro, bInterU);
			else
				iDirRet = DirReadFrz(bDayFrzIdx[j], bRecBuf+bTimeLen+1+5+1, 
									 bFN, wPN, wIDs, &tmRec, pMtrPro, bInterU); 
			
			if (iDirRet < 0) //����ʧ��
			{
				fPnStatus = false;
			}
			else
			{
				SaveNewFrzRec(tmRec, bRecBuf, bFN, wPN, bTimeLen, wRecLen);
				*pfSuccOnce = true;
			}
		}
	}

	if (!fPnStatus)
		return RD_ERR_RDFAIL;	//����ʧ��

	return RD_ERR_OK;
}


bool IsSupPnType(BYTE bPnType)
{
	switch(bPnType)
	{	
	case TSK_DMMTR:		
	case TSK_DFMTR:		
	case TSK_PFMTR:		
	case TSK_DEMDMMTR:		
	case TSK_DEMDFMTR:
		return true;
	}

	return false;
}

//�Բ����������ݽ������ ����ʱ��+������+����ʱ��+����+����
bool SaveNewFrzRec(TTime tmCur, BYTE* pbRec, BYTE bFN, WORD wPN, BYTE bTimeLen, WORD wRecLen)
{
	BYTE bRateNum = 4;
	//BYTE bClass = 2;
	//BYTE idx ;
	BYTE bTimeBuf[5]; //ʱ��ĳ���Ϊ5���ֽ�
	TTime tmNow;
	bool fRet = false;

	TimeToFmt15(&tmCur, bTimeBuf);
	memcpy(pbRec, bTimeBuf+5-bTimeLen, bTimeLen);//����ʱ��(�¶��ᡪ2 �ն��ᡪ3)

	pbRec[bTimeLen] = wPN;//�������

	GetCurTime(&tmNow);
	TimeToFmt15(&tmNow, pbRec+bTimeLen+1);//����ʱ��

	//bRateNum = GetPnRateNum(wPN);
	pbRec[bTimeLen+5+1] = bRateNum;

	fRet = PipeAppend(TYPE_FRZ_TASK, bFN, pbRec, wRecLen);
	
	return fRet;
	
}

//��������ͨ��������Э���ʽ���ݵ�ת��
//������
//		@pbDst	 Ҫת��Ϊ��Ŀ���ʽֵ
//		@wSrcId	 ͨ������ԭʼID	
//		@pbSrc	 ͨ�����ݸ�ʽ��ԭʼ����
//		@wSrcLen ͨ�����ݸ�ʽ��ԭʼ����
//���أ���������ת��OK,����ת��������ݸ�ʽ�ĳ���
int DirRdCommToProType(BYTE* pbDst, WORD wSrcId, BYTE* pbSrc, WORD wSrcLen, bool fSpec)
{
	int iRet = -1;
	int i;	
	BYTE tLen=0;
	BYTE iLen=0;
	DWORD dwVal;
	//int32 iVal32;
	uint64 iVal64;
	BYTE bInvdData = GetInvalidData(INVALID_DATA);

	switch (wSrcId>>8)
	{		
	case 0x9a: //�й�����
	case 0x94:
		iLen = sizeof(uint64);				
		for (i=0; i<wSrcLen/iLen && i<5; i++)
		{	
			memcpy((BYTE*)&iVal64, pbSrc+i*iLen, iLen);
			if (iVal64 == INVALID_VAL64)
			{
				//memset(pbDst+tLen, INVALID_DATA, 5);
				return -2;//��һ������Ϊ��Ч������Ҫ����
			}
			else
				Uint64ToBCD(iVal64, pbDst+tLen, 5);				
			tLen += 5;					
		}	
		iRet = tLen;			
		break;

	case 0x9b: //�޹�����
	case 0x95:
		iLen = sizeof(uint64);				
		for (i=0; i<wSrcLen/iLen && i<5; i++)
		{	
			memcpy((BYTE*)&iVal64, pbSrc+i*iLen, iLen);
			if (iVal64 == INVALID_VAL64)
			{
				//memset(pbDst+tLen, INVALID_DATA, 4);
				return -2;
			}
			else
			{
				iVal64 /= 100;	//-2С��λ
				Uint64ToBCD(iVal64, pbDst+tLen, 4);		
			}
			tLen += 4;					
		}	
		iRet = tLen;			
		break;

	case 0x9c://����
	case 0xa1:
	case 0xa0:
	case 0xa4:
	case 0xa5:
	case 0xb1:
	case 0xb4:
	case 0xb5:
		if((wSrcId == 0x9c8f) || (wSrcId == 0x9caf) || (wSrcId == 0xb11f) || (wSrcId == 0xb12f)||
			(wSrcId == 0xb41f) || (wSrcId == 0xb42f) || (wSrcId == 0xb51f) || (wSrcId == 0xb52f))//����ʱ��
		{
			if (fSpec)
			{
				for (i=0; i<5; i++)
				{
					memcpy(pbDst+3*(i+1)+4*i, pbSrc+4*i, 4);
				}
				iRet = 20;
			}
			
			break;
			
		}
		iLen = sizeof(DWORD);				
		for (i=0; i<wSrcLen/iLen && i<5; i++)
		{	
			memcpy((BYTE*)&dwVal, pbSrc+i*iLen, iLen);
			if (dwVal == INVALID_VAL)
			{
				//memset(pbDst+tLen, INVALID_DATA, 3);
				return -2;
			}
			else
			{
				if (fSpec)
				{
					DWORDToBCD(dwVal, pbDst+tLen+4*i, 3);	
				}
				else
				{
					DWORDToBCD(dwVal, pbDst+tLen, 3);	
				}
				
			}
			tLen += 3;					
		}	
		iRet = tLen;	
		break;	
	default:		
		break;
	}
	return iRet;
}

bool SaveNewMeterTask(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, TTime *tSaveTime)
{
	BYTE mBuf[32];
	BYTE bLen = 0;
	BYTE bFn, i,j;

	TTime time;	
	memset((BYTE*)&time, 0, sizeof(TTime));

	for (i=0; i<pbBuf[0]; i++)
	{   
		time.nMinute = BcdToByte(pbBuf[bLen+1]);
		time.nHour = BcdToByte(pbBuf[bLen+2]);
		time.nDay  = BcdToByte(pbBuf[bLen+3]);
		time.nMonth = BcdToByte(pbBuf[bLen+4]);
		time.nYear = BcdToByte(pbBuf[bLen+5])+2000;		
		if ( IsInvalidTime(&time) )
			break;

		if (time.nMinute != 0)//ֻ����Сʱ����
		{
            if (pbBuf[0] != 0)
    			bLen += ((wLen-1)/pbBuf[0]);
			continue;
		}
		if ( IsDiffDay(tSaveTime, &time) ) //������ǵ�������ߣ��򲻴�
		{
            if (pbBuf[0] != 0)
    			bLen += ((wLen-1)/pbBuf[0]); //ÿ�ʼ�¼��ʵ�ʳ���
			continue;
		}

		TimeToFmt15(&time, mBuf);		//Fmt15 5�ֽ�
		memcpy(mBuf+5, &wPn, 1);		//������� 1�ֽ�	

		//��������������
		if (wID>=0x3701 && wID<=0x3704)	//���������޹�����Ҫ����˳��
		{
			if ((bFn=GetFnFromCurveId(0x3701))==0xff)
				return false;

			memcpy(mBuf+6, &pbBuf[bLen+1+5], 4);
			if (IsBcdCode(mBuf+6, 4))
				PipeAppend(TYPE_FRZ_TASK, bFn, mBuf, 10);				

			memcpy(mBuf+6, &pbBuf[bLen+1+13], 4);
			if (IsBcdCode(mBuf+6, 4))
				PipeAppend(TYPE_FRZ_TASK, bFn+1, mBuf, 10);				

			memcpy(mBuf+6, &pbBuf[bLen+1+9], 4);
			if (IsBcdCode(mBuf+6, 4))
				PipeAppend(TYPE_FRZ_TASK, bFn+2, mBuf, 10);			

			memcpy(mBuf+6, &pbBuf[bLen+1+17], 4);		
			if (IsBcdCode(mBuf+6, 4))
				PipeAppend(TYPE_FRZ_TASK, bFn+3, mBuf, 10);

		}
		else if (wID>=0x3745 && wID<=0x3748) //������Ҫ����˳��
		{
			if ((bFn=GetFnFromCurveId(0x3745))==0xff)
				return false;

			memcpy(mBuf+6, &pbBuf[bLen+1+5], 4);
			PipeAppend(TYPE_FRZ_TASK, bFn, mBuf, 10);						

			memcpy(mBuf+6, &pbBuf[bLen+1+17], 4);
			PipeAppend(TYPE_FRZ_TASK, bFn+1, mBuf, 10);		

			memcpy(mBuf+6, &pbBuf[bLen+1+9], 4);
			PipeAppend(TYPE_FRZ_TASK, bFn+2, mBuf, 10);		

			memcpy(mBuf+6, &pbBuf[bLen+1+13], 4);					
			PipeAppend(TYPE_FRZ_TASK, bFn+3, mBuf, 10);		
		}
		else if (wID>=0x3681 && wID<=0x3688)//����
		{				
			if ((bFn=GetFnFromCurveId(0x3681))==0xff)
				return false;

			for (j=0; j<8; j++)
			{
				memcpy(mBuf+6, &pbBuf[bLen+1+5+j*3], 3);				
				PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);					
			}
		}
		else if (wID>=0x3689 && wID<=0x3694)//��ѹ������
		{				
			if ((bFn=GetFnFromCurveId(0x3689))==0xff)
				return false;

			for (j=0; j<6; j++)
			{
				if (j < 3)
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+5+(j<<1)], 2); //ȥ��Ƶ��
					PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 8);
				}
				else
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+11+(j-3)*3], 3); //ȥ��Ƶ��				
					PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);
				}
			}
		}
		else if (wID>=0x3705 && wID<=0x3708)//��������
		{		
			if ((bFn=GetFnFromCurveId(0x3705))==0xff)
				return false;

			for (j=0; j<4; j++)
			{
				memcpy(mBuf+6, &pbBuf[bLen+1+5+(j<<1)], 2); //
				PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 8);
			}
		}	
        if (pbBuf[0] != 0)
    		bLen += ((wLen-1)/pbBuf[0]); //ÿ�ʼ�¼��ʵ�ʳ���
	}

	return true;

}

int SchComTaskRec(BYTE bFn, BYTE bPn, TTime tTime, BYTE *pbBuf, BYTE bInterU)
{
	int iRet = 0;
	
	if (TdbOpenTable(bFn) != TDB_ERR_OK)
	{
		TdbCloseTable(bFn);
		return -2;
	}

	iRet = TdbReadRec (bFn, bPn, tTime, pbBuf);
    TdbCloseTable(bFn);
	if (iRet > 0)
	{
		if (IsAllAByte(pbBuf, INVALID_DATA, iRet) && bInterU==TIME_UNIT_HOUR)
        {            
			return -1;
        }

		if (bInterU == TIME_UNIT_DAY)
		{
			DTRACE(DB_COMPENMTR, ("ReRdDayMonFrz:  FN%d rec already exit, pn=%d %d-%d-%d\n", 
				bFn, bPn,
				tTime.nYear, tTime.nMonth, tTime.nDay));
		}
		else if (bInterU == TIME_UNIT_MONTH)
		{
			DTRACE(DB_COMPENMTR, ("ReRdDayMonFrz:  FN%d rec already exit, pn=%d %d-%d-00\n", 
				bFn, bPn,
				tTime.nYear, tTime.nMonth));
		}
		else if (bInterU == TIME_UNIT_HOUR)
		{
			DTRACE(DB_COMPENMTR, ("ReRdDayMonFrz:  FN%d rec already exit, pn=%d %02d-%02d-%02d %02d:00:00\n", 
				bFn, bPn,
				tTime.nYear, tTime.nMonth, tTime.nDay, tTime.nHour));
		}
		
		return 1;
	}
		
	return -1;
}


void InitMtrReRd()
{
	GetCurTime(&g_tmOld);
	memset(g_bRdTimes, 0, sizeof(g_bRdTimes));
}

//������pfSuccOnce ��������ɹ���һ��������򷵻�true,���򷵻�false
BYTE DoMtrReRd(BYTE bThrId, bool* pfSuccOnce)
{
	WORD wPN = 0;
	BYTE bRdErr = 0;
	struct TMtrPro* pMtrPro;
	BYTE bRdTimes;
	TTime tmNow;
	GetCurTime(&tmNow);

	if (IsDiffDay(&tmNow, &g_tmOld))
	{
		memset(g_bRdTimes, 0, sizeof(g_bRdTimes));//ʧ�ܴ�����������
		g_tmOld = tmNow;
	}

	for (wPN=1; wPN<PN_NUM; wPN++)
	{
		if (!IsV07Mtr(wPN))
			continue;

		bRdTimes = (g_bRdTimes[wPN>>2] >> (wPN%4*2)) & 0x03;
		if (bRdTimes == 3)
			continue;

		pMtrPro = SetupThrdForPn(bThrId, wPN);
		if (pMtrPro == NULL)
			continue;

		bRdErr = ReRdDayMonFrz(wPN, pMtrPro, bThrId, pfSuccOnce);
		if (bRdErr == RD_ERR_RDFAIL) //����ʧ��
		{
			bRdTimes++;
			bRdTimes &= 0x03;
			g_bRdTimes[wPN>>2] &= ~(0x03 << (wPN%4*2));
			g_bRdTimes[wPN>>2] |= bRdTimes << (wPN%4*2);
			if (bRdTimes == 3)
			{
				//SaveMeterFailExc(wPN);
			}
		}
		else if (bRdErr != RD_ERR_OK)	//�������󣬱��磺��ǰ����ֹͣ�������ֱ��״̬
		{
			return bRdErr;	//Ӧ���̷���
		}
	}

	return RD_ERR_OK;
}

//������pfSuccOnce ��������ɹ���һ��������򷵻�true,���򷵻�false
BYTE DoMtrCurveReRd(BYTE bThrId, bool* pfSuccOnce)
{
	WORD wPN = 0;
	int iRet = 0;
	struct TMtrPro* pMtrPro;
	BYTE bRdErr;
	BYTE bCurveFrzStatus = 0;

	for (wPN=1; wPN<PN_NUM; wPN++)
	{
		if (!IsV07Mtr(wPN))
			continue;

		pMtrPro = SetupThrdForPn(bThrId, wPN);
		if (pMtrPro == NULL)
			continue;

		iRet = ReadItemEx(BN24, PN0, 0x4110, &bCurveFrzStatus);//���߶���ģʽ
		if (iRet<0 || bCurveFrzStatus==0)
			continue;

		bRdErr = ReRdCurveFrz(wPN, pMtrPro, bThrId, pfSuccOnce);
		if (bRdErr != RD_ERR_OK)
			return bRdErr;
	}

	return RD_ERR_OK;
}

/*************************************�������֡���End*********************************************/


BYTE GetRdMtrTaskValidNum()
{
	//�������ò���id 0xe0000230~0xe0000250
	BYTE bPn;
	WORD wID = 0x0b32;
	BYTE bValidNum = 0;
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	for( bPn= 1; bPn < 32; bPn++)
	{
		memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);

		ReadItemEx(BN0, bPn, wID, g_ExFlashBuf);

		if(TaskCheck(g_ExFlashBuf, g_ExFlashBuf+512, 0xe0000230+bPn-1) > 0)
		{
			bValidNum++;
		}

		//memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
		//if(readfile(wFile, i*EXSECT_SIZE, g_ExFlashBuf, -1))//���ļ�
		//{
		//	if (CheckFile(wFile, g_ExFlashBuf, 0))
		//	{
		//		//ѭ����ǰ�ĸ��ֽڣ��鿴��Ϣ��ֱ��β��
		//		for(j=0; j<4;j++)
		//		{
		//			if(g_ExFlashBuf[j*SG_TASK_LEN]==0x55 && g_ExFlashBuf[j*SG_TASK_LEN+1]==0xaa)
		//			{
		//				if(ByteToWord(&g_ExFlashBuf[j*SG_TASK_LEN+2]) > 0)
		//				{
		//					bValidNum++;
		//				}
		//			}
		//		}			
		//	}
		//}
	}
	SignalSemaphore(g_semExFlashBuf);
	return bValidNum;
}

//������wID ����ţ���0x0301~0x03FE/0x0401~0x04FE
//		pbBuf �������������ã�
//���أ��ɹ����ض����������ֽ��������򷵻�-ERR_ITEM,
int ReadTaskConfig(DWORD dwID, BYTE* pbBuf)
{
	WORD wPn=0;
	WORD wID=0;
	WORD wFileSize = 0;
	WORD wReadLen = 0;

	if(dwID>=0xE0000230 && dwID<=0xE0000250)
	{
		wPn = dwID - 0xE0000230;
		wID = 0x0b32;
		wReadLen = 712;
	}
	else if(dwID>=0xe0000301 && dwID<=0xE00003fe)
	{
		wPn = dwID - 0xe0000301;
		wID = 0x0b11;
		wReadLen = 512;
	}
	else if(dwID>=0xe0000401 && dwID<=0xE00004fe)
	{
		wPn = dwID - 0xe0000401;
		wID = 0x0b21;
		wReadLen = 281;
	}

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
	ReadItemEx(BN0, wPn, wID, g_ExFlashBuf);

	if(!IsAllAByte(g_ExFlashBuf, 0x00, wReadLen) && TaskCheck(g_ExFlashBuf, g_ExFlashBuf+wReadLen, dwID)>0)
	{
		wFileSize = TaskCheck(g_ExFlashBuf, g_ExFlashBuf+wReadLen, dwID);
	}
	else
		wFileSize = 0;


	memcpy(pbBuf, g_ExFlashBuf, wFileSize);
	SignalSemaphore(g_semExFlashBuf);
	return wFileSize;


// 	WORD wFile = 0;
// 	WORD wFileSize = 0;
// 	BYTE bSect = 0;
// 	BYTE bPage = 0;
// 
// 	if(dwID>=0xE0000230 && dwID<=0xE0000250)
// 	{
// 		bSect = (dwID - 0xE0000230)/8;//����
// 		bPage = (dwID - 0xE0000230)%8;//ҳ
// 		wFile = 0x81;
// 	}
// 	else if(dwID>=0xe0000301 && dwID<=0xe00003fe)
// 	{
// 		bSect = (dwID - 0xe0000301)/8;//����
// 		bPage = (dwID - 0xe0000301)%8;//ҳ
// 		wFile = 0x82;
// 	}
// 	else if(dwID>=0xe0000401 && dwID<=0xe00004fe)
// 	{
// 		bSect = (dwID - 0xe0000401)/8;//����
// 		bPage = (dwID - 0xe0000401)%8;//ҳ
// 		wFile = 0x83;
// 	}
// 
// 	//�����Ѿ����ڵ��������ã��ж������Ƿ��޸ģ����ǣ�����Ҫɾ���������Ӧ�����ݿ�
// 	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
// 	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
// 	if(readfile(wFile, bSect*EXSECT_SIZE, g_ExFlashBuf, -1))//���ļ�
// 	{
// 		if (CheckFile(wFile, g_ExFlashBuf, 0))
// 		{
// 			//ѭ����ǰ�ĸ��ֽڣ��鿴��Ϣ��ֱ��β��
// 			while(g_ExFlashBuf[bPage*SG_TASK_LEN]==0x55 && g_ExFlashBuf[bPage*SG_TASK_LEN+1]==0xaa)
// 			{
// 				wFileSize = ByteToWord(&g_ExFlashBuf[bPage*SG_TASK_LEN+2]);
// 				if(wFileSize>0 && wFileSize<=506)
// 				{
// 					memcpy(pbBuf, &g_ExFlashBuf[bPage*SG_TASK_LEN+4], wFileSize);
// 				}
// 			}
// 		}
// 	}
// 	SignalSemaphore(g_semExFlashBuf);
// 	return wFileSize;
}

//����:���������ý��м��
//����:@pbCfg ������������
//	   @pbEnd �������ý�������ܵ����λ��,ʵ�ʿ���С��
//����:�����ȷ�򷵻��������õĳ���,���򷵻�-1
int TaskCheck(BYTE* pbCfg, BYTE* pbEnd, DWORD dwID)
{
	WORD wLen;
	BYTE bNum = 0;

	if(((dwID>>SG_GET_TASK_TYPE)&SG_DWORD_GET_LOW_BYTE)==SG_TASK_COMMON)
	{
		if (pbEnd-pbCfg < SG_COMMTASK_CFG_FIXLEN)
			return -1;

		wLen = SG_BYTE_PN*(*(pbCfg + SG_COMMTASK_CFG_FIXLEN - 1));
		wLen += SG_BYTE_DI*(*(pbCfg + SG_COMMTASK_CFG_FIXLEN + wLen));
		wLen += SG_COMMTASK_CFG_FIXLEN + SG_BYTE_DI_GROUP;

		if (pbEnd-pbCfg < wLen)
			return -1;

		return wLen;
	}
	else if(((dwID>>SG_GET_TASK_TYPE)&SG_DWORD_GET_LOW_BYTE)==SG_TASK_FW)
	{
		if (pbEnd-pbCfg < SG_FWTASK_CFG_FIXLEN)
			return -1;

		wLen = *(pbCfg + SG_FWTASK_CFG_FIXLEN - 1);
		wLen = wLen + SG_FWTASK_CFG_FIXLEN;

		if (pbEnd-pbCfg < wLen)
			return -1;

		return wLen;
	}
	else if(((dwID>>SG_GET_TASK_TYPE)&SG_DWORD_GET_LOW_BYTE)==SG_TASK_RDMTR)//0xe00002**�� ������
	{
		if (pbEnd-pbCfg < SG_RDMTR_CFG_FIXLEN)
			return -1;

		bNum = *(pbCfg + SG_RDMTR_CFG_FIXLEN - 2);
		if(bNum == INVALID_DATA)//��Ϣ���ʶ����ΪFFʱ �� ��Ϣ���ʶ�̶���FF��FF
			bNum = 1;
		wLen = SG_BYTE_PN*bNum;

		bNum = *(pbCfg + SG_RDMTR_CFG_FIXLEN + wLen -1);
		if(bNum == INVALID_DATA)//���ݱ�������ΪFFʱ �� ���ݱ�ʶ����̶���FF FF FF FF
			bNum = 1;
		wLen += SG_BYTE_DI*bNum;

		wLen += SG_RDMTR_CFG_FIXLEN;

		return wLen;
	}
	else
	{
		return -1;
	}

}

//���ܣ�д��������
//������wID �����
//		pbbuf �����������õĻ���
//		wLen ����Ĵ�С
//
//���أ� �ɹ��򷵻�ʵ��д��Ĵ�С�����򷵻�0
int WriteTaskConfig(DWORD dwID, BYTE* pbBuf, WORD wLen, BYTE bPerm, BYTE* pbPassword)
{
	WORD wPn;
	WORD wID;
	DWORD dwTaskID = dwID;
	DWORD dwTmpID=0;
	WORD wMaxLen = 0;
	BYTE bCfgFlag[64];
	if(dwID>=0xE0000230 && dwID<=0xE0000250)
 	{
 		wPn = dwID - 0xE0000230;//�������ת���ɲ������
		wID = 0x0b32;
		wMaxLen = 712;
	}
 	else if(dwID>=0xe0000301 && dwID<=0xe00003fe)
 	{
 		wPn = dwID - 0xe0000301;//�������ת���ɲ������
		wID = 0x0b11;
		wMaxLen = 512;
 	}
 	else if(dwID>=0xe0000401 && dwID<=0xe00004fe)
 	{
 		wPn = dwID - 0xe0000401;//�������ת���ɲ������
		wID = 0x0b21;
		wMaxLen = 281;
 	}

	dwTaskID &= SG_DWORD_GET_LOW_WORD;
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
	if(ReadItemEx(BN0, wPn, wID, g_ExFlashBuf)>0)
	{
		if(wLen <= wMaxLen)
		{
			if (memcmp(g_ExFlashBuf, pbBuf, wLen) == 0)
			{
 				DTRACE(DB_TASK, ("WriteTaskConfig: The config just wrote is the same as the old one \n "));
				SignalSemaphore(g_semExFlashBuf);
				return wLen;
 			}
			memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
			memcpy(g_ExFlashBuf, pbBuf, wLen);

			WriteItemEx(BN0, wPn, wID, g_ExFlashBuf);	
		}
		else
		{
			DTRACE(DB_TASK, ("WriteTaskConfig: task%d config too long.\r\n"));
			SignalSemaphore(g_semExFlashBuf);
			return 0;
		}

	}
	else
	{
		SignalSemaphore(g_semExFlashBuf);
		return 0;
	}
	SignalSemaphore(g_semExFlashBuf);
	



	WaitSemaphore(m_semTaskCfg, SYS_TO_INFINITE);
	ReadItemEx(BN0, PN0, 0x0b40, bCfgFlag);	
	if((dwTaskID>>SG_GET_TASK_TYPE) == SG_TASK_COMMON)
	{
		dwTmpID = dwTaskID&SG_DWORD_GET_LOW_BYTE;
	 	bCfgFlag[(dwTmpID-1)/8] |= (1<<(dwTmpID-1)%8);
	}
	else if((dwTaskID>>SG_GET_TASK_TYPE)==SG_TASK_RDMTR)
	{
	
	}
	else if((dwTaskID>>SG_GET_TASK_TYPE) == SG_TASK_FW)
	{
 		dwTmpID = dwTaskID&SG_DWORD_GET_LOW_BYTE;
		bCfgFlag[(dwTmpID-1)/8 + 32] |= (1<<((dwTmpID-1)%8));
	}
 	WriteItemEx(BN0, PN0, 0x0b40, bCfgFlag);

	//������������־(���ڸ�������ִ�д���)
	memset(bCfgFlag, 0, sizeof(bCfgFlag));
	ReadItemEx(BN0, PN0, 0x0b41, bCfgFlag);
	if((dwTaskID>>SG_GET_TASK_TYPE) == SG_TASK_COMMON)
	{
		dwTmpID = dwTaskID&SG_DWORD_GET_LOW_BYTE;
	 	bCfgFlag[(dwTmpID-1)/8] |= (1<<(dwTmpID-1)%8);
	}
	else if((dwTaskID>>SG_GET_TASK_TYPE) == SG_TASK_FW)
	{
 		dwTmpID = dwTaskID&SG_DWORD_GET_LOW_BYTE;
		bCfgFlag[(dwTmpID-1)/8 + 32] |= (1<<((dwTmpID-1)%8));
	}
 	WriteItemEx(BN0, PN0, 0x0b41, bCfgFlag);
	
 	SignalSemaphore(m_semTaskCfg);
	SetInfo(INFO_TASK_PARA);//д�µ���������
	DTRACE(DB_TASK, ("WriteTaskConfig: Write  Successfully.\r\n"));
	return wLen;
// 	WORD wFile = 0;
// 	BYTE bCfgFlag[64];
// 	WORD wFileSize = 0;
// 	DWORD dwTaskID = dwID;
// 	BYTE bSect = 0;
// 	BYTE bPage = 0;
// 
// 	if(dwID>=0xE0000230 && dwID<=0xE0000250)
// 	{
// 		bSect = (dwID - 0xE0000230)/8;//����
// 		bPage = (dwID - 0xE0000230)%8;//ҳ
// 		wFile = 0x81;
// 	}
// 	else if(dwID>=0xe0000301 && dwID<=0xe00003fe)
// 	{
// 		bSect = (dwID - 0xe0000301)/8;//����
// 		bPage = (dwID - 0xe0000301)%8;//ҳ
// 		wFile = 0x82;
// 	}
// 	else if(dwID>=0xe0000401 && dwID<=0xe00004fe)
// 	{
// 		bSect = (dwID - 0xe0000401)/8;//����
// 		bPage = (dwID - 0xe0000401)%8;//ҳ
// 		wFile = 0x83;
// 	}
// 	dwTaskID &= SG_DWORD_GET_LOW_WORD;
// 
// 	//�����Ѿ����ڵ��������ã��ж������Ƿ��޸ģ����ǣ�����Ҫɾ���������Ӧ�����ݿ�
// 	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
// 	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
// 	if(readfile(wFile, bSect*EXSECT_SIZE, g_ExFlashBuf, -1))//���ļ�
// 	{
// 		if (CheckFile(wFile, g_ExFlashBuf, 0))
// 		{
// 			//ѭ����ǰ�ĸ��ֽڣ��鿴��Ϣ��ֱ��β��
// 			while(g_ExFlashBuf[bPage*SG_TASK_LEN]==0x55 && g_ExFlashBuf[bPage*SG_TASK_LEN+1]==0xaa)
// 			{
// 				wFileSize = ByteToWord(&g_ExFlashBuf[bPage*SG_TASK_LEN+2]);
// 			}
// 		}
// 	}
// 	//����������ͬ�������������ݲ�һ������ɾ��ԭ�����ļ�������ʼ����Ч��ִ�С������������Ҳ��ͬ��ֻ�ǽ��������ó���Ч
// 	if(wFileSize > 0)//��0x55 �� 0xaa��ǰ���£����ܱ�֤��д��
// 	{
// 		if (wFileSize != wLen)
// 		{
// 			memset(&g_ExFlashBuf[bPage*SG_TASK_LEN], 0x00, SG_TASK_LEN);
// 			MakeFile(wFile, g_ExFlashBuf);
// 			writefile(wFile, bSect*EXSECT_SIZE, g_ExFlashBuf);		
// 			DTRACE(DB_TASK, ("WriteTaskConfig: fail to delete the old config.\r\n"));
// 			return 0;
// 		}
// 		else
// 		{
// 			if (wLen > 506)//512 �ǹ�Լ�涨��512���Ҷ���Ϊ506������
// 			{
// 				DTRACE(DB_TASK, ("WriteTaskConfig: task%d config too long.\r\n"));
// 				return 0;
// 			}
// 			if (memcmp(g_ExFlashBuf+bPage*SG_TASK_LEN+4, pbBuf, wLen) == 0)
// 			{
// 				DTRACE(DB_TASK, ("WriteTaskConfig: The config just wrote is the same as the old one \n "));
// 				return wLen;
// 			}
// 			else
// 			{
// 				memset(&g_ExFlashBuf[bPage*SG_TASK_LEN], 0x00, SG_TASK_LEN);
// 				MakeFile(wFile, g_ExFlashBuf);
// 				writefile(wFile, 0, g_ExFlashBuf);	
// 				DTRACE(DB_CRITICAL, ("WriteTaskConfig: delete the old config.\r\n"));
// 				//return 0;
// 			}
// 		}
// 	}
// 	
// 
// 	g_ExFlashBuf[bPage*SG_TASK_LEN] = 0x55;
// 	g_ExFlashBuf[bPage*SG_TASK_LEN] = 0xaa;
// 	memcpy(g_ExFlashBuf+bPage*SG_TASK_LEN+2,(BYTE *)&wLen, 2);
// 	memcpy(g_ExFlashBuf+bPage*SG_TASK_LEN+4, pbBuf, wLen);
// 	MakeFile(wFile, g_ExFlashBuf);
// 	if (!writefile(wFile, bSect*EXSECT_SIZE, g_ExFlashBuf))
// 	{
// 		DTRACE(DB_TASK, ("WriteTaskConfig: fail to create file.\r\n"));
// 		return -ERR_ITEM;
// 	}
// 	WaitSemaphore(m_semTaskCfg, SYS_TO_INFINITE);
// 	ReadItemEx(BN3, PN0, 0x3202, bCfgFlag);
// 	if((dwTaskID>>SG_GET_TASK_TYPE) == SG_TASK_COMMON)
// 	{
// 		dwTaskID &= SG_DWORD_GET_LOW_BYTE;
// 		bCfgFlag[(dwTaskID-1)/8] |= (1<<(dwTaskID-1)%8);
// 	}
// 	else if((dwTaskID>>SG_GET_TASK_TYPE)==SG_TASK_RDMTR)
// 	{
// 
// 	}
// 	else
// 	{
// 		dwTaskID &= SG_DWORD_GET_LOW_BYTE;
// 		bCfgFlag[(dwTaskID-1)/8 + 32] |= (1<<((dwTaskID-1)%8));
// 	}
// 	WriteItemEx(BN3, PN0, 0x3202, bCfgFlag);
// 	SignalSemaphore(m_semTaskCfg);
// 	SignalSemaphore(g_semExFlashBuf);
// 	SetInfo(INFO_TASK_PARA);//д�µ���������
// 	DTRACE(DB_TASK, ("WriteTaskConfig: Write  Successfully.\r\n"));
// 	return wLen;
}
//�����תΪ����
DWORD IntervsToMinutes(BYTE bInterU,BYTE bInterV)
{
	switch (bInterU)
	{
	case TIME_UNIT_MINUTE:
		return bInterV ;
		break;
	case TIME_UNIT_HOUR:
		return bInterV*60;
		break;
	case TIME_UNIT_DAY:
		return bInterV*24*60;
		break;
	case TIME_UNIT_MONTH://TODO::���㷨
		return 28*bInterV*24*60;
		break;
	}
}

//extern TSem g_semEsam;
//extern BYTE g_bEsamTxRxBuf[1800];


int ReadTaskData(BYTE* pbRx, BYTE* pbTx, DWORD* rdwRecCnt, int iMaxFrmSize, bool fReadData)
{
	TTime tmSch;
	int iBufNum;				//�������ܴ��¼������
	WORD wRecSize;				//�������ݿⷵ�ص�ÿ�ʼ�¼�Ĵ�С
	BYTE* pbRxStatu = pbRx;
	BYTE* pbTxStatu = pbTx;
	DWORD dwInterRate;			//���ݼ��ʱ��
	WORD wDensity;				//�����ܶ�
	int iRet;
	BYTE bFn, bTaski, bTaskj;	
	BYTE bTaskCfgTempBuf[COMMTASK_REC_MAXLEN];	
	BYTE* pbRdBuf = &g_bReadData[0];
	DWORD dwLastSchTime;
	WORD wTask;
	DWORD dwSchTime, dwLastTime, dwTmIntervs;
	bool fInvaildDensity;
	int iDataCodeLen, iAllDataCodeLen;
	int iCount;

	//�����ʼ���еõ�
	BYTE bSmplIntervU, bSmplIntervV, bSndFreq;
	TTime tmLastRec;
	BYTE bCommonTaskNum=0, bFwdTaskNum=0;
	int iFrmSize=0, iDataSize=0;
	DWORD dwID=0;


	//2014-09-06  ���
	WORD wID=0;
	TComTaskCfg tComTaskCfg;
	TFwdTaskCfg tFwdTaskCfg;
	DWORD iPast=0, dwMonths, dwComSampCycle=0;
	TTime tBaseSampleTime, tTime;
	DWORD dwRealIntervU = 0;
	BYTE bTaskFwdType;

	//WaitSemaphore(g_semEsam, SYS_TO_INFINITE);	

	iRet = 0;
	ReadItemEx(BN0, POINT0, 0x0b10, &bCommonTaskNum);								//���µ�ǰ����ͨ��������
	ReadItemEx(BN0, POINT0, 0x0b20, &bFwdTaskNum);									//���µ�ǰ���м���������
	if ((bCommonTaskNum==0)&&(bFwdTaskNum==0))										//������Ч��־,���������÷����ı��ʱ��,�����ó���Ч
	{		
		//SignalSemaphore(g_semEsam);
		return 0;
	}

	memset(&tmLastRec, 0, sizeof(tmLastRec));
	memset(bTaskCfgTempBuf, 0, sizeof(bTaskCfgTempBuf));

	iBufNum=0;
	dwInterRate=0;
	wDensity = 0;
	bSmplIntervU=1;
	bSmplIntervV=1;
	bSndFreq=1;
	DTRACE(DB_TASK, ("ReadTaskData : The number demands is up(%d)!\n", *rdwRecCnt));
	
	//2014-09-06  ���
	dwID = ByteToDWORD(pbRxStatu+2, 4);
	if ((dwID&0x0000ff00) == 0x00000300)
		wID = 0x0b11;
	else if ((dwID&0x0000ff00) == 0x00000400)
		wID = 0x0b21;
	
	wTask = (WORD)(*(pbRxStatu+2))-1;
	ReadItemEx(BN0, wTask, wID, bTaskCfgTempBuf);//��0xE0000300+wTask�Ķ�Ӧ��ֵ����bTaskCfgTempBuf��

	//2014-09-06  ���
	if (wID == 0x0b21)
	{
		memcpy(&tFwdTaskCfg, bTaskCfgTempBuf, sizeof(TFwdTaskCfg));
		FwdToComTaskPara(&tComTaskCfg, &tFwdTaskCfg);		
		memcpy(bTaskCfgTempBuf, &tComTaskCfg, sizeof(TFwdTaskCfg));
	}
	else if (wID == 0x0b11)
		memcpy(&tComTaskCfg, bTaskCfgTempBuf, sizeof(TFwdTaskCfg));

	
	if (bTaskCfgTempBuf[0] == 0)					//������Ч�Ա�־
	{	
		//SignalSemaphore(g_semEsam);
		return 0;
	}
	
	switch(bTaskCfgTempBuf[14])
	{
	case 0:
		bSmplIntervU = TIME_UNIT_MINUTE;
		break;
	case 1:
		bSmplIntervU = TIME_UNIT_HOUR;
		break;
	case 2:
		bSmplIntervU = TIME_UNIT_DAY;
		break;
	case 3:
		bSmplIntervU = TIME_UNIT_MONTH;
		break;
	default:
		bSmplIntervU = TIME_UNIT_MINUTE;
		break;
	}
	bSmplIntervV = bTaskCfgTempBuf[15];

	iCount = 0;

	*pbTxStatu++ = *pbRxStatu++;
	*pbTxStatu++ = *pbRxStatu++;
	*pbTxStatu++ = *pbRxStatu++;
	*pbTxStatu++ = *pbRxStatu++;
	*pbTxStatu++ = *pbRxStatu++;
	*pbTxStatu++ = *pbRxStatu++;

	tmSch.nYear = BcdToWORD(pbRxStatu);
	pbRxStatu += 2;
	tmSch.nMonth = BcdToByte(*pbRxStatu++);
	tmSch.nDay = BcdToByte(*pbRxStatu++);
	tmSch.nHour = BcdToByte(*pbRxStatu++);
	tmSch.nMinute =BcdToByte(*pbRxStatu++);
	tmSch.nSecond = 0;
	dwSchTime = TimeToSeconds(&tmSch);						//��ʼʱ��

	//2014-09-06  ������ʼʱ��Ϊ��ӽ�����ʱ����Ǹ����ݵ��ʱ��
	memset(&tBaseSampleTime, 0, sizeof(TTime));
	Fmt15ToTime((BYTE *)&tComTaskCfg.bComSampBasTime, &tBaseSampleTime);
	if (dwSchTime <= TimeToSeconds(&tBaseSampleTime))
	{
		dwSchTime = TimeToSeconds(&tBaseSampleTime);
	}
	else
	{
		iPast = TaskIntervsPast(&tBaseSampleTime, &tmSch, tComTaskCfg.bComSampIntervU, tComTaskCfg.bComSampIntervV);

		
		if (tComTaskCfg.bComSampIntervU != TIME_UNIT_MONTH_TASK)
		{			
			if (tComTaskCfg.bComSampIntervU == TIME_UNIT_MINUTE_TASK)	dwRealIntervU = 60;
			if (tComTaskCfg.bComSampIntervU == TIME_UNIT_HOUR_TASK)		dwRealIntervU = 60*60;
			if (tComTaskCfg.bComSampIntervU == TIME_UNIT_DAY_TASK)		dwRealIntervU = 60*60*24;
				
			if ((TimeToSeconds(&tmSch) - TimeToSeconds(&tBaseSampleTime))%(dwRealIntervU*tComTaskCfg.bComSampIntervV) != 0)				
				AddIntervsInTask(&tBaseSampleTime, tComTaskCfg.bComSampIntervU, tComTaskCfg.bComSampIntervV*(iPast+1));
			else
				AddIntervsInTask(&tBaseSampleTime, tComTaskCfg.bComSampIntervU, tComTaskCfg.bComSampIntervV*(iPast/*+1*/));
		}
		//�µ�����������(��ȡÿ���µ�ʱ����ǰǰ���Ĳ���ʱ�䲻һ����
		//�е���29�죬�е�30�죬�е�31��)
		//	AddIntervsInTask(&tBaseSampleTime, tComTaskCfg.bComSampIntervU, tComTaskCfg.bComSampIntervV*(iPast/*+1*/));
		
		dwSchTime = TimeToSeconds(&tBaseSampleTime);
	}
	
	tmSch.nYear = BcdToWORD(pbRxStatu);
	pbRxStatu += 2;
	tmSch.nMonth = BcdToByte(*pbRxStatu++);
	tmSch.nDay = BcdToByte(*pbRxStatu++);
	tmSch.nHour = BcdToByte(*pbRxStatu++);
	tmSch.nMinute =BcdToByte(*pbRxStatu++);
	tmSch.nSecond = 0;
	dwLastTime = TimeToSeconds(&tmSch);						//����ʱ��

	wDensity = *pbRxStatu++;								//�����ܶ�
	dwTmIntervs = IntervsToMinutes(bSmplIntervU, bSmplIntervV)*60;//��
	fInvaildDensity = false;

	switch (wDensity)										//�������ܶȻ��ʱ����
	{
	case 0:
		//2014-06-02  �޸�(�ܶ�Ϊ0  ʱ�����������õĳ�ȡ���ʳ�ȡ)
		//dwInterRate = dwTmIntervs*bSndFreq;
		dwInterRate = dwTmIntervs*bSndFreq*tComTaskCfg.bComRatio;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		dwInterRate = (wDensity==1 ? 1 : wDensity==2 ? 5 : wDensity==3 ? 15 : wDensity==4 ? 30 : 60);
		dwInterRate *= 60;
		if(dwInterRate < dwTmIntervs)
			fInvaildDensity = true;
		break;
	case 6:
		dwInterRate = 24 * 60 * 60;
		break;
	case 7:
		if (bSmplIntervU!=TIME_UNIT_MONTH || bSmplIntervV>1)
			fInvaildDensity = true;
		dwInterRate = 0;
		break;
	default:		
		//SignalSemaphore(g_semEsam);
		return 0;
	}


	iDataCodeLen = 0;
	iAllDataCodeLen = 0;
	bTaski=0;
	bTaskj=0;
	dwLastSchTime=0;

	if (*(pbRxStatu-16) == 0x03)							//��ͨ����
	{
		bFn = wTask+FN_COMSTAT;
		do
		{
			if (bTaskCfgTempBuf[8]==0)						//���ݽṹ��ʽ 0��ʾ��������ʽ��֯����
			{
				iDataSize = 3;
				iAllDataCodeLen = 0;
				if (dwInterRate!=0)
				{
					SecondsToTime(dwSchTime,&tmSch);

					//2014-09-06  �޸�Ϊǰ��N  ����Ȼ�£������ǹ̶���ֵ
					if (tComTaskCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)
					{
						memset(&tTime, 0, sizeof(TTime));
						Fmt15ToTime((BYTE *)&tComTaskCfg.bComSampBasTime, &tTime);	//������׼ʱ��
						dwMonths = MonthsPast(&tTime, &tmSch);				//Ҫ��ȡ�����ݵ�ʱ������ڲ�����׼ʱ��������Ȼ��
						if (0 != dwMonths%tComTaskCfg.bComSampIntervV)
							dwComSampCycle = dwMonths/tComTaskCfg.bComSampIntervV*tComTaskCfg.bComSampIntervV + tComTaskCfg.bComSampIntervV*(*rdwRecCnt) + 1;
						else
							dwComSampCycle = dwMonths/tComTaskCfg.bComSampIntervV*tComTaskCfg.bComSampIntervV + tComTaskCfg.bComSampIntervV*(*rdwRecCnt);
						AddIntervsInTask(&tTime, tComTaskCfg.bComSampIntervU, dwComSampCycle); 
						dwLastSchTime = TimeToSeconds(&tTime);
					}
					else
						dwLastSchTime = TimeToSeconds(&tmSch)+dwInterRate*(*rdwRecCnt);

					if (dwLastSchTime < dwLastTime)
					{
						SecondsToTime(dwLastSchTime,&tmSch);
						if (TdbOpenTable(bFn) != TDB_ERR_OK)
						{
							TdbCloseTable(bFn);							
							//SignalSemaphore(g_semEsam);
							return -2;
						}

						//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
						memset(pbRdBuf, 0, 1024);

						iRet = TdbReadRec (bFn, wTask, tmSch, pbRdBuf);
						TdbCloseTable(bFn);
						if (iRet > 0)
						{
							if (!fReadData)					//��̽�Ƿ�����һ֡������У����ش���0����
							{
								//SignalSemaphore(g_semEsam);
								return 1;
							}
							
							*pbTxStatu++ = 0;				//���ݽṹ��ʽ
							*pbTxStatu++ = bTaskCfgTempBuf[19]*bTaskCfgTempBuf[19+1+2*bTaskCfgTempBuf[19]];		//����������n��m��
							*pbTxStatu++ = 0;

							for (bTaski=0; bTaski<bTaskCfgTempBuf[19]; bTaski++)								//��Ϣ���ʶ����n
							{
								for (bTaskj=0; bTaskj<bTaskCfgTempBuf[19+1+2*bTaskCfgTempBuf[19]]; bTaskj++)	//���ݱ�ʶ��������m
								{
									memcpy(pbTxStatu, bTaskCfgTempBuf+19+1+2*bTaski, 2);						//��Ϣ���ʶ
									pbTxStatu += 2;
									memcpy(pbTxStatu, bTaskCfgTempBuf+19+1+2*bTaskCfgTempBuf[19]+1+4*bTaskj, 4);//���ݱ�ʶ����
									pbTxStatu += 4;
									memcpy(&dwID, bTaskCfgTempBuf+19+1+2*bTaskCfgTempBuf[19]+1+4*bTaskj, 4);
									iDataCodeLen = GetItemLenDw(BN0, dwID);										//ȡ�����ݱ�ʶ�����Ӧ���ֽڳ��� �μ���¼C ����iDataCodeLen
									memcpy(pbTxStatu, pbRdBuf+iAllDataCodeLen, iDataCodeLen);				//���ݱ�ʶ����
									pbTxStatu += iDataCodeLen;
									iAllDataCodeLen += iDataCodeLen;

									//ʱ���ʽΪ5�ֽڣ���Ϊ1�ֽ�
									*pbTxStatu++ = ByteToBcd(tmSch.nYear-2000);
									*pbTxStatu++ = ByteToBcd(tmSch.nMonth);
									*pbTxStatu++ = ByteToBcd(tmSch.nDay);
									*pbTxStatu++ = ByteToBcd(tmSch.nHour);
									*pbTxStatu++ = ByteToBcd(tmSch.nMinute);

									iDataSize = iDataSize+2+4+iDataCodeLen+5;
								}
							}
							//6.1.6.1ʱ���ʽΪ6�ֽڣ���Ϊ2�ֽ�
							*pbTxStatu++ = 0x20;
							*pbTxStatu++ = ByteToBcd(tmSch.nYear-2000);
							*pbTxStatu++ = ByteToBcd(tmSch.nMonth);
							*pbTxStatu++ = ByteToBcd(tmSch.nDay);
							*pbTxStatu++ = ByteToBcd(tmSch.nHour);
							*pbTxStatu++ = ByteToBcd(tmSch.nMinute);

							iDataSize = iDataSize+6;
							iFrmSize = iFrmSize+iDataSize;
						}
						//SignalSemaphore(g_semExFlashBuf);
					}
					else
						break;

					*rdwRecCnt = *rdwRecCnt + 1;
				}
			}
			else if (bTaskCfgTempBuf[8]==1)					//���ݽṹ��ʽ 1��ʾ������������ݸ�ʽ��֯����
			{
				iDataSize = 1;
				iAllDataCodeLen = 0;
				if (dwInterRate!=0)
				{
					SecondsToTime(dwSchTime,&tmSch);

					//2014-09-06  �޸�Ϊǰ��N  ����Ȼ�£������ǹ̶���ֵ
					if (tComTaskCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)
					{
						memset(&tTime, 0, sizeof(TTime));
						Fmt15ToTime((BYTE *)&tComTaskCfg.bComSampBasTime, &tTime);	//������׼ʱ��
						dwMonths = MonthsPast(&tTime, &tmSch);				//Ҫ��ȡ�����ݵ�ʱ������ڲ�����׼ʱ��������Ȼ��
						if (0 != dwMonths%tComTaskCfg.bComSampIntervV)
							dwComSampCycle = dwMonths/tComTaskCfg.bComSampIntervV*tComTaskCfg.bComSampIntervV + tComTaskCfg.bComSampIntervV*(*rdwRecCnt) + 1;
						else
							dwComSampCycle = dwMonths/tComTaskCfg.bComSampIntervV*tComTaskCfg.bComSampIntervV + tComTaskCfg.bComSampIntervV*(*rdwRecCnt);
						AddIntervsInTask(&tTime, tComTaskCfg.bComSampIntervU, dwComSampCycle); 
						dwLastSchTime = TimeToSeconds(&tTime);
					}
					else
						dwLastSchTime = TimeToSeconds(&tmSch)+dwInterRate*(*rdwRecCnt);

					if (dwLastSchTime < dwLastTime)
					{
						SecondsToTime(dwLastSchTime,&tmSch);
						if (TdbOpenTable(bFn) != TDB_ERR_OK)
						{
							TdbCloseTable(bFn);							
							//SignalSemaphore(g_semEsam);
							return -2;
						}

						//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
						memset(pbRdBuf, 0, 1024);

						iRet = TdbReadRec (bFn, wTask, tmSch, pbRdBuf);
						TdbCloseTable(bFn);
						if (iRet > 0)
						{
							if (!fReadData)					//��̽�Ƿ�����һ֡������У����ش���0����
							{
								//SignalSemaphore(g_semEsam);
								return 1;
							}

							*pbTxStatu++ = 1;				//���ݽṹ��ʽ

							for (bTaski=0; bTaski<bTaskCfgTempBuf[19]; bTaski++)								//��Ϣ���ʶ����n
							{
								for (bTaskj=0; bTaskj<bTaskCfgTempBuf[19+1+2*bTaskCfgTempBuf[19]]; bTaskj++)	//���ݱ�ʶ��������m
								{
									if (bTaskj == 0)
									{
										memcpy(pbTxStatu, bTaskCfgTempBuf+19+1+2*bTaski, 2);						//��Ϣ���ʶ
										pbTxStatu += 2;
									}
									memcpy(&dwID, bTaskCfgTempBuf+19+1+2*bTaskCfgTempBuf[19]+1+4*bTaskj, 4);
									iDataCodeLen = GetItemLenDw(BN0, dwID);										//ȡ�����ݱ�ʶ�����Ӧ���ֽڳ��� �μ���¼C ����iDataCodeLen
									memcpy(pbTxStatu, pbRdBuf+iAllDataCodeLen, iDataCodeLen);				//���ݱ�ʶ����
									pbTxStatu += iDataCodeLen;
									iAllDataCodeLen += iDataCodeLen;

									if (bTaskj == 0)
										iDataSize = iDataSize+2+iDataCodeLen;
									else
										iDataSize = iDataSize+iDataCodeLen;
								}
							}

							//6.1.6.1ʱ���ʽΪ6�ֽڣ���Ϊ2�ֽ�
							*pbTxStatu++ = 0x20;
							*pbTxStatu++ = ByteToBcd(tmSch.nYear-2000);
							*pbTxStatu++ = ByteToBcd(tmSch.nMonth);
							*pbTxStatu++ = ByteToBcd(tmSch.nDay);
							*pbTxStatu++ = ByteToBcd(tmSch.nHour);
							*pbTxStatu++ = ByteToBcd(tmSch.nMinute);

							iDataSize = iDataSize+6;
							iFrmSize = iFrmSize+iDataSize;
						}
						//SignalSemaphore(g_semExFlashBuf);
					}
					else
						break;

					*rdwRecCnt = *rdwRecCnt + 1;
				}
			}
			iMaxFrmSize = iMaxFrmSize - iDataSize;

			if (iMaxFrmSize<0)
			{
				pbTxStatu = pbTxStatu - iDataSize;
				*rdwRecCnt = *rdwRecCnt - 1;
				iFrmSize = iFrmSize - iDataSize;
			}
			Sleep(10);
		}while (iMaxFrmSize>=0);
	}
	else if (*(pbRxStatu-16) == 0x04)						//�м�����
	{
		bFn = wTask+FN_FWDSTAT;
		do
		{
			//2014-09-06  ���
			iDataSize = 0;
			
			if (dwInterRate!=0)
			{
				SecondsToTime(dwSchTime,&tmSch);

				//2014-09-06  �޸�Ϊǰ��N  ����Ȼ�£������ǹ̶���ֵ
				if (tComTaskCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)
				{
					memset(&tTime, 0, sizeof(TTime));
					Fmt15ToTime((BYTE *)&tComTaskCfg.bComSampBasTime, &tTime);	//������׼ʱ��
					dwMonths = MonthsPast(&tTime, &tmSch);				//Ҫ��ȡ�����ݵ�ʱ������ڲ�����׼ʱ��������Ȼ��
					if (0 != dwMonths%tComTaskCfg.bComSampIntervV)
						dwComSampCycle = dwMonths/tComTaskCfg.bComSampIntervV*tComTaskCfg.bComSampIntervV + tComTaskCfg.bComSampIntervV*(*rdwRecCnt) + 1;
					else
						dwComSampCycle = dwMonths/tComTaskCfg.bComSampIntervV*tComTaskCfg.bComSampIntervV + tComTaskCfg.bComSampIntervV*(*rdwRecCnt);
					AddIntervsInTask(&tTime, tComTaskCfg.bComSampIntervU, dwComSampCycle); 
					dwLastSchTime = TimeToSeconds(&tTime);
				}
				else
					dwLastSchTime = TimeToSeconds(&tmSch)+dwInterRate*(*rdwRecCnt);

				if (dwLastSchTime < dwLastTime)
				{
					SecondsToTime(dwLastSchTime,&tmSch);
					if (TdbOpenTable(bFn) != TDB_ERR_OK)
					{
						TdbCloseTable(bFn);						
						//SignalSemaphore(g_semEsam);
						return -2;
					}

					//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
					memset(pbRdBuf, 0, 1024);

					iRet = TdbReadRec (bFn, wTask, tmSch, pbRdBuf);
					TdbCloseTable(bFn);
					if (iRet > 0)
					{
						if (!fReadData)					//��̽�Ƿ�����һ֡������У����ش���0����
						{
							//SignalSemaphore(g_semEsam);
							return 1;
						}

						*pbTxStatu = bTaskCfgTempBuf[8];
						*(pbTxStatu+1) = pbRdBuf[0];
						memcpy(pbTxStatu+2, pbRdBuf+1, pbRdBuf[0]);

						iDataSize = iDataSize+pbRdBuf[0]+2;
						pbTxStatu += iDataSize;

						//6.1.6.1ʱ���ʽΪ6�ֽڣ���Ϊ2�ֽ�
						*pbTxStatu++ = 0x20;
						*pbTxStatu++ = ByteToBcd(tmSch.nYear-2000);
						*pbTxStatu++ = ByteToBcd(tmSch.nMonth);
						*pbTxStatu++ = ByteToBcd(tmSch.nDay);
						*pbTxStatu++ = ByteToBcd(tmSch.nHour);
						*pbTxStatu++ = ByteToBcd(tmSch.nMinute);

						iDataSize = iDataSize+6;
						iFrmSize = iFrmSize+iDataSize;
					}
					//SignalSemaphore(g_semExFlashBuf);
				}
				else
					break;

				*rdwRecCnt = *rdwRecCnt + 1;
			}

			iMaxFrmSize = iMaxFrmSize - iDataSize;

			if (iMaxFrmSize<0)
			{
				pbTxStatu = pbTxStatu - iDataSize;
				*rdwRecCnt = *rdwRecCnt - 1;
				iFrmSize = iFrmSize - iDataSize;
			}
			Sleep(10);
		}while (iMaxFrmSize>=0);
	}
	else
	{
		//SignalSemaphore(g_semEsam);
		return 0;
	}

	//SignalSemaphore(g_semEsam);
	return iFrmSize;
}

//����������dwID�ҵ���Ӧ��bFn
//�������壺dwID:���еĳ���ID��4 �ֽ�
//          
//����ֵ��  ���Ӧ��bFnֵ
//          0�������ڴ�������
BYTE FindFn(DWORD dwID)
{
	bool fFound = false;
	BYTE bFn;
	WORD i;

	for (i=0; i<COMM_TASK_NUM; i++)
	{
		if ((dwID == g_taskCtrl[i].dwID) || ((dwID | 0xff) == g_taskCtrl[i].dwID) || ((dwID | 0xff00) == g_taskCtrl[i].dwID)) 
		{
			fFound = true;
			bFn = g_taskCtrl[i].bFN;
			break;
		}
		else
			continue;
		
	}
	if (!fFound)
	{
		return 0;
	}

	return bFn;
}

/*************************************�������֡���End*********************************************/
