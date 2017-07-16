/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ�����: CctIf.h
 * ժ    Ҫ: ���ļ���Ҫʵ��376.2�ز�Э��ӿ�
 * ��ǰ�汾: 1.0
 * ��    ��: ᯼���
 * �������: 2014��8��
 *********************************************************************************************************/
#ifndef CCTIF_H
#define CCTIF_H

#include "Typedef.h"
#include "Comm.h"
#include "LibCctConst.h"

typedef struct
{
	TCommPara m_tCommPara;

	bool m_fRxComlpete;
	WORD m_wRxDataLen;
	WORD m_nRxStep;
	WORD m_nRxCnt;
	WORD m_wRxPtr;
	BYTE m_bResetCnt;
}TCctCommPara;

typedef struct{
	BYTE bFwdDepth;			//�м����
	BYTE bModule;			//ͨ��ģ���ʶ��TO_MOD_RT/TO_MOD_MTR
	//0��ʾ�Լ�������ͨ��ģ�������1��ʾ���ز����ͨ��ģ�����
	BYTE bRt;				//·�ɱ�ʶ��D0=0��ʾͨ��ģ���·�ɻ�����·��ģʽ��D0=1��ʾͨ��ģ�鲻��·�ɻ�������·ģʽ��
	BYTE bCn;				//�ŵ���ʶ��ȡֵ0~15��0��ʾ�����ŵ���1~15���α�ʾ��1~15�ŵ���
	BYTE bCnChar;			//���ͨ������
	BYTE bPhase;			//ʵ�����߱�ʶ��ʵ��ӽڵ��߼����ŵ����ڵ�Դ���0Ϊ��ȷ����1~3���α�ʾ���Ϊ��1�ࡢ��2�ࡢ��3�ࡣ
	BYTE bAnsSigQlty;		//ĩ��Ӧ���ź�Ʒ��
	BYTE bCmdSigQlty;		//ĩ�������ź�Ʒ��
	BYTE bRptEvtFlg;        //�����ϱ�ʱ���ʶλ
	BYTE bSN;				//�������к�
}TUpInf;		//698��׼·��������Ϣ��,ֻ�ڱ��ļ�ʹ��

bool CctProOpenComm(TCommPara* pCommPara);
WORD DtToFn(BYTE* pbDt);
void nToDt(WORD wFn, BYTE* pbDt);
void FnToDt(WORD wFn, BYTE* pbDt);
void GetUpInf(BYTE* pbBuf, TUpInf* pUpInf);
WORD CctProMakeFrm(WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE bAfn, WORD wFn, BYTE* pbData, WORD wDataLen, BYTE bAddrLen, BYTE bMode, BYTE bPRM, BYTE bCn, BYTE bRcReserved);
DWORD CctSend(TCommPara* pCommPara, BYTE *p, DWORD dwLen);
DWORD CctReceive(TCommPara* pCommPara, BYTE* p, DWORD wLen);
int CctRcvFrame(BYTE* pbBlock, int nLen, TCctCommPara* ptCctCommPara, WORD wFrmLenBytes, BYTE* pbCctRxBuf, DWORD* pdwLastRxClick);

#endif