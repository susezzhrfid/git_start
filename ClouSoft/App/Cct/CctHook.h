/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctHook.h
 * ժ    Ҫ: ���ļ���Ҫʵ�ּ����ҹ��ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2009��9��
 *********************************************************************************************************/
#ifndef CCTHOOK_H
#define CCTHOOK_H

//����:	ȡ���ز��ڵ�ĵ�ַ,����ͨ���ɼ��ն˵ĵ��,��ȡ�ɼ��ն˵ĵ�ַ
//����:	@wPn �������
//		@pbAddr �������ص�ַ
//����:	����ɹ��򷵻�true,���򷵻�false
bool GetPlcNodeAddr(WORD wPn, BYTE* pbAddr);

//����:	��ȡ����ַ,
//����:	@wPn �������
//		@pbAddr �������ص�ַ
//����:	����ɹ��򷵻�true,���򷵻�false
//��ע:	�����ز��� �ز������ַ�����ַһ��,
//		���ڲɼ���ģ������ȡĿ�ĵ���ַ.
bool GetCctMeterAddr(WORD wPn, BYTE* pbAddr);

void GetPlcPnMask(BYTE* pbPnMask);
const BYTE* GetCctPnMask();
WORD PlcAddrToPn(const BYTE* pb485Addr, const BYTE* pbAcqAddr);

bool IsInReadPeriod();

bool GetCurRdPeriod(WORD* pwMinStart, WORD* pwMinEnd);

#endif //CCTHOOK_H
