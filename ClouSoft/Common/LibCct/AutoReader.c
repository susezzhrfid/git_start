/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AutoReader.h
 * 摘    要：本文件主要实现自动抄表基类的定义,
 * 			 作为CPlcRouter,CEsReader,CRS485Reader等自身执行主动抄表类的基类
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2014年08月
 *
 * 备    注：$本文件为标准库的一部分,请不要将协议相关的部分加入到本文件
 *
 *			$自动抄表类跟其它抄表类的区别是,其它抄表类一般都是根据应用提交
 *			 的需求进行抄表的,而自动抄表类则执行相对固定的操作,按照固定的
 *			 抄表规则抄收固定的数据项,把电表数据项往数据库搬
************************************************************************************************************/

#include "AutoReader.h"
#include "MtrCtrl.h"
#include "StdReader.h"
#include "Trace.h"
#include "SysDebug.h"
#include "FaCfg.h"

TStdReader* g_ptStdReader = NULL;
bool g_fCctInitOk = false;

//描述:初始化
bool CctAutoReaderInit(TAutoReader* ptAutoReader)
{
	ptAutoReader->m_bState = AR_S_LEARN_ALL;

	ptAutoReader->m_fDirOp = false;	//外界的直接操作
	ptAutoReader->m_fStopRdMtr = false;	//停止抄表状态
	ptAutoReader->m_fInReadPeriod = true;	//是否在抄表时段内
	ptAutoReader->m_fSchMtr = false;	//不在搜索状态
	ptAutoReader->m_fCmdSch = false;

	ptAutoReader->m_dwLastDirOpClick = 0;
	ptAutoReader->m_wMainSleep = 200;
	return true;
}

//描述:载波路由线程
void CctAutoReaderRunThread()
{
	int iRes;

	TStdPara m_tStdPara;
	TAutoReader m_tAutoReader;
	TStdReader m_tStdReader;
	BYTE bLoopBuf[256];
	BYTE bStudyFlag[POINT_NUM/8+1] = {0,};
	DTRACE(DB_CCT, ("CctAutoReader::RunThread started\n"));

	LoadStdPara(AR_LNK_STD_TC, &m_tStdPara);
	m_tStdReader.m_ptStdPara = &m_tStdPara;
	CctAutoReaderInit(&m_tAutoReader);

	m_tStdReader.m_ptAutoReader = &m_tAutoReader;

	m_tStdReader.m_pbStudyFlag = bStudyFlag;
	m_tStdReader.m_tLoopBuf.m_pbLoopBuf = bLoopBuf;
	m_tStdReader.m_tLoopBuf.m_wBufSize = sizeof(bLoopBuf);
	if(!InitStdReader(&m_tStdReader))
		return;

	g_ptStdReader = &m_tStdReader;
	
	while (1)
	{
		LockReader();
		
		iRes = UpdateReader(&m_tStdReader);	//进行路由器更新
		
		switch (m_tAutoReader.m_bState)
		{
		case AR_S_LEARN_ALL:
			iRes = DoLearnAll(&m_tStdReader);
			break;
		case AR_S_AUTO_READ:
			//iRes = DoStudyMd(&m_tStdReader);
			break;
		//case AR_S_IDLE:
			//iRes = DoIdle();
			//break;
		case AR_S_EXIT:
			UnLockReader();
			return ;

		default:
			m_tAutoReader.m_bState = AR_S_LEARN_ALL;
			break;
		}

		UnLockReader(); //只对对东软这类的被动抄表器有效,对于自动路由算法及485这样的主动抄表器无效

		Sleep(m_tAutoReader.m_wMainSleep);	//485抄表主睡眠2s,载波主睡眠200ms
	}
}