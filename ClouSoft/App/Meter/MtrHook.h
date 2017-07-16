/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrHook.h
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��4��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *			 $�����ﲻҪ�����inline,����Ϳ��ļ�һ�����ʱ�ض�λ
 *********************************************************************************************************/
#ifndef MTRHOOK_H
#define MTRHOOK_H
#include "TypeDef.h"
#include "Comm.h"
#include "MtrStruct.h"

int IsIdNeedToRead(WORD wPn, const TMtrRdCtrl* pRdCtrl);
bool GetItemTimeScope(WORD wPn, const TMtrRdCtrl* pRdCtrl, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime);
void OnMtrErrEstb(WORD wPn, BYTE* pbData);	//�������ȷ��(����������)
void OnMtrErrRecv(WORD wPn);	//������ϻָ�(����������)
WORD MtrAddrToPn(const BYTE* pb485Addr);
void DoMtrRdStat();
void DoBrcastMtrTm();
void ClrMtrRdStat();
#endif //MTRHOOK_H