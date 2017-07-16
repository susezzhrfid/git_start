/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DataManager.h
 * ժ    Ҫ�����ļ���Ҫʵ��ϵͳ���ݿ���������д�洢����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
*********************************************************************************************************/
#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "TypeDef.h"
#include "FaConst.h"
#include "SysArch.h"
#include "LibDbStruct.h"
#include "LibDbConst.h"

bool DbInit(TDbCtrl* pDbCtrl);
int DbReadItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwStartTime, DWORD dwEndTime);
int DbWriteItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwTime);
TDataItem DbGetItemEx(WORD wBank, WORD wPn, WORD wID);

void DbTimeAdjBackward(DWORD dwTime);

bool InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum);
int SearchPnMap(BYTE bSch, WORD wPn);
int MapToPn(BYTE bSch, WORD wMn);
int NewPnMap(BYTE bSch, WORD wPn);
bool DeletePnMap(BYTE bSch, WORD wPn);
int GetPnMapRealNum(BYTE bSch);
int SavePnMap();

bool DbClearBankData(WORD wBank, WORD wSect);
bool DbLoadBankDefaultRAM(const TBankCtrl* pBankCtrl, DWORD dwOffset);
bool DbLoadBankDefaultFLASH(const TBankCtrl* pBankCtrl, DWORD dwOffset);
bool DbLoadBankDefault(const TBankCtrl* pBankCtrl);

void DbInitTimeData(BYTE bSect, BYTE bDynBn, DWORD dwCurIntervTime);
void DbSetCurInterv(BYTE bSect, BYTE bDynBn, DWORD dwCurIntervTime);
DWORD DbGetCurInterv(BYTE bSect, BYTE bDynBn);

#endif  //DATAMANAGER_H


