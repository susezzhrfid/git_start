/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrHook.cpp
 * 摘    要：本文件主要用来定义通信接口库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年4月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *			 $在这里不要定义成inline,方便和库文件一起编译时重定位
 *********************************************************************************************************/
#include "MtrHook.h"
#include "FaAPI.h"
#include "DbGbAPI.h"
#include "DbConst.h"
#include "SysDebug.h"
#include "ComAPI.h"
#include "LibDbAPI.h"
#include "DbAPI.h"
#include "MtrAPI.h"
#include "ComAPI.h"
#include "ExcTask.h"
#include "DbFmt.h"
#include "MtrCtrl.h"
#include "FlashMgr.h"

//参数：@iPhase 1单相；3三相，-1不判断
bool IsCtrlFlgMismatch(DWORD dwFlg, BYTE bMtrPro,
					   BYTE bProfMode, BYTE bDayMode, BYTE bDayFlgMode, 
					   BYTE bDemDayMode, BYTE bDemDayFlgMode, BYTE bMonSettMode, BYTE bDemMonSettMode, BYTE bIntervU)
{
	if ((bMtrPro!=MTRPRO_07 && (dwFlg&M97)==0) || 
		(bMtrPro==MTRPRO_07 && (dwFlg&M07)==0) ||	//电表协议不符合

		((dwFlg&PFCUR) && bMtrPro==MTRPRO_07 && bProfMode==1) ||		//当前曲线数据项，07表配置抄电表
		((dwFlg&PFMTR) && (bMtrPro!=MTRPRO_07 || bProfMode==0)) ||	//电表曲线数据项，97表不抄或配置成抄当前

		((dwFlg&DMCUR) && bIntervU==TIME_UNIT_DAY && bMtrPro==MTRPRO_07 && bDayMode==1) ||	//当前日数据项，07表配置抄电表冻结
		((dwFlg&DMMTR) && bIntervU==TIME_UNIT_DAY && (bMtrPro!=MTRPRO_07 || bDayMode==0)) ||	//电表日冻结数据项，97表不抄

		((dwFlg&DMCUR) && bIntervU==TIME_UNIT_MONTH && bMtrPro==MTRPRO_07 && bMonSettMode!=0) ||	//当前月冻结数据项，07表配置成抄电表冻结不抄
  		((dwFlg&DMMTR) && bIntervU==TIME_UNIT_MONTH && (bMtrPro!=MTRPRO_07 || bMonSettMode!=1)) ||	//电表当前/电表冻结数据项，月冻结抄实时或结算日不抄
		((dwFlg&MSETT) && bMonSettMode!=2) ||	//电表结算日数据项，非抄结算日模式不抄

		((dwFlg&DFCUR) && bDayFlgMode==1) ||	//当前抄表日日数据项，配置抄电表冻结
		((dwFlg&DFMTR) && bDayFlgMode==0) ||	//电表日月冻结数据项，配置抄电表当前

		((dwFlg&DEMDMCUR) && bIntervU==TIME_UNIT_DAY && bMtrPro==MTRPRO_07 && bDemDayMode==1) || 	//需量当前日数据项，07表配置抄电表冻结
		((dwFlg&DEMDMMTR) && bIntervU==TIME_UNIT_DAY && (bMtrPro!=MTRPRO_07 || bDemDayMode==0)) ||	//需量电表日冻结数据项，97表不抄或配置成抄当前

		((dwFlg&DEMDMCUR) && bIntervU==TIME_UNIT_MONTH && bMtrPro==MTRPRO_07 && bDemMonSettMode!=0) ||	//电表当前/电表冻结数据项，07表需量月冻结非抄当前不抄    
   		((dwFlg&DEMDMMTR) && bIntervU==TIME_UNIT_MONTH && (bMtrPro!=MTRPRO_07 || bDemMonSettMode!=1)) ||	//电表当前/电表冻结数据项，需量月冻结抄实时或结算日不抄    
		((dwFlg&DEMMSETT) && bDemMonSettMode!=2) ||	//需量电表结算日数据项，非抄结算日模式不抄

		((dwFlg&DEMDFCUR) && bDemDayFlgMode==1) ||	//需量当前抄表日日数据项，配置抄电表冻结
		((dwFlg&DEMDFMTR) && bDemDayFlgMode==0))	//需量电表日月冻结数据项，配置抄电表当前
	{
		return true;
	}
	else
	{
		return false;
	}
}

extern DWORD g_dwComTaskRecTm[DYN_PN_NUM][64];
//描述：数据项(主要针对g_CctRdCtrl[]中配置的)是否需要抄读
//返回:如果该数据项目前需要抄但还未抄、时间刚好符合则返回0,如果已经抄完了则返回1,如果需要抄但时间还没到则返回-1,
//	   如果该数据项对于该电表来说不需要抄则返回-2，其它情况返回-3
int IsIdNeedToRead(WORD wPn, const TMtrRdCtrl* pRdCtrl)
{
	const BYTE* pbFn = pRdCtrl->bFn;
	const DWORD* pdwFnFlg = pRdCtrl->dwFnFlg;
	DWORD dwFlg = 0;
	WORD i;
	WORD wID = pRdCtrl->wID;
	TTime now;
	BYTE bClass, bMtrPro, bProfMode, bDayMode, bDayFlgMode, bDemDayMode, bDemDayFlgMode, bMonthSettMode, bDemMonthSettMode;
	BYTE bFn = 0;
//	BYTE bMode = 0;	
	BYTE bNum = *pbFn++;
	BYTE bBuf[8];
	pdwFnFlg++;	//跳过个数

	bMtrPro = GetPnMtrPro(wPn);

	bProfMode = 0;
	ReadItemEx(BN24, PN0, 0x4110, &bProfMode);	//0x3003 1 曲线冻结模式字,0抄电表当前数据；1抄电表曲线
	if (wID==0xb61f && pRdCtrl->bIntervU==TIME_UNIT_HOUR)
		bProfMode = 0; //07表除电量示值曲线目前只支持抄当前

	bDayMode = 0;
	ReadItemEx(BN24, PN0, 0x4111, &bDayMode); //0x3004 1 日冻结模式字,0抄电表当前数据；1抄电表冻结

	bDayFlgMode = 0;
	ReadItemEx(BN24, PN0, 0x4112, &bDayFlgMode); //0x3005 1 抄表日冻结模式字,0抄电表当前数据；1抄电表冻结

	bDemDayMode = 0;
	ReadItemEx(BN24, PN0, 0x4113, &bDemDayMode); //0x3006 1 需量日冻结模式字,0抄电表当前数据；1抄电表冻结

	bDemDayFlgMode = 0;
	ReadItemEx(BN24, PN0, 0x4114, &bDemDayFlgMode); //0x3007 1 需量抄表日冻结模式字,0抄电表当前数据；1抄电表冻结
    
   	bMonthSettMode = 0;
	ReadItemEx(BN24, PN0, 0x4115, &bMonthSettMode); //0x3007 1 月冻结模式字,0抄电表当前数据；1抄电表冻结；2抄电表结算日
    
	bDemMonthSettMode = 0;
	ReadItemEx(BN24, PN0, 0x4116, &bDemMonthSettMode); //0x3007 1 需量月冻结模式字,0抄电表当前数据；1抄电表冻结；2抄电表结算日

	GetCurTime(&now);

	for (i=0; i<bNum; i++)
	{
		dwFlg = *pdwFnFlg++;
		bFn = *pbFn++;
		bClass = pRdCtrl->bIntervU==TIME_UNIT_MTR ? 1 : 2;

		if (IsCtrlFlgMismatch(dwFlg, bMtrPro, bProfMode, 
							  bDayMode, bDayFlgMode,	
							  bDemDayMode, bDemDayFlgMode,
                              bMonthSettMode, bDemMonthSettMode, pRdCtrl->bIntervU))
		{
			continue;
		}

		if (bFn == ALLPN)		//所有测量点特征字
		{
			//不做任何东西，就是为了避免进行if (!IsFnSupport(wPn, bFn, 2))的判断				
		}
		else if (bFn > FN_MAX)
		{
			if (GetErcType(bFn - ERCPN) == 0)
				continue;
		}
		else
		{
			BYTE bThrId = GetPnThread(wPn);
			if (IsComTaskDone(wPn, bFn, &now, g_dwComTaskRecTm[bThrId]))
				continue;
		}

		if (pRdCtrl->bIntervU == TIME_UNIT_DAY)		//日冻结限定在日切换后一个小时内抄完
		{
			if (now.nHour==0 && now.nMinute<pRdCtrl->tmStartTime.nMinute && bDayMode==1)	//抄电表冻结才延迟,抄实时不延迟
				return -1;	//如果需要抄但时间还没到则返回-1
		}

		/*if (pRdCtrl->bIntervU == TIME_UNIT_DAYFLG)
		{
			TTime tmRd = now;
			GetPnDate(wPn, bBuf);
			dwFlg = ByteToDWord(bBuf);
			if ((dwFlg & (1<<(now.nDay-1))) == 0) //不是抄表日
				continue;	//当前时间不符合抄读要求
			tmRd.nHour = BcdToByte(bBuf[5]);
			tmRd.nMinute = BcdToByte(bBuf[4]);
			tmRd.nSecond = 0; //保证到达起始分钟能执行
			dwFlg = SecondsPast(&tmRd, &now);
			if (dwFlg==0 || dwFlg>3600*2)
				continue;	//当前时间不符合抄读要求
		}*/

		if (pRdCtrl->bIntervU == TIME_UNIT_MONTH)
		{
			if (now.nDay!=1 || now.nHour!=0)
				continue;	//当前时间不符合抄读要求
			if (now.nMinute < pRdCtrl->tmStartTime.nMinute)
				return -1;	//如果需要抄但时间还没到则返回-1
		}

		//前面进行了基本判断，都目前为止都支持，下面进行一些特殊ID的判断 
		if (bMtrPro == MTRPRO_07 &&
			(wID==0x9a1f || wID==0x9a2f || (wID>=0x9b1f && wID<=0x9b6f) || (wID>=0x9c0f && wID<=0x9cbf))) //日冻结
		{
			DWORD dwDays;
			WORD wValidNum = 0;	
	
			TBankItem BankItem;
			BankItem.wBn = BN0;	//BANK号先定为多功能表,后面冻结的时候CctQueryItemTime()会自动动态调整
			BankItem.wPn = wPn;
			BankItem.wID = 0x9a00; //上次日冻结时间
	
			dwDays = DaysFrom2000(&now);
			QueryItemTimeMbi(dwDays*24*60*60, (dwDays+1)*24*60*60, &BankItem, 1, bBuf, &wValidNum);
			if (wValidNum == 0)	//上次日冻结时间还没抄到,或者时间不对,其它冻结数据项就先不抄了
				return -1;	//如果需要抄但时间还没到则返回-1,
		}

		return 0;	//对于当前FN来说，本测量点是支持的，返回需要抄读
	}

	return -2; //如果该数据项对于该电表来说不需要抄则返回-2
}


//描述:取得集抄数据项读取的时间范围(起始时间~结束时间)
//备注:可能有些协议不能简单根据RdCtrl.tmStartTime来确定开始时间和结束时间,可能需要根据参数来控制
// 	   到相应的case中修改即可
bool GetItemTimeScope(WORD wPn, const TMtrRdCtrl* pRdCtrl, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime)
{
	DWORD dwDayFlg;
	WORD wIntervV;
	TTime tmNow, tmStart, tmEnd;
	BYTE bMtrInterv;
	BYTE bBuf[10];

	tmNow = *pNow;
	tmNow.nSecond = 0;		//不考虑秒
	tmStart = tmNow;
	tmEnd = tmNow;

	switch (pRdCtrl->bIntervU)
	{
	case TIME_UNIT_MTR:		//抄表间隔
	case TIME_UNIT_STA:
		bMtrInterv = GetMeterInterv(wPn);
		if (bMtrInterv < 60)	//等同于分钟
		{
			wIntervV = bMtrInterv;
			if (wIntervV == 0)
				wIntervV = 15;

			tmStart.nMinute = tmStart.nMinute / wIntervV * wIntervV + pRdCtrl->tmStartTime.nMinute;	//按分钟间隔规整间隔的起始时间

			//求结束时间
			tmEnd = tmStart;
			AddIntervs(&tmEnd, TIME_UNIT_MINUTE, wIntervV);
		}
		else	//等同于小时
		{
			tmStart.nMinute = pRdCtrl->tmStartTime.nMinute;

			//求结束时间
			tmEnd = tmStart;
			tmEnd.nMinute = 0;
			tmEnd.nSecond = 0;
			AddIntervs(&tmEnd, TIME_UNIT_HOUR, 1);
		}

		break;

	case TIME_UNIT_MINUTE://分钟冻结,半小时按照分钟来配置
		//求开始时间
		wIntervV = pRdCtrl->bIntervV;
		if (wIntervV == 0)
			wIntervV = 15;

		tmStart.nMinute = tmStart.nMinute / wIntervV * wIntervV + pRdCtrl->tmStartTime.nMinute;	//按分钟间隔规整间隔的起始时间

		//求结束时间
		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_MINUTE, wIntervV);
		break;

	case TIME_UNIT_HOUR:	//小时
		//求开始时间
		wIntervV = pRdCtrl->bIntervV;
		if (wIntervV == 0)
			wIntervV = 1;

		tmStart.nMinute = pRdCtrl->tmStartTime.nMinute;
		tmStart.nHour = tmStart.nHour / wIntervV * wIntervV;	//按小时间隔规整间隔的起始时间

		//求结束时间
		tmEnd = tmStart;
		tmEnd.nMinute = 0;
		tmEnd.nSecond = 0;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, wIntervV);
		break;

	case TIME_UNIT_DAY:		//日冻结限定在日切换后一个小时内抄完
		//求开始时间
		tmStart.nHour = 0;
		tmStart.nMinute = 0; //pRdCtrl->tmStartTime.nMinute; 因为入库数据时标是按抄表归整的
		tmStart.nSecond = 0;

		//求结束时间
		tmEnd.nHour = 23;
		tmEnd.nMinute = 59;
		tmEnd.nSecond = 59;

		break;
	/*case TIME_UNIT_DAYFLG:		
		GetPnDate(wPn, bBuf);	//GetMtrDate(PORT_GB485, bBuf);
		
		dwDayFlg = ByteToDWord(bBuf);
		if ((dwDayFlg & (1<<(tmNow.nDay-1))) == 0) //不是抄表日
			return false;	//当前时间不符合抄读要求,返回false不抄
				
		//求抄表的开始时间
		tmStart.nHour = BcdToByte(bBuf[5]);
		tmStart.nMinute = BcdToByte(bBuf[4]); 		
		tmStart.nSecond = 0;
		wIntervV = GetMeterInterv(wPn);
        if (wIntervV == 0)  //必须防止除0
            return false; 
		dwDayFlg = TimeToMinutes(&tmStart)/wIntervV*wIntervV; //因为入库数据时标是按抄表归整的
		MinutesToTime(dwDayFlg, &tmStart);

		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_HOUR, 2);
		//tmEnd.nHour = 23;
		//tmEnd.nMinute = 59;
		//tmEnd.nSecond = 59;
		break;	*/
	case TIME_UNIT_MONTH:
		if (tmNow.nDay != 1)
			return false;	//当前时间不符合抄读要求,返回false不抄

		//求开始时间
		tmStart.nDay = 1;	//月冻结限死在1号冻结,国网要求在月末进行冻结
		tmStart.nHour = 0;		//pRdCtrl->tmStartTime.nHour;
		tmStart.nMinute = 0; //pRdCtrl->tmStartTime.nMinute; 因为入库数据时标是按抄表归整的
		tmStart.nSecond = 0;

		//求结束时间:当月1日23:59
		tmEnd.nDay = 1;
		tmEnd.nHour = 00;
		tmEnd.nMinute = 59;
		tmEnd.nSecond = 59;
		break;

	default:
		return false;
	}

/*	if (pRdCtrl->wID == 0x9a00) //固定查询当前抄表间隔的，为了每个间隔都去补抄一次
	{
		wIntervV = GetMeterInterv(wPn);
		dwDayFlg = TimeToMinutes(pNow)/wIntervV*wIntervV; //因为入库数据时标是按抄表归整的
		MinutesToTime(dwDayFlg, &tmStart);

		tmEnd = tmStart;
		AddIntervs(&tmEnd, TIME_UNIT_MINUTE, wIntervV);
	}*/

	*pdwStartTime = TimeToSeconds(&tmStart);
	*pdwEndTime = TimeToSeconds(&tmEnd);
	return true;
}


//描述:抄表故障确认(单个测量点),在故障的发生/恢复时候的回调函数,用来生成告警事件
void OnMtrErrEstb(WORD wPn, BYTE* pbData)	
{
	BYTE m = 0, n = 0;

	if (wPn > POINT0 && wPn < PN_NUM) 
	{
		m = (wPn)/8;
		n = (wPn)%8;

		g_bRdMtrAlr[m] |= 0x01<<n;

	}
}

//描述:抄表故障恢复(单个测量点),在故障的发生/恢复时候的回调函数,用来生成告警事件
void OnMtrErrRecv(WORD wPn)	
{
	BYTE m = 0, n = 0;

	if (wPn > POINT0 && wPn < PN_NUM) 
	{
		m = (wPn)/8;
		n = (wPn)%8;

		g_bRdMtrAlr[m] &= ~(0x01<<n);

	}		

}

//描述:通过电表地址转换为测量点号
//参数:@pb485Addr 电表地址,
//返回:如果找到匹配的测量点则返回测量点号,否则返回0
WORD MtrAddrToPn(const BYTE* pb485Addr)
{
	BYTE bMtrAddr[6];
	WORD wPn = 1;

	while (1)
	{
		if (wPn >= POINT_NUM)
			break;

		if ( !IsMtrPn(wPn) )
		{
			wPn++;
			continue;
		}

		GetMeterAddr(wPn, bMtrAddr);
		if (memcmp(pb485Addr, bMtrAddr, 6) == 0)
		{
			return wPn;
		}

		wPn++;
	}

	return 0;
}

//描述:统计集抄抄读信息
void DoMtrRdStat()
{
	BYTE bBuf[16];
	BYTE bPort;
	TTime tNow;
	BYTE bStat;
	WORD wMtrNum = 0, wSuccNum = 0; //要抄电表总数&& 抄表成功块数
	//static DWORD dwCurIntervSec[LOGIC_PORT_NUM] = {0};
	static bool fIsClr = true;


	for(bPort=LOGIC_PORT_MIN; bPort<=LOGIC_PORT_MAX; bPort++)
		GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);


	bPort = 31;
	GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);

	//入库
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
	if(IsAllAByte(bBuf, 0x00, sizeof(bBuf)))
		fIsClr = true;

	DWORDToBCD(wMtrNum, bBuf, 2); //要抄电表总数
	if (wMtrNum == 0)
		bStat = 0; //无表可抄时，填未抄表
	else if (wMtrNum == wSuccNum)
		bStat = 2; //全部抄完
	else
		bStat = 1; //当前抄表工作状态标志:正在抄表

	if(BcdToDWORD( &bBuf[2], 2) <= wSuccNum)
		DWORDToBCD(wSuccNum, &bBuf[2], 2); //抄表成功块数					

	if (bStat == 1)
	{
		if (wSuccNum>=0 && fIsClr)
		{
			fIsClr = false;
			GetCurTime(&tNow);
			TimeTo6Bcd(&tNow,bBuf+4);
            memset(bBuf+10, 0, 6); //结束时间清0
		}
	}
	else if (bStat == 2)
	{
		GetCurTime(&tNow);
		TimeTo6Bcd(&tNow,bBuf+10);
	}
	else
		memset(bBuf+4, 0, 12);

	WriteItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
}

void ClrMtrRdStat()
{
	BYTE bBuf[16];
	//入库
	memset(bBuf, 0, sizeof(bBuf));
	
	WriteItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
}
//删除表时需要更新集抄抄读信息
void UpdRdStat()
{
    TTime tNow;
    BYTE bBuf[16];
	BYTE bPort;
	BYTE bStat;
	WORD wMtrNum, wSuccNum; //要抄电表总数&& 抄表成功块数
    

	for(bPort=LOGIC_PORT_MIN; bPort<=LOGIC_PORT_MAX; bPort++)
		GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);


	bPort = 31;
	GetMtrNumByPort(bPort, &wMtrNum, &wSuccNum);

	//入库
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
	DWORDToBCD(wMtrNum, bBuf, 2); //要抄电表总数
	if (wMtrNum == 0)
		bStat = 0; //无表可抄时，填未抄表
	else if (wMtrNum == wSuccNum)
		bStat = 2; //全部抄完
	else
		bStat = 1; //当前抄表工作状态标志:正在抄表
	DWORDToBCD(wSuccNum, &bBuf[2], 2); //抄表成功块数					

	if (bStat == 2)
	{
		GetCurTime(&tNow);
		TimeToFmt1(&tNow,bBuf+10);
	}

	WriteItemEx(BN0, PN0, 0x8868, bBuf);	//C1F11
}

//描述:广播时间是否到达
bool IsBroadcastTime(BYTE bPort)
{
#ifdef PRO_698
	BYTE bBuf[120];
	TTime tmNow;	
	TTime tmTime;
	static DWORD dwLastMin[LOGIC_PORT_NUM] = {0};
	DWORD dwNow, dwMin;

	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return false;

	GetCurTime(&tmNow);
	dwNow = TimeToMinutes(&tmNow);
	if (dwLastMin[bPort - LOGIC_PORT_MIN] == dwNow)
		return false;

	dwLastMin[bPort - LOGIC_PORT_MIN] = dwNow; //端口号从2开始
	if (ReadItemEx(BN0, bPort-1, 0x021f, bBuf) > 0)	//F33
	{
		if (bBuf[1] & 0x8)  //广播校时使能 D3
		{
			GetCurTime(&tmTime);
			tmTime.nMinute = BcdToByte(bBuf[10]);
			tmTime.nHour = BcdToByte(bBuf[11]);            
			tmTime.nDay = BcdToByte(bBuf[12]);
			if (tmTime.nDay == 0)    //每天对时
			{
				if (tmTime.nHour==tmNow.nHour && tmTime.nMinute==tmNow.nMinute)        
					return true;
			}
			else
			{
				dwMin = TimeToMinutes(&tmTime);
				if (dwNow == dwMin)
					return true;
			}
		}
	}
	return false;
#endif
}

//描述:取广播校时最新时间的帧
BYTE GetAdjTimeFrm(BYTE* pbFrm)
{
	TTime time;
	WORD i;

	pbFrm[0] = 0x68;
	memset(&pbFrm[1], 0x99, 6);
	pbFrm[7] = 0x68;
	pbFrm[8] = 0x08; //cmd
	pbFrm[9] = 0x06; //len
	GetCurTime(&time);
	pbFrm[10] = ByteToBcd(time.nSecond&0xff);
	pbFrm[11] = ByteToBcd(time.nMinute&0xff);
	pbFrm[12] = ByteToBcd(time.nHour&0xff);
	pbFrm[13] = ByteToBcd(time.nDay&0xff);
	pbFrm[14] = ByteToBcd(time.nMonth&0xff);
	pbFrm[15] = ByteToBcd((time.nYear-2000)&0xff);

    //+0x33
    for (i=10; i<16; i++)
	{
  	    pbFrm[i] += 0x33;
	}	 
	pbFrm[16] = CheckSum(pbFrm, 16);
	pbFrm[17] = 0x16;

	return 18;
}

//描述:该函数实现表计监控操作时645帧透传功能
//返回:成功返回1,失败返回-1
int DoBroadcastFwdCmd(BYTE bPort, DWORD dwBaudRate)
{
	int iPort;
	TCommPara CommPara;
	char szBuf[128];
	BYTE bBuf[20];
	BYTE bFrmLen = GetAdjTimeFrm(bBuf);

	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return -1;

	iPort = MeterPortToPhy(bPort);
	if (iPort < 0)
		return -1;

	CommPara.wPort = (WORD)iPort;
	CommPara.dwBaudRate = dwBaudRate;	
	CommPara.bParity =  EVENPARITY;
	CommPara.bByteSize = 8;
	CommPara.bStopBits = ONESTOPBIT;

	GetDirRdCtrl(bPort);	//取得直抄的控制权
	if ( !MtrProOpenComm(&CommPara) )
	{
		ReleaseDirRdCtrl(bPort); //释放直抄的控制权
		return -1;
	}

	if (CommWrite(CommPara.wPort, bBuf, bFrmLen, 1000) != bFrmLen)
	{
		DTRACE(DB_FAPROTO, ("DoBroadcastFwdCmd: fail to write comm.\r\n")); 
		ReleaseDirRdCtrl(bPort); //释放直抄的控制权
		return -1;
	}
	ReleaseDirRdCtrl(bPort); //释放直抄的控制权
	sprintf(szBuf, "Port %d dwBaudRate %d DoBroadcastFwdCmd Adj Frm->", bPort, dwBaudRate);
	TraceBuf(DB_CRITICAL, szBuf, bBuf, bFrmLen);
	return 1;
}

void DoBroadcastMtrTime(BYTE bPort)
{
	DoBroadcastFwdCmd(bPort, CBR_1200);
	Sleep(500);
	DoBroadcastFwdCmd(bPort, CBR_2400);
	Sleep(500);
	DoBroadcastFwdCmd(bPort, CBR_4800);
	Sleep(500);
	DoBroadcastFwdCmd(bPort, CBR_9600);
}

void DoBrcastMtrTm()
{
	if (IsBroadcastTime(2))
		DoBroadcastMtrTime(2);
	if (IsBroadcastTime(3))
		DoBroadcastMtrTime(3);
	if (IsBroadcastTime(4))
		DoBroadcastMtrTime(4);
}
