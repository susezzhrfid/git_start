/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProStruct.h
 * ժ    Ҫ�����ļ���Ҫ����ͨ�Žṹ�Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef PROSTRUCT_H
#define PROSTRUCT_H
#include "TypeDef.h"

#define MTRFWD_BUFSIZE		300

typedef struct{
	bool 	fUdp; 
	DWORD 	dwRemoteIP; 
	WORD 	wRemotePort;
	DWORD 	dwBakIP;
	WORD	wBakPort;
}TMasterIp;	//��վIP��ַ����

typedef struct{
	bool 	fUdp; 
	WORD 	wLocalPort;		//�󶨵ı��ض˿ں�
}TSvrPara;	//�ն��������������

#endif //PROSTRUCT_H

