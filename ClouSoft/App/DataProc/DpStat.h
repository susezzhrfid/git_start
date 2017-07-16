/***********************************************************************************************
* Copyright (c) 2007,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DpStat.h
* 摘    要: 本文件提供2类数据统计功能接口
* 当前版本：1.0
* 作    者：杨 凡
* 完成日期：2007年8月
* 备    注：
***********************************************************************************************/
#ifndef DPSTAT_H
#define DPSTAT_H
#include  "ComStruct.h"


typedef struct{
	WORD         	wUpUpLimitVolt;			//电压上上限
	WORD            wUpLimitVolt;			//电压合格上限
	WORD         	wLowLowLimitVolt;		//电压下下限
	WORD         	wLowLimitVolt;		//电压合格下限

	WORD            wUnbalanceVoltLimit;     //三相电压不平衡限值
	WORD         	wUnbalanceCurLimit;		//三相电流不平衡限值
}TLimitVolt;

typedef struct{
  TTime tmLastRun;//上次运行的时间
  WORD wVoltUpUpTime;  //越上上限时间
  WORD wVoltUpTime;    //越上限时间
  WORD wVoltLowLowTime;//越下下限时间
  WORD wVoltLowTime;   //越下限时间
  WORD wVoltTime;      //合格时间
  WORD wMaxVolt;       //最大值
  WORD wMinVolt;       //最小值
  WORD wAvrVolt;       //平均值
  BYTE bTime[6];        //最大值最小值发生时间
  DWORD dwTotalVolt;   //求平均电压时的累计叠加电压
  WORD  wTotalTimes;   //求平均电压时的累计叠加统计次数
}TVoltStat;
  
void DpInit(TLimitVolt* pLimitVolt, WORD wPn);
void LoadLimitPara(TLimitVolt* pLimitVolt, WORD wPn);
void DoDpDataStat(TTime* tmNow, TVoltStat* pDayStat, TVoltStat* pMonStat);
//void DayChange(TTime* tmNow, TVoltStat* pDayStat, BYTE *piData, DWORD dwCurMin);
//void MonthChange(TTime* tmNow, TVoltStat* pMonStat, BYTE *piData, DWORD dwCurMin);
void DataPro(TVoltStat* pStat, BYTE *pData, DWORD dwTime);
void DayChange(TVoltStat* pDayStat, BYTE *piData, DWORD dwCurMin);
void MonthChange(TVoltStat* pMonStat, BYTE *piData, DWORD dwCurMin);
void MtrDayChange(WORD wPn, BYTE* pStatdata, BYTE *pbCurData, DWORD dwCurMin, TTime* ptmLast);
void MtrMonChange(WORD wPn, BYTE* pStatdata, BYTE *pbCurData, DWORD dwCurMin, TTime* ptmLast);
DWORD GetStartM(DWORD dwCurTimeM, int iType);
DWORD GetEndM(DWORD dwCurTimeM, int iType);
void DoVoltStat(TVoltStat* pDayStat, TVoltStat* pMonStat, TLimitVolt* pLimitVolt, BYTE *piData,DWORD  dwCurMin, DWORD dwIntervM);
void DoDpMtrDemandStat(WORD wPn, BYTE* pbDayData, BYTE* pbMonData, TTime* ptmNow, TTime* ptmLast);
void DoDpMtrUnBalanceStat(WORD wPn, BYTE* pbUnBalanceData,TTime* ptmNow, TTime* ptmLast);
void DoDpMtrUStat(WORD wPn, BYTE* pbDayData, BYTE* pbMonData, DWORD* pdwTotalV, WORD* pwStatCnt, TTime* ptmNow, TTime* ptmLast);
#endif //DPSTAT_H

