/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֵ��ĳ������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��1��
 *********************************************************************************************************/
#ifndef MTRCTRL_H
#define MTRCTRL_H
#include "MtrStruct.h"
#include "LibDbConst.h"
#include "DbConst.h"
#include "SysArch.h"
#include "MtrProAPI.h"

//����������
#define RD_ERR_OK			0		//�޴�����ȫ����
#define RD_ERR_UNFIN		1		//û����
#define RD_ERR_PWROFF		2		//ͣ��
#define RD_ERR_485			3		//485�������
#define RD_ERR_PARACHG		4		//���������
#define RD_ERR_INTVCHG		5		//���������
#define RD_ERR_DIR			6		//����ֱ��
#define RD_ERR_STOPRD		7		//ֹͣ����
#define RD_ERR_RDFAIL		8		//����ʧ��

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
extern BYTE g_ProfFrzFlg[DYN_PN_NUM][30];//���߶����־λ
bool IsPnDataLoaded(WORD wPn); //������ֱ�������Ƿ��Ѿ�������ڴ�
bool LoadPnDirData(WORD wPn);  //���������ֱ������
struct TMtrPro* SetupThrdForPn(BYTE bThrId, WORD wPn);
BYTE GetRdMtrState(BYTE bThrId);
bool GetMtrDayFrzIdx(struct TMtrPro* pMtrPro, TTime tmStart, BYTE* pbDayFrzIdx);
int DirReadFrz(BYTE bFrzTimes, BYTE* pbRxBuf, BYTE bFN, WORD wPN, WORD wIDs, TTime* time, struct TMtrPro* pMtrPro, BYTE bInterU);

void GetDirRdCtrl(BYTE bPort);	//ȡ��ֱ���Ŀ���Ȩ
void ReleaseDirRdCtrl(BYTE bPort); //�ͷ�ֱ���Ŀ���Ȩ
extern DWORD GetCurIntervSec(WORD wPn, TTime* ptmNow);
extern int DoDirMtrRd(WORD wBn, WORD wPn, WORD wID, TTime now);
#endif //MTRCTRL_H
