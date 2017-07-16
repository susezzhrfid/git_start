/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AutoReader.h
 * ժ    Ҫ�����ļ���Ҫʵ���Զ��������Ķ���,
 * 			 ��ΪCPlcRouter,CEsReader,CRS485Reader������ִ������������Ļ���
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2014��08��
 *
 * ��    ע��$���ļ�Ϊ��׼���һ����,�벻Ҫ��Э����صĲ��ּ��뵽���ļ�
 *
 *			$�Զ�������������������������,����������һ�㶼�Ǹ���Ӧ���ύ
 *			 ��������г����,���Զ���������ִ����Թ̶��Ĳ���,���չ̶���
 *			 ��������չ̶���������,�ѵ�������������ݿ��
************************************************************************************************************/

#include "AutoReader.h"
#include "MtrCtrl.h"
#include "StdReader.h"
#include "Trace.h"
#include "SysDebug.h"
#include "FaCfg.h"

TStdReader* g_ptStdReader = NULL;
bool g_fCctInitOk = false;

//����:��ʼ��
bool CctAutoReaderInit(TAutoReader* ptAutoReader)
{
	ptAutoReader->m_bState = AR_S_LEARN_ALL;

	ptAutoReader->m_fDirOp = false;	//����ֱ�Ӳ���
	ptAutoReader->m_fStopRdMtr = false;	//ֹͣ����״̬
	ptAutoReader->m_fInReadPeriod = true;	//�Ƿ��ڳ���ʱ����
	ptAutoReader->m_fSchMtr = false;	//��������״̬
	ptAutoReader->m_fCmdSch = false;

	ptAutoReader->m_dwLastDirOpClick = 0;
	ptAutoReader->m_wMainSleep = 200;
	return true;
}

//����:�ز�·���߳�
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
		
		iRes = UpdateReader(&m_tStdReader);	//����·��������
		
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

		UnLockReader(); //ֻ�ԶԶ�������ı�����������Ч,�����Զ�·���㷨��485������������������Ч

		Sleep(m_tAutoReader.m_wMainSleep);	//485������˯��2s,�ز���˯��200ms
	}
}