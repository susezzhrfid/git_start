/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrProAPI.h
 * 摘    要：本文件主要包含抄表协议的基本API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
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

typedef struct //各电表协议的需要保存的变量
{
	//BYTE*	pbUnsupIdFlg; //TODO:为了节省内存，只开一个测量点的空间，使用时导入
	TMtrPriv tMtrPriv;
}TMtrSaveInf;

extern struct TMtrPro g_MtrPro[DYN_PN_NUM];
bool LoadMtrInfo(WORD wPn, TMtrPara* pMtrPara, TMtrSaveInf* pSaveInf);
bool SaveMtrInfo(WORD wPn, TMtrSaveInf* pSaveInf);
struct TMtrPro* CreateMtrPro(WORD wPn, TMtrPara* pMtrPara, TMtrSaveInf* pSaveInf, bool fInit, BYTE bThrId);
int AskMtrItem(struct TMtrPro* pMtrPro, WORD wPn, WORD wID, BYTE* pbBuf);
bool LoadMtrPara(WORD wPn, TMtrPara* pMtrPara);
#endif //MTRPROAPI_H


