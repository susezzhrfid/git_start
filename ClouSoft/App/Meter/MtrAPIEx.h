/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterAPIEx.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ���ӿ���Ҫ���ݲ�ͬЭ���޸ĵĲ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef METERAPIEX_H
#define METERAPIEX_H
#include "MtrStruct.h"

/////////////////////////////////////////////////////////////////////////
//�����ṩ�ı�׼�ӿں���
const WORD* Bank0To645ID(WORD wID);
bool IsMtrID(WORD wID);
WORD MtrCmbTo645IdNum(WORD wID);
DWORD GetMtrIdRdDelay(WORD wPn, WORD wID);
bool IsGrpID(WORD wID);
bool IsPnID(WORD wID);

bool IsCurveId(WORD wID);		//�Ƿ����߶���ID	
bool IsDayFrzId(WORD wID);		//�Ƿ��ն���ID
/////////////////////////////////////////////////////////////////////////
//�ڲ�ʹ�õĺ���


#endif //METERAPIEX_H