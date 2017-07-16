/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：STDReader.h
 * 摘    要：本文件实现了376.2路由的控制(自动路由控制)
 * 当前版本：1.0
 * 作    者：曾敏涛
 * 完成日期：2014年8月
 *
 * 备    注：$本文件为标准库的一部分,请不要将协议相关的部分加入到本文件
************************************************************************************************************/
#ifndef STDREADER_H
#define STDREADER_H

#include "Typedef.h"
#include "LoopBuf.h"
#include "AutoReader.h"
#include "LibCctConst.h"
#include "CctIf.h"
#include "ComStruct.h"
//#include "bios.h"

//是否使用新376.2协议
//#define EN_NEW_376_2   1
//#undef EN_NEW_376_2 


//帧控制域C
#define FRM_C_PRM_1		            0x01	//DIR,启动
#define FRM_C_PRM_0			        0x00	//DIR,从动

#define FRM_C_M_MST		        	0x01    //集中式路由载波通信
#define FRM_C_M_DIST			    0x02    //分布式路由载波通信
#define FRM_C_M_RADIO		        0x0a    //微功率无线通信
#define FRM_C_M_ETH			    	0x14    //以太网通信

#define TO_MOD_RT		0	//0表示对集中器的通信模块操作
#define TO_MOD_MTR		1	//1表示对载波表的通信模块操作

#define  ROUTETRSANMIT   //透传主站下发的376.2协议帧

typedef struct{
	TAutoRdrPara RdrPara;	//自动抄表器参数


	BYTE bLink;				//链路类型,用来区分不同厂家
	WORD wFrmLenBytes;      //帧长度中的字节个数,1表示1个字节,2表示2个字节
	BYTE bRunMode;          //东软模块运行模式，1-3.5代模式 2-3.5代扩展模式 3-3代模式

}TStdPara;		//698标准路由参数

typedef struct
{
	TStdPara* m_ptStdPara;
	TAutoReader* m_ptAutoReader;
	bool m_fSyncAllMeter;
	BYTE m_bKplvResetCnt;
	BYTE m_bKeepAliveMin;
	DWORD m_dwLastRxClick;
	DWORD m_dwResumeRdClick;	//上回路由不抄表,恢复抄表的时标
	DWORD m_dwUpdStatusClick;	//上回更新状态的时标
	DWORD m_dwSyncMeterClick;

	TTime m_tmUdp;		//最后一次更新时标

	TLoopBuf m_tLoopBuf;

	TCctCommPara m_tCctCommPara;
	
	BYTE* m_pbCctTxBuf;	//发送缓冲区
	BYTE* m_pbCctRxBuf; //接收缓冲区

	bool m_fRestartRoute;
	BYTE* m_pbStudyFlag;
}TStdReader;

bool InitStdReader(TStdReader* ptStdReader);

void InitRcv(TStdReader* ptStdReader);
WORD MakeFrm(WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE bAfn, WORD wFn, BYTE* pbData, WORD wDataLen, BYTE bAddrLen, BYTE bMode, BYTE bPRM, BYTE bCn, BYTE bRcReserved);
void SetAddrField(TStdReader* ptStdReader, BYTE* pbTxBuf, BYTE* pbAddr);
BYTE MakeCct645AskItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm, bool bByAcq, BYTE* pb485Addr, BYTE bCtrl);
void ReadMeter(TStdReader* ptStdReader, WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE* pbAddr, WORD wPn, DWORD dwID, BYTE bRdFlg, bool fByAcqUnit, BYTE bCn);
bool RxHandleFrm(TStdReader* ptStdReader, DWORD dwSeconds);
bool DefHanleFrm(TStdReader* ptStdReader);

bool SimpleTxRx(TStdReader* ptStdReader, DWORD dwLen, DWORD dwSeconds, BYTE bSendTimes);
bool CtrlRoute(TStdReader* ptStdReader, WORD wFn);

void HardReset(TStdReader* ptStdReader, BYTE bFn);
bool ResumeReadMeter(TStdReader* ptStdReader);
bool StopReadMeter(TStdReader* ptStdReader);
bool RestartRouter(TStdReader* ptStdReader);
bool IsAllowResumeRdMtr(TStdReader* ptStdReader); //允许恢复抄表
bool TcRestartRouter(TStdReader* ptStdReader);
//描述:	设置工作状态(载波模块)
void SetWorkMode(TStdReader* ptStdReader, BYTE bMode);
bool WriteMainNode(TStdReader* ptStdReader);
bool ReadMainNode(TStdReader* ptStdReader);

int ReadPlcNodes(TStdReader* ptStdReader, BYTE *pbBuf, WORD wStartPn);

BYTE  GetRdNum(); //同步表地址时一次读取的从节点个数

void SetSyncFlag(BYTE* pbSyncFlag, int iNum);
bool GetSyncFlag(BYTE* pbSyncFlag, int iNum);
void ClrSyncFlag(BYTE* pbSyncFlag, int iNum);
WORD  GetStartPn(WORD wLastPn, WORD wReadedCnt);

void AutoResumeRdMtr(TStdReader* ptStdReader);
bool TcRestartNewDay(TStdReader* ptStdReader);
bool DoCctInfo(TStdReader* ptStdReader); 
int RIDIsExist(BYTE* bMeterAddr, BYTE* pbBuf);
bool DelNode(TStdReader* ptStdReader, BYTE* pbAddr);
void ClearAllRid(TStdReader* ptStdReader, WORD wFn);	//清除所有节点(载波模块) 注意：友讯达和弥亚威使用F3清除所有测量点参数
bool AddOneNode(TStdReader* ptStdReader, WORD wNo, BYTE* pbAddr);
bool SyncMeterAddr(TStdReader* ptStdReader);
int  UpdateReader(TStdReader* ptStdReader);
int DoLearnAll(TStdReader* ptStdReader);
BYTE Make376TransFrm(TStdReader* ptStdReader, BYTE* pbAFn, BYTE bCtrl, BYTE* pbBuf, BYTE b645Len, BYTE bAddrLen);
BYTE CctMakeBroadcastFrm(TStdReader* ptStdReader, BYTE *pbBuf, BYTE* pbFrm, BYTE bFrmLen, BYTE *pbAddr);
bool IsNoConfirmBroad(); //广播命令是否无应答，晓程和御辉需重载，其余使用基类接口
BYTE CctMake3762Unlocked(TStdReader* ptStdReader, BYTE *pbCmdBuf, BYTE bLen);
int CctTransmit645CmdUnlocked(TStdReader* ptStdReader, BYTE *pbCmdBuf, BYTE bLen, BYTE *pbData, BYTE* pbrLen, BYTE bTimeOut);
int CctTransmit645Cmd(BYTE* pbCmdBuf, BYTE bLen, BYTE* pbData, BYTE* pbrLen, BYTE bTimeOut);

extern TStdReader* g_ptStdReader;

#endif //STDREADER_H

     