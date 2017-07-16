/*********************************************************************************************************
* Copyright (c) 2009,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：StdReader.c
* 摘    要：本文件实现了376.2载波路由的控制基类(自动路由控制)
* 当前版本：2.0
* 作    者：岑坚宇
* 完成日期：2014年8月

* 备    注：$本文件为标准库的一部分,请不要将协议相关的部分加入到本文件
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
	
	ptStdReader->m_tCctCommPara.m_tCommPara.wPort = COMM_METER3;//有待处理
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

	ptStdReader->m_dwResumeRdClick = 0;	//上回路由不抄表,恢复抄表的时标
	ptStdReader->m_dwUpdStatusClick = 0;	//上回更新状态的时标

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

//描述:设置帧地址域
void SetAddrField(TStdReader* ptStdReader, BYTE* pbTxBuf, BYTE* pbAddr)
{
	memcpy(&pbTxBuf[FRM_ADDR], ptStdReader->m_ptStdPara->RdrPara.bMainAddr, 6);
	memcpy(&pbTxBuf[FRM_ADDR+6], pbAddr, 6);
}

WORD MakeFrm(WORD wFrmLenBytes, BYTE* pbTxBuf, BYTE bAfn, WORD wFn, BYTE* pbData, WORD wDataLen, BYTE bAddrLen, BYTE bMode, BYTE bPRM, BYTE bCn, BYTE bRcReserved)
{   
	BYTE bLenBytes = wFrmLenBytes-1;	//帧长度字节数	

	//信息域R	
	pbTxBuf[3 + bLenBytes] = bAddrLen==0?0:4; //D2通信模块标识：0表示对集中器的通信模块操作;1表示对载波表的通信模块操作。
	pbTxBuf[4 + bLenBytes] = bCn;		//信道
	pbTxBuf[5 + bLenBytes] = 0x28;
	pbTxBuf[6 + bLenBytes] = 0;//默认通信速率//0x32;
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

//描述:组645抄读数据帧
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

	//是否需要在645数据域中添加采集器地址应由应用层统一处理，不应放到通用接口中
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

//描述:抄读电量
//参数：bRtMod通信模块标识 0：对集中器的通信模块操作，1：对载波表的通信模块操作
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

	if (bRdFlg == RD_FLG_TORD)	//可以抄读
	{
		//组数据内容
		GetCctMeterAddr(wPn, bMtrAddr);
		bData[0] = RD_FLG_TORD;					//抄读标志
		bData[1] = MakeCct645AskItemFrm(bMtrPro, pbAddr, dwID, &bData[2], fByAcqUnit, bMtrAddr, 0);
		bData[bData[1]+2] = 0;	//载波从节点附属节点数量n

		//组路由通信帧
		SetAddrField(ptStdReader, pbTxBuf, pbAddr);

		if (bRtFlg == TO_MOD_RT) //对集中器的通信模块操作无地址域
			dwFrmLen = MakeFrm(wFrmLenBytes, pbTxBuf, AFN_RTRD, 1, bData, bData[1]+3, 0, FRM_C_M_MST, FRM_C_PRM_0, bCn, 0);
		else
			dwFrmLen = MakeFrm(wFrmLenBytes, pbTxBuf, AFN_RTRD, 1, bData, bData[1]+3, 12, FRM_C_M_MST, FRM_C_PRM_0, bCn, 0);
		
		if (fByAcqUnit) //带采集器则需上层调用者给出采集器地址
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
	else //(bRdFlg==RD_FLG_SUCC || bRdFlg==RD_FLG_FAIL)	//抄读成功/失败都回成功
	{
		bData[0] = bRdFlg;//RD_FLG_SUCC;		//抄读标志：01H为抄读成功,00H为抄读失败
		bData[1] = 0x0; 			//L＝0
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

//描述:处理帧
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
	//1.以后可以考虑用变量bAddrLen来规避回复帧中带不带地址的造成的差异,
	//  比如m_bCctRxBuf[FRM_AFN]和m_bCctRxBuf[FRM_AFN_A]可以统一成m_bCctRxBuf[FRM_ADDR+bAddrLen]
	if (UpInf.bModule == TO_MOD_RT) //对集中器的通信模块操作,不带地址域
	{
		if (ptStdReader->m_pbCctRxBuf[FRM_AFN]==AFN_RTRD && ptStdReader->m_pbCctRxBuf[FRM_LEN]>FRM_AFN+5) //询问抄读幁
		{
			if (ptStdReader->m_fSyncAllMeter)
			{
				//iSchRet = SearchAnUnReadID(&m_bCctRxBuf[FRM_AFN+4], m_bLinkType, &RwItem);	//这个接口只负责内部ID
				
				//1.先尝试不认地址模式
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
					//只提供0x9010供学习
					if (GetSyncFlag(ptStdReader->m_pbStudyFlag, wTmpPn))//已经学习到了，置成功
						ReadMeter(ptStdReader, ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, &ptStdReader->m_pbCctRxBuf[FRM_AFN+4], wTmpPn, dwID, RD_FLG_SUCC, false, ptStdReader->m_pbCctRxBuf[4+ptStdReader->m_ptStdPara->wFrmLenBytes-1]);
					else
						ReadMeter(ptStdReader, ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, &ptStdReader->m_pbCctRxBuf[FRM_AFN+4], wTmpPn, dwID, RD_FLG_TORD, false, ptStdReader->m_pbCctRxBuf[4+ptStdReader->m_ptStdPara->wFrmLenBytes-1]);

					
				}
			}
			ptStdReader->m_ptAutoReader->m_wMainSleep = 50;

			return true;
		}
		else if (ptStdReader->m_pbCctRxBuf[FRM_AFN]==AFN_REP && ptStdReader->m_pbCctRxBuf[FRM_LEN]>FRM_AFN+5) //东软上报抄读数据没地址域
		{
			if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == F2)//简单处理成认为学习成功
 			{
				wTmpPn = PlcAddrToPn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+8], NULL);
				if (wTmpPn>0 && wTmpPn<POINT_NUM)
				{
 					SetSyncFlag(ptStdReader->m_pbStudyFlag, (int)wTmpPn);
				}
 			}
			ptStdReader->m_ptAutoReader->m_wMainSleep = 20;		//等待数据回来主睡眠为20ms
			MakeConfirm(ptStdReader);
			return true;
		}
// 		else if (m_bCctRxBuf[FRM_AFN]==AFN_REP && m_bCctRxBuf[FRM_LEN]>FRM_AFN+5) //东软上报抄读数据没地址域
// 		{
// 			if (DtToFn(&m_bCctRxBuf[FRM_AFN+1]) == F2)
// 			{
// 				SaveRepData(RwInf, UpInf, FRM_AFN); //瑞斯康需重载该接口，无需保存数据，直接返回确认帧
// 				m_wMainSleep = 20;		//等待数据回来主睡眠为20ms
// 			}
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN+1]) == F1)	//F1:上报载波从节点信息
// 			{
// 				SaveRepAddr(RepMtrInf); //上报地址
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
// 		else if (m_bCctRxBuf[FRM_AFN]==AFN_QRYDATA && m_bCctRxBuf[FRM_LEN]>FRM_AFN+5) //东软上报抄读数据没地址域
// 		{
// 			//上报的模块运行模式信息长度：13（用户数据区前面的固定长度）+应用数据区中的上报信息至少为40个字节，否则为不完整帧
// 			if (DtToFn(&m_bCctRxBuf[FRM_AFN+1])==F10 && wRxFrmLen>(13+40)) 
// 				RtRunModResolve(&m_bCctRxBuf[FRM_AFN+3]);
// 		}
// 
// 		PlcExtent(&m_bCctRxBuf[FRM_AFN]); //各载波厂家各自扩展的一些AFN的处理
	}
	else //if (UpInf.bModule == TO_MOD_MTR) //对载波表的通信模块操作时才有地址域
	{
		//TODO:是否应该判断一下是否收到的命令是不是'F1:上报载波从节点信息'
		//注意：瑞斯康需验证是否需满足(m_bCctRxBuf[FRM_LEN]>FRM_AFN+5)条件，本流程中未进行验证
		if (ptStdReader->m_pbCctRxBuf[FRM_AFN_A]==AFN_REP && ptStdReader->m_pbCctRxBuf[FRM_LEN]>FRM_AFN+5)
		{
			if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN_A+1]) == F2)	// F2:上报抄读数据
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
// 			else if (DtToFn(&m_bCctRxBuf[FRM_AFN_A+1]) == F2)	// F2:上报抄读数据
// 			{
// 				SaveRepData(RwInf, UpInf, FRM_AFN_A);
// 				return true;
// 			}
// #ifdef EN_NEW_376_2		
// 			//Note:376.2协议新增加内容为AFN=6,F3上报路由工况变动信息
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F3)  //F3上报路由工况变动信息
// 			{
// 				m_bRepRoutInfo=GetRoutInfo(&m_bCctRxBuf[FRM_AFN_A+3]);
// 				return true;
// 			}			
// 			//Note: 376.2协议新增内容AFN=6,F4,上报从节点信息及设备类型
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F4) //F4:上报从节点信息及设备类型
// 			{
// 				//GetNodesInfo(&m_bCctRxBuf[FRM_AFN_A+3],m_tRptInfo);
// 				//合以前程序
// 				SaveSchMtrFrm(FRM_AFN_A);
// 				return true;
// 			}
// 			//Note: 376.2协议新增内容AFN=6,F5上报从节点事件
// 			//分析考虑 SaveRepData值对比，是否能够合并
// 			else if(DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F5) //F5上报从节点事件
// 			{
// 				//调用解析上报从节点事件数据帧内容
// 				//GetMtrRptEvnt(&m_bCctRxBuf[FRM_AFN_A+3],m_tRptMtrEvnt);
// 				//合以前程序
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
// 		//1.目前只有鼎信的模块支持F2:上报抄读数据,
// 		//  但是鼎信错误地把通信模块标识设置为TO_MOD_MTR
// 		//2.晓程不支持支持F2:上报抄读数据,
// 
// 		if (m_bCctRxBuf[FRM_AFN_A]==AFN_REP && 
// 			DtToFn(&m_bCctRxBuf[FRM_AFN_A+1])==F2 && //F2:上报抄读数据,
// 			m_bCctRxBuf[FRM_LEN]>FRM_AFN_A+5) 
// 		{
// 			SaveRepData(RwInf, UpInf, FRM_AFN_A);
// 			m_wMainSleep = 20;		//等待数据回来主睡眠为20ms
// 
// 			return true;
// 		}
	}

	return false;	//没处理到帧,返回false
}

/*
//描述:	接收一个数据块,判断是否接收到一个完整的通信帧
//返回:	返回已经扫描过的字节数,如果收到一个完整的通信帧则返回正数,否则返回负数
int RcvFrame(TStdReader* ptStdReader, BYTE* pbBlock, int nLen)
{
	ptStdReader->m_tCctCommPara.m_nRxStep = 0;    //若帧不完整，循环缓冲区中的数据不会删除，故每次进来都可从头开始解析，直至找到一个完整的帧才会拷贝到m_bCctRxBuf
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
				if(ptStdReader->m_tCctCommPara.m_wRxDataLen > (WORD)nLen   //算出的376.2帧长度已超过传入的数据长度
					|| ptStdReader->m_tCctCommPara.m_wRxDataLen + i - 2 > nLen  //算出的帧尾位置已超过传入的数据长度
					|| bHead[ptStdReader->m_tCctCommPara.m_wRxDataLen + i - 3] != 0x16)  //帧结束符不是0x16
				{
					//这里如果是接受不全或非法的报文，将m_nRxStep置为0，相当于将m_bCctRxBuf中的数据清空，
					//同时，在RxHandleFrm不删除扫描过的数据，等后续收完了再一起拷贝到本函数中重新进行解析    add by CPJ at 2012-10-11
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
				iRxLen = ptStdReader->m_tCctCommPara.m_wRxDataLen - 3 - ptStdReader->m_ptStdPara->wFrmLenBytes; //这里减出来可能会小于0，需加保护 add by CPJ at 2012-16-19

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

//描述:接收并处理帧,每次接收到一个帧就返回
//参数:@dwSeconds 接收等待的秒数
// 	   @pfLeft 用来返回是否有剩余的字节还没处理完
//返回:接收到一个帧则返回true,否则返回false
bool RxHandleFrm(TStdReader* ptStdReader, DWORD dwSeconds)
{
	int len = 0;
	BYTE bBuf[200];
	int nScanLen = 0;
	DWORD dwOldTick = GetTick();
	bool fUseLBuf = ptStdReader->m_ptStdPara->RdrPara.fUseLoopBuf;

	//注意:在本函数及其所调用的默认帧处理函数DefHanleFrm()中,不要使用m_bTxBuf来组发送帧,
	// 	   否则会破坏调用者已经组好的发送帧

	//注意:这里不能将m_nRxStep=0,因为有些函数调用本函数的时候,有可能调用多次来组一个帧
	//	   对于那些要清除之前接收帧字节的情况,请在调用本函数前先调用InitRcv()
	do
	{	
		len = 0;
		if (fUseLBuf)
			len = LoopBufGetBufLen(&ptStdReader->m_tLoopBuf);

		if (len <= 0 || !ptStdReader->m_tCctCommPara.m_fRxComlpete)	//优先处理循环缓冲区的数据，或者缓冲区有数据，但接收不完整，所以应去把剩下的接收回来一起处理
		{				//如果循环缓冲区还有数据，先处理完缓冲区的数据在接收，否则有可能会溢出	
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
					LoopBufDeleteFromBuf(&ptStdReader->m_tLoopBuf, nScanLen); //删除已经扫描的数据

				//if (IsNeedToDropFrm(bBuf, len, nScanLen) == false)	//不需要丢弃帧
				{
					if (DefHanleFrm(ptStdReader) == false) //帧组成功了,但是没有被默认帧处理函数所处理,要出到外面去处理
						return true;
				}

				//NOTE:
				//如果遇到有被默认帧处理函数所处理的帧,则在本函数默默地将它处理掉,
				//外面调用的函数就当成从来没收到过这样的帧,反正这些帧到了外面也不会被处理,
				//然后要继续处理剩下的帧,避免路由模块一下子上来多个默认处理帧,
				//如果不及时处理,会导致后面的帧错位
			}
			else if(len>=sizeof(bBuf)-10) //缓冲区满了，但是还没有一个完整的帧
			{
				if (fUseLBuf)
					LoopBufDeleteFromBuf(&ptStdReader->m_tLoopBuf, len); //删除已经扫描的数据
			}
			//else if (nScanLen<0 && m_fUncompleted)   //不全的报文不应该删除，应该等后面把剩下的报文收全了再从循环缓冲区完整拷贝到RcvFrame去解析     modify by CPJ at 2012-10-11
			//{ 
			// if (m_ptRdrPara->fUseLoopBuf)
			//	 m_LoopBuf.DeleteFromBuf(-nScanLen); //删除已经扫描的数据
			//}
		}

		if (dwSeconds != 0)
			Sleep(100);	//防止程序在这里陷入死循环

	} while (GetTick()-dwOldTick <= dwSeconds*1000);

	return false;
}

//描述：对于返回类型是简单的确认/否认帧（如停止、恢复抄表等命令类交互帧）的发送和接收进行的封装，
//参数：
//      @dwLen：发送长度
//      @dwSeconds：接收超时时间，单位s，默认3s
//      @bSendTimes：发送次数，默认1次
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
				if (ptStdReader->m_pbCctRxBuf[FRM_AFN] == AFN_CON)   //确认帧/否认
				{
					if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == F1)   //确认
					{
						DTRACE(DB_CCT, ("Receive a confirm frm!\r\n"));
						return true;
					}
					else  //否认
					{
						DTRACE(DB_CCT, ("Receive a disaffirm frm!\r\n"));
						return false;
					}
				}
				else //AFN非确认/否认
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

//描述是否允许恢复抄表
bool IsAllowResumeRdMtr(TStdReader* ptStdReader)
{
	if (ptStdReader->m_ptAutoReader->m_fStopRdMtr || !ptStdReader->m_ptAutoReader->m_fInReadPeriod || ptStdReader->m_ptAutoReader->m_dwLastDirOpClick !=0)
		return false;

	return true;
}

//描述:	硬件复位(载波模块)
void HardReset(TStdReader* ptStdReader, BYTE bFn)
{
	BYTE bData[20];
	DWORD dwFrmLen = 0;
	memset(bData, 0, sizeof(bData));

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_INIT, bFn, bData, 0, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

	SimpleTxRx(ptStdReader, dwFrmLen, 5, 1);
	return ;
}

//描述:启动/停止载波路由
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

//描述：停止抄表，对于被动式不需要停止的载波类型重载为空即可
bool StopReadMeter(TStdReader* ptStdReader)
{
	if (ptStdReader->m_ptAutoReader->m_dwLastDirOpClick != 0)	
		return true;
        
        return CtrlRoute(ptStdReader, F2);  //停止抄表
}

bool RestartRouter(TStdReader* ptStdReader)
{
	if (ptStdReader->m_ptAutoReader->m_fStopRdMtr || !ptStdReader->m_ptAutoReader->m_fInReadPeriod || ptStdReader->m_ptAutoReader->m_dwLastDirOpClick !=0)
		return false;

	return true;
}

//描述：恢复抄表，对于被动式不需要恢复的载波类型重载为空即可
//      注意：晓程的恢复抄表为进入学习状态
bool ResumeReadMeter(TStdReader* ptStdReader)
{
	if(!IsAllowResumeRdMtr(ptStdReader))
		return true;

	return CtrlRoute(ptStdReader, F3); //恢复抄表
}

bool TcRestartRouter(TStdReader* ptStdReader)
{
	TTime tSearchT;
	BYTE rbDuration;
	//if(IsInSchMtrTime(tSearchT, rbDuration) || m_bSchMtrState!=SCHMTR_S_IDLE)	//在自动搜表或手动搜表时段不能重启抄表)
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

//描述:	清除所有节点(载波模块)
void ClearAllRid(TStdReader* ptStdReader, WORD wFn)
{

	BYTE bData[20];
	DWORD dwOldClick =0, dwFrmLen=0;
	memset(bData, 0, sizeof(bData));

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_INIT, wFn, bData, 0, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);//厂商:晓程,东软

	SimpleTxRx(ptStdReader, dwFrmLen, 12, 1);
}

//描述:写主节点
//备注:$如果模块出厂时已经设置了地址,就用模块自身的地址,否则用传进去的地址
// 	   $只有在读主节点地址出错的情况下才会调用本函数设置主节点地址
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

//描述:	读主节点
bool ReadMainNode(TStdReader* ptStdReader)
{
	BYTE bData[20];
	DWORD  dwFrmLen=0;
	int i;
	memset(bData, 0, sizeof(bData));
	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_QRYDATA, 4, bData, 0, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);
	
	for (i=0; i<3; i++) //最多重发3次
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
						{//主节点要是唯一的，目前用终端地址作为主节点
							ptStdReader->m_fSyncAllMeter = false;
							ClearAllRid(ptStdReader, 2);
							return WriteMainNode(ptStdReader);
						}
						else	//如果模块出厂时已经设置了地址,就用模块自身的地址,否则用传进去的地址
						{
							//for (int i=0; i<6; i++)
							//	m_bMainAddr[i] = m_bCctRxBuf[FRM_AFN+3+i];
							//memcpy(ptStdReader->m_ptStdPara->RdrPara.bMainAddr, &ptStdReader->m_pbCctRxBuf[FRM_AFN+3], 6); //直接拷贝就行了
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

BYTE  GetRdNum(){return 5;}; //同步表地址时一次读取的从节点个数

//注意一次最多读10个,节省内存
int ReadPlcNodes(TStdReader* ptStdReader, BYTE *pbBuf, WORD wStartPn)
{
	WORD wStartP = wStartPn;	//TODO:是否应该从0开始

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
	DWORD dwDelay=1000; //按最大的（弥亚威）超时时间来，多等几百ms对别的模块没有影响，
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

		WordToByte(wStartP, &bData[0]);		//起始指针
		bData[2] = bPtr;					//要读取的载波节点数

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
						//模块返回异常，返回的是确认，但没有数据，比如麦希的模块<--  68 0F 00 8A 00 00 00 00 00 04 10 02 00 A0 16
						//既然能返回确认，里面总得要有数据
						if(wRxLen <= 15) 
							return 0;

						wTotalNum = ByteToWord(&ptStdReader->m_pbCctRxBuf[FRM_AFN+3]);

						bNum = ptStdReader->m_pbCctRxBuf[FRM_AFN+5];	//本次读取节点数

						if (bNum > 0)
						{
							wTmp += bNum;

							wStartP = GetStartPn(wStartP, bNum);

							WordToByte(wStartP, &bData[0]);

							{//东软的无效的地址在前面,如果地址不连续的话东软会返回超过总数的电表地址来，地址不连续的情况下不能用这个
								p2 = bBuf;
								for (i = bNum-1; i>=0; i--)
								{
									memcpy(p2, &ptStdReader->m_pbCctRxBuf[FRM_AFN+6+i*8], 6);
									p2 += 6;									
								}

								//因为东软的路由表地址跳着设置的话，某个索引位置没有表它会把下面一个有效的表填到这个位置上，
								//所以如果东软路由电表跳着配的话实际会读出超过电表总数的表出来，其中很多是重复的，这里需要把这个重复的删掉
								if (wRdNum == 0)
								{//第一次读肯定没有重复的
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

								wTmp = wRdNum;	//东软测量点不连接时，要修正一下路由节点数
							}

							if (wTotalNum == wTmp)	//读完了
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

//描述：超过2分钟没收到路由请求，发送恢复抄表
void AutoResumeRdMtr(TStdReader* ptStdReader)
{
	BYTE bTmp = 2;
	if (ptStdReader->m_ptAutoReader->m_dwLastDirOpClick!=0 && GetClick()>(ptStdReader->m_ptAutoReader->m_dwLastDirOpClick + (DWORD)bTmp*60))//停止直抄2分钟后恢复路由
	{
		ptStdReader->m_ptAutoReader->m_dwLastDirOpClick = 0;
		DTRACE(DB_CCT, ("AutoResumeRdMtr::KeepAlive: ResumeReadMeter m_dwLastDirOpClick!=0 && GetClick()>(m_dwLastDirOpClick+%d.\r\n",(DWORD )bTmp*60));
		ResumeReadMeter(ptStdReader);	//恢复自动抄表
		if (ptStdReader->m_fSyncAllMeter)
		{
			ptStdReader->m_ptAutoReader->m_bState = AR_S_LEARN_ALL;
		}
	}

	if(ptStdReader->m_ptAutoReader->m_dwLastDirOpClick==0)//非直抄时段保持路由状态
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

//描述:	激活机制
BYTE KeepAlive(TStdReader* ptStdReader)
{
	if (ptStdReader->m_bKplvResetCnt > 3)
	{
		DTRACE(DB_CCT, ("CStdReader :: KeepAlive poweroff router.\n"));
		//硬件复位调用
		ptStdReader->m_ptAutoReader->m_bState=AR_S_EXIT;		//退出路由流程，重新识别模块
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

//描述:	清除路由板电表节点
bool DelNode(TStdReader* ptStdReader, BYTE* pbAddr)
{
	DWORD  dwFrmLen=0;
	int iLen = 0;
	BYTE bData[20], bBuf[512];

	memset(bData, 0, sizeof(bData));

	bData[0] = 0x01;
	memcpy(&bData[1], pbAddr, 6);

	dwFrmLen = MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_SETRT, 2, bData, 7, 0, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);

	InitRcv(ptStdReader); //清空缓冲区，避免收到错误的帧

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
	
	InitRcv(ptStdReader); //清空缓冲区，避免收到错误的帧

	for (i=0; i<3; i++) //最多重发3次	
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
						SetInfo(INFO_PLC_CLRRT); //清路由
						DTRACE(DB_CCT, ("CStdReader::AddOneNode: frm mismatch, Fail!!!.\n"));
						return false;
					}
				} 
				else 
				{  
					DTRACE(DB_CCT, ("CStdReader::AddOneNode: frm mismatch, Fail!!!.\n"));
					SetInfo(INFO_PLC_CLRRT); //清路由
					return false;						
				}
			}			
		}

		Sleep(1000);
	}

	DTRACE(DB_CCT, ("CStdReader::AddOneNode: Fail!!!.\n"));


	return false;
}

//描述：组376.2透传帧接口，基类中为东软标准接口，其余类型若不符合需重载
//参数：@bAFn:透传使用的AFN
//      @bCtrl:规约类型
//      @pb645buf:透传的报文指针，@b645Len:645报文长度，@bAddrLen:376.2地址域长度
//add by CPJ at 2012-10-04
BYTE Make376TransFrm(TStdReader* ptStdReader, BYTE* pbAFn, BYTE bCtrl, BYTE* pbBuf, BYTE b645Len, BYTE bAddrLen)
{
	BYTE bTmpbuf[256] = {0,};
	//*pbAFn = AFN_RTFWD;	

	bTmpbuf[0] = bCtrl;     //规约类型
	bTmpbuf[1] = 0;         //从节点数量
	bTmpbuf[2] = b645Len;   //报文长度

	memcpy(&bTmpbuf[3], pbBuf, b645Len);

	return MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, *pbAFn, F1, bTmpbuf, b645Len+3, bAddrLen, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);
}

//描述：组376.2广播帧接口，基类中为东软标准接口，其余类型若不相同需重载
//参数：@pbBuf：376.2广播帧指针
//      @pbFrm：原始报文指针，@bFrmLen：原始广播数据长度
//      @pbAddr：广播地址
BYTE CctMakeBroadcastFrm(TStdReader* ptStdReader, BYTE *pbBuf, BYTE* pbFrm, BYTE bFrmLen, BYTE *pbAddr)
{
	SetAddrField(ptStdReader, ptStdReader->m_pbCctTxBuf, pbAddr);

	return MakeFrm(ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctTxBuf, AFN_CTRL, 3, pbBuf, (WORD )bFrmLen+2, 12, FRM_C_M_MST, FRM_C_PRM_1, 0, 0);
}

bool IsNoConfirmBroad(){return false;}; //广播命令是否无应答，晓程和御辉需重载，其余使用基类接口

//描述:启动广播命令,用来实现广播校时,广播冻结等功能
//参数:@bCtrl 控制字,保持跟698.42兼容,扩展BC_CTL_ADJTIME 0x10为广播校时
// 					00H=透明传输; 01H=DLT/645-1997; 02H=DLT/645-2007; 03H=相位识别功能; 04H FFH保留。
//	   @pbFrm 报文内容
// 	   @bFrmLen  报文长度
/*TODO: modify by CPJ at 2012-10-04
2012-10-04 之后版本流程更改为：为使该接口流程更加统一和规范化，故本接口只负责发送广播及对广播命令回复帧的检查，
后续如瑞斯康的重发或其余类型的恢复抄表由派生类接口完成 */
bool CctBroadcast(TStdReader* ptStdReader, BYTE bCtrl, BYTE* pbBuf, BYTE* pbFrm, BYTE bFrmLen)
{
	int iLen = 0;
	int i;
	DWORD dwLen=0, dwOldClick=0, dwFrmLen;
	WORD wOffset = 0;
	BYTE bAddr[]={0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
	
	if (!StopReadMeter(ptStdReader))	//停止自动抄表
		return false;

	DTRACE(DB_CCT, ("CStdReader::Broadcast: start.\r\n"));

	if (bCtrl == BC_CTL_ADJTIME) //对于本接口来说,广播校时没什么特殊处理,用回698.42的控制字
		bCtrl = CCT_MTRPRO_07;	//m_pStdPara->RdrPara.bMtrPro;

	pbBuf[0] = bCtrl;
	pbBuf[1] = bFrmLen;
	memcpy(&pbBuf[2], pbFrm, bFrmLen);

	dwFrmLen = CctMakeBroadcastFrm(ptStdReader, pbBuf, pbFrm, bFrmLen, bAddr);

	if(dwFrmLen <= 0)
		return false;

	for (i=0; i<1; i++) //最多重发3次
	{
		if (CctSend(&ptStdReader->m_tCctCommPara.m_tCommPara, ptStdReader->m_pbCctTxBuf, dwFrmLen) == dwFrmLen)
		{
			dwLen = 0;
			dwOldClick = GetClick();

			if (RxHandleFrm(ptStdReader, 3))
			{
				if (ptStdReader->m_pbCctRxBuf[FRM_AFN] == AFN_CON)
				{
					if (DtToFn(&ptStdReader->m_pbCctRxBuf[FRM_AFN+1]) == 1)	//确认帧
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

	//疑问：目前仅根据功能码和长度判断规约类型是否不够完善？ 2012-10-04
	if (bBuf[8]==0x1C || bBuf[8]==3 || bBuf[9]==4 || bBuf[9]==10)//3给电表转发时有C=3,肯定是07表
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
	ptStdReader->m_tCctCommPara.m_nRxStep = 0;  //防止之前收到的部分帧而影响解析
	memset(ptStdReader->m_pbCctRxBuf, 00, sizeof(ptStdReader->m_pbCctRxBuf));
	memcpy(bBuf, pbCmdBuf, bLen);
	if(CctRcvFrame(bBuf, bLen, &ptStdReader->m_tCctCommPara, ptStdReader->m_ptStdPara->wFrmLenBytes, ptStdReader->m_pbCctRxBuf, &ptStdReader->m_dwLastRxClick)!=bLen)
	{
		ptStdReader->m_tCctCommPara.m_nRxStep = 0;  //防止之前收到的部分帧而影响解析
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

	//透传广播命令如校时
	if(IsAllAByte(&bBuf[1], 0x99, 6))
	{
		if(bBuf[8]==0x04)
			CctBroadcast(ptStdReader, 1, bBuf, pbCmdBuf+iFrmPos, bTxLen);
		else if(bBuf[8]==0x08)
			CctBroadcast(ptStdReader, 2, bBuf, pbCmdBuf+iFrmPos, bTxLen);

		*pbrLen = 0;
		return 1;
	}

	for (i=0; i<3; i++) //最多重发6次
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

		//疑问：目前仅根据功能码和长度判断规约类型是否不够完善？ 2012-10-04
		if (bBuf[8]==0x1C || bBuf[8]==3 || bBuf[9]==4 || bBuf[9]==10)//3给电表转发时有C=3,肯定是07表
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
					//无论是使用数据转发还是监控从节点，其返回帧格式均相同，区别为不带地址域少12字节
					if (ptStdReader->m_pbCctRxBuf[1] < (bAfnPos==FRM_AFN_A ? 30 : (30-12)))
					{//没有应答帧
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

//描述:	透传645幁
int CctTransmit645Cmd(BYTE* pbCmdBuf, BYTE bLen, BYTE* pbData, BYTE* pbrLen, BYTE bTimeOut)
{
	int iRet;
	
	g_ptStdReader->m_ptAutoReader->m_fDirOp = true;	//外界的直接操作
	if (!StopReadMeter(g_ptStdReader))	//停止自动抄表
	{
		g_ptStdReader->m_ptAutoReader->m_fDirOp = false;
		return -1;
	}

	g_ptStdReader->m_ptAutoReader->m_dwLastDirOpClick = GetClick(); 

	iRet = CctTransmit645CmdUnlocked(g_ptStdReader, pbCmdBuf, bLen, pbData, pbrLen, bTimeOut);

	ResumeReadMeter(g_ptStdReader);	//恢复自动抄表
	g_ptStdReader->m_ptAutoReader->m_fDirOp = false;
	return iRet;
}

//描述:自动同步终端数据库和载波路由中的载波表地址
//参数:true-初始化同步终端和集中器中所有的电表地址
//返回:
//TODO:基类的同步表地址函数中只做同步相关的操作，在恢复抄表前额外的操作，
//     如硬件复位等由派生类自己完成 modify by CPJ at 2012-09-21
bool SyncMeterAddr(TStdReader* ptStdReader)
{	
	BYTE bBuf[8];
	BYTE  m_bMeterAddr[32][6];	//记录已经加入到载波模块的表地址
	BYTE  m_bSyncFlag[32/8+1];


	BYTE bMeterAddr[CCT_POINT_NUM+1][6];	//用于存放从路由下载的载波表地址
	BYTE bAddr[6];
	WORD wPn;
	bool fRet = true,fNoMtr=true;
	int iDesMeterCnt = 0;
	int iSrcMeterPtr = 0;
	int i, j;
	bool fNewNode = true;
	bool fFindPlase = false;

	//从路由下载的载波地址
	DTRACE(DB_CCT, ("CStdReader :: SyncMeterAddr : Sysnc all meter address.\n"));
	memset(bMeterAddr, 0, sizeof(bMeterAddr));

	iDesMeterCnt = ReadPlcNodes(ptStdReader, (BYTE *)&bMeterAddr, 1);

	fRet = true;	//在失败的时候置为false,用来记录同步过程中发生的错误
	memset(m_bSyncFlag, 0, sizeof(m_bSyncFlag));
	memset(m_bMeterAddr, 0, sizeof(m_bMeterAddr));

	//把数据库中的载波地址载入到m_bMeterAddr
	for (wPn=0; wPn<POINT_NUM; wPn++)
	{
		//不能直接在GetPlcNodeAddr接口中判断是否为ee，因为可能保存搜表地址时可能会搜出地址为ee的测量点，
		//这个是合法的，但不能同步进路由, modify by CPJ at 2013-05-20
		if (GetPlcNodeAddr(wPn, bAddr) && !IsAllAByte(bAddr, 0xee, 6)) 
		{
			fNoMtr=false;
			//查看是否有重复的节点地址,比如同一采集器下的485表,采集器地址会出现多次
			fNewNode = true;
			fFindPlase = false;
			for (i=0; i<CCT_POINT_NUM; i++)
			{
				if (!GetSyncFlag(m_bSyncFlag, i))//有效节点才比较
				{
					if (!fFindPlase)
					{
						j = i;//为排除测量点号和载波节点号不同步而增加
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

			//有效载波节点才增加
			if (fNewNode)
			{
				memcpy(m_bMeterAddr[j], bAddr, 6);
				SetSyncFlag(m_bSyncFlag, j); //置更改标志
			}
		}
	}

	//理论上这里进行了参数初始化操作，下面没必要再重新删除一回，
	//这种保护性措施对有的模块可能会存在问题，对于那些删除不干净的问题，可以让模块厂家去完善路由
	//如东软，删除一个已经不存在的节点会返回否认，东软目前已经建议不要这么做 modify by CPJ at 2012-06-01
	if (fNoMtr)
	{
		ClearAllRid(ptStdReader, 2);
		ptStdReader->m_fSyncAllMeter = true; //在外面统一处
		g_fCctInitOk = true;

		DTRACE(DB_CCT, ("CStdReader :: SyncMeterAddr : PLC nodes synchronized done.\n"));

		//一个表都没有，也就没必要重启抄表了，直接返回
		return true;
	}

	//比较从路由下载的载波地址和数据库中配置的载波地址
	//如果某个测量点的载波地址在路由中已经存在,则清更新标志,不用再更新到路由中去
	for (i=0; i<iDesMeterCnt; i++)
	{
		//iSrcMeterPtr = RIDIsExist(&m_bMeterAddr[0][0], bMeterAddr[i]); //从路由下载的载波表地址是否在数据库中有配置
		//if (iSrcMeterPtr >= 0)
		if(memcmp(&m_bMeterAddr[i], &bMeterAddr[i], 6) == 0)//严格对齐
		{
			//ClrSyncFlag(m_bSyncFlag, iSrcMeterPtr);
			ClrSyncFlag(m_bSyncFlag, i);
		}
		else
		{
			if (DelNode(ptStdReader, bMeterAddr[i]) == false)	//TODO:删除失败待处理
				fRet = false;
		}
	}

	for (i=0; i<CCT_POINT_NUM; i++)
	{
		if (GetSyncFlag(m_bSyncFlag, i)) //剩下有标志位的都是路由里没有相应载波地址配置的
		{
			//if(g_fPlcModChg)  //载波模块被更换了
			//	return false;

			if (AddOneNode(ptStdReader, i, m_bMeterAddr[i]))
			{
				ptStdReader->m_dwLastRxClick = GetClick();		//表数较多时，同步时间较长，这里防止同步后不必要的模块复位	
				ClrSyncFlag(m_bSyncFlag, i);
				//只有更新成功了才把新的表地址更新到m_bMeterAddr[wPn]且
				//清更新标志
				//否则不更新,等下回再来
			}
			else
			{
				DelNode(ptStdReader, m_bMeterAddr[i]);
				fRet = false;
			}
		}
	}

	if (fRet) //只有全部操作成功才把m_fSyncAllMeter置为true,否则下回还要同步一次
	{
		ptStdReader->m_fSyncAllMeter=true; //在外面统一处理
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
	if (ptStdReader->m_fSyncAllMeter == true)	//已经同步过路由的载波地址,不用再干了
		return true;

	StopReadMeter(ptStdReader);

	SetWorkMode(ptStdReader, WM_RDMTR);
	ReadMainNode(ptStdReader);

	SyncMeterAddr(ptStdReader);

	if(ptStdReader->m_fSyncAllMeter)
		TcRestartRouter(ptStdReader);//鼎信是重启路由.

	DTRACE(DB_CCT, ("CTcStdReader :: SyncMeterAddr : PLC nodes synchronized done.\n"));	
	return true;
}

//描述:	进行自动抄表器的状态更新，目前该流程为标准流程，各载波不需要更改或重载，具体差异在各分步骤中体现
//返回:	如果有必要中断当前进行的自动抄表和学习过程的则返回负数表明原因，
//		否则则返回0
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
	//1.死机检测，包括两部分：1.模块死机检测，2.停止抄表2分钟后自动恢复抄表
	KeepAlive(ptStdReader); 

	if (AR_S_EXIT == ptStdReader->m_ptAutoReader->m_bState)	//路由模块还没初始化正确,没正确识别到载波模块类型
		return 0;	//放到KeepAlive()后,还有机会给模块重新上电

	///////////////////////////////////////////////////////////
	//2.抄表时段检测
	//if (IsInReadPeriod()) //在抄表时段内
	//{
	//	if (!m_fInReadPeriod)
	//	{
	//		DTRACE(DB_CCT, ("CStdReader::UpdateReader: sw into rd period.\r\n"));

	//		m_fInReadPeriod = true;

	//		//恢复抄表之前需要判断是否已成功同步表地址，如果没有同步过，同步后SyncMeterAddr函数里面会发送恢复抄表
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
	//3.同步电表地址，本步骤应该在除初始化之外的所有操作之前进行，不同的载波类型重载该接口
	TCSyncMeterAddr(ptStdReader);
	//SyncMeterAddr(ptStdReader);


	///////////////////////////////////////////////////////////
	//4.超过2分钟没有收到路由的抄表请求了，需要发送恢复抄表命令
	AutoResumeRdMtr(ptStdReader);


	///////////////////////////////////////////////////////////
	//5.新的一天，重新抄表
	if (IsDiffDay(&now, &ptStdReader->m_tmUdp) && wCurMin >= ptStdReader->m_ptStdPara->RdrPara.bDayFrzDelay)
	{
		DTRACE(DB_CCT, ("CStdReader::UpdateReader: restart router at new day.\n"));
		ptStdReader->m_fRestartRoute = false;
		StopReadMeter(ptStdReader);

		//跨日后路由器的相关操作，如瑞斯康的复位载波模块电源，力合微的硬件复位等
		TcRestartNewDay(ptStdReader);
		ptStdReader->m_ptAutoReader->m_fStopRdMtr = false; //因为路由模块的恢复抄表函数ResumeReadMeter(),可能判断到m_fStopRdMtr==true,就不执行相应操作了,所以这里要先把m_fStopRdMtr置为false
		RestartRouter(ptStdReader);
		memset(ptStdReader->m_pbStudyFlag, 0x00, POINT_NUM/8+1);//清测量点学习成功标志
	}

	/////////////////////////////////////////////////////////////
	//6.被动式路由类型的日常抄表流程，如晓程、御辉、瑞斯康，
	//  对此类路由来说还需对学习和抄表状态的转换进行控制，不同类型单独重载实现
	//  对不需要走该接口的载波如东软、鼎信需重载为空函数。
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
		//if( wCompletePercent<wLeastCompletePercent &&bVal == 1 && wCurMin %10 == 0)//同一分钟计算多次
		//	CalcuCompletedPercent();	
		//DoUnReadID();
	}

	///////////////////////////////////////////////////////////
	//7.抄表日
	//if (!m_fRestartRoute && CctIsMtrDate(PORT_CCT_PLC, now)) //抄表日抄表时间到了
	//{
	//	ReadMtrDate(); //对于主动式路由如东软、鼎信类载波需重新启动路由，对被动式路由重载为空即可
	//}

	if (wCurMin >= ptStdReader->m_ptStdPara->RdrPara.bDayFrzDelay)
		ptStdReader->m_tmUdp = now;	//最后一次更新时标

	///////////////////////////////////////////////////////////
	//8.消息控制相关操作，如电表参数变更、清路由，进行路由升级等
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
	//9.重点户抄表
	//ReadVipDate(); 

	//10.自动校时
	//DoAdjTime();

	//
	//ReadInterV();

	//是否允许主动上报
// 	BYTE bRptCmd;
// 	ReadItemEx(BN23, PN0, 0x3032, &bRptCmd); //为1允许上报,为2暂停上报
// 	if(bRptCmd == 1)
// 	{
// 		AllowReport(ROUTER_RPT_START);  //为1允许上报
// 		bRptCmd = 0;
// 		WriteItemEx(BN23,PN0,0x3032,&bRptCmd);
// 	}
// 	else if(bRptCmd ==2)
// 	{
// 		AllowReport(ROUTER_RPT_STOP);  //为2暂停上报
// 		bRptCmd = 0;
// 		WriteItemEx(BN23,PN0,0x3032,&bRptCmd);
// 	}
// 
// 	//判断是否更新F150
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

//描述:自动学习流程(主要处理自动抄读冻结数据)
int DoLearnAll(TStdReader* ptStdReader)
{
	if (RxHandleFrm(ptStdReader, 1))	//该处理的自动抄读帧,都在里面处理完了,返回true时,帧组成功了,但是没有被默认帧处理函数所处理
		//ExHandleFrm(); //特殊帧处理函数

	return 0;
}
