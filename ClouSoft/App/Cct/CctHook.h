/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctHook.h
 * 摘    要: 本文件主要实现集抄挂钩接口
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2009年9月
 *********************************************************************************************************/
#ifndef CCTHOOK_H
#define CCTHOOK_H

//描述:	取得载波节点的地址,对于通过采集终端的电表,则取采集终端的地址
//参数:	@wPn 测量点号
//		@pbAddr 用来返回地址
//返回:	如果成功则返回true,否则返回false
bool GetPlcNodeAddr(WORD wPn, BYTE* pbAddr);

//描述:	获取电表地址,
//参数:	@wPn 测量点号
//		@pbAddr 用来返回地址
//返回:	如果成功则返回true,否则返回false
//备注:	对于载波表 载波物理地址与电表地址一致,
//		对于采集器模型这里取目的电表地址.
bool GetCctMeterAddr(WORD wPn, BYTE* pbAddr);

void GetPlcPnMask(BYTE* pbPnMask);
const BYTE* GetCctPnMask();
WORD PlcAddrToPn(const BYTE* pb485Addr, const BYTE* pbAcqAddr);

bool IsInReadPeriod();

bool GetCurRdPeriod(WORD* pwMinStart, WORD* pwMinEnd);

#endif //CCTHOOK_H
