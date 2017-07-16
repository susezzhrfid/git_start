/*********************************************************************************************************
 * Copyright (c) 2014,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AutoSend.c
 * ժ    Ҫ�����ļ���Ҫʵ��������������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2014��9��
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
#include "AutoSendTask.h"
#include "DbSgAPI.h"
//////////////////////////////////////////////////////////////////
//TRptCtrl g_ComRptCtrl[MAX_COMMON_TASK];	//��ͨ����
//TRptCtrl g_FwdCtrl[MAX_FWD_TASK];		//�м�����

void TraceSecsToTime(char *pStr, DWORD dwSecs)
{
	TTime t;
	SecondsToTime(dwSecs, &t);
	DTRACE(DB_TASK, ("%s",pStr));
	DTRACE(DB_TASK, (" %d-%02d-%02d %02d:%02d:%02d\r\n",t.nYear,t.nMonth,t.nDay,t.nHour,t.nMinute,t.nSecond));
}

bool IsTaskExist(BYTE bTaskType, BYTE bTaskNo)
{
	BYTE bCfgFlag[64];
	memset(bCfgFlag, 0, sizeof(bCfgFlag));
	ReadItemEx(BN0, PN0, 0x0b40, bCfgFlag);

	if (bTaskType==COMMON_TASK_TYPE)
		if (bCfgFlag[bTaskNo/8]&(1<<(bTaskNo%8)))
			return true;

	if (bTaskType==FWD_TASK_TYPE)
		if (bCfgFlag[bTaskNo/8+32]&(1<<(bTaskNo%8)))
			return true;
			
	return false;
}

//���������㵥����ͨ����ÿ�ʼ�¼�ĳ���
//������@bTaskNo��ͨ�����
//���أ�ÿ�ʼ�¼�ĳ���
int GetComTaskPerDataLen(BYTE bTaskNo)
{
	BYTE wPn;
	WORD wID = 0x0b11;
	DWORD dwLen = 0;
	//BYTE bPnMask[256];
	WORD i, j, wPointNum = 0;
	//BYTE bTaskParaData[512];

	BYTE bNum;
	DWORD dwID;
	BYTE bFnNum;
	WORD wDataLen;

	bool fMtrPn = false;
	//BYTE bPort = 0;

	if (bTaskNo > 0xfe)
		return 0;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, 512);
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	if (IsAllAByte(g_ExFlashBuf, 0x00, 512))
	{
		SignalSemaphore(g_semExFlashBuf);		
		return 0;
	}

	if (g_ExFlashBuf[0] == 0)// || (g_ExFlashBuf[8] == 0))//������Ч���������ϱ�
	{
		SignalSemaphore(g_semExFlashBuf);		
		return 0;
	}

	//memset(bPnMask, 0, sizeof(bPnMask));
	if ((g_ExFlashBuf[20] == 0xff) && (g_ExFlashBuf[21] == 0xff))//ȫ��������
	{
		
		for (wPn = 1; wPn < PN_NUM; wPn++)
		{
			fMtrPn = IsPnValid(wPn);
			if (!fMtrPn)
				continue;
			
			if (!GetPnProp(wPn))
				continue;
			
			//bPort = GetPnPort(wPn);
			//if (bPort > 30)//��485��֧������
				//continue;
			
			wPointNum++;
		}
	}
	else//������λ�������
	{
		for (i = 0; i < g_ExFlashBuf[19]; i++)
		{
			if (g_ExFlashBuf[20+2*i] != 0 && g_ExFlashBuf[20+2*i+1] != 0)
			{
				for (j=0; j<8; j++)
				{
					
					if (g_ExFlashBuf[20+2*i] & (1<<j))
					{
						wPn = (g_ExFlashBuf[20+2*i+1]-1)*8+j+1;

						fMtrPn = IsPnValid(wPn);
						if (!fMtrPn)
							continue;
						
						if (!GetPnProp(wPn))
							continue;
						
						//bPort = GetPnPort(wPn);
						//if (bPort > 30)//��485��֧������
							//continue;
						wPointNum++;
					}
				}
			}
		}
	}

	//TraceBuf(DB_FAPROTO, "g_ExFlashBuf:", g_ExFlashBuf, 50);
	bFnNum = (BYTE)*(g_ExFlashBuf+20+g_ExFlashBuf[19]*2);
	for (bNum=0; bNum < bFnNum; bNum++)
	{
		dwID = ByteToDWORD(g_ExFlashBuf+20+g_ExFlashBuf[19]*2+1+bNum*4, 4);
		wDataLen = GetItemLenDw(BN0, dwID); 
		dwLen += wPointNum*wDataLen;
	}

	SignalSemaphore(g_semExFlashBuf);

	return dwLen;

}


//���������㵥����ͨ����ÿ�ʼ�¼������PN FN����
//������@bTaskNo��ͨ�����
//���أ�ÿ�ʼ�¼�е�PN FN����
bool GetOneTaskDataPFnNum(BYTE bTaskNo, WORD* pwPnNum, WORD* pwFnNum)
{
	BYTE wPn;
	WORD wID = 0x0b11;
	DWORD dwLen = 0;
	WORD i, j, wPointNum = 0;

	BYTE bFnNum;
	WORD wDataLen;
	bool fMtrPn = false;
	
	if (bTaskNo > 0xfe)
		return false;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, sizeof(g_ExFlashBuf));
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	if (IsAllAByte(g_ExFlashBuf, 0x00, 512))
	{
		SignalSemaphore(g_semExFlashBuf);		
		return false;
	}

	if (g_ExFlashBuf[0] == 0)// || (g_ExFlashBuf[8] == 0))//������Ч���������ϱ�
	{
		SignalSemaphore(g_semExFlashBuf);		
		return false;
	}

	if ((g_ExFlashBuf[20] == 0xff) && (g_ExFlashBuf[21] == 0xff))//ȫ��������
	{
		
		for (wPn = 1; wPn < PN_NUM; wPn++)
		{
			fMtrPn = IsPnValid(wPn);
			if (!fMtrPn)
				continue;
			
			if (!GetPnProp(wPn))
				continue;
						
			wPointNum++;
		}
	}
	else//������λ�������
	{
		for (i = 0; i < g_ExFlashBuf[19]; i++)
		{
			if (g_ExFlashBuf[20+2*i] != 0 && g_ExFlashBuf[20+2*i+1] != 0)
			{
				for (j=0; j<8; j++)
				{
					
					if (g_ExFlashBuf[20+2*i] & (1<<j))
					{
						wPn = (g_ExFlashBuf[20+2*i+1]-1)*8+j+1;

						fMtrPn = IsPnValid(wPn);
						if (!fMtrPn)
							continue;
						
						if (!GetPnProp(wPn))
							continue;
						
						wPointNum++;
					}
				}
			}
		}
	}

	//TraceBuf(DB_FAPROTO, "g_ExFlashBuf:", g_ExFlashBuf, 50);
	*pwFnNum = (BYTE)*(g_ExFlashBuf+20+g_ExFlashBuf[19]*2);
	*pwPnNum = wPointNum;
	SignalSemaphore(g_semExFlashBuf);

	return true;
}


//���������㵥���м�����ÿ�ʼ�¼�ĳ���
//������@bTaskNo��ͨ�����
//���أ�ÿ�ʼ�¼�ĳ���
int GetFwdTaskPerDataLen(BYTE bTaskNo)
{
	return 255;
}

//���������㵥����ͨ������Ҫ�洢�ļ�¼����
//������@bTaskNo��ͨ�����
//���أ���Ҫ�洢�ļ�¼����(0��ʾһֱ��¼����0��ʾ��Ҫ��¼�ı���)
DWORD GetComTaskRecNum(BYTE bTaskNo)
{
	BYTE bSmplIntervU,bSmplIntervV;
	DWORD dwRecNum = 0;
	//BYTE bTaskParaData[512];

	if (bTaskNo > 0xfe)
		return -1;

	//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	//memset(g_ExFlashBuf, 0x00, 512);
	//ReadItemEx(BN0, bTaskNo, 0x0b11, g_ExFlashBuf);	
	//dwRecNum = ByteToWord(&g_ExFlashBuf[17]);
	//SignalSemaphore(g_semExFlashBuf);

	//if (0 == dwRecNum)//��Զִ�� �ͷ������ֵ
	{
		bSmplIntervU = GetComTaskSmplIntervU(bTaskNo);
		bSmplIntervV = GetComTaskSmplInterV(bTaskNo);
		if (bSmplIntervV == 0)
			bSmplIntervV = 1;
		if (TIME_UNIT_MINUTE == bSmplIntervU) return 7*1440/bSmplIntervV;
		if (TIME_UNIT_HOUR == bSmplIntervU)   return 15*24;
		if (TIME_UNIT_DAY == bSmplIntervU)    return 31;
		if (TIME_UNIT_MONTH == bSmplIntervU)  return 12;
	}

	return dwRecNum;
}


//���������㵥���м�������Ҫ�洢�ļ�¼����
//������@bTaskNo��ͨ�����
//���أ���Ҫ�洢�ļ�¼����(0��ʾһֱ��¼����0��ʾ��Ҫ��¼�ı���)
DWORD GetFwdTaskRecNum(BYTE bTaskNo)
{
	DWORD dwRecNum = 0;
	BYTE bSmplIntervU = 0, bSmplIntervV;	
	//BYTE bTaskParaData[512];
	if (bTaskNo > 0xfe)
		return -1;


	//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	//memset(g_ExFlashBuf, 0x00, 512);
	//ReadItemEx(BN0, bTaskNo, 0x0b21, g_ExFlashBuf);	
	//dwRecNum = ByteToWord(&g_ExFlashBuf[17]);
	//SignalSemaphore(g_semExFlashBuf);

	//if (0 == dwRecNum)//��Զִ�� �ͷ������ֵ
	{
		bSmplIntervU = GetFwdTaskSmplIntervU(bTaskNo);
		bSmplIntervV = GetFwdTaskSmplInterV(bTaskNo);
		if (bSmplIntervV == 0)
			bSmplIntervV = 1;
		if (TIME_UNIT_MINUTE == bSmplIntervU) return 7*1440/bSmplIntervV;
		if (TIME_UNIT_HOUR == bSmplIntervU)   return 15*24;
		if (TIME_UNIT_DAY == bSmplIntervU)    return 31;
		if (TIME_UNIT_MONTH == bSmplIntervU)  return 12;
	}
		
	return dwRecNum;
}

//��������ȡ��ͨ����ɼ������λ
BYTE  GetComTaskSmplIntervU(BYTE bTaskNo)
{
	TComTaskCfg tComTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b11, g_ExFlashBuf);
	memcpy(&tComTaskRptCfg, g_ExFlashBuf, sizeof(TComTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_MINUTE_TASK) return TIME_UNIT_MINUTE;
	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_HOUR_TASK)   return TIME_UNIT_HOUR;
	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_DAY_TASK)    return TIME_UNIT_DAY;
	if (tComTaskRptCfg.bComSampIntervU == TIME_UNIT_MONTH_TASK)	 return TIME_UNIT_MONTH;		
	
}

//��������ȡ�м�����ɼ������λ
BYTE  GetFwdTaskSmplIntervU(BYTE bTaskNo)
{
	TFwdTaskCfg tFwdTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b21, g_ExFlashBuf);
	memcpy(&tFwdTaskRptCfg, g_ExFlashBuf, sizeof(TFwdTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_MINUTE_TASK) return TIME_UNIT_MINUTE;
	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_HOUR_TASK)   return TIME_UNIT_HOUR;
	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_DAY_TASK)    return TIME_UNIT_DAY;
	if (tFwdTaskRptCfg.bFwdSampIntervU == TIME_UNIT_MONTH_TASK)	 return TIME_UNIT_MONTH;		
}

//��������ȡ��ͨ����ɼ�����
BYTE GetComTaskSmplInterV(BYTE bTaskNo)
{
	TComTaskCfg tComTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b11, g_ExFlashBuf);
	memcpy(&tComTaskRptCfg, g_ExFlashBuf, sizeof(TComTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	return tComTaskRptCfg.bComSampIntervV;
}


//��������ȡ�м�����ɼ�����
BYTE GetFwdTaskSmplInterV(BYTE bTaskNo)
{
	TFwdTaskCfg tFwdTaskRptCfg;
	//BYTE bTaskParaData[512];
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, 0x0b21, g_ExFlashBuf);
	memcpy(&tFwdTaskRptCfg, g_ExFlashBuf, sizeof(TFwdTaskCfg));
	SignalSemaphore(g_semExFlashBuf);

	return tFwdTaskRptCfg.bFwdSampIntervV;
}

//�������ж���ͨ�����Ƿ���Ч
//������@bTaskNo��ͨ�����
//���أ���Ч�򷵻�true;��֮����false;
bool IsTaskValid(BYTE bTaskType, BYTE bTaskNo)
{	
	WORD wID;
	bool fRet = true;
	//BYTE bTaskParaData[512];
	//char str[16];

	if (bTaskNo > 0xfe)
		return false;

	if (bTaskType == COMMON_TASK_TYPE)
		wID = 0x0b11;
	if (bTaskType == FWD_TASK_TYPE)
		wID = 0x0b21;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	memset(g_ExFlashBuf, 0x00, 512);
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	//TraceBuf(DB_FAPROTO, "g_ExFlashBuf:", g_ExFlashBuf, 100);
	if (IsAllAByte(g_ExFlashBuf, 0x00, 512) || IsAllAByte(g_ExFlashBuf, 0xff, 512))
	{
		SignalSemaphore(g_semExFlashBuf);		
		//DTRACE(DB_TASK, ("bTaskType=%d, bTaskNo=%d Para InValidate.\n", bTaskType, bTaskNo));
		return false;
	}

	if (g_ExFlashBuf[0] == 0)// || g_ExFlashBuf[8] == 0)//������Ч���������ϱ�
	{
		fRet = false;
		//DTRACE(DB_TASK, ("bTaskType=%d, bTaskNo=%d is InValidate.\n", bTaskType, bTaskNo));
	}

	SignalSemaphore(g_semExFlashBuf);


	return fRet;

}


//�������м��������ת��Ϊ��ͨ���������ʽ
//���ȡ���ݶ�ȡ�ӿ���ͳһ�ķ�ʽ�������ݶ�ȡ
void FwdToComTaskPara(TComTaskCfg* pComTaskRptCfg, TFwdTaskCfg* pFwdTaskRptCfg)
{
	pComTaskRptCfg->bComTaskValid = pFwdTaskRptCfg->bFwdTaskValid;				//������Ч
	memcpy(pComTaskRptCfg->bComRptBasTime, pFwdTaskRptCfg->bFwdRptBasTime, 5);	//�ϱ���׼ʱ��

	pComTaskRptCfg->bComRptIntervU = pFwdTaskRptCfg->bFwdRptIntervU;			//���ڵ�λ
	pComTaskRptCfg->bComRptIntervV = pFwdTaskRptCfg->bFwdRptIntervV;			//�ϱ�����

	pComTaskRptCfg->bComDataStru = pFwdTaskRptCfg->bFwdFwdType;					//�м����ʹ浽���ݷ�ʽ 
	memcpy(pComTaskRptCfg->bComSampBasTime, pFwdTaskRptCfg->bFwdSampBasTime, 5);//������׼ʱ��

	pComTaskRptCfg->bComSampIntervU = pFwdTaskRptCfg->bFwdSampIntervU;			//�������ڵ�λ
	pComTaskRptCfg->bComSampIntervV = pFwdTaskRptCfg->bFwdSampIntervV;			//��������	

	pComTaskRptCfg->bComRatio = pFwdTaskRptCfg->bFwdRatio;						//��ȡ����
	pComTaskRptCfg->bComDoTaskTimes = pFwdTaskRptCfg->bFwdDoTaskTimes;			//����ִ�д���
}


//��һ�����ͼ���ڴ��ڵ����ݵķֶζ�ȡ
//����һ�ζ�ȡ[dwStepTime, dwStartTime)�����ڴ��ڵ�������󲻳���1KB
bool GetOneFrmStartEndTime(BYTE bTaskType, BYTE bTaskNo, DWORD* pdwStepTime, DWORD dwStartTime, DWORD dwEndTime, TComTaskCfg* pCfg, bool* pfOnce)
{
	DWORD dwRptSecondsU = 0;
	DWORD dwSampleSecondsU = 0;
	TTime tStepTime, tLastRptTime;
	WORD  wDataLen = 0, bRecNum = 0, bRecTmpNum = 0;
	WORD  wPnNum=0, wFnNum=0;

	*pfOnce = false;
	*pdwStepTime = dwStartTime;
	
	if((pCfg == NULL) || (pCfg->bComTaskValid == 0) || 
		(pCfg->bComRptIntervV == 0) || (pCfg->bComSampIntervV == 0) ||
		(dwStartTime > dwEndTime) || (pdwStepTime == NULL))
		return false;

	if (pCfg->bComRptIntervU == TIME_UNIT_MINUTE_TASK)dwRptSecondsU = 60;
	if (pCfg->bComRptIntervU == TIME_UNIT_HOUR_TASK)  dwRptSecondsU = 60*60;
	if (pCfg->bComRptIntervU == TIME_UNIT_DAY_TASK)   dwRptSecondsU = 60*60*24;
	if (pCfg->bComRptIntervU == TIME_UNIT_MONTH_TASK)   dwRptSecondsU = 60*60*24*30;		
	
	if (pCfg->bComSampIntervU == TIME_UNIT_MINUTE_TASK)dwSampleSecondsU = 60;
	if (pCfg->bComSampIntervU == TIME_UNIT_HOUR_TASK)  dwSampleSecondsU = 60*60;
	if (pCfg->bComSampIntervU == TIME_UNIT_DAY_TASK)   dwSampleSecondsU = 60*60*24;
	if (pCfg->bComSampIntervU == TIME_UNIT_MONTH_TASK)   dwSampleSecondsU = 60*60*24*30;		

	//�ɼ�������ϱ������
	if (dwRptSecondsU*pCfg->bComRptIntervV < dwSampleSecondsU*pCfg->bComSampIntervV)
	{
		SecondsToTime(*pdwStepTime, &tStepTime);
		if(dwSampleSecondsU == 60*60*24*30)//��     �˴���30û������Don't worry
		{
			tStepTime.nDay = 1;
			tStepTime.nHour = 0;
			tStepTime.nMinute = 0;
			tStepTime.nSecond = 0;
		}
		else if(dwSampleSecondsU == 60*60*24)//��
		{
			tStepTime.nHour = 0;
			tStepTime.nMinute = 0;
			tStepTime.nSecond = 0;
		}
		else if(dwSampleSecondsU == 60*60)	//ʱ
		{
			tStepTime.nMinute = 0;
			tStepTime.nSecond = 0;
		}
		else if (dwSampleSecondsU == 60)	//��
		{
			tStepTime.nSecond = 0;
		}

		//�����λ���ʱ�������һ���ɼ���������ݣ���ʱ���Բ��ÿ��ǳ�ȡ������
		if (pCfg->bComRptIntervU == pCfg->bComSampIntervU)
		{
				AddIntervsInTask(&tStepTime , pCfg->bComSampIntervU, -(pCfg->bComSampIntervV));
		}
		
		*pfOnce = true;
		*pdwStepTime = TimeToSeconds(&tStepTime);
		wDataLen = GetComTaskPerDataLen(bTaskNo);

		if (wDataLen == 0)
			return false;
	}
	else
	{
		//(1)�������һ���ϱ��������ʼʱ��,��dwStartTime  ����һ���ϱ������ʼʱ��
		//����������������������ݵ���bRecNum��
		//(2)����tStepTime��ֵ�����㷽ʽΪ:tStepTime�ĳ�ʼֵΪ������ϱ���ʼʱ�䣬Ȼ��
		//tStepTime����ǰ��N ���ɼ����(N <= bRecNum)��ֻҪN  ����������ܳ��ȳ���1000Byte  ����ͣ����
		//�ص�[pdwStepTime, dwStartTime)  ����Ϳ����ڶ�ȡ�����в�����1000Byte�����ݡ�
		//(3)���ݶ�ȡ��ɺ�dwStartTime  ������dwStepTime �� �ظ�����(1)��

		SecondsToTime(dwEndTime, &tLastRptTime);		
		AddIntervsInTask(&tLastRptTime , pCfg->bComRptIntervU, -(pCfg->bComRptIntervV));
		//SecondsToTime(dwEndTime - BASE_TIME_DELAY*60, &tLastRptTime);		
		AddIntervsInTask(&tLastRptTime , pCfg->bComRptIntervU, -(pCfg->bComRptIntervV));

		if (dwRptSecondsU*pCfg->bComRptIntervV == dwSampleSecondsU*pCfg->bComSampIntervV)
			bRecNum = 1;
		else if(dwRptSecondsU*pCfg->bComRptIntervV > dwSampleSecondsU*pCfg->bComSampIntervV)
			bRecNum = (dwStartTime - TimeToSeconds(&tLastRptTime))/(dwSampleSecondsU*pCfg->bComSampIntervV*pCfg->bComRatio);

		if (dwStartTime <= TimeToSeconds(&tLastRptTime))
			return false;

		SecondsToTime(*pdwStepTime, &tStepTime);
		while ((wDataLen < 1000) && (bRecTmpNum < bRecNum))
		{
			bRecTmpNum++;
			if (bTaskType == FWD_TASK_TYPE)
				wDataLen += 255;//�м�������ʱ �̶�Ϊ255
			else if (bTaskType == COMMON_TASK_TYPE)
			{				
				wDataLen += GetComTaskPerDataLen(bTaskNo);				
				if (!GetOneTaskDataPFnNum(bTaskNo, &wPnNum, &wFnNum))
					return false;
				if (pCfg->bComDataStru == 0)			//��������ʽ
					wDataLen += (3+wPnNum*wFnNum*11+6);	//������ʽ(1Byte), ���ݸ���(2Byte), PN(2Byte)+dwID(4Byte)+Time(5Byte)=11Byte, Time(6Byte)
				else if (pCfg->bComDataStru == 1)		//�����������ʽ
					wDataLen += (3+wFnNum*2+6);			//������ʽ(1Byte), PN(2Byte), Time(6Byte)
			}

			AddIntervsInTask(&tStepTime , pCfg->bComSampIntervU, -(pCfg->bComSampIntervV*pCfg->bComRatio));
			if(TimeToSeconds(&tStepTime) <= TimeToSeconds(&tLastRptTime))
				break;
		}

		//�ܳ��ȴ���1000Byte��ÿ������С��1000Byte  ʱdwStepTime ���Ժ���һ����ȡ��
		if (bTaskType == COMMON_TASK_TYPE)
			if((GetComTaskPerDataLen(bTaskNo) < 1000) && (bRecNum != 1) && (wDataLen > 1000))
				AddIntervsInTask(&tStepTime , pCfg->bComSampIntervU, pCfg->bComSampIntervV*pCfg->bComRatio);


	}
	
	if (wDataLen != 0)
	{
		*pdwStepTime = TimeToSeconds(&tStepTime);
		return true;
	}
		
	return false;
}

//ģ����վ��������һ����������֡
int MakeComTaskReqRptFrm(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro, TComTaskCfg* pCfg, DWORD dwStepTime, DWORD dwStartTime)
{
	DWORD dwDI;
	BYTE bTime1[6], bTime2[6];

	TTime tStepTime;
	TTime tStartTime;

	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;

	SecondsToTime(dwStepTime, &tStepTime);	
	SecondsToTime(dwStartTime, &tStartTime);

	TimeToBcd(&tStepTime, bTime1);
	TimeToBcd(&tStartTime, bTime2);

	*pbTx++ = 0;			//DA1
	*pbTx++ = 0;			//DA2	
	pGbPro->wTxPtr += 2;

	if (bTaskType == COMMON_TASK_TYPE)
		dwDI = 0xe0000300;
	else if (bTaskType == FWD_TASK_TYPE)
		dwDI = 0xe0000400;

	dwDI += (bTaskNo+1);
	memcpy(pbTx, &dwDI, 4);	//DI
	pbTx += 4;
	pGbPro->wTxPtr += 4;

	memcpy(pbTx, bTime1, 6);//��ʼʱ��
	pbTx += 6;
	memcpy(pbTx, bTime2, 6);//����ʱ��
	pbTx += 6;
	pGbPro->wTxPtr += 12;

	*pbTx++ = 0;			//�����ܶ�(0-��ʾ�����������õĲ������ںͳ�ȡ���ʳ�ȡ����)
	pGbPro->wTxPtr++;
	SgMakeMasterReqFrm(pPro, true, SG_LFUN_DATAREPLY, SG_AFUN_ASKTASK, pGbPro->wTxPtr);
	TraceBuf(DB_FAPROTO, "MakeTaskReqFrm -->", pGbPro->pbTxBuf, pGbPro->wTxPtr+2);

	pGbPro->wTxPtr += 2;
	return pGbPro->wTxPtr;
}

void DoRptTaskData(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro, TComTaskCfg* pCfg, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen;
	bool fRet=true, fOnce = false;
	DWORD dwStepTime;
	DWORD dwStartTime1=dwStartTime;
	while(fRet)
	{		
		//��[dwStepTime, dwStartTime)Ϊ�����ǰ����ȡ��ÿ�ζ�ȡ�����ݿ�����1KB֮��
		//�����dwStartTime����Ϊ��һ��dwStepTime��ʱ�䣬���»�ȡ�µ�dwStepTime
		//Ȼ���ȡ�µ�[dwStepTime, dwStartTime)�����ڵ����ݣ�fRet����fasle  �����ݿɶ�
		fRet = GetOneFrmStartEndTime(bTaskType, bTaskNo, &dwStepTime, dwStartTime1, dwEndTime, pCfg, &fOnce);

		if (!fRet)
			break;

		//��һ֡�������ͨ�������ݵ�֡��ģ����վ�·�
		iLen = MakeComTaskReqRptFrm(bTaskType, bTaskNo, pPro, pCfg, dwStepTime, dwStartTime1);
		if (iLen > 0)
		{
			TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
			pGbPro->wRxStep = 0;		//��֮ǰ�Ľ�����0

			pGbPro->fRptState = true;	//������������״̬
			pPro->pfnRxFrm(pPro, pGbPro->pbTxBuf, iLen);
			pGbPro->fRptState = false;
			pGbPro->bMySEQ = (pGbPro->bMySEQ+1) & 0x0f; 
		}
		else
			break;

		if (fOnce)
			return;

		dwStartTime1 = dwStepTime;
	}
}


//���أ�true:��ʾ��ǰʱ������� pdwStartTime���ر������ʼʱ�䣬 pdwEndTime���ر��������ʱ�䣻
//		false:��ʾ��ǰʱ����δ����
bool GetRptTimeScope(TComTaskCfg* pCfg, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime)
{
	int iPast;
	DWORD dwCurSec, dwStartSec;
	TTime tmStart, tmEnd;

	Fmt15ToTime(pCfg->bComRptBasTime, &tmStart);
	tmStart.nSecond = 0;

	iPast = TaskIntervsPast(&tmStart, pNow, pCfg->bComRptIntervU, pCfg->bComRptIntervV);	//�ӻ�׼ʱ�俪ʼ�����������������ڸ���iPast	
	if (iPast < 0)		//�Ȼ�׼ʱ����
		return false;

	//tmStart = pCfg->tmStart;	//��׼ʱ��
	AddIntervsInTask(&tmStart, pCfg->bComRptIntervU, pCfg->bComRptIntervV*iPast);		//��iPast������-����ǰ�����ʼʱ��

	dwCurSec = TimeToSeconds(pNow);	//��ǰʱ�����ڼ������ʼʱ��
	dwStartSec = TimeToSeconds(&tmStart);	//��ǰʱ�����ڼ������ʼʱ��
	if (dwCurSec < dwStartSec)	//��û����ǰ�����ʼʱ��
		return false;
	

	//�����ʱ��
	tmEnd = tmStart;
	//AddIntervsInTask(&tmEnd, TIME_UNIT_MINUTE_TASK, BASE_TIME_DELAY);		//�����ڴӻ�׼ʱ�俪ʼ������������+5�������ϱ�
	//AddIntervs(&tmEnd, pCfg->bIntervU, pCfg->bIntervV);		//��1������ʱ��-����ǰ�������ʱ��
	AddIntervsInTask(&tmEnd, pCfg->bComRptIntervU, pCfg->bComRptIntervV);		//��Ϊ������������ڶ��ɽ���

	*pdwStartTime = TimeToSeconds(&tmStart);
	*pdwEndTime = TimeToSeconds(&tmEnd);
	return true;
}


bool DoRptTask(BYTE bTaskType, BYTE bTaskNo, struct TPro* pPro)
{
	WORD wID, wTaskTimesID, wTaskTimes=0, wTaskDoneTimeID;
	TTime tmNow;
	DWORD dwCurSec, dwStartTime, dwEndTime, dwRptDoneSecond;
	TComTaskCfg tComTaskRptCfg;
	TFwdTaskCfg tFwdTaskRptCfg;
	char str[16];

	if (bTaskType == COMMON_TASK_TYPE)
	{
		wID = 0x0b11;
		wTaskDoneTimeID = 0x0b35;
	}
	if (bTaskType == FWD_TASK_TYPE)
	{
		wID = 0x0b21;
		wTaskDoneTimeID = 0x0b36;
	}
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, wID, g_ExFlashBuf);
	if (bTaskType == COMMON_TASK_TYPE)
		memcpy(&tComTaskRptCfg, g_ExFlashBuf, sizeof(TComTaskCfg));
	else if (bTaskType == FWD_TASK_TYPE)
		memcpy(&tFwdTaskRptCfg, g_ExFlashBuf, sizeof(TFwdTaskCfg));
	SignalSemaphore(g_semExFlashBuf);
	
	if (bTaskType == FWD_TASK_TYPE)
		FwdToComTaskPara(&tComTaskRptCfg, &tFwdTaskRptCfg);//����ת��
	
	//WaitSemaphore(g_semEsam, SYS_TO_INFINITE);	

	GetCurTime(&tmNow);
	if (GetRptTimeScope(&tComTaskRptCfg, &tmNow, &dwStartTime, &dwEndTime))	//�ҵ���ʱ������ֹʱ��
	{
		dwCurSec = TimeToSeconds(&tmNow);
		if (dwCurSec >= dwStartTime && dwCurSec<dwEndTime)	//��ǰʱ���������ҵ�ǰ���δִ�й�
		{	
			ReadItemEx(BN0, bTaskNo, wTaskDoneTimeID, (BYTE *)&dwRptDoneSecond);			
			
			if (dwStartTime != dwRptDoneSecond)			//ÿ�����ֻ����һ�Σ�һ�α�����һ�������������
			{				
				//ReadItemEx(BN0, bTaskNo, wTaskTimesID, (BYTE*)&wTaskTimes);
				//if (wTaskTimes != 0)//����ִ�д���
				//{
					//if(wTaskTimes > tComTaskRptCfg.bComDoTaskTimes)
					//	return true;
					//else
					//{
					//	wTaskTimes++;
					//	WriteItemEx(BN0, bTaskNo, wTaskTimesID, (BYTE*)&wTaskTimes);
					//}
				//}
			
				DoRptTaskData(bTaskType, bTaskNo, pPro, &tComTaskRptCfg, dwStartTime, dwEndTime);
				
				sprintf(str, (bTaskType == COMMON_TASK_TYPE)?("COM_TASK"):("FWD_TASK"));
				DTRACE(DB_TASK, ("\nAutoSendTask, %s, bTaskNo=%d\n", str, bTaskNo+1));
				TraceSecsToTime("ThisTime dwStartRptTime= ",dwStartTime);					
				TraceSecsToTime("LastTime dwDoneRptSecond=",dwRptDoneSecond);						
				
				dwRptDoneSecond = dwStartTime;
				WriteItemEx(BN0, bTaskNo, wTaskDoneTimeID, (BYTE *)&dwRptDoneSecond);
				
			}
		}
	}

	//SignalSemaphore(g_semEsam);
	return true;
}

