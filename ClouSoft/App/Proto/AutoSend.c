/*********************************************************************************************************
 * Copyright (c) 2013,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AutoSend.c
 * ժ    Ҫ�����ļ���Ҫʵ��������������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2013��3��
*********************************************************************************************************/
#include "GbPro.h"
#include "SysDebug.h"
#include "DbGbAPI.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "ExcTask.h"
#include "ProAPI.h"
#include "MtrCtrl.h"
#include "LibDbStruct.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "DbHook.h"
#include "DbFmt.h"
#include "CommonTask.h"
#include "SysApi.h"
#include "FileMgr.h"
#include "FlashMgr.h"
#include "EsamCmd.h"
#include "AutoSend.h"
#include "AutoSendTask.h"
#include "DbSgAPI.h"
//////////////////////////////////////////////////////////////////
/*
�澯�����ϱ������֣��������ֵ��
���λ0�����λ255�ֱ��Ӧ
�������E2000001H~E20000FFH
�ı�����0���Σ�1�����Ρ�
*/
bool IsAlrReport(DWORD dwAlrID)
{
	BYTE bAlrMask[32];
	BYTE bModeTest;
	WORD n;
	if (dwAlrID<0xe200001 || dwAlrID>0xe200004e)
		return false;

	n = dwAlrID - 0xe2000001;

	ReadItemEx(BN0, PN0, 0x8033, bAlrMask);

	if (bAlrMask[n/8] & (1<<n%8))   //1������
	{
		// 		DTRACE(DB_TASK, ("%s : alr 0x%04x report enable.\r\n", __FUNCTION__, dwAlrID));
		return true;
	}
	else
	{
		// 		DTRACE(DB_TASK, ("%s : alr 0x%04x report disable.\r\n", __FUNCTION__, dwAlrID));
		return false;
	}
}

int MakeClass3Frm(struct TPro* pPro, BYTE bEc)
{
	WORD wTxPtr;
	BYTE bCmd, bAFN;
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	//int iLen;	

	bAFN = SG_AFUN_ASKCLASS3;
	//FnPnToBytes(1, 0, pbTx);
	pbTx += 4;
	*pbTx++ = bEc; //��ʼָ��
	*pbTx++ = bEc+1; //����ָ��
	wTxPtr = SG_LOC_DA1 + 6;

	bCmd = SG_LFUN_ASKN2DATA;
	bCmd |= SG_CTL_FCV;

	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;
	SgMakeMasterReqFrm(pPro, true, bCmd, bAFN, wTxPtr);

	wTxPtr += 2;	//У��λ�ͽ�����
	return wTxPtr;
}

//�������澯�ϱ�
bool DoRptAlarm(struct TPro* pPro)
{
	//����дָ�벢��֡
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = &pSgPro->pbTxBuf[SG_LOC_DA1];
	int iRet;
	WORD wNewAlarmNo, wLastAlarmNo, wAlarmNum;
	WORD i, j = 0;
	BYTE bTmpBuf[280];
	DWORD dwClcik, dwLastClick;

	//����������ܸ澯��Ϊ6����ôwAlarmNumΪ6���澯ָ��Ϊ0~5���������6�ʶ����ˣ��¸澯ָ��ָ��6
	ExFlashRdData(FADDR_ALRREC+2, NULL, (BYTE* )&wNewAlarmNo, 2);			//ȡ�µĸ澯ָ�룬����wNewAlarmNo
	ReadItemEx(BN24, PN0, 0x5030, (BYTE* )&wLastAlarmNo);					//ȡ�ϴζ��澯ָ�룬����wLastAlarmNo
	ExFlashRdData(FADDR_ALRREC, NULL, (BYTE* )&wAlarmNum, 2);				//ȡ�ܹ�����ĸ澯��������wAlarmNum

	if (wNewAlarmNo > wAlarmNum)									//����µĸ澯ָ�볬���ܸ澯��������false
		return false;

	if ((wAlarmNum>500)||(wNewAlarmNo>500))
		return false;

	while (wNewAlarmNo != wLastAlarmNo)
	{
		pSgPro->fRptState = true;									//�澯������������״̬

		ReadOneAlr(FADDR_ALRREC, bTmpBuf, wLastAlarmNo);			//����wLastAlarmNo��Ӧ�ļ�¼��bTmpBuf

		wLastAlarmNo += 1;
		if ((wLastAlarmNo >= 500))									//�������澯��500���ص���һ���澯
			wLastAlarmNo = 0;
		WriteItemEx(BN24, PN0, 0x5030, (BYTE* )&wLastAlarmNo);		//������ζ��澯ָ��

		if (!IsAlrReport(ByteToDWORD(bTmpBuf+3, 4)))				//�ø澯������ΪΪ0�����ϱ�������������һ��
		{
			pSgPro->fRptState = false;								//�澯������������״̬������false
			continue;
		}
		
   		pbTx -= j;
		PnToBytes(ByteToWord(bTmpBuf+1), bTmpBuf+1);
		j = 2+4+GetFmtARDLen(ByteToDWORD(bTmpBuf+3, 4));
		memcpy(pbTx, bTmpBuf+1, j);
		pbTx += j;

		i = 3;
REPORT_ALARM_FRM:
		dwLastClick = GetClick();
		pSgPro->pbTxBuf[SG_LOC_SEQ] = (pSgPro->pbTxBuf[SG_LOC_SEQ]++) & SG_SEQ_SEQ | SG_SEQ_CON | SG_SEQ_FIN | SG_SEQ_FIR;//��֡��ȷ��
		if (!((ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) >= 0xE2000001) && (ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) <= 0xE200004E)))
		{
			DTRACE(DB_FAPROTO, ("DoRptAlarm auotosend id=%x wrong, exit!\r\n", ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4)));
			pSgPro->fRptState = false;									//�澯������������״̬������false
			return false;
		}

		SgMakeTxFrm(pPro, true, SG_LFUN_ASKNOREPLY, SG_AFUN_ASKALARM, (WORD)(pbTx - pSgPro->pbTxBuf));
        SetLedCtrlMode(LED_ALARM, LED_MODE_OFF);   //�澯��        
		pSgPro->fRptState = false;									//�澯������������״̬������false
		i -= 1;

		do{
			iRet = pPro->pfnRcvFrm(pPro);
			if (iRet > 0)
			{
				if (pSgPro->pbRxBuf[SG_LOC_AFN]==SG_AFUN_CONFIRM)
				{
					i = 3;
					break;
				}
				else
					dwClcik = GetClick();
			}
			else
				dwClcik = GetClick();

		}while (dwClcik-dwLastClick < 20);
		
		if (i==0)													//�ϱ����ζ�û�յ���ȷ��֡������һ�´˸澯���ϱ���һ���澯֡
		{
			WriteAlrRec(FADDR_RPTFAILREC, bTmpBuf+1, j);
			break;													//һ�ν�������һ���澯��������ʧ�ܴ���ʱ��
		}
		else if (i<3)												//�澯�ϱ�����3�Σ������ϱ�
		{
			pSgPro->fRptState = true;								//�澯������������״̬
			goto REPORT_ALARM_FRM;
		}
	}

	return true;
}

static BYTE g_bTaskNo = 0;
static BYTE g_bFwdTaskNo = 0;
//��ע����������ͨ���̵߳��ã�
//		1������2�����ݣ�ֱ�Ӳ�ѯ����⣬��ͨ���̵߳��������ⲻ��
//		2������1�����ݵ������������񣬳�������Ӧ����MtrRdThread()���Զ���ɣ�������ÿ��������ĵ�ǰ������
//		   ��������ִ�е�ʱ�򣬰�ÿ������������ݵ��뼴��
bool GbAutoSend(struct TPro* pPro)
{
	static DWORD dwLastClick = 0;
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE bCommonTaskNum = 0, bFwdTaskNum = 0;

	//����ܿ���,�ж��Ƿ������ϱ�
	/*if ( !IsAdmitReport() )
		return false;*/

	if (GetClick()-dwLastClick < 3)	 //���������ⲿFLASH���������Ƚϴ󣬲�ҪƵ��ִ��
		return true;

	if (pGbPro->wRxStep!=0 && GetClick()-pGbPro->dwRxClick<3)	//֮ǰ�Ľ���֡�����գ������������
		return true;

	//DoRptAlr(pPro);
	DoRptAlarm(pPro);

	if (GetInfo(INFO_TASK_INIT))
		UpdTaskInfo();

	ReadItemEx(BN0, POINT0, 0x0b10, &bCommonTaskNum);	//��ǰ����ͨ��������
	ReadItemEx(BN0, POINT0, 0x0b20, &bFwdTaskNum);		//��ǰ���м���������
	if (bCommonTaskNum == 0 || bFwdTaskNum == 0)
		DTRACE(DB_TASK, ("GbAutoSend bCommonTaskNum=%d, bFwdTaskNum=%d\n", bCommonTaskNum, bFwdTaskNum));

	if (bCommonTaskNum != 0)
	{
		while (g_bTaskNo < MAX_COMMON_TASK)//��ͨ������
		{
			//if (!IsTaskExist(MAX_COMMON_TASK, i))
			//	continue;
				
			if (IsTaskValid(COMMON_TASK_TYPE, g_bTaskNo))
			{
				DoRptTask(COMMON_TASK_TYPE, g_bTaskNo, pPro);

				g_bTaskNo++;
				if (g_bTaskNo >= MAX_COMMON_TASK)
					g_bTaskNo=0;
				break;
			}

			g_bTaskNo++;
			if (g_bTaskNo >= MAX_COMMON_TASK)
			{
				g_bTaskNo=0;
				break;
			}
		}
	}

	
	if (bFwdTaskNum != 0)
	{
		while (g_bFwdTaskNo < MAX_FWD_TASK)//�м�������
		{
			//if (!IsTaskExist(MAX_COMMON_TASK, i))
			//	continue;
					
			if (IsTaskValid(FWD_TASK_TYPE, g_bFwdTaskNo))
			{
				DoRptTask(FWD_TASK_TYPE, g_bFwdTaskNo, pPro);

				g_bFwdTaskNo++;
				if (g_bFwdTaskNo >= MAX_FWD_TASK)
					g_bFwdTaskNo=0;
				break;
			}

			g_bFwdTaskNo++;
			if (g_bFwdTaskNo >= MAX_FWD_TASK)
			{
				g_bFwdTaskNo=0;
				break;
			}
		}
	}
	
	dwLastClick = GetClick();
	return true;
}
