/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ��Э����ص����ݿ��׼�ӿ�֮�����չ�ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
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
	DWORD dwAlrId;//��Ӧ��Э���еĸ澯IDΪ4���ֽ�
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

bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass);//����:�˲������Ƿ�֧�ִ�Fn
//bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub); //��ȡ�û��û�����ź�С���
bool GetUserTypeAndVip(WORD wPn, BYTE* bVip, BYTE* bType);//��ȡ���������ͺ��ص㻧����

void UpdPnMap();
WORD GetInMtrPn();
void SaveSoftVerChg();
bool InitDB(void);
WORD GetFmtARDLen(DWORD dwId);
DWORD GetAlrID(BYTE i);

#ifdef PRO_698
//����װ������������ֵ

bool DelPortMtrs(BYTE bPort);
#endif
void InitInMeterPn();

bool IsV07Mtr(WORD wPn);//�Ƿ�V2007��645Э�������
void GetFrzStatus(WORD wPn, BYTE* pbCurveStatus, BYTE* pbDayFrzStatus, BYTE* pbDayFlgFrzStatus);//��ȡ�����㶳���������
void ClearCurveFrzFlg(WORD wPn);//�������������ÿ���96�����ã�������������ʱ

void SetPnRateNum(WORD wPn, BYTE bRateNum);//���ò����������
BYTE GetPnRateNum(WORD wPn);//��ȡ������ķ�����
bool IsChgRateNumByMtr();//�Ƿ���ݵ���ص������ݵ�ʵ�ʳ����޸Ĳ�����ķ�����

void SetPnSPHTDMtr(WORD wPn, BYTE bSPMR); //���ò�����Ϊ�����ʱ��single-phase time-division meter�ı�־
bool IsPnSPHTDMtr(WORD wPn); //��ѯ�������Ƿ�Ϊ�����ʱ��ı�־
bool IsSinglePhaseV07Mtr(WORD wPn);
BYTE IsSIDV97Mtr(WORD wPn);

WORD GetAcPn();

bool IsAlrEnable(DWORD dwAlrID);
bool IsCctPn(WORD wPn);
bool IsEventId(DWORD dwId);
WORD GetEventLen(DWORD dwId);
#endif //DBAPI_H
