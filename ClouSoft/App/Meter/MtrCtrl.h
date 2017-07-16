/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrCtrl.h
 * 摘    要：本文件主要实现电表的抄表控制
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年1月
 *********************************************************************************************************/
#ifndef MTRCTRL_H
#define MTRCTRL_H
#include "MtrStruct.h"
#include "LibDbConst.h"
#include "DbConst.h"
#include "SysArch.h"
#include "MtrProAPI.h"

//抄读错误定义
#define RD_ERR_OK			0		//无错误，完全抄完
#define RD_ERR_UNFIN		1		//没抄完
#define RD_ERR_PWROFF		2		//停电
#define RD_ERR_485			3		//485抄表故障
#define RD_ERR_PARACHG		4		//电表参数变更
#define RD_ERR_INTVCHG		5		//抄表间隔变更
#define RD_ERR_DIR			6		//正在直抄
#define RD_ERR_STOPRD		7		//停止抄表
#define RD_ERR_RDFAIL		8		//抄读失败

extern bool g_fDirRd;
extern BYTE g_bDirRdStep;
extern BYTE g_bMtrRdStep[DYN_PN_NUM];
extern BYTE g_bMtrRdStatus[REAL_PN_MASK_SIZE];
extern TMtrPara g_MtrPara[DYN_PN_NUM];
extern TMtrSaveInf g_MtrSaveInf[DYN_PN_NUM];

void  LockReader();
void  UnLockReader();
BYTE* GetPnTmpData(WORD wNum, BYTE bThrId);
void MtrCtrlInit();
TThreadRet MtrRdThread(void* pvPara);
extern BYTE g_ProfFrzFlg[DYN_PN_NUM][30];//曲线冻结标志位
bool IsPnDataLoaded(WORD wPn); //测量点直抄数据是否已经导入进内存
bool LoadPnDirData(WORD wPn);  //导入测量点直抄数据
struct TMtrPro* SetupThrdForPn(BYTE bThrId, WORD wPn);
BYTE GetRdMtrState(BYTE bThrId);
bool GetMtrDayFrzIdx(struct TMtrPro* pMtrPro, TTime tmStart, BYTE* pbDayFrzIdx);
int DirReadFrz(BYTE bFrzTimes, BYTE* pbRxBuf, BYTE bFN, WORD wPN, WORD wIDs, TTime* time, struct TMtrPro* pMtrPro, BYTE bInterU);

void GetDirRdCtrl(BYTE bPort);	//取得直抄的控制权
void ReleaseDirRdCtrl(BYTE bPort); //释放直抄的控制权
extern DWORD GetCurIntervSec(WORD wPn, TTime* ptmNow);
extern int DoDirMtrRd(WORD wBn, WORD wPn, WORD wID, TTime now);
#endif //MTRCTRL_H
