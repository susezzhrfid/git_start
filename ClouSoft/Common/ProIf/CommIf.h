/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CommIf.h
 * 摘    要：本文件实现了串口通信接口类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef COMMIF_H
#define COMMIF_H
#include "ProIf.h"
#include "Comm.h"

typedef struct{
	//参数
	bool	fDebug;		//是否跟调试输出口共用

	WORD 	wPort; 		//端口号
	DWORD 	dwBaudRate; 
	BYTE 	bByteSize; 
	BYTE 	bStopBits; 
	BYTE 	bParity;
}TCommIf;	//串行通信接口子类结构


//供外部调用的接口函数
bool CommIfInit(struct TProIf* pProIf);

#endif  //COMMIF_H


