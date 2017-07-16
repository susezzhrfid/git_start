/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FaAPI.h
 * 摘    要：系统实现主要的API
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#ifndef FAAPI_H
#define FAAPI_H
#include "Typedef.h"
#include "stdio.h"
#include "Sysarch.h"
#include "FaStruct.h"
#include "SysDebug.h"
#include "ProEx.h"

//自动抄表器的索引定义
#define READER_PLC	0	
#define READER_4851	1
#define READER_4852	2
#define READER_4853	3

#define READER_NUM	4	//自动抄表器的个数

extern BYTE g_bRdMtrAlr[12];
extern BYTE g_bRdMtrAlrStatus[12];

extern TSem   g_semMtrPnMap;

extern DWORD g_dwExtCmdFlg;
extern DWORD g_dwExtCmdClick;
extern DWORD g_dwUpdateTime;
extern BYTE g_b485RxLed;
extern BYTE g_b485TxLed;
extern BYTE g_bMsRxLed;
extern BYTE g_bMsTxLed;
extern bool g_fAlertLed;
extern TSoftVerChg g_SoftVerChg;	 //缓存的版本变更事件
extern TPowerOffTmp g_PowerOffTmp;     //掉电暂存变量
extern bool g_fStopMtrRd;
extern bool g_fUpdFatOk;
extern DWORD g_dwLastStopMtrClick;
extern WORD g_wStopSec;
extern struct TPro g_proMaster;	//主站通信协议基类
extern bool IsDownSoft();
extern bool IsAutoSend();
extern void SetTmrRptState(bool fValid);
extern bool IsDownSoft();
extern bool g_fRxLedTestCmd;
extern bool g_fTermTestMode;
//extern BYTE g_bRxBatTask0Cnt;
//extern BYTE g_bRxBatTask1Cnt;
extern DWORD g_dwTimes[3];
extern BYTE g_bDefaultCfgID;
extern BYTE g_bEnergyClrPnt;
extern bool g_fDownLoad;

bool SavePoweroffTmp(bool fSaveAc);
bool IsPowerOff();
extern int MainThread(void);
extern void FaResetAllPara();
extern void FaResetData();
extern void FaResetExPara();
bool IsMtr485Port(BYTE bPort);
extern void StopMtrRd(WORD wStopSec);
extern bool PipeAppend(BYTE bType, BYTE bFn, BYTE* pbData, WORD wDataLen);
void InitSysClock();
DWORD Un(WORD wPn);
DWORD In(WORD wPn);
WORD GetCriticalVolt(void);
WORD GetNoVoltPercent(WORD wPn);
WORD  GetRecvVoltPercent(WORD wPn);
WORD  GetNoCurPercent(WORD wPn);
WORD  GetRecCurPercent(WORD wPn);
extern bool IsNorSuportId(WORD wID);
void InitTestMode();
//extern BYTE GetSignStrength();
extern void DoLedCtrl(BYTE bID);
extern WORD g_wLedBurstCnt[MAX_LED_NUM];
extern void DoLedBurst(BYTE bID);
int GetMeterPortFunc(WORD wMeterPort);
bool IsAcqLogicPort(BYTE bPort);
int Transmit645Cmd(BYTE* pbCmdBuf, BYTE bLen, BYTE* pbData, BYTE* pbrLen, BYTE bTimeOut, WORD wPn, BYTE bPort);
int DirectTransmit645Cmd(WORD wPn, WORD wPort, BYTE* pbCmdBuf, BYTE bCmdLen, BYTE* pbRet, BYTE* pbRetLen, BYTE bTimeOut);
int ComTransmit645Cmd(TCommPara tComPara, BYTE* pbCmdBuf, BYTE bCmdLen, BYTE* pbRet, BYTE* pbRetLen, BYTE bTimeOut);
#endif

