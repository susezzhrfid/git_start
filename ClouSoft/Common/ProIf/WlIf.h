/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�WlIf.h
 * ժ    Ҫ�����ļ�ʵ��������ͨ�Žӿ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef WLIF_H
#define WLIF_H

#include "ProIf.h"

typedef struct{
	//����	
	WORD 	wPort; 		//�˿ں�
	DWORD 	dwBaudRate; 
	BYTE 	bByteSize; 
	BYTE 	bStopBits; 
	BYTE 	bParity;
	//����
	
}TWLIf;	//Socket�ӿ�����


//���ⲿ���õĽӿں���
bool WlIfInit(struct TProIf* pProIf);

#endif  //WLIF_H
