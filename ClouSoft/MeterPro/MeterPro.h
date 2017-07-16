/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterPro.h
 * 摘    要：本文件主要包含抄表协议的基本API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef METERPRO_H
#define METERPRO_H

#include "ComAPI.h"
#include "Trace.h"
//#include "SysAPI.h"
#include "DbConst.h"
#include "SysDebug.h"
#include "Comm.h"
#include "MtrStruct.h"

#define MET_INVALID_VAL		0xffffffff  //无效数据

#define MET_ZXYG	0		//正向有功
#define MET_FXYG	1		//反向有功
#define MET_ZXWG	2		//正向无功
#define MET_FXWG	3		//反向无功
#define MET_WGX1	4		//一象限无功
#define MET_WGX2	5		//二象限无功
#define MET_WGX3	6		//三象限无功
#define MET_WGX4	7		//四象限无功
#define MET_GXWG	8		//感性无功
#define MET_RXWG	9		//容性无功
#define MET_ZXSZ	10		//正向视在
#define MET_FXSZ	11		//反向视在 

#define TXRX_RETRYNUM	2	//重发次数

#define MTR_FRM_SIZE  256//270    645协议的长度只有一个字节，不可能超过256

#define COM_TIMEOUT  1000	//串口缺省延时

////////////////////////////////
//电表参数
struct TMtrPro
{
	TMtrPara* pMtrPara;	//电表参数
	void*	pvMtrPro;	//类似this指针
	BYTE*	pbRxBuf; 
	BYTE*	pbTxBuf;	

	//各电表协议成员函数
	//bool(* pfnInit)(struct TMtrPro* pMtrPro); //电表协议初次初始化 
	int	(* pfnAskItem)(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf); //获取数据的对外接口函数
	// bool(* pfnBroadcastTime)(TMtrPro* pMtrPro, const TTime& time);//对时	
	
	bool (* pfnRcvBlock)(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //解析接收函数
	void (* pfnGetProPrintType)(BYTE* pbPrintPro, char* pszProName);//获取打印协议名称
}; 

////////////////////////////////////////////////////////////////////////////////////////////
//MtrProIf私有成员变量
//共用收发缓存
extern BYTE m_bInvdData;
extern BYTE m_MtrTxBuf[DYN_PN_NUM][MTR_FRM_SIZE];
extern BYTE m_MtrRxBuf[DYN_PN_NUM][MTR_FRM_SIZE];

/////////////////////////////////////////////////////////////////////////////////////
//各电表协议公共函数
void InitMeterPro();
int SchStrPos(char* pStr, int iStrLen, char c);
WORD GetCRC16(BYTE* pbyt,int iCount);
float POW10(signed char n); //10的n次方
int64 POW10N(signed char n);//10的n次方（n> 0）
int64 Round(int64 iVal64);  //四舍五入	
bool IsRateId(WORD wID);		//是否跟费率有关的计量ID
bool IsDemdId(WORD wID);		//是否需量ID（含需量发生时间）
bool IsDemdTime(WORD wID);		//是否需量时间
bool IsLastMonthId(WORD wID);	//是否上月ID	
bool IsPhaseEngId(WORD wID);	//是否分相电能ID
bool Is645NotSuptId(WORD wID);	//是否是645（97版）不支持需要快速返回的ID
BYTE GetBlockIdNum(WORD wID);	//取块ID的子ID个数
BYTE Get645TypeLength(WORD wID);//取645类型的数据长度
WORD SetCommDefault(WORD wID, BYTE* pbBuf); //设置通用格式ID的无效值
void CheckRate(BYTE* pbRateTab, BYTE* pbData, BYTE nlen);	//调整分费率
void CheckDecimal(BYTE bToltLen, BYTE bItemLen, BYTE bNewDec, BYTE bOldDec, BYTE* pbBuf); //调整小数位	
void CheckDecimalNew(BYTE bDstLen, BYTE bSrcLen, BYTE bDstDec, BYTE bSrcDec, BYTE* pbDstBuf, BYTE* pbSrcBuf);//调整某项的小数位
WORD Data645ToComm(WORD wID, BYTE* pbBuf, WORD wLen);//97版645协议格式数据转公共格式

WORD Id645V07toDL645(WORD wExtId);//将非2007版645协议读取的扩展ID转为读相应645ID，以兼容698终端上的读取
WORD Data645to645V07(WORD w645Id, BYTE* pbBuf, WORD wLen);//将97版645读回的数据转为2007版64的数据格式，以兼容698终端上的读取

/////////////////////////////////////////////////////////////////////////////////////
//串口操作函数
bool MtrProOpenComm(TCommPara* pComPara);//要判断串口是否已经打开,参数是否需要改变
WORD MtrProSend(WORD wPortNo, BYTE* pbTxBuf, WORD wLen); //要添加发送码打印

/////////////////////////////////////////////////////////////////////////////////////////
//内部接口函数
bool ReadCommFrm(struct TMtrPro* pMtrPro, void* pTmpInf, DWORD dwDelayTime, DWORD dwMaxSec, DWORD dwMinSec, DWORD dwTimeOut, 
				 DWORD dwReadLength, DWORD dwBufSize, BYTE* pbBuf, DWORD dwFrmSize);

#endif //METERPRO_H


