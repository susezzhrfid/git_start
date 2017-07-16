/*********************************************************************************************************
 * Copyright (c) 2014,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DoTask.c
 * ժ    Ҫ�����ļ���Ҫʵ���������ݵĲɼ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2014��9��
*********************************************************************************************************/
#include "DoTask.h"

//TRptCtrl g_ComDoCtrl[MAX_COMMON_TASK];	//��ͨ����
//TRptCtrl g_FwdDoCtrl[MAX_FWD_TASK];		//�м�����

bool DoRdTaskData(BYTE bTaskType, BYTE bTaskNo, TComTaskCfg *pCfg, DWORD dwStartTime, DWORD dwEndTime, BYTE bThrId)
{
	int i, j, k;
	WORD wID, wPn, wID1;
	WORD bNum, bFnNum, wDataLen = 0;
	DWORD dwID;
	int iRet = 0;
	struct TMtrPro* pMtrPro;
	TProIdInfo ProIdInfo = {0};
	WORD* pwSubID;
	BYTE* pbTmpBuf = g_bEsamTxRxBuf;
	//BYTE bRxBuf[256];	
	BYTE bBuf[256];
	BYTE* pbData = pbTmpBuf;
	WORD bSubID=0;
	TTime tTime, Now;

	DWORD dwRdSec;
	WORD wSubNum, wBN = BN0;
	BYTE bRdMode, bPort=0;
	int iThrId, iLen=0;
	bool fMtrPn;
	BYTE bTaskParaBuf[512];
	BYTE bRxDataLen = 0;
	TCommPara tComPara;
	BYTE iPort=0;

	WORD iDataLen=0;

	if (g_fStopMtrRd)
	{
		DTRACE(DB_TASK, ("DoRdTaskData g_fStopMtrRd.\n"));
		return false;
	}

	if (bTaskType == COMMON_TASK_TYPE)
		wID = 0x0b11;
	if (bTaskType == FWD_TASK_TYPE)
		wID = 0x0b21;

	//GetPnProp()

	if (bTaskType == COMMON_TASK_TYPE)//��ͨ����
	{	
		//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
		ReadItemEx(BN0, bTaskNo, wID, bTaskParaBuf);

		pbData += 5;
		if ((bTaskParaBuf[20] == 0xff) && (bTaskParaBuf[21] == 0xff))//ȫ��������
		{
			for (wPn = 1; wPn < PN_NUM; wPn++)
			{
				fMtrPn = IsPnValid(wPn);
				if (!fMtrPn)
					continue;
				
				if (!GetPnProp(wPn))
					continue;
				
				//bPort = GetPnPort(wPn);
				if (((IsCctPn(wPn)) && (bThrId == 0)) || ((!IsCctPn(wPn)) && (bThrId == 1)))//485���������ز�������ֿ���
				{			
					//DTRACE(DB_TASK, ("wPn=%d is not this bThrId=%d TaskPn\n", wPn, bThrId));
					return false;
				}

				//��ȡ����ID��
				bFnNum = *(bTaskParaBuf+19+bTaskParaBuf[19]*2+1);
				for (bNum=0; bNum < bFnNum; bNum++)
				{					
					iLen = 0;
					dwID = ByteToDWORD(bTaskParaBuf+19+bTaskParaBuf[19]*2+1+1+bNum*4, 4); 
					//wDataLen = GetItemLenDw(BN0, dwID); 

					if (!GetProIdInfo(dwID, &ProIdInfo, false))
						continue;
					//pwSubID = ProId2RdId(ProIdInfo.dwProId, &bSubID);
					pwSubID = ProId2SubId(ProIdInfo.dwProId, &wSubNum);

					GetMeterPara(wPn, &g_MtrPara[bThrId]);
					pMtrPro = CreateMtrPro(wPn, &g_MtrPara[bThrId], &g_MtrSaveInf[bThrId], (bSubID == 0), bThrId);
					if (pMtrPro == NULL)
						continue;

					for(k = 0; k<wSubNum; k++)
					{
						if(GetItemLen(wBN,pwSubID[k]) > 0)
						{
							if ((iLen+GetItemLen(wBN,pwSubID[k])) > 256)
							{
								//SignalSemaphore(g_semExFlashBuf); 										
								return true;
							}
						}
						else
							continue;

						GetCurTime(&Now); //ʱ��ͳһ�������ȡ����֤��д������ͬһ������
						wID = pwSubID[k];
						if (IsMtrID(wID) && !MeterSpecierId(dwID))
						{
							iRet = ReadItemEx(BN24, PN0, 0x4105, &bRdMode);
							if (iRet>0 && bRdMode==3)// && !fRptState) //ʵʱ��ȡ�������F38ͬ����Ч
							{							//�������Ͳ���ֱ�ӳ�������Ѽ���������ݵ������
								if (g_fStopMtrRd)
								{
									DTRACE(DB_TASK, ("DoRdTaskData g_fStopMtrRd.\n"));
									return false;
								}

								if (ReadItemEx(BN0,wPn,0x890a,&bPort) < 0)
								{
									DTRACE(DB_TASK, ("Get bPort Fail.\n"));
									return false;
								}

								g_bDirRdStep = 1;
								
								GetDirRdCtrl(bPort); //ȡ��ֱ���Ŀ���Ȩ 	

								
								DTRACE(DB_TASK, ("bThrId=%d, bTaskNo=%d, DoDirMtrRd.\n", bThrId, bTaskNo+1));
								iThrId = DoDirMtrRd(wBN, wPn, wID, Now);
								ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
								g_bDirRdStep = 0;
								if (iThrId < 0)
								{
									DTRACE(DB_TASK, ("iThrId=%d.\n", iThrId));
									return false;
								}
							}
							else
							{
								iThrId = GrabPnThread(wPn); //GetPnThread
								SetDynPn(iThrId, wPn);	//����ϵͳ�⣬����ò����������
							}
						}

						if (fMtrPn && !IsPnDataLoaded(wPn)) //���������ݻ�û������ڴ�
						{
							if (!LoadPnDirData(wPn))	//���������ֱ������
							{
								DTRACE(DB_TASK, ("DoTask fail to rd pn=%d's data!\n", wPn));
								//SignalSemaphore(g_semExFlashBuf); 										
								return false;
							}
						}

						dwRdSec = GetCurIntervSec(wPn, &Now);	//���ϵ�ǰ������ʱ��
						if (/*fRptState && */dwRdSec==TimeToMinutes(&Now)*60)
							dwRdSec -= GetMeterInterv(wPn)*60;

						if(MeterSpecierId(dwID))
						{
							dwRdSec = 0;
						}

						if (iLen + ProIdInfo.wDataLen > 256)
							return true;
						
						ReadItemTm(wBN, wPn, pwSubID[k], bBuf+iLen, dwRdSec, INVALID_TIME); //û���������ݶ��Ѿ��ó���Ч����
						iLen += GetItemLen(wBN,pwSubID[k]);

					}
					
					//TraceBuf(DB_TASK, "DoRdTaskData pbData:", bBuf, iLen);
					memcpy(pbData+iDataLen, bBuf+ProIdInfo.wOffset, ProIdInfo.wDataLen);								
					if (iDataLen + GetComTaskPerDataLen(bTaskNo) > 1024)
					{
						DTRACE(DB_TASK, ("DoTask fail iDataLen > 1024.\n"));
						return true;
					}
					
					iDataLen += ProIdInfo.wDataLen;
					//if((ProIdInfo.dwProId != ProIdInfo.dwId) && (C_01R == ProIdInfo.bCode))
					//	wSubNum--;

				}
			}
		}
		else//������λ�������
		{
			for (i = 0; i < bTaskParaBuf[19]; i++)
			{
				if (bTaskParaBuf[20+2*i] != 0 && bTaskParaBuf[20+2*i+1] != 0)
				{
					for (j=0; j<8; j++)
					{
						if (bTaskParaBuf[20+2*i] & (1<<j))
						{
							//��ȡ������
							wPn = (bTaskParaBuf[20+2*i+1]-1)*8+j+1;
							
							fMtrPn = IsPnValid(wPn);
							if (!fMtrPn)
								continue;

							if (!GetPnProp(wPn))
								continue;

							//bPort = GetPnPort(wPn);
							if (((IsCctPn(wPn)) && (bThrId == 0)) || ((!IsCctPn(wPn)) && (bThrId == 1)))//485���������ز�������ֿ���
							{			
								//DTRACE(DB_TASK, ("wPn=%d is not this bThrId=%d TaskPn\n", wPn, bThrId));
								return false;
							}


							//��ȡ����ID��
							bFnNum = *(bTaskParaBuf+19+bTaskParaBuf[19]*2+1);
							for (bNum=0; bNum < bFnNum; bNum++)
							{
								iLen = 0;
								dwID = ByteToDWORD(bTaskParaBuf+19+bTaskParaBuf[19]*2+1+1+bNum*4, 4); 
								//wDataLen = GetItemLenDw(BN0, dwID); 

								if (!GetProIdInfo(dwID, &ProIdInfo, false))
									continue;
								//pwSubID = ProId2RdId(ProIdInfo.dwProId, &bSubID);
								pwSubID = ProId2SubId(ProIdInfo.dwProId, &wSubNum);

								GetMeterPara(wPn, &g_MtrPara[bThrId]);
								pMtrPro = CreateMtrPro(wPn, &g_MtrPara[bThrId], &g_MtrSaveInf[bThrId], (bSubID == 0), bThrId);
								if (pMtrPro == NULL)
									continue;

								for(k = 0; k<wSubNum; k++)
								{
									if(GetItemLen(wBN,pwSubID[k]) > 0)
									{
										if ((iLen+GetItemLen(wBN,pwSubID[k])) > 256)
										{
											//SignalSemaphore(g_semExFlashBuf);											
											return true;
										}
									}
									else
										continue;

									GetCurTime(&Now); //ʱ��ͳһ�������ȡ����֤��д������ͬһ������
									wID = pwSubID[k];
									if (IsMtrID(wID) && !MeterSpecierId(dwID))
									{
										iRet = ReadItemEx(BN24, PN0, 0x4105, &bRdMode);
										if (iRet>0 && bRdMode==3)// && !fRptState) //ʵʱ��ȡ�������F38ͬ����Ч
										{							//�������Ͳ���ֱ�ӳ�������Ѽ���������ݵ������
											if (g_fStopMtrRd)
											{
												DTRACE(DB_TASK, ("DoRdTaskData g_fStopMtrRd.\n"));
												return false;
											}

											if (ReadItemEx(BN0,wPn,0x890a,&bPort) < 0)
											{
												DTRACE(DB_TASK, ("Get bPort Fail.\n"));
												return false;
											}

											g_bDirRdStep = 1;
																			
											GetDirRdCtrl(bPort); //ȡ��ֱ���Ŀ���Ȩ												
											DTRACE(DB_TASK, ("DoDirMtrRd, bTaskNo=%d\n", bTaskNo+1));
											iThrId = DoDirMtrRd(wBN, wPn, wID, Now);
											ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
											g_bDirRdStep = 0;
											if (iThrId < 0)
											{
												DTRACE(DB_TASK, ("iThrId=%d.\n", iThrId));
												return true;
											}
										}
										else
										{
											iThrId = GrabPnThread(wPn); //GetPnThread
											SetDynPn(iThrId, wPn);	//����ϵͳ�⣬����ò����������
										}
									}

									if (fMtrPn && !IsPnDataLoaded(wPn)) //���������ݻ�û������ڴ�
									{
										if (!LoadPnDirData(wPn))	//���������ֱ������
										{
											DTRACE(DB_TASK, ("DoTask fail to rd pn=%d's data!\r\n", wPn));
											//SignalSemaphore(g_semExFlashBuf);											
											return false;
										}
									}

									dwRdSec = GetCurIntervSec(wPn, &Now);	//���ϵ�ǰ������ʱ��
									if (/*fRptState && */dwRdSec==TimeToMinutes(&Now)*60)
										dwRdSec -= GetMeterInterv(wPn)*60;

									if(MeterSpecierId(dwID))
									{
										dwRdSec = 0;
									}
									
									if (iLen + ProIdInfo.wDataLen > 256)
										return true;

									ReadItemTm(wBN, wPn, pwSubID[k], bBuf+iLen, dwRdSec, INVALID_TIME); //û���������ݶ��Ѿ��ó���Ч����
									iLen += GetItemLen(wBN,pwSubID[k]);

								}

								//TraceBuf(DB_TASK, "DoRdTaskData pbData:", bBuf, iLen);
								memcpy(pbData+iDataLen, bBuf+ProIdInfo.wOffset, ProIdInfo.wDataLen);								
								if (iDataLen + GetComTaskPerDataLen(bTaskNo) > 1024)
								{
									DTRACE(DB_TASK, ("DoTask fail iDataLen > 1024.\n"));
									return true;
								}
								
								iDataLen += ProIdInfo.wDataLen;
								//if((ProIdInfo.dwProId != ProIdInfo.dwId) && (C_01R == ProIdInfo.bCode))
								//	wSubNum--;
																
							}
						}
					}
				}
			}
		}

		//SignalSemaphore(g_semExFlashBuf);

		if (iLen > 0)
		{
			SecondsToTime(dwStartTime, &tTime);
			TimeToFmt14(&tTime, pbTmpBuf);	//5Byte��ʱ��14-08-27 15:00	
			iDataLen += 5;
			
			DTRACE(DB_TASK, ("bThrId=%d, COM_TASK, bTaskNo=%d, ", bThrId, bTaskNo+1));
			TraceBuf(DB_TASK, "TaskData:", pbTmpBuf, iDataLen);
			return PipeAppend(bTaskType, bTaskNo, pbTmpBuf, iDataLen);
		}
	}
	else if (bTaskType == FWD_TASK_TYPE)//�м�����
	{
		ReadItemEx(BN0, bTaskNo, wID, bTaskParaBuf);
		iPort = MeterPortToPhy(bTaskParaBuf[19]); //����:������߼��˿ڵ�����˿ڵ�ӳ��
		if (iPort < 0)
		{
			DTRACE(DB_TASK, ("FWD_TASK, MeterPortToPhy Fail.\n"));			
			return false;
		}

		//bPort = GetPnPort(wPn);
		//if (((PORT_CCT_PLC == bTaskParaBuf[19]) && (bThrId == 1)) ||	//�߳�1ֻ�м̶˿�31������
		//	((bTaskParaBuf[19] < PORT_CCT_PLC) && (bThrId == 0)))		//�߳�0ֻ�м�485������
		//	;
		//else
		//	return false;

		if (g_fStopMtrRd)
		{
			DTRACE(DB_TASK, ("DoRdTaskData g_fStopMtrRd.\n"));
			return false;
		}
		
		tComPara.wPort = bTaskParaBuf[19];
		tComPara.dwBaudRate = ValToBaudrate(bTaskParaBuf[20]);
		tComPara.bParity = ValToParity(bTaskParaBuf[21]);
		tComPara.bByteSize = ValToByteSize(bTaskParaBuf[22]);
		tComPara.bStopBits = ValToStopBits(bTaskParaBuf[23]);
		
		DTRACE(DB_TASK, ("FWD_TASK, ComTransmit645Cmd, bTaskNo=%d\n", bTaskNo+1));
		iRet = ComTransmit645Cmd(tComPara, &bTaskParaBuf[26], bTaskParaBuf[25], bBuf+6, &bRxDataLen, bTaskParaBuf[24]);

		if (iRet > 0)	
		{
			SecondsToTime(dwStartTime, &tTime);
			TimeToFmt14(&tTime, bBuf);				//ʱ��			

			bBuf[5] = bRxDataLen;
			wDataLen = 5+1+bRxDataLen;
			
			DTRACE(DB_TASK, ("bThrId=%d, FWD_TASK, bTaskNo=%d, ", bThrId, bTaskNo+1));
			TraceBuf(DB_TASK, "TaskData:", bBuf, wDataLen);
			return PipeAppend(bTaskType, bTaskNo, bBuf, wDataLen);//pbData[0]�м����ͣ�pbData[1]645֡����
		}
	}

	return false;
	//��������

}

//���أ�true:��ʾ��ǰʱ������� pdwStartTime���ر������ʼʱ�䣬 pdwEndTime���ر��������ʱ�䣻
//		false:��ʾ��ǰʱ����δ����
bool GetDoTaskTimeScope(TComTaskCfg* pCfg, const TTime* pNow, DWORD* pdwStartTime, DWORD* pdwEndTime)
{
	int iPast;
	DWORD dwCurSec, dwStartSec;
	TTime tmStart, tmEnd;

	Fmt15ToTime(pCfg->bComSampBasTime, &tmStart);
	tmStart.nSecond = 0;

	iPast = TaskIntervsPast(&tmStart, pNow, pCfg->bComSampIntervU, pCfg->bComSampIntervV);	//�ӻ�׼ʱ�俪ʼ�����������������ڸ���iPast	
	if (iPast < 0)		//�Ȼ�׼ʱ����
		return false;

	//tmStart = pCfg->tmStart;	//��׼ʱ��
	AddIntervsInTask(&tmStart, pCfg->bComSampIntervU, pCfg->bComSampIntervV*iPast);		//��iPast������-����ǰ�����ʼʱ��

	dwCurSec = TimeToSeconds(pNow);	//��ǰʱ�����ڼ������ʼʱ��
	dwStartSec = TimeToSeconds(&tmStart);	//��ǰʱ�����ڼ������ʼʱ��
	if (dwCurSec < dwStartSec)	//��û����ǰ�����ʼʱ��
		return false;
	

	//�����ʱ��
	tmEnd = tmStart;
	//AddIntervsInTask(&tmEnd, TIME_UNIT_MINUTE_TASK, BASE_TIME_DELAY);		//�����ڴӻ�׼ʱ�俪ʼ������������+5�������ϱ�
	//AddIntervs(&tmEnd, pCfg->bIntervU, pCfg->bIntervV);		//��1������ʱ��-����ǰ�������ʱ��
	AddIntervsInTask(&tmEnd, pCfg->bComSampIntervU, pCfg->bComSampIntervV);		//������������ڶ��ɽ���

	*pdwStartTime = TimeToSeconds(&tmStart);
	*pdwEndTime = TimeToSeconds(&tmEnd);
	return true;
}

bool DoDataTask(BYTE bTaskType, BYTE bTaskNo, BYTE bThrId)
{
	WORD wID, wTaskTimesID, wTaskTimes=0, wTaskDoneTimeID;
	TTime tmNow;
	DWORD dwCurSec, dwStartTime, dwEndTime, dwTaskDoneSecond;
	TComTaskCfg tComTaskCfg;
	TFwdTaskCfg tFwdTaskCfg;
	BYTE bTaskParaData[512];
	char str[16];
	bool fRet=false;

	if (bTaskType == COMMON_TASK_TYPE)
	{
		wID = 0x0b11;
		wTaskTimesID = 0x0b33;
		wTaskDoneTimeID = 0x0b37;
	}
	if (bTaskType == FWD_TASK_TYPE)
	{
		wID = 0x0b21;		
		wTaskTimesID = 0x0b34;		
		wTaskDoneTimeID = 0x0b38;
	}

	//WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	ReadItemEx(BN0, bTaskNo, wID, bTaskParaData);
	if (bTaskType == COMMON_TASK_TYPE)
		memcpy(&tComTaskCfg, bTaskParaData, sizeof(TComTaskCfg));
	else if (bTaskType == FWD_TASK_TYPE)
		memcpy(&tFwdTaskCfg, bTaskParaData, sizeof(TFwdTaskCfg));
	//SignalSemaphore(g_semExFlashBuf);

	if (bTaskType == FWD_TASK_TYPE)
		FwdToComTaskPara(&tComTaskCfg, &tFwdTaskCfg);//����ת��


	GetCurTime(&tmNow);	
	if (GetDoTaskTimeScope(&tComTaskCfg, &tmNow, &dwStartTime, &dwEndTime)) //�ҵ���ʱ������ֹʱ��
	{
		dwCurSec = TimeToSeconds(&tmNow);
		if (dwCurSec >= dwStartTime && dwCurSec<dwEndTime)	//��ǰʱ���������ҵ�ǰ���δִ�й�
		{						
			ReadItemEx(BN0, bTaskNo, wTaskDoneTimeID, (BYTE *)&dwTaskDoneSecond);

			if (dwStartTime != dwTaskDoneSecond)			//ÿ�����ֻ����һ�Σ�
			{					
				ReadItemEx(BN0, bTaskNo, wTaskTimesID, (BYTE* )&wTaskTimes);
				if (tComTaskCfg.bComDoTaskTimes != 0)//����ִ�д���
				{
					if(wTaskTimes >= tComTaskCfg.bComDoTaskTimes)
					{
						DTRACE(DB_TASK, ("bThrId=%d, %s, bTaskNo=%d, wTaskTimes=%d, tComTaskCfg.bComDoTaskTimes=%d\n", 
								bThrId, str, bTaskNo+1, wTaskTimes, tComTaskCfg.bComDoTaskTimes));
						return true;
					}
				}
				
				WaitSemaphore(g_semEsam, SYS_TO_INFINITE);	
				fRet = DoRdTaskData(bTaskType, bTaskNo, &tComTaskCfg, dwStartTime, dwEndTime, bThrId);
				SignalSemaphore(g_semEsam);				
				if (!fRet)
				{
					//DTRACE(DB_TASK, ("DoRdTaskData Fail fRet=false.\n"));
					return false;
				}
				
				sprintf(str, (bTaskType == COMMON_TASK_TYPE)?("COM_TASK"):("FWD_TASK"));
				DTRACE(DB_TASK, ("\nDoTask, %s, bTaskNo=%d\n", str, bTaskNo+1));
				TraceSecsToTime("ThisTime dwStartTime=     ",dwStartTime); 				
				TraceSecsToTime("LastTime dwTaskDoneSecond=",dwTaskDoneSecond);					

				dwTaskDoneSecond = dwStartTime;
				WriteItemEx(BN0, bTaskNo, wTaskDoneTimeID, (BYTE *)&dwTaskDoneSecond);

				if ((tComTaskCfg.bComDoTaskTimes != 0) && (wTaskTimes < tComTaskCfg.bComDoTaskTimes))//����ִ�д���
				{
					wTaskTimes++;
					WriteItemEx(BN0, bTaskNo, wTaskTimesID, (BYTE* )&wTaskTimes);
				}
			}
		}
	}

	return false;
}

void UpdTaskInfo()
{
	BYTE bCfgFlag[64];
	BYTE bCommonTaskNum=0; 
	BYTE bFwdTaskNum=0;
	BYTE i,j, bTaskNo;
	WORD wTaskTimes = 0;
	DWORD dwTaskDoneSecond=0;

	//UpdFat();
	
	ReadItemEx(BN0, PN0, 0x0b40, bCfgFlag);
	
	for (i=0; i<32; i++)
	{	
		if ((bCfgFlag[i] == 0) && (bCfgFlag[i+32] == 0))
			continue;

		for (j=0; j<8; j++)
		{
			if (bCfgFlag[i]&(1<<j))
				bCommonTaskNum++;
			if (bCfgFlag[i+32]&(1<<j))
				bFwdTaskNum++;
		}
	}

	WriteItemEx(BN0, POINT0, 0x0b10, &bCommonTaskNum);	//���µ�ǰ����ͨ��������
	WriteItemEx(BN0, POINT0, 0x0b20, &bFwdTaskNum);		//���µ�ǰ���м���������

	//��������б������������ִ�д�������	
	ReadItemEx(BN0, PN0, 0x0b41, bCfgFlag);
	for (i = 0; i < 32; i++)
	{
		if ((bCfgFlag[i] == 0) && (bCfgFlag[i+32] == 0))
			continue;
		
		for (j=0; j<8; j++)
		{
			if (bCfgFlag[i]&(1<<j))
			{
				bTaskNo = i*8+j;
				WriteItemEx(BN0, bTaskNo, 0x0b33, (BYTE* )&wTaskTimes);//�������	
				WriteItemEx(BN0, bTaskNo, 0x0b37, (BYTE* )&dwTaskDoneSecond);//���ʱ��
				bCfgFlag[i] &=  ~(1<<j);		//�������־����
				DTRACE(DB_TASK, ("COM_TASK, bTaskNo=%d, clear DoTaskTimes & dwTaskDoneSecond.\n", bTaskNo+1));
			}
			if (bCfgFlag[i+32]&(1<<j))
			{
				bTaskNo = i*8+j;
				WriteItemEx(BN0, bTaskNo, 0x0b34, (BYTE* )&wTaskTimes);//�������								
				WriteItemEx(BN0, bTaskNo, 0x0b38, (BYTE* )&dwTaskDoneSecond);//���ʱ��
				bCfgFlag[i+32] &=  ~(1<<j);		//�������־����
				DTRACE(DB_TASK, ("FWD_TASK, bTaskNo=%d, clear DoTaskTimes & dwTaskDoneSecond.\n", bTaskNo+1));
			}
		}
	}

	WriteItemEx(BN0, PN0, 0x0b41, bCfgFlag);	
	DTRACE(DB_TASK, ("UpdTaskInfo wTaskTimes=%d, bCommonTaskNum=%d, bFwdTaskNum=%d\n", wTaskTimes, bCommonTaskNum, bFwdTaskNum));
}

void ResetTaskInfo()
{
	BYTE bTaskNo;
	BYTE wTaskTimes=0;
	BYTE dwTaskDoneSecond=0;
	
	for (bTaskNo = 0; bTaskNo < MAX_COMMON_TASK; bTaskNo++)
	{
		WriteItemEx(BN0, bTaskNo, 0x0b33, (BYTE* )&wTaskTimes);//��ͨ�������	
		WriteItemEx(BN0, bTaskNo, 0x0b37, (BYTE* )&dwTaskDoneSecond);//���ʱ��

		WriteItemEx(BN0, bTaskNo, 0x0b34, (BYTE* )&wTaskTimes);//�м��������								
		WriteItemEx(BN0, bTaskNo, 0x0b38, (BYTE* )&dwTaskDoneSecond);//���ʱ��
	}
	
	DTRACE(DB_TASK, ("ResetTaskInfo Clear wTaskTimes And dwTaskDoneSecond.\n"));
}

bool DoTask(BYTE bThrId)
{
	int i;
	static DWORD dwDoTaskLastClick[2] = {0, 0};
	BYTE bCfgFlag[64];
	BYTE bCommonTaskNum=0, bFwdTaskNum=0;
	
	if (GetClick()-dwDoTaskLastClick[bThrId] < 3)	 //���������ⲿFLASH���������Ƚϴ󣬲�ҪƵ��ִ��
		return true;

	if (GetInfo(INFO_TASK_INIT))
		UpdTaskInfo();

	
	ReadItemEx(BN0, POINT0, 0x0b10, &bCommonTaskNum);	//��ǰ����ͨ��������
	ReadItemEx(BN0, POINT0, 0x0b20, &bFwdTaskNum);		//��ǰ���м���������
	if (bCommonTaskNum == 0 || bFwdTaskNum == 0)
		DTRACE(DB_TASK, ("bCommonTaskNum=%d, bFwdTaskNum=%d\n", bCommonTaskNum, bFwdTaskNum));

	if (bCommonTaskNum != 0)
	{
		for (i = 0; i<MAX_COMMON_TASK; i++)//��ͨ������
		{
			//if (!IsTaskExist(MAX_COMMON_TASK, i))
			//	continue;
			if (g_fStopMtrRd)
			  break;

			if (IsTaskValid(COMMON_TASK_TYPE, i))		
				DoDataTask(COMMON_TASK_TYPE, i, bThrId);
		}
	}

	if (bFwdTaskNum != 0)
	{
		for (i = 0; i<MAX_FWD_TASK; i++)//�м�������
		{
			//if (!IsTaskExist(MAX_COMMON_TASK, i))
			//	continue;
		  	if (g_fStopMtrRd)
			  break;
			
			if (IsTaskValid(FWD_TASK_TYPE, i))		
				DoDataTask(FWD_TASK_TYPE, i, bThrId);
		}
	}

	dwDoTaskLastClick[bThrId] = GetClick();
	return true;
}

