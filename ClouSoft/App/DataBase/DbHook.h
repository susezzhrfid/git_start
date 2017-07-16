/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbHook.h
 * 摘    要：本文件主要用来定义系统库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *********************************************************************************************************/
#ifndef DBHOOK_H
#define DBHOOK_H
#include "TypeDef.h"

/////////////////////////////////////////////////////////////////////////
//系统库的代码库需要的挂钩/回调函数定义
bool IsPnValid(WORD wPn);
//WORD* CmbToSubID(WORD wBn, WORD wID);
const WORD* CmbToSubID(WORD wBn, WORD wID);
int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet);
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet);
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet);
bool PswCheck(BYTE bPerm, BYTE* pbPassword);

BYTE GetInvalidData(BYTE bErr); 	//获取本系统的无效数据的定义
bool IsInvalidData(BYTE* p, WORD wLen);	//是否是无效数据，无效数据可能存在多种定义

/////////////////////////////////////////////////////////////////////////
//在实现挂钩/回调函数时需要额外定义的函数
WORD CmbToSubIdNum(WORD wBn, WORD wID);

//江苏判断是否是单相分时表所用函数
bool IsSP_TDMeter(BYTE bMain, BYTE bSub);

BYTE GetCurveInterv();


#endif //DBHOOK_H

