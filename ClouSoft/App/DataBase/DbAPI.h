/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbAPI.h
 * 摘    要：本文件主要实现协议相关的数据库标准接口之外的扩展接口
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef DBAPI_H
#define DBAPI_H
#include "TypeDef.h"
#include "FaCfg.h"
#include "sysarch.h"
//#include "sysapi.h"
#include "LibDbAPI.h"
#include "DbHook.h"

#define MAINTYPE_CLASS0		0
#define MAINTYPE_CLASSA		1//0
#define MAINTYPE_CLASSB		2//1
#define MAINTYPE_CLASSC		3//2
#define MAINTYPE_CLASSD		4//3
#define MAINTYPE_CLASSE		5//4
#define MAINTYPE_CLASSF		6//5
typedef struct
{
	DWORD dwAlrId;//对应新协议中的告警ID为4个字节
	BYTE bFmt;
}TAlrTaskCtrl;

BYTE GetPnProp(WORD wPn);
BYTE GetPnMtrPro(WORD wPn);
BYTE GetCctPnMtrPro(WORD wPn);
BYTE GetPnPort(WORD wPn);
bool IsPnType(WORD wPn, WORD wType);
bool IsMtrPn(WORD wPn);
bool IsGrpValid(WORD wPn);
bool IsGrpC1Fn(BYTE bFN);
bool IsGrpC2Fn(BYTE bFN);
bool IsDCPnValid(WORD wPn);
bool IsDcC1Fn(BYTE bFn);
bool IsDcC2Fn(BYTE bFn);
BYTE GetErcType(BYTE bErc);
BYTE GetConnectType(WORD wPn);
void ClearF9ZeroPara(BYTE bFn);
void ClearF9TailPara(BYTE bFn,BYTE bMaxNum);

bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass);//描述:此测量点是否支持此Fn
//bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub); //获取用户用户大类号和小类号
bool GetUserTypeAndVip(WORD wPn, BYTE* bVip, BYTE* bType);//获取测量点类型和重点户属性

void UpdPnMap();
WORD GetInMtrPn();
void SaveSoftVerChg();
bool InitDB(void);
WORD GetFmtARDLen(DWORD dwId);
DWORD GetAlrID(BYTE i);

#ifdef PRO_698
//电表的装置序号与测量点值

bool DelPortMtrs(BYTE bPort);
#endif
void InitInMeterPn();

bool IsV07Mtr(WORD wPn);//是否V2007版645协议测量点
void GetFrzStatus(WORD wPn, BYTE* pbCurveStatus, BYTE* pbDayFrzStatus, BYTE* pbDayFlgFrzStatus);//获取测量点冻结参数设置
void ClearCurveFrzFlg(WORD wPn);//清除测量点曲线每天的96点设置，用于正常换日时

void SetPnRateNum(WORD wPn, BYTE bRateNum);//设置测量点费率数
BYTE GetPnRateNum(WORD wPn);//获取测量点的费率数
bool IsChgRateNumByMtr();//是否根据电表返回电能数据的实际长度修改测量点的费率数

void SetPnSPHTDMtr(WORD wPn, BYTE bSPMR); //设置测量点为单相分时表single-phase time-division meter的标志
bool IsPnSPHTDMtr(WORD wPn); //查询测量点是否为单相分时表的标志
bool IsSinglePhaseV07Mtr(WORD wPn);
BYTE IsSIDV97Mtr(WORD wPn);

WORD GetAcPn();

bool IsAlrEnable(DWORD dwAlrID);
bool IsCctPn(WORD wPn);
bool IsEventId(DWORD dwId);
WORD GetEventLen(DWORD dwId);
#endif //DBAPI_H
