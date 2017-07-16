/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AutoReader.h
 * ժ    Ҫ�����ļ���Ҫʵ���Զ��������Ķ���,
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 *
 * ��    ע��$���ļ�Ϊ��׼���һ����,�벻Ҫ��Э����صĲ��ּ��뵽���ļ�
 *
 *			$�Զ�������������������������,����������һ�㶼�Ǹ���Ӧ���ύ
 *			 ��������г����,���Զ���������ִ����Թ̶��Ĳ���,���չ̶���
 *			 ��������չ̶���������,�ѵ�������������ݿ��
************************************************************************************************************/
#ifndef AUTOREADER_H
#define AUTOREADER_H

#include "Typedef.h"


//�Զ��������Ĺ���״̬
#define AR_S_AUTO_READ		0	//�Զ�����ģʽ
#define AR_S_IDLE			1	//����ģʽ
#define AR_S_LEARN_ALL		2	//ȫ��ѧϰ
#define AR_S_EXIT           3   //�ز�ģ�鱻�γ������˳���ǰ����

typedef struct{
	BYTE bLogicPort;	//�߼��˿ں�,ָ����ͨ��Э���ϸ�ÿ��ͨ���涨�Ķ˿ں�,����������˿ں�
	BYTE bPhyPort;		//����˿ں�
	
	//����������ʱ�겻��ȷ,�ն��ӳ�һ��ʱ����ȥ����
	BYTE bDayFrzDelay;		//�ն��᳭���ӳ�ʱ��,��λ����,�������������ն�ʱ��ȵ���

	bool fUseLoopBuf;		//�Ƿ�ʹ��ѭ��������
	BYTE bMainAddr[6];		//���ڵ��ַ
}TAutoRdrPara;			//�Զ�����������

typedef struct
{
	BYTE m_bState;
	
	bool m_fInReadPeriod;   //�Ƿ��ڳ���ʱ����
	bool m_fDirOp;          //����ֱ�Ӳ���
	bool m_fStopRdMtr;      //ֹͣ����״̬
	bool m_fSchMtr;         //�Ƿ����������
	bool m_fCmdSch;         //�Ƿ��������ѱ�
	
	DWORD m_dwLastDirOpClick;   //��һ�β���·����ʱ�꣬��㳭��ֹͣ������·�ɵȲ���
	
	WORD m_wMainSleep;
}TAutoReader;

bool CctAutoReaderInit(TAutoReader* tAutoReader);
void CctAutoReaderRunThread();

extern bool g_fCctInitOk;

#endif	//AUTOREADER_H
