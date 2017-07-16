/*********************************************************************************************************
 * Copyright (c) 2014,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DoTaskTask.h
 * 摘    要：本文件主要实现主动上送任务
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2014年9月
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
