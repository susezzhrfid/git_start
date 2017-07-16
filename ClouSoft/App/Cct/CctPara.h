/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctPara.h
 * 摘    要：本文件主要实现对集抄参数的装载
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2009年9月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
#ifndef CCTPARA_H
#define CCTPARA_H
#include "apptypedef.h"

void LoadAutoRdrPara(BYTE bLink, TAutoRdrPara* pRdrPara);
void LoadStdPara(BYTE bLink, TStdPara* pStdPara);
#endif //CCTPARA_H
