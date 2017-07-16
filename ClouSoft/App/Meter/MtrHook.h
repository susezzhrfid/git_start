/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrHook.h
 * 摘    要：本文件主要用来定义通信接口库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年4月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *			 $在这里不要定义成inline,方便和库文件一起编译时重定位
 *********************************************************************************************************/
#ifndef MTRHOOK_H
#define MTRHOOK_H
#include "TypeDef.h"
#include "Comm.h"
#include "MtrStruct.h"

int IsIdNeedToRead(WORD wPn, const TMtrRdCtrl* pRdCtrl);
bool GetItemTimeScope(WORD wPn, const TMtrRdCtrl* pRdCtrl, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime);
void OnMtrErrEstb(WORD wPn, BYTE* pbData);	//抄表故障确认(单个测量点)
void OnMtrErrRecv(WORD wPn);	//抄表故障恢复(单个测量点)
WORD MtrAddrToPn(const BYTE* pb485Addr);
void DoMtrRdStat();
void DoBrcastMtrTm();
void ClrMtrRdStat();
#endif //MTRHOOK_H