/********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Pro.h
 * ժ    Ҫ�����ļ�ʵ����ͨ��Э�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef PROTO_H
#define PROTO_H

#include "TypeDef.h"
#include "ProIf.h"


struct TPro{
	void* pvPro;			//ͨ��Э����������
	struct TProIf* pIf;		//ͨ�Žӿ�

    BYTE bProMode; //Э��
	//bool fLocal;
	//bool fAutoSend;			//�Ƿ�����������͵Ĺ���

	//�麯������Ҫʵ����Ϊ����Э��Ķ�Ӧ����
	int (* pfnRcvFrm)(struct TPro* pPro);		//�������ղ�����֡
	bool (* pfnRxFrm)(struct TPro* pPro, BYTE* pbRxBuf, int iLen);		//�ⲿ����������ݲ�����֡
	int (* pfnRcvBlock)(struct TPro* pPro, BYTE* pbBlock, int nLen);	//��֡
	bool (* pfnHandleFrm)(struct TPro* pPro);	//����֡

	bool (* pfnLogin)(struct TPro* pPro);			//��½
	bool (* pfnLogoff)(struct TPro* pPro);			//��½�˳�
	bool (* pfnBeat)(struct TPro* pPro);			//����
	bool (* pfnAutoSend)(struct TPro* pPro);		//��������
	bool (* pfnIsNeedAutoSend)(struct TPro* pPro);	//�Ƿ���Ҫ��������
	void (* pfnLoadUnrstPara)(struct TPro* pPro);	//װ�طǸ�λ����
	void (* pfnDoProRelated)(struct TPro* pPro);	//��һЩЭ����صķǱ�׼������
	bool (* pfnSend)(struct TPro* pPro, BYTE* pbTxBuf, WORD wLen);	//��pIf->pIfpfnSend()�Ķ��η�װ�������س����⺯��
	int (* pfnReceive)(struct TPro* pPro, BYTE* pbRxBuf, WORD wBufSize); //��pIf->pfnReceive()�Ķ��η�װ�������س����⺯����int���Է��ش���
};	//ͨ��Э�����ṹ


////////////////////////////////////////////////////////////////////////////////////////////
//Pro������������
void ProInit(struct TPro* pPro);

#endif //PROTO_H
