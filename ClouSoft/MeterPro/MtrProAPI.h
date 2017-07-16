/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrProAPI.h
 * ժ    Ҫ�����ļ���Ҫ��������Э��Ļ���API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef MTRPROAPI_H
#define MTRPROAPI_H

#include "DL645.h"
#include "DL645V07.h"
#include "MeterPro.h"

typedef union
{
	T645Priv 	t645Priv ;   
	TV07Priv	tV07Priv;  
}TMtrPriv;		

typedef struct //�����Э�����Ҫ����ı���
{
	//BYTE*	pbUnsupIdFlg; //TODO:Ϊ�˽�ʡ�ڴ棬ֻ��һ��������Ŀռ䣬ʹ��ʱ����
	TMtrPriv tMtrPriv;
}TMtrSaveInf;

extern struct TMtrPro g_MtrPro[DYN_PN_NUM];
bool LoadMtrInfo(WORD wPn, TMtrPara* pMtrPara, TMtrSaveInf* pSaveInf);
bool SaveMtrInfo(WORD wPn, TMtrSaveInf* pSaveInf);
struct TMtrPro* CreateMtrPro(WORD wPn, TMtrPara* pMtrPara, TMtrSaveInf* pSaveInf, bool fInit, BYTE bThrId);
int AskMtrItem(struct TMtrPro* pMtrPro, WORD wPn, WORD wID, BYTE* pbBuf);
bool LoadMtrPara(WORD wPn, TMtrPara* pMtrPara);
#endif //MTRPROAPI_H


