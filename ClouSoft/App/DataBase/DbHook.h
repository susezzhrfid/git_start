/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbHook.h
 * ժ    Ҫ�����ļ���Ҫ��������ϵͳ��Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *********************************************************************************************************/
#ifndef DBHOOK_H
#define DBHOOK_H
#include "TypeDef.h"

/////////////////////////////////////////////////////////////////////////
//ϵͳ��Ĵ������Ҫ�Ĺҹ�/�ص���������
bool IsPnValid(WORD wPn);
//WORD* CmbToSubID(WORD wBn, WORD wID);
const WORD* CmbToSubID(WORD wBn, WORD wID);
int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet);
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet);
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet);
bool PswCheck(BYTE bPerm, BYTE* pbPassword);

BYTE GetInvalidData(BYTE bErr); 	//��ȡ��ϵͳ����Ч���ݵĶ���
bool IsInvalidData(BYTE* p, WORD wLen);	//�Ƿ�����Ч���ݣ���Ч���ݿ��ܴ��ڶ��ֶ���

/////////////////////////////////////////////////////////////////////////
//��ʵ�ֹҹ�/�ص�����ʱ��Ҫ���ⶨ��ĺ���
WORD CmbToSubIdNum(WORD wBn, WORD wID);

//�����ж��Ƿ��ǵ����ʱ�����ú���
bool IsSP_TDMeter(BYTE bMain, BYTE bSub);

BYTE GetCurveInterv();


#endif //DBHOOK_H

