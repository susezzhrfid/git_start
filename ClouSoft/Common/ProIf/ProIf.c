/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：PotoIf.cpp
 * 摘    要：本文件实现了通信接口基类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：接口的状态切换: (休眠)->(复位)->(连接)->(登录)->(传输)
 *          各状态下错误计数的清零规则:各状态的错误计数由各状态自己控制,某状态不清零别的状态的计数
 *********************************************************************************************************/
#include "ProIf.h"
//#include "FaCfg.h"
//#include "FaConst.h"
#include "Info.h"
//#include "Trace.h"
//#include "sysapi.h"

////////////////////////////////////////////////////////////////////////////////////////////
//ProIf默认接口函数

//描述：休眠状态
bool ProIfDorman(struct TProIf* pProIf)
{
	return true;
}

//描述：复位接口
bool ProIfReset(struct TProIf* pProIf)
{
	return true;
}

//描述：发送函数
bool ProIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen)
{
	return true;
}

//描述：接收函数
int ProIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize)
{
	return true;
}

void ProIfTrans(struct TProIf* pProIf)
{
	struct TPro* pPro = pProIf->pPro;	//通信协议
	pPro->pfnRcvFrm(pPro); 	//接收到的一帧,并已经对其进行处理
							//如果需要直接使用串口的循环缓冲区以达到节省内存的目的，重载pfnRcvFrm()函数
}

//描述：接口复位成功时的回调函数
void ProIfOnResetOK(struct TProIf* pProIf)
{

}

//描述：接口复位失败时的回调函数
void ProIfOnResetFail(struct TProIf* pProIf)	
{

}

//描述：收到完整帧时的回调函数:主要更新链路状态,比如心跳等
void ProIfOnRcvFrm(struct TProIf* pProIf)
{
	pProIf->dwRxClick = GetClick();
}

//描述：接口相关特殊处理函数
void ProIfDoIfRelated(struct TProIf* pProIf)	
{

}

//描述：接口相关装载非复位参数
void ProIfLoadUnrstPara(struct TProIf* pProIf)
{

}

//描述：通信接口基类初始化
void ProIfInit(struct TProIf* pProIf)
{
	//外部必须配置的参数
	//char* pszName;			//接口名称
	//WORD wMaxFrmBytes;
	//void* pvIf;			//通信接口子类数据,根据bIfType的不同，指向TGprsIf、TCommIf等实例
	//struct TPro* pPro;	//通信协议

	//虚函数，需要实例化为具体接口的对应函数
	pProIf->pfnDorman = ProIfDorman;			//休眠
	pProIf->pfnReset = ProIfReset;				//复位接口
	pProIf->pfnSend = ProIfSend;				//发送函数
	pProIf->pfnReceive = ProIfReceive;			//接收函数
	pProIf->pfnTrans = ProIfTrans;				//传输状态函数
	pProIf->pfnOnResetOK = ProIfOnResetOK;		//接口复位成功时的回调函数
	pProIf->pfnOnResetFail = ProIfOnResetFail;	//接口复位失败时的回调函数
	pProIf->pfnOnRcvFrm = ProIfOnRcvFrm;		//收到完整帧时的回调函数
	pProIf->pfnDoIfRelated = ProIfDoIfRelated;	//接口相关特殊处理函数
   	pProIf->LoadUnrstPara = ProIfLoadUnrstPara;	//接口相关装载非复位参数

	//数据
	//BYTE bIfType;
	pProIf->dwRxClick = 0;	//最近一次接收到报文的时间
	pProIf->bState = IF_STATE_RST;   	//接口状态机:休眠->复位(MODEM复位、PPP拨号)->传输(客户端状态机：连接->登陆->通信->主动断开->空闲)

	pProIf->fExit = false;			//是否要退出
	pProIf->fExitDone = false;		//退出是否完成
}	


void SetMaxFrmBytes(struct TProIf* pProIf, WORD wMaxFrmBytes)
{
	pProIf->wMaxFrmBytes = wMaxFrmBytes;
}
