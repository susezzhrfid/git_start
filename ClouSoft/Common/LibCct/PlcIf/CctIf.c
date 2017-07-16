/*********************************************************************************************************
* Copyright (c) 2009,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称: CctIf.c
* 摘    要: 本文件主要实现载波协议接口
* 当前版本: 1.0
* 作    者: 曾敏涛
* 完成日期: 2014年8月
* 备	注:	
*			1>TCS\TCC\TCM 之间关系 S集中器C载波通道M带计量载波通道;
*			2>对应青岛鼎信规约Ver1.11 2008/10/03
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
	//pComm->SetTimeouts(1000); //串口缺省延时是1S,以防被某些表协议改到很短了

	bool fCommOpen = false;
	TCommPara CommPara;
	if ( CommIsOpen(pCommPara->wPort) )
	{
		if (CommGetPara(pCommPara->wPort, &CommPara)) //函数内部已有检测串口是否打开	
		{
			fCommOpen = true;
			if ( pCommPara->wPort==CommPara.wPort
				&& pCommPara->dwBaudRate==CommPara.dwBaudRate
				&& pCommPara->bByteSize==CommPara.bByteSize
				&& pCommPara->bParity==CommPara.bParity
				&& pCommPara->bStopBits==CommPara.bStopBits )
			{
				return true;	//串口波特率相同的情况下不用重新再打开串口			
			}
		}
	}

	if ( fCommOpen ) 
	{
		if ((pCommPara->wPort!=COMM_GPRS) && (pCommPara->wPort!=COMM_LOCAL))//不能误关GPRS与红外
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

//描述:帧格式中的DT到FN转换
//返回:正确则返回FN,否则返回0
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

//描述:FN到帧格式中DT的转换
void nToDt(WORD wFn, BYTE* pbDt)
{
	pbDt[0] = 0x1<<((wFn-1)%8);
	pbDt[1] = (BYTE)((wFn-1)/8);
}

//描述:FN到帧格式中DT的转换
void FnToDt(WORD wFn, BYTE* pbDt)
{
	pbDt[0] = 0x1<<((wFn-1)%8);
	pbDt[1] = (BYTE)((wFn-1)/8);
}

//描述:取上行报文信息域R
void GetUpInf(BYTE* pbBuf, TUpInf* pUpInf)
{
	pUpInf->bFwdDepth = pbBuf[0] >> 4;			//中继深度
	pUpInf->bModule = (pbBuf[0]>>2) & 0x01;		//通信模块标识：0表示对集中器的通信模块操作，1表示对载波表的通信模块操作
	pUpInf->bRt = pbBuf[0] & 0x01;				//路由标识：D0=0表示通信模块带路由或工作在路由模式，D0=1表示通信模块不带路由或工作在旁路模式。
	pUpInf->bCn = pbBuf[1] & 0x0f;				//信道标识：取值0~15，0表示不分信道、1~15依次表示第1~15信道。
	pUpInf->bCnChar = pbBuf[2] >> 4;			//电表通道特征
	pUpInf->bPhase = pbBuf[2] & 0x0f;			//实测相线标识：实测从节点逻辑主信道所在电源相别，0为不确定，1~3依次表示相别为第1相、第2相、第3相。
	pUpInf->bAnsSigQlty= pbBuf[3] >> 4;			//末级应答信号品质
	pUpInf->bCmdSigQlty = pbBuf[3] & 0x0f;		//末级命令信号品质
	pUpInf->bRptEvtFlg = pbBuf[4]&0x01;         //上报事件标识
	pUpInf->bSN = pbBuf[5];                     //报文序列号
}

//描述:组帧应用数据域
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
	BYTE bLenBytes = wFrmLenBytes-1;	//帧长度字节数	
	WORD wAppDataFieldLen = 0;
	WORD i;

	//组帧应用数据域
	BYTE bCS = 0; 
	pbTxBuf[0] = 0x68;

	if(pbData==NULL && wDataLen!=0)
	{
		DTRACE(DB_CRITICAL, ("CStdReader::MakeFrm: fail, duo to pbData==NULL and wDataLen!=0!!!\r\n"));
		return 0;
	}

	wAppDataFieldLen = MakeAppDataField(bAfn, wFn, pbData, wDataLen, &pbTxBuf[9+bAddrLen+bLenBytes]); 

	wLen = 6 + bAddrLen + wAppDataFieldLen + 5 + bLenBytes;
	pbTxBuf[1] = (BYTE )wLen;	//长度L
	if (bLenBytes == 1)
		pbTxBuf[2] =(BYTE )(wLen>>8)  ;	//长度L
	pbTxBuf[2 + bLenBytes] = (BYTE) (bPRM*(0x01<<6)  + bMode); //控制域C

#ifdef EN_NEW_376_2
	//376.2增加的帧序号
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

//描述：通过串口向载波路由发数据
//返回：已发送长度
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

//描述：载波路由的数据
//返回：接收到的长度
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

//描述:	接收一个数据块,判断是否接收到一个完整的通信帧
//返回:	返回已经扫描过的字节数,如果收到一个完整的通信帧则返回正数,否则返回负数
//pbBlock        收到报文
//nLen           收到报文长度
//ptCctCommPara  376.2结构解析的中间参数
//wFrmLenBytes   376.2结构的长度节数
//pbCctRxBuf     输出的完整376.2帧
//pdwLastRxClick 最近接收完整376.2帧的时间
int CctRcvFrame(BYTE* pbBlock, int nLen, TCctCommPara* ptCctCommPara, WORD wFrmLenBytes, BYTE* pbCctRxBuf, DWORD* pdwLastRxClick)
{
	int iRxLen = 0;
	BYTE *bHead = pbBlock;
	int i;
	BYTE b;

	ptCctCommPara->m_nRxStep = 0;    //若帧不完整，循环缓冲区中的数据不会删除，故每次进来都可从头开始解析，直至找到一个完整的帧才会拷贝到m_bCctRxBuf
	
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
				if(ptCctCommPara->m_wRxDataLen>(WORD)nLen   //算出的376.2帧长度已超过传入的数据长度
					|| ptCctCommPara->m_wRxDataLen+i-2>nLen  //算出的帧尾位置已超过传入的数据长度
					|| bHead[ptCctCommPara->m_wRxDataLen+i-3]!=0x16)  //帧结束符不是0x16
				{
					//这里如果是接受不全或非法的报文，将m_nRxStep置为0，相当于将m_bCctRxBuf中的数据清空，
					//同时，在RxHandleFrm不删除扫描过的数据，等后续收完了再一起拷贝到本函数中重新进行解析    add by CPJ at 2012-10-11
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
				iRxLen = ptCctCommPara->m_wRxDataLen - 3 - wFrmLenBytes; //这里减出来可能会小于0，需加保护 add by CPJ at 2012-16-19

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