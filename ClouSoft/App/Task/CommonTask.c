/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CommonTask.c
 * 摘    要：本文件主要实现任务数据库普通任务的数据采集
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
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

//#define	CLASS1_FN_NUM	32				//一类小时冻结数据映射到二类曲线数据的数量
#define SINGL_METER  1
#define MULTI_METER  2
#define OTHER_METER  3

#define NOM_METER  0
#define VIP_METER  1

BYTE g_bReadData[1024]; 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ComTask全局数据定义
//普通任务的控制结构:每两行一项
static const TCommTaskCtrl g_taskCtrl[] =
{//	bFN		IDCnt		wID				bLen		上行通信ID			间隔单位			记录个数		测量点特征字
	//DayFrozen    抄电表自身日冻结
	{1,		1,			{0x3761},		21,			0x050601ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结正向有功电能示值（总，各费率）
	{2,		1,			{0x3762},		21,			0x050603ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结正向无功电能示值（总，各费率）
	{3,		1,			{0x3763},		21,			0x050602ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结反向有功电能示值（总，各费率）
	{4,		1,			{0x3764},		21,			0x050604ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结反向无功电能示值（总，各费率）
	{5,		1,			{0x3765},		21,			0x050605ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结一象限无功电能示值（总，各费率）
	{6,		1,			{0x3766},		21,			0x050606ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结二象限无功电能示值（总，各费率）
	{7,		1,			{0x3767},		21,			0x050607ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结三象限无功电能示值（总，各费率）
	{8,		1,			{0x3768},		21,			0x050608ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //日冻结四象限无功电能示值（总，各费率）
	//DayFrozen    抄电表当前数据作为日冻结
	{1,		1,			{0x166f},		21,			0x050601ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结正向有功电能示值（总，各费率）
	{2,		1,			{0x167f},		21,			0x050603ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结正向无功电能示值（总，各费率）
	{3,		1,			{0x168f},		21,			0x050602ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结反向有功电能示值（总，各费率）
	{4,		1,			{0x169f},		21,			0x050604ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结反向无功电能示值（总，各费率）
	{5,		1,			{0x16af},		21,			0x050605ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结一象限无功电能示值（总，各费率）
	{6,		1,			{0x16bf},		21,			0x050606ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结二象限无功电能示值（总，各费率）
	{7,		1,			{0x16cf},		21,			0x050607ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结三象限无功电能示值（总，各费率）
	{8,		1,			{0x16df},		21,			0x050608ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //日冻结四象限无功电能示值（总，各费率）

	//电能表状态字、日冻结通信流量，不管‘抄电表当前’或‘抄电表冻结’，都用同一个ID，
	{9,		1,			{0xc86f},		14,			0x040005ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //电表状态字数据块
	{9,		1,			{0xc86f},		14,			0x040005ff,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //电表状态字数据块

	{10,	1,			{0x886b},		2,			0xe1800012,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_P0,	TSK_DMCUR}}, //日通信流量
	{10,	1,			{0x886b},		2,			0xe1800012,			TIME_UNIT_DAY,		31,		{TASK_PN_TYPE_P0,	TSK_DMMTR}}, //日通信流量

	//MonthFrozen  抄电表自身月冻结
	{11,	1,			{0x3793},		41,		0x0101ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMMTR}}, //月正向有功最大需量及发生时间
	{12,	1,			{0x3795},		41,		0x0102ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMMTR}}, //月反向有功最大需量及发生时间
	{13,	1,			{0x3777},		21,		0x0001ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结正向有功电能示值（总，各费率）
	{14,	1,			{0x3778},		21,		0x0003ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结正向无功电能示值（总，各费率）
	{15,	1,			{0x3779},		21,		0x0002ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结反向有功电能示值（总，各费率）
	{16,	1,			{0x3780},		21,		0x0004ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结反向无功电能示值（总，各费率）
	{17,	1,			{0x3781},		21,		0x0005ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结一象限无功电能示值（总，各费率）
	{18,	1,			{0x3782},		21,		0x0006ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结二象限无功电能示值（总，各费率）
	{19,	1,			{0x3783},		21,		0x0007ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结三象限无功电能示值（总，各费率）
	{20,	1,			{0x3784},		21,		0x0008ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月冻结四象限无功电能示值（总，各费率）

	//MonthFrozen  抄电表当前作为月冻结
	{11,	1,			{0x20cf},		41,		0x0101ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMCUR}}, //月正向有功最大需量及发生时间
	{12,	1,			{0x20df},		41,		0x0102ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMDMCUR}}, //月反向有功最大需量及发生时间
	{13,	1,			{0x166f},		21,		0x0001ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结正向有功电能示值（总，各费率）
	{14,	1,			{0x167f},		21,		0x0003ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结正向无功电能示值（总，各费率）
	{15,	1,			{0x168f},		21,		0x0002ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结反向有功电能示值（总，各费率）
	{16,	1,			{0x169f},		21,		0x0004ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结反向无功电能示值（总，各费率）
	{17,	1,			{0x16af},		21,		0x0005ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结一象限无功电能示值（总，各费率）
	{18,	1,			{0x16bf},		21,		0x0006ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结二象限无功电能示值（总，各费率）
	{19,	1,			{0x16cf},		21,		0x0007ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结三象限无功电能示值（总，各费率）
	{20,	1,			{0x16df},		21,		0x0008ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月冻结四象限无功电能示值（总，各费率）


	//MonthFrozen  抄电表上一结算日
	{11,	1,			{0x3789},		41,		0x0101ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMMSETT}}, //月正向有功最大需量及发生时间
	{12,	1,			{0x3791},		41,		0x0102ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DEMMSETT}}, //月反向有功最大需量及发生时间
	{13,	1,			{0x3877},		21,		0x0001ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结正向有功电能示值（总，各费率）
	{14,	1,			{0x3878},		21,		0x0003ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结正向无功电能示值（总，各费率）
	{15,	1,			{0x3879},		21,		0x0002ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结反向有功电能示值（总，各费率）
	{16,	1,			{0x3880},		21,		0x0004ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结反向无功电能示值（总，各费率）
	{17,	1,			{0x3881},		21,		0x0005ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结一象限无功电能示值（总，各费率）
	{18,	1,			{0x3882},		21,		0x0006ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结二象限无功电能示值（总，各费率）
	{19,	1,			{0x3883},		21,		0x0007ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结三象限无功电能示值（总，各费率）
	{20,	1,			{0x3884},		21,		0x0008ff01,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月冻结四象限无功电能示值（总，各费率）

	{21,	1,			{0x2200},		27,		0xe100c010,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMMTR}}, //月电压越限统计数据
	{21,	1,			{0x2200},		27,		0xe100c010,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_DMCUR}}, //月电压越限统计数据
	{21,	1,			{0x2200},		27,		0xe100c010,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_MTR,	TSK_MSETT}}, //月电压越限统计数据

	{22,	1,			{0x886d},		3,		0xe1800014,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_P0,	TSK_DMMTR}}, //月通信流量
	{22,	1,			{0x886d},		3,		0xe1800014,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_P0,	TSK_DMCUR}}, //月通信流量
	{22,	1,			{0x886d},		3,		0xe1800014,			TIME_UNIT_MONTH,	12,		{TASK_PN_TYPE_P0,	TSK_MSETT}}, //月通信流量

/*
	//曲线数据   抄电表冻结曲线   
	//暂时先不做抄电表冻结曲线
	{23,	1,			{0x3701},		21,		0x0001ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //整点正向有功电能示值（总，各费率）
	{24,	1,			{0x3702},		21,		0x0003ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //整点正向无功电能示值（总，各费率）
	{25,	1,			{0x3703},		21,		0x0002ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //整点反向有功电能示值（总，各费率）
	{26,	1,			{0x3704},		21,		0x0004ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //整点反向无功电能示值（总，各费率）
	{27,	1,			{0xb61f},		18,		0x0201ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //整点电压
	{28,	1,			{0xb62f},		18,		0x0202ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //整点电流
	{29,	1,			{0xb65f},		2,		0x02060000,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFMTR}}, //整点总功率因数示值
*/	

	//曲线数据   抄电表当前数据
	{23,	1,			{0x166f},		21,		0x0001ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //整点正向有功电能示值（总，各费率）
	{24,	1,			{0x167f},		21,		0x0003ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //整点正向无功电能示值（总，各费率）
	{25,	1,			{0x168f},		21,		0x0002ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //整点反向有功电能示值（总，各费率）
	{26,	1,			{0x169f},		21,		0x0004ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //整点反向无功电能示值（总，各费率）
	{27,	1,			{0xb61f},		18,		0x0201ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //整点电压
	{28,	1,			{0xb62f},		18,		0x0202ff00,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //整点电流
	{29,	1,			{0xb65f},		2,		0x02060000,			TIME_UNIT_HOUR,		240,	{TASK_PN_TYPE_MTR,	TSK_PFCUR}}, //整点总功率因数示值
};
#define COMM_TASK_NUM (sizeof(g_taskCtrl)/sizeof(TCommTaskCtrl))


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ComTask实现

//描述：通过FN取得记录控制结构
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

//描述：通过FN取得记录控制结构
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
 		return 4;	//加上时间(年月日)和测量点号(BYTE)
 	case TIME_UNIT_MONTH:
 		return 3;	//加上时间(年月)和测量点号(BYTE)

 	case TIME_UNIT_HOUR:
 		return  4;	//加上时间(年月日)和测量点号(BYTE)
 	}
}

//描述：通过记录控制结构取得记录数据部分的长度
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

//描述：通过记录控制结构取得记录的长度
WORD ComTaskGetRecSize(const TCommTaskCtrl* pTaskCtrl)
{
	WORD wRecSize;

	if (pTaskCtrl == NULL)
		return 0;
	
	wRecSize = ComTaskGetDataLen(pTaskCtrl);

	wRecSize += ComTaskGetRecOffset(pTaskCtrl);

	return wRecSize;
}

//描述：通过记录控制结构取得记录的笔数
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


//描述：通过记录控制结构取得记录保存的月份数
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


//描述：通过FN取得记录的长度
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

	pRecTm->nSecond = 0;	//不考虑秒
	tmStart = *pRecTm;
	tmEnd = *pRecTm;

	switch (pTaskCtrl->bIntervU)
	{
	case TIME_UNIT_HOUR:	//小时
		pRecTm->nMinute = 0;
		
		//求开始时间
		tmStart.nMinute = 0;

		//求结束时间
		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, 1);
		break;

	case TIME_UNIT_DAY:		//日冻结限定在日切换后一个小时内抄完
		//if (pRecTm->nHour != 0)
		//	return false;

		//pRecTm->nHour = 0;
		//pRecTm->nMinute = 0;

		//求开始时间
		tmStart.nHour = 0;
		tmStart.nMinute = 0;
		tmStart.nSecond = 0;

		//求结束时间
		tmEnd.nHour = 23;
		tmEnd.nMinute = 59;
		tmEnd.nSecond = 59;

		break;
	/*case TIME_UNIT_DAYFLG:		
		GetPnDate(wPn, bBuf);	 //GetMtrDate(PORT_GB485, bBuf);

		dwDayFlg = ByteToDWord(bBuf);
		if ((dwDayFlg & (1<<(pRecTm->nDay-1))) == 0) //不是抄表日
			return false;	//当前时间不符合抄读要求,返回false不抄

		tmEnd.nHour = BcdToByte(bBuf[5]);
		tmEnd.nMinute = BcdToByte(bBuf[4]);
		pRecTm->nSecond = 30; //保证到达起始分钟能执行
		dwDayFlg = SecondsPast(&tmEnd, pRecTm);
		if (dwDayFlg==0 || dwDayFlg>3600*2)
			return false;	//当前时间不符合抄读要求
		//求抄表的开始时间
		tmStart.nHour = BcdToByte(bBuf[5]);		//对小时进行限制，再规定小时执行
		tmStart.nMinute = BcdToByte(bBuf[4]);		//分钟
		tmStart.nSecond = 0;

		wIntervV = GetMeterInterv(wPn);
        if (wIntervV == 0)  //必须防止除0
            return false; 
		dwDayFlg = TimeToMinutes(&tmStart)/wIntervV*wIntervV;
		MinutesToTime(dwDayFlg, &tmStart);
		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, 2);
		break;	*/
	case TIME_UNIT_MONTH:
		if (pRecTm->nDay!=1 || pRecTm->nHour!=0)
			return false;	//当前时间不符合抄读要求,返回false不抄

		//求开始时间
		tmStart.nDay = 1;	//月冻结限死在1号冻结,国网要求在月末进行冻结
		tmStart.nHour = 0;		//pRdCtrl->tmStartTime.nHour;
		tmStart.nMinute = 0;

		//求结束时间:当月1日23:59
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


//描述：判断普通任务是否需要执行
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
		ReadItemEx(BN24, PN0, 0x4110, &bProfMode);	//0x4110 1 曲线冻结模式字,0抄电表当前数据；1抄电表曲线
// 		if (pTaskCtrl->bFN<101 || pTaskCtrl->bFN>104)
// 			bProfMode = 0; //07表除电量示值曲线目前只支持抄当前

		bDayMode = 0;
		ReadItemEx(BN24, PN0, 0x4111, &bDayMode); //0x4111 1 日冻结模式字,0抄电表当前数据；1抄电表冻结

		bDayFlgMode = 0;
		ReadItemEx(BN24, PN0, 0x4112, &bDayFlgMode); //0x4112 1 抄表日冻结模式字,0抄电表当前数据；1抄电表冻结

		bDemDayMode = 0;
		ReadItemEx(BN24, PN0, 0x4113, &bDemDayMode); //0x4113 1 需量日月冻结模式字,0抄电表当前数据；1抄电表冻结

		bDemDayFlgMode = 0;
		ReadItemEx(BN24, PN0, 0x4114, &bDemDayFlgMode); //0x4114 1 需量抄表日月冻结模式字,0抄电表当前数据；1抄电表冻结
        
   		bMonSettMode = 0;
		ReadItemEx(BN24, PN0, 0x4115, &bMonSettMode); //0x4115 1 月冻结模式字,0抄电表当前数据；1抄电表冻结
        
   		bDemMonSettMode = 0;
		ReadItemEx(BN24, PN0, 0x4116, &bDemMonSettMode); //0x4116 1 需量月冻结模式字,0抄电表当前数据；1抄电表日冻结；2抄电表结算日

		switch(pTaskCtrl->bPnChar[1])
		{
		case TSK_DMCUR:		//DAY&MONTH FROZEN CURRENT 在日月冻结配置成抄电表当前数据时抄读
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //月冻结抄结算日
			{
				if (bMonSettMode==0 || (bMonSettMode!=2 && !IsV07Mtr(wPn)))
					return true;
				else
					return false;
			}

			if (bDayMode==0 || !IsV07Mtr(wPn))	//0x3004 1 日月冻结模式字,0抄电表当前数据；1抄电表冻结
				return true;
			else
				return false;

		case TSK_DMMTR:		//DAY&MONTH FROZEN METER FRZ 在日月冻结配置成抄电表冻结时抄读
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //月冻结抄结算日
			{
				if (bMonSettMode==1 && IsV07Mtr(wPn))
					return true;
				else
					return false;
			}

			if (bDayMode==1 && IsV07Mtr(wPn))	//0x3004 1 日月冻结模式字,0抄电表当前数据；1抄电表冻结
				return true;
			else
				return false;
            
		case TSK_MSETT:            
	            if (pTaskCtrl->bIntervU==TIME_UNIT_MONTH && bMonSettMode==2)    //月冻结抄结算日
				return true;
	            else
				return false;


		case TSK_PFCUR:		//PROFILE FROZEN CURRENT 在曲线配置成抄电表当前数据时抄读
//			if (bProfMode==0 || !IsV07Mtr(wPn))	//0x3003 1 曲线冻结模式字,0抄电表当前数据；1抄电表曲线
				return true;
// 			else
// 				return false;


		case TSK_DEMDMCUR:	//DEMAND DAY&MONTH FROZEN CURRENT 需量在日月冻结配置成抄电表当前数据时抄读
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //月冻结抄结算日
			{
				if (bDemMonSettMode==0 || !IsV07Mtr(wPn))	//0x3006 1 需量日月冻结模式字,0抄电表当前数据；1抄电表冻结
					return true;
				else
					return false;
			}
            


		case TSK_DEMDMMTR:		//DEMAND DAY&MONTH FROZEN METER FRZ 需量在日月冻结配置成抄电表冻结时抄读
			if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)    //月冻结抄结算日
			{
				if (bDemMonSettMode==1 && IsV07Mtr(wPn))	//0x3006 1 需量日月冻结模式字,0抄电表当前数据；1抄电表冻结
					return true;
				else
					return false;
			}

        
		case TSK_DEMMSETT:
			if (pTaskCtrl->bIntervU==TIME_UNIT_MONTH && bDemMonSettMode==2)    //需量月冻结抄结算日
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
        //AddIntervs(pRecTm, TIME_UNIT_DAY, -1);	//南网20号日冻结意义为19号23：59：59秒数据
   		TimeTo4Bcd(pRecTm, pbRec);
		pbRec += 4;
		*pbRec++ = wPn&0xff;
		break;		//加上时间(年月日时分)和测量点号(BYTE)
	case TIME_UNIT_MONTH:
        //AddIntervs(pRecTm, TIME_UNIT_MONTH, -1);	//南网20号日冻结意义为19号23：59：59秒数据
		TimeTo4Bcd(pRecTm, pbRec);
		pbRec += 4;
		*pbRec++ = wPn&0xff;
		break;		//加上时间(年月)和测量点号(BYTE)
	case TIME_UNIT_HOUR:
		TimeTo4Bcd(pRecTm, pbRec);
		pbRec += 4;
		*pbRec++ = wPn&0xff;
		break;	//加上时间(FMT15)和测量点号(BYTE)
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


//描述：执行测量点Pn的所有普通任务
//参数：@pTm 当前间隔的时间
//		@pdwRecTime 由导入导出线程提供的当前测量点的普通任务记录时间，
//					共定义64个，索引就是该测量点需要执行的任务的序号，随着F39或冻结模式字的改变序号可能会改变，
//					低3字节表示从BASETIME以来的小时，高字节表示相应的FN，用来防止序号的变动
bool DoComTask(WORD wPn, TTime* pTm, DWORD* pdwRecTime, int Vip_Num, BYTE bVip, BYTE bType)
{
	DWORD dwStartTm, dwEndTm, dwRecHour; //查询系统库用的时标
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

	for(i=0; i<COMM_TASK_NUM; i++)   //针对某个测量点 ，所有的任务都遍历一遍
	{
		const TCommTaskCtrl* pTaskCtrl = &g_taskCtrl[i];
		if (!ComTaskIsNeedToDo(wPn, pTaskCtrl))		//判断普通任务不需要执行
			continue;

		bFn = pTaskCtrl->bFN;
		//曲线数据判断，是重点户就执行对应的任务，反之不处理。重点户函数还未提供接口，后续处理
		if (bFn >= 23 && bFn <= 29)
			if ((bVip != VIP_METER) || (Vip_Num > MAX_VIP_METER))	//不是重点户或者超过10个重点户,不执行
				continue;

		if (bType == SINGL_METER)
			if (bFn != 1 && bFn != 3 && bFn != 9 && bFn != 13 && bFn != 15 && bFn != 23 && bFn != 25 )
				continue;

		tmRec = *pTm;
		if (ComTaskGetCurRecTime(wPn, pTaskCtrl, &tmRec, &dwStartTm, &dwEndTm)) //获取做任务的起止时间
		{
			if (pTaskCtrl->bIntervU==TIME_UNIT_DAY)
				dwRecHour = DaysFrom2000(&tmRec);
			else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
				dwRecHour = MonthFrom2000(&tmRec);
			else
				dwRecHour = TimeToHours(&tmRec);
			if (dwRecHour!=(*pdwRecTime&0xffffff) || pTaskCtrl->bFN!=(*pdwRecTime>>24))	//记录还没生成过
			{
				pwID = (WORD* )pTaskCtrl->wID;
				if (QueryItemTimeMid(dwStartTm, dwEndTm, BN0, wPn, pwID, pTaskCtrl->bIDCnt, bRecBuf, &wValidNum) > 0) //判断数据是否到达
				{
					memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));
					wRecOff = ComTaskGetRecOffset(pTaskCtrl);	//记录头偏移
					if (ReadItemMid(BN0, wPn, pwID, pTaskCtrl->bIDCnt, bRecBuf+5, dwStartTm, dwEndTm) > 0) //读多个ID
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

//描述：任务是否执行
//参数：@pTm 当前间隔的时间
//		@pdwRecTime 由导入导出线程提供的当前测量点的普通任务记录时间，
//					共定义64个，索引就是该测量点需要执行的任务的序号，随着F39或冻结模式字的改变序号可能会改变，
//					低3字节表示从BASETIME以来的小时，高字节表示相应的FN，用来防止序号的变动
bool IsComTaskDone(WORD wPn, BYTE bFn, TTime* pTm, DWORD* pdwRecTime)
{
	DWORD dwRecHour; //查询系统库用的时标
	WORD i;
	TTime tmRec = *pTm;

	for(i=0; i<COMM_TASK_NUM; i++)
	{
		const TCommTaskCtrl* pTaskCtrl = &g_taskCtrl[i];
		if (!ComTaskIsNeedToDo(wPn, pTaskCtrl))		//判断普通任务不需要执行
			continue;
		if (pTaskCtrl->bFN == bFn)
		{
			if (pTaskCtrl->bIntervU==TIME_UNIT_DAY || pTaskCtrl->bIntervU==TIME_UNIT_DAYFLG)
				dwRecHour = DaysFrom2000(&tmRec);
			else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
				dwRecHour = MonthFrom2000(&tmRec);
			else
				dwRecHour = TimeToHours(&tmRec);
			if (dwRecHour==(*pdwRecTime&0xffffff) && pTaskCtrl->bFN==(*pdwRecTime>>24))	//记录生成过
			{
				return true;
			}
		}

		pdwRecTime++;
	}

	return false;
}

//描述：分析该FN的最新动态，需要申请or需要释放
//参数：@bFN 需要分析的FN
bool IsFnSupByPn(BYTE bFn)
{
	WORD wPn;
	//日/月/曲线冻结数据
	if (bFn < FN_COMSTAT)
		return true;

	//普通任务
	if (bFn < FN_FWDSTAT)
	{
		//判断任务是否配置
		return IsTaskValid(COMMON_TASK_TYPE, bFn - FN_COMSTAT);
	}
	//中继任务
	if (bFn < FN_MAX)
	{
		//判断任务是否配置
		return IsTaskValid(FWD_TASK_TYPE, bFn - FN_FWDSTAT);
	}

	return false;
}



/*************************************补抄部分——Start*********************************************/
TTime g_tmOld;
BYTE g_bRdTimes[(PN_NUM+3)/4+1];	//为了节省内存，每个测量点占2bits


//补抄当天的曲线
//参数：pfSuccOnce 如果补抄成功过一个数据项，则返回true,否则返回false
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

	for (i=bReRdTimes; i>=0; i--) //补抄当天
	{
		tmNow.nHour = (BYTE )i;
		for (wTask=0; wTask<COMM_TASK_NUM; wTask++)
		{
			bRdState = GetRdMtrState(bThrId);	//取得当前的抄表状态
			if (bRdState != RD_ERR_OK)  //当前处于停止抄表或者直抄状态
				return bRdState;	

			if (!IsFnSupport(wPN, g_taskCtrl[wTask].bFN, 2))	//该测量点不支持FN)		//判断普通任务不需要执行
				continue;

			if (!IsSupPnType(g_taskCtrl[wTask].bPnChar[1]))
				continue;

			if (g_taskCtrl[wTask].bIntervU != TIME_UNIT_HOUR) 
				continue;

			bFN = g_taskCtrl[wTask].bFN;
			wIDs = g_taskCtrl[wTask].dwID;
			
			if ((bFN>81 && bFN<=88) || (bFN>89 && bFN<=94) || 
				(bFN>101 && bFN<=104) || (bFN>105 && bFN<=108) || 
				(bFN>145 && bFN<=148))	//负荷曲线的ID相同，只补抄一次
				continue;

			iRet = SchComTaskRec(bFN, wPN, tmNow, bRecBuf, TIME_UNIT_HOUR);
			if (iRet > 0)	//该笔记录已经存在
				continue;
			
			memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));
			iDirRet = DirReadFrz(0, bRecBuf, bFN, wPN, wIDs, &tmNow, pMtrPro, TIME_UNIT_HOUR); 
			if (iDirRet >0)//抄表成功
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

//参数：pfSuccOnce 如果补抄成功过一个数据项，则返回true,否则返回false
BYTE ReRdDayMonFrz(WORD wPN, struct TMtrPro* pMtrPro, BYTE bThrId, bool* pfSuccOnce)
{
	WORD wRecLen=0;
	WORD wTask = 0;
	WORD wIDs =  0;

	bool fPnStatus = true; //当前测量点的补抄状态
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

	ReadItemEx(BN24, PN0, 0x4111, &bDayFrzStatus); //07版电表日冻结模式字,对97版不起作用0:不读取电表冻结,终端自行冻结、1:读取电表自身冻结
	ReadItemEx(BN24, PN0, 0x4115, &bMonthSettMode); //月冻结模式 <示值部分>
	ReadItemEx(BN24, PN0, 0x4116, &bMonFrzStatus); //需量月冻结模式字  
	
												   

	GetCurTime(&now);
	now.nSecond = 0;
	now.nMinute = 0;
	now.nHour = 0;

	if (bDayFrzStatus == 1) //只有日冻结需要补抄时标
	{
		if (!GetMtrDayFrzIdx(pMtrPro, now, bDayFrzIdx))	//取得电表前3天日冻结的索引
		{
			if (bMonFrzStatus!=2 && bMonthSettMode!=2)
			{
				return RD_ERR_RDFAIL;	//抄读失败
			}
		}
			
	}

	for (wTask=0; wTask<COMM_TASK_NUM; wTask++)
	{
		tmNow = now;
		
		if (!IsFnSupport(wPN, g_taskCtrl[wTask].bFN, 2))	//该测量点不支持FN)		
			continue;

/*		if (!IsSupPnType(g_taskCtrl[wTask].bPnChar[1]))
		{
			continue;
		}
*/
		if (g_taskCtrl[wTask].bIntervU==TIME_UNIT_MONTH )//月冻结
		{
			if ((bMonFrzStatus==2 && g_taskCtrl[wTask].bPnChar[1]==TSK_DEMMSETT) || 
					(bMonthSettMode==2 && g_taskCtrl[wTask].bPnChar[1]==TSK_MSETT) )
			{
				bInterU = TIME_UNIT_MONTH;
				bTimeLen = 2;
				bRdTimes = 1; //上1抄表日
				tmNow.nDay = 0;
			}
			else
			{
				continue;
			}
			
		}
		else if (g_taskCtrl[wTask].bIntervU==TIME_UNIT_DAY && IsSupPnType(g_taskCtrl[wTask].bPnChar[1]))//日冻结
		{
			if (bDayFrzStatus != 1)
				continue;

			bInterU = TIME_UNIT_DAY;
			bTimeLen = 3;
			bRdTimes = 3; //上3天
		}
		else
		{
			continue; //其他类型的任务不进行补抄
		}

		for (j=0; j<bRdTimes; j++) 
		{
			bRdState = GetRdMtrState(bThrId);	//取得当前的抄表状态
			if (bRdState != RD_ERR_OK)  //当前处于停止抄表或者直抄状态
				return bRdState;	

			memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));

			pbRec = bRecBuf;
			pbRec += 7; //给一笔记录的冻结时标(5)和测量点号(2)留出7个字节的空间

			tmStart = tmNow;
			AddIntervs(&tmStart, bInterU, -(j+1));

			tmRec = tmStart;
			if (bDayFrzIdx[j]==INVALID_DATA && bInterU==TIME_UNIT_DAY)//没有当天的冻结数据
			{
				continue;
			}

			bFN = g_taskCtrl[wTask].bFN;
			wIDs = g_taskCtrl[wTask].dwID;
			wRecLen = ComTaskGetFnRecSize(bFN);
			
			iRet = SchComTaskRec(bFN, wPN, tmRec, bRecBuf, bInterU);
			if (iRet>0 || iRet==-2)	//该日/月的记录在任务库中存在
				continue;

			memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));
			

			//bFrzTimes = DaysPast(&tmStart,&tmNow);//按冻结次数进行补抄
			if (bInterU == TIME_UNIT_MONTH)
				iDirRet = DirReadFrz(j, bRecBuf+bTimeLen+1+5+1, 
									 bFN, wPN, wIDs, &tmRec, pMtrPro, bInterU);
			else
				iDirRet = DirReadFrz(bDayFrzIdx[j], bRecBuf+bTimeLen+1+5+1, 
									 bFN, wPN, wIDs, &tmRec, pMtrPro, bInterU); 
			
			if (iDirRet < 0) //补抄失败
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
		return RD_ERR_RDFAIL;	//抄读失败

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

//对补抄到的数据进行入库 冻结时标+测量点+抄表时间+费率+数据
bool SaveNewFrzRec(TTime tmCur, BYTE* pbRec, BYTE bFN, WORD wPN, BYTE bTimeLen, WORD wRecLen)
{
	BYTE bRateNum = 4;
	//BYTE bClass = 2;
	//BYTE idx ;
	BYTE bTimeBuf[5]; //时标的长度为5个字节
	TTime tmNow;
	bool fRet = false;

	TimeToFmt15(&tmCur, bTimeBuf);
	memcpy(pbRec, bTimeBuf+5-bTimeLen, bTimeLen);//冻结时标(月冻结—2 日冻结—3)

	pbRec[bTimeLen] = wPN;//测量点号

	GetCurTime(&tmNow);
	TimeToFmt15(&tmNow, pbRec+bTimeLen+1);//抄表时间

	//bRateNum = GetPnRateNum(wPN);
	pbRec[bTimeLen+5+1] = bRateNum;

	fRet = PipeAppend(TYPE_FRZ_TASK, bFN, pbRec, wRecLen);
	
	return fRet;
	
}

//描述：从通用类型向协议格式数据的转换
//参数：
//		@pbDst	 要转换为的目标格式值
//		@wSrcId	 通用数据原始ID	
//		@pbSrc	 通用数据格式的原始数据
//		@wSrcLen 通用数据格式的原始长度
//返回：大于零则转换OK,返回转换后的数据格式的长度
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
	case 0x9a: //有功电能
	case 0x94:
		iLen = sizeof(uint64);				
		for (i=0; i<wSrcLen/iLen && i<5; i++)
		{	
			memcpy((BYTE*)&iVal64, pbSrc+i*iLen, iLen);
			if (iVal64 == INVALID_VAL64)
			{
				//memset(pbDst+tLen, INVALID_DATA, 5);
				return -2;//有一个数据为无效，则都需要补抄
			}
			else
				Uint64ToBCD(iVal64, pbDst+tLen, 5);				
			tLen += 5;					
		}	
		iRet = tLen;			
		break;

	case 0x9b: //无功电能
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
				iVal64 /= 100;	//-2小数位
				Uint64ToBCD(iVal64, pbDst+tLen, 4);		
			}
			tLen += 4;					
		}	
		iRet = tLen;			
		break;

	case 0x9c://需量
	case 0xa1:
	case 0xa0:
	case 0xa4:
	case 0xa5:
	case 0xb1:
	case 0xb4:
	case 0xb5:
		if((wSrcId == 0x9c8f) || (wSrcId == 0x9caf) || (wSrcId == 0xb11f) || (wSrcId == 0xb12f)||
			(wSrcId == 0xb41f) || (wSrcId == 0xb42f) || (wSrcId == 0xb51f) || (wSrcId == 0xb52f))//需量时间
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

		if (time.nMinute != 0)//只保存小时数据
		{
            if (pbBuf[0] != 0)
    			bLen += ((wLen-1)/pbBuf[0]);
			continue;
		}
		if ( IsDiffDay(tSaveTime, &time) ) //如果不是当天的曲线，则不存
		{
            if (pbBuf[0] != 0)
    			bLen += ((wLen-1)/pbBuf[0]); //每笔记录的实际长度
			continue;
		}

		TimeToFmt15(&time, mBuf);		//Fmt15 5字节
		memcpy(mBuf+5, &wPn, 1);		//测量点号 1字节	

		//以下是数据内容
		if (wID>=0x3701 && wID<=0x3704)	//正反相有无功电能要调整顺序
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
		else if (wID>=0x3745 && wID<=0x3748) //四象限要调整顺序
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
		else if (wID>=0x3681 && wID<=0x3688)//功率
		{				
			if ((bFn=GetFnFromCurveId(0x3681))==0xff)
				return false;

			for (j=0; j<8; j++)
			{
				memcpy(mBuf+6, &pbBuf[bLen+1+5+j*3], 3);				
				PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);					
			}
		}
		else if (wID>=0x3689 && wID<=0x3694)//电压、电流
		{				
			if ((bFn=GetFnFromCurveId(0x3689))==0xff)
				return false;

			for (j=0; j<6; j++)
			{
				if (j < 3)
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+5+(j<<1)], 2); //去掉频率
					PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 8);
				}
				else
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+11+(j-3)*3], 3); //去掉频率				
					PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);
				}
			}
		}
		else if (wID>=0x3705 && wID<=0x3708)//功率因素
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
    		bLen += ((wLen-1)/pbBuf[0]); //每笔记录的实际长度
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

//参数：pfSuccOnce 如果补抄成功过一个数据项，则返回true,否则返回false
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
		memset(g_bRdTimes, 0, sizeof(g_bRdTimes));//失败次数过日清零
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
		if (bRdErr == RD_ERR_RDFAIL) //抄读失败
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
		else if (bRdErr != RD_ERR_OK)	//其他错误，比如：当前处于停止抄表或者直抄状态
		{
			return bRdErr;	//应立刻返回
		}
	}

	return RD_ERR_OK;
}

//参数：pfSuccOnce 如果补抄成功过一个数据项，则返回true,否则返回false
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

		iRet = ReadItemEx(BN24, PN0, 0x4110, &bCurveFrzStatus);//曲线冻结模式
		if (iRet<0 || bCurveFrzStatus==0)
			continue;

		bRdErr = ReRdCurveFrz(wPN, pMtrPro, bThrId, pfSuccOnce);
		if (bRdErr != RD_ERR_OK)
			return bRdErr;
	}

	return RD_ERR_OK;
}

/*************************************补抄部分——End*********************************************/


BYTE GetRdMtrTaskValidNum()
{
	//抄表配置参数id 0xe0000230~0xe0000250
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
		//if(readfile(wFile, i*EXSECT_SIZE, g_ExFlashBuf, -1))//读文件
		//{
		//	if (CheckFile(wFile, g_ExFlashBuf, 0))
		//	{
		//		//循环读前四个字节，查看信息域，直到尾部
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

//参数：wID 任务号，从0x0301~0x03FE/0x0401~0x04FE
//		pbBuf 读到的任务配置，
//返回：成功返回读到的配置字节数，否则返回-ERR_ITEM,
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
// 		bSect = (dwID - 0xE0000230)/8;//扇区
// 		bPage = (dwID - 0xE0000230)%8;//页
// 		wFile = 0x81;
// 	}
// 	else if(dwID>=0xe0000301 && dwID<=0xe00003fe)
// 	{
// 		bSect = (dwID - 0xe0000301)/8;//扇区
// 		bPage = (dwID - 0xe0000301)%8;//页
// 		wFile = 0x82;
// 	}
// 	else if(dwID>=0xe0000401 && dwID<=0xe00004fe)
// 	{
// 		bSect = (dwID - 0xe0000401)/8;//扇区
// 		bPage = (dwID - 0xe0000401)%8;//页
// 		wFile = 0x83;
// 	}
// 
// 	//对于已经存在的任务配置，判读任务是否被修改，若是，则需要删除该任务对应的数据库
// 	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
// 	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
// 	if(readfile(wFile, bSect*EXSECT_SIZE, g_ExFlashBuf, -1))//读文件
// 	{
// 		if (CheckFile(wFile, g_ExFlashBuf, 0))
// 		{
// 			//循环读前四个字节，查看信息域，直到尾部
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

//描述:对任务配置进行检查
//参数:@pbCfg 任务配置内容
//	   @pbEnd 任务配置结束最大能到达的位置,实际可能小于
//返回:如果正确则返回任务配置的长度,否则返回-1
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
	else if(((dwID>>SG_GET_TASK_TYPE)&SG_DWORD_GET_LOW_BYTE)==SG_TASK_RDMTR)//0xe00002**： 配表参数
	{
		if (pbEnd-pbCfg < SG_RDMTR_CFG_FIXLEN)
			return -1;

		bNum = *(pbCfg + SG_RDMTR_CFG_FIXLEN - 2);
		if(bNum == INVALID_DATA)//信息点标识组数为FF时 ， 信息点标识固定填FF　FF
			bNum = 1;
		wLen = SG_BYTE_PN*bNum;

		bNum = *(pbCfg + SG_RDMTR_CFG_FIXLEN + wLen -1);
		if(bNum == INVALID_DATA)//数据编码组数为FF时 ， 数据标识编码固定填FF FF FF FF
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

//功能：写任务数据
//参数：wID 任务号
//		pbbuf 包含任务配置的缓冲
//		wLen 缓冲的大小
//
//返回： 成功则返回实际写入的大小，否则返回0
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
 		wPn = dwID - 0xE0000230;//将任务号转换成测量点号
		wID = 0x0b32;
		wMaxLen = 712;
	}
 	else if(dwID>=0xe0000301 && dwID<=0xe00003fe)
 	{
 		wPn = dwID - 0xe0000301;//将任务号转换成测量点号
		wID = 0x0b11;
		wMaxLen = 512;
 	}
 	else if(dwID>=0xe0000401 && dwID<=0xe00004fe)
 	{
 		wPn = dwID - 0xe0000401;//将任务号转换成测量点号
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

	//任务参数变更标志(用于更新任务执行次数)
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
	SetInfo(INFO_TASK_PARA);//写新的任务配置
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
// 		bSect = (dwID - 0xE0000230)/8;//扇区
// 		bPage = (dwID - 0xE0000230)%8;//页
// 		wFile = 0x81;
// 	}
// 	else if(dwID>=0xe0000301 && dwID<=0xe00003fe)
// 	{
// 		bSect = (dwID - 0xe0000301)/8;//扇区
// 		bPage = (dwID - 0xe0000301)%8;//页
// 		wFile = 0x82;
// 	}
// 	else if(dwID>=0xe0000401 && dwID<=0xe00004fe)
// 	{
// 		bSect = (dwID - 0xe0000401)/8;//扇区
// 		bPage = (dwID - 0xe0000401)%8;//页
// 		wFile = 0x83;
// 	}
// 	dwTaskID &= SG_DWORD_GET_LOW_WORD;
// 
// 	//对于已经存在的任务配置，判读任务是否被修改，若是，则需要删除该任务对应的数据库
// 	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
// 	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
// 	if(readfile(wFile, bSect*EXSECT_SIZE, g_ExFlashBuf, -1))//读文件
// 	{
// 		if (CheckFile(wFile, g_ExFlashBuf, 0))
// 		{
// 			//循环读前四个字节，查看信息域，直到尾部
// 			while(g_ExFlashBuf[bPage*SG_TASK_LEN]==0x55 && g_ExFlashBuf[bPage*SG_TASK_LEN+1]==0xaa)
// 			{
// 				wFileSize = ByteToWord(&g_ExFlashBuf[bPage*SG_TASK_LEN+2]);
// 			}
// 		}
// 	}
// 	//如果任务号相同，但是任务内容不一样，则删除原配置文件，并初始化有效并执行。如果任务内容也相同，只是将任务设置成有效
// 	if(wFileSize > 0)//在0x55 和 0xaa的前提下，才能保证被写过
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
// 			if (wLen > 506)//512 是规约规定的512吗？我定义为506可以吗？
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
// 	SetInfo(INFO_TASK_PARA);//写新的任务配置
// 	DTRACE(DB_TASK, ("WriteTaskConfig: Write  Successfully.\r\n"));
// 	return wLen;
}
//将间隔转为分钟
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
	case TIME_UNIT_MONTH://TODO::月算法
		return 28*bInterV*24*60;
		break;
	}
}

//extern TSem g_semEsam;
//extern BYTE g_bEsamTxRxBuf[1800];


int ReadTaskData(BYTE* pbRx, BYTE* pbTx, DWORD* rdwRecCnt, int iMaxFrmSize, bool fReadData)
{
	TTime tmSch;
	int iBufNum;				//缓冲里能存记录的条数
	WORD wRecSize;				//任务数据库返回的每笔记录的大小
	BYTE* pbRxStatu = pbRx;
	BYTE* pbTxStatu = pbTx;
	DWORD dwInterRate;			//数据间隔时间
	WORD wDensity;				//数据密度
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

	//任务初始化中得到
	BYTE bSmplIntervU, bSmplIntervV, bSndFreq;
	TTime tmLastRec;
	BYTE bCommonTaskNum=0, bFwdTaskNum=0;
	int iFrmSize=0, iDataSize=0;
	DWORD dwID=0;


	//2014-09-06  添加
	WORD wID=0;
	TComTaskCfg tComTaskCfg;
	TFwdTaskCfg tFwdTaskCfg;
	DWORD iPast=0, dwMonths, dwComSampCycle=0;
	TTime tBaseSampleTime, tTime;
	DWORD dwRealIntervU = 0;
	BYTE bTaskFwdType;

	//WaitSemaphore(g_semEsam, SYS_TO_INFINITE);	

	iRet = 0;
	ReadItemEx(BN0, POINT0, 0x0b10, &bCommonTaskNum);								//更新当前的普通任务总数
	ReadItemEx(BN0, POINT0, 0x0b20, &bFwdTaskNum);									//更新当前的中继任务总数
	if ((bCommonTaskNum==0)&&(bFwdTaskNum==0))										//任务有效标志,当任务配置发生改变的时候,被设置成无效
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
	
	//2014-09-06  添加
	dwID = ByteToDWORD(pbRxStatu+2, 4);
	if ((dwID&0x0000ff00) == 0x00000300)
		wID = 0x0b11;
	else if ((dwID&0x0000ff00) == 0x00000400)
		wID = 0x0b21;
	
	wTask = (WORD)(*(pbRxStatu+2))-1;
	ReadItemEx(BN0, wTask, wID, bTaskCfgTempBuf);//读0xE0000300+wTask的对应的值存于bTaskCfgTempBuf中

	//2014-09-06  添加
	if (wID == 0x0b21)
	{
		memcpy(&tFwdTaskCfg, bTaskCfgTempBuf, sizeof(TFwdTaskCfg));
		FwdToComTaskPara(&tComTaskCfg, &tFwdTaskCfg);		
		memcpy(bTaskCfgTempBuf, &tComTaskCfg, sizeof(TFwdTaskCfg));
	}
	else if (wID == 0x0b11)
		memcpy(&tComTaskCfg, bTaskCfgTempBuf, sizeof(TFwdTaskCfg));

	
	if (bTaskCfgTempBuf[0] == 0)					//任务有效性标志
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
	dwSchTime = TimeToSeconds(&tmSch);						//起始时间

	//2014-09-06  调整起始时间为最接近给定时间的那个数据点的时间
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
		//月的情况下面调整(读取每个月的时候向前前进的步长时间不一样，
		//有的是29天，有的30天，有的31天)
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
	dwLastTime = TimeToSeconds(&tmSch);						//结束时间

	wDensity = *pbRxStatu++;								//数据密度
	dwTmIntervs = IntervsToMinutes(bSmplIntervU, bSmplIntervV)*60;//秒
	fInvaildDensity = false;

	switch (wDensity)										//由数据密度获得时间间隔
	{
	case 0:
		//2014-06-02  修改(密度为0  时，按任务设置的抽取倍率抽取)
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

	if (*(pbRxStatu-16) == 0x03)							//普通任务
	{
		bFn = wTask+FN_COMSTAT;
		do
		{
			if (bTaskCfgTempBuf[8]==0)						//数据结构方式 0表示自描述格式组织数据
			{
				iDataSize = 3;
				iAllDataCodeLen = 0;
				if (dwInterRate!=0)
				{
					SecondsToTime(dwSchTime,&tmSch);

					//2014-09-06  修改为前进N  个自然月，而不是固定的值
					if (tComTaskCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)
					{
						memset(&tTime, 0, sizeof(TTime));
						Fmt15ToTime((BYTE *)&tComTaskCfg.bComSampBasTime, &tTime);	//采样基准时间
						dwMonths = MonthsPast(&tTime, &tmSch);				//要读取的数据的时间相对于采样基准时间跨过的自然月
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
							if (!fReadData)					//试探是否有下一帧，如果有，返回大于0的数
							{
								//SignalSemaphore(g_semEsam);
								return 1;
							}
							
							*pbTxStatu++ = 0;				//数据结构方式
							*pbTxStatu++ = bTaskCfgTempBuf[19]*bTaskCfgTempBuf[19+1+2*bTaskCfgTempBuf[19]];		//数据组数（n×m）
							*pbTxStatu++ = 0;

							for (bTaski=0; bTaski<bTaskCfgTempBuf[19]; bTaski++)								//信息点标识组数n
							{
								for (bTaskj=0; bTaskj<bTaskCfgTempBuf[19+1+2*bTaskCfgTempBuf[19]]; bTaskj++)	//数据标识编码组数m
								{
									memcpy(pbTxStatu, bTaskCfgTempBuf+19+1+2*bTaski, 2);						//信息点标识
									pbTxStatu += 2;
									memcpy(pbTxStatu, bTaskCfgTempBuf+19+1+2*bTaskCfgTempBuf[19]+1+4*bTaskj, 4);//数据标识编码
									pbTxStatu += 4;
									memcpy(&dwID, bTaskCfgTempBuf+19+1+2*bTaskCfgTempBuf[19]+1+4*bTaskj, 4);
									iDataCodeLen = GetItemLenDw(BN0, dwID);										//取得数据标识编码对应的字节长度 参见附录C 存于iDataCodeLen
									memcpy(pbTxStatu, pbRdBuf+iAllDataCodeLen, iDataCodeLen);				//数据标识内容
									pbTxStatu += iDataCodeLen;
									iAllDataCodeLen += iDataCodeLen;

									//时间格式为5字节，年为1字节
									*pbTxStatu++ = ByteToBcd(tmSch.nYear-2000);
									*pbTxStatu++ = ByteToBcd(tmSch.nMonth);
									*pbTxStatu++ = ByteToBcd(tmSch.nDay);
									*pbTxStatu++ = ByteToBcd(tmSch.nHour);
									*pbTxStatu++ = ByteToBcd(tmSch.nMinute);

									iDataSize = iDataSize+2+4+iDataCodeLen+5;
								}
							}
							//6.1.6.1时间格式为6字节，年为2字节
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
			else if (bTaskCfgTempBuf[8]==1)					//数据结构方式 1表示按任务定义的数据格式组织数据
			{
				iDataSize = 1;
				iAllDataCodeLen = 0;
				if (dwInterRate!=0)
				{
					SecondsToTime(dwSchTime,&tmSch);

					//2014-09-06  修改为前进N  个自然月，而不是固定的值
					if (tComTaskCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)
					{
						memset(&tTime, 0, sizeof(TTime));
						Fmt15ToTime((BYTE *)&tComTaskCfg.bComSampBasTime, &tTime);	//采样基准时间
						dwMonths = MonthsPast(&tTime, &tmSch);				//要读取的数据的时间相对于采样基准时间跨过的自然月
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
							if (!fReadData)					//试探是否有下一帧，如果有，返回大于0的数
							{
								//SignalSemaphore(g_semEsam);
								return 1;
							}

							*pbTxStatu++ = 1;				//数据结构方式

							for (bTaski=0; bTaski<bTaskCfgTempBuf[19]; bTaski++)								//信息点标识组数n
							{
								for (bTaskj=0; bTaskj<bTaskCfgTempBuf[19+1+2*bTaskCfgTempBuf[19]]; bTaskj++)	//数据标识编码组数m
								{
									if (bTaskj == 0)
									{
										memcpy(pbTxStatu, bTaskCfgTempBuf+19+1+2*bTaski, 2);						//信息点标识
										pbTxStatu += 2;
									}
									memcpy(&dwID, bTaskCfgTempBuf+19+1+2*bTaskCfgTempBuf[19]+1+4*bTaskj, 4);
									iDataCodeLen = GetItemLenDw(BN0, dwID);										//取得数据标识编码对应的字节长度 参见附录C 存于iDataCodeLen
									memcpy(pbTxStatu, pbRdBuf+iAllDataCodeLen, iDataCodeLen);				//数据标识内容
									pbTxStatu += iDataCodeLen;
									iAllDataCodeLen += iDataCodeLen;

									if (bTaskj == 0)
										iDataSize = iDataSize+2+iDataCodeLen;
									else
										iDataSize = iDataSize+iDataCodeLen;
								}
							}

							//6.1.6.1时间格式为6字节，年为2字节
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
	else if (*(pbRxStatu-16) == 0x04)						//中继任务
	{
		bFn = wTask+FN_FWDSTAT;
		do
		{
			//2014-09-06  添加
			iDataSize = 0;
			
			if (dwInterRate!=0)
			{
				SecondsToTime(dwSchTime,&tmSch);

				//2014-09-06  修改为前进N  个自然月，而不是固定的值
				if (tComTaskCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)
				{
					memset(&tTime, 0, sizeof(TTime));
					Fmt15ToTime((BYTE *)&tComTaskCfg.bComSampBasTime, &tTime);	//采样基准时间
					dwMonths = MonthsPast(&tTime, &tmSch);				//要读取的数据的时间相对于采样基准时间跨过的自然月
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
						if (!fReadData)					//试探是否有下一帧，如果有，返回大于0的数
						{
							//SignalSemaphore(g_semEsam);
							return 1;
						}

						*pbTxStatu = bTaskCfgTempBuf[8];
						*(pbTxStatu+1) = pbRdBuf[0];
						memcpy(pbTxStatu+2, pbRdBuf+1, pbRdBuf[0]);

						iDataSize = iDataSize+pbRdBuf[0]+2;
						pbTxStatu += iDataSize;

						//6.1.6.1时间格式为6字节，年为2字节
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

//描述：根据dwID找到相应的bFn
//变量定义：dwID:下行的抄读ID，4 字节
//          
//返回值：  相对应的bFn值
//          0：不存在此数据项
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

/*************************************补抄部分——End*********************************************/
