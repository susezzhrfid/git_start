/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称: CctIf.h
 * 摘    要: 本文件主要实现376.2载波协议接口
 * 当前版本: 1.0
 * 作    者: 岑坚宇
 * 完成日期: 2014年8月
 *********************************************************************************************************/
#ifndef CCTIF_H
#define CCTIF_H

#include "Typedef.h"
#include "Comm.h"
#include "LibCctConst.h"

typedef struct
{
	TCommPara m_tCommPara;

	bool m_fRxComlpete;
	WORD m_wRxDataLen;
	WORD m_nRxStep;
	WORD m_nRxCnt;
	WORD m_wRxPtr;
	BYTE m_bResetCnt;
}TCctCommPara;

typedef struct{
	BYTE bFwdDepth;			//中继深度
	BYTE bModule;			//通信模块标识：TO_MOD_RT/TO_MOD_MTR
	//0表示对集中器的通信模块操作，1表示对载波表的通信模块操作
	BYTE bRt;				//路由标识：D0=0表示通信模块带路由或工作在路由模式，D0=1表示通信模块不带路由或工作在旁路模式。
	BYTE bCn;				//信道标识：取值0~15，0表示不分信道、1~15依次表示第1~15信道。
	BYTE bCnChar;			//电表通道特征
	BYTE bPhase;			//实测相线标识：实测从节点逻辑主信道所在电源相别，0为不确定，1~3依次表示相别为第1相、第2相、第3相。
	BYTE bAnsSigQlty;		//末级应答信号品质
	BYTE bCmdSigQlty;		//末级命令信号品质
	BYTE bRptEvtFlg;        //主动上报时间标识位
	BYTE bSN;				//报文序列号
}TUpInf;		//698标准路由上行信息域,只在本文件使用

bool CctProOpenComm(TCommPara* pCommPara);
WORD DtToFn(BYTE* pbDt);
void nToDt(WORD wFn, BYTE* pbDt);
void FnToDt(WORD wFn, BYTE* pbDt);
void GetUpInf(BYTE* pbBuf, TUpInf* pUpInf);
WORD CctProMakeFrm(WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE bAfn, WORD wFn, BYTE* pbData, WORD wDataLen, BYTE bAddrLen, BYTE bMode, BYTE bPRM, BYTE bCn, BYTE bRcReserved);
DWORD CctSend(TCommPara* pCommPara, BYTE *p, DWORD dwLen);
DWORD CctReceive(TCommPara* pCommPara, BYTE* p, DWORD wLen);
int CctRcvFrame(BYTE* pbBlock, int nLen, TCctCommPara* ptCctCommPara, WORD wFrmLenBytes, BYTE* pbCctRxBuf, DWORD* pdwLastRxClick);

#endif