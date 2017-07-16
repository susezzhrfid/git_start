/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֵ�ѹ�����Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2009��1��
 *********************************************************************************************************/
//#include "bios.h"
#include "CctIf.h"
#include "AutoReader.h"
#include "StdReader.h"
#include "Trace.h"
#include "SysDebug.h"
#include "CctAPI.h"
#include "FaCfg.h"
#include "DrvCfg.h"
#include "Comm.h"


//��ʼ���ɹ���־
bool g_fAutoReaderInitOk = false;

bool IsCctOK(TStdReader* ptStdReader)
{
	if (ptStdReader->m_fSyncAllMeter)
	{
		return true;
	}
}

//����:�ú���ʵ�ֱ�Ƽ�ز���ʱ645֡͸������
//����:�ɹ�����1,ʧ�ܷ���0
int CctDirectTransmit645Cmd(BYTE* pbCmdBuf, BYTE bCmdLen, BYTE* pbRet, BYTE* pbRetLen, BYTE bTimeOut)
{
	int iRet = 0;
	if(!g_fAutoReaderInitOk || NULL == g_ptStdReader)
		return -1;
	
	iRet = CctTransmit645Cmd(pbCmdBuf, bCmdLen, pbRet, pbRetLen, bTimeOut);
	return iRet;
}

//���������ù̶�֡��ʽ��ȡ�ز�ģ��ĳ��̴���
bool CctReadFactoryCode()
{
	DWORD dwRet = 0;
	BYTE  bLink = AR_LNK_STD_ES;
	BYTE  bTxBuf[] = {0x68, 0x0F, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x45, 0x16};
	BYTE  bRxBuf[128] = {0};
	WORD  wPos = 0;
	BYTE i;
	bool fOpenOK;

	//NOTE ��ȡ���̴������̣�
	//1.��λ�ز�ģ���Դ��ĳЩģ���綫���ڸ�λ·�ɺ�������ϱ���ǰ����ģʽ����Ϣ��
	
	//2.����5s������ĳЩģ�������ϱ�����Ϣ��
	//3.ģ������г�������δ�������ʿ�����ֹͣ·�ɵȲ�����ֱ�Ӷ����̴��룬�����θ�ģ��Ĳ���

	TCommPara CommPara;
	CommPara.wPort = COMM_METER3;
	CommPara.dwBaudRate = CBR_9600;	
	CommPara.bParity =  EVENPARITY;
	CommPara.bByteSize = 8;
	CommPara.bStopBits = ONESTOPBIT;

	fOpenOK = CctProOpenComm(&CommPara);

	if(!fOpenOK)
	{
		DTRACE(DB_CRITICAL, ("CctReadFactoryCode :: Open Com fail.\r\n"));
		return false;
	}

	DTRACE(DB_CCTTXFRM, ("CctReadFactoryCode :: Open Com%d ok.\r\n",COMM_METER3));

	for(i=0; i<3; i++)
	{
                CommRead(CommPara.wPort, NULL, 0, 300); //�����һ�´���
		CommWrite(CommPara.wPort, bTxBuf, sizeof(bTxBuf), 1000);

		TraceBuf(DB_CCTTXFRM, "CctReadFactoryCode -->", bTxBuf, sizeof(bTxBuf));

		Sleep(500);
		memset(bRxBuf, 0x00, sizeof(bRxBuf));
				
		dwRet = CommRead(CommPara.wPort, bRxBuf, sizeof(bRxBuf), 300);
                //return true;
                if (dwRet>0)
		{	
			TraceBuf(DB_CCTRXFRM, "CctReadFactoryCode <--", bRxBuf, dwRet);
		
			//���ģ�����ǰ����0x55��Ŀǰ�ݲ����Ƿ�֡�����
			for(wPos = 0; wPos<sizeof(bRxBuf); wPos++)
			{
				if (bRxBuf[wPos] == 0x68)
					break;
			}

			if(wPos>=sizeof(bRxBuf))
				continue;

			if (bRxBuf[FRM_AFN+wPos] == AFN_QRYDATA)
			{
				if((bRxBuf[FRM_AFN+1+wPos])==0x01 && (bRxBuf[FRM_AFN+2+wPos])==0x00)
				{
					DTRACE(DB_CCT, ("CctReadFactoryCode :: Read Factory Code: %c%c .\n",bRxBuf[FRM_AFN+3+wPos],bRxBuf[FRM_AFN+4+wPos]));
					DTRACE(DB_CCT, ("CctReadFactoryCode :: Read Cct mode Version: %02x%02x Time:%02x-%02x-%02x .\n",bRxBuf[FRM_AFN+11+wPos], bRxBuf[FRM_AFN+10+wPos], bRxBuf[FRM_AFN+9+wPos],bRxBuf[FRM_AFN+8+wPos],bRxBuf[FRM_AFN+7+wPos]));
					if (bRxBuf[FRM_AFN+3+wPos]=='X' &&  bRxBuf[FRM_AFN+4+wPos]=='C')	
					{
						if (bRxBuf[FRM_AFN+5+wPos]==0x32 && bRxBuf[FRM_AFN+6+wPos]==0x32)//������
							bLink = AR_LNK_STD_XC_A;
						else
							bLink = AR_LNK_STD_XC;
					}
					else  if (bRxBuf[FRM_AFN+3+wPos]=='S' &&  bRxBuf[FRM_AFN+4+wPos]=='E')						
						bLink = AR_LNK_STD_ES;							
					else  if (bRxBuf[FRM_AFN+3+wPos]=='C' &&  bRxBuf[FRM_AFN+4+wPos]=='T')						
						bLink = AR_LNK_STD_TC;                       
					else if (bRxBuf[FRM_AFN+3+wPos]=='1' && bRxBuf[FRM_AFN+4+wPos]=='0')
						bLink = AR_LNK_STD_RC;                        
					else if (bRxBuf[FRM_AFN+3+wPos]=='F' && bRxBuf[FRM_AFN+4+wPos]=='C')
						bLink = AR_LNK_STD_FC;
					else if(bRxBuf[FRM_AFN+3+wPos]=='i' && bRxBuf[FRM_AFN+4+wPos]=='m')						
						bLink = AR_LNK_STD_MI;
					else if(bRxBuf[FRM_AFN+3+wPos]=='I' && bRxBuf[FRM_AFN+4+wPos]=='M')
						bLink = AR_LNK_STD_MI;
					else if(bRxBuf[FRM_AFN+3+wPos]=='S' && bRxBuf[FRM_AFN+4+wPos]=='R')						
						bLink = AR_LNK_STD_SR;
					else if(bRxBuf[FRM_AFN+3+wPos]=='L' && bRxBuf[FRM_AFN+4+wPos]=='M')	
						bLink = AR_LNK_STD_LM;
					else if(bRxBuf[FRM_AFN+3+wPos]=='H' && bRxBuf[FRM_AFN+4+wPos]=='L')	
						bLink = AR_LNK_STD_LM_N;
					else if(bRxBuf[FRM_AFN+3+wPos]=='R' && bRxBuf[FRM_AFN+4+wPos]=='L')						
						bLink = AR_LNK_STD_SR;
					else if(bRxBuf[FRM_AFN+3+wPos]=='J' && bRxBuf[FRM_AFN+4+wPos]=='X')						
						bLink = AR_LNK_STD_XC;	
					else if(bRxBuf[FRM_AFN+3+wPos]=='G' && bRxBuf[FRM_AFN+4+wPos]=='D')	
						bLink = AR_LNK_STD_GD;
					else if(bRxBuf[FRM_AFN+3+wPos]=='L' && bRxBuf[FRM_AFN+4+wPos]=='K')	
						bLink = AR_LNK_STD_LK;
					else if(bRxBuf[FRM_AFN+3+wPos]=='N' && bRxBuf[FRM_AFN+4+wPos]=='T')
						bLink =AR_LNK_STD_NT;
					else if(bRxBuf[FRM_AFN+3+wPos]=='H' && bRxBuf[FRM_AFN+4+wPos]=='R')
						bLink =AR_LNK_STD_HR;
					else if(bRxBuf[FRM_AFN+3+wPos]=='M' && bRxBuf[FRM_AFN+4+wPos]=='X')
						bLink =AR_LNK_STD_MX;
					else if(bRxBuf[FRM_AFN+3+wPos]=='m' && bRxBuf[FRM_AFN+4+wPos]=='x')
						bLink =AR_LNK_STD_MX;
					else if(bRxBuf[FRM_AFN+3+wPos]=='C' &&  bRxBuf[FRM_AFN+4+wPos]=='Z')
						bLink =AR_LNK_STD_ZC;
					else if(bRxBuf[FRM_AFN+3+wPos]=='7'  && bRxBuf[FRM_AFN+4+wPos]=='3')//����������ר��(ʶ����˹��ģ��)
						bLink =AR_LNK_STD_NT_HB;
					else
						continue;
					//WriteItemEx(BN15, PN0, 0x5006, &bLink);	//�ز�ģ������
					
					CommClose(CommPara.wPort);
					return true;
				}
				else
				{
					DTRACE(DB_CRITICAL, ("CctReadFactoryCode::ReadVersion: fail-1 !!!\r\n"));
				}
			}
			else 
			{  
				DTRACE(DB_CRITICAL, ("CctReadFactoryCode::ReadVersion fail-2 !!!\r\n"));			
			}

		}
	}
	CommClose(CommPara.wPort);
	return false;
	
}

//��ʼ���ز���
bool InitCctPlc()
{
	//CPlcRouter* pPlcRouter;
	BYTE bBuf[10] = {0,};

	if(g_fAutoReaderInitOk)
		return true;
	
	if(!CctReadFactoryCode()) //��ȡ���̴���ʧ��ʱ�迼���Ƿ�Ϊ��Ѷ������ʱ�쳣�����
	{
// 		char szFileName[128] = {0};
// 		int iRet = ReadItemEx(BN23, PN0, 0x3022, (BYTE*)szFileName);
// 		if(iRet>=2 && (szFileName[0]=='F'||szFileName[0]=='f') && (szFileName[1]=='C'||szFileName[1]=='c')) //��Ѷ������ʧ�ܺ�Ĳ���������
// 		{
// 			DTRACE(DB_CRITICAL,("Start Update FC module !!! \n"));
// 			bBuf[0] = AR_LNK_STD_FC;
// 			WriteItemEx(BN15, PN0, 0x5006, bBuf);	//�ز�ģ������
// 		}
// 		else
		return false;
	}
	
	//ReadItemEx(BN15, PN0, 0x5006, bBuf);	//�ز�ģ������

	if (bBuf[0] == AR_LNK_STD_TC) //��׼��698.42����������
	{
		//LoadStdPara(AR_LNK_STD_TC, &g_tStdPara);
		
		//if (!CctAutoReaderInit(&g_tAutoReader, &g_tStdPara.RdrPara))
		//	return false;
		//InitStdReader(&g_tAutoReader, &g_tStdPara);
	}
	
	g_fAutoReaderInitOk = true;
}

//����:С������ӿڳ�ʼ��
bool InitCct()
{
	g_fAutoReaderInitOk = false; 

	InitCctPlc();
	DTRACE(DB_CCT, ("CctAPI: InitCct init OK !.\n"));
	return true;
}

void GetCctAutoReader()
{
	BYTE bReadCnt = 0;
	if (!g_fAutoReaderInitOk)
	{
		while(1)
		{
			if(bReadCnt>10)
			{
#ifndef SYS_WIN
				PlcReset();//δ����
#endif
				bReadCnt = 0;
				Sleep(5000);
			}

			if(!InitCctPlc())
			{
				Sleep(5000);
				bReadCnt++;
			}
			else
				break;
		}
		DTRACE(DB_CRITICAL, ("NewMeterThread : AutoReader init OK ,thread  created OK.\n"));
	}
}

//����:	�̶߳���
TThreadRet AutoReaderPlcThread(void* pvArg)
{
	//����������
	BYTE bChangeCnt = 0;

	while(1)
	{
		if(bChangeCnt>10) //�ݶ�Ϊ10�ΰɣ���ֹ���ֳ��Ȳ��ģ��������࣬����һ�±ȽϺ�
		{
			SetInfo(INFO_APP_RST);
			return THREAD_RET_ERR;
		}

		GetCctAutoReader();

		if(!g_fAutoReaderInitOk) //���ɹ�,��������
		{
			DTRACE(DB_CRITICAL, ("AutoReaderPlcThread: GetCctAutoReader fail!.\r\n"));
			SetInfo(INFO_APP_RST);
			return THREAD_RET_ERR;
		}

		//��ʼ���ڴ��������
		CctAutoReaderRunThread();
		
		g_fAutoReaderInitOk = false;

		DTRACE(DB_CRITICAL, ("AutoReaderPlcThread: AutoReadThread exit!.\r\n"));
		Sleep(5000);
		bChangeCnt++;
	}
	return 	THREAD_RET_OK; 
}

//����:	�½�С�������߳�
void NewCctThread()
{
	//�ز�
	NewThread("Cct", AutoReaderPlcThread, NULL, 640, THREAD_PRIORITY_LOWEST);
}