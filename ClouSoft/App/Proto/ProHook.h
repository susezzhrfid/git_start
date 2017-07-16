/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProHook.h
 * 摘    要：本文件主要用来定义通信接口库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *			 $在这里不要定义成inline,方便和库文件一起编译时重定位
 *********************************************************************************************************/
#ifndef PROHOOK_H
#define PROHOOK_H
#include "Modem.h"

//流量统计用到的接口函数
void AddFlux(DWORD dwLen);	 	//累计流量的接口函数
bool IsFluxOver();	//流量是否超过月定值

bool GprsIsInPeriod();	//GPRS是否处于在线时段
void GprsOnFluxOver();	//回调函数,用于生成告警记录等用途
bool GprsIsTxComplete(DWORD dwStartClick); //告警和主动上送数据是否都送完
void UpdModemInfo(TModemInfo* pModemInfo);

#endif //PROHOOK_H