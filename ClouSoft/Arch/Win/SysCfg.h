/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Syscfg.h
 * 摘    要：平台相关的一些定义
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#ifndef SYSCFG_H
#define SYSCFG_H
#include "Typedef.h"

#define SYS_WIN

#define USER_PATH  "d:\\fafiles\\"
#define USER_PARA_PATH  "d:\\fafiles\\para\\"
#define USER_DATA_PATH  "d:\\fafiles\\data\\"
#define USER_CFG_PATH   USER_PARA_PATH
#define USER_TASKDATA_PATH	USER_DATA_PATH

//#define COMM_DEBUG  0   //调试口

#define __no_init


#define COMM_GPRS_FRMSIZE	156
#define COMM_WIREL_FRMSIZE	156
#define COMM_LOCAL_FRMSIZE	156

#define COMM_ETHERNET_FRMSIZE  156

#define GPRS_TX_SIZE      256 
#define LOCAL_TX_SIZE      64  
#define WL_TX_SIZE         156

#define ETHERNET_TX_SIZE   156

#define BYTE_WAKEUP       0xfe 

#endif

