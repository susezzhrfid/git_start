/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CommIf.h
 * ժ    Ҫ�����ļ�ʵ���˴���ͨ�Žӿ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef COMMIF_H
#define COMMIF_H
#include "ProIf.h"
#include "Comm.h"

typedef struct{
	//����
	bool	fDebug;		//�Ƿ����������ڹ���

	WORD 	wPort; 		//�˿ں�
	DWORD 	dwBaudRate; 
	BYTE 	bByteSize; 
	BYTE 	bStopBits; 
	BYTE 	bParity;
}TCommIf;	//����ͨ�Žӿ�����ṹ


//���ⲿ���õĽӿں���
bool CommIfInit(struct TProIf* pProIf);

#endif  //COMMIF_H


