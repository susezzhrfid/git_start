/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�StatMgr.h
 * ժ    Ҫ�����ļ���Ҫʵ���ն�ͳ����Ϣ������ͳ����Ĺ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��������ƽ
 * ������ڣ�2008��7��
 *********************************************************************************************************/
#ifndef STATMGR_H
#define STATMGR_H
#include "DpStat.h"
//#include "Sys_arch.h"
#include "ComAPI.h"
//extern DWORD g_dwComTaskRecTm[DYN_PN_NUM][64];				//��ͨ�����м�����
typedef struct{
	TTime tmLastRun;		//���һ�����е�ʱ��,���������ϵ��ʱ���ж������Ƿ����л�
	//WORD wDayRstStart;		//�ն˵��ո�λ������ʼֵ,���л���ʱ�������㵱�ո�λ����
	//WORD wMonRstStart;		//�ն˵��¸�λ������ʼֵ,���л���ʱ�������㵱�¸�λ����
	//WORD wDayPowerTime;		//�ն˵��չ���ʱ��,ÿ����ִ�е�ʱ���1
	//WORD wMonPowerTime;		//�ն˵��¹���ʱ��,ÿ����ִ�е�ʱ���1
	DWORD dwDayFlux;		//�ն�GPRS������
	DWORD dwMonFlux;		//�ն�GPRS������
}TTermStat;				//�ն�ͳ����Ϣ,ÿ����д�뵽�ļ�ϵͳ��

void DoTermStat(TTime* tmNow, TTermStat* pTermStat);
bool DoMgrDataStat(TTime* tmNow);
#endif
