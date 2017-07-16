/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DL645V07.h
 * 摘    要：本文件给出07版645抄表协议的相关定义
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef DL645V07_H
#define DL645V07_H
#include "MeterPro.h"

typedef struct
{
	WORD wDL645Id;
	DWORD dwProId;
	WORD wDL645Len;
	WORD wProLen;	
}TItemList;

typedef struct
{
	TTime tmStart;
	BYTE bNum;	
}TRdLoadInfo;

typedef struct 
{
	BYTE bSubPro; //子协议配置号			
}TV07Priv;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;
	BYTE bRdNextSeq;
	bool fRdNext;
	TRdLoadInfo tRdLoadInfo;
}TV07Tmp;


//07版645协议对外接口函数定义
bool Mtr645V07Init(struct TMtrPro* pMtrPro, bool fInit, BYTE bThrId);
int DL645V07AskItem(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);//读数据接口
bool DL645V07RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //解析接收函数
void DL645V07GetProPrintType(BYTE* pbPrintPro, char* pszProName);//获取打印协议名称

int MtrReadFrz(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, WORD wID, BYTE* pbBuf, WORD wSubIdx);

#endif //DL645V07_H


