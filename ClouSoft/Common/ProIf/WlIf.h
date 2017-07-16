/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：WlIf.h
 * 摘    要：本文件实现了无线通信接口类
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef WLIF_H
#define WLIF_H

#include "ProIf.h"

typedef struct{
	//参数	
	WORD 	wPort; 		//端口号
	DWORD 	dwBaudRate; 
	BYTE 	bByteSize; 
	BYTE 	bStopBits; 
	BYTE 	bParity;
	//数据
	
}TWLIf;	//Socket接口子类


//供外部调用的接口函数
bool WlIfInit(struct TProIf* pProIf);

#endif  //WLIF_H
