/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ComStruct.h
 * 摘    要：本文件主要用来定义一些公用的数据结构
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备注：
 *********************************************************************************************************/
#ifndef COMSTRUCT_H
#define COMSTRUCT_H
#include "TypeDef.h"

typedef struct{
	WORD  nYear;
	BYTE  nMonth;
	BYTE  nDay;
	BYTE  nHour;
	BYTE  nMinute;
	BYTE  nSecond;
	BYTE  nWeek;
}TTime;

#endif //COMSTRUCT_H
