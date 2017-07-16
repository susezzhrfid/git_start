/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterFmt.h
 * 摘    要：本文件主要实现跟协议相关的抄表数据项的格式转换、
 × 			 数据保存、ID转换问题
 * 当前版本：1.0
 * 作    者：岑坚宇、杨凡
 * 完成日期：2008年2月
 *********************************************************************************************************/
#ifndef METERFMT_H
#define METERFMT_H
#include "TypeDef.h"
#include "MtrStruct.h"

/////////////////////////////////////////////////////////////////////////
//对外提供的接口函数
//bool SaveMeterTask(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, DWORD dwTime, BYTE bErr);
int SaveMeterItem(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, DWORD dwTime, BYTE bErr);

BYTE GetFnFromCurveId(WORD wId);

#endif //METERFMT_H
