/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TermnExc.h
 * ժ    Ҫ��ʵ�ַǵ��澯���жϣ���¼����ѯ
 *
 * ��    ��: 1.0 1
 * ��    �ߣ�����
 * ������ڣ�2008-06-20
 *
 * ȡ���汾��1.0 0
 * ԭ �� ��:
 * ������ڣ�
 * ��    ע:
 
*********************************************************************************************************/
#ifndef TERM_EXCTASK_H
#define TERM_EXCTASK_H

typedef struct{
	BYTE bRdFailHapFlg[12];
	TTime tmOccur;
	TTime tmBackTime;
}TMtrRdErr;			//����ʧ��			ARD21	Len = 263

typedef struct{
  	BYTE bEstbCnt;
	BYTE bRecvCnt;
	bool fPowerOff;         //��ǰ�Ƿ�ͣ���־
  	bool fOldPowerOff;      //�ϴ��Ƿ�ͣ���־
	DWORD dwEstbTime;
}TPwrOffCtrl;		//��������/�ϵ�		ARD2	Len = 91

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TFluxOver;			//��ͨ������Խ��	ARD10	Len = 10

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TTermBatteryLow;	//ʱ�ӵ�ص�ѹ��	ARD1	Len = 7


typedef struct{
	TMtrRdErr tMtrRdErr;			//����ʧ��			ARD21	Len = 263
    TPwrOffCtrl tPwrOffCtrl;		//��������/�ϵ�		ARD2	Len = 91
	TFluxOver tFluxOver;			//��ͨ������Խ��	ARD10	Len = 10
	TTermBatteryLow tTermBatteryLow;//ʱ�ӵ�ص�ѹ��	ARD1	Len = 7
}TTermExcCtrl;  //�ն��¼����ƽṹ

extern TTermExcCtrl g_TermAlrCtrl;


void InitTermExc(TTermExcCtrl* pCtrl);
bool DoTermExc(TTermExcCtrl* pCtrl);

#endif  