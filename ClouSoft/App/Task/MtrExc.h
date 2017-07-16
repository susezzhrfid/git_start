/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrExc.h
 * ժ    Ҫ��ʵ�ָ澯���жϣ���¼����ѯ
 *
 * ��    ��: 1.0 
 * ��    �ߣ�
 * ������ڣ�2011��3��
************************************************************************************************************/
#ifndef MTR_EXCTASK_H
#define MTR_EXCTASK_H

#include "FaCfg.h"
#include "DbConst.h"
#include "ComStruct.h"

//#include "ExcTask.h"

#define ALR_HAPPEN		1
#define ALR_RECOVER		0

//********************************����¼������ύ����ID)***************************************
#define BYTE_DATE_TIME 6						//ʱ���ֽ���(������ʱ����)
#define SG_BYTE_POSITIVE_POWER 4				//�����й��ܵ����ֽ���
#define SG_BYTE_NEGATIVE_POWER 4				//�����й��ܵ����ֽ���
#define SG_BYTE_TYPE_EVENT		1				//�����¼�����

//����״̬��־
#define DISORDER_U      0x01
#define DISORDER_I      0x02

#define CONNECT_1P    	1	//�����
#define CONNECT_3P3W    3
#define CONNECT_3P4W    4

typedef struct{
	BYTE bBackBuf[9];
}TMtrCVLess;		//ʧѹ			ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[9];
}TMtrCIMisss;		//ʧ��				ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[9];
}TMtrCIRevs;		//��������			ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[6];
}TMtrPrgTm;			//���ʱ�����		ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[2];
}TMtrPwSta;			//�̵�����λ		ARD12	Len = 9

typedef struct{
	bool fExcValid;
	TTime tmOccur;
	DWORD dwLastMin;
}TMtrClockErr;		//ʱ���쳣�澯		ARD13	Len = 20

typedef struct{
	BYTE bBackBuf[84];
}TMtrTou;			//ʱ�λ���ʸ���	ARD1	Len = 7

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TMtrBuyEngLack;	//ʣ���Ѳ���		ARD4	Len = 27

typedef struct{
	BYTE bBackBuf[11];
	bool fExcValid;
	DWORD dwPLastClick;
	DWORD dwLastMin;
}TMtrMeterStop;		//���ܱ�ͣ��		ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[8];
	bool fExcValid;
	BYTE bPreBuf[76];
	WORD wDataLen;
}TMtrEnergyDec;		//ʾ���½�			ARD3	Len = 159

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TMtrBatteryLow;	//ʱ�ӵ�ص�ѹ��	ARD1	Len = 7

typedef struct {
	TMtrCVLess tMtrCVLess;			//ʧѹ�澯			ARD2	Len = 91
	TMtrCIMisss tMtrCIMisss;		//ʧ���澯			ARD2	Len = 91
	TMtrCIRevs tMtrCIRevs;			//��������澯		ARD2	Len = 91
	TMtrPrgTm tMtrPrgTm;			//���ʱ����ĸ澯	ARD2	Len = 91
	TMtrPwSta tMtrPwSta;			//�̵�����λ�澯	ARD12	Len = 9
	TMtrClockErr tMtrClockErr;		//ʱ���쳣�澯		ARD13	Len = 20
	TMtrTou tMtrTou;				//ʱ�λ���ʸ���	ARD1	Len = 7
	TMtrBuyEngLack tMtrBuyEngLack;	//ʣ���Ѳ���		ARD4	Len = 27
	TMtrMeterStop tMtrMeterStop;	//���ܱ�ͣ��		ARD2	Len = 91
	TMtrEnergyDec tMtrEnergyDec;	//ʾ���½�			ARD3	Len = 159
	TMtrBatteryLow tMtrBatteryLow;	//ʱ�ӵ�ص�ѹ��	ARD1	Len = 7
}TMtrAlrCtrl;	//len = 142+54+5+27+18+27+"ERC27"+"ERC30"

/*
//�ն�ͣ���¼�
typedef struct {


}TPwrOffCtrl;
*/

extern TMtrAlrCtrl g_MtrAlrCtrl[DYN_PN_NUM];	//���澯���ƽṹ

bool IsMtrAddrOK(BYTE *bBuf);

void InitMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl);
bool DoMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl);
bool DoTestConnectExc(BYTE *pbBuf);
bool MtrRelayTask(WORD wPn, TMtrPwSta* pCtrl);
bool IsTerminalParaChgRec(BYTE *pdwDI, DWORD dwLen, TTime tmTime);
#endif 
