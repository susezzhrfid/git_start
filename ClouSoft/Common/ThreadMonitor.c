/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ThreadMonitor.cpp
 * 摘    要：本文件主要实现线程监控
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "ThreadMonitor.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////
//ThreadMonitor私有成员变量
static DWORD	m_dwClick;		//内部的秒计数
static TSem 	m_semMonitor;	//监控器的总的资源保护              
static DWORD 	m_dwRunClick[THRD_MNTR_NUM];	//运行计数
static DWORD 	m_dwUpdInterv[THRD_MNTR_NUM];	//更新间隔,单位秒,不为0表示占用
static char 	m_szThreadName[THRD_MNTR_NUM][THRD_NAME_LEN];	//线程名称

///////////////////////////////////////////////////////////////////////////////////
//ThreadMonitor实现

//描述:初始化线程监控
bool InitThreadMonitor()
{
	m_semMonitor = NewSemaphore(1, 1); //监控器的总的资源保护
	memset(&m_dwRunClick, 0, sizeof(m_dwRunClick));			//运行计数
	memset(&m_dwUpdInterv, 0, sizeof(m_dwUpdInterv));	//更新间隔,单位秒
	m_dwClick = 0;
	return true;
}


//描述:申请线程监控ID
//参数:@dwUpdInterv 线程更新间隔,单位秒
int ReqThreadMonitorID(char* pszName, DWORD dwUpdInterv)
{
	int iID = -1;
	WORD i;
	if (dwUpdInterv == 0)
		return -1;
	
	WaitSemaphore(m_semMonitor, SYS_TO_INFINITE);
	for (i=0; i<THRD_MNTR_NUM; i++)
	{
		if (m_dwUpdInterv[i] == 0)	//该ID没有被占用
		{
			m_dwRunClick[i] = m_dwClick;		//更新时标,避免中途申请引起马上复位
			m_dwUpdInterv[i] = dwUpdInterv;		//m_dwUpdInterv[i]不为0表示占用
			iID = i;
			
			memset(m_szThreadName[iID], 0, THRD_NAME_LEN);
			if (pszName != NULL)
			{
				int iLen = strlen(pszName);
				if (iLen > THRD_NAME_LEN-1)
					iLen = THRD_NAME_LEN-1;
				
				iLen++;	//包括'\0'
				memcpy(m_szThreadName[iID], pszName, iLen);
			}
		
			if (m_szThreadName[iID][0] == '\0')
				memcpy(m_szThreadName[iID], "unknow-thrd", strlen("unknow-thrd")+1);
		
			m_szThreadName[iID][THRD_NAME_LEN-1] = '\0';
			
			break;
		}	
	}
	
	SignalSemaphore(m_semMonitor);
	return iID;
}

//描述:释放线程监控ID	
void ReleaseThreadMonitorID(int iID)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
		return;
	
	WaitSemaphore(m_semMonitor, SYS_TO_INFINITE);
		
	m_dwUpdInterv[iID] = 0;
	m_dwRunClick[iID] = 0;
	
	SignalSemaphore(m_semMonitor);
}


//描述:由各个已经注册申请的线程对自己的运行时标进行更新
void UpdThreadRunClick(int iID)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
		return;
		
	if (m_dwUpdInterv[iID] != 0)	//该线程监控ID被正确分配
	{	
		m_dwRunClick[iID] = m_dwClick;
	}
}

//描述:对已经注册申请的线程进行监控,每秒执行一次
//返回:0表示正常,-(线程号+1)表示该线程没有及时更新,需要复位
int DoThreadMonitor()
{
	WORD i;
	m_dwClick++;
	for (i=0; i<THRD_MNTR_NUM; i++)
	{
		if (m_dwUpdInterv[i] != 0)	//该ID使用
		{
			if (m_dwClick-m_dwRunClick[i] > m_dwUpdInterv[i])
				return -(i+1);
		}	
	}
	
	return 0;
}

//描述:取得监控的线程名称
bool GetMonitorThreadName(int iID, char* pszName)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
	{
		memset(pszName, 0, THRD_NAME_LEN);
		return false;
	}
	else
	{		
		memcpy(pszName, m_szThreadName[iID], THRD_NAME_LEN);
		return true;
	}
}
