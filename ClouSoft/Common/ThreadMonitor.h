/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ThreadMonitor.h
 * 摘    要：本文件主要实现线程监控
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef THREADMONITRO_H
#define THREADMONITRO_H
#include "TypeDef.h"
#include "Sysarch.h"

#define THRD_MNTR_NUM			6	//最大监控线程数量,根据需要少配点
#define THRD_NAME_LEN			4   //32 名字取太长费内存,名字实际也没什么用

////////////////////////////////////////////////////////////////////////////////////////////
//ThreadMonitor公共函数定义
bool InitThreadMonitor();
int ReqThreadMonitorID(char* pszName, DWORD dwUdpInterv);
void ReleaseThreadMonitorID(int iID);
void UpdThreadRunClick(int iID);
int DoThreadMonitor();
bool GetMonitorThreadName(int iID, char* pszName);

#endif //THREADMONITRO_H