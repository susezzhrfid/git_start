/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TermnExc.h
 * 摘    要：实现非电表告警的判断，记录，查询
 *
 * 版    本: 1.0 1
 * 作    者：陈曦
 * 完成日期：2008-06-20
 *
 * 取代版本：1.0 0
 * 原 作 者:
 * 完成日期：
 * 备    注:
 
*********************************************************************************************************/
#ifndef TERM_EXCTASK_H
#define TERM_EXCTASK_H

typedef struct{
	BYTE bRdFailHapFlg[12];
	TTime tmOccur;
	TTime tmBackTime;
}TMtrRdErr;			//抄表失败			ARD21	Len = 263

typedef struct{
  	BYTE bEstbCnt;
	BYTE bRecvCnt;
	bool fPowerOff;         //当前是否停电标志
  	bool fOldPowerOff;      //上次是否停电标志
	DWORD dwEstbTime;
}TPwrOffCtrl;		//集中器掉/上电		ARD2	Len = 91

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TFluxOver;			//月通信流量越限	ARD10	Len = 10

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TTermBatteryLow;	//时钟电池电压低	ARD1	Len = 7


typedef struct{
	TMtrRdErr tMtrRdErr;			//抄表失败			ARD21	Len = 263
    TPwrOffCtrl tPwrOffCtrl;		//集中器掉/上电		ARD2	Len = 91
	TFluxOver tFluxOver;			//月通信流量越限	ARD10	Len = 10
	TTermBatteryLow tTermBatteryLow;//时钟电池电压低	ARD1	Len = 7
}TTermExcCtrl;  //终端事件控制结构

extern TTermExcCtrl g_TermAlrCtrl;


void InitTermExc(TTermExcCtrl* pCtrl);
bool DoTermExc(TTermExcCtrl* pCtrl);

#endif  