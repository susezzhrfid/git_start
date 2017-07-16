/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProIfConst.h
 * 摘    要：本文件主要用来定义通信接口的常量
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROIFCONST_H
#define PROIFCONST_H

//错误状态
#define GPRS_ERR_UNK   -1	//还没有初始化
#define GPRS_ERR_OK		0	//没有错误
#define GPRS_ERR_RST	1	//复位模块失败
#define GPRS_ERR_REG	2	//注册网络失败
#define GPRS_ERR_PPP	3	//拨号失败
#define GPRS_ERR_CON	4	//连接主站失败

#endif //PROIFCONST_H
