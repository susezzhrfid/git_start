/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrCfg.h
 * ժ    Ҫ�������������ļ�,��Ҫ��������ϵͳ�����������ƽṹ
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��1��
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

