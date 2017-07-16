/*********************************************************************************************************
 * Copyright (c) 2014,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DoTaskTask.h
 * ժ    Ҫ�����ļ���Ҫʵ��������������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2014��9��
*********************************************************************************************************/

#ifndef DOTASK_H
#define DOTASK_H
#include "FaAPI.h"
#include "MtrAPI.h"
#include "MtrStruct.h"
#include "MtrCtrl.h"
#include "MtrFmt.h"
#include "MtrHook.h"
#include "MtrCfg.h"
#include "LibDbAPI.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "MtrProAPI.h"
#include "FaAPI.h"
#include "MtrExc.h"
#include "CommonTask.h"
#include "DbAPI.h"
#include "FlashMgr.h"
#include "ExcTask.h"
#include "MtrAPIEx.h"
#include "ProEx.h"
#include "SysApi.h"
#include  "SearchMeter.h"
#include  "DbCfg.h"
#include "DrvCfg.h"
#include  "DbGbAPI.h"
#include "DbFmt.h"
#include "BatMtrTask.h"
#include "AutoSendTask.h"
#include "DbSgAPI.h"
#include "FlashMgr.h"
#include "EsamCmd.h"
#include "TypeDef.h"

void UpdTaskInfo();
void ResetTaskInfo();
bool DoTask(BYTE bThrId);
bool DoDataTask(BYTE bTaskType, BYTE bTaskNo, BYTE bThrId);

bool GetDoTaskTimeScope(TComTaskCfg* pCfg, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime);
bool DoRdTaskData(BYTE bTaskType, BYTE bTaskNo, TComTaskCfg *pCfg, DWORD dwStartTime, DWORD dwEndTime, BYTE bThrId);

#endif
