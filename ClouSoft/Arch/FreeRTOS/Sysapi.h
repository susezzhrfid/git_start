/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Sysapi.h
 * 摘    要：系统相关的一些API
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#ifndef SYSAPI_H
#define SYSAPI_H
#include "Typedef.h"
#include "ComStruct.h"

bool GetSysTime(TTime* pTime);

bool SetSysTime(const TTime* pTime);

void SyncTimer();

//描述：系统初始化
//参数：无
//返回：无
void SysInit();

//描述：系统结束
//参数：无
//返回：无
void SysExit();

#endif
