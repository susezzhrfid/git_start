/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterFmt.h
 * ժ    Ҫ�����ļ���Ҫʵ�ָ�Э����صĳ���������ĸ�ʽת����
 �� 			 ���ݱ��桢IDת������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��2��
 *********************************************************************************************************/
#ifndef METERFMT_H
#define METERFMT_H
#include "TypeDef.h"
#include "MtrStruct.h"

/////////////////////////////////////////////////////////////////////////
//�����ṩ�Ľӿں���
//bool SaveMeterTask(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, DWORD dwTime, BYTE bErr);
int SaveMeterItem(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, DWORD dwTime, BYTE bErr);

BYTE GetFnFromCurveId(WORD wId);

#endif //METERFMT_H
