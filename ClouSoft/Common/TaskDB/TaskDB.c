/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TaskDB.c
 * 摘    要：任务数据库实现（任务数据管理，任务记录管理，任务搜索）
  *
 * 当前版本：1.0.0
 * 作    者：吴若文
 * 完成日期：2011-03-23
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
 * 备    注：1、任务库使用的时候需要像使用文件一样，先成功打开然后才能执行相关操作
 *           2、任务库添加记录不需要再设置保存信息，排序信息等。全部由任务库安排好
 *			 3、从任务库读取记录的时候，不再支持搜索条件以及搜索结果排序条件的设置，这此工作改由应用程序完成。
 *				应用程序按自己的需求将主站读记录的请求拆分，再一条条从任务库读取
 * 版本信息:
 ---2011-03-14:---V1.01----吴若文---	
 	1、
************************************************************************************************************/
//#include "stdafx.h"
#include <stdio.h>
#include "TaskDB.h"
#include "CommonTask.h"
#include "DbConst.h"
#include "FlashMgr.h"
#include "ComConst.h"
#include "SysDebug.h"
#include "ComAPI.h"
#include "DbAPI.h"


#define TDB_VERSION			"1.0.0"
#define TDB_RECBUF_LEN      256	//没有记录超过256个字节长度。


TTdbCtrl	g_TdbCtrl;

//描述：任务库初始化
//参数：NONE
//返回：成功返回true
bool TdbInit()
{
	memset(&g_TdbCtrl, 0, sizeof(g_TdbCtrl));
	g_TdbCtrl.fIsLocked = false;
	g_TdbCtrl.bFn = INVALID_FN;
    
    g_TdbCtrl.semTdb = NewSemaphore(1, 1);

	if (!FlashInit())
		return false;

	DTRACE(DB_TASKDB, ("TaskDB::Init:Ver %s.\r\n", TDB_VERSION));

	return true;
}


//描述：任务库是否上锁
//参数：NONE
//返回：上锁返回true
bool TdbIsLock()
{
	return g_TdbCtrl.fIsLocked;
}

//描述：给任务库上锁
//参数：NONE
//返回：NONE
void TdbLock()
{
	g_TdbCtrl.fIsLocked = true;
}

//描述：给任务库解锁
//参数：NONE
//返回：NONE
void TdbUnLock()
{
	g_TdbCtrl.fIsLocked = false;
}


void TdbWaitSemaphore()
{
	WaitSemaphore(g_TdbCtrl.semTdb, SYS_TO_INFINITE);
}

void TdbSignalSemaphore()
{
	SignalSemaphore(g_TdbCtrl.semTdb);
}

//描述：计算单个普通任务需要的存储空间
//参数：@bFn普通任务号
//返回：需要使用的空间
DWORD CalcSpacePerComTask(BYTE bFn)
{
	g_TdbCtrl.bSmplIntervU = GetComTaskSmplIntervU(bFn);
	g_TdbCtrl.bSmplIntervV = GetComTaskSmplInterV(bFn);
	g_TdbCtrl.dwPerDataLen = GetComTaskPerDataLen(bFn)+5;
	return g_TdbCtrl.dwPerDataLen*GetComTaskRecNum(bFn);
}

//描述：计算一笔普通任务相对于该任务起始存储偏移
//参数：@bFn普通任务号
//		@bBuf该笔记录时间
//返回：相对于任务存储首地址偏移
DWORD CalcComTaskOneRecOffSet(BYTE bFn, BYTE* bBuf)
{
	BYTE bWeek, bTmBuf[5];
	TTime tmRec;
	DWORD dwDay, dwOffSet = 0;
	memset(&tmRec, 0, sizeof(tmRec));
	memcpy(bTmBuf, bBuf, sizeof(bTmBuf));
	Swap(bTmBuf, sizeof(bTmBuf));
	Fmt15ToTime(bTmBuf, &tmRec);
	switch(g_TdbCtrl.bSmplIntervU)
	{
		case TIME_UNIT_MINUTE://分
		{
			bWeek = DayOfWeek(&tmRec);
			if (g_TdbCtrl.bSmplIntervV != 0)
				dwOffSet = ((bWeek-1)*1440 + tmRec.nHour*60 + tmRec.nMinute)/g_TdbCtrl.bSmplIntervV; //每分钟一笔,可存一个星期
			break;
		}
		case TIME_UNIT_HOUR://时
		{
			dwDay = DaysFrom2000(&tmRec);
			dwOffSet = dwDay%15*24 + tmRec.nHour; //每小时一笔,可存半个月
			break;
		}
		case TIME_UNIT_DAY://日
		{
			dwOffSet = tmRec.nDay - 1; //每天存一笔,可存一个月
			break;
		}
		case TIME_UNIT_MONTH://月
		{
			dwOffSet = tmRec.nMonth - 1; //每月存一笔,可存一年
			break;
		}
	}

	dwOffSet *= g_TdbCtrl.dwPerDataLen; //5字节是记录时间
	return dwOffSet;
}

//描述：计算单个中继任务需要的存储空间
//参数：@bFn中继任务号
//返回：需要使用的空间
DWORD CalcSpacePerFwdTask(BYTE bFn)
{
	g_TdbCtrl.bSmplIntervU = GetFwdTaskSmplIntervU(bFn);
	g_TdbCtrl.bSmplIntervV = GetFwdTaskSmplInterV(bFn);
	g_TdbCtrl.dwPerDataLen = GetFwdTaskPerDataLen(bFn)+5;
	return g_TdbCtrl.dwPerDataLen*GetFwdTaskRecNum(bFn);
}

//描述：计算一笔中继任务相对于该任务起始存储偏移
//参数：@bFn中继任务号
//		@bBuf该笔记录时间
//返回：相对于任务存储首地址偏移
DWORD CalcFwdTaskOneRecOffSet(BYTE bFn, BYTE* bBuf)
{
	BYTE bWeek, bTmBuf[5];
	TTime tmRec;
	DWORD dwDay, dwOffSet = 0;
	memset(&tmRec, 0, sizeof(tmRec));
	memcpy(bTmBuf, bBuf, sizeof(bTmBuf));
	Swap(bTmBuf, sizeof(bTmBuf));
	Fmt15ToTime(bTmBuf, &tmRec);
	switch(g_TdbCtrl.bSmplIntervU)
	{
		case TIME_UNIT_MINUTE://分
		{
			bWeek = DayOfWeek(&tmRec);
			if (g_TdbCtrl.bSmplIntervV != 0)
				dwOffSet = ((bWeek-1)*1440 + tmRec.nHour*60 + tmRec.nMinute)/g_TdbCtrl.bSmplIntervV; //每分钟一笔,可存一个星期
			break;
		}
		case TIME_UNIT_HOUR://时
		{
			dwDay = DaysFrom2000(&tmRec);
			dwOffSet = dwDay%15*24 + tmRec.nHour; //每小时一笔,可存半个月
			break;
		}
		case TIME_UNIT_DAY://日
		{
			dwOffSet = tmRec.nDay - 1; //每天存一笔,可存一个月
			break;
		}
		case TIME_UNIT_MONTH://月
		{
			dwOffSet = tmRec.nMonth - 1; //每月存一笔,可存一年
			break;
		}
	}

	dwOffSet *= g_TdbCtrl.dwPerDataLen; //5字节是记录时间
	return dwOffSet;
}


//描述：计算bFn需要多少空间，单位：BYTE
//参数：@bFn 二类冻结的Fn
//返回：需要扇区数
DWORD CalcFnNeedSpaces(BYTE bFn)
{
	const TCommTaskCtrl *pTaskCtrl = ComTaskFnToCtrl(bFn);

	if (bFn < FN_COMSTAT)
	{
		if (pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_P0)
		{
			return CalcSpacesPerPn(pTaskCtrl); //终端数据只分配一份
		}
		else	
			return CalcSpacesPerPn(pTaskCtrl) * PN_VALID_NUM;
	}
	else if (bFn < FN_FWDSTAT)
	{
		return CalcSpacePerComTask(bFn-FN_COMSTAT);
	}
	else if (bFn < FN_MAX)
	{
		return CalcSpacePerFwdTask(bFn-FN_FWDSTAT);
	}
	else
		return 0;
}

//描述：计算bFn需要多少扇区的空间，单位：section
//参数：@bFn 二类冻结的Fn
//返回：需要扇区数
WORD CalcFnNeedSects(BYTE bFn)
{
	DWORD dwNeedSpaces = CalcFnNeedSpaces(bFn);

	return (dwNeedSpaces%(EXSECT_SIZE-2)? dwNeedSpaces/(EXSECT_SIZE-2) + 1 : dwNeedSpaces/(EXSECT_SIZE-2));
}

//描述：计算单个Pn的bFn数据需要多少空间，单位：Byte，以TCommTaskCtrl为对象
//参数：@pTaskCtrl bFn的任务控制结构
//返回：需要使用的空间总和
DWORD CalcSpacesPerPn(const TCommTaskCtrl *pTaskCtrl)
{
	//曲线数据每个测量点
	return CalcPerPnMonRecSize(pTaskCtrl) * ComTaskGetMonthNum(pTaskCtrl);
}

//描述：计算单个Pn单个月的bFn数据需要多少空间，单位：Byte，以TCommTaskCtrl为对象
//参数：@pTaskCtrl bFn的任务控制结构
//返回：需要使用的空间
DWORD CalcPerPnMonRecSize(const TCommTaskCtrl *pTaskCtrl)
{
	DWORD dwRecSize;

	if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)//月冻结以及抄表日冻结，一月一笔
		dwRecSize = ComTaskGetDataLen(pTaskCtrl) + ComTaskGetRecOffset(pTaskCtrl) + 1;	//+1 加的是校验
	else
		dwRecSize = CalcPerPnDayRecSize(pTaskCtrl) * ComTaskGetRecNumPerPnMon(pTaskCtrl);

	return dwRecSize;
}

//描述：计算单个Pn单天的bFn数据需要多少空间，单位：Byte，以TCommTaskCtrl为对象
//参数：@pTaskCtrl bFn的任务控制结构
//返回：需要使用的空间
DWORD CalcPerPnDayRecSize(const TCommTaskCtrl *pTaskCtrl)
{
	WORD wRecSize;

	if (pTaskCtrl == NULL)
		return 0;
	
	if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
		return 0;

	wRecSize = ComTaskGetDataLen(pTaskCtrl);
	
	if (pTaskCtrl->bIntervU < TIME_UNIT_DAY)
	{
		wRecSize *= 24;
		wRecSize += CURVE_PER_DAY*ComTaskGetRecOffset(pTaskCtrl);	//曲线数据这里不是1天的数据，只是1笔曲线数据（6个小时1笔）
		wRecSize += CURVE_PER_DAY;	//+1 加的是校验
	}
	else
	{
		wRecSize += ComTaskGetRecOffset(pTaskCtrl);	//曲线数据这里不是1天的数据，只是1笔曲线数据（6个小时1笔）
		wRecSize += 1;	//+1 加的是校验
	}

	return wRecSize;
}


//描述：计算单个Pn曲线一笔记录的bFn数据需要多少空间，单位：Byte，以TCommTaskCtrl为对象
//参数：@pTaskCtrl bFn的任务控制结构
//返回：需要使用的空间
DWORD CalcOneCurveRecSize(const TCommTaskCtrl *pTaskCtrl)
{
	WORD wRecSize;

	if (pTaskCtrl == NULL)
		return 0;

	if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
		return 0;

	wRecSize = ComTaskGetDataLen(pTaskCtrl);

	if (pTaskCtrl->bIntervU < TIME_UNIT_DAY)
		wRecSize *= (24/CURVE_PER_DAY);	//6

	wRecSize += ComTaskGetRecOffset(pTaskCtrl);	//曲线数据这里不是1天的数据，只是1笔曲线数据（6个小时1笔）
	wRecSize += 1;	//+1 加的是校验

	return wRecSize;
}


//描述：从[wByte].bBitOffset开始，在pbFat文件分配表中标志nCount个扇区，
//参数：@pbFat 文件分配表
//		@wByte 文件分配表字节上的偏移
//		@bBit 文件分配表字节里位的偏移
//		@nCount 标志的个数
//返回：NONE
bool SectMarkUsed(BYTE* pbFat, WORD wByte, BYTE bBit)
{
	if (wByte>=FAT_FLG_SIZE || bBit>7) //检查参数
		return false;

	pbFat[wByte] |= (0x01<<bBit);

	return true;
}

//描述：检查bTime和tTime是否匹配
//参数：@bTime BYTE格式的时间
//		@tTime TTime格式的时间
//		@bIntervU 冻结类型
//返回：如果匹配返回true，否则返回false
bool IsSchTimeMatch(BYTE *bTime, TTime tTime, BYTE bIntervU)
{
	 if (bIntervU == TIME_UNIT_MONTH)//月冻结需要年-月就可以了
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_HOUR)	//曲线读回来的bTime格式为分-时-日-月-年 ，其中分-时都为0
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[2]))   )
			return true;
		else
			return false;
	}
	else if ((bIntervU==TIME_UNIT_DAY))	//其余需要判断时标的格式均为年-月-日
	{
		//曲线冻结要定位到小时，需要年-月-日
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[2]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_MINUTE)
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[2]))
			&& (tTime.nHour==BcdToByte(bTime[3]))
			&& ((tTime.nMinute)==BcdToByte(bTime[4]))  )
			return true;
		else
			return false;
	}

	return false;
}

//描述：检查bTime和tTime是否匹配
//参数：@bTime BYTE格式的时间
//		@tTime TTime格式的时间
//		@bIntervU 冻结类型
//返回：如果匹配返回true，否则返回false
bool IsSchRecTimeMatch(BYTE *bTime, TTime tTime, BYTE bIntervU)
{
	if (bIntervU == TIME_UNIT_MINUTE)
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[4]))
			&& (tTime.nMonth==BcdToByte(bTime[3]))
			&& (tTime.nDay==BcdToByte(bTime[2]))
			&& (tTime.nHour==BcdToByte(bTime[1]))
			&& ((tTime.nMinute)==BcdToByte(bTime[0]))  )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_MONTH)//月冻结需要年-月就可以了
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[1]))
			&& (tTime.nMonth==BcdToByte(bTime[0]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_HOUR)	//曲线读回来的bTime格式为分-时-日-月-年 ，其中分-时都为0
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[4]))
			&& (tTime.nMonth==BcdToByte(bTime[3]))
			&& (tTime.nDay==BcdToByte(bTime[2]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_DAY)	//其余需要判断时标的格式均为年-月-日
	{
		//曲线冻结要定位到小时，需要年-月-日
		if (   (tTime.nYear%100==BcdToByte(bTime[2]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[0]))   )
			return true;
		else
			return false;
	}
	
	return false;
}

//-----------------------------------------------------------------------
//		通过管道取得的记录格式，明确如下：
//		日冻结、抄表日冻结
//		时间(年月日)+测量点号(BYTE)+数据------日->月->年->PN->Data
//		月冻结
//		时间(年月)+测量点号(BYTE)+数据--------月->年->PN->Data
//		曲线
//		时间(FMT15)+测量点号(BYTE)+数据-------分->时->日->月->年->PN->Data
//-----------------------------------------------------------------------
//描述：取出数据头里的月数据内容
//参数：@*pbData 管道里传过来的数据包
//		@*pTaskCtrl 控制结构
//返回：返回pbData里对应于pTaskCtrl格式的月数据内容
BYTE GetMonth(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl)
{
// 	if (pTaskCtrl->bIntervU==TIME_UNIT_HOUR || pTaskCtrl->bIntervU==TIME_UNIT_MINUTE)
// 		return BcdToByte(*(pbData+3));
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAY)
// 		return BcdToByte(*(pbData+1));
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
// 		return BcdToByte(*pbData);
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAYFLG)
// 		return BcdToByte(*(pbData+1));
	if (pTaskCtrl->bIntervU)
	{
		return BcdToByte(*(pbData+1));
	}
	else
		return 0;
}

//描述：取出数据头里的日数据内容
//参数：@*pbData 管道里传过来的数据包
//		@*pTaskCtrl 控制结构
//返回：返回pbData里对应于pTaskCtrl格式的日数据内容
BYTE GetDay(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl)
{
// 	if (pTaskCtrl->bIntervU==TIME_UNIT_HOUR || pTaskCtrl->bIntervU==TIME_UNIT_MINUTE)
// 		return BcdToByte(*(pbData+2));
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAY)
// 		return BcdToByte(*pbData);
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAYFLG)
// 		return BcdToByte(*pbData);
	if (pTaskCtrl->bIntervU)
	{
		return BcdToByte(*(pbData+2));
	}
	else
		return 0;
}

//描述：为bFn申请Flash空间
//参数：@bFn 二类冻结的Fn
//返回：如果正确申请到空间，返回申请空间的首地址（一个大于0的正数）
//      如果申请失败，会返回小于0的错误码
//注意：申请空间的文件分配表结构为如下定义：
//		文件分配表不是根据Flash地址从头到尾对应分布的，而是从Flash的动态空间开始
//		也就是说FnMalcTab[0]对应的扇区是Flash动态空间开始的第一个扇区，FnMalcTab分配一直到Flash空间最末
//		因为Flash空间还有一部分是静态分配了的，这部分空间不在文件分配表里表示，所以文件分配表最末必定有
//		一部分的位置是无效的，这一段无效的扇区数为STATICAREA_USED_SECT/8。这一段文件分配表将在初始化的时候
//		被置为0。
//		文件分配表的最后一位将用作文件分配表的校验位，校验值等于文件分配表前FAT_SIZE-1个长度的和取反
bool TdbMalloc(BYTE bFn, BYTE* pbFat, BYTE bOrder)
{
	int iByte;
	WORD wNeedSects;	//需要请求的扇区数
	WORD wAllocSects = 0;	//已经分配的扇区数
	int iBit;
	
	if (TdbIsLock())	//任务库是否已经上锁，删除空间的时候给TDB_ERR_LOCK
	{
		DTRACE(DB_TASKDB, ("TaskDB::TdbMalloc: TaskDB is locked\r\n"));
		return false; //上锁的时候只许删除，不许申请 
	}

	memset(pbFat, 0, FAT_SIZE);

	wNeedSects = CalcFnNeedSects(bFn); //计算出bFn所需扇区数，需要用到g_TdbCtrl
	wAllocSects = 0;
    	
    if ((bOrder&0x01) == 0) //正序分配
    {
    	//从全局分配表中搜集空间
    	for (iByte=bOrder%5; iByte<FAT_FLG_SIZE; iByte++)
    	{
    		if (g_TdbCtrl.bGlobalFat[iByte] == 0xff)
    			continue;
    
    		for (iBit=0; iBit<8; iBit++)
    		{
    			if ((iByte<<3)+iBit >= DYN_SECT_NUM)	//已经遍历完了动态分配区，该退出了
    				goto TdbMalloc_end;
    
    			if (wAllocSects == wNeedSects) //凑齐了足够的空间，记录并返回
    				goto TdbMalloc_end;	//有可能会是在最后一个扇区才刚好分配完，因此完成申请分配的判断需要放到外面做
    
    			if ((g_TdbCtrl.bGlobalFat[iByte] & (1<<iBit)) == 0) //该扇区未被使用，占用它
    			{
    				SectMarkUsed(pbFat, iByte, iBit);
    				wAllocSects++;
    			}
    		}	//for (bBit=0; bBit<8; bBit++)
    	}	//for (wByte=0; wByte<FAT_FLG_SIZE; wByte++)
    }
    else//反序分配
    {    
        //从全局分配表中搜集空间
        for (iByte=FAT_FLG_SIZE-1-bOrder%5; iByte>=0; iByte--)
        {
            if (g_TdbCtrl.bGlobalFat[iByte] == 0xff)
    			continue;
            for (iBit=7; iBit>=0; iBit--)
            {
                if ((iByte<<3)+iBit >= DYN_SECT_NUM)	//超出了动态分配区，跳过
    				continue;
                if (wAllocSects == wNeedSects) //凑齐了足够的空间，记录并返回
    				goto TdbMalloc_end;	//有可能会是在最后一个扇区才刚好分配完，因此完成申请分配的判断需要放到外面做
                if ((g_TdbCtrl.bGlobalFat[iByte] & (1<<iBit)) == 0) //该扇区未被使用，占用它
    			{
    				SectMarkUsed(pbFat, iByte, iBit);
    				wAllocSects++;
    			}
            }
        }
    }

TdbMalloc_end:
	if (wAllocSects == wNeedSects)
	{
		MakeFat(pbFat);

		// 将增加了申请信息的bFn文件分配表写入Flash
		if (!WriteFat(bFn, pbFat))
		{
			DTRACE(DB_TASKDB, ("TdbMalloc: Write FN%d FAT fail\r\n", bFn));
			return false;
		}

/*		if (!CleanFlash(bFn, pbFat))	//格式化申请到的空间
		{
			DTRACE(DB_TASKDB, ("TdbMalloc: Clean FN%d Spaces Fail\r\n", bFn));
			return false;
		}
        ------- 速度太慢，分配的时候不清空动态扇区，写数据的时候再清空*/
		for (iByte=0; iByte<FAT_FLG_SIZE; iByte++)
			g_TdbCtrl.bGlobalFat[iByte] |= pbFat[iByte];//记录全局变量
        
		DTRACE(DB_TASKDB, ("TdbMalloc: FN%d Malloc Sucess\r\n", bFn));
		return true;
	}

	DTRACE(DB_TASKDB, ("TdbMalloc: FN%d malloc fail, Because flash is full\r\n", bFn));
	return false;
}

//描述：释放bFn申请的Flash空间
//参数：@bFn 二类冻结的Fn
//返回：如果正确释放空间，返回1；否则返回相应的错误码
bool TdbFree(BYTE bFn, BYTE* pbFat)
{
	memset(pbFat, 0x00, FAT_SIZE);

	MakeFat(pbFat);

	if (!WriteFat(bFn, pbFat))
	{
		DTRACE(DB_TASKDB, ("TdbFree: Fn%d free FAT fail\r\n", bFn));
		return false;
	}
	
	DTRACE(DB_TASKDB, ("TdbFree: Fn%d free FAT ok\r\n", bFn));
	return true;
}

//描述：该函数仅做判断系统是否有为bFn申请空间，并预读相应的文件分配表
//       必须与TdbCloseTable成对出现，无论打开成功与否
//参数：@bFn 二类冻结的Fn
//返回：如果已经分配空间，返回1；否则返回相应的错误码
int TdbOpenTable (BYTE bFn)
{
	WORD wNeedSects;
	const TCommTaskCtrl* pTaskCtrl = ComTaskFnToCtrl(bFn);
    
    TdbWaitSemaphore();

	if (TdbIsLock())	//任务库是否已经上锁，删除空间的时候给TDB_ERR_LOCK
	{
		DTRACE(DB_TASKDB, ("TdbOpenTable: Tdb is locked\r\n"));
		return TDB_ERR_LOCK; //上锁的时候只许删除，不许申请
	}
	
	//if (pTaskCtrl == NULL)
		//return TDB_ERR_NOTEXIST;

	if (ReadFat(bFn, g_TdbCtrl.bFat) < 0)
	{
		g_TdbCtrl.bFn = INVALID_FN;

		DTRACE(DB_TASKDB, ("TdbOpenTable: Read FN%d malloc table fail!\r\n", bFn));
		return TDB_ERR_NOTEXIST;
	}

	wNeedSects = CalcFnNeedSects(bFn);
	if (CalcuBitNum(g_TdbCtrl.bFat, FAT_FLG_SIZE) != wNeedSects)	
	{
		g_TdbCtrl.bFn = INVALID_FN;
		DTRACE(DB_TASKDB, ("TdbOpenTable: FN%d malloc bad!\r\n", bFn));
		return TDB_ERR_NOTEXIST;
	}
	
	g_TdbCtrl.bFn = bFn;
	return TDB_ERR_OK;
}

//描述：清空预读的文件分配表
//      必须与TdbOpenTable成对出现
//参数：@bFn 二类冻结的Fn
//返回：如果正确清空了，返回1；否则返回相应的错误码
int TdbCloseTable(BYTE bFn)
{
    g_TdbCtrl.bFn = INVALID_FN;   
    TdbSignalSemaphore();
	return bFn;
}

/*
通过管道取得的记录格式，明确如下：
日冻结
时间(年月日)+测量点号(BYTE)+数据------年->月->日->PN->Data
月冻结
时间(年月)+测量点号(BYTE)+数据--------年->月->PN->Data
曲线
时间(FMT15)+测量点号(BYTE)+数据-------年->月->日->PN->Data
*/
//描述：往任务库添加一笔记录，新方案往任务库添加记录只支持一笔笔加
//参数：@bFn 二类冻结的Fn
//      @pbData需要添加到任务库的记录
//      @nLen记录长度
//返回：如果正确清空了，返回1；否则返回相应的错误码
int TdbAppendRec(BYTE bFn, BYTE *pbData, WORD nLen)
{
	BYTE bTimeLen = 3;	//读写记录带下的有效时标长度
	BYTE bPnInRec;	//作从FLASH里读出记录后比较PN号用
	//for test
	BYTE bBufTmp[132];//记录的临时缓存，128是以曲线数据计算，曲线最长数据为5个字节，加上块头的时标和测量点号，21*6+4+1=131，(6个点数据+3个字节时标+1字节测量点号+1个字节的校验总共131
	WORD wSect, wFullRecSize;
	int iPn;
	DWORD dwAddr = 0, dwOffset;//dwAddr数据的具体地址，dwOffset数据具体地址的全局偏移
	const TCommTaskCtrl	*pTaskCtrl = ComTaskFnToCtrl(bFn);

	if (TdbIsLock())
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: Tdb is locked\r\n"));
		return TDB_ERR_LOCK;
	}

	if (g_TdbCtrl.bFn != bFn)	//任务库还没打开
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d tab not opened!\r\n", bFn));
		return TDB_ERR_UNKNOW;
	}

	if (pTaskCtrl == NULL)
	{
		if (bFn >= FN_COMSTAT)
		{
			if (bFn < FN_FWDSTAT)
			{
				if (nLen != g_TdbCtrl.dwPerDataLen)
				{
					DTRACE(DB_TASKDB, ("TdbAppendRec: ComTask FN%d Record lens is not matched with g_taskCtrl.\r\n", bFn));
					return TDB_ERR_DATALEN;
				}
				//计算该笔记录相对于任务记录首地址偏移
				dwOffset = CalcComTaskOneRecOffSet(bFn-FN_COMSTAT, pbData);
			}
			else if (bFn < FN_MAX)
			{
				if (nLen > g_TdbCtrl.dwPerDataLen)
				{
					DTRACE(DB_TASKDB, ("TdbAppendRec: FwdTask FN%d Record lens is not matched with g_taskCtrl.\r\n", bFn));
					return TDB_ERR_DATALEN;
				}
				//计算该笔记录相对于任务记录首地址偏移
				dwOffset = CalcFwdTaskOneRecOffSet(bFn-FN_COMSTAT, pbData);
			}
			else
				return TDB_ERR_UNKNOW;

			wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//查找文件分配表里，动态分配空间的偏移，定位到要操作的扇区
			if (wSect == 0xffff)
			{
				DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
				return TDB_ERR_UNKNOW;
			}
		
			//好，万事具备，上东风，先来扇区首地址
			dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;
		
			//上具体地址了，开始修正由扇区标志和校验引起的误差
			dwOffset %= (EXSECT_SIZE-2); //原来dwOffset就是从扇区首开始算的，所以只需直接模余就可以了

			if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, pbData, nLen) < 0)
				return TDB_ERR_FLS_RD_FAIL;
			if (bFn < FN_FWDSTAT)
			{
				DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: ComTask FN%03d, %02x-%02x-%02x %02x:%02x append record succes!\r\n", bFn-FN_COMSTAT, pbData[4], pbData[3], pbData[2], pbData[1], pbData[0]));
			}
			else
			{
				DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FwdTask FN%03d, %02x-%02x-%02x %02x:%02x append record succes!\r\n", bFn-FN_FWDSTAT, pbData[4], pbData[3], pbData[2], pbData[1], pbData[0]));
			}
			return 1;
		}
		else
			return TDB_ERR_UNKNOW;
	}

	bTimeLen = ComTaskGetRecOffset(pTaskCtrl)-1; //从g_TdbCtrl确认时间长度
	//根据Pn计算该段记录存在Fn块的偏移量
	bPnInRec = *(pbData+4);
	//长度检查
	if (nLen != ComTaskGetRecSize(pTaskCtrl))
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d Record lens is not matched with g_taskCtrl.\r\n", bFn));
		return TDB_ERR_DATALEN;
	}

	//根据Pn计算该段记录存在Fn块的偏移量
	//bPnInRec = *(pbData+bTimeLen);
#if MTRPNMAP!=PNUNMAP		
	iPn = SearchPnMap(MTRPNMAP, bPnInRec);
#else
	if (bPnInRec > 0)
		iPn = bPnInRec - 1;
	else
		iPn = 0;
#endif

	if (iPn<0 || iPn>PN_VALID_NUM)
	{
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%d record's PN%d can not Search in PnMap.\r\n", bFn, bPnInRec));
		return TDB_ERR_PNFAIL;
	}

	if (pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_P0)
		iPn = 0; //交采或终端数据只分配一份

	dwOffset = CalcSpacesPerPn(pTaskCtrl);	//单个Pn占用的空间
	dwOffset *= iPn;

	//计算每个Pn点内月数据偏移，年是固定的，无论哪类数据，月数据都是第2位
	//此次计算过后，dwOffset已经指向所存放的月数据首
	dwOffset += CalcPerPnMonRecSize(pTaskCtrl)*((GetMonth(pbData, pTaskCtrl)-1)%ComTaskGetMonthNum(pTaskCtrl));

	//再细化到Pn块内的具体数据点的偏移，有2种情况
	//1、冻结是以日为统一时标存储的，包括曲线和日冻结
	//   曲线由于一天的数据共用一个时标，所以也以日为单个数据点处理
	if (pTaskCtrl->bIntervU<TIME_UNIT_MONTH || pTaskCtrl->bIntervU== TIME_UNIT_DAYFLG)	//月冻结以下均视为曲线和日冻结数据
		dwOffset += CalcPerPnDayRecSize(pTaskCtrl)*(GetDay(pbData, pTaskCtrl)-1);//定位到日了

	wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//查找文件分配表里，动态分配空间的偏移，定位到要操作的扇区
	if (wSect == 0xffff)
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
		return TDB_ERR_UNKNOW;
	}

	//好，万事具备，上东风，先来扇区首地址
	dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;	
	
	//这里要开始分了，曲线特殊处理，先处理非曲线，简单些，不需要比较时标与测量点号，直接写
	if (pTaskCtrl->bIntervU == TIME_UNIT_DAY)
	{//对于非曲线数据，此时已经是具体数据所在的位置，可以直接添加数据了
		//先拷贝备份
        
        	//上具体地址了，开始修正由扇区标志和校验引起的误差
    	dwOffset %= (EXSECT_SIZE-2); //原来dwOffset就是从扇区首开始算的，所以只需直接模余就可以了

		wFullRecSize = nLen + 1;	//完整的一笔记录的长度（包括校验和）
        if (wFullRecSize >= sizeof(bBufTmp))
            return TDB_ERR_UNKNOW;
		memcpy(bBufTmp, pbData, 3);  //时标
		memcpy(bBufTmp+3, pbData+4, nLen-3); //费率数，测量点和数据
		MakeData(bBufTmp, wFullRecSize); //添加校验

		if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wFullRecSize) < 0)
			return TDB_ERR_FLS_WR_FAIL;
	}
	//月数据
	else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
	{
       	//上具体地址了，开始修正由扇区标志和校验引起的误差
    	dwOffset %= (EXSECT_SIZE-2); //原来dwOffset就是从扇区首开始算的，所以只需直接模余就可以了
		wFullRecSize = nLen + 1;	//完整的一笔记录的长度（包括校验和）
		if (wFullRecSize >= sizeof(bBufTmp))
			return TDB_ERR_UNKNOW;
		memcpy(bBufTmp, pbData, 2);  //时标
		memcpy(bBufTmp+2, pbData+4, nLen-2); //测量点和数据
		MakeData(bBufTmp, wFullRecSize); //添加校验

		if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wFullRecSize) < 0)
			return TDB_ERR_FLS_WR_FAIL;
	}
	else//好了，轮到曲线
	{//对于曲线数据，此时还只在日数据块首
		//第一步，把一天的数据都读回来
		WORD wLenPerRec = CalcOneCurveRecSize(pTaskCtrl);	//曲线一笔记录(6个小时)的数据长度，不包括校验 xxxxx：+ 1
		WORD bTmpOffset = 0;//具体数据点相对日数据块的偏移
		BYTE bIdx=0, bOffset=0;

        if (wLenPerRec >= sizeof(bBufTmp))
            return TDB_ERR_UNKNOW;

		if (BcdToByte(*(pbData+3)) >= 24)	//判断小时合法性
			return TDB_ERR_UNKNOW;
		else
			bIdx = BcdToByte(*(pbData+3)) / (24/CURVE_PER_DAY);	//曲线6个小时一笔记录,这里计算当前小时是一天中的第几笔

		dwOffset += bIdx*wLenPerRec;	//定位到第几笔记录

        //上具体地址了，开始修正由扇区标志和校验引起的误差
    	dwOffset %= (EXSECT_SIZE-2); //原来dwOffset就是从扇区首开始算的，所以只需直接模余就可以了        
		if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wLenPerRec) < 0)
			return TDB_ERR_FLS_RD_FAIL;

		if (!CheckData(bBufTmp, wLenPerRec))	//xxxxx：应该新造一份记录
        {
			//return TDB_ERR_DATA_CHKSUM;
            bBufTmp[5] = INVALID_DATA;     //不能直接返回，重新做一份数据
        }

		//好了，读出来就该比较时标了，测量点也需要比较,时标只检测年-月-日，其中一个不匹配都不成功
		if ((memcmp(&bBufTmp[0], pbData, 3) != 0) || (bPnInRec != bBufTmp[3]))
		{
			//有一个不一样，全部数据删除
			memset(bBufTmp, INVALID_DATA, sizeof(bBufTmp));

			// 写时标以及测量点号
			memcpy(&bBufTmp[0], pbData, 3);	//3个长度的时标

			bBufTmp[3] = *(pbData+4);	//1个测量点
		}

		bOffset = BcdToByte(*(pbData+3)) % (24/CURVE_PER_DAY);
		//指向当前记录时标的具体位置，并改数据
		bTmpOffset = ComTaskGetRecOffset(pTaskCtrl) + bOffset*ComTaskGetDataLen(pTaskCtrl);
        if ((bTmpOffset+nLen-4) >= sizeof(bBufTmp))
            return TDB_ERR_UNKNOW;
		memcpy(&bBufTmp[bTmpOffset], pbData+5, nLen-4);

		//添加校验
		MakeData(bBufTmp, wLenPerRec);

		//写回到Flash里
		if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wLenPerRec) < 0)
			return TDB_ERR_FLS_WR_FAIL;
	}

	if (pTaskCtrl->bIntervU <= TIME_UNIT_HOUR)
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%03d, PN%03d %02x-%02x-%02x %02x:%02x append record succes!\r\n", bFn, *(pbData+bTimeLen), pbData[4], pbData[3], pbData[2], pbData[1], pbData[0]));
	else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%03d, PN%03d %02x-%02x append record succes!\r\n", bFn, *(pbData+bTimeLen), pbData[1], pbData[0]));
	else
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%03d, PN%03d %02x-%02x-%02x append record succes!\r\n", bFn, *(pbData+bTimeLen), pbData[2], pbData[1], pbData[0]));
	return 1;
}


//描述：从任务库读取一笔记录出来，也只支持一笔笔的读
//参数：@bFn 读取的二类冻结的Fn
//      @wPn 读取Pn测量点/普通任务索引号/中继任务索引号的数据
//      @tTime 读取记录的时标
//      @pbBuf 保存读回记录的Buffer
//      @bRate 读取曲线密度，仅仅适用于交采的每分钟曲线，其余时设置为0
//注意：任务库不检查pbBuf的长度，应用程序需要自己保证
int TdbReadRec(BYTE bFn, WORD wPn, TTime tTime, BYTE *pbBuf)
{
	DWORD dwAddr, dwOffset; //dwOffset具体地址的偏移量, dwAddr具体地址
	int iPn, iRet;//真实的测量点号
	const TCommTaskCtrl* pTaskCtrl = ComTaskFnToCtrl(bFn);
	WORD wSect, nDataLen, nLen = 0;//nDataLen单个记录的长度, nLen存在Flash中的记录长度（包括时标和PN号还有校验）
//	WORD wNeedSects;

	BYTE bSmplIntervU, bSmplInterV;
	BYTE bTimeLen, bBufTmp[132]; //搜索数据缓冲区
	BYTE bPnInRec;	//记录里的测量点号
	BYTE bTimeInFlash[] = {0x00, 0x00, 0x00, 0x00, 0x00};
	
	if (TdbIsLock())
	{
		iRet =  TDB_ERR_LOCK;
		goto TdbReadRec_err_ret;
	}

	if (g_TdbCtrl.bFn != bFn)	//任务库还没打开
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: FN%d tab not opened!\r\n", bFn));
		iRet = TDB_ERR_NOTEXIST;
		goto TdbReadRec_err_ret;
	}

	if (pTaskCtrl == NULL)
	{
		if (bFn >= FN_COMSTAT)
		{
			TimeToFmt15(&tTime, bBufTmp);
			Swap(bBufTmp, 5);
			//计算该笔记录相对于任务记录首地址偏移
			if (bFn < FN_FWDSTAT)
				dwOffset = CalcComTaskOneRecOffSet(bFn-FN_COMSTAT, bBufTmp);
			else if (bFn < FN_MAX)
				dwOffset = CalcFwdTaskOneRecOffSet(bFn-FN_FWDSTAT, bBufTmp);
			else
			{
				iRet = TDB_ERR_NOTEXIST;
				goto TdbReadRec_err_ret;
			}
			wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//查找文件分配表里，动态分配空间的偏移，定位到要操作的扇区
			if (wSect == 0xffff)
			{
				DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
				return TDB_ERR_UNKNOW;
			}

			//好，万事具备，上东风，先来扇区首地址
			dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;

			//上具体地址了，开始修正由扇区标志和校验引起的误差
			dwOffset %= (EXSECT_SIZE-2); //原来dwOffset就是从扇区首开始算的，所以只需直接模余就可以了

			/*if (bFn < FN_FWDSTAT)
				nDataLen = GetComTaskPerDataLen(bFn-FN_COMSTAT); //单个记录的长度,仅数据部分,也是需要返回的长度
			else //if (bFn < FN_MAX)
				nDataLen = GetFwdTaskPerDataLen(bFn-FN_FWDSTAT); //单个记录的长度,仅数据部分,也是需要返回的长度*/

			nDataLen = g_TdbCtrl.dwPerDataLen - 5;
			nLen = nDataLen + 5;//记录的长度,   再+5-----记录时间

			//先把数据记录时间读回来
			if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bTimeInFlash, 5) < 0)
			{
				iRet = TDB_ERR_FLS_RD_FAIL;
				goto TdbReadRec_err_ret;
			}

			if (g_TdbCtrl.bSmplIntervU == TIME_UNIT_MINUTE)
			{
				TTime tmRec;
				DWORD dwRecMins, dwRdMins, dwPastMin;
				Swap(bTimeInFlash, 5);
				Fmt15ToTime(bTimeInFlash, &tmRec);
				dwRecMins = MinutesFrom2000(&tmRec);
				dwRdMins = MinutesFrom2000(&tTime);
				if (dwRecMins < dwRdMins)
					dwPastMin = dwRdMins - dwRecMins;
				else
					dwPastMin = dwRecMins - dwRdMins;
				if (dwPastMin > g_TdbCtrl.bSmplIntervV)
				{
					DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: time is not match!\r\n"));
					iRet = TDB_ERR_SCH_FAIL;
					goto TdbReadRec_err_ret;
				}
			}
			else if (!IsSchTimeMatch(bTimeInFlash, tTime, g_TdbCtrl.bSmplIntervU)) //时间比较不对
			{
				DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: time is not match!\r\n"));
				iRet = TDB_ERR_SCH_FAIL;
				goto TdbReadRec_err_ret;
			}

			//计算该笔记录相对于任务记录首地址偏移，除去记录时间
			if (bFn < FN_FWDSTAT)
				dwOffset = CalcComTaskOneRecOffSet(bFn-FN_COMSTAT, bBufTmp) + 5;
			else if (bFn < FN_MAX)
				dwOffset = CalcFwdTaskOneRecOffSet(bFn-FN_FWDSTAT, bBufTmp) + 5;
			else
			{
				iRet = TDB_ERR_NOTEXIST;
				goto TdbReadRec_err_ret;
			}
			wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//查找文件分配表里，动态分配空间的偏移，定位到要操作的扇区
			if (wSect == 0xffff)
			{
				DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
				return TDB_ERR_UNKNOW;
			}

			//好，万事具备，上东风，先来扇区首地址
			dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;

			//上具体地址了，开始修正由扇区标志和校验引起的误差
			dwOffset %= (EXSECT_SIZE-2); //原来dwOffset就是从扇区首开始算的，所以只需直接模余就可以了

			//把数据读回来---不带记录时间
			if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, pbBuf, nDataLen) < 0)
			{
				iRet = TDB_ERR_FLS_RD_FAIL;
				goto TdbReadRec_err_ret;
			}

			//DTRACE(DB_TASKDB, ("TdbReadRec: read record sucess!\r\n"));	//xxxxx:这个去掉,否则打印太多
			return nDataLen;
		}
		else
		{
			iRet = TDB_ERR_NOTEXIST;
			goto TdbReadRec_err_ret;
		}
	}

	nDataLen = ComTaskGetDataLen(pTaskCtrl); //单个记录的长度,仅数据部分,也是需要返回的长度
	
	//wNeedSects = CalcFnNeedSects(bFn);

	//根据Pn计算该段记录存在Fn块的偏移量
#if MTRPNMAP!=PNUNMAP		
	iPn = SearchPnMap(MTRPNMAP,bPn);
#else
	if (wPn > 0)
		iPn = wPn - 1;
	else
		iPn = 0;
#endif

	if (iPn<0 || iPn>PN_VALID_NUM)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: the require PN%d can not Search in PnMap.\r\n", wPn));
		iRet = TDB_ERR_PNFAIL;
		goto TdbReadRec_err_ret;
	}

	bTimeLen = ComTaskGetRecOffset(pTaskCtrl)-1;	//时间长度
	nLen = nDataLen + bTimeLen + 1 + 1;//记录的长度,1----PN号    再+1-----校验   ----nLen现在专用于非曲线冻结
	
	if (pTaskCtrl->bIntervU < TIME_UNIT_DAY)	//小于日冻结就判定为曲线冻结，为兼容非江苏版
	{
		nLen = nDataLen;	//曲线数据的时标和Pn号是每一天的数据共用一个，因此单笔记录只保存数据，校验不在这
	}

	if (pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_AC || pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_P0)
		iPn = 0; //交采或终端数据只分配一份

	dwOffset = CalcSpacesPerPn(pTaskCtrl);	//单个Pn占用的空间
	dwOffset *= iPn;

	//计算每个Pn点内月数据偏移，年是固定的，无论哪类数据，月数据都是第2位
	//此次计算过后，dwOffset已经指向所存放的月数据首
	dwOffset += CalcPerPnMonRecSize(pTaskCtrl)*((tTime.nMonth-1)%ComTaskGetMonthNum(pTaskCtrl));

	//再细化到Pn块内的具体数据点的偏移，有2种情况
	//1、冻结是以日为统一时标存储的，包括曲线和日冻结
	//   曲线由于一天的数据共用一个时标，所以也以日为单个数据点处理
	if (pTaskCtrl->bIntervU<TIME_UNIT_MONTH)	//月冻结以下均视为曲线和日冻结数据
		dwOffset += CalcPerPnDayRecSize(pTaskCtrl)*(tTime.nDay-1);//定位到日了
	
	//OK，准备工作差不多了，下一步，实际地址的干活，先读文件分配表
	wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//查找文件分配表里，动态分配空间的偏移，定位到要操作的扇区
	if (wSect == 0xffff)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: FN%d's malloc table is invalid!\r\n",bFn));
		iRet = TDB_ERR_UNKNOW;
		goto TdbReadRec_err_ret;
	}

	//好，万事具备，上东风，先来扇区首地址
	dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;
	//现在要开始读数据了，根据曲线和非曲线区分，因为曲线需要特殊处理，先拿非曲线开刀
	if (pTaskCtrl->bIntervU >= TIME_UNIT_DAY)
	{//非曲线数据此时已经定位到搜索带下的时标对应的位置上了

        //对于日冻结、月冻结和抄表日冻结，此时已经是要保存的记录位置了，微调补充扇区标志和校验即可
	    dwOffset %= (EXSECT_SIZE-2); //既然扇区地址已经定了，偏移就从扇区首开始行了
        if (nLen >= sizeof(bBufTmp))
        {
            iRet = TDB_ERR_UNKNOW;
            goto TdbReadRec_err_ret;
        }
		//先把数据读回来---带着校验的
		if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, nLen) < 0)
		{
			iRet = TDB_ERR_FLS_RD_FAIL;
			goto TdbReadRec_err_ret;
		}

		if (!CheckData(bBufTmp, nLen))
		{
			iRet = TDB_ERR_DATA_CHKSUM;
			goto TdbReadRec_err_ret;
		}

		bPnInRec = bBufTmp[bTimeLen];
		memcpy(bTimeInFlash, bBufTmp, bTimeLen);
		if (!IsSchTimeMatch(bTimeInFlash, tTime, pTaskCtrl->bIntervU) || (bPnInRec!=wPn))
		{	//时标不一样或者测量点不一致，直接回无效
			DTRACE(DB_TASKDB, ("TdbReadRec: time not match!\r\n"));
			iRet = TDB_ERR_SCH_FAIL;
			goto TdbReadRec_err_ret;
		}
		else //返回，需要把时标和PN号扔掉
			memcpy(pbBuf, &bBufTmp[nLen-nDataLen-1], nDataLen);	//-1减的是校验

		//DTRACE(DB_TASKDB, ("TdbReadRec: read record sucess!\r\n"));	//xxxxx:这个去掉,否则打印太多
		return nDataLen;
	}
	else //if (pTaskCtrl->bIntervU == TIME_UNIT_HOUR)//好了，处理曲线
	{//对于曲线数据，此时还只在日数据块首
		//第一步，把一天的数据都读回来
 		WORD wLenPerRec = CalcOneCurveRecSize(pTaskCtrl);	//一天的数据长度，包括校验 xxxxx：+ 1
 		WORD bTmpOffset = 0;//具体数据点相对日数据块的偏移
		BYTE bIdx = 0, bOffset=0;
 		if (wLenPerRec >= sizeof(bBufTmp))
         {
             iRet = TDB_ERR_UNKNOW;
             goto TdbReadRec_err_ret;
        }

		if (tTime.nHour >= 24)	//判断小时合法性
			return TDB_ERR_UNKNOW;
		else
			bIdx = tTime.nHour / (24/CURVE_PER_DAY);	//曲线6个小时一笔记录,这里计算当前小时是一天中的第几笔

		dwOffset += bIdx*wLenPerRec;	//定位到第几笔记录

	    //曲线数据还停在日块数据上，因为还要判断时标，暂不计算具体记录的偏移
	    dwOffset %= (EXSECT_SIZE-2); //既然扇区地址已经定了，偏移就从扇区首开始行了
		if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wLenPerRec) < 0)
		{
			iRet = TDB_ERR_FLS_RD_FAIL;
			goto TdbReadRec_err_ret;
		}

		if (!CheckData(bBufTmp, wLenPerRec))
		{
			iRet = TDB_ERR_DATA_CHKSUM;
			goto TdbReadRec_err_ret;
		}

		bPnInRec = bBufTmp[bTimeLen];
		memcpy(bTimeInFlash, bBufTmp, 3);	//曲线存的是3个长度的时标

		//好了，读出来就该比较时标了，测量点也需要比较,时标只检测年-月-日，其中一个不匹配都不成功
		if (!IsSchTimeMatch(bTimeInFlash, tTime, pTaskCtrl->bIntervU) || (bPnInRec!=wPn)) 
		{//时标不一样，直接回无效
			DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: time is not match!\r\n"));
			iRet = TDB_ERR_SCH_FAIL;
			goto TdbReadRec_err_ret;
		}
		else//返回，需要把时标和PN号扔掉
		{
			bOffset = tTime.nHour % (24/CURVE_PER_DAY);		//指向当前记录时标的具体位置，并改数据
			bTmpOffset = ComTaskGetRecOffset(pTaskCtrl) + bOffset*nDataLen;//找到数据偏移
			//bTmpOffset = ComTaskGetRecOffset(pTaskCtrl) + bOffset*ComTaskGetDataLen(pTaskCtrl);

			DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: read record sucess!\r\n"));
			memcpy(pbBuf, &bBufTmp[bTmpOffset], nDataLen);
			return nDataLen;
		}
	}

	//DTRACE(DB_TASKDB, ("TdbReadRec: can not search record with these condition!\r\n"));
	iRet = TDB_ERR_SCH_FAIL;

TdbReadRec_err_ret:
	memset(pbBuf, INVALID_DATA, nDataLen);
	return iRet;
}

