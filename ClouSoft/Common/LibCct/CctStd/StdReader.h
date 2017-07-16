/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�STDReader.h
 * ժ    Ҫ�����ļ�ʵ����376.2·�ɵĿ���(�Զ�·�ɿ���)
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2014��8��
 *
 * ��    ע��$���ļ�Ϊ��׼���һ����,�벻Ҫ��Э����صĲ��ּ��뵽���ļ�
************************************************************************************************************/
#ifndef STDREADER_H
#define STDREADER_H

#include "Typedef.h"
#include "LoopBuf.h"
#include "AutoReader.h"
#include "LibCctConst.h"
#include "CctIf.h"
#include "ComStruct.h"
//#include "bios.h"

//�Ƿ�ʹ����376.2Э��
//#define EN_NEW_376_2   1
//#undef EN_NEW_376_2 


//֡������C
#define FRM_C_PRM_1		            0x01	//DIR,����
#define FRM_C_PRM_0			        0x00	//DIR,�Ӷ�

#define FRM_C_M_MST		        	0x01    //����ʽ·���ز�ͨ��
#define FRM_C_M_DIST			    0x02    //�ֲ�ʽ·���ز�ͨ��
#define FRM_C_M_RADIO		        0x0a    //΢��������ͨ��
#define FRM_C_M_ETH			    	0x14    //��̫��ͨ��

#define TO_MOD_RT		0	//0��ʾ�Լ�������ͨ��ģ�����
#define TO_MOD_MTR		1	//1��ʾ���ز����ͨ��ģ�����

#define  ROUTETRSANMIT   //͸����վ�·���376.2Э��֡

typedef struct{
	TAutoRdrPara RdrPara;	//�Զ�����������


	BYTE bLink;				//��·����,�������ֲ�ͬ����
	WORD wFrmLenBytes;      //֡�����е��ֽڸ���,1��ʾ1���ֽ�,2��ʾ2���ֽ�
	BYTE bRunMode;          //����ģ������ģʽ��1-3.5��ģʽ 2-3.5����չģʽ 3-3��ģʽ

}TStdPara;		//698��׼·�ɲ���

typedef struct
{
	TStdPara* m_ptStdPara;
	TAutoReader* m_ptAutoReader;
	bool m_fSyncAllMeter;
	BYTE m_bKplvResetCnt;
	BYTE m_bKeepAliveMin;
	DWORD m_dwLastRxClick;
	DWORD m_dwResumeRdClick;	//�ϻ�·�ɲ�����,�ָ������ʱ��
	DWORD m_dwUpdStatusClick;	//�ϻظ���״̬��ʱ��
	DWORD m_dwSyncMeterClick;

	TTime m_tmUdp;		//���һ�θ���ʱ��

	TLoopBuf m_tLoopBuf;

	TCctCommPara m_tCctCommPara;
	
	BYTE* m_pbCctTxBuf;	//���ͻ�����
	BYTE* m_pbCctRxBuf; //���ջ�����

	bool m_fRestartRoute;
	BYTE* m_pbStudyFlag;
}TStdReader;

bool InitStdReader(TStdReader* ptStdReader);

void InitRcv(TStdReader* ptStdReader);
WORD MakeFrm(WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE bAfn, WORD wFn, BYTE* pbData, WORD wDataLen, BYTE bAddrLen, BYTE bMode, BYTE bPRM, BYTE bCn, BYTE bRcReserved);
void SetAddrField(TStdReader* ptStdReader, BYTE* pbTxBuf, BYTE* pbAddr);
BYTE MakeCct645AskItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm, bool bByAcq, BYTE* pb485Addr, BYTE bCtrl);
void ReadMeter(TStdReader* ptStdReader, WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE* pbAddr, WORD wPn, DWORD dwID, BYTE bRdFlg, bool fByAcqUnit, BYTE bCn);
bool RxHandleFrm(TStdReader* ptStdReader, DWORD dwSeconds);
bool DefHanleFrm(TStdReader* ptStdReader);

bool SimpleTxRx(TStdReader* ptStdReader, DWORD dwLen, DWORD dwSeconds, BYTE bSendTimes);
bool CtrlRoute(TStdReader* ptStdReader, WORD wFn);

void HardReset(TStdReader* ptStdReader, BYTE bFn);
bool ResumeReadMeter(TStdReader* ptStdReader);
bool StopReadMeter(TStdReader* ptStdReader);
bool RestartRouter(TStdReader* ptStdReader);
bool IsAllowResumeRdMtr(TStdReader* ptStdReader); //����ָ�����
bool TcRestartRouter(TStdReader* ptStdReader);
//����:	���ù���״̬(�ز�ģ��)
void SetWorkMode(TStdReader* ptStdReader, BYTE bMode);
bool WriteMainNode(TStdReader* ptStdReader);
bool ReadMainNode(TStdReader* ptStdReader);

int ReadPlcNodes(TStdReader* ptStdReader, BYTE *pbBuf, WORD wStartPn);

BYTE  GetRdNum(); //ͬ�����ַʱһ�ζ�ȡ�Ĵӽڵ����

void SetSyncFlag(BYTE* pbSyncFlag, int iNum);
bool GetSyncFlag(BYTE* pbSyncFlag, int iNum);
void ClrSyncFlag(BYTE* pbSyncFlag, int iNum);
WORD  GetStartPn(WORD wLastPn, WORD wReadedCnt);

void AutoResumeRdMtr(TStdReader* ptStdReader);
bool TcRestartNewDay(TStdReader* ptStdReader);
bool DoCctInfo(TStdReader* ptStdReader); 
int RIDIsExist(BYTE* bMeterAddr, BYTE* pbBuf);
bool DelNode(TStdReader* ptStdReader, BYTE* pbAddr);
void ClearAllRid(TStdReader* ptStdReader, WORD wFn);	//������нڵ�(�ز�ģ��) ע�⣺��Ѷ���������ʹ��F3������в��������
bool AddOneNode(TStdReader* ptStdReader, WORD wNo, BYTE* pbAddr);
bool SyncMeterAddr(TStdReader* ptStdReader);
int  UpdateReader(TStdReader* ptStdReader);
int DoLearnAll(TStdReader* ptStdReader);
BYTE Make376TransFrm(TStdReader* ptStdReader, BYTE* pbAFn, BYTE bCtrl, BYTE* pbBuf, BYTE b645Len, BYTE bAddrLen);
BYTE CctMakeBroadcastFrm(TStdReader* ptStdReader, BYTE *pbBuf, BYTE* pbFrm, BYTE bFrmLen, BYTE *pbAddr);
bool IsNoConfirmBroad(); //�㲥�����Ƿ���Ӧ�����̺����������أ�����ʹ�û���ӿ�
BYTE CctMake3762Unlocked(TStdReader* ptStdReader, BYTE *pbCmdBuf, BYTE bLen);
int CctTransmit645CmdUnlocked(TStdReader* ptStdReader, BYTE *pbCmdBuf, BYTE bLen, BYTE *pbData, BYTE* pbrLen, BYTE bTimeOut);
int CctTransmit645Cmd(BYTE* pbCmdBuf, BYTE bLen, BYTE* pbData, BYTE* pbrLen, BYTE bTimeOut);

extern TStdReader* g_ptStdReader;

#endif //STDREADER_H

     