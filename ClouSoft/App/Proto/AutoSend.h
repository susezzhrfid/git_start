/*********************************************************************************************************
 * Copyright (c) 2013,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AutoSend.h
 * ժ    Ҫ�����ļ���Ҫʵ��������������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2013��3��
*********************************************************************************************************/
#ifndef AUTOSEND_H
#define AUTOSEND_H
#include "GbPro.h"
#include "ComStruct.h"

//extern TRptCtrl g_ComRptCtrl[MAX_COMMON_TASK];	//��ͨ����
//extern TRptCtrl g_FwdCtrl[MAX_FWD_TASK];		//�м�����

bool IsAlrReport(DWORD dwAlrID);
bool DoRptAlarm(struct TPro* pPro);
int MakeClass3Frm(struct TPro* pPro, BYTE bEc);
bool  GbAutoSend(struct TPro* pPro);

#endif //AUTOSEND_H
