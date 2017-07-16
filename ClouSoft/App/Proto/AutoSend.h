/*********************************************************************************************************
 * Copyright (c) 2013,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AutoSend.h
 * 摘    要：本文件主要实现主动上送任务
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2013年3月
*********************************************************************************************************/
#ifndef AUTOSEND_H
#define AUTOSEND_H
#include "GbPro.h"
#include "ComStruct.h"

//extern TRptCtrl g_ComRptCtrl[MAX_COMMON_TASK];	//普通任务
//extern TRptCtrl g_FwdCtrl[MAX_FWD_TASK];		//中继任务

bool IsAlrReport(DWORD dwAlrID);
bool DoRptAlarm(struct TPro* pPro);
int MakeClass3Frm(struct TPro* pPro, BYTE bEc);
bool  GbAutoSend(struct TPro* pPro);

#endif //AUTOSEND_H
