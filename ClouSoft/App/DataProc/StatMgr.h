/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：StatMgr.h
 * 摘    要：本文件主要实现终端统计信息及数据统计类的管理
 * 当前版本：1.0
 * 作    者：杨凡、李湘平
 * 完成日期：2008年7月
 *********************************************************************************************************/
#ifndef STATMGR_H
#define STATMGR_H
#include "DpStat.h"
//#include "Sys_arch.h"
#include "ComAPI.h"
//extern DWORD g_dwComTaskRecTm[DYN_PN_NUM][64];				//普通任务中间数据
typedef struct{
	TTime tmLastRun;		//最后一次运行的时间,用来重新上电的时候判断日月是否发生切换
	//WORD wDayRstStart;		//终端当日复位次数起始值,日切换的时候用来算当日复位次数
	//WORD wMonRstStart;		//终端当月复位次数起始值,月切换的时候用来算当月复位次数
	//WORD wDayPowerTime;		//终端当日供电时间,每分钟执行的时候加1
	//WORD wMonPowerTime;		//终端当月供电时间,每分钟执行的时候加1
	DWORD dwDayFlux;		//终端GPRS日流量
	DWORD dwMonFlux;		//终端GPRS月流量
}TTermStat;				//终端统计消息,每分钟写入到文件系统中

void DoTermStat(TTime* tmNow, TTermStat* pTermStat);
bool DoMgrDataStat(TTime* tmNow);
#endif
