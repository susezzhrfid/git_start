/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AutoReader.h
 * 摘    要：本文件主要实现自动抄表基类的定义,
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 *
 * 备    注：$本文件为标准库的一部分,请不要将协议相关的部分加入到本文件
 *
 *			$自动抄表类跟其它抄表类的区别是,其它抄表类一般都是根据应用提交
 *			 的需求进行抄表的,而自动抄表类则执行相对固定的操作,按照固定的
 *			 抄表规则抄收固定的数据项,把电表数据项往数据库搬
************************************************************************************************************/
#ifndef AUTOREADER_H
#define AUTOREADER_H

#include "Typedef.h"


//自动抄表器的工作状态
#define AR_S_AUTO_READ		0	//自动抄表模式
#define AR_S_IDLE			1	//空闲模式
#define AR_S_LEARN_ALL		2	//全面学习
#define AR_S_EXIT           3   //载波模块被拔出，需退出当前工作

typedef struct{
	BYTE bLogicPort;	//逻辑端口号,指的是通信协议上给每个通道规定的端口号,而不是物理端口号
	BYTE bPhyPort;		//物理端口号
	
	//抄读电表冻结的时标不正确,终端延迟一段时间再去抄读
	BYTE bDayFrzDelay;		//日冻结抄读延迟时间,单位分钟,主用用来避免终端时间比电表快

	bool fUseLoopBuf;		//是否使用循环缓冲区
	BYTE bMainAddr[6];		//主节点地址
}TAutoRdrPara;			//自动抄表器参数

typedef struct
{
	BYTE m_bState;
	
	bool m_fInReadPeriod;   //是否在抄表时段内
	bool m_fDirOp;          //外界的直接操作
	bool m_fStopRdMtr;      //停止抄表状态
	bool m_fSchMtr;         //是否在搜索电表
	bool m_fCmdSch;         //是否在命令搜表
	
	DWORD m_dwLastDirOpClick;   //上一次操作路由器时标，如点抄、停止、启动路由等操作
	
	WORD m_wMainSleep;
}TAutoReader;

bool CctAutoReaderInit(TAutoReader* tAutoReader);
void CctAutoReaderRunThread();

extern bool g_fCctInitOk;

#endif	//AUTOREADER_H
