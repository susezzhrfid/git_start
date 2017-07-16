/********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Pro.h
 * 摘    要：本文件实现了通信协议基类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROTO_H
#define PROTO_H

#include "TypeDef.h"
#include "ProIf.h"


struct TPro{
	void* pvPro;			//通信协议子类数据
	struct TProIf* pIf;		//通信接口

    BYTE bProMode; //协议
	//bool fLocal;
	//bool fAutoSend;			//是否具有主动上送的功能

	//虚函数，需要实例化为具体协议的对应函数
	int (* pfnRcvFrm)(struct TPro* pPro);		//主动接收并处理帧
	bool (* pfnRxFrm)(struct TPro* pPro, BYTE* pbRxBuf, int iLen);		//外部传入接收数据并处理帧
	int (* pfnRcvBlock)(struct TPro* pPro, BYTE* pbBlock, int nLen);	//组帧
	bool (* pfnHandleFrm)(struct TPro* pPro);	//处理帧

	bool (* pfnLogin)(struct TPro* pPro);			//登陆
	bool (* pfnLogoff)(struct TPro* pPro);			//登陆退出
	bool (* pfnBeat)(struct TPro* pPro);			//心跳
	bool (* pfnAutoSend)(struct TPro* pPro);		//主动上送
	bool (* pfnIsNeedAutoSend)(struct TPro* pPro);	//是否需要主动上送
	void (* pfnLoadUnrstPara)(struct TPro* pPro);	//装载非复位参数
	void (* pfnDoProRelated)(struct TPro* pPro);	//做一些协议相关的非标准的事情
	bool (* pfnSend)(struct TPro* pPro, BYTE* pbTxBuf, WORD wLen);	//对pIf->pIfpfnSend()的二次封装，可重载成特殊函数
	int (* pfnReceive)(struct TPro* pPro, BYTE* pbRxBuf, WORD wBufSize); //对pIf->pfnReceive()的二次封装，可重载成特殊函数，int可以返回错误
};	//通信协议基类结构


////////////////////////////////////////////////////////////////////////////////////////////
//Pro公共函数定义
void ProInit(struct TPro* pPro);

#endif //PROTO_H
