/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TaskStruct.h
 * 摘    要：本文件主要用来定义任务数据结构
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2009年9月
 *********************************************************************************************************/
#ifndef TASKSTRUCT_H
#define TASKSTRUCT_H
#include "TypeDef.h"
#include "TaskConst.h"

//#define MAX_ITEMS_PER_CTASK 8

#define TSK_DMCUR		1		//DAY&MONTH FROZEN CURRENT 在日月冻结配置成抄电表当前数据时抄读
#define TSK_DMMTR		2		//DAY&MONTH FROZEN METER FRZ 在日月冻结配置成抄电表冻结时抄读
#define TSK_DFCUR		3		//DAYFLG FROZEN CURRENT 在抄表日冻结配置成抄电表当前数据时抄读
#define TSK_DFMTR		4		//DAYFLG FROZEN METER 在抄表日冻结配置成抄电表冻结时抄读
#define TSK_PFCUR		5		//PROFILE FROZEN CURRENT 在曲线配置成抄电表当前数据时抄读
#define TSK_PFMTR		6		//PROFILE FROZEN METER 在曲线配置成抄电表冻结时抄读
#define TSK_DEMDMCUR	7		//DEMAND DAY&MONTH FROZEN CURRENT 需量在日月冻结配置成抄电表当前数据时抄读
#define TSK_DEMDMMTR	8		//DEMAND DAY&MONTH FROZEN METER FRZ 需量在日月冻结配置成抄电表冻结时抄读
#define TSK_DEMDFCUR	9		//DEMAND DAYFLG FROZEN CURRENT 需量在抄表日月冻结配置成抄电表当前数据时抄读
#define TSK_DEMDFMTR	10		//DEMAND DAYFLG FROZEN METER FRZ 需量在抄表日月冻结配置成抄电表冻结时抄读
#define TSK_MSETT   	11		//MONTH SETT FRZ 月冻结在抄电表结算日时抄读
#define TSK_DEMMSETT	12		//DEMAND MONTH SETT FRZ 月冻结需量在抄电表结算日时抄读

#define MTR_WID_MAX		2        //ID组中最多的ID个数，暂时定为2个


typedef struct
{
	BYTE	bFN;							//数据项标识bFNs[MAX_ITEMS_PER_CTASK]
	BYTE	bIDCnt;							//wID[MTR_WID_MAX]数组中有效的ID个数
	WORD	wID[MTR_WID_MAX];				//数据项依赖的数据ID
	BYTE	bLen;							//所有ID数据项的总长度
	DWORD	dwID;							//对应上行通信ID 4个字节
	BYTE	bIntervU;						//捕获间隔单位
	WORD 	wRecSaveNum;					//记录保存个数
	BYTE	bPnChar[2];						//要执行此任务的测量点特征字,替代原来bPnType	[PNCHAR_MAX]
}TCommTaskCtrl;		//普通任务控制结构,集抄与负控的普通任务控制结构统一为一个

typedef struct
{
	BYTE	bSrcFn;		//一类小时冻结数据项标识
	BYTE	bDstFn;		//对应的二类曲线数据项标识
}TMaptoClass2Fn;

#endif //TASKSTRUCT_H