/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrExc.h
 * 摘    要：实现告警的判断，记录，查询
 *
 * 版    本: 1.0 
 * 作    者：
 * 完成日期：2011年3月
************************************************************************************************************/
#ifndef MTR_EXCTASK_H
#define MTR_EXCTASK_H

#include "FaCfg.h"
#include "DbConst.h"
#include "ComStruct.h"

//#include "ExcTask.h"

#define ALR_HAPPEN		1
#define ALR_RECOVER		0

//********************************电表事件（需提交抄表ID)***************************************
#define BYTE_DATE_TIME 6						//时间字节数(年月日时分秒)
#define SG_BYTE_POSITIVE_POWER 4				//正向有功总电能字节数
#define SG_BYTE_NEGATIVE_POWER 4				//反向有功总电能字节数
#define SG_BYTE_TYPE_EVENT		1				//控制事件类型

//相序状态标志
#define DISORDER_U      0x01
#define DISORDER_I      0x02

#define CONNECT_1P    	1	//单相表
#define CONNECT_3P3W    3
#define CONNECT_3P4W    4

typedef struct{
	BYTE bBackBuf[9];
}TMtrCVLess;		//失压			ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[9];
}TMtrCIMisss;		//失流				ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[9];
}TMtrCIRevs;		//潮流反向			ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[6];
}TMtrPrgTm;			//编程时间更改		ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[2];
}TMtrPwSta;			//继电器变位		ARD12	Len = 9

typedef struct{
	bool fExcValid;
	TTime tmOccur;
	DWORD dwLastMin;
}TMtrClockErr;		//时钟异常告警		ARD13	Len = 20

typedef struct{
	BYTE bBackBuf[84];
}TMtrTou;			//时段或费率更改	ARD1	Len = 7

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TMtrBuyEngLack;	//剩余电费不足		ARD4	Len = 27

typedef struct{
	BYTE bBackBuf[11];
	bool fExcValid;
	DWORD dwPLastClick;
	DWORD dwLastMin;
}TMtrMeterStop;		//电能表停走		ARD2	Len = 91

typedef struct{
	BYTE bBackBuf[8];
	bool fExcValid;
	BYTE bPreBuf[76];
	WORD wDataLen;
}TMtrEnergyDec;		//示度下降			ARD3	Len = 159

typedef struct{
	bool fExcValid;
	TTime tmOccur;
}TMtrBatteryLow;	//时钟电池电压低	ARD1	Len = 7

typedef struct {
	TMtrCVLess tMtrCVLess;			//失压告警			ARD2	Len = 91
	TMtrCIMisss tMtrCIMisss;		//失流告警			ARD2	Len = 91
	TMtrCIRevs tMtrCIRevs;			//潮流反向告警		ARD2	Len = 91
	TMtrPrgTm tMtrPrgTm;			//编程时间更改告警	ARD2	Len = 91
	TMtrPwSta tMtrPwSta;			//继电器变位告警	ARD12	Len = 9
	TMtrClockErr tMtrClockErr;		//时钟异常告警		ARD13	Len = 20
	TMtrTou tMtrTou;				//时段或费率更改	ARD1	Len = 7
	TMtrBuyEngLack tMtrBuyEngLack;	//剩余电费不足		ARD4	Len = 27
	TMtrMeterStop tMtrMeterStop;	//电能表停走		ARD2	Len = 91
	TMtrEnergyDec tMtrEnergyDec;	//示度下降			ARD3	Len = 159
	TMtrBatteryLow tMtrBatteryLow;	//时钟电池电压低	ARD1	Len = 7
}TMtrAlrCtrl;	//len = 142+54+5+27+18+27+"ERC27"+"ERC30"

/*
//终端停电事件
typedef struct {


}TPwrOffCtrl;
*/

extern TMtrAlrCtrl g_MtrAlrCtrl[DYN_PN_NUM];	//电表告警控制结构

bool IsMtrAddrOK(BYTE *bBuf);

void InitMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl);
bool DoMtrExc(WORD wPn, TMtrAlrCtrl* pCtrl);
bool DoTestConnectExc(BYTE *pbBuf);
bool MtrRelayTask(WORD wPn, TMtrPwSta* pCtrl);
bool IsTerminalParaChgRec(BYTE *pdwDI, DWORD dwLen, TTime tmTime);
#endif 
