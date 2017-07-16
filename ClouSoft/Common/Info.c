/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Info.cpp
 * 摘    要：本文件实现应用层各线程间消息通信的机制
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备注：通知消息的原理
 * 		1.用来在参数或系统状态发生改变时,通知某个线程做相应更新
 × 		3.发起者多次把消息置成true,对接收者来说相当于只收到一个消息
 × 		4.当一个消息要发给多个线程时,应该每个线程都分配一个消息的标识
 *********************************************************************************************************/
//#include "stdafx.h"
#include "ComConst.h"
#include "FaConst.h"
#include "Info.h"
#include "sysarch.h"
#include <string.h>
//#include "sysapi.h"

static DWORD g_dwInfoClick;
static TSem g_semInfo;
static BYTE g_fInfo[INFO_NUM/8+1];  //每位可以表示一个信息
static BYTE g_bInfoDelayFlg[INFO_NUM/8+1];

void InitInfo()
{
	g_dwInfoClick = 0;
	g_semInfo = NewSemaphore(1, 1);
	memset(&g_fInfo, 0, sizeof(g_fInfo));
	memset(&g_bInfoDelayFlg, 0, sizeof(g_bInfoDelayFlg));
}

//描述:设置通知消息
//参数:@wID 消息标识
//	   @fInfo 消息值
void SetInfo(WORD wID)
{
	WaitSemaphore(g_semInfo, SYS_TO_INFINITE);
	
	if (wID < INFO_NUM)
		g_fInfo[wID>>3] |= 1<<(wID&7);    
	
	SignalSemaphore(g_semInfo);	
}

//描述:清除通知消息
//参数:@wID 消息标识
//	   @fInfo 消息值
void ClrInfo(WORD wID)
{
	WaitSemaphore(g_semInfo, SYS_TO_INFINITE);

	if (wID < INFO_NUM)
        g_fInfo[wID>>3] &= ~(1<<(wID&7));

	SignalSemaphore(g_semInfo);	
}

//描述:设置延时到达的通知消息,主要针对数据库里的的参数被通信协议修改了,
//	   在修改后不是马上就把消息发送给线程,而是在通信协议不再修改参数后,
//	   再把消息发送给线程
//	   延时又分为长延时和短延时
//参数:@wID 消息标识
void SetDelayInfo(WORD wID)
{
	if (wID < INFO_NUM)
	{
		DWORD dwClick = GetClick();

		WaitSemaphore(g_semInfo, SYS_TO_INFINITE);

        g_fInfo[wID>>3] |= 1<<(wID&7);    
		g_dwInfoClick = dwClick;	//即使无延时,时间还是更新
		if (wID < INFO_NO_DELAY_START)
			g_bInfoDelayFlg[wID>>3] |= 1 << (wID&7);
		
		SignalSemaphore(g_semInfo);
	}
}


//描述:取得通知消息,如果消息为true,会自动把消息清成false
//返回:消息值
bool GetInfo(WORD wID)
{
	WORD wIndex;
	BYTE bFlg;
	bool fRet = false;
	
	if (wID < INFO_NUM)
	{
		DWORD dwClick = GetClick();
		
		WaitSemaphore(g_semInfo, SYS_TO_INFINITE);
		
		wIndex = wID >> 3;
		bFlg = 1 << (wID&7);
		if (g_bInfoDelayFlg[wIndex] & bFlg) //设置了延时到达的通知消息
		{
			if (g_fInfo[wID>>3]&(1<<(wID&7)))
			{	
				if (wID < INFO_SHORT_DELAY_START)
				{	
					if (dwClick-g_dwInfoClick > INFO_LONG_DELAY) //等到长延时时间到才把消息给应用线程
						fRet = true;
				}
				else if (wID < INFO_NO_DELAY_START)
				{	
					if (dwClick-g_dwInfoClick > INFO_SHORT_DELAY) //等到长延时时间到才把消息给应用线程
						fRet = true;
				}
				else
				{
					fRet = true;
				}
			}
		}
		else if (g_fInfo[wID>>3]&(1<<(wID&7))) //没有设置延时到达的通知消息
		{					   //马上提交	
			fRet = true;
		}
		
		if (fRet)
		{
			g_bInfoDelayFlg[wIndex] &= ~bFlg;
			g_fInfo[wID>>3] &= ~(1<<(wID&7));
		}
		SignalSemaphore(g_semInfo);
	}
	
	return fRet;
}
