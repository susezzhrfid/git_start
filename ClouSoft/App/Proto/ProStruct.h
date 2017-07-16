/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProStruct.h
 * 摘    要：本文件主要包含通信结构的定义
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROSTRUCT_H
#define PROSTRUCT_H
#include "TypeDef.h"

#define MTRFWD_BUFSIZE		300

typedef struct{
	bool 	fUdp; 
	DWORD 	dwRemoteIP; 
	WORD 	wRemotePort;
	DWORD 	dwBakIP;
	WORD	wBakPort;
}TMasterIp;	//主站IP地址参数

typedef struct{
	bool 	fUdp; 
	WORD 	wLocalPort;		//绑定的本地端口号
}TSvrPara;	//终端自身服务器参数

#endif //PROSTRUCT_H

