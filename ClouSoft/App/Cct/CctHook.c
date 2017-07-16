/*********************************************************************************************************
* Copyright (c) 2009,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称: CctHook.cpp
* 摘    要: 本文件主要实现集抄挂钩接口,改写的挂钩主要来自集抄库,集抄普通任务等
* 当前版本: 1.1
* 作    者: 
* 完成日期: 2009年9月
* 备    注: 
*********************************************************************************************************/
//#include "stdafx.h"
#include "CctAPI.h"
#include "MtrAPI.h"
#include "CctHook.h"
#include "ComAPI.h"
#include "LibDbConst.h"
#include "LibDbAPI.h"
#include "DbConst.h"

//描述:取得载波节点的地址,对于通过采集终端的电表,则取采集终端的地址
//参数:@wPn 测量点号
//	   @pbAddr 用来返回地址
//返回:如果成功则返回true,否则返回false
bool GetPlcNodeAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bLink;
	BYTE bBuf[12];

	if (!IsCctPn(wPn))
		return false;
	
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf)<=0)  //测量点地址
		return false;

	ReadItemEx(BN0,wPn,0x8909,bBuf+6);  //测量点对应采集终端地址

// 	if (bLink & LINK_ACQ)
// 	{
// 		memcpy(pbAddr, bBuf+6, 6);
// 	}
// 	else
	{
		memcpy(pbAddr, bBuf, 6);
	}

	//地址有效性判断
	if(IsAllAByte(pbAddr, 0, 6) || IsAllAByte(pbAddr, 0xff, 6))
		return false;
	else
		return true;
}

// 描述:取广播校时最新时间的帧
// BYTE CctGetAdjTimeFrm(BYTE* pbFrm)
// {
// 	const BYTE bAddr[6] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
// 	TTime time;
// 
// 	GetCurTime(&time);
// 	pbFrm[10] = ByteToBcd(time.nSecond&0xff);
// 	pbFrm[11] = ByteToBcd(time.nMinute&0xff);
// 	pbFrm[12] = ByteToBcd(time.nHour&0xff);
// 	pbFrm[13] = ByteToBcd(time.nDay&0xff);
// 	pbFrm[14] = ByteToBcd(time.nMonth&0xff);
// 	pbFrm[15] = ByteToBcd((time.nYear-2000)&0xff);
// 
// 	return (BYTE )Make645Frm(pbFrm, bAddr, 0x08, 6);
// }

//描述:	获取电表地址,
//参数:	@wPn 测量点号
//		@pbAddr 用来返回地址
//返回:	如果成功则返回true,否则返回false
//备注:	对于载波表, 载波物理地址与电表地址一致,
//		对于采集器模型,这里取目的电表地址.
bool GetCctMeterAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bBuf[100];
	if (!IsCctPn(wPn))
		return false;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf)<=0)
		return false;

	memcpy(pbAddr, bBuf, 6);
	return true;
}

//描述:取得载波测量点屏蔽位,不包括采集器节点,但包括独立载波表节点和通过采集器采集的485表
void GetPlcPnMask(BYTE* pbPnMask)
{
	ReadItemEx(BN17, PN0, 0x7005, pbPnMask); 
}

//描述:取得载波测量点屏蔽位的读指针,不包括采集器节点,但包括独立载波表节点和通过采集器采集的485表
const BYTE* GetCctPnMask()
{
	return GetItemRdAddr(BN17, PN0, 0x7005); 
}

//描述:通过载波表地址转换为测量点号
//参数:@pb485Addr 电表地址,
// 				  对于独立载波表节点既是电表地址也是载波物理地址,
// 				  对于通过采集器采集的485表,只是电表地址
// 	   @pbAcqAddr 采集器地址,目前看来不需要传递这样的参数,只是留有这样的接口
// 				  对于独立载波表节点根本就不用传,置为NULL即可
// 				  对于通过采集器采集的485表,传递采集器地址
//返回:如果找到匹配的测量点则返回测量点号,否则返回0
WORD PlcAddrToPn(const BYTE* pb485Addr, const BYTE* pbAcqAddr)
{
	BYTE bMtrAddr[6];
	WORD wPn;


	for (wPn = 1; wPn < POINT_NUM; wPn++)
	{
		if(GetPlcNodeAddr(wPn, bMtrAddr))
		{
			if (memcmp(bMtrAddr, pb485Addr, 6) == 0)
			{
				return wPn;
			}
		}
		else
			continue;
	}

// 	const BYTE* pbPnMask = GetCctPnMask();	//取得载波测量点屏蔽位,不包括采集器节点,但包括独立载波表节点和通过采集器采集的485表
// 	if (pbPnMask == NULL)
// 		return 0;
// 
// 	wPn = 0;
// 	while (1)
// 	{
// 		wPn = SearchPnFromMask(pbPnMask, wPn);	//这里搜出的测量点都是载波的
// 		if (wPn >= POINT_NUM)
// 			break;
// 
// 		GetCctMeterAddr(wPn, bMtrAddr);
// 		if (memcmp(pb485Addr, bMtrAddr, 6) == 0)
// 		{
// 			return wPn;
// 		}
// 
// 		wPn++;
// 	}

	return 0;
}

//描述:是否在抄表时段内,把暂停抄表和恢复抄表的功能用本函数来实现
bool IsInReadPeriod()
{
	WORD  wMinStart = 0;
	WORD  wMinEnd = 0;

	return GetCurRdPeriod(&wMinStart, &wMinEnd);
}

//描述:取当前抄表时段的起始时间和结束时间
bool GetCurRdPeriod(WORD* pwMinStart, WORD* pwMinEnd)
{
	TTime tmNow;
	WORD  wMinNow = 0;
	WORD  wMinStart = 0;	//抄表时段起始时间
	WORD  wMinEnd = 0;		//抄表时段结束时间
	BYTE bBuf[120];

	//GetCurTime(&tmNow);
	//wMinNow = (WORD )tmNow.nHour*0x100 + tmNow.nMinute;

	//if (ReadItemEx(BN0, PORT_CCT_PLC, 0x021f, bBuf) > 0)	//FN33 包含通信端口号
	//{					//目前假定只有载波通道会用到抄表时段,所以固定用端口号PORT_CCT_PLC
	//	if (bBuf[0] == 0)	//没有配置
	//	{
	//		*pwMinStart = 0;
	//		*pwMinEnd = 24*0x100 + 0;
	//		return true;
	//	}

	//	if (bBuf[13] > 24)
	//		return false;

	//	for (WORD i=0; i<bBuf[13]; i++)
	//	{
	//		wMinStart = BcdToByte(bBuf[14+i*4]) + (WORD )BcdToByte(bBuf[15+i*4])*0x100;
	//		wMinEnd = BcdToByte(bBuf[16+i*4]) + (WORD )BcdToByte(bBuf[17+i*4])*0x100;

	//		if (wMinNow>=wMinStart && wMinNow<wMinEnd)
	//		{	
	//			*pwMinStart = wMinStart;
	//			*pwMinEnd = wMinEnd;
	//			return true;
	//		}
	//	}
	//}

	//*pwMinStart = 0;
	//*pwMinEnd = 0;
	//return false;	
	*pwMinStart = 0;
	*pwMinEnd = 24*0x100 + 0;
	return true;
}