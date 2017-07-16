/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProHook.c
 * 摘    要：本文件主要用来定义通信接口库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *********************************************************************************************************/
#include "FaAPI.h"
#include "ProAPI.h"
#include "FaCfg.h"
#include "ExcTask.h"
#include "ProHook.h"
#include "LibDbAPI.h"

//描述:累计流量的接口函数
//备注:可以转向调用统计类的相关函数,
// 	   最好只是先暂时更新统计类的成员变量,否则每次接收发送都写系统库,会影响通信效率
void AddFlux(DWORD dwLen)
{
	BYTE bKbData[4] = {0};
 #ifdef PRO_698
//  DWORD dwDayFlux = 0;
//  DWORD dwMonFlux = 0;
  if (!IsDownSoft())
  {
	//WaitSemaphore(m_semTermLog);			//统计的整个过程都要进行保护,
	
//	ReadItemEx(BN0, PN0, 0x1500, (BYTE*)&dwDayFlux); //698 C1F10
//	ReadItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);
	
//	dwDayFlux += dwLen;		//终端GPRS日流量
//	dwMonFlux += dwLen;		//终端GPRS月流量

//	WriteItemEx(BN0, PN0, 0x1500, (BYTE*)&dwDayFlux); //698 C1F10
//	WriteItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);

	g_PowerOffTmp.tTermStat.dwDayFlux += dwLen;		//终端GPRS日流量
	g_PowerOffTmp.tTermStat.dwMonFlux += dwLen;		//终端GPRS月流量

	DWORDToBCD(g_PowerOffTmp.tTermStat.dwDayFlux/1024,  bKbData, 2);
	WriteItemEx(BN0, PN0, 0x886a, bKbData); //698 C1F10
	DWORDToBCD(g_PowerOffTmp.tTermStat.dwMonFlux/1024,  bKbData, 3);
	WriteItemEx(BN0, PN0, 0x886c, bKbData);
  }
 #endif
}

//描述:累计流量的接口函数
bool IsFluxOver()	
{
	DWORD dwMonFlux = 0;
	DWORD dwFluxLimit = 0;
#ifdef PRO_698
    if (IsDownSoft())
    	return false;
	//DWORD dwMonFlux = 0;
    ReadItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);

	//DWORD dwFluxLimit = 0;
	ReadItemEx(BN0, PN0, 0x024f, (BYTE*)&dwFluxLimit); //F36：终端上行通信流量门限设置,月通信流量门限为0，表示系统不需要终端进行流量控制

	if (dwFluxLimit>0 && dwMonFlux>dwFluxLimit)
		return true;
	else
		return false;
#else
	return false;
#endif
}

//描述:回调函数,用于生成告警记录等用途
//备注:只需在第一次超流量的时候生成告警记录,这个由本函数或者它的调用函数来判断
void GprsOnFluxOver()	
{
#ifdef PRO_698
	BYTE bAlrBuf[16];
	
	TTime now;
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	GetCurTime(&now);

	ReadItemEx(BN0, PN0, 0x1501, &bAlrBuf[0]); //当月已发生的通信流量
	ReadItemEx(BN0, PN0, 0x024f, &bAlrBuf[4]); //月通信流量门限,F36：终端上行通信流量门限设置

// 	SaveAlrData(ERC_FLUXOVER, now, bAlrBuf,0,0);
	DTRACE(DB_METER_EXC, ("GprsOnFluxOver: ########## \n"));
#endif
}


//描述:GPRS是否处于在线时段
bool GprsIsInPeriod()
{	
  	BYTE bBuf[16];
#ifdef PRO_698
	if (ReadItemEx(BN0, PN0, 0x008f, bBuf) > 0) //C1F8
	{
        //时段在线模式允许在线时段标志：D0~D23按位顺序对应表示0~23点
		//置"1"表示允许在线时段，置"0"表示禁止在线时段，当相邻时段的设定值相同时，合并为一个长时段。

		TTime now;
		GetCurTime(&now);
		if (bBuf[5+now.nHour>>3] & (1<<(now.nHour&7)))
			return true;
	}

	return false;
#else
	return true;
#endif
}

//描述:GPRS通道的告警和主动上送数据是否都送完,遇到需要掉线的时候,等待告警和主动上送数据
//	   全部送完再掉线.如果不用等待告警和主动数据全部送完就可以掉线,直接返回true
//参数:@dwStartClick 流量超标的起始时标,用来控制在多少分钟内没送完也必须掉线
//返回:如果告警和主动数据全部送完则返回true,否则返回false
bool GprsIsTxComplete(DWORD dwStartClick) 
{
	return false;
}

//描述：把模块版本、CCID等信息更新到系统库，给MODEM库的回调接口，在更新完相关信息后调用
void UpdModemInfo(TModemInfo* pModemInfo)
{
	WriteItemEx(BN0, PN0, 0x21Ef, (BYTE *)pModemInfo);
}
