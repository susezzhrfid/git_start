/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�PotoIf.cpp
 * ժ    Ҫ�����ļ�ʵ����ͨ�Žӿڻ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע���ӿڵ�״̬�л�: (����)->(��λ)->(����)->(��¼)->(����)
 *          ��״̬�´���������������:��״̬�Ĵ�������ɸ�״̬�Լ�����,ĳ״̬��������״̬�ļ���
 *********************************************************************************************************/
#include "ProIf.h"
//#include "FaCfg.h"
//#include "FaConst.h"
#include "Info.h"
//#include "Trace.h"
//#include "sysapi.h"

////////////////////////////////////////////////////////////////////////////////////////////
//ProIfĬ�Ͻӿں���

//����������״̬
bool ProIfDorman(struct TProIf* pProIf)
{
	return true;
}

//��������λ�ӿ�
bool ProIfReset(struct TProIf* pProIf)
{
	return true;
}

//���������ͺ���
bool ProIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen)
{
	return true;
}

//���������պ���
int ProIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize)
{
	return true;
}

void ProIfTrans(struct TProIf* pProIf)
{
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��
	pPro->pfnRcvFrm(pPro); 	//���յ���һ֡,���Ѿ�������д���
							//�����Ҫֱ��ʹ�ô��ڵ�ѭ���������Դﵽ��ʡ�ڴ��Ŀ�ģ�����pfnRcvFrm()����
}

//�������ӿڸ�λ�ɹ�ʱ�Ļص�����
void ProIfOnResetOK(struct TProIf* pProIf)
{

}

//�������ӿڸ�λʧ��ʱ�Ļص�����
void ProIfOnResetFail(struct TProIf* pProIf)	
{

}

//�������յ�����֡ʱ�Ļص�����:��Ҫ������·״̬,����������
void ProIfOnRcvFrm(struct TProIf* pProIf)
{
	pProIf->dwRxClick = GetClick();
}

//�������ӿ�������⴦����
void ProIfDoIfRelated(struct TProIf* pProIf)	
{

}

//�������ӿ����װ�طǸ�λ����
void ProIfLoadUnrstPara(struct TProIf* pProIf)
{

}

//������ͨ�Žӿڻ����ʼ��
void ProIfInit(struct TProIf* pProIf)
{
	//�ⲿ�������õĲ���
	//char* pszName;			//�ӿ�����
	//WORD wMaxFrmBytes;
	//void* pvIf;			//ͨ�Žӿ���������,����bIfType�Ĳ�ͬ��ָ��TGprsIf��TCommIf��ʵ��
	//struct TPro* pPro;	//ͨ��Э��

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pProIf->pfnDorman = ProIfDorman;			//����
	pProIf->pfnReset = ProIfReset;				//��λ�ӿ�
	pProIf->pfnSend = ProIfSend;				//���ͺ���
	pProIf->pfnReceive = ProIfReceive;			//���պ���
	pProIf->pfnTrans = ProIfTrans;				//����״̬����
	pProIf->pfnOnResetOK = ProIfOnResetOK;		//�ӿڸ�λ�ɹ�ʱ�Ļص�����
	pProIf->pfnOnResetFail = ProIfOnResetFail;	//�ӿڸ�λʧ��ʱ�Ļص�����
	pProIf->pfnOnRcvFrm = ProIfOnRcvFrm;		//�յ�����֡ʱ�Ļص�����
	pProIf->pfnDoIfRelated = ProIfDoIfRelated;	//�ӿ�������⴦����
   	pProIf->LoadUnrstPara = ProIfLoadUnrstPara;	//�ӿ����װ�طǸ�λ����

	//����
	//BYTE bIfType;
	pProIf->dwRxClick = 0;	//���һ�ν��յ����ĵ�ʱ��
	pProIf->bState = IF_STATE_RST;   	//�ӿ�״̬��:����->��λ(MODEM��λ��PPP����)->����(�ͻ���״̬��������->��½->ͨ��->�����Ͽ�->����)

	pProIf->fExit = false;			//�Ƿ�Ҫ�˳�
	pProIf->fExitDone = false;		//�˳��Ƿ����
}	


void SetMaxFrmBytes(struct TProIf* pProIf, WORD wMaxFrmBytes)
{
	pProIf->wMaxFrmBytes = wMaxFrmBytes;
}
