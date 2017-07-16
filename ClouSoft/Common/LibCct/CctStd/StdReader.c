/*********************************************************************************************************
* Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�StdReader.c
* ժ    Ҫ�����ļ�ʵ����376.2�ز�·�ɵĿ��ƻ���(�Զ�·�ɿ���)
* ��ǰ�汾��2.0
* ��    �ߣ�᯼���
* ������ڣ�2014��8��

* ��    ע��$���ļ�Ϊ��׼���һ����,�벻Ҫ��Э����صĲ��ּ��뵽���ļ�
************************************************************************************************************/
#include "StdReader.h"
#include "CctAPI.h"
#include "CctHook.h"
//#include "bios.h"
//#include "syscfg.h"
#include "ComAPI.h"
#include "Trace.h"
#include "SysDebug.h"
//#include "sysapi.h"
#include "DbAPI.h"
#include "FaAPI.h"

#include "CctIf.h"
#include "MeterPro.h"
#include "DrvCfg.h"
#include "Drivers.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//CStdReader

bool InitStdReader(TStdReader* ptStdReader)
{
	bool fOpenOK = false;
	ptStdReader->m_pbCctTxBuf = &m_MtrTxBuf[1][0];
	ptStdReader->m_pbCctRxBuf = &m_MtrRxBuf[1][0];
	if (!LoopBufInit(&ptStdReader->m_tLoopBuf))
			return false;
	
	ptStdReader->m_tCctCommPara.m_tCommPara.wPort = COMM_METER3;//�д�����
	ptStdReader->m_tCctCommPara.m_tCommPara.dwBaudRate = CBR_9600;	
	ptStdReader->m_tCctCommPara.m_tCommPara.bParity =  EVENPARITY;
	ptStdReader->m_tCctCommPara.m_tCommPara.bByteSize = 8;
	ptStdReader->m_tCctCommPara.m_tCommPara.bStopBits = ONESTOPBIT;


	fOpenOK = CctProOpenComm(&ptStdReader->m_tCctCommPara.m_tCommPara);
	if (!fOpenOK)
	{
		DTRACE(DB_CCT, ("CStdReader::Init: fail to open COMM%d for StdRdr\n", COMM_METER3));
		return false;
	}

	ptStdReader->m_fSyncAllMeter = false;
	ptStdReader->m_bKplvResetCnt = 0;
	ptStdReader->m_bKeepAliveMin = 12;
	ptStdReader->m_dwLastRxClick = 0;

	ptStdReader->m_dwResumeRdClick = 0;	//�ϻ�·�ɲ�����,�ָ������ʱ��
	ptStdReader->m_dwUpdStatusClick = 0;	//�ϻظ���״̬��ʱ��

	GetCurTime(&ptStdReader->m_tmUdp);
	
	/*m_dwFcUpdClick = 0;
	m_dwSRUpdClick = 0;*/
	ptStdReader->m_dwSyncMeterClick = 0;

	ptStdReader->m_fRestartRoute = false;

	return true;
}

void SetSyncFlag(BYTE* pbSyncFlag, int iNum) { pbSyncFlag[iNum/8] |= (1<<(iNum%8)); };
bool GetSyncFlag(BYTE* pbSyncFlag, int iNum) { return ((pbSyncFlag[iNum/8]&(1<<(iNum%8))) != 0); };
void ClrSyncFlag(BYTE* pbSyncFlag, int iNum) { pbSyncFlag[iNum/8] &= ~(1<<(iNum%8)); };
WORD  GetStartPn(WORD wLastPn, WORD wReadedCnt){return (wLastPn+wReadedCnt);};

void InitRcv(TStdReader* ptStdReader) 
{ 
	ptStdReader->m_tCctCommPara.m_nRxStep = 0; 

	if (ptStdReader->m_ptStdPara->RdrPara.fUseLoopBuf)
		LoopBufClrBuf(&ptStdReader->m_tLoopBuf);
}

//����:����֡��ַ��
void SetAddrField(TStdReader* ptStdReader, BYTE* pbTxBuf, BYTE* pbAddr)
{
	memcpy(&pbTxBuf[FRM_ADDR], ptStdReader->m_ptStdPara->RdrPara.bMainAddr, 6);
	memcpy(&pbTxBuf[FRM_ADDR+6], pbAddr, 6);
}

WORD MakeFrm(WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE bAfn, WORD wFn, BYTE* pbData, WORD wDataLen, BYTE bAddrLen, BYTE bMode, BYTE bPRM, BYTE bCn, BYTE bRcReserved)
{   
	BYTE bLenBytes = wFrmLenBytes-1;	//֡�����ֽ���	

	//��Ϣ��R	
	pbTxBuf[3 + bLenBytes] = bAddrLen==0?0:4; //D2ͨ��ģ���ʶ��0��ʾ�Լ�������ͨ��ģ�����;1��ʾ���ز����ͨ��ģ�������
	pbTxBuf[4 + bLenBytes] = bCn;		//�ŵ�
	pbTxBuf[5 + bLenBytes] = 0x28;
	pbTxBuf[6 + bLenBytes] = 0;//Ĭ��ͨ������//0x32;
	pbTxBuf[7 + bLenBytes] = 0;
	pbTxBuf[8 + bLenBytes] = 0;

	return CctProMakeFrm(wFrmLenBytes, pbTxBuf, bAfn, wFn, pbData, wDataLen, bAddrLen, bMode, bPRM, bCn, bRcReserved);
}

void MakeConfirm(TStdReader* ptStdReader)
{
	BYTE bData[4] = {0xFF, 0xFF, 0x01, 0x00};
	DWORD dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_CON, F1, bData, 4, 0, FRM_C_M_MST, FRM_C_PRM_0, ptStdReader->m_pbCctRxBuf[FRM_INF+1]&0x0f, 0);
	CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwFrmLen);
}

//����:��645��������֡
BYTE MakeCct645AskItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm, bool bByAcq, BYTE* pb485Addr, BYTE bCtrl)
{ 
	WORD i;
	WORD wPn = PlcAddrToPn(pbAddr, NULL);
	BYTE bDataLen, bCS = 0;

	pbFrm[0] = 0x68;
	memcpy(pbFrm+1, pbAddr, 6);
	pbFrm[7] = 0x68;

	if(bCtrl == 0)
		pbFrm[8] = bMtrPro==CCT_MTRPRO_07 ? 0x11 : 0x01 ;		
	else
		pbFrm[8] = bCtrl;
	
	if(bMtrPro==CCT_MTRPRO_07)
	{
		memcpy(pbFrm+10, &dwID, 4);
		bDataLen = 4;

	}
	else //if (bMtrPro == CCT_MTRPRO_97)
	{		
		memcpy(pbFrm+10, &dwID, 2);
		bDataLen = 2;
	}

	//�Ƿ���Ҫ��645����������Ӳɼ�����ַӦ��Ӧ�ò�ͳһ������Ӧ�ŵ�ͨ�ýӿ���
	if (bByAcq)
	{
		memcpy(pbFrm+10+bDataLen, pb485Addr, 6);
		bDataLen += 6;
	}

	pbFrm[9] = bDataLen;

	for (i=0; i<bDataLen; i++)
		pbFrm[10+i] += 0x33;

	for (i=0; i<10+bDataLen; i++)
		bCS += pbFrm[i];

	pbFrm[10+bDataLen] = bCS;
	pbFrm[11+bDataLen] = 0x16;

	return 12+bDataLen;
}

//����:��������
//������bRtModͨ��ģ���ʶ 0���Լ�������ͨ��ģ�������1�����ز����ͨ��ģ�����
void ReadMeter(TStdReader* ptStdReader, WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE* pbAddr, WORD wPn, DWORD dwID, BYTE bRdFlg, bool fByAcqUnit, BYTE bCn)
{
	DWORD dwFrmLen = 0;
	BYTE bData[256];
	BYTE bMtrAddr[6];
	BYTE bMtrPro;
	BYTE bRtFlg;//GetAutoRdRtFlg();
	memset(bMtrAddr,0,6);

	bMtrPro = GetCctPnMtrPro(wPn);
	//bMtrPro = CCT_MTRPRO_07;
	bRtFlg = TO_MOD_MTR;

	if (bRdFlg == RD_FLG_TORD)	//���Գ���
	{
		//����������
		GetCctMeterAddr(wPn, bMtrAddr);
		bData[0] = RD_FLG_TORD;					//������־
		bData[1] = MakeCct645AskItemFrm(bMtrPro, pbAddr, dwID, &bData[2], fByAcqUnit, bMtrAddr, 0);
		bData[bData[1]+2] = 0;	//�ز��ӽڵ㸽���ڵ�����n

		//��·��ͨ��֡
		SetAddrField(ptStdReader, pbTxBuf, pbAddr);

		if (bRtFlg == TO_MOD_RT) //�Լ�������ͨ��ģ������޵�ַ��
			dwFrmLen = MakeFrm(wFrmLenBytes, pbTxBuf, AFN_RTRD, 1, bData, bData[1]+3, 0, FRM_C_M_MST, FRM_C_PRM_0, bCn, 0);
		else
			dwFrmLen = MakeFrm(wFrmLenBytes, pbTxBuf, AFN_RTRD, 1, bData, bData[1]+3, 12, FRM_C_M_MST, FRM_C_PRM_0, bCn, 0);
		
		if (fByAcqUnit) //���ɼ��������ϲ�����߸����ɼ�����ַ
		{
			DTRACE(DB_CCT, ("CStdReader::ReadMeter: %02x%02x%02x%02x%02x%02x, Pn=%d, ID=0x%08x.\r\n", 
				bMtrAddr[5], bMtrAddr[4], bMtrAddr[3], bMtrAddr[2], bMtrAddr[1], bMtrAddr[0], 
				wPn, dwID));
		}
		else
		{
			DTRACE(DB_CCT, ("CStdReader::ReadMeter: %02x%02x%02x%02x%02x%02x, Pn=%d, ID=0x%08x.\r\n", 
				bData[8], bData[7], bData[6], bData[5], bData[4], bData[3], 
				wPn, dwID));
		}
	}
	else //(bRdFlg==RD_FLG_SUCC || bRdFlg==RD_FLG_FAIL)	//�����ɹ�/ʧ�ܶ��سɹ�
	{
		bData[0] = bRdFlg;//RD_FLG_SUCC;		//������־��01HΪ�����ɹ�,00HΪ����ʧ��
		bData[1] = 0x0; 			//L��0
		bData[2] = 0x0;				//n=0
		SetAddrField(ptStdReader, pbTxBuf, pbAddr);

		if (bRtFlg == TO_MOD_RT)
		{
			dwFrmLen = MakeFrm(wFrmLenBytes, pbTxBuf, AFN_RTRD, 1, bData, 3, 0, FRM_C_M_MST, FRM_C_PRM_0, bCn, 0);
		}
		else
		{
			dwFrmLen = MakeFrm(wFrmLenBytes, pbTxBuf, AFN_RTRD, 1, bData, 3, 12, FRM_C_M_MST, FRM_C_PRM_0, bCn, 0);
		}
	}

	CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, pbTxBuf, dwFrmLen);
}

//����:����֡
bool DefHanleFrm(TStdReader* ptStdReader)
{
	DWORD dwID = 0x9010;
	TUpInf UpInf;
	bool fByAcq;
	BYTE bAddr[6];
	WORD wTmpPn = 0;
	BYTE bMtrPro;
	WORD wRxFrmLen = ByteToWord(&ptStdReader->m_pbCctRxBuf[FRM_LEN]);
	
	ptStdReader->m_dwLastRxClick = GetClick();

	GetUpInf(&ptStdReader->m_pbCctRxBuf[FRM_INF], &UpInf);
	
	//TODO:
	//1.�Ժ���Կ����ñ���bAddrLen����ܻظ�֡�д�������ַ����ɵĲ���,
	//  ����m_bCctRxBuf[FRM_AFN]��m_bCctRxBuf[FRM_AFN_A]����ͳһ��m_bCctRxBuf[FRM_ADDR+bAddrLen]
	if (UpInf.bModule == TO_MOD_RT) //�Լ�������ͨ��ģ�����,������ַ��
	{
		if (ptStdReader->m_pbCctRxBuf[FRM_AFN]==AFN_RTRD && ptStdReader->m_pbCctRxBuf[FRM_LEN]>FRM_AFN+5) //ѯ�ʳ�����
		{
			if (ptStdReader->m_fSyncAllMeter)
			{
				//iSchRet = SearchAnUnReadID(&m_bCctRxBuf[FRM_AFN+4], m_bLinkType, &RwItem);	//����ӿ�ֻ�����ڲ�ID
				
				//1.�ȳ��Բ��ϵ�ַģʽ
				wTmpPn = PlcAddrToPn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+4], NULL);
				
				bMtrPro = GetCctPnMtrPro(wTmpPn);
				//bMtrPro = CCT_MTRPRO_07;
				//wTmpPn = 2;
				if (CCT_MTRPRO_07 ==bMtrPro)
					dwID = 0x00010000;
				else if (bMtrPro)
					dwID = 0x9010;
				
				if (wTmpPn>0 && wTmpPn<POINT_NUM)
				{
					//GetCctMeterAddr(RwItem.wPn, bAddr);
					//ֻ�ṩ0x9010��ѧϰ
					if (GetSyncFlag(ptStdReader->m_pbStudyFlag, wTmpPn))//�Ѿ�ѧϰ���ˣ��óɹ�
						ReadMeter(ptStdReader, ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, &ptStdReader->m_pbCctRxBuf[FRM_AFN+4], wTmpPn, dwID, RD_FLG_SUCC, false, ptStdReader->m_pbCctRxBuf[4+ptStdReader->m_ptStdPara->wFrmLenBytes-1]);
					else
						ReadMeter(ptStdReader, ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, &ptStdReader->m_pbCctRxBuf[FRM_AFN+4], wTmpPn, dwID, RD_FLG_TORD, false, ptStdReader->m_pbCctRxBuf[4+ptStdReader->m_ptStdPara->wFrmLenBytes-1]);

					
				}
			}
			ptStdReader->m_ptAutoReader->m_wMainSleep = 50;

			return true;
		}
		else if (ptStdReader->m_pbCctRxBuf[FRM_AFN]==AFN_REP && ptStdReader->m_pbCctRxBuf[FRM_LEN]>FRM_AFN+5) //�����ϱ���������û��ַ��
		{
			if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == F2)//�򵥴������Ϊѧϰ�ɹ�
 			{
				wTmpPn = PlcAddrToPn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+8], NULL);
				if (wTmpPn>0 && wTmpPn<POINT_NUM)
				{
 					SetSyncFlag(ptStdReader->m_pbStudyFlag, (int)wTmpPn);
				}
 			}
			ptStdReader->m_ptAutoReader->m_wMainSleep = 20;		//�ȴ����ݻ�����˯��Ϊ20ms
			MakeConfirm(ptStdReader);
			return true;
		}
// 		else if (m_bCctRxBuf[FRM_AFN]==AFN_REP && m_bCctRxBuf[FRM_LEN]>FRM_AFN+5) //�����ϱ���������û��ַ��
// 		{
// 			if (DtToFn(&m_bCctRxBuf[FRM_AFN+1]) == F2)
// 			{
// 				SaveRepData(RwInf, UpInf, FRM_AFN); //��˹�������ظýӿڣ����豣�����ݣ�ֱ�ӷ���ȷ��֡
// 				m_wMainSleep = 20;		//�ȴ����ݻ�����˯��Ϊ20ms
// 			}
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN+1]) == F1)	//F1:�ϱ��ز��ӽڵ���Ϣ
// 			{
// 				SaveRepAddr(RepMtrInf); //�ϱ���ַ
// 			}
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN+1]) == F3)
// 			{
// 				if (m_bCctRxBuf[FRM_AFN+3] == 0x02)
// 				{
// 					m_fEndSchMtrFlg = true;
// 					DTRACE(DB_CCT, ("CStdReader :: Route Report Sch Mtr Finished  !!!.\n"));
// 				}
// 			}
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN+1]) == F4)
// 				SaveSchMtrFrm(FRM_AFN);
// 			else
// 				SaveRepData(RwInf, UpInf, FRM_AFN);
// 
// 			return true;
// 		}
// 		else if (m_bCctRxBuf[FRM_AFN]==AFN_QRYDATA && m_bCctRxBuf[FRM_LEN]>FRM_AFN+5) //�����ϱ���������û��ַ��
// 		{
// 			//�ϱ���ģ������ģʽ��Ϣ���ȣ�13���û�������ǰ��Ĺ̶����ȣ�+Ӧ���������е��ϱ���Ϣ����Ϊ40���ֽڣ�����Ϊ������֡
// 			if (DtToFn(&m_bCctRxBuf[FRM_AFN+1])==F10 && wRxFrmLen>(13+40)) 
// 				RtRunModResolve(&m_bCctRxBuf[FRM_AFN+3]);
// 		}
// 
// 		PlcExtent(&m_bCctRxBuf[FRM_AFN]); //���ز����Ҹ�����չ��һЩAFN�Ĵ���
	}
	else //if (UpInf.bModule == TO_MOD_MTR) //���ز����ͨ��ģ�����ʱ���е�ַ��
	{
		//TODO:�Ƿ�Ӧ���ж�һ���Ƿ��յ��������ǲ���'F1:�ϱ��ز��ӽڵ���Ϣ'
		//ע�⣺��˹������֤�Ƿ�������(m_bCctRxBuf[FRM_LEN]>FRM_AFN+5)��������������δ������֤
		if (ptStdReader->m_pbCctRxBuf[FRM_AFN_A]==AFN_REP && ptStdReader->m_pbCctRxBuf[FRM_LEN]>FRM_AFN+5)
		{
			if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN_A+1]) == F2)	// F2:�ϱ���������
 			{
				wTmpPn = PlcAddrToPn(&ptStdReader->m_pbCctRxBuf[FRM_AFN_A+8], NULL);
				//wTmpPn = 2;
				if (wTmpPn>0 && wTmpPn<POINT_NUM)
				{
					SetSyncFlag(ptStdReader->m_pbStudyFlag, (int)wTmpPn);
				}
 			}
			MakeConfirm(ptStdReader);
			return true;
		}
// 		if (m_bCctRxBuf[FRM_AFN_A]==AFN_REP && m_bCctRxBuf[FRM_LEN]>FRM_AFN+5) 
// 		{
// 			if (DtToFn(&m_bCctRxBuf[FRM_AFN_A+1]) == F1)
// 			{
// 				SaveRepAddr(RepMtrInf);	
// 				return true;
// 			}
// 			else if (DtToFn(&m_bCctRxBuf[FRM_AFN_A+1]) == F2)	// F2:�ϱ���������
// 			{
// 				SaveRepData(RwInf, UpInf, FRM_AFN_A);
// 				return true;
// 			}
// #ifdef EN_NEW_376_2		
// 			//Note:376.2Э������������ΪAFN=6,F3�ϱ�·�ɹ����䶯��Ϣ
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F3)  //F3�ϱ�·�ɹ����䶯��Ϣ
// 			{
// 				m_bRepRoutInfo=GetRoutInfo(&m_bCctRxBuf[FRM_AFN_A+3]);
// 				return true;
// 			}			
// 			//Note: 376.2Э����������AFN=6,F4,�ϱ��ӽڵ���Ϣ���豸����
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F4) //F4:�ϱ��ӽڵ���Ϣ���豸����
// 			{
// 				//GetNodesInfo(&m_bCctRxBuf[FRM_AFN_A+3],m_tRptInfo);
// 				//����ǰ����
// 				SaveSchMtrFrm(FRM_AFN_A);
// 				return true;
// 			}
// 			//Note: 376.2Э����������AFN=6,F5�ϱ��ӽڵ��¼�
// 			//�������� SaveRepDataֵ�Աȣ��Ƿ��ܹ��ϲ�
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F5) //F5�ϱ��ӽڵ��¼�
// 			{
// 				//���ý����ϱ��ӽڵ��¼�����֡����
// 				//GetMtrRptEvnt(&m_bCctRxBuf[FRM_AFN_A+3],m_tRptMtrEvnt);
// 				//����ǰ����
// 				SaveRepData(RwInf, UpInf, FRM_AFN_A);
// 				return true;
// 			}
// #endif		
// 		}
// 		else if (m_bCctRxBuf[FRM_AFN_A] == AFN_DEBUG)
// 		{
// 			CctDebug();
// 			return true;
// 		}
// 
// 		//NOTE:
// 		//1.Ŀǰֻ�ж��ŵ�ģ��֧��F2:�ϱ���������,
// 		//  ���Ƕ��Ŵ���ذ�ͨ��ģ���ʶ����ΪTO_MOD_MTR
// 		//2.���̲�֧��֧��F2:�ϱ���������,
// 
// 		if (m_bCctRxBuf[FRM_AFN_A]==AFN_REP && 
// 			DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F2 && //F2:�ϱ���������,
// 			m_bCctRxBuf[FRM_LEN]>FRM_AFN_A+5) 
// 		{
// 			SaveRepData(RwInf, UpInf, FRM_AFN_A);
// 			m_wMainSleep = 20;		//�ȴ����ݻ�����˯��Ϊ20ms
// 
// 			return true;
// 		}
	}

	return false;	//û����֡,����false
}

/*
//����:	����һ�����ݿ�,�ж��Ƿ���յ�һ��������ͨ��֡
//����:	�����Ѿ�ɨ������ֽ���,����յ�һ��������ͨ��֡�򷵻�����,���򷵻ظ���
int RcvFrame(TStdReader* ptStdReader, BYTE* pbBlock, int nLen)
{
	ptStdReader->m_tCctCommPara.m_nRxStep = 0;    //��֡��������ѭ���������е����ݲ���ɾ������ÿ�ν������ɴ�ͷ��ʼ������ֱ���ҵ�һ��������֡�Ż´����m_bCctRxBuf
	int iRxLen = 0;
	BYTE *bHead = pbBlock;
	ptStdReader->m_tCctCommPara..m_fRxComlpete = false;
	for (int i=0; i<nLen; i++)
	{
		BYTE b = *pbBlock++;
		switch (ptStdReader->m_tCctCommPara.m_nRxStep) 
		{
		case 0:
			if (b == 0x68)
			{
				ptStdReader->m_pbCctRxBuf[0] = 0x68;
				ptStdReader->m_tCctCommPara.m_wRxPtr = 1;
				ptStdReader->m_tCctCommPara.m_nRxCnt = ptStdReader->m_ptStdPara->wFrmLenBytes;
				ptStdReader->m_tCctCommPara.m_nRxStep = 1;
			}
			break;
		case 1:
			ptStdReader->m_pbCctRxBuf[m_wRxPtr++] = b;
			ptStdReader->m_tCctCommPara.m_nRxCnt--;

			if (m_nRxCnt == 0)
			{
				if (ptStdReader->m_ptStdPara->wFrmLenBytes == 1)
					ptStdReader->m_tCctCommPara.m_wRxDataLen = ptStdReader->m_pbCctRxBuf[1];
				else
					ptStdReader->m_tCctCommPara.m_wRxDataLen = ptStdReader->m_pbCctRxBuf[1] + ((WORD)ptStdReader->m_pbCctRxBuf[2]<<8);

				//StdRdr <--  4E 23 00 08 F9 FF FF FF 00 00 02 15 68 57 01 00 00 00 00 68 91 09 34 33 39 38 33 33 3B 43 45 C3 16 B1 16 68 13 00 81 00 00 40 00 00 00 00 01 00 FF FF 00 00 C0 16
				if(ptStdReader->m_tCctCommPara.m_wRxDataLen > (WORD)nLen   //�����376.2֡�����ѳ�����������ݳ���
					|| ptStdReader->m_tCctCommPara.m_wRxDataLen + i - 2 > nLen  //�����֡βλ���ѳ�����������ݳ���
					|| bHead[ptStdReader->m_tCctCommPara.m_wRxDataLen + i - 3] != 0x16)  //֡����������0x16
				{
					//��������ǽ��ܲ�ȫ��Ƿ��ı��ģ���m_nRxStep��Ϊ0���൱�ڽ�m_bCctRxBuf�е�������գ�
					//ͬʱ����RxHandleFrm��ɾ��ɨ��������ݣ��Ⱥ�����������һ�𿽱��������������½��н���    add by CPJ at 2012-10-11
					ptStdReader->m_tCctCommPara.m_nRxStep = 0;     
					break;
				}

				ptStdReader->m_tCctCommPara.m_nRxCnt = ptStdReader->m_tCctCommPara.m_wRxDataLen -1 - ptStdReader->m_ptStdPara->wFrmLenBytes;
				ptStdReader->m_tCctCommPara.m_nRxStep = 2;
			}
			break;
		case 2:
			ptStdReader->m_pbCctRxBuf[m_wRxPtr++] = b;
			m_nRxCnt--;

			if (m_nRxCnt == 0)
			{
				ptStdReader->m_tCctCommPara.m_nRxStep = 0;
				iRxLen = ptStdReader->m_tCctCommPara.m_wRxDataLen - 3 - ptStdReader->m_ptStdPara->wFrmLenBytes; //������������ܻ�С��0����ӱ��� add by CPJ at 2012-16-19

				if (iRxLen>0 && ptStdReader->m_tCctCommPara.m_wRxPtr >= 2 && 
					ptStdReader->m_pbCctRxBuf[ptStdReader->m_tCctCommPara.m_wRxPtr-2]==CheckSum(ptStdReader->m_pbCctRxBuf + 1 + ptStdReader->m_ptStdPara->wFrmLenBytes, (WORD)iRxLen)
					&& ptStdReader->m_pbCctRxBuf[ptStdReader->m_tCctCommPara.m_wRxPtr-1]==0x16)
				{
					ptStdReader->m_tCctCommPara.m_fRxComlpete = true;
					ptStdReader->m_dwLastRxClick = GetClick();
					return i+1;
				}
			}
			break;
		default:
			ptStdReader->m_tCctCommPara.m_nRxStep = 0;
			break;
		}
	}

	return -nLen;
}
*/

//����:���ղ�����֡,ÿ�ν��յ�һ��֡�ͷ���
//����:@dwSeconds ���յȴ�������
// 	   @pfLeft ���������Ƿ���ʣ����ֽڻ�û������
//����:���յ�һ��֡�򷵻�true,���򷵻�false
bool RxHandleFrm(TStdReader* ptStdReader, DWORD dwSeconds)
{
	int len = 0;
	BYTE bBuf[200];
	int nScanLen = 0;
	DWORD dwOldTick = GetTick();
	bool fUseLBuf = ptStdReader->m_ptStdPara->RdrPara.fUseLoopBuf;

	//ע��:�ڱ��������������õ�Ĭ��֡������DefHanleFrm()��,��Ҫʹ��m_bTxBuf���鷢��֡,
	// 	   ������ƻ��������Ѿ���õķ���֡

	//ע��:���ﲻ�ܽ�m_nRxStep=0,��Ϊ��Щ�������ñ�������ʱ��,�п��ܵ��ö������һ��֡
	//	   ������ЩҪ���֮ǰ����֡�ֽڵ����,���ڵ��ñ�����ǰ�ȵ���InitRcv()
	do
	{	
		len = 0;
		if (fUseLBuf)
			len = LoopBufGetBufLen(&ptStdReader->m_tLoopBuf);

		if (len <= 0 || !ptStdReader->m_tCctCommPara.m_fRxComlpete)	//���ȴ���ѭ�������������ݣ����߻����������ݣ������ղ�����������Ӧȥ��ʣ�µĽ��ջ���һ����
		{				//���ѭ���������������ݣ��ȴ����껺�����������ڽ��գ������п��ܻ����	
			len = CctReceive(&ptStdReader->m_tCctCommPara.m_tCommPara, bBuf, sizeof(bBuf));
			if (len>0 && fUseLBuf)
				LoopBufPutToBuf(&ptStdReader->m_tLoopBuf, bBuf, len);
		}

		if (fUseLBuf)
			len = LoopBufRxFromBuf(&ptStdReader->m_tLoopBuf, bBuf, sizeof(bBuf)-10);

		if (len > 0)
		{
			nScanLen = CctRcvFrame(bBuf, len, &ptStdReader->m_tCctCommPara, ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctRxBuf, &ptStdReader->m_dwLastRxClick);
			if (nScanLen > 0)
			{
				if (fUseLBuf)
					LoopBufDeleteFromBuf(&ptStdReader->m_tLoopBuf, nScanLen); //ɾ���Ѿ�ɨ�������

				//if (IsNeedToDropFrm(bBuf, len, nScanLen) == false)	//����Ҫ����֡
				{
					if (DefHanleFrm(ptStdReader) == false) //֡��ɹ���,����û�б�Ĭ��֡������������,Ҫ��������ȥ����
						return true;
				}

				//NOTE:
				//��������б�Ĭ��֡�������������֡,���ڱ�����ĬĬ�ؽ��������,
				//������õĺ����͵��ɴ���û�յ���������֡,������Щ֡��������Ҳ���ᱻ����,
				//Ȼ��Ҫ��������ʣ�µ�֡,����·��ģ��һ�����������Ĭ�ϴ���֡,
				//�������ʱ����,�ᵼ�º����֡��λ
			}
			else if(len>=sizeof(bBuf)-10) //���������ˣ����ǻ�û��һ��������֡
			{
				if (fUseLBuf)
					LoopBufDeleteFromBuf(&ptStdReader->m_tLoopBuf, len); //ɾ���Ѿ�ɨ�������
			}
			//else if (nScanLen<0 && m_fUncompleted)   //��ȫ�ı��Ĳ�Ӧ��ɾ����Ӧ�õȺ����ʣ�µı�����ȫ���ٴ�ѭ������������������RcvFrameȥ����     modify by CPJ at 2012-10-11
			//{ 
			// if (m_ptRdrPara->fUseLoopBuf)
			//	 m_LoopBuf.DeleteFromBuf(-nScanLen); //ɾ���Ѿ�ɨ�������
			//}
		}

		if (dwSeconds != 0)
			Sleep(100);	//��ֹ����������������ѭ��

	} while (GetTick()-dwOldTick <= dwSeconds*1000);

	return false;
}

//���������ڷ��������Ǽ򵥵�ȷ��/����֡����ֹͣ���ָ�����������ཻ��֡���ķ��ͺͽ��ս��еķ�װ��
//������
//      @dwLen�����ͳ���
//      @dwSeconds�����ճ�ʱʱ�䣬��λs��Ĭ��3s
//      @bSendTimes�����ʹ�����Ĭ��1��
bool SimpleTxRx(TStdReader* ptStdReader, DWORD dwLen, DWORD dwSeconds, BYTE bSendTimes)
{
	int i;
	if(dwLen==0 || dwSeconds==0 || bSendTimes==0)
		return false;

	for (i=0; i<bSendTimes; i++)
	{		
		if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwLen) == dwLen)
		{
			Sleep(10);
			if (RxHandleFrm(ptStdReader, dwSeconds))			 
			{		
				if (ptStdReader->m_pbCctRxBuf[FRM_AFN] == AFN_CON)   //ȷ��֡/����
				{
					if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == F1)   //ȷ��
					{
						DTRACE(DB_CCT, ("Receive a confirm frm!\r\n"));
						return true;
					}
					else  //����
					{
						DTRACE(DB_CCT, ("Receive a disaffirm frm!\r\n"));
						return false;
					}
				}
				else //AFN��ȷ��/����
				{
					DTRACE(DB_CCT, ("Receive a unknow frm!\r\n"));
					return false;
				}
			}
			else
			{
				DTRACE(DB_CCT, ("TimeOut, Router no ans!\r\n"));
				continue;
			}
		}
		else
		{
			DTRACE(DB_CCT, ("Send fail!\r\n"));
		}
	}

	return false;
}

//�����Ƿ�����ָ�����
bool IsAllowResumeRdMtr(TStdReader* ptStdReader)
{
	if (ptStdReader->m_ptAutoReader->m_fStopRdMtr || !ptStdReader->m_ptAutoReader->m_fInReadPeriod || ptStdReader->m_ptAutoReader->m_dwLastDirOpClick !=0)
		return false;

	return true;
}

//����:	Ӳ����λ(�ز�ģ��)
void HardReset(TStdReader* ptStdReader, BYTE bFn)
{
	BYTE bData[20];
	DWORD dwFrmLen = 0;
	memset(bData, 0, sizeof(bData));

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_INIT, bFn, bData, 0, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

	SimpleTxRx(ptStdReader, dwFrmLen, 5, 1);
	return ;
}

//����:����/ֹͣ�ز�·��
bool CtrlRoute(TStdReader* ptStdReader, WORD wFn)
{
	BYTE bData[20];
	DWORD dwOldClick=0, dwFrmLen=0;
	
	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_CTRLRT, wFn, bData, 0, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

	if(SimpleTxRx(ptStdReader, dwFrmLen, 5, 1))
	{
		switch(wFn)
		{case 1:DTRACE(DB_CCT, ("CStdReader ::Reboot Route OK!!!.\r\n"));return true;
		case 2:DTRACE(DB_CCT, ("CStdReader ::Stop Route OK!!!.\r\n"));return true;
		case 3:DTRACE(DB_CCT, ("CStdReader ::Resume Route OK!!!.\r\n")); return true;
		}
	}

	//DTRACE(DB_CCT, ("CStdReader::CtrlRoute: fail!!!.\r\n"));
	return false;
}

//������ֹͣ�������ڱ���ʽ����Ҫֹͣ���ز���������Ϊ�ռ���
bool StopReadMeter(TStdReader* ptStdReader)
{
	if (ptStdReader->m_ptAutoReader->m_dwLastDirOpClick != 0)	
		return true;
        
        return CtrlRoute(ptStdReader, F2);  //ֹͣ����
}

bool RestartRouter(TStdReader* ptStdReader)
{
	if (ptStdReader->m_ptAutoReader->m_fStopRdMtr || !ptStdReader->m_ptAutoReader->m_fInReadPeriod || ptStdReader->m_ptAutoReader->m_dwLastDirOpClick !=0)
		return false;

	return true;
}

//�������ָ��������ڱ���ʽ����Ҫ�ָ����ز���������Ϊ�ռ���
//      ע�⣺���̵Ļָ�����Ϊ����ѧϰ״̬
bool ResumeReadMeter(TStdReader* ptStdReader)
{
	if(!IsAllowResumeRdMtr(ptStdReader))
		return true;

	return CtrlRoute(ptStdReader, F3); //�ָ�����
}

bool TcRestartRouter(TStdReader* ptStdReader)
{
	TTime tSearchT;
	BYTE rbDuration;
	//if(IsInSchMtrTime(tSearchT, rbDuration) || m_bSchMtrState!=SCHMTR_S_IDLE)	//���Զ��ѱ���ֶ��ѱ�ʱ�β�����������)
	//	return true;
	if(!RestartRouter(ptStdReader))
	{	
		DTRACE(DB_CCT, ("CTcStdReader::RestartRouter: Not in period.\r\n"));
		return true;
	}
	ptStdReader->m_ptAutoReader->m_dwLastDirOpClick = 0;
	return CtrlRoute(ptStdReader, 1);
}

void SetWorkMode(TStdReader* ptStdReader, BYTE bMode)
{
	BYTE bData[20];
	DWORD dwFrmLen = 0;
	memset(bData, 0, sizeof(bData));

	bData[0] = bMode | 0x02;

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_SETRT, F4, bData, 3, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

	if(SimpleTxRx(ptStdReader, dwFrmLen, 5, 1))
	{
		if (bMode == WM_STUDY)
			DTRACE(DB_CCT, ("CStdReader::SetWorkMode: set study mode ok!\n"));
		else
			DTRACE(DB_CCT, ("CStdReader::SetWorkMode: set readmeter mode ok\n"));
	};
}

//����:	������нڵ�(�ز�ģ��)
void ClearAllRid(TStdReader* ptStdReader, WORD wFn)
{

	BYTE bData[20];
	DWORD dwOldClick =0, dwFrmLen=0;
	memset(bData, 0, sizeof(bData));

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_INIT, wFn, bData, 0, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);//����:����,����

	SimpleTxRx(ptStdReader, dwFrmLen, 12, 1);
}

//����:д���ڵ�
//��ע:$���ģ�����ʱ�Ѿ������˵�ַ,����ģ������ĵ�ַ,�����ô���ȥ�ĵ�ַ
// 	   $ֻ���ڶ����ڵ��ַ���������²Ż���ñ������������ڵ��ַ
bool WriteMainNode(TStdReader* ptStdReader)
{
	BYTE bData[20],bBuf[20];
	DWORD  dwFrmLen=0;
	memset(bData, 0, sizeof(bData));
	memset(bBuf, 0, sizeof(bBuf));

	memcpy(bData, ptStdReader->m_ptStdPara->RdrPara.bMainAddr, 6);
	//memcpy(m_bMainAddr, ptStdReader->m_ptStdPara->RdrPara.bMainAddr, 6);

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_CTRL, 1, bData, 6, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

	if(SimpleTxRx(ptStdReader, dwFrmLen, 5, 1))
	{
		DTRACE(DB_CCT, ("CStdReader :: Write Main Note %02x%02x%02x%02x%02x%02x , Ok!!!\n",bData[5],bData[4],bData[3],bData[2],bData[1],bData[0]));
		Sleep(4000);
		return true;
	}

	DTRACE(DB_CCT, ("CStdReader :: Write Main Note fail!!!.\n"));
	return false;
}

//����:	�����ڵ�
bool ReadMainNode(TStdReader* ptStdReader)
{
	BYTE bData[20];
	DWORD  dwFrmLen=0;
	int i;
	memset(bData, 0, sizeof(bData));
	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_QRYDATA, 4, bData, 0, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);
	
	for (i=0; i<3; i++) //����ط�3��
	{	
		if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwFrmLen) == dwFrmLen)
		{					
			if (RxHandleFrm(ptStdReader, 3))
			{
				if (ptStdReader->m_pbCctRxBuf[FRM_AFN] ==AFN_QRYDATA)
				{
					if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1])==4)
					{
						if(IsAllAByte(&ptStdReader->m_pbCctRxBuf[FRM_AFN+3],0xff, 6) || 
							IsAllAByte(&ptStdReader->m_pbCctRxBuf[FRM_AFN+3], 0, 6)   ||
							((memcmp(&ptStdReader->m_pbCctRxBuf[FRM_AFN+3], ptStdReader->m_ptStdPara->RdrPara.bMainAddr, 6)!=0))   
							)
						{//���ڵ�Ҫ��Ψһ�ģ�Ŀǰ���ն˵�ַ��Ϊ���ڵ�
							ptStdReader->m_fSyncAllMeter = false;
							ClearAllRid(ptStdReader, 2);
							return WriteMainNode(ptStdReader);
						}
						else	//���ģ�����ʱ�Ѿ������˵�ַ,����ģ������ĵ�ַ,�����ô���ȥ�ĵ�ַ
						{
							//for (int i=0; i<6; i++)
							//	m_bMainAddr[i] = m_bCctRxBuf[FRM_AFN+3+i];
							//memcpy(ptStdReader->m_ptStdPara->RdrPara.bMainAddr, &ptStdReader->m_pbCctRxBuf[FRM_AFN+3], 6); //ֱ�ӿ���������
							DTRACE(DB_CCT, ("CStdReader :: Read Main Note %02x%02x%02x%02x%02x%02x !!!\n",
								ptStdReader->m_pbCctRxBuf[FRM_AFN+8],ptStdReader->m_pbCctRxBuf[FRM_AFN+7],ptStdReader->m_pbCctRxBuf[FRM_AFN+6],ptStdReader->m_pbCctRxBuf[FRM_AFN+5],ptStdReader->m_pbCctRxBuf[FRM_AFN+4],ptStdReader->m_pbCctRxBuf[FRM_AFN+3]));
							return true;
						}                            
					}
					else
					{

						DTRACE(DB_CCT, ("CStdReader :: Read Main Note fail!!!.\n"));
						return false;
					}
				}
				else 
				{  
					DTRACE(DB_CCT, ("CStdReader :: Read Main Note fail!!!.\n"));
					return false;						
				}
			}			
		}
	}

	DTRACE(DB_CCT, ("CStdReader :: Read Main Note fail!!!.\n"));
	return false;
}

BYTE  GetRdNum(){return 5;}; //ͬ�����ַʱһ�ζ�ȡ�Ĵӽڵ����

//ע��һ������10��,��ʡ�ڴ�
int ReadPlcNodes(TStdReader* ptStdReader, BYTE *pbBuf, WORD wStartPn)
{
	WORD wStartP = wStartPn;	//TODO:�Ƿ�Ӧ�ô�0��ʼ

	BYTE bBuf[120];
	WORD wTotalNum = 0;
	WORD wTmp = 0;
	BYTE bNum,bPtr=GetRdNum();
	BYTE bData[3];
	BYTE *p1 = pbBuf;
	BYTE *p2;
	WORD wRdNum = 0;
	WORD wRxLen =0;
	BYTE bAddr[6];
	bool fExist;
	DWORD dwDelay=1000; //�����ģ�����������ʱʱ��������ȼ���ms�Ա��ģ��û��Ӱ�죬
	int i;
	int j;
	DWORD dwFrmLen = 0;

	DWORD dwOldClick = GetClick();

	if (pbBuf == NULL)
		return -1;
	do
	{
		//if(g_fPlcModChg)
		//	return 0;

		WordToByte(wStartP, &bData[0]);		//��ʼָ��
		bData[2] = bPtr;					//Ҫ��ȡ���ز��ڵ���

		dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_QRYRT, 0x02, bData, 3, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

		if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwFrmLen) == dwFrmLen)
		{
			if (RxHandleFrm(ptStdReader, 3))
			{
				if (ptStdReader->m_pbCctRxBuf[FRM_AFN] == AFN_QRYRT)
				{
					if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == 2)
					{
						wRxLen = ByteToWord(&ptStdReader->m_pbCctRxBuf[FRM_LEN]);
						//ģ�鷵���쳣�����ص���ȷ�ϣ���û�����ݣ�������ϣ��ģ��<--  68 0F 00 8A 00 00 00 00 00 04 10 02 00 A0 16
						//��Ȼ�ܷ���ȷ�ϣ������ܵ�Ҫ������
						if(wRxLen <= 15) 
							return 0;

						wTotalNum = ByteToWord(&ptStdReader->m_pbCctRxBuf[FRM_AFN+3]);

						bNum = ptStdReader->m_pbCctRxBuf[FRM_AFN+5];	//���ζ�ȡ�ڵ���

						if (bNum > 0)
						{
							wTmp += bNum;

							wStartP = GetStartPn(wStartP, bNum);

							WordToByte(wStartP, &bData[0]);

							{//�������Ч�ĵ�ַ��ǰ��,�����ַ�������Ļ�����᷵�س��������ĵ���ַ������ַ������������²��������
								p2 = bBuf;
								for (i = bNum-1; i>=0; i--)
								{
									memcpy(p2, &ptStdReader->m_pbCctRxBuf[FRM_AFN+6+i*8], 6);
									p2 += 6;									
								}

								//��Ϊ�����·�ɱ��ַ�������õĻ���ĳ������λ��û�б����������һ����Ч�ı�����λ���ϣ�
								//�����������·�ɵ��������Ļ�ʵ�ʻ����������������ı���������кܶ����ظ��ģ�������Ҫ������ظ���ɾ��
								if (wRdNum == 0)
								{//��һ�ζ��϶�û���ظ���
									memcpy(pbBuf, bBuf, bNum*6);
									wRdNum = bNum;
								}
								else
								{
									for (i=0; i<bNum; i++)
									{
										memcpy(bAddr, &bBuf[i*6], 6);
										fExist = false;
										for (j=0; j<wRdNum; j++)
										{
											if (memcmp(bAddr, pbBuf+j*6, 6) == 0)
											{
												fExist = true;
												break;
											}
										}
										if (!fExist)
										{
											memcpy(pbBuf+wRdNum*6, bAddr, 6);
											wRdNum++;
										}
									}
								}

								wTmp = wRdNum;	//��������㲻����ʱ��Ҫ����һ��·�ɽڵ���
							}

							if (wTotalNum == wTmp)	//������
								break;
						}
						else
						{
							if (wTotalNum == 0)
								break;
						}
					}
					else
					{
						DTRACE(DB_CCT, ("CStdReader :: ReadPlcNodes Start Pn:%d fail!!!.\n", wStartP));
						if(AR_LNK_STD_XC == ptStdReader->m_ptStdPara->bLink)
						{
							wStartP++;
							if(wStartP>=1024)
								break;

							continue;
						}

						break;
					}
				}
				else 
				{  
					DTRACE(DB_CCT, ("CStdReader :: ReadPlcNodes Start Pn:%d fail!!!.\n", wStartP));
					if(AR_LNK_STD_XC == ptStdReader->m_ptStdPara->bLink)
					{
						wStartP++;
						if(wStartP>=1024)
							break;

						continue;
					}


					break;
				}			
			}
		}

		Sleep(dwDelay);

	} while (GetClick()-dwOldClick <300);

	DTRACE(DB_CCT, ("CStdReader::ReadPlcNodes Ok!!!\n"));

	return wTotalNum;
}


//*****************************************************************//

//����������2����û�յ�·�����󣬷��ͻָ�����
void AutoResumeRdMtr(TStdReader* ptStdReader)
{
	BYTE bTmp = 2;
	if (ptStdReader->m_ptAutoReader->m_dwLastDirOpClick!=0 && GetClick()>(ptStdReader->m_ptAutoReader->m_dwLastDirOpClick + (DWORD)bTmp*60))//ֱֹͣ��2���Ӻ�ָ�·��
	{
		ptStdReader->m_ptAutoReader->m_dwLastDirOpClick = 0;
		DTRACE(DB_CCT, ("AutoResumeRdMtr::KeepAlive: ResumeReadMeter m_dwLastDirOpClick!=0 && GetClick()>(m_dwLastDirOpClick+%d.\r\n",(DWORD )bTmp*60));
		ResumeReadMeter(ptStdReader);	//�ָ��Զ�����
		if (ptStdReader->m_fSyncAllMeter)
		{
			ptStdReader->m_ptAutoReader->m_bState = AR_S_LEARN_ALL;
		}
	}

	if(ptStdReader->m_ptAutoReader->m_dwLastDirOpClick==0)//��ֱ��ʱ�α���·��״̬
	{
		if (ptStdReader->m_fSyncAllMeter)
		{
			ptStdReader->m_ptAutoReader->m_bState = AR_S_LEARN_ALL;
		}
	}
}

bool TcRestartNewDay(TStdReader* ptStdReader)
{
	HardReset(ptStdReader, 1);
	RxHandleFrm(ptStdReader, 10);
	Sleep(10000);
	ptStdReader->m_fSyncAllMeter = false;
	return true;
}

//����:	�������
BYTE KeepAlive(TStdReader* ptStdReader)
{
	if (ptStdReader->m_bKplvResetCnt > 3)
	{
		DTRACE(DB_CCT, ("CStdReader :: KeepAlive poweroff router.\n"));
		//Ӳ����λ����
		ptStdReader->m_ptAutoReader->m_bState=AR_S_EXIT;		//�˳�·�����̣�����ʶ��ģ��
		return 1; 
	}
	else if (GetClick()-ptStdReader->m_dwLastRxClick > ptStdReader->m_bKeepAliveMin*60*60)
	{
		HardReset(ptStdReader, 1);
		ptStdReader->m_fSyncAllMeter = false;
		ptStdReader->m_dwLastRxClick = GetClick();
		ptStdReader->m_bKplvResetCnt++;
		return 2;
	}

	return 0;
}

int RIDIsExist(BYTE* bMeterAddr, BYTE* pbBuf)
{
	int i;
	for (i=0; i<CCT_POINT_NUM+1; i++)
	{
		if (memcmp(pbBuf, &bMeterAddr[i*6], 6) == 0)
			return i;
	}

	return -1;
}

//����:	���·�ɰ���ڵ�
bool DelNode(TStdReader* ptStdReader, BYTE* pbAddr)
{
	DWORD  dwFrmLen=0;
	int iLen = 0;
	BYTE bData[20], bBuf[512];

	memset(bData, 0, sizeof(bData));

	bData[0] = 0x01;
	memcpy(&bData[1], pbAddr, 6);

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_SETRT, 2, bData, 7, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

	InitRcv(ptStdReader); //��ջ������������յ������֡

	if(SimpleTxRx(ptStdReader, dwFrmLen, 5, 1))
	{
		DTRACE(DB_CCT, ("CStdReader:: DelNode: %02x%02x%02x%02x%02x%02x ok!!!\n",pbAddr[5],pbAddr[4],pbAddr[3],pbAddr[2],pbAddr[1],pbAddr[0]));
		return true;
	}

	DTRACE(DB_CCT, ("CStdReader :: DelNode One Note fail!!!.\n"));
	return false;
}

bool AddOneNode(TStdReader* ptStdReader, WORD wNo, BYTE* pbAddr)
{
	char szAddr[16];
	BYTE bData[20];
	DWORD  dwFrmLen=0;
	int i;
	BYTE bMtrPro;

	memset(bData, 0, sizeof(bData));
	bData[0] = 0x01;
	memcpy(&bData[1], pbAddr, 6);


	bMtrPro = GetCctPnMtrPro(wNo);
	if (bMtrPro == CCT_MTRPRO_07)
		bData[7] = 2;
	else
		bData[7] = 1;
	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_SETRT, 1, bData, 10, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);
	
	InitRcv(ptStdReader); //��ջ������������յ������֡

	for (i=0; i<3; i++) //����ط�3��	
	{				
		if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwFrmLen) == dwFrmLen)
		{				
			if (RxHandleFrm(ptStdReader, 3))
			{					
				if (ptStdReader->m_pbCctRxBuf[FRM_AFN] == AFN_CON)
				{
					if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == F1)
					{
						DTRACE(DB_CCT, ("CStdReader::AddOneNode: NO=%d, %s OK!!!.\n",
							wNo, MtrAddrToStr(pbAddr, szAddr)));
						return true;
					}
					else if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1])==F2 && ptStdReader->m_pbCctRxBuf[FRM_AFN+3]==6)
					{                           
						DTRACE(DB_CCT, ("CStdReader::AddOneNode: This note is existed, AddNote Fail!!!.\n"));
						return false;
					}
					else
					{
						SetInfo(INFO_PLC_CLRRT); //��·��
						DTRACE(DB_CCT, ("CStdReader::AddOneNode: frm mismatch, Fail!!!.\n"));
						return false;
					}
				} 
				else 
				{  
					DTRACE(DB_CCT, ("CStdReader::AddOneNode: frm mismatch, Fail!!!.\n"));
					SetInfo(INFO_PLC_CLRRT); //��·��
					return false;						
				}
			}			
		}

		Sleep(1000);
	}

	DTRACE(DB_CCT, ("CStdReader::AddOneNode: Fail!!!.\n"));


	return false;
}

//��������376.2͸��֡�ӿڣ�������Ϊ�����׼�ӿڣ�����������������������
//������@bAFn:͸��ʹ�õ�AFN
//      @bCtrl:��Լ����
//      @pb645buf:͸���ı���ָ�룬@b645Len:645���ĳ��ȣ�@bAddrLen:376.2��ַ�򳤶�
//add by CPJ at 2012-10-04
BYTE Make376TransFrm(TStdReader* ptStdReader, BYTE* pbAFn, BYTE bCtrl, BYTE* pbBuf, BYTE b645Len, BYTE bAddrLen)
{
	BYTE bTmpbuf[256] = {0,};
	//*pbAFn = AFN_RTFWD;	

	bTmpbuf[0] = bCtrl;     //��Լ����
	bTmpbuf[1] = 0;         //�ӽڵ�����
	bTmpbuf[2] = b645Len;   //���ĳ���

	memcpy(&bTmpbuf[3], pbBuf, b645Len);

	return MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, *pbAFn, F1, bTmpbuf, b645Len+3, bAddrLen, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);
}

//��������376.2�㲥֡�ӿڣ�������Ϊ�����׼�ӿڣ���������������ͬ������
//������@pbBuf��376.2�㲥ָ֡��
//      @pbFrm��ԭʼ����ָ�룬@bFrmLen��ԭʼ�㲥���ݳ���
//      @pbAddr���㲥��ַ
BYTE CctMakeBroadcastFrm(TStdReader* ptStdReader, BYTE *pbBuf, BYTE* pbFrm, BYTE bFrmLen, BYTE *pbAddr)
{
	SetAddrField(ptStdReader, ptStdReader->m_pbCctTxBuf, pbAddr);

	return MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_CTRL, 3, pbBuf, (WORD )bFrmLen+2, 12, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);
}

bool IsNoConfirmBroad(){return false;}; //�㲥�����Ƿ���Ӧ�����̺����������أ�����ʹ�û���ӿ�

//����:�����㲥����,����ʵ�ֹ㲥Уʱ,�㲥����ȹ���
//����:@bCtrl ������,���ָ�698.42����,��չBC_CTL_ADJTIME 0x10Ϊ�㲥Уʱ
// 					00H=͸������; 01H=DLT/645-1997; 02H=DLT/645-2007; 03H=��λʶ����; 04H FFH������
//	   @pbFrm ��������
// 	   @bFrmLen  ���ĳ���
/*TODO: modify by CPJ at 2012-10-04
2012-10-04 ֮��汾���̸���Ϊ��Ϊʹ�ýӿ����̸���ͳһ�͹淶�����ʱ��ӿ�ֻ�����͹㲥���Թ㲥����ظ�֡�ļ�飬
��������˹�����ط����������͵Ļָ�������������ӿ���� */
bool CctBroadcast(TStdReader* ptStdReader, BYTE bCtrl, BYTE* pbBuf, BYTE* pbFrm, BYTE bFrmLen)
{
	int iLen = 0;
	int i;
	DWORD dwLen=0, dwOldClick=0, dwFrmLen;
	WORD wOffset = 0;
	BYTE bAddr[]={0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
	
	if (!StopReadMeter(ptStdReader))	//ֹͣ�Զ�����
		return false;

	DTRACE(DB_CCT, ("CStdReader::Broadcast: start.\r\n"));

	if (bCtrl == BC_CTL_ADJTIME) //���ڱ��ӿ���˵,�㲥Уʱûʲô���⴦��,�û�698.42�Ŀ�����
		bCtrl = CCT_MTRPRO_07;	//m_pStdPara->RdrPara.bMtrPro;

	pbBuf[0] = bCtrl;
	pbBuf[1] = bFrmLen;
	memcpy(&pbBuf[2], pbFrm, bFrmLen);

	dwFrmLen = CctMakeBroadcastFrm(ptStdReader, pbBuf, pbFrm, bFrmLen, bAddr);

	if(dwFrmLen <= 0)
		return false;

	for (i=0; i<1; i++) //����ط�3��
	{
		if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwFrmLen) == dwFrmLen)
		{
			dwLen = 0;
			dwOldClick = GetClick();

			if (RxHandleFrm(ptStdReader, 3))
			{
				if (ptStdReader->m_pbCctRxBuf[FRM_AFN] == AFN_CON)
				{
					if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == 1)	//ȷ��֡
						DTRACE(DB_CCT, ("CStdReader::Broadcast: rx confirm frm.\r\n"));
					return true;
				}
				else
				{
					DTRACE(DB_CCT, ("CStdReader::Broadcast: fail to rx confirm frm!\r\n"));
					return false;
				}
			}
			else 
			{
				if (IsNoConfirmBroad())
					return true;
				else
				{
					DTRACE(DB_CCT, ("CStdReader::Broadcast: fail to rx frm.\n"));
					return false;
				}
			}
		}
	} 

	return false;
}

BYTE CctMake3762Unlocked(TStdReader* ptStdReader, BYTE *pbCmdBuf, BYTE bLen)
{
	BYTE bAfnPos = FRM_AFN_A;
	BYTE bNodeAddr[6];
	BYTE bMtrAddr[6];
	BYTE bData[2], bBuf[256];
	DWORD dwLen=0, dwOldClick, dwFrmId;
	BYTE bFrmLen;
	int iLen = 0;
	int i=0, iDataLen, iIdLen,j, iFrmPos;
	//bool fByAcq = false;
	//bool IsAcq = false;
	//BYTE bCS = 0;
	BYTE bTxLen=0;
	//WORD wPn = 1;
	BYTE bAskType;
	
	bTxLen=0;
	for ( i=0;i<10;i++)
	{
		if (pbCmdBuf[i]==0x68 && pbCmdBuf[i+7]==0x68)
		{
			bTxLen=bLen-i;
			memcpy(bBuf,pbCmdBuf+i,bTxLen);
			iFrmPos = i;
			break;
		}
	}
	if (i>=10)
		return 0;
	
	StopReadMeter(ptStdReader);
	CommRead(ptStdReader->m_tCctCommPara.m_tCommPara.wPort, NULL, 0, 200);
    ptStdReader->m_ptAutoReader->m_dwLastDirOpClick = GetClick();

	memset(bNodeAddr, 0, 6);
	memset(bMtrAddr, 0, 6);		
	memcpy(bMtrAddr,&bBuf[1],6);

	//wPn = PlcAddrToPn(bMtrAddr, NULL);	

	// 		fByAcq = IsReadByAcqUnit(wPn);
	// 		if (fByAcq)
	// 		{
	// 			IsAcq = true;
	// 			GetPlcNodeAddr(wPn, bNodeAddr);		    		
	// 			bTxLen = SetAcqAddrFor645Frm(bBuf, bTxLen, bNodeAddr);
	// 		}

	//���ʣ�Ŀǰ�����ݹ�����ͳ����жϹ�Լ�����Ƿ񲻹����ƣ� 2012-10-04
	if (bBuf[8]==0x1C || bBuf[8]==3 || bBuf[9]==4 || bBuf[9]==10)//3�����ת��ʱ��C=3,�϶���07��
		bData[0] = 2;
	else
		bData[0] = 1;

	if(bTxLen == 0)
		return 0;

	//TraceBuf(DB_CCT, "CStdReader::Transmit645CmdUnlocked: tx -> ",bBuf, bTxLen);

	//if (IsAcq)
	//	SetAddrField(ptStdReader->m_pbCctTxBuf, bNodeAddr);
	//else
	SetAddrField(ptStdReader, ptStdReader->m_pbCctTxBuf, bMtrAddr);

	bAskType = AFN_RTFWD;	

	bFrmLen = Make376TransFrm(ptStdReader, &bAskType, bData[0], bBuf, bTxLen, 12);

	if(bFrmLen == 0)
		return 0;
	TraceBuf(DB_CCTTXFRM, "Cct --> ", ptStdReader->m_pbCctTxBuf, bFrmLen);
	
	return bFrmLen;
}

int CctTransmit645CmdUnlocked(TStdReader* ptStdReader, BYTE *pbCmdBuf, BYTE bLen,BYTE *pbData, BYTE* pbrLen, BYTE bTimeOut)
{
	BYTE bAfnPos = FRM_AFN_A;
	BYTE bNodeAddr[6];
	BYTE bMtrAddr[6];
	BYTE bData[2], bBuf[256];
	DWORD dwLen=0, dwFrmLen, dwOldClick, dwFrmId;
	int iLen = 0;
	int i=0, iDataLen, iIdLen,j, iFrmPos;
	//bool fByAcq = false;
	//bool IsAcq = false;
	//BYTE bCS = 0;
	BYTE bTxLen=0;
	WORD wPn = 1;
	BYTE bAskType;

	if(bTimeOut == 0)
		bTimeOut = TRANSMIT_TIME_OUT;

#ifdef ROUTETRSANMIT
	ptStdReader->m_tCctCommPara.m_nRxStep = 0;  //��ֹ֮ǰ�յ��Ĳ���֡��Ӱ�����
	memset(ptStdReader->m_pbCctRxBuf, 00, sizeof(ptStdReader->m_pbCctRxBuf));
	memcpy(bBuf, pbCmdBuf, bLen);
	if(CctRcvFrame(bBuf, bLen, &ptStdReader->m_tCctCommPara, ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctRxBuf, &ptStdReader->m_dwLastRxClick)!=bLen)
	{
		ptStdReader->m_tCctCommPara.m_nRxStep = 0;  //��ֹ֮ǰ�յ��Ĳ���֡��Ӱ�����
		memset(ptStdReader->m_pbCctRxBuf, 0x00, bLen);
		memset(bBuf, 0x00, sizeof(bBuf));
		goto Transmit645;
	}

	memset(ptStdReader->m_pbCctRxBuf, 0x00, sizeof(ptStdReader->m_pbCctRxBuf));

	if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, bBuf, bLen) == bLen)
	{
		if (RxHandleFrm(ptStdReader, bTimeOut))
		{
			memcpy(pbData, ptStdReader->m_pbCctRxBuf, ptStdReader->m_tCctCommPara.m_wRxDataLen);
			*pbrLen = (BYTE)ptStdReader->m_tCctCommPara.m_wRxDataLen;
			DTRACE(DB_CCT, ("CStdReader::Transmit645CmdUnlocked: Transmit 376.2 Proto Frame Success!!!\n"));
			return 1;
		}
	}

	DTRACE(DB_CCT, ("CStdReader::Transmit645CmdUnlocked: Transmit 376.2 Proto Frame Fail!!!\n"));
	*pbrLen = 0;

	return 0;

#endif

Transmit645:
	bTxLen=0;
	for ( i=0;i<10;i++)
	{
		if (pbCmdBuf[i]==0x68 && pbCmdBuf[i+7]==0x68)
		{
			bTxLen=bLen-i;
			memcpy(bBuf,pbCmdBuf+i,bTxLen);
			iFrmPos = i;
			break;
		}
	}
	if (i>=10)
		return -1;

	//͸���㲥������Уʱ
	if(IsAllAByte(&bBuf[1], 0x99, 6))
	{
		if(bBuf[8]==0x04)
			CctBroadcast(ptStdReader, 1, bBuf, pbCmdBuf+iFrmPos, bTxLen);
		else if(bBuf[8]==0x08)
			CctBroadcast(ptStdReader, 2, bBuf, pbCmdBuf+iFrmPos, bTxLen);

		*pbrLen = 0;
		return 1;
	}

	for (i=0; i<3; i++) //����ط�6��
	{
		if (i > 0)
			StopReadMeter(ptStdReader);

		memset(bNodeAddr, 0, 6);
		memset(bMtrAddr, 0, 6);		
		memcpy(bMtrAddr,&bBuf[1],6);

		wPn = PlcAddrToPn(bMtrAddr, NULL);	

// 		fByAcq = IsReadByAcqUnit(wPn);
// 		if (fByAcq)
// 		{
// 			IsAcq = true;
// 			GetPlcNodeAddr(wPn, bNodeAddr);		    		
// 			bTxLen = SetAcqAddrFor645Frm(bBuf, bTxLen, bNodeAddr);
// 		}

		//���ʣ�Ŀǰ�����ݹ�����ͳ����жϹ�Լ�����Ƿ񲻹����ƣ� 2012-10-04
		if (bBuf[8]==0x1C || bBuf[8]==3 || bBuf[9]==4 || bBuf[9]==10)//3�����ת��ʱ��C=3,�϶���07��
			bData[0] = 2;
		else
			bData[0] = 1;

		if(bTxLen == 0)
			return -1;

		TraceBuf(DB_CCT, "CStdReader::Transmit645CmdUnlocked: tx -> ",bBuf, bTxLen);

		//if (IsAcq)
		//	SetAddrField(ptStdReader->m_pbCctTxBuf, bNodeAddr);
		//else
		SetAddrField(ptStdReader, ptStdReader->m_pbCctTxBuf, bMtrAddr);

		bAskType = AFN_RTFWD;	

		dwFrmLen = Make376TransFrm(ptStdReader, &bAskType, bData[0], bBuf, bTxLen, 12);

		if(dwFrmLen == 0)
			return -1;

		InitRcv(ptStdReader);

		if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwFrmLen) == dwFrmLen)
		{
			if (RxHandleFrm(ptStdReader, bTimeOut))
			{
				if (ptStdReader->m_pbCctRxBuf[bAfnPos]==bAskType && DtToFn(&ptStdReader->m_pbCctRxBuf[bAfnPos+1])==1)
				{
					//������ʹ������ת�����Ǽ�شӽڵ㣬�䷵��֡��ʽ����ͬ������Ϊ������ַ����12�ֽ�
					if (ptStdReader->m_pbCctRxBuf[1] < (bAfnPos==FRM_AFN_A ? 30 : (30-12)))
					{//û��Ӧ��֡
						DTRACE(DB_CCT, ("CStdReader::Transmit645CmdUnlocked: rx no ans!!!.\n"));
						return -1;
					}

					*pbrLen = (int )ptStdReader->m_pbCctRxBuf[bAfnPos+4];	//14=5+9
// 					if (IsAcqAddIn645(fByAcq))
// 					{
// 						memcpy(bBuf,&m_bRxBuf[bAfnPos+5],rbLen);
// 						memcpy(bBuf+1,&m_bRxBuf[bAfnPos+5+10+m_bRxBuf[bAfnPos+5+9]-6], 6);
// 						for (j=0; j<6; j++)
// 							bBuf[1+j] -= 0x33;
// 						rbLen -= 6;
// 						bBuf[9] = bBuf[9] - 6;
// 						bCS = 0;
// 						for (j=0; j<10+bBuf[9]; j++)
// 							bCS += bBuf[j];
// 						bBuf[10+bBuf[9]] = bCS;
// 						bBuf[10+bBuf[9]+1] = 0x16;
// 						memcpy(pbData,bBuf,rbLen);
// 					}
					//else
					memcpy(pbData, &ptStdReader->m_pbCctRxBuf[bAfnPos+5], *pbrLen);

					TraceBuf(DB_CCT, "CStdReader::Transmit645CmdUnlocked: rx <- ",pbData, *pbrLen);
					return 1;
				}
			}
		}
	}

	DTRACE(DB_CCT, ("CStdReader::Transmit645CmdUnlocked: rx no ans!!!.\n"));

	return -1;
}

//����:	͸��645��
int CctTransmit645Cmd(BYTE* pbCmdBuf, BYTE bLen, BYTE* pbData, BYTE* pbrLen, BYTE bTimeOut)
{
	int iRet;
	
	g_ptStdReader->m_ptAutoReader->m_fDirOp = true;	//����ֱ�Ӳ���
	if (!StopReadMeter(g_ptStdReader))	//ֹͣ�Զ�����
	{
		g_ptStdReader->m_ptAutoReader->m_fDirOp = false;
		return -1;
	}

	g_ptStdReader->m_ptAutoReader->m_dwLastDirOpClick = GetClick(); 

	iRet = CctTransmit645CmdUnlocked(g_ptStdReader, pbCmdBuf, bLen, pbData, pbrLen, bTimeOut);

	ResumeReadMeter(g_ptStdReader);	//�ָ��Զ�����
	g_ptStdReader->m_ptAutoReader->m_fDirOp = false;
	return iRet;
}

//����:�Զ�ͬ���ն����ݿ���ز�·���е��ز����ַ
//����:true-��ʼ��ͬ���ն˺ͼ����������еĵ���ַ
//����:
//TODO:�����ͬ�����ַ������ֻ��ͬ����صĲ������ڻָ�����ǰ����Ĳ�����
//     ��Ӳ����λ�����������Լ���� modify by CPJ at 2012-09-21
bool SyncMeterAddr(TStdReader* ptStdReader)
{	
	BYTE bBuf[8];
	BYTE  m_bMeterAddr[32][6];	//��¼�Ѿ����뵽�ز�ģ��ı��ַ
	BYTE  m_bSyncFlag[32/8+1];


	BYTE bMeterAddr[CCT_POINT_NUM+1][6];	//���ڴ�Ŵ�·�����ص��ز����ַ
	BYTE bAddr[6];
	WORD wPn;
	bool fRet = true,fNoMtr=true;
	int iDesMeterCnt = 0;
	int iSrcMeterPtr = 0;
	int i, j;
	bool fNewNode = true;
	bool fFindPlase = false;

	//��·�����ص��ز���ַ
	DTRACE(DB_CCT, ("CStdReader :: SyncMeterAddr : Sysnc all meter address.\n"));
	memset(bMeterAddr, 0, sizeof(bMeterAddr));

	iDesMeterCnt = ReadPlcNodes(ptStdReader, (BYTE *)&bMeterAddr, 1);

	fRet = true;	//��ʧ�ܵ�ʱ����Ϊfalse,������¼ͬ�������з����Ĵ���
	memset(m_bSyncFlag, 0, sizeof(m_bSyncFlag));
	memset(m_bMeterAddr, 0, sizeof(m_bMeterAddr));

	//�����ݿ��е��ز���ַ���뵽m_bMeterAddr
	for (wPn=0; wPn<POINT_NUM; wPn++)
	{
		//����ֱ����GetPlcNodeAddr�ӿ����ж��Ƿ�Ϊee����Ϊ���ܱ����ѱ��ַʱ���ܻ��ѳ���ַΪee�Ĳ����㣬
		//����ǺϷ��ģ�������ͬ����·��, modify by CPJ at 2013-05-20
		if (GetPlcNodeAddr(wPn, bAddr) && !IsAllAByte(bAddr, 0xee, 6)) 
		{
			fNoMtr=false;
			//�鿴�Ƿ����ظ��Ľڵ��ַ,����ͬһ�ɼ����µ�485��,�ɼ�����ַ����ֶ��
			fNewNode = true;
			fFindPlase = false;
			for (i=0; i<CCT_POINT_NUM; i++)
			{
				if (!GetSyncFlag(m_bSyncFlag, i))//��Ч�ڵ�űȽ�
				{
					if (!fFindPlase)
					{
						j = i;//Ϊ�ų�������ź��ز��ڵ�Ų�ͬ��������
						fFindPlase = true;
					}
					continue;
				}

				if (memcmp(&m_bMeterAddr[i], bAddr, 6) == 0)
				{
					fNewNode = false;		
					break;
				}			
			}

			//��Ч�ز��ڵ������
			if (fNewNode)
			{
				memcpy(m_bMeterAddr[j], bAddr, 6);
				SetSyncFlag(m_bSyncFlag, j); //�ø��ı�־
			}
		}
	}

	//��������������˲�����ʼ������������û��Ҫ������ɾ��һ�أ�
	//���ֱ����Դ�ʩ���е�ģ����ܻ�������⣬������Щɾ�����ɾ������⣬������ģ�鳧��ȥ����·��
	//�綫��ɾ��һ���Ѿ������ڵĽڵ�᷵�ط��ϣ�����Ŀǰ�Ѿ����鲻Ҫ��ô�� modify by CPJ at 2012-06-01
	if (fNoMtr)
	{
		ClearAllRid(ptStdReader, 2);
		ptStdReader->m_fSyncAllMeter = true; //������ͳһ��
		g_fCctInitOk = true;

		DTRACE(DB_CCT, ("CStdReader :: SyncMeterAddr : PLC nodes synchronized done.\n"));

		//һ����û�У�Ҳ��û��Ҫ���������ˣ�ֱ�ӷ���
		return true;
	}

	//�Ƚϴ�·�����ص��ز���ַ�����ݿ������õ��ز���ַ
	//���ĳ����������ز���ַ��·�����Ѿ�����,������±�־,�����ٸ��µ�·����ȥ
	for (i=0; i<iDesMeterCnt; i++)
	{
		//iSrcMeterPtr = RIDIsExist(&m_bMeterAddr[0][0], bMeterAddr[i]); //��·�����ص��ز����ַ�Ƿ������ݿ���������
		//if (iSrcMeterPtr >= 0)
		if(memcmp(&m_bMeterAddr[i], &bMeterAddr[i], 6) == 0)//�ϸ����
		{
			//ClrSyncFlag(m_bSyncFlag, iSrcMeterPtr);
			ClrSyncFlag(m_bSyncFlag, i);
		}
		else
		{
			if (DelNode(ptStdReader, bMeterAddr[i]) == false)	//TODO:ɾ��ʧ�ܴ�����
				fRet = false;
		}
	}

	for (i=0; i<CCT_POINT_NUM; i++)
	{
		if (GetSyncFlag(m_bSyncFlag, i)) //ʣ���б�־λ�Ķ���·����û����Ӧ�ز���ַ���õ�
		{
			//if(g_fPlcModChg)  //�ز�ģ�鱻������
			//	return false;

			if (AddOneNode(ptStdReader, i, m_bMeterAddr[i]))
			{
				ptStdReader->m_dwLastRxClick = GetClick();		//�����϶�ʱ��ͬ��ʱ��ϳ��������ֹͬ���󲻱�Ҫ��ģ�鸴λ	
				ClrSyncFlag(m_bSyncFlag, i);
				//ֻ�и��³ɹ��˲Ű��µı��ַ���µ�m_bMeterAddr[wPn]��
				//����±�־
				//���򲻸���,���»�����
			}
			else
			{
				DelNode(ptStdReader, m_bMeterAddr[i]);
				fRet = false;
			}
		}
	}

	if (fRet) //ֻ��ȫ�������ɹ��Ű�m_fSyncAllMeter��Ϊtrue,�����»ػ�Ҫͬ��һ��
	{
		ptStdReader->m_fSyncAllMeter=true; //������ͳһ����
		g_fCctInitOk = true;
		DTRACE(DB_CCT, ("CStdReader :: SyncMeterAddr : PLC nodes synchronized done.\n"));
	}
	else
	{
		ClearAllRid(ptStdReader, 2);
	}

	return true;
}

bool TCSyncMeterAddr(TStdReader* ptStdReader)
{	
	if (ptStdReader->m_fSyncAllMeter == true)	//�Ѿ�ͬ����·�ɵ��ز���ַ,�����ٸ���
		return true;

	StopReadMeter(ptStdReader);

	SetWorkMode(ptStdReader, WM_RDMTR);
	ReadMainNode(ptStdReader);

	SyncMeterAddr(ptStdReader);

	if(ptStdReader->m_fSyncAllMeter)
		TcRestartRouter(ptStdReader);//����������·��.

	DTRACE(DB_CCT, ("CTcStdReader :: SyncMeterAddr : PLC nodes synchronized done.\n"));	
	return true;
}

//����:	�����Զ���������״̬���£�Ŀǰ������Ϊ��׼���̣����ز�����Ҫ���Ļ����أ���������ڸ��ֲ���������
//����:	����б�Ҫ�жϵ�ǰ���е��Զ������ѧϰ���̵��򷵻ظ�������ԭ��
//		�����򷵻�0
// modify last time at 2012-10-30 by CPJ
int UpdateReader(TStdReader* ptStdReader)
{
	TTime now;
	BYTE bBuf[32];
	WORD wStudyTime;
	WORD wCurMin = 0;
	static DWORD dwStartClick = 0;
	static bool  fStartSync = false;

	GetCurTime(&now);
	wCurMin = (WORD )now.nHour*60 + now.nMinute;

	memset(bBuf, 0xff, sizeof(bBuf));
	
	///////////////////////////////////////////////////////////
	//1.������⣬���������֣�1.ģ��������⣬2.ֹͣ����2���Ӻ��Զ��ָ�����
	KeepAlive(ptStdReader); 

	if (AR_S_EXIT == ptStdReader->m_ptAutoReader->m_bState)	//·��ģ�黹û��ʼ����ȷ,û��ȷʶ���ز�ģ������
		return 0;	//�ŵ�KeepAlive()��,���л����ģ�������ϵ�

	///////////////////////////////////////////////////////////
	//2.����ʱ�μ��
	//if (IsInReadPeriod()) //�ڳ���ʱ����
	//{
	//	if (!m_fInReadPeriod)
	//	{
	//		DTRACE(DB_CCT, ("CStdReader::UpdateReader: sw into rd period.\r\n"));

	//		m_fInReadPeriod = true;

	//		//�ָ�����֮ǰ��Ҫ�ж��Ƿ��ѳɹ�ͬ�����ַ�����û��ͬ������ͬ����SyncMeterAddr��������ᷢ�ͻָ�����
	//		if(m_fSyncAllMeter)
	//			ResumeReadMeter();
	//	}
	//}
	//else
	//{
	//	if (m_fInReadPeriod)
	//	{
	//		DTRACE(DB_CCT, ("CStdReader::UpdateReader: sw out of rd period.\r\n"));

	//		if (StopReadMeter())
	//			m_fInReadPeriod = false;
	//	}
	//}

	//if (ptStdReader->m_fSyncAllMeter)
	{		 
		//AutoSchMtr();
	}

	///////////////////////////////////////////////////////////
	//3.ͬ������ַ��������Ӧ���ڳ���ʼ��֮������в���֮ǰ���У���ͬ���ز��������ظýӿ�
	TCSyncMeterAddr(ptStdReader);
	//SyncMeterAddr(ptStdReader);


	///////////////////////////////////////////////////////////
	//4.����2����û���յ�·�ɵĳ��������ˣ���Ҫ���ͻָ���������
	AutoResumeRdMtr(ptStdReader);


	///////////////////////////////////////////////////////////
	//5.�µ�һ�죬���³���
	if (IsDiffDay(&now, &ptStdReader->m_tmUdp) && wCurMin >= ptStdReader->m_ptStdPara->RdrPara.bDayFrzDelay)
	{
		DTRACE(DB_CCT, ("CStdReader::UpdateReader: restart router at new day.\n"));
		ptStdReader->m_fRestartRoute = false;
		StopReadMeter(ptStdReader);

		//���պ�·��������ز���������˹���ĸ�λ�ز�ģ���Դ������΢��Ӳ����λ��
		TcRestartNewDay(ptStdReader);
		ptStdReader->m_ptAutoReader->m_fStopRdMtr = false; //��Ϊ·��ģ��Ļָ�������ResumeReadMeter(),�����жϵ�m_fStopRdMtr==true,�Ͳ�ִ����Ӧ������,��������Ҫ�Ȱ�m_fStopRdMtr��Ϊfalse
		RestartRouter(ptStdReader);
		memset(ptStdReader->m_pbStudyFlag, 0x00, POINT_NUM/8+1);//�������ѧϰ�ɹ���־
	}

	/////////////////////////////////////////////////////////////
	//6.����ʽ·�����͵��ճ��������̣������̡����ԡ���˹����
	//  �Դ���·����˵�����ѧϰ�ͳ���״̬��ת�����п��ƣ���ͬ���͵�������ʵ��
	//  �Բ���Ҫ�߸ýӿڵ��ز��綫������������Ϊ�պ�����
	//if (!fStartSync)
	{
		//BYTE bVal = 0;
		//WORD wCompletePercent;
		//WORD wLeastCompletePercent;
		//ReadItemEx(BN23, PN0, 0x3026, &bVal);
		//ReadItemEx(BN17, PN0, 0x7053, (BYTE*)&wCompletePercent);
		//ReadItemEx(BN23, PN0, 0x302D, (BYTE*)&wLeastCompletePercent);
		//if (0 == wLeastCompletePercent)
		//	wLeastCompletePercent = 80;
		//if( wCompletePercent<wLeastCompletePercent &&bVal == 1 && wCurMin %10 == 0)//ͬһ���Ӽ�����
		//	CalcuCompletedPercent();	
		//DoUnReadID();
	}

	///////////////////////////////////////////////////////////
	//7.������
	//if (!m_fRestartRoute && CctIsMtrDate(PORT_CCT_PLC, now)) //�����ճ���ʱ�䵽��
	//{
	//	ReadMtrDate(); //��������ʽ·���綫���������ز�����������·�ɣ��Ա���ʽ·������Ϊ�ռ���
	//}

	if (wCurMin >= ptStdReader->m_ptStdPara->RdrPara.bDayFrzDelay)
		ptStdReader->m_tmUdp = now;	//���һ�θ���ʱ��

	///////////////////////////////////////////////////////////
	//8.��Ϣ������ز�������������������·�ɣ�����·��������
// 	static bool fInfoPlcChanged = false;
// 	DWORD  dwDelayClick = 0;
	if (GetInfo(INFO_PLC_PARA))
	{
		DTRACE(DB_CCT, ("CStdReader::UpdateReader: Meter para changed, please wait 40s.\n"));
		dwStartClick = GetClick();
		fStartSync = true; 

	}
	if (GetClick()-dwStartClick>=40 && fStartSync==true)
	{
		ptStdReader->m_fSyncAllMeter = false;
		fStartSync = false;
		//m_fIsWorking = false;
		//m_fIsNetOk = false;
 	}

	//DoCctInfo(ptStdReader);

	////////////////////////////////////////////
	//9.�ص㻧����
	//ReadVipDate(); 

	//10.�Զ�Уʱ
	//DoAdjTime();

	//
	//ReadInterV();

	//�Ƿ����������ϱ�
// 	BYTE bRptCmd;
// 	ReadItemEx(BN23, PN0, 0x3032, &bRptCmd); //Ϊ1�����ϱ�,Ϊ2��ͣ�ϱ�
// 	if(bRptCmd == 1)
// 	{
// 		AllowReport(ROUTER_RPT_START);  //Ϊ1�����ϱ�
// 		bRptCmd = 0;
// 		WriteItemEx(BN23,PN0,0x3032,&bRptCmd);
// 	}
// 	else if(bRptCmd ==2)
// 	{
// 		AllowReport(ROUTER_RPT_STOP);  //Ϊ2��ͣ�ϱ�
// 		bRptCmd = 0;
// 		WriteItemEx(BN23,PN0,0x3032,&bRptCmd);
// 	}
// 
// 	//�ж��Ƿ����F150
// 	if(m_fUpdF10)
// 	{
// 		if (SaveSearchPnToF10())
// 			m_fUpdF10 = false;
// 	}
// 
// 	if (GetInfo(INFO_PLC_RESET))
// 	{
// 		RestartRouter();
// 	}

	if (ptStdReader->m_fSyncAllMeter)
	{
		ptStdReader->m_ptAutoReader->m_bState = AR_S_LEARN_ALL;
	}
	return 1;
}

//����:�Զ�ѧϰ����(��Ҫ�����Զ�������������)
int DoLearnAll(TStdReader* ptStdReader)
{
	if (RxHandleFrm(ptStdReader, 1))	//�ô�����Զ�����֡,�������洦������,����trueʱ,֡��ɹ���,����û�б�Ĭ��֡������������
		//ExHandleFrm(); //����֡������

	return 0;
}
