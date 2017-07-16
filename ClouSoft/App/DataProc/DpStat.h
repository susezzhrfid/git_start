/***********************************************************************************************
* Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�DpStat.h
* ժ    Ҫ: ���ļ��ṩ2������ͳ�ƹ��ܽӿ�
* ��ǰ�汾��1.0
* ��    �ߣ��� ��
* ������ڣ�2007��8��
* ��    ע��
***********************************************************************************************/
#ifndef DPSTAT_H
#define DPSTAT_H
#include  "ComStruct.h"


typedef struct{
	WORD         	wUpUpLimitVolt;			//��ѹ������
	WORD            wUpLimitVolt;			//��ѹ�ϸ�����
	WORD         	wLowLowLimitVolt;		//��ѹ������
	WORD         	wLowLimitVolt;		//��ѹ�ϸ�����

	WORD            wUnbalanceVoltLimit;     //�����ѹ��ƽ����ֵ
	WORD         	wUnbalanceCurLimit;		//���������ƽ����ֵ
}TLimitVolt;

typedef struct{
  TTime tmLastRun;//�ϴ����е�ʱ��
  WORD wVoltUpUpTime;  //Խ������ʱ��
  WORD wVoltUpTime;    //Խ����ʱ��
  WORD wVoltLowLowTime;//Խ������ʱ��
  WORD wVoltLowTime;   //Խ����ʱ��
  WORD wVoltTime;      //�ϸ�ʱ��
  WORD wMaxVolt;       //���ֵ
  WORD wMinVolt;       //��Сֵ
  WORD wAvrVolt;       //ƽ��ֵ
  BYTE bTime[6];        //���ֵ��Сֵ����ʱ��
  DWORD dwTotalVolt;   //��ƽ����ѹʱ���ۼƵ��ӵ�ѹ
  WORD  wTotalTimes;   //��ƽ����ѹʱ���ۼƵ���ͳ�ƴ���
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

