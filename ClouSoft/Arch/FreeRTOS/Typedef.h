/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Typedef.h
 * ժ    Ҫ�����ļ���Ҫ����ϵͳ���õ�����������
 * ��ǰ�汾��1.0.0
 * ��    �ߣ�����
 * ������ڣ�2012��2��
 * ��    ע��
 ******************************************************************************/
#ifndef  TYPEDEF_H
#define  TYPEDEF_H
#include <stdbool.h>

#ifndef NULL
#define NULL 0
#endif

//typedef unsigned short u_short;
#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE 
#define TRUE   1
#endif

typedef bool BOOL;

typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef unsigned long long  uint64; 
typedef unsigned long long  DDWORD;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

//typedef  unsigned char          U8;
//typedef  unsigned short         U16;
//typedef  unsigned long          U32;
//typedef unsigned char  INT8U;
//typedef unsigned short INT16U;
//typedef unsigned long  INT32U;
//typedef char  INT8;
//typedef short INT16;
//typedef long  INT32;
//typedef unsigned int uint32_t;
//typedef unsigned short uint16_t;
//typedef unsigned char uint8_t;

#endif
