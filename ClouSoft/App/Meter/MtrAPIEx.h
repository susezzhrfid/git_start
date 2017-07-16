/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterAPIEx.h
 * 摘    要：本文件主要实现抄表接口需要根据不同协议修改的部分
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef METERAPIEX_H
#define METERAPIEX_H
#include "MtrStruct.h"

/////////////////////////////////////////////////////////////////////////
//对外提供的标准接口函数
const WORD* Bank0To645ID(WORD wID);
bool IsMtrID(WORD wID);
WORD MtrCmbTo645IdNum(WORD wID);
DWORD GetMtrIdRdDelay(WORD wPn, WORD wID);
bool IsGrpID(WORD wID);
bool IsPnID(WORD wID);

bool IsCurveId(WORD wID);		//是否曲线冻结ID	
bool IsDayFrzId(WORD wID);		//是否日冻结ID
/////////////////////////////////////////////////////////////////////////
//内部使用的函数


#endif //METERAPIEX_H