/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ThreadMonitor.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���̼߳��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#include "ThreadMonitor.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////
//ThreadMonitor˽�г�Ա����
static DWORD	m_dwClick;		//�ڲ��������
static TSem 	m_semMonitor;	//��������ܵ���Դ����              
static DWORD 	m_dwRunClick[THRD_MNTR_NUM];	//���м���
static DWORD 	m_dwUpdInterv[THRD_MNTR_NUM];	//���¼��,��λ��,��Ϊ0��ʾռ��
static char 	m_szThreadName[THRD_MNTR_NUM][THRD_NAME_LEN];	//�߳�����

///////////////////////////////////////////////////////////////////////////////////
//ThreadMonitorʵ��

//����:��ʼ���̼߳��
bool InitThreadMonitor()
{
	m_semMonitor = NewSemaphore(1, 1); //��������ܵ���Դ����
	memset(&m_dwRunClick, 0, sizeof(m_dwRunClick));			//���м���
	memset(&m_dwUpdInterv, 0, sizeof(m_dwUpdInterv));	//���¼��,��λ��
	m_dwClick = 0;
	return true;
}


//����:�����̼߳��ID
//����:@dwUpdInterv �̸߳��¼��,��λ��
int ReqThreadMonitorID(char* pszName, DWORD dwUpdInterv)
{
	int iID = -1;
	WORD i;
	if (dwUpdInterv == 0)
		return -1;
	
	WaitSemaphore(m_semMonitor, SYS_TO_INFINITE);
	for (i=0; i<THRD_MNTR_NUM; i++)
	{
		if (m_dwUpdInterv[i] == 0)	//��IDû�б�ռ��
		{
			m_dwRunClick[i] = m_dwClick;		//����ʱ��,������;�����������ϸ�λ
			m_dwUpdInterv[i] = dwUpdInterv;		//m_dwUpdInterv[i]��Ϊ0��ʾռ��
			iID = i;
			
			memset(m_szThreadName[iID], 0, THRD_NAME_LEN);
			if (pszName != NULL)
			{
				int iLen = strlen(pszName);
				if (iLen > THRD_NAME_LEN-1)
					iLen = THRD_NAME_LEN-1;
				
				iLen++;	//����'\0'
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

//����:�ͷ��̼߳��ID	
void ReleaseThreadMonitorID(int iID)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
		return;
	
	WaitSemaphore(m_semMonitor, SYS_TO_INFINITE);
		
	m_dwUpdInterv[iID] = 0;
	m_dwRunClick[iID] = 0;
	
	SignalSemaphore(m_semMonitor);
}


//����:�ɸ����Ѿ�ע��������̶߳��Լ�������ʱ����и���
void UpdThreadRunClick(int iID)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
		return;
		
	if (m_dwUpdInterv[iID] != 0)	//���̼߳��ID����ȷ����
	{	
		m_dwRunClick[iID] = m_dwClick;
	}
}

//����:���Ѿ�ע��������߳̽��м��,ÿ��ִ��һ��
//����:0��ʾ����,-(�̺߳�+1)��ʾ���߳�û�м�ʱ����,��Ҫ��λ
int DoThreadMonitor()
{
	WORD i;
	m_dwClick++;
	for (i=0; i<THRD_MNTR_NUM; i++)
	{
		if (m_dwUpdInterv[i] != 0)	//��IDʹ��
		{
			if (m_dwClick-m_dwRunClick[i] > m_dwUpdInterv[i])
				return -(i+1);
		}	
	}
	
	return 0;
}

//����:ȡ�ü�ص��߳�����
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
