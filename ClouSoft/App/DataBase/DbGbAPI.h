/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbGbAPI.h
 * 摘    要：本文件主要实现数据库的数据结构定义
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef DBGBAPI_H      
#define DBGBAPI_H      
#include "TypeDef.h"
#include "FaCfg.h"
#include "LibDbStruct.h"

#define GB_RDERR_FAIL		1		//读失败
#define GB_RDERR_NOROOM		2		//空间不够,一旦返回这个错误，读缓冲中没有任何有效的数据

int SgReadClass1(DWORD dwId, WORD wPn, BYTE* pbTx, int iTxBufSize, bool fRptState);
int SgReadClass2(DWORD dwID, WORD wPn, BYTE* pbRx, BYTE* pbTx, int iTxBufSize, WORD* pwStart, bool fRptState);
//int SgReadClass3(DWORD dwId, WORD wPn, DWORD dwStartm, DWORD dwEndtm, BYTE* pbBuf, BYTE* bRdNum, int iTxBufLeft);
int SgReadClass3(DWORD dwId, WORD wPn, DWORD dwStartm, DWORD dwEndtm, BYTE* pbBuf, WORD* wRdNum, WORD wTotal, int iTxBufLeft);

#endif //DBGBAPI_H