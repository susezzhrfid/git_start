/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterPro.h
 * ժ    Ҫ�����ļ���Ҫ��������Э��Ļ���API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef METERPRO_H
#define METERPRO_H

#include "ComAPI.h"
#include "Trace.h"
//#include "SysAPI.h"
#include "DbConst.h"
#include "SysDebug.h"
#include "Comm.h"
#include "MtrStruct.h"

#define MET_INVALID_VAL		0xffffffff  //��Ч����

#define MET_ZXYG	0		//�����й�
#define MET_FXYG	1		//�����й�
#define MET_ZXWG	2		//�����޹�
#define MET_FXWG	3		//�����޹�
#define MET_WGX1	4		//һ�����޹�
#define MET_WGX2	5		//�������޹�
#define MET_WGX3	6		//�������޹�
#define MET_WGX4	7		//�������޹�
#define MET_GXWG	8		//�����޹�
#define MET_RXWG	9		//�����޹�
#define MET_ZXSZ	10		//��������
#define MET_FXSZ	11		//�������� 

#define TXRX_RETRYNUM	2	//�ط�����

#define MTR_FRM_SIZE  256//270    645Э��ĳ���ֻ��һ���ֽڣ������ܳ���256

#define COM_TIMEOUT  1000	//����ȱʡ��ʱ

////////////////////////////////
//������
struct TMtrPro
{
	TMtrPara* pMtrPara;	//������
	void*	pvMtrPro;	//����thisָ��
	BYTE*	pbRxBuf; 
	BYTE*	pbTxBuf;	

	//�����Э���Ա����
	//bool(* pfnInit)(struct TMtrPro* pMtrPro); //���Э����γ�ʼ�� 
	int	(* pfnAskItem)(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf); //��ȡ���ݵĶ���ӿں���
	// bool(* pfnBroadcastTime)(TMtrPro* pMtrPro, const TTime& time);//��ʱ	
	
	bool (* pfnRcvBlock)(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //�������պ���
	void (* pfnGetProPrintType)(BYTE* pbPrintPro, char* pszProName);//��ȡ��ӡЭ������
}; 

////////////////////////////////////////////////////////////////////////////////////////////
//MtrProIf˽�г�Ա����
//�����շ�����
extern BYTE m_bInvdData;
extern BYTE m_MtrTxBuf[DYN_PN_NUM][MTR_FRM_SIZE];
extern BYTE m_MtrRxBuf[DYN_PN_NUM][MTR_FRM_SIZE];

/////////////////////////////////////////////////////////////////////////////////////
//�����Э�鹫������
void InitMeterPro();
int SchStrPos(char* pStr, int iStrLen, char c);
WORD GetCRC16(BYTE* pbyt,int iCount);
float POW10(signed char n); //10��n�η�
int64 POW10N(signed char n);//10��n�η���n> 0��
int64 Round(int64 iVal64);  //��������	
bool IsRateId(WORD wID);		//�Ƿ�������йصļ���ID
bool IsDemdId(WORD wID);		//�Ƿ�����ID������������ʱ�䣩
bool IsDemdTime(WORD wID);		//�Ƿ�����ʱ��
bool IsLastMonthId(WORD wID);	//�Ƿ�����ID	
bool IsPhaseEngId(WORD wID);	//�Ƿ�������ID
bool Is645NotSuptId(WORD wID);	//�Ƿ���645��97�棩��֧����Ҫ���ٷ��ص�ID
BYTE GetBlockIdNum(WORD wID);	//ȡ��ID����ID����
BYTE Get645TypeLength(WORD wID);//ȡ645���͵����ݳ���
WORD SetCommDefault(WORD wID, BYTE* pbBuf); //����ͨ�ø�ʽID����Чֵ
void CheckRate(BYTE* pbRateTab, BYTE* pbData, BYTE nlen);	//�����ַ���
void CheckDecimal(BYTE bToltLen, BYTE bItemLen, BYTE bNewDec, BYTE bOldDec, BYTE* pbBuf); //����С��λ	
void CheckDecimalNew(BYTE bDstLen, BYTE bSrcLen, BYTE bDstDec, BYTE bSrcDec, BYTE* pbDstBuf, BYTE* pbSrcBuf);//����ĳ���С��λ
WORD Data645ToComm(WORD wID, BYTE* pbBuf, WORD wLen);//97��645Э���ʽ����ת������ʽ

WORD Id645V07toDL645(WORD wExtId);//����2007��645Э���ȡ����չIDתΪ����Ӧ645ID���Լ���698�ն��ϵĶ�ȡ
WORD Data645to645V07(WORD w645Id, BYTE* pbBuf, WORD wLen);//��97��645���ص�����תΪ2007��64�����ݸ�ʽ���Լ���698�ն��ϵĶ�ȡ

/////////////////////////////////////////////////////////////////////////////////////
//���ڲ�������
bool MtrProOpenComm(TCommPara* pComPara);//Ҫ�жϴ����Ƿ��Ѿ���,�����Ƿ���Ҫ�ı�
WORD MtrProSend(WORD wPortNo, BYTE* pbTxBuf, WORD wLen); //Ҫ��ӷ������ӡ

/////////////////////////////////////////////////////////////////////////////////////////
//�ڲ��ӿں���
bool ReadCommFrm(struct TMtrPro* pMtrPro, void* pTmpInf, DWORD dwDelayTime, DWORD dwMaxSec, DWORD dwMinSec, DWORD dwTimeOut, 
				 DWORD dwReadLength, DWORD dwBufSize, BYTE* pbBuf, DWORD dwFrmSize);

#endif //METERPRO_H


