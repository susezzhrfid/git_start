/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrCfg.h
 * 摘    要：集抄的配置文件,主要用来配置系统库的数据项及控制结构
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年1月
 *********************************************************************************************************/
#ifndef MTRCFG_H
#define MTRCFG_H
#include "TypeDef.h"
#include "MtrStruct.h"

int MtrGetRdCtrlNum();
const TMtrRdCtrl* MtrGetRdCtrl(WORD* pwItemNum);
void SaveLastRdMtrData(WORD wID, WORD wPn, BYTE* pbBuf);
//const TID2IDCfg* CctGetIdInTo07Cfg(WORD* pwItemNum);
//const TID2IDCfg* CctGetIdInTo97Cfg(WORD* pwItemNum);

#endif //MTRCFG_H

