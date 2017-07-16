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

//取批量任务数据函数
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
		    //如果2个扇区读取错误，则不继续进行读取
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
			//0x55 0xaa + 2个字节的帧长 + 本帧内容
			if (g_ExFlashBuf[wSectOff]==0x55 && g_ExFlashBuf[wSectOff+1]==0xaa)
			{
				wTaskLen = ByteToWord(&g_ExFlashBuf[wSectOff+2]);
				if (bSect == *pwSect)	//已经到达需要处理的部分
				{	
					if (wTaskLen == 0)
						break;//为释放信号量，循环全部用break退出
						
					wSectOff += 4;
					pBatTask1->wTaskDataLen = ByteToWord(&g_ExFlashBuf[wSectOff+3]);
					
					if (pBatTask1->wTaskDataLen > sizeof(pBatTask1->bTaskData))//长度非法判断
					{
					    SignalSemaphore(g_semExFlashBuf);
					    return false;
					}
						
					memrcpy(pBatTask1->bTaskData, &g_ExFlashBuf[wSectOff+5], pBatTask1->wTaskDataLen);
					//注意下面 要使用 wSectoff
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
					else if((*pwMtrIdx) > wPnNum)	//超过本段的测量点数量
						break;
	
					//if(fIsNewSect)
					//{
					//	//新的bTaskPart处理 需要将处理总台数计入文件
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
			else	//段开始标志错误
			   break;
		}
		bFlashSect++;
	}
	
    SignalSemaphore(g_semExFlashBuf);
    
	return false;
	
}

//取批量身份认证任务数据函数
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
			//0x55 0xaa + 2个字节的帧长 + 本帧内容
			if (g_ExFlashBuf[wSectOff]==0x55 && g_ExFlashBuf[wSectOff+1]==0xaa)
			{
				wTaskLen = ByteToWord(&g_ExFlashBuf[wSectOff+2]);
				wSectOff += 4; //跳过前导字节
				
				wTaskDataLen = ByteToWord(&g_ExFlashBuf[wSectOff+3]);
				
				if (wTaskDataLen==0x02 && IsAllAByte(&g_ExFlashBuf[wSectOff+5+2+2], 0xaa, 8)) //任务数据长度为0x02 且 任务数据为0x00 0x00
				{	//任务格式为：0000，任务类型：00，任务数据长度Ln：0002，
					//任务数据:0000（注不同省公司任务数据不同，测试情况下为0000），本次配置电表数量n：0001
					//IsAllAByte(&bTaskBuf[5], 0x00, 2)
					//通用表号
					//已处理数据+格式+类型+数据长度+数据+本次电能表数量
					wOffset = wSectOff + 5 + 2 + 2;
					memrcpy(pBatTask0->bMtrKeyCiph, &g_ExFlashBuf[wOffset+8], 32); //表的密钥密文
					pBatTask0->wTaskDataLen = 2; //任务数据长度
					memrcpy(pBatTask0->bTaskData, &g_ExFlashBuf[wSectOff+5], pBatTask0->wTaskDataLen);//任务数据
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
						if (memcmp(pBatTask0->bAddr, &g_ExFlashBuf[wOffset], 8) == 0) //找到表号
						{
							pBatTask0->wTaskDataLen = wTaskDataLen;//任务数据长度
							memrcpy(pBatTask0->bTaskData, &g_ExFlashBuf[wSectOff + 5], wTaskDataLen);//任务数据
							
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

//描述:组645批量任务帧
//pbAddr : 电表表号
//bTaskID: 0_身份认证任务，1_对时任务
//pbFrm： 部分数据内容
//pbGetData:  电表返回数据
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
	wPn = MtrAddrToPn(b645Addr); //通过表地址转换为测量点号

    if(!IsV07Mtr(wPn))//wPn返回值是否有效
    	return 0;
	

	ReadItemEx(BN0, wPn, 0x8902, bBuf);

	for (i=0; i<4; i++)
		bCmdFrm[i] = 0xFE;

	bCmdFrm[4] = 0x68;
	memcpy(bCmdFrm+5, b645Addr, 6);
	bCmdFrm[11] = 0x68;
	if(bTaskID == 0)
	{
		bCmdFrm[12] = 0x03;//功能码
		bCmdFrm[13] = 0x20;//数据长度
		//数据标识
		bCmdFrm[14] = 0xFF;
		bCmdFrm[15] = 0x00;
		bCmdFrm[16] = 0x00;
		bCmdFrm[17] = 0x07;

		//操作者代码(待改正)
		bCmdFrm[18] = 0x78;
		bCmdFrm[19] = 0x56;
		bCmdFrm[20] = 0x34;
		bCmdFrm[21] = 0x12;

		//密文ER1+随机数R1+分散因子（表号）
		memcpy(&bCmdFrm[22], pbFrm, bFrmLen); //密文ER1+随机数R1
		memcpy(&bCmdFrm[22+bFrmLen], b645Addr, sizeof(b645Addr));
	}
	else if(bTaskID == 1)
	{
		bCmdFrm[12] = 0x14;//功能码
		bCmdFrm[13] = 0x20;//数据长度
		//数据标识
		/* 0x0C; 0x01;0x00;0x04;*/
		memcpy(&bCmdFrm[14], pbFrm, 4);
		pbFrm += 4;

		//PA 密码
		bCmdFrm[18] = 0x98;
		bCmdFrm[19] = 0x00;
		bCmdFrm[20] = 0x00;
		bCmdFrm[21] = 0x00;


		//操作者代码(待改正)
		bCmdFrm[22] = 0x78;
		bCmdFrm[23] = 0x56;
		bCmdFrm[24] = 0x34;
		bCmdFrm[25] = 0x12;

		//密文
		memcpy(&bCmdFrm[26], pbFrm, 16); //密文ER1+随机数R1
		pbFrm += 16;
		memcpy(&bCmdFrm[42], pbFrm, 4);
	}
	else if(bTaskID == 2)
	{
		bCmdFrm[12] = 0x11;//功能码
		bCmdFrm[13] = 0x04;//数据长度
		//数据标识


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
    //GetDirRdCtrl();	//取得直抄的控制权
	if ( !MtrProOpenComm(&CommPara) )
	{
		//ReleaseDirRdCtrl(); //释放直抄的控制权
		return 0;
	}
	
	CommRead(CommPara.wPort, NULL, 0, 300); //先清除一下串口

	TraceBuf(DB_FAFRM, "DoForward tx -->", bCmdFrm, bCmdFrm[13]+16);//xzz  加密测过后删除
	if (CommWrite(CommPara.wPort, bCmdFrm, bCmdFrm[13]+16, 1000) != (bCmdFrm[13]+16))
	{
		DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: first fail to write comm.\r\n")); 
		if (CommWrite(CommPara.wPort, bCmdFrm, bCmdFrm[13]+16, 1000) != (bCmdFrm[13]+16))
		{
			//ReleaseDirRdCtrl(); //释放直抄的控制权
			DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: second fail to write comm.\r\n"));
			return 0;
		}
	}
	
	i = 0;
	wRetLen = 0;
	dwTmpClick = GetTick();
	while (GetTick()-dwTmpClick < 10000)    //n次尝试读取数据
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
			i++; //连续无通信计数
			
		#ifdef SYS_LINUX
			//20090817 ARM平台下长帧需要读3次才能完整
			//if (dwTimeOut <= 300) //若读延时较快的协议，可多读一次以保证断帧时收数据的可靠性
			wErrCnt = 2;
		#endif

			if (fBegin && i>wErrCnt)
				break;
		}					
	}
	//ReleaseDirRdCtrl(); //释放直抄的控制权

	DTRACE(DB_FAPROTO, ("Trans645BatTaskFrm: wRetLen = %d.\r\n", wRetLen));
	TraceBuf(DB_FAFRM, "DoForward rx <--", bBuf, wRetLen);//xzz  加密测过后删除

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
	
	//接收帧的校验和判断
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
	//TraceBuf(DB_FAFRM, "DoForward rx <--", pbRetFrm, bGetDataLen);//xzz  加密测过后删除

	return bGetDataLen;		//返回整个645完整帧的长度
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
	if (*(pbInfo+1) == 0)	//没有任务可做
		return false;

	if (g_PowerOffTmp.bBatTaskInfo[2] >= *(pbInfo+1))
	{
		//bSect = 0;
		//g_bRxBatTask1Cnt = 0;//接受任务数据置0
		//g_bRxBatTask0Cnt = 0;
		return false;
	}

	//此段代码留着，后续肯定要用
	ReadItemEx(BN0, PN0, 0x07ff, &bIsDoBatTask);//终端对电能表对时功能开关  0：关闭 1：开启
	if(0 == bIsDoBatTask)
	{
	  	DTRACE(DB_FAFRM, ("DoBatMtrCmd: Mtr. Tm Switch 0ff......\r\n"));
		return false;
	}

	if (!GetBatMtrTask1Data(&tBatTask1, &wMtrIdx, g_PowerOffTmp.bBatTaskInfo+2))
	{
		//g_bRxBatTask1Cnt = 0;//接受任务数据置0
		//g_bRxBatTask0Cnt = 0;
		return false;
	}

	memrcpy(tBatTask0.bAddr, tBatTask1.bAddr, 8);
	bPort = 0;
	wPn = MtrAddrToPn(tBatTask0.bAddr); //通过表地址转换为测量点号
	if (wPn == 0)//wPn返回值是否有效
	{
		DTRACE(DB_FAFRM, ("DoBatMtrCmd:: wPn is %d!!! \r\n", wPn));
		WordToByte(wMtrIdx, pbInfo+3);
		return true;//虽然错误了，但是为了进行下一块表的任务还是返回真
	}
	else
	{
		bPort = GetPnPort(wPn);
		if(bPort == 0)
		{
			DTRACE(DB_FAPROTO, ("DoBatMtrCmd:: wPn_Port is Zero !!! \r\n"));
			WordToByte(wMtrIdx, pbInfo+3);
			return true;//虽然错误了，但是为了进行下一块表的任务还是返回真
		}
	}

	if (!GetBatMtrTask0Data(&tBatTask0, &bP2))
	{
		//g_bRxBatTask1Cnt = 0;//接受任务数据置0
		//g_bRxBatTask0Cnt = 0;
	  
		return true;//虽然错误了，但是为了进行下一块表的任务还是返回真
	}
	
	//memrcpy(tBatTask0.bAddr, tBatTask1.bAddr, 8);

	EsamGetMtrAuth(bP2, tBatTask0.bMtrKeyCiph, tBatTask1.bAddr, tBatTask0.bTaskData, bR1, bER1);
	//将bR1+bER1转发给电表获取 R2
	memrcpy(bCmdFrm, bER1, 8);  //倒位 sizeof(bER1)
	memrcpy(bCmdFrm+8, bR1, 8); //倒位 sizeof(bR1)
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

		memrcpy(bR2, bRetFrm+14, 4); //倒位
	}
	else
	{
		///出错后更新一下任务处理的信息，否则会一直重复
		WordToByte(wMtrIdx, pbInfo+3);
		return true;//虽然错误了，但是为了进行下一块表的任务还是返回真
	}

	b645DataLen = 0;

	b645DataLen = EsamGetAdjTmCiph(bTaskFmt, tBatTask1.bTaskData, tBatTask1.wTaskDataLen, tBatTask1.bMtrKeyCiph, bR2, bRetFrm);
	
	if (b645DataLen>0 || g_fTermTestMode)
	{
	   memrcpy(bCmdFrm, bRetFrm, 4);//ID
	   memrcpy(bCmdFrm+4, bRetFrm+4, 16);//密文
	   memrcpy(bCmdFrm+20, bRetFrm+20, 4);//MAC
	}
	else
	   return true;//虽然错误了，但是为了进行下一块表的任务还是返回真
	
	b645DataLen = 0;
	b645DataLen = Trans645BatTaskFrm(tBatTask0.bAddr, 1, 24, bCmdFrm, bRetFrm);

	
    if(g_fTermTestMode)
	{
		//if (b645DataLen>0/* && bRetFrm[8]==0x94*/)	//正常应答
		//{
			DWORD dwTimes =0;
			wPn =0;
			memset(bCmdFrm, 0x00, sizeof(bCmdFrm));
			
			Sleep(5000);//读取电表校时次数时先等待一会，让台子给电能表设置成功
			///组电表645帧取电表校时次数
			b645DataLen = Trans645BatTaskFrm(tBatTask0.bAddr, 2, 0 ,bCmdFrm, bRetFrm);
			if(b645DataLen>0)
			{
				if(bRetFrm[8] == 0x91)
				{
					dwTimes = BcdToDWORD(bRetFrm+14, 3);//校时总次数
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
		if (b645DataLen>0 && bRetFrm[8]==0x94)	//正常应答
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
	return true;//虽然错误了，但是为了进行下一块表的任务还是返回真
}