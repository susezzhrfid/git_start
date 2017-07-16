/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：PoIf.h
 * 摘    要：本文件实现了通信接口基类定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROTOIF_H
#define PROTOIF_H

#include "TypeDef.h"
#include "sysarch.h"
#include "Pro.h"
#include "ProIfCfg.h"

#define GPRS_MAX_BYTES     	1024       
#define SOCK_MAX_BYTES      GPRS_MAX_BYTES	//跟GPRS相同，好发现问题
#define COMM_MAX_BYTES      512	
#define SMS_MAX_BYTES       140
#define ETHER_MAX_BYTES     1024
#define PPP_MAX_BYTES       1024
#define WIRELESS_MAX_BYTES  256	

//接口类型
#define IF_UNKNOWN		0
#define IF_GPRS         1
#define IF_COMM     	2
#define IF_SOCKET       3	//基于操作系统socket套接字
#define IF_R230M		4	//230M电台
#define IF_WIRELESS    	5   //小无线

#define IF_RST_OK  		0  	//复位成功
#define IF_RST_HARDFAIL 1	//硬复位失败
#define IF_RST_SOFTFAIL 2	//软复位失败(协议层)

//接口状态机,接口的状态切换: (休眠)->(复位)->(连接)->(登录)->(传输)
#define IF_STATE_DORMAN  	0 //休眠
#define IF_STATE_RST  		1 //复位
//#define IF_STATE_CONNECT 	2 //连接
//#define IF_STATE_LOGIN  	3 //登录
#define IF_STATE_TRANS  	4 //传输	

#define IF_DEBUG_INTERV		(2*60)	//调试输出的间隔,单位秒

struct TProIf{
	//外部必须配置的参数
	void* pvIf;				//通信接口子类数据,根据bIfType的不同，指向TGprsIf、TCommIf等实例
	struct TPro* pPro;		//通信协议

	//虚函数，需要实例化为具体接口的对应函数
	bool (* pfnDorman)(struct TProIf* pProIf);			//休眠
	bool (* pfnReset)(struct TProIf* pProIf);			//复位接口
	bool (* pfnSend)(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen);			//发送函数
	int (* pfnReceive)(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize);	//接收函数,int支持错误返回
	void (* pfnTrans)(struct TProIf* pProIf);			//传输状态函数
	void (* pfnOnResetOK)(struct TProIf* pProIf);	//接口复位成功时的回调函数
	void (* pfnOnResetFail)(struct TProIf* pProIf);	//接口复位失败时的回调函数
	void (* pfnOnRcvFrm)(struct TProIf* pProIf);	//收到完整帧时的回调函数:主要更新链路状态,比如心跳等
	void (* pfnDoIfRelated)(struct TProIf* pProIf);	//接口相关特殊处理函数
   	void (* LoadUnrstPara)(struct TProIf* pProIf);	//接口相关非复位参数函数

	//数据
	char* pszName;		//接口名称，***如果需要改变默认值，请在调用相应接口的初始化函数后，直接赋值修改****
	WORD wMaxFrmBytes;	//最大帧长度，***如果需要改变默认值，请在调用相应接口的初始化函数后，直接赋值修改****
	BYTE bIfType;		//接口类型
	DWORD dwRxClick;	//最近一次接收到报文的时间
	BYTE  bState;   	//接口状态机:休眠->复位(MODEM复位、PPP拨号)->传输(客户端状态机：连接->登陆->通信->主动断开->空闲)
    bool fExit;			//是否要退出
    bool fExitDone;		//退出是否完成    
};	//通信接口基类结构


////////////////////////////////////////////////////////////////////////////////////////////
//ProIf公共函数定义
void ProIfInit(struct TProIf* pProIf);

void SetMaxFrmBytes(struct TProIf* pProIf, WORD wMaxFrmBytes);

#endif //PROTOIF_H
 
