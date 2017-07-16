/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TypeDef.h
 * 摘    要：本文件主要定义系统中用到的数据类型
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#ifndef  TYPEDEF_H
#define  TYPEDEF_H

#include<windows.h>

#ifndef NULL
#define NULL 0
#endif

#define false FALSE
#define true TRUE

typedef BOOL bool;

typedef unsigned short u_short;

typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef unsigned long long  uint64; 
typedef unsigned long long  DDWORD;

typedef unsigned char BYTE;
typedef unsigned short WORD;

//typedef unsigned int DWORD;

#endif
