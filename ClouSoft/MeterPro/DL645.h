/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Dl645.h
 * 摘    要：本文件给出97版645抄表协议的相关定义
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef DL645_H
#define DL645_H

#include "MeterPro.h"

//97版645协议变量结构定义
typedef struct 
{
	//BYTE bSubPro; //子协议配置号		
	bool fRd901f;
	bool fRd9010;
	bool fRdSID;
	BYTE bAddrByte; //地址不足6字节(12位)的填充字节	
}T645Priv;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;		
}T645Tmp;

//97版645协议对外接口函数定义
bool Mtr645Init(struct TMtrPro* pMtrPro, bool fInit, BYTE bThrId);
int DL645AskItem(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);//读数据接口
bool DL645RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //解析接收函数
void DL645GetProPrintType(BYTE* pbPrintPro, char* pszProName);//获取打印协议名称


#endif //DL645_H


