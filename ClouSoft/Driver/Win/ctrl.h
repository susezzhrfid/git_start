/*********************************************************************************************************
 * Copyright (c) 2003,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：pppif.cpp
 * 摘    要：本文件主要包含了PPP接口的数据定义和函数实现，遵循lwIP接口层摸板的样式编写而成
 *
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2003年4月25日
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 **********************************************************************************************************/
#ifndef __LWIP_CTRL_H__
#define __LWIP_CTRL_H__
#include "TypeDef.h"

bool ETLinkUp(void);
void SetNetIfDefaultET(void);
void SetNetIfDefaultPPP(void);

#endif //__LWIP_CTRL_H__