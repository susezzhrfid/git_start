/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TaskDB.h
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
************************************************************************************************************/
#pragma once
#ifndef _TASKDB_H
#define _TASKDB_H

#include "TypeDef.h"
#include "SysArch.h"
#include "Comstruct.h"
#include "TaskStruct.h"
#include "FlashMgr.h"

//任务库的错误类型
#define TDB_ERR_OK			0	//无错误
#define TDB_ERR_UNKNOW		-1	//未知错误
#define TDB_ERR_EXIST		-2	//Fn已经分配过空间了
#define TDB_ERR_NOTEXIST	-3	//Fn并没有分配空间
#define TDB_ERR_FULL		-4	//Flash空间不足
#define TDB_ERR_LOCK		-5	//任务库上锁
#define TDB_ERR_DATALEN		-6	//数据长度错误
#define TDB_ERR_PNFAIL		-7	//Pn无效，没有映射
#define TDB_ERR_FLS_RD_FAIL	-8	//Flash读失败
#define TDB_ERR_FLS_WR_FAIL	-9	//Flash写失败
#define TDB_ERR_SCH_FAIL	-10	//搜索记录失败
#define TDB_ERR_SECT_CHKSUM	-11	//扇区校验失败
#define TDB_ERR_WRONGOBJ	-12	//操作对象错误，关闭错Fn空间，错事件类型错等
#define TDB_ERR_RD_PARA		-13	//读系统库参数错误
#define TDB_ERR_FN          -14 //传下的FN不在范围内
#define TDB_ERR_MACT_CHKSUM -15 //文件分配表校验失败
#define TDB_ERR_FN_INVALID	-16	//Fn无效，即Fn没有在TCommTaskCtrl g_taskCtrl[]中配置
#define TDB_ERR_DATA_CHKSUM	-17	//数据校验错误

#define INVALID_FN		0x00

typedef struct
{
	TSem	semTdb;	     	//任务库控制信号量
	bool	fIsLocked;		//任务库是否被锁定
	BYTE	bFn;			//正常使用任务库的文件号，打开时赋值，关闭时清除
	BYTE	bFat[FAT_SIZE]; 		//打开一个FN的表时，要把该FN的文件分配表先读上来
	BYTE	bGlobalFat[FAT_SIZE]; //全局的FAT
	DWORD	dwPerDataLen;	//每笔普通或中继任务长度
	BYTE	bSmplIntervU;	//采样周期单位
	BYTE	bSmplIntervV;	//采样周期
}TTdbCtrl;


extern TTdbCtrl	g_TdbCtrl;


/******************************************************************************
 *	此部分为任务库提供给应用程序的接口函数
*******************************************************************************/
//描述：任务库初始化
//参数：NONE
//返回：成功返回true
bool TdbInit();

//描述：为bFn申请Flash空间
//参数：@bFn 二类冻结的Fn，调用的时候必须保证Fn有效，并且FN在TCommTaskCtrl g_taskCtrl[]里有配置
//返回：如果正确申请到空间，返回申请空间的首地址（一个大于0的正数）
//      如果申请失败，会返回小于0的错误码
bool TdbMalloc(BYTE bFn, BYTE* pbFat, BYTE bOrder);

//描述：释放bFn申请的Flash空间
//参数：@bFn 二类冻结的Fn
//返回：如果正确释放空间，返回1；否则返回相应的错误码
bool TdbFree(BYTE bFn, BYTE* pbFat);

//描述：该函数仅做判断系统是否有为bFn申请空间，并预读相应的文件分配表
//参数：@bFn 二类冻结的Fn
//返回：如果已经分配空间，返回1；否则返回相应的错误码
int TdbOpenTable (BYTE bFn);

//描述：清空预读的文件分配表
//参数：@bFn 二类冻结的Fn
//返回：如果正确清空了，返回1；否则返回相应的错误码
int TdbCloseTable(BYTE bFn);

//描述：从任务库读取一笔记录出来，也只支持一笔笔的读
//参数：@bFn 读取的二类冻结的Fn
//      @bPn 读取Pn测量点的数据
//      @tTime 读取记录的时标
//      @pbBuf 保存读回记录的Buffer
//      @bRate 读取曲线密度，仅仅适用于交采的每分钟曲线，其余时设置为0
//注意：任务库不检查pbBuf的长度，应用程序需要自己保证
int TdbReadRec (BYTE bFn, WORD wPn, TTime tTime, BYTE *pbBuf);


//描述：往任务库添加一笔记录，新方案往任务库添加记录只支持一笔笔加
//参数：@bFn 二类冻结的Fn
//      @pbData需要添加到任务库的记录
//      @nLen记录长度
//返回：如果正确清空了，返回1；否则返回相应的错误码
int TdbAppendRec(BYTE bFn, BYTE *pbData, WORD nLen);
/******************************************************************************
 *	提供给应用程序的接口函数定义到此为止，以下为任务库内部使用
*******************************************************************************/

//描述：取出数据头里的月数据内容
//参数：@*pbData 管道里传过来的数据包
//		@*pTaskCtrl 控制结构
//返回：返回pbData里对应于pTaskCtrl格式的月数据内容
BYTE GetMonth(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl);

//描述：取出数据头里的日数据内容
//参数：@*pbData 管道里传过来的数据包
//		@*pTaskCtrl 控制结构
//返回：返回pbData里对应于pTaskCtrl格式的日数据内容
BYTE GetDay(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl);

//描述：给任务库上锁
//参数：NONE
//返回：NONE
void TdbLock();

//描述：给任务库解锁
//参数：NONE
//返回：NONE
void TdbUnLock();

//描述：任务库是否上锁
//参数：NONE
//返回：上锁返回true
bool TdbIsLock();

void TdbWaitSemaphore();
void TdbSignalSemaphore();

//--------------------------------------------------------------------------------------------------------
// 以下Calc*********的各个函数，都没有检查pTaskCtrl的合法性，需要调用者自行保证pTaskCtrl是合法的
//描述：计算单个Pn单天的bFn数据需要多少空间，单位：Byte，以TCommTaskCtrl为对象
//参数：@pTaskCtrl bFn的任务控制结构
//返回：需要使用的空间
DWORD CalcPerPnDayRecSize(const TCommTaskCtrl *pTaskCtrl);

//描述：计算单个Pn单个月的bFn数据需要多少空间，单位：Byte，以TCommTaskCtrl为对象
//参数：@pTaskCtrl bFn的任务控制结构
//返回：需要使用的空间
DWORD CalcPerPnMonRecSize(const TCommTaskCtrl *pTaskCtrl);

//描述：计算单个Pn的bFn数据需要多少空间，单位：Byte，以TCommTaskCtrl为对象
//参数：@pTaskCtrl bFn的任务控制结构
//返回：需要使用的空间总和
DWORD CalcSpacesPerPn(const TCommTaskCtrl *pTaskCtrl);

//描述：计算bFn需要多少扇区的空间，单位：section
//参数：@bFn 二类冻结的Fn
//返回：需要扇区数
WORD CalcFnNeedSects(BYTE bFn);

//描述：计算bFn需要多少空间，单位：BYTE
//参数：@bFn 二类冻结的Fn
//返回：需要扇区数
DWORD CalcFnNeedSpaces(BYTE bFn);

//描述：检查bTime和tTime是否匹配
//参数：@bTime BYTE格式的时间
//		@tTime TTime格式的时间
//		@bIntervU 冻结类型
//返回：如果匹配返回true，否则返回false
bool IsSchTimeMatch(BYTE *bTime, TTime tTime, BYTE bIntervU);

//描述：从[wByteOffset].bBitOffset开始，在pbFat文件分配表中标志nCount个扇区，
//参数：@pbFat 文件分配表
//		@wByteOffset 文件分配表字节上的偏移
//		@bBitOffset 文件分配表字节里位的偏移
//		@nCount 标志的个数
//返回：NONE
bool SectMarkUsed(BYTE* pbFat, WORD wByte, BYTE bBit);

#endif	//_TASKDB_H
