/*********************************************************************************************************
* Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ�����: CctIf.c
* ժ    Ҫ: ���ļ���Ҫʵ���ز�Э��ӿ�
* ��ǰ�汾: 1.0
* ��    ��: ������
* �������: 2014��8��
* ��	ע:	
*			1>TCS\TCC\TCM ֮���ϵ S������C�ز�ͨ��M�������ز�ͨ��;
*			2>��Ӧ�ൺ���Ź�ԼVer1.11 2008/10/03
*********************************************************************************************************/
#include "CctIf.h"
//#include "InUart.h"
#include "stdio.h"
#include "Trace.h"
#include "SysDebug.h"
#include "FaCfg.h"
#include "MeterPro.h"
#include "DrvCfg.h"

bool CctProOpenComm(TCommPara* pCommPara)
{
	//pComm->SetTimeouts(1000); //����ȱʡ��ʱ��1S,�Է���ĳЩ��Э��ĵ��ܶ���

	bool fCommOpen = false;
	TCommPara CommPara;
	if ( CommIsOpen(pCommPara->wPort) )
	{
		if (CommGetPara(pCommPara->wPort, &CommPara)) //�����ڲ����м�⴮���Ƿ��	
		{
			fCommOpen = true;
			if ( pCommPara->wPort==CommPara.wPort
				&& pCommPara->dwBaudRate==CommPara.dwBaudRate
				&& pCommPara->bByteSize==CommPara.bByteSize
				&& pCommPara->bParity==CommPara.bParity
				&& pCommPara->bStopBits==CommPara.bStopBits )
			{
				return true;	//���ڲ�������ͬ������²��������ٴ򿪴���			
			}
		}
	}

	if ( fCommOpen ) 
	{
		if ((pCommPara->wPort!=COMM_GPRS) && (pCommPara->wPort!=COMM_LOCAL))//�������GPRS�����
		{
			if ( !CommClose(pCommPara->wPort) )
			{
				DTRACE(DB_METER, ("CctPro::OpenComm : fail to close COM=%d.\r\n", CommPara.wPort));
				return false;
			}
		}
	}

	if ( CommOpen(pCommPara->wPort, pCommPara->dwBaudRate, pCommPara->bByteSize, pCommPara->bStopBits, pCommPara->bParity) )
	{
		return true;
	}
	else
	{
		DTRACE(DB_METER, ("CctPro::OpenComm : fail to open COM=%d.\r\n", pCommPara->wPort));
		return false;
	}
}

//����:֡��ʽ�е�DT��FNת��
//����:��ȷ�򷵻�FN,���򷵻�0
WORD DtToFn(BYTE* pbDt)
{
	WORD i;
	const BYTE bDtToFn[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	for (i=0; i<8; i++)
	{
		if (pbDt[0] == bDtToFn[i])
		{
			return (WORD )pbDt[1]*8 + i + 1;
		}
	}

	return 0;
}

//����:FN��֡��ʽ��DT��ת��
void nToDt(WORD wFn, BYTE* pbDt)
{
	pbDt[0] = 0x1<<((wFn-1)%8);
	pbDt[1] = (BYTE)((wFn-1)/8);
}

//����:FN��֡��ʽ��DT��ת��
void FnToDt(WORD wFn, BYTE* pbDt)
{
	pbDt[0] = 0x1<<((wFn-1)%8);
	pbDt[1] = (BYTE)((wFn-1)/8);
}

//����:ȡ���б�����Ϣ��R
void GetUpInf(BYTE* pbBuf, TUpInf* pUpInf)
{
	pUpInf->bFwdDepth = pbBuf[0] >> 4;			//�м����
	pUpInf->bModule = (pbBuf[0]>>2) & 0x01;		//ͨ��ģ���ʶ��0��ʾ�Լ�������ͨ��ģ�������1��ʾ���ز����ͨ��ģ�����
	pUpInf->bRt = pbBuf[0] & 0x01;				//·�ɱ�ʶ��D0=0��ʾͨ��ģ���·�ɻ�����·��ģʽ��D0=1��ʾͨ��ģ�鲻��·�ɻ�������·ģʽ��
	pUpInf->bCn = pbBuf[1] & 0x0f;				//�ŵ���ʶ��ȡֵ0~15��0��ʾ�����ŵ���1~15���α�ʾ��1~15�ŵ���
	pUpInf->bCnChar = pbBuf[2] >> 4;			//���ͨ������
	pUpInf->bPhase = pbBuf[2] & 0x0f;			//ʵ�����߱�ʶ��ʵ��ӽڵ��߼����ŵ����ڵ�Դ���0Ϊ��ȷ����1~3���α�ʾ���Ϊ��1�ࡢ��2�ࡢ��3�ࡣ
	pUpInf->bAnsSigQlty= pbBuf[3] >> 4;			//ĩ��Ӧ���ź�Ʒ��
	pUpInf->bCmdSigQlty = pbBuf[3] & 0x0f;		//ĩ�������ź�Ʒ��
	pUpInf->bRptEvtFlg = pbBuf[4]&0x01;         //�ϱ��¼���ʶ
	pUpInf->bSN = pbBuf[5];                     //�������к�
}

//����:��֡Ӧ��������
WORD MakeAppDataField(BYTE bAfn, WORD wFn, BYTE* pbData, WORD wLen, BYTE* pbDataField)
{     
	pbDataField[0] = bAfn; 
	FnToDt(wFn, &pbDataField[1]);
	memcpy(pbDataField+3, pbData, wLen);

	return wLen+3;
}

WORD CctProMakeFrm(WORD wFrmLenBytes,BYTE* pbTxBuf, BYTE bAfn, WORD wFn, BYTE* pbData, WORD wDataLen, BYTE bAddrLen, BYTE bMode, BYTE bPRM, BYTE bCn, BYTE bRcReserved)
{   
	WORD wLen;
	BYTE bLenBytes = wFrmLenBytes-1;	//֡�����ֽ���	
	WORD wAppDataFieldLen = 0;
	WORD i;

	//��֡Ӧ��������
	BYTE bCS = 0; 
	pbTxBuf[0] = 0x68;

	if(pbData==NULL && wDataLen!=0)
	{
		DTRACE(DB_CRITICAL, ("CStdReader::MakeFrm: fail, duo to pbData==NULL and wDataLen!=0!!!\r\n"));
		return 0;
	}

	wAppDataFieldLen = MakeAppDataField(bAfn, wFn, pbData, wDataLen, &pbTxBuf[9+bAddrLen+bLenBytes]); 

	wLen = 6 + bAddrLen + wAppDataFieldLen + 5 + bLenBytes;
	pbTxBuf[1] = (BYTE )wLen;	//����L
	if (bLenBytes == 1)
		pbTxBuf[2] =(BYTE )(wLen>>8)  ;	//����L
	pbTxBuf[2 + bLenBytes] = (BYTE) (bPRM*(0x01<<6)  + bMode); //������C

#ifdef EN_NEW_376_2
	//376.2���ӵ�֡���
	BYTE bSendSN = 0;
	if(IsRouterSN(bAfn))
	{
		bSendSN = m_bRxSN;
	}
	else
	{
		bSendSN = m_bTxSN;
		m_bTxSN++;
	}
	pbTxBuf[8 + bLenBytes] = bSendSN;
#endif

	bCS = 0;
	for (i=2+bLenBytes; i<2+1+6+bAddrLen+wAppDataFieldLen+bLenBytes; i++)
		bCS += pbTxBuf[i];

	pbTxBuf[wAppDataFieldLen+bAddrLen+9+bLenBytes] = bCS;
	pbTxBuf[wAppDataFieldLen+bAddrLen+10+bLenBytes] = 0x16;

	return 6 + bAddrLen + wAppDataFieldLen + 5 + bLenBytes;
}

//������ͨ���������ز�·�ɷ�����
//���أ��ѷ��ͳ���
DWORD CctSend(TCommPara* pCommPara, BYTE *p, DWORD dwLen)
{
	char szBuf[48];
	if (!CctProOpenComm(pCommPara))
	{
		DTRACE(DB_CCT, ("Cct :: Send : com is closed.\n"));
		return 0;
	}

	if (dwLen>0 && IsDebugOn(DB_CCTTXFRM))
	{
		sprintf(szBuf, "Cct --> ");
		TraceBuf(DB_CCTTXFRM, szBuf, p, dwLen);
	}

	return CommWrite(pCommPara->wPort, p, dwLen, COM_TIMEOUT);
}

//�������ز�·�ɵ�����
//���أ����յ��ĳ���
DWORD CctReceive(TCommPara* pCommPara, BYTE* p, DWORD wLen)
{
	DWORD dwRet = 0;
	DWORD dwTimeOut = 200;
	char szBuf[48];
	dwRet = CommRead(pCommPara->wPort, p, wLen, dwTimeOut);

	if (dwRet>0 && IsDebugOn(DB_CCTRXFRM))
	{
		sprintf(szBuf, "Cct <-- ");
		TraceBuf(DB_CCTRXFRM, szBuf, p, dwRet);
	}
	
	return dwRet;
}

//����:	����һ�����ݿ�,�ж��Ƿ���յ�һ��������ͨ��֡
//����:	�����Ѿ�ɨ������ֽ���,����յ�һ��������ͨ��֡�򷵻�����,���򷵻ظ���
//pbBlock        �յ�����
//nLen           �յ����ĳ���
//ptCctCommPara  376.2�ṹ�������м����
//wFrmLenBytes   376.2�ṹ�ĳ��Ƚ���
//pbCctRxBuf     ���������376.2֡
//pdwLastRxClick �����������376.2֡��ʱ��
int CctRcvFrame(BYTE* pbBlock, int nLen, TCctCommPara* ptCctCommPara, WORD wFrmLenBytes, BYTE* pbCctRxBuf, DWORD* pdwLastRxClick)
{
	int iRxLen = 0;
	BYTE *bHead = pbBlock;
	int i;
	BYTE b;

	ptCctCommPara->m_nRxStep = 0;    //��֡��������ѭ���������е����ݲ���ɾ������ÿ�ν������ɴ�ͷ��ʼ������ֱ���ҵ�һ��������֡�Ż´����m_bCctRxBuf
	
	ptCctCommPara->m_fRxComlpete = false;
	for (i=0; i<nLen; i++)
	{
		b = *pbBlock++;
		switch (ptCctCommPara->m_nRxStep) 
		{
		case 0:
			if (b == 0x68)
			{
				pbCctRxBuf[0] = 0x68;
				ptCctCommPara->m_wRxPtr = 1;
				ptCctCommPara->m_nRxCnt = wFrmLenBytes;
				ptCctCommPara->m_nRxStep = 1;
			}
			break;
		case 1:
			pbCctRxBuf[ptCctCommPara->m_wRxPtr++] = b;
			ptCctCommPara->m_nRxCnt--;

			if (ptCctCommPara->m_nRxCnt == 0)
			{
				if (wFrmLenBytes == 1)
					ptCctCommPara->m_wRxDataLen = pbCctRxBuf[1];
				else
					ptCctCommPara->m_wRxDataLen = pbCctRxBuf[1] + ((WORD)pbCctRxBuf[2]<<8);

				//StdRdr <--  4E 23 00 08 F9 FF FF FF 00 00 02 15 68 57 01 00 00 00 00 68 91 09 34 33 39 38 33 33 3B 43 45 C3 16 B1 16 68 13 00 81 00 00 40 00 00 00 00 01 00 FF FF 00 00 C0 16
				if(ptCctCommPara->m_wRxDataLen>(WORD)nLen   //�����376.2֡�����ѳ�����������ݳ���
					|| ptCctCommPara->m_wRxDataLen+i-2>nLen  //�����֡βλ���ѳ�����������ݳ���
					|| bHead[ptCctCommPara->m_wRxDataLen+i-3]!=0x16)  //֡����������0x16
				{
					//��������ǽ��ܲ�ȫ��Ƿ��ı��ģ���m_nRxStep��Ϊ0���൱�ڽ�m_bCctRxBuf�е�������գ�
					//ͬʱ����RxHandleFrm��ɾ��ɨ��������ݣ��Ⱥ�����������һ�𿽱��������������½��н���    add by CPJ at 2012-10-11
					ptCctCommPara->m_nRxStep = 0;     
					break;
				}

				ptCctCommPara->m_nRxCnt = ptCctCommPara->m_wRxDataLen -1 - wFrmLenBytes;
				ptCctCommPara->m_nRxStep = 2;
			}
			break;
		case 2:
			pbCctRxBuf[ptCctCommPara->m_wRxPtr++] = b;
			ptCctCommPara->m_nRxCnt--;

			if (ptCctCommPara->m_nRxCnt == 0)
			{
				ptCctCommPara->m_nRxStep = 0;
				iRxLen = ptCctCommPara->m_wRxDataLen - 3 - wFrmLenBytes; //������������ܻ�С��0����ӱ��� add by CPJ at 2012-16-19

				if (iRxLen>0 && ptCctCommPara->m_wRxPtr>=2 && 
					pbCctRxBuf[ptCctCommPara->m_wRxPtr - 2]==CheckSum(pbCctRxBuf + 1 + wFrmLenBytes, (WORD)iRxLen)
					&& pbCctRxBuf[ptCctCommPara->m_wRxPtr-1]==0x16)
				{
					ptCctCommPara->m_fRxComlpete = true;
					*pdwLastRxClick = GetClick();
					return i+1;
				}
			}
			break;
		default:
			ptCctCommPara->m_nRxStep = 0;
			break;
		}
	}

	return -nLen;
}