/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcHook.h
 * ժ    Ҫ�����ļ���Ҫ�������彻�ɿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *********************************************************************************************************/
#ifndef ACHOOK_H
#define ACHOOK_H
#include "Typedef.h"
#include "ComStruct.h"

/////////////////////////////////////////////////////////////////////////
//���ɿ�Ĵ������Ҫ��׼�Ĺҹ�/�ص���������
void AcOnClrDemand(WORD wPn);
void AcOnDateFrz(WORD wPn, const TTime time);
bool AcIsDateFrozen(WORD wPn, const TTime time);
void AcTrigerSavePn(WORD wPn);
void AcTrigerSavePn2(WORD wPn0, WORD wPn1);

#endif //ACHOOK_H
