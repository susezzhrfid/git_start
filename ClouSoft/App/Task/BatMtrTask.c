#include <stdio.h>
#include "MtrAPI.h"
#include "FaAPI.h"
#include "BatMtrTask.h"
#include "MtrHook.h"
#include "FileMgr.h"
#include "FlashMgr.h"
#include "EsamCmd.h"
#include "DbAPI.h"
#include "MeterPro.h"

//ȡ�����������ݺ���
bool GetBatMtrTask1Data(TBatTask1* pBatTask1, WORD* pwMtrIdx, BYTE* pwSect)
{
	//bool fIsNewSect = false;
	WORD wTaskLen;
	WORD wOffset, wSectOff;
	WORD /*wTotalPn, */wPnNum;
	BYTE bSect = 0;
	BYTE bFlashSect = 0;
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	while(bFlashSect < 2)
	{
		memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
		if(!readfile(FILE_BATTASK1 ,bFlashSect*EXSECT_SIZE, g_ExFlashBuf, -1))
		{
		    //���2��������ȡ�����򲻼������ж�ȡ
			SignalSemaphore(g_semExFlashBuf);
			return false;
		  //bFlashSect++;
		  //continue;
		}
		
		if(!CheckFile(FILE_BATTASK1, g_ExFlashBuf, 0))
		{
			SignalSemaphore(g_semExFlashBuf);
			return false;
		  //bFlashSect++;
		  //continue;
		} 
		
		if((bSect==*pwSect) && *pwSect!=0)
		    break;
				
		wSectOff = 0;
		while (wSectOff < EXSECT_SIZE)
		{
			//0x55 0xaa + 2���ֽڵ�֡�� + ��֡����
			if (g_ExFlashBuf[wSectOff]==0x55 && g_ExFlashBuf[wSectOff+1]==0xaa)
			{
				wTaskLen = ByteToWord(&g_ExFlashBuf[wSectOff+2]);
				if (bSect == *pwSect)	//�Ѿ�������Ҫ����Ĳ���
				{	
					if (wTaskLen == 0)
						break;//Ϊ�ͷ��ź�����ѭ��ȫ����break�˳�
						
					wSectOff += 4;
					pBatTask1->wTaskDataLen = ByteToWord(&g_ExFlashBuf[wSectOff+3]);
					
					if (pBatTask1->wTaskDataLen > sizeof(pBatTask1->bTaskData))//���ȷǷ��ж�
					{
					    SignalSemaphore(g_semExFlashBuf);
					    return false;
					}
						
					memrcpy(pBatTask1->bTaskData, &g_ExFlashBuf[wSectOff+5], pBatTask1->wTaskDataLen);
					//ע������ Ҫʹ�� wSectoff
					wOffset = wSectOff + 2 + 1 + 2 + pBatTask1->wTaskDataLen;
					wPnNum = ByteToWord(&g_ExFlashBuf[wOffset]);
					wOffset += 2;
					
					if ((*pwMtrIdx) >= wPnNum)
					{
					    SignalSemaphore(g_semExFlashBuf);
						return false;
					}
						
    				//if ((*pwMtrIdx) == 0)
					//{
						//fIsNewSect = true;
					//}
	
					wOffset += (*pwMtrIdx) * (32 + 8);
	
					memrcpy(pBatTask1->bAddr, &g_ExFlashBuf[wOffset], 8);
					memrcpy(pBatTask1->bMtrKeyCiph, &g_ExFlashBuf[wOffset+8], 32);
	
					(*pwMtrIdx)++;
					if ((*pwMtrIdx) == wPnNum)
					{
						*pwSect += 1;
						(*pwMtrIdx) = 0;
					}
					else if((*pwMtrIdx) > wPnNum)	//�������εĲ���������
						break;
	
					//if(fIsNewSect)
					//{
					//	//�µ�bTaskPart���� ��Ҫ��������̨�������ļ�
					//	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
					//	readfile(FILE_BATTASK_STATUS,0, g_ExFlashBuf, -1);
					//	CheckFile(FILE_BATTASK_STATUS, g_ExFlashBuf, 0);
					//	g_ExFlashBuf[0] = 0x01;
					//	wTotalPn = ByteToWord(g_ExFlashBuf+1) + wPnNum;
					//	WordToByte(wTotalPn, g_ExFlashBuf+1);
					//	MakeFile(FILE_BATTASK_STATUS, g_ExFlashBuf);
					//	writefile(FILE_BATTASK_STATUS, 0, g_ExFlashBuf);
					//}
					SignalSemaphore(g_semExFlashBuf);
					return true;
				}
				else
				{
					wSectOff += 4+wTaskLen;
					bSect++;
					continue;
				}
			}
			else	//�ο�ʼ��־����
			   break;
		}
		bFlashSect++;
	}
	
    SignalSemaphore(g_semExFlashBuf);
    
	return false;
	
}

//ȡ���������֤�������ݺ���
bool GetBatMtrTask0Data(TBatTask0* pBatTask0, BYTE* bP2)
{
	
	WORD wTaskLen, wTaskDataLen, wPn;
	WORD wPnNum;
	WORD wOffset, wSectOff;
	bool fGetCiph = false;
	BYTE bFlashSect = 0;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	while(bFlashSect < 2)
	{
		memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
		if(!readfile(FILE_BATTASK0 ,0, g_ExFlashBuf, -1))
		{
			SignalSemaphore(g_semExFlashBuf);
			return false;
		}
			
		if(!CheckFile(FILE_BATTASK0, g_ExFlashBuf, 0))
		{
			SignalSemaphore(g_semExFlashBuf);
			return false;
		} 
	   
		wSectOff = 0;
		while(wSectOff < EXSECT_SIZE)
		{
			//0x55 0xaa + 2���ֽڵ�֡�� + ��֡����
			if (g_ExFlashBuf[wSectOff]==0x55 && g_ExFlashBuf[wSectOff+1]==0xaa)
			{
				wTaskLen = ByteToWord(&g_ExFlashBuf[wSectOff+2]);
				wSectOff += 4; //����ǰ���ֽ�
				
				wTaskDataLen = ByteToWord(&g_ExFlashBuf[wSectOff+3]);
				
				if (wTaskDataLen==0x02 && IsAllAByte(&g_ExFlashBuf[wSectOff+5+2+2], 0xaa, 8)) //�������ݳ���Ϊ0x02 �� ��������Ϊ0x00 0x00
				{	//�����ʽΪ��0000���������ͣ�00���������ݳ���Ln��0002��
					//��������:0000��ע��ͬʡ��˾�������ݲ�ͬ�����������Ϊ0000�����������õ������n��0001
					//IsAllAByte(&bTaskBuf[5], 0x00, 2)
					//ͨ�ñ��
					//�Ѵ�������+��ʽ+����+���ݳ���+����+���ε��ܱ�����
					wOffset = wSectOff + 5 + 2 + 2;
					memrcpy(pBatTask0->bMtrKeyCiph, &g_ExFlashBuf[wOffset+8], 32); //�����Կ����
					pBatTask0->wTaskDataLen = 2; //�������ݳ���
					memrcpy(pBatTask0->bTaskData, &g_ExFlashBuf[wSectOff+5], pBatTask0->wTaskDataLen);//��������
					*bP2 = 2;
					fGetCiph = true;
				}
				else
				{
					wOffset = wSectOff + 2 + 1 + 2 + wTaskDataLen;
					wPnNum = ByteToWord(&g_ExFlashBuf[wOffset]);
    				wOffset += 2;
	    			for (wPn=0; wPn<wPnNum; wPn++)
					{
						if (memcmp(pBatTask0->bAddr, &g_ExFlashBuf[wOffset], 8) == 0) //�ҵ����
						{
							pBatTask0->wTaskDataLen = wTaskDataLen;//�������ݳ���
							memrcpy(pBatTask0->bTaskData, &g_ExFlashBuf[wSectOff + 5], wTaskDataLen);//��������
							
							memrcpy(pBatTask0->bMtrKeyCiph, &g_ExFlashBuf[wOffset+8], 32);
							*bP2 = 1;
							SignalSemaphore(g_semExFlashBuf);
							return true;
						}
						wOffset += (8 + 32);
					}
				}
		
				wSectOff += wTaskLen;
			}
			else
			  break;
		}
		bFlashSect++;
	}
	
	SignalSemaphore(g_semExFlashBuf);
	return fGetCiph;
}

//����:��645��������֡
//pbAddr : �����
//bTaskID: 0_�����֤����1_��ʱ����
//pbFrm�� ������������
//pbGetData:  ���������
BYTE Trans645BatTaskFrm(BYTE* pbAddr, BYTE bTaskID, BYTE bFrmLen, BYTE* pbFrm, BYTE* pbRetFrm)
{
	BYTE b645Addr[8];
	BYTE bCmdFrm[128];
	BYTE bBuf[128];
	bool fBegin = false;
	WORD wRetLen = 0;
	WORD wErrCnt = 30;
	TCommPara CommPara;
	WORD wPn;
	BYTE bGetDataLen;
	BYTE bCs = 0x00;
	int i = 0;
	DWORD dwTmpClick, dwLen;

	memcpy(b645Addr, pbAddr, sizeof(b645Addr));
	wPn = MtrAddrToPn(b645Addr); //ͨ�����ַת��Ϊ�������

    if(!IsV07Mtr(wPn))//wPn����ֵ�Ƿ���Ч
    	return 0;
	

	ReadItemEx(BN0, wPn, 0x8902, bBuf);

	for (i=0; i<4; i++)
		bCmdFrm[i] = 0xFE;

	bCmdFrm[4] = 0x68;
	memcpy(bCmdFrm+5, b645Addr, 6);
	bCmdFrm[11] = 0x68;
	if(bTaskID == 0)
	{
		bCmdFrm[12] = 0x03;//������
		bCmdFrm[13] = 0x20;//���ݳ���
		//���ݱ�ʶ
		bCmdFrm[14] = 0xFF;
		bCmdFrm[15] = 0x00;
		bCmdFrm[16] = 0x00;
		bCmdFrm[17] = 0x07;

		//�����ߴ���(������)
		bCmdFrm[18] = 0x78;
		bCmdFrm[19] = 0x56;
		bCmdFrm[20] = 0x34;
		bCmdFrm[21] = 0x12;

		//����ER1+�����R1+��ɢ���ӣ���ţ�
		memcpy(&bCmdFrm[22], pbFrm, bFrmLen); //����ER1+�����R1
		memcpy(&bCmdFrm[22+bFrmLen], b645Addr, sizeof(b645Addr));
	}
	else if(bTaskID == 1)
	{
		bCmdFrm[12] = 0x14;//������
		bCmdFrm[13] = 0x20;//���ݳ���
		//���ݱ�ʶ
		/* 0x0C; 0x01;0x00;0x04;*/
		memcpy(&bCmdFrm[14], pbFrm, 4);
		pbFrm += 4;

		//PA ����
		bCmdFrm[18] = 0x98;
		bCmdFrm[19] = 0x00;
		bCmdFrm[20] = 0x00;
		bCmdFrm[21] = 0x00;


		//�����ߴ���(������)
		bCmdFrm[22] = 0x78;
		bCmdFrm[23] = 0x56;
		bCmdFrm[24] = 0x34;
		bCmdFrm[25] = 0x12;

		//����
		memcpy(&bCmdFrm[26], pbFrm, 16); //����ER1+�����R1
		pbFrm += 16;
		memcpy(&bCmdFrm[42], pbFrm, 4);
	}
	else if(bTaskID == 2)
	{
		bCmdFrm[12] = 0x11;//������
		bCmdFrm[13] = 0x04;//���ݳ���
		//���ݱ�ʶ


		//ID
		bCmdFrm[14] = 0x00;
		bCmdFrm[15] = 0x04;
		bCmdFrm[16] = 0x30;
		bCmdFrm[17] = 0x03;

	}

	for(i=0; i<bCmdFrm[13]; i++)
	{
		bCmdFrm[14+i] += 0x33;
	}

	for(i=0; i<bCmdFrm[13]+10; i++)
	{
		bCs += bCmdFrm[i+4]; 
	}

	bCmdFrm[bCmdFrm[13]+14] = bCs;
	bCmdFrm[bCmdFrm[13]+15] = 0x16;

	i = MeterPortToPhy(bBuf[4]&0x1f);
	if (i < 0)
	{
		DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: phy port err.\r\n"));
		return 0;
	}
	CommPara.wPort = i;//MeterPortToPhy(bBuf[4]&0x1f);

	CommPara.dwBaudRate = GbValToBaudrate(bBuf[MTR_BAUD_OFFSET]>>5);	
	CommPara.bParity =  EVENPARITY;	
	CommPara.bByteSize = 8;
	CommPara.bStopBits = ONESTOPBIT;	
	if (CommPara.dwBaudRate == 0)
		CommPara.dwBaudRate = CBR_2400;

    memset(bBuf, 0x00, sizeof(bBuf));
    //GetDirRdCtrl();	//ȡ��ֱ���Ŀ���Ȩ
	if ( !MtrProOpenComm(&CommPara) )
	{
		//ReleaseDirRdCtrl(); //�ͷ�ֱ���Ŀ���Ȩ
		return 0;
	}
	
	CommRead(CommPara.wPort, NULL, 0, 300); //�����һ�´���

	TraceBuf(DB_FAFRM, "DoForward tx -->", bCmdFrm, bCmdFrm[13]+16);//xzz  ���ܲ����ɾ��
	if (CommWrite(CommPara.wPort, bCmdFrm, bCmdFrm[13]+16, 1000) != (bCmdFrm[13]+16))
	{
		DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: first fail to write comm.\r\n")); 
		if (CommWrite(CommPara.wPort, bCmdFrm, bCmdFrm[13]+16, 1000) != (bCmdFrm[13]+16))
		{
			//ReleaseDirRdCtrl(); //�ͷ�ֱ���Ŀ���Ȩ
			DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: second fail to write comm.\r\n"));
			return 0;
		}
	}
	
	i = 0;
	wRetLen = 0;
	dwTmpClick = GetTick();
	while (GetTick()-dwTmpClick < 10000)    //n�γ��Զ�ȡ����
	{
		dwLen = CommRead(CommPara.wPort, bBuf+wRetLen, sizeof(bBuf)-wRetLen, 200);

		if (dwLen > 0)
		{
		    i = 0;
			wRetLen += (WORD)dwLen;
			fBegin = true;
		}
		else
		{
			i++; //������ͨ�ż���
			
		#ifdef SYS_LINUX
			//20090817 ARMƽ̨�³�֡��Ҫ��3�β�������
			//if (dwTimeOut <= 300) //������ʱ�Ͽ��Э�飬�ɶ��һ���Ա�֤��֡ʱ�����ݵĿɿ���
			wErrCnt = 2;
		#endif

			if (fBegin && i>wErrCnt)
				break;
		}					
	}
	//ReleaseDirRdCtrl(); //�ͷ�ֱ���Ŀ���Ȩ

	DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: wRetLen = %d.\r\n", wRetLen));
	TraceBuf(DB_FAFRM, "DoForward rx <--", bBuf, wRetLen);//xzz  ���ܲ����ɾ��

	if (wRetLen == 0)
		return 0;

	for ( i=0; i<wRetLen; i++)
	{
		if (bBuf[i] == 0x68)
			break;
	}

	if (i+12 > wRetLen)
	{
		DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: wRetLen err.\r\n"));
		return 0;
	}

	memcpy(pbRetFrm, &bBuf[i],wRetLen-i);
	
	//����֡��У����ж�
	bCs = 0x00;
	for(i=0; i<pbRetFrm[9]+10; i++)
	   bCs += pbRetFrm[i];
	   
	if(bCs != pbRetFrm[i])
	{
		DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: bCs err.\r\n"));
	 	return 0;
	}
	 	
	bGetDataLen =  pbRetFrm[9]+12;

	for (i=0; i<pbRetFrm[9]; i++)
	{
		pbRetFrm[10+i] -= 0x33;
	}
	//TraceBuf(DB_FAFRM, "DoForward rx <--", pbRetFrm, bGetDataLen);//xzz  ���ܲ����ɾ��

	return bGetDataLen;		//��������645����֡�ĳ���
}

bool DoBatMtrCmd()
{
	WORD wMtrIdx = 0;
	TBatTask0  tBatTask0;
	TBatTask1  tBatTask1;
	BYTE* pbInfo;
	BYTE bCmdFrm[128];
	BYTE bRetFrm[256];
	BYTE bP2 = 1, bR1[16], bER1[16], bR2[16], bIsDoBatTask = 0;
	BYTE b645DataLen = 0, i;
	WORD wOKPn, wPn = 0;
	BYTE bPort;
	const BYTE bTaskFmt[2] = {0x04, 0x04};

	pbInfo = g_PowerOffTmp.bBatTaskInfo;

	wMtrIdx = ByteToWord(pbInfo+3);
	if (*(pbInfo+1) == 0)	//û���������
		return false;

	if (g_PowerOffTmp.bBatTaskInfo[2] >= *(pbInfo+1))
	{
		//bSect = 0;
		//g_bRxBatTask1Cnt = 0;//��������������0
		//g_bRxBatTask0Cnt = 0;
		return false;
	}

	//�˶δ������ţ������϶�Ҫ��
	ReadItemEx(BN0, PN0, 0x07ff, &bIsDoBatTask);//�ն˶Ե��ܱ��ʱ���ܿ���  0���ر� 1������
	if(0 == bIsDoBatTask)
	{
	  	DTRACE(DB_FAFRM, ("DoBatMtrCmd: Mtr. Tm Switch 0ff......\r\n"));
		return false;
	}

	if (!GetBatMtrTask1Data(&tBatTask1, &wMtrIdx, g_PowerOffTmp.bBatTaskInfo+2))
	{
		//g_bRxBatTask1Cnt = 0;//��������������0
		//g_bRxBatTask0Cnt = 0;
		return false;
	}

	memrcpy(tBatTask0.bAddr, tBatTask1.bAddr, 8);
	bPort = 0;
	wPn = MtrAddrToPn(tBatTask0.bAddr); //ͨ�����ַת��Ϊ�������
	if (wPn == 0)//wPn����ֵ�Ƿ���Ч
	{
		DTRACE(DB_FAFRM, ("DoBatMtrCmd:: wPn is %d!!! \r\n", wPn));
		WordToByte(wMtrIdx, pbInfo+3);
		return true;//��Ȼ�����ˣ�����Ϊ�˽�����һ���������Ƿ�����
	}
	else
	{
		bPort = GetPnPort(wPn);
		if(bPort == 0)
		{
			DTRACE(DB_FAPROTO, ("DoBatMtrCmd:: wPn_Port is Zero !!! \r\n"));
			WordToByte(wMtrIdx, pbInfo+3);
			return true;//��Ȼ�����ˣ�����Ϊ�˽�����һ���������Ƿ�����
		}
	}

	if (!GetBatMtrTask0Data(&tBatTask0, &bP2))
	{
		//g_bRxBatTask1Cnt = 0;//��������������0
		//g_bRxBatTask0Cnt = 0;
	  
		return true;//��Ȼ�����ˣ�����Ϊ�˽�����һ���������Ƿ�����
	}
	
	//memrcpy(tBatTask0.bAddr, tBatTask1.bAddr, 8);

	EsamGetMtrAuth(bP2, tBatTask0.bMtrKeyCiph, tBatTask1.bAddr, tBatTask0.bTaskData, bR1, bER1);
	//��bR1+bER1ת��������ȡ R2
	memrcpy(bCmdFrm, bER1, 8);  //��λ sizeof(bER1)
	memrcpy(bCmdFrm+8, bR1, 8); //��λ sizeof(bR1)
	b645DataLen = Trans645BatTaskFrm(tBatTask0.bAddr, 0, 16, bCmdFrm, bRetFrm);

	if (b645DataLen>0 || g_fTermTestMode)
	{
		if(bRetFrm[8]==0x83 && !g_fTermTestMode)
		{
			WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
			memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
			readfile(FILE_BATTASK_STATUS, 0, g_ExFlashBuf, -1);

			wOKPn = ByteToWord(g_ExFlashBuf+3);
			for(i=0; i<wOKPn; i++)
			{
				if(memcmp(tBatTask0.bAddr, g_ExFlashBuf+5+8*i, 8)==0)
					break;
			}
			if(i == wOKPn)
			{
				wOKPn += 1;
				memcpy(g_ExFlashBuf+3, (BYTE*)&wOKPn, 2);
				memcpy(g_ExFlashBuf+5+((wOKPn-1)<<3), tBatTask0.bAddr, 8);
				MakeFile(FILE_BATTASK_STATUS, g_ExFlashBuf);
				writefile(FILE_BATTASK_STATUS, 0, g_ExFlashBuf);
			}
			SignalSemaphore(g_semExFlashBuf);
		}

		memrcpy(bR2, bRetFrm+14, 4); //��λ
	}
	else
	{
		///��������һ�����������Ϣ�������һֱ�ظ�
		WordToByte(wMtrIdx, pbInfo+3);
		return true;//��Ȼ�����ˣ�����Ϊ�˽�����һ���������Ƿ�����
	}

	b645DataLen = 0;

	b645DataLen = EsamGetAdjTmCiph(bTaskFmt, tBatTask1.bTaskData, tBatTask1.wTaskDataLen, tBatTask1.bMtrKeyCiph, bR2, bRetFrm);
	
	if (b645DataLen>0 || g_fTermTestMode)
	{
	   memrcpy(bCmdFrm, bRetFrm, 4);//ID
	   memrcpy(bCmdFrm+4, bRetFrm+4, 16);//����
	   memrcpy(bCmdFrm+20, bRetFrm+20, 4);//MAC
	}
	else
	   return true;//��Ȼ�����ˣ�����Ϊ�˽�����һ���������Ƿ�����
	
	b645DataLen = 0;
	b645DataLen = Trans645BatTaskFrm(tBatTask0.bAddr, 1, 24, bCmdFrm, bRetFrm);

	
    if(g_fTermTestMode)
	{
		//if (b645DataLen>0/* && bRetFrm[8]==0x94*/)	//����Ӧ��
		//{
			DWORD dwTimes =0;
			wPn =0;
			memset(bCmdFrm, 0x00, sizeof(bCmdFrm));
			
			Sleep(5000);//��ȡ���Уʱ����ʱ�ȵȴ�һ�ᣬ��̨�Ӹ����ܱ����óɹ�
			///����645֡ȡ���Уʱ����
			b645DataLen = Trans645BatTaskFrm(tBatTask0.bAddr, 2, 0 ,bCmdFrm, bRetFrm);
			if(b645DataLen>0)
			{
				if(bRetFrm[8] == 0x91)
				{
					dwTimes = BcdToDWORD(bRetFrm+14, 3);//Уʱ�ܴ���
				}
			}

			wPn = MtrAddrToPn(tBatTask0.bAddr);
			//if(g_dwTimes[wPn] != dwTimes)
			if(g_fTermTestMode)
			{
				WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
				memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
				if(readfile(FILE_BATTASK_STATUS,0, g_ExFlashBuf, -1))
				{
					if(CheckFile(FILE_BATTASK_STATUS, g_ExFlashBuf, 0))
					{
						WORD wOKPn = ByteToWord(g_ExFlashBuf+2048+3);
						wOKPn += 1;
						memcpy(g_ExFlashBuf+2048+3, (BYTE*)&wOKPn, 2);
						memcpy(g_ExFlashBuf+5+2048+((wOKPn-1)<<3), tBatTask0.bAddr, 8);
						wOKPn = ByteToWord(g_ExFlashBuf+3);
						for(i=0; i<wOKPn; i++)
						{
							if(memcmp(tBatTask0.bAddr, g_ExFlashBuf+5+8*i, 8)==0)
								break;
						}
						if(i == wOKPn)
						{
							wOKPn += 1;
							memcpy(g_ExFlashBuf+3, (BYTE*)&wOKPn, 2);
							memcpy(g_ExFlashBuf+5+((wOKPn-1)<<3), tBatTask0.bAddr, 8);
						}
						MakeFile(FILE_BATTASK_STATUS, g_ExFlashBuf);
						writefile(FILE_BATTASK_STATUS, 0, g_ExFlashBuf);	
					}
				}
				SignalSemaphore(g_semExFlashBuf);
			}

	     //}
	}
	else
	{
		if (b645DataLen>0 && bRetFrm[8]==0x94)	//����Ӧ��
		{
			WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
			memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
			if(readfile(FILE_BATTASK_STATUS,0, g_ExFlashBuf, -1))
			{
				if(CheckFile(FILE_BATTASK_STATUS, g_ExFlashBuf, 0))
				{
					WORD wOKPn = ByteToWord(g_ExFlashBuf+3);
					wOKPn += 1;
					memcpy(g_ExFlashBuf+2048+3, (BYTE*)&wOKPn, 2);
					memcpy(g_ExFlashBuf+5+2048+((wOKPn-1)<<3), tBatTask0.bAddr, 8);
					MakeFile(FILE_BATTASK_STATUS, g_ExFlashBuf);
					writefile(FILE_BATTASK_STATUS, 0, g_ExFlashBuf);	
				}
			}
			SignalSemaphore(g_semExFlashBuf);
		}

	}
	

	WordToByte(wMtrIdx, pbInfo+3);
	return true;//��Ȼ�����ˣ�����Ϊ�˽�����һ���������Ƿ�����
}