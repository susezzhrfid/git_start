/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctPara.cpp
 * 摘    要：本文件主要实现对集抄参数的装载
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2009年9月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
//#include "stdafx.h"
//#include "syscfg.h"
#include "AutoReader.h"

#include "DbAPI.h"
#include "CctHook.h"
#include "FaAPI.h"
#include "CctAPI.h"
#include "DrvCfg.h"
//描述:装载自动抄表器参数
//备注:可以根据不同的通道bLink来进一步特殊化
//描述:路由器运行模式

void LoadAutoRdrPara(BYTE bLink, TAutoRdrPara* pRdrPara)
{
	DWORD dwAddr = 0;
	BYTE bBuf[16];
	pRdrPara->bDayFrzDelay = 1;		//日冻结抄读延迟时间,单位分钟,主用用来避免终端时间比电表快

// 	//消息宏定义,避免编译过程中库用到的宏定义跟应用程序不一致
// 	pRdrPara->wInfoPlcPara = INFO_PLC_PARA;   //载波参数更改
// 	pRdrPara->wInfoPlcClrRt = INFO_PLC_CLRRT; //清路由
// 	pRdrPara->wInfoPlcStopRd = INFO_PLC_STOPRD; //停止抄表
// 	pRdrPara->wInfoPlcResumeRd = INFO_PLC_RESUMERD;	 //恢复抄表
// 	pRdrPara->wInfoPlcRdAllRtInfo = INFO_PLC_RDALLRTINFO;	//读取所有节点的中继路由信息
// 	pRdrPara->wInfoUpdateRT = INFO_PLC_UPDATE_ROUTE;       //载波路由器升级
// 	pRdrPara->wInfoChannelChange = INFO_PLC_WIRLESS_CHANGE;       //无线通信参数变更
// 	//pRdrPara->wInfoManualNetwork = INFO_RF_MANUAL_NETWORK;
	//pRdrPara->wInfoPlcSetRfCh = INFO_RF_SET_CH;	//无线设置信道号
	//pRdrPara->wInfoCmdNetwork = INFO_RF_CMD_NETWORK;//无线命令组网
	//pRdrPara->wInfoCmdStopNetwork = INFO_RF_STOP_NETWORK;//无线停止组网
	//pRdrPara->wInfoDelRfNode = INFO_RF_DEL_NODE;
	
	pRdrPara->fUseLoopBuf = true;		//是否使用循环缓冲区
	
	memset(pRdrPara->bMainAddr, 0, sizeof(pRdrPara->bMainAddr));	//主节点地址
	ReadItemEx(BN0, PN0, 0x8020, pRdrPara->bMainAddr); //行政区划码A1
	ReadItemEx(BN0, PN0, 0x8021, (BYTE*)&dwAddr);	//终端地址A2
	//IntToBCD(dwAddr, pRdrPara->bMainAddr+3 ,3);
	memcpy(pRdrPara->bMainAddr+3,&dwAddr,3);

	/*ReadItemEx(BN0, PN0, 0x231f, bBuf);
	memcpy(bBuf, pRdrPara->bMainAddr, 6);
	WriteItemEx(BN0, PN0, 0x231f, bBuf);*/
	//pRdrPara->bMainAddr+2
	////补抄及选抄的时段标志
	//if (bLink==AR_LNK_ES || bLink==AR_LNK_STD_ES)
	//{
	//   ReadItemEx(BN15, PN0, 0x5314, pRdrPara->bReRdTmFlg);
	//   ReadItemEx(BN15, PN0, 0x5315, pRdrPara->bSelRdTmFlg);
	//}
	//else
	//{
 //      ReadItemEx(BN15, PN0, 0x5312, pRdrPara->bReRdTmFlg);
	//   ReadItemEx(BN15, PN0, 0x5313, pRdrPara->bSelRdTmFlg);
	//}
}

//描述:装载698.42路由参数
void LoadStdPara(BYTE bLink, TStdPara* pStdPara)
{
	BYTE bRunMode = 1;
	memset(pStdPara, 0, sizeof(TStdPara));
	LoadAutoRdrPara(bLink, &pStdPara->RdrPara);
    
	pStdPara->wFrmLenBytes = 2;
	pStdPara->bLink = bLink;

	//if (bLink == AR_LNK_STD_ES)
	//{
		ReadItemEx(BN15, PN0, 0x5321, &bRunMode);
		pStdPara->bRunMode = bRunMode;
	//}
	
	pStdPara->RdrPara.bLogicPort = PORT_CCT_PLC;	//逻辑端口号,指的是通信协议上给每个通道规定的端口号,而不是物理端口号
	pStdPara->RdrPara.bPhyPort = COMM_METER3;	//物理端口号
}
