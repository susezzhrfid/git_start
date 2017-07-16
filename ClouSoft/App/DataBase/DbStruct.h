/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbStruct.h
 * 摘    要：本文件主要用来定义各版本特殊的数据结构
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 *********************************************************************************************************/
#ifndef DBSTRUCT_H
#define DBSTRUCT_H
#include "SysArch.h"
#include "LibDbStruct.h"
#include "TypeDef.h"

typedef struct{
	BYTE  bValid:1;        				//有效的标志  
	BYTE  bNotFix:7;					//长度是否固定的标志
	BYTE  bBank;        				//对应Bank标识    
    WORD  wID;         					//对应ID标识  
	BYTE  bType;						//PN类型  
	WORD  wPNMax;						//PN数量  
}TConvertFNIDDesc;


#endif //DBSTRUCT_H