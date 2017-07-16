/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DL645.c
 * 摘    要：本文件给出97版645抄表协议的功能实现
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "DL645.h"
#include "DbAPI.h"
#include "MtrCtrl.h"

#define DL645_CMD          8 
#define DL645_LEN          9
#define DL645_DATA         10

//#define DL645_CMD_RESERVE        0x00
#define DL645_CMD_ASK_DATA       0x01
#define DL645_CMD_ASK_NEXT       0x02
//#define DL645_CMD_REASK          0x03
//#define DL645_CMD_WRITE_DATA     0x04
#define DL645_CMD_BC_TIME        0x08
//#define DL645_CMD_WRITE_ADDR     0x0a
//#define DL645_CMD_CHG_BR         0x0c
//#define DL645_CMD_CHG_PSW        0x0f
//#define DL645_CMD_DMD_RESET      0x10

//#define DL645_CMD_MAX         DL645_CMD_DMD_RESET
#define DL645_CMD_GET   0x1f

//645协议自用函数
void FillAddrBuf(BYTE bAddrByte, BYTE* pbAddr);
WORD DL645MakeFrm(T645Priv* pMtr645, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen);
int DL645TxRx(struct TMtrPro* pMtrPro, T645Tmp* pTmp645, WORD wID, WORD wLen, BYTE bReTryTimes);
int AskItemBID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);
int AskItem1BID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes);
int AskItemSID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);
int AskItem1SID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes);
int DL645AskItem1(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);

//描述:645协议初始化函数
bool Mtr645Init(struct TMtrPro* pMtrPro, bool fInit, BYTE bThrId)
{
	BYTE bRdType = 0;
	T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;

	pMtrPro->pfnAskItem = DL645AskItem;	
	pMtrPro->pfnRcvBlock = DL645RcvBlock;
	pMtrPro->pfnGetProPrintType = DL645GetProPrintType;

	pMtrPro->pbTxBuf = &m_MtrTxBuf[bThrId][0];
	pMtrPro->pbRxBuf = &m_MtrRxBuf[bThrId][0];
	memset(pMtrPro->pbTxBuf, 0, MTR_FRM_SIZE); 
	memset(pMtrPro->pbRxBuf, 0, MTR_FRM_SIZE); 
	
	if (fInit)//表计初始化的第一次
	{
		pMtr645->fRd9010 = 0;
		pMtr645->fRd901f = 0;
		pMtr645->fRdSID = 0;		
        pMtr645->bAddrByte = 0;
	}	

	//判断只单抄还是单块自适应协议
/*	bRdType = IsSIDV97Mtr(pMtrPro->pMtrPara->wPn);
	if (bRdType == 2) //走只单抄协议
		pMtr645->fRdSID = true;
	else if (bRdType == 1) //走块抄协议
		pMtr645->fRdSID = false;
	else
		return false;	*/

	return true;
}

//描述:抄读接口数据对费率块ID以及C86f块ID的特殊处理
int DL645AskItem(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{	
	int iRet = 0;		
	WORD wID1 = wID;	
	BYTE bNum = 0;
	BYTE bLen = 0;	

	if (wID == 0xc86f) //如果是块数据，其他的状态字都读不到,
	{		
		bNum = 7;
		bLen = 2;
		wID1 = Id645V07toDL645(wID);				
		iRet = DL645AskItem1(pMtrPro, wID1, pbBuf);			

		if (iRet > 0) //把获得的645数据转成2007版645数据	
		{
			iRet = Data645to645V07(wID1, pbBuf, iRet);	
			memset(pbBuf+iRet, 0, bNum*bLen-iRet);
			iRet = bNum*bLen;						
		}		
	}		
	else//其他块ID以及单ID请求
	{	
		wID1 = Id645V07toDL645(wID); 		
		iRet = DL645AskItem1(pMtrPro, wID1, pbBuf);		

		if (iRet>0 && wID!=wID1) //把获得的645数据转成2007版645数据					
			iRet = Data645to645V07(wID1, pbBuf, iRet);		
	}

	return iRet;
}

BYTE bSchSpecAddr(BYTE* pbAddr)
{
	BYTE i, n=0;
	for (i=0; i<6; i++)
	{
		if (pbAddr[6-i-1]==0xAA || pbAddr[6-i-1]==0x99)
			n ++;
		else 
			break;
	}
	return n;
}

//描述:645接收验帧函数
bool DL645RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize)
{
    BYTE n;
	WORD i;
	//T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;
	T645Tmp* pTmp645 = (T645Tmp* )pTmpInf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf; 
    BYTE* pbTxBuf = pMtrPro->pbTxBuf;

	for ( ; dwLen; dwLen--)
	{
		BYTE b = *pbBlock++;

		switch (pTmp645->nRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pbRxBuf[0] = 0x68;
				pTmp645->wRxPtr = 1;
				pTmp645->wRxCnt = 9;       
				pTmp645->nRxStep = 1;
			}
			break;
		case 1:    //数据域前的数据
			pbRxBuf[pTmp645->wRxPtr++] = b;
			pTmp645->wRxCnt --;
			if (pTmp645->wRxCnt == 0)   //接收完，进行校验
			{
                n = bSchSpecAddr(&pbTxBuf[1]); //避开广播地址的返回
				if (memcmp(&pbRxBuf[1], &pbTxBuf[1], 6-n)==0 && pbRxBuf[7]==0x68)
				{
					pTmp645->wRxDataLen = pbRxBuf[9];
					pTmp645->wRxCnt = pTmp645->wRxDataLen + 2;
					pTmp645->nRxStep = 2;
				}
				else
				{
					pTmp645->nRxStep = 0;
				}
			}
			break;
		case 2:     //数据 + 检验码 + 结束码
			pbRxBuf[pTmp645->wRxPtr++] = b;
			pTmp645->wRxCnt --;
			if (pTmp645->wRxCnt == 0)   //接收完，进行校验
			{
				pTmp645->nRxStep = 0;

				if (pbRxBuf[pTmp645->wRxPtr-1]==0x16 && pbRxBuf[pTmp645->wRxPtr-2]==CheckSum(pbRxBuf, pTmp645->wRxDataLen+10))
				{
					for (i=10; i<10+pTmp645->wRxDataLen; i++)
						pbRxBuf[i] -= 0x33;

					return true;    //接收到完整的一帧
				}
			}
			break;
		default:
			pTmp645->nRxStep = 0;
			break;
		} //switch (m_nRxStep) 
	}

	return false;
}

//描述:获取协议打印信息
void DL645GetProPrintType(BYTE* pbPrintPro, char* pszProName)
{
	*pbPrintPro = DB_645;
	memcpy(pszProName, "DL645", sizeof("DL645"));	
}

/////////////////////////////////////////////////////
//以下为645协议内部使用的函数定义

//描述:根据填充标志对地址进行填充
void FillAddrBuf(BYTE bAddrByte, BYTE* pbAddr)
{
	BYTE i;

	for (i=0; i<6; i++)
	{
		if (pbAddr[6-i-1] != 0)
			break;				
	}

	if (bAddrByte == 2)
	{
		memset(pbAddr+6-i, 0xaa, i);
	}	
	else
	{
		memset(pbAddr+6-i, 0, i);
	}
}

WORD DL645MakeFrm(T645Priv* pMtr645, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen)
{	
	WORD i;	
	pbTxBuf[0] = 0x68;

	memcpy(&pbTxBuf[1], pbAddr, 6);

	if (bCmd != DL645_CMD_BC_TIME)
		FillAddrBuf(pMtr645->bAddrByte, &pbTxBuf[1]);

	pbTxBuf[7] = 0x68;
	pbTxBuf[8] = bCmd;
	pbTxBuf[9] = bLen;

    //+0x33
    for (i=10; i<(WORD)bLen+10; i++)
	{
  	    pbTxBuf[i] += 0x33;
	}	 
	
	pbTxBuf[10+(WORD)bLen] = CheckSum(pbTxBuf, (WORD)bLen+10);
	pbTxBuf[11+(WORD)bLen] = 0x16;

	return bLen+12;
}    


int DL645TxRx(struct TMtrPro* pMtrPro, T645Tmp* pTmp645, WORD wID, WORD wLen, BYTE bReTryTimes)
{	
	BYTE n;
	bool fReadSuccess = false;

	//T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;

	for (n=0; n<bReTryTimes; n++)
	{
		if(n > 0) //重发的时候先清除一下串口
		{			
		    CommRead(pMtrPara->CommPara.wPort, NULL, 0, 200);					
		}

        if (g_fDirRd && !g_bDirRdStep)
            break;

        if (MtrProSend(pMtrPara->CommPara.wPort, pbTxBuf, wLen) != wLen)
		{
			DTRACE(DB_645, ("CDL645::TxRx : fail to write comm.\r\n")); 
			continue;//return 0;
		}
		
		pTmp645->nRxStep = 0;		

		fReadSuccess = ReadCommFrm(pMtrPro, (void*)pTmp645, 0, 4, 2, 200, MTR_FRM_SIZE, 0, NULL, 0);

		if (fReadSuccess)	//接收到一个完整的帧
		{	
			if ((pbRxBuf[DL645_CMD]&DL645_CMD_GET) == (pbRxBuf[DL645_CMD]&DL645_CMD_GET))
			{
				if ((pbRxBuf[DL645_CMD]&0xc0) == 0x80)   //帧校验正确
				{
					WORD wRxID = pbRxBuf[DL645_DATA] + (WORD)pbRxBuf[DL645_DATA+1]*0x100;
					if (wRxID != wID)
					{							
						DTRACE(DB_645, ("CDL645:: Tx_ID:%x != Rx_ID:%x\r\n", wID, wRxID)); 									
					}
					else
					{
		 	 			return 1;
					}
				}
				else if ((pbRxBuf[DL645_CMD]&0xc0) == 0xc0)   //帧校验正确
				{
					DTRACE(DB_645, ("CDL645::TxRx : rx = not surport data.\r\n")); 
		 			return -1;
				}
			}
		}
 		DTRACE(DB_645, ("CDL645::TxRx : fail to rx frame.\r\n")); 
	}

	return 0;
}

//描述:记录抄读901F成功的相关信息
void SetRead901fSuccess(struct TMtrPro* pMtrPro, T645Tmp* pTmp645)
{
		WORD wLen = 0;
		BYTE* pbRxBuf = pMtrPro->pbRxBuf;
		TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
		T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	

		pMtr645->fRd901f = true;
	
		if (pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xaa || pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xdd)//科陆表实测									
				wLen = pTmp645->wRxDataLen-3;				
		else							
				wLen = pTmp645->wRxDataLen-2;
				
		if ((wLen%4)==0 && (wLen/4==3 || wLen/4==4))
		{	
			//浙江需求,分时表返回长度是2费率或3费率,则自适应改变费率顺序				
			pMtrPara->bRateTab[0] = 2;//费率顺序	
			pMtrPara->bRateTab[1] = 3;			
			pMtrPara->bRateTab[2] = 4;			
			pMtrPara->bRateTab[3] = 0;
			DTRACE(DB_645, ("CDL645::AskItem1 : Adjust RateNum By Read 0x901f.\r\n")); 
		}				
}

//描述：请求数据打包发送并接收
int DL645MakeAskData(struct TMtrPro* pMtrPro, T645Tmp* pTmp645, WORD wID, BYTE bReTryTimes)
{
	WORD wFrmLen;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
    TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	

	pbTxBuf[DL645_DATA] = (BYTE)wID;
	pbTxBuf[DL645_DATA+1] = (BYTE)(wID>>8);		
	wFrmLen = DL645MakeFrm(pMtr645, pbTxBuf, pMtrPara->bAddr, DL645_CMD_ASK_DATA, 2);

	return DL645TxRx(pMtrPro, pTmp645, wID, wFrmLen, bReTryTimes);
}

//描述:645协议抄读接口函数
int DL645AskItem1(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{
	int iRet= -1;
	T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;

	if ( pMtr645->fRdSID ) //走只单抄协议
		iRet = AskItemSID(pMtrPro, wID, pbBuf);
	else 
		iRet = AskItemBID(pMtrPro, wID, pbBuf);

	if (iRet > 0)	
		iRet = Data645ToComm(wID, pbBuf, (WORD)iRet);	

	return iRet;
}


//描述：读取那些DL645有此数据标识的数据,有块读块,没块拆分
int AskItemBID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{
	BYTE i, j;
	BYTE bNum, bItemLen;
	BYTE bTemp[100];
	bool bfind=false;
	WORD wSubID=0; 	
	int iLen=0, iRv;

	T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	
	
	if ((wID&0x000F) == 0xF)
	{
		if (IsRateId(wID) && !pMtr645->fRd901f && pMtr645->bAddrByte!=0)
		{		
			//简化不必要的抄表
		}
		else
		{
			iLen = AskItem1BID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);
			if(iLen > 0) return iLen;
		}
		
		if (pMtr645->bAddrByte == 0)  //基本尝试都未成功，则不再进行拆分尝试
			return iLen;	

		if (!pMtr645->fRd901f && !pMtr645->fRd9010)	//如果9010以及901F均通信不上,则不再进行拆分尝试
			return iLen;

		if (pMtr645->fRd901f && IsRateId(wID))	//如果901F支持,则所有的费率计量数据均不再拆分
		{
			if (iLen < 0)	//确定不支持
				return -1;
			else if (wID != 0x901f)		//本块ID不返回且901F支持,则视作本块ID不支持(江苏集抄需求)
			{
				DTRACE(DB_645, ("CDL645::AskItem: wID=%x failed. test 0x901f\r\n", wID));
				if ( AskItem1BID(pMtrPro, 0x901f, pbBuf, 1) > 0)
					return -1;
				return 0;
			}
            else
				return 0;
		}
		
		wSubID = wID&0xFF00;
		if (IsRateId(wID) || wSubID==0xB600 || wSubID==0xB300)
		{
			bNum = GetBlockIdNum(wID);
			bItemLen = Get645TypeLength(wID);
			memset(pbBuf, m_bInvdData,  bNum*bItemLen);		
			
			iLen = 0;
			for (i=0; i<bNum; i++)
			{
				memset(bTemp, 0, sizeof(bTemp));
				if (wID==0xB61F || wID==0xB62F)
					j = i+1;			
				else
					j = i;
				iRv = AskItem1BID(pMtrPro, (wID&0xFFF0)+j, bTemp, TXRX_RETRYNUM);
				if (iRv > 0)
				{
					bfind = true;
					memcpy(pbBuf+iLen, bTemp, iRv);
					iLen += iRv;
				}
				else
				{	
					if (iRv < 0) //如果ID表示不支持（含通信返回不支持以及非通信检测不支持的情况），也算通信OK的，上次不予以补抄
						bfind = true;

					if (bNum == 5) //电量、需量、需量时间之类的数据
					{
						if (iRv<0 && i==0) //如果第一个子ID就不支持，则块ID视作不支持
							return -1;
						else if (iRv == -2) //非通信检测不支持的情况(费率调整或是不支持抄读的ID)
							iLen += bItemLen;
						else //if (iRv == -1) //注意需是通信返回的不支持,再停止抄后续子ID,否则单ID依据参数进行费率调整的时候,也会有不支持的,但要继续请求支持的子ID
						{
							iLen = bNum*bItemLen;
							break;
						}
					}
					else //瞬时量或断相
					{
						if (j == 0)	//总量不予判断
							iLen += bItemLen;
						else	//以A相为依据返回,后面的子ID不抄了
						{
							if (iRv<0 && j==1) //如果A相就不支持，则块ID视作不支持
							{
								if ( bfind && wID!=0xb67f ) //表示总已经回OK了
									iLen = bNum*bItemLen;
								else
									return -1;
							}
							else
								iLen = bNum*bItemLen;
							break;						
						}						
					}
				}			
			}

			if ( !bfind ) //子ID一个都没回答
			{
				iLen = 0;
				if (wID != 0x901f)	//本块ID不返回且901F支持,则视作本块ID不支持(江苏集抄需求)
				{
					if ( pMtr645->fRd901f )
					{
						DTRACE(DB_645, ("CDL645::AskItem: wID=%x failed. test 0x901f\r\n", wID));
						if ( AskItem1BID(pMtrPro, 0x901f, pbBuf, 1) > 0)
							return -1;
						return 0;
					}
					else if ( pMtr645->fRd9010 )
					{
						DTRACE(DB_645, ("CDL645::AskItem: wID=%x failed. test 0x9010\r\n", wID));
						if ( AskItem1BID(pMtrPro, 0x9010, pbBuf, 1) > 0)
							return -1;
						return 0;
					}
				}
			}
		}	
	}
	else
	{
		if (wID==0x9010 && pMtr645->bAddrByte!=0 && pMtr645->fRd9010)
		{
			iLen = AskItem1BID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);
		}
		else if (wID==0x9010 && pMtr645->bAddrByte!=0 && pMtr645->fRd901f)
		{
			DTRACE(DB_645, ("CDL645::AskItem: 0x9010 is replaced with 0x901f\r\n", wID));
			iLen = AskItem1BID(pMtrPro, 0x901f, pbBuf, TXRX_RETRYNUM);
			if (iLen > 0)	iLen = 4;
		}
		else
		{
			iLen = AskItem1BID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);	
			if (iLen==0 && pMtr645->bAddrByte!=0)	//本ID不返回且关键ID支持,则视作本ID不支持(江苏集抄需求)
			{
				if ( pMtr645->fRd901f )
				{
					DTRACE(DB_645, ("CDL645::AskItem: wID=%x failed. test 0x901f\r\n", wID));
					if (AskItem1BID(pMtrPro, 0x901f, pbBuf, 1) > 0)
					{
						if (wID == 0x9010)//当表配置后直接就抄表故障的情况下,第一轮抄表恢复由于判断9010不支持(江苏表)而会导致本轮抄表故障不能恢复
							return 4;
						else
							return -1;
					}
					return 0;
				}
				else if (pMtr645->fRd9010 && wID!=0x9010)
				{
					DTRACE(DB_645, ("CDL645::AskItem: wID=%x failed. test 0x9010\r\n", wID));
					if ( AskItem1BID(pMtrPro, 0x9010, pbBuf, 1) > 0)
						return -1;
					return 0;
				}
			}
		}
	}

	if (iLen == -2) //转化配置不支持的ID的返回值
		return -1;

	return iLen;	
}

//描述：读取那些DL645有此数据标识的数据.尝试块与单
int AskItem1BID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes)
{
	BYTE find = 0, i;
	int iRet = -1, iLen;
	WORD wtID;
	BYTE mBuf[80], eLen, bRxLen, bItemLen;

	T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	//BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;

	//收帧解析的临时变量,每次处理重新开始
	T645Tmp tTmp645; 
	T645Tmp* pTmp645 = &tTmp645;
	memset(&tTmp645, 0, sizeof(T645Tmp));	

	if ( !MtrProOpenComm(&pMtrPara->CommPara) )		return 0;

	DTRACE(DB_645, ("CDL645::AskItem1 : wID = %x.\r\n", wID)); 

	if ( Is645NotSuptId(wID) ) //分相电能及相位角、视在功率、零序电流等不支持	
		return -2;	

	//根据费率的参数,调整读取的费率的单ID
	//费率参数的意义,是将对应电表上来的费率顺序,依次对应到终端的费率位置去,比如参数i0=2;i1=3;i2=4;i3=0,表示电表回来的第一个费率数据(抄表ID9011),是对应终端的峰费率9012的位置
	//则需注意,读取峰费率,读取ID是9012,实际的抄表ID是9011,返回的数据认做9012
	if ( IsRateId(wID) )
	{
		if ((wID&0x000f)!=0x000f && (wID&0x000f)!=0x0000) //分费率转换
		{
			for (i=0; i<4; i++)
			{
				if (pMtrPara->bRateTab[i] == (wID&0x000f))
				{					
					DTRACE(DB_645, ("CDL645::AskItem1BID : Pn=%d, wID = %x.adjust rate to read =%x\r\n",pMtrPara ->wPn, wID, (wID&0xfff0)+i+1)); 
					wID = (wID&0xfff0)+i+1;
					find = 1;
					break;
				}
			}
			if ( !find )	
			{
				DTRACE(DB_645, ("CDL645::AskItem1BID : Pn=%d, wID = %x.not find adjust rate.\r\n",pMtrPara ->wPn, wID)); 
				return -2;		
			}
		}
	}

	//确定地址域不够6字节是填0还是填0xaa
	wtID = 0x901f;
	if (pMtr645->bAddrByte == 0) //没尝试过，需要进行尝试
	{			
		DTRACE(DB_645, ("CDL645::AskItem1BID start trying***: Pn=%d, m_fRd901f=%d, m_fRd9010=%d\r\n", pMtrPara->wPn, pMtr645->fRd901f, pMtr645->fRd9010));
		iRet = DL645MakeAskData(pMtrPro, pTmp645, wtID, 1);
		if (iRet > 0) //地址填0 明确的支持或不支持都表明通信OK	
			pMtr645->bAddrByte = 1;	
		else 
		{
			if (iRet < 0)	//当901f明确不支持时,还必须试下9010是否支持	
				pMtr645->bAddrByte = 1;	

			wtID = 0x9010;
			iRet = DL645MakeAskData(pMtrPro, pTmp645, wtID, 1);
			
			if (iRet!=0 || pMtr645->bAddrByte==1) //地址填0 明确的支持或不支持都表明通信OK	
				pMtr645->bAddrByte = 1;		
			else
			{		
				wtID = 0x901f;
				pMtr645->bAddrByte = 2;
				iRet = DL645MakeAskData(pMtrPro, pTmp645, wtID, 1);

				if (iRet <= 0)	//当901f明确不支持时,还必须试下9010是否支持	
				{
					wtID = 0x9010;				
					iRet = DL645MakeAskData(pMtrPro, pTmp645, wtID, 1);

					if (iRet == 0)	//下次再试	 
					{
						pMtr645->bAddrByte = 0;			
						return iRet;				
					}//else //地址填AA 明确的支持或不支持都表明通信OK							
				}//else //地址填AA 明确的支持或不支持都表明通信OK						
			}	
		}
		if (wtID == 0x901f) //支持块
		{
			if (iRet > 0)
			{
				SetRead901fSuccess(pMtrPro, pTmp645);
			}
			else
				pMtr645->fRd901f = false;
		}
		else if (wtID == 0x9010) //支持单
		{
			if (iRet > 0)
				pMtr645->fRd9010 = true;
			else
				pMtr645->fRd9010 = false;
		}

		////////////////////////////////////////
		//此时通信一定OK,再行尝试是否支持块数据		
		if ( !pMtr645->fRd901f ) //块数据不成功,再进行尝试
		{			
			iRet = DL645MakeAskData(pMtrPro, pTmp645, 0x901f, 1);

			if (iRet > 0) //地址填0 明确的支持或不支持都表明通信OK			
				SetRead901fSuccess(pMtrPro, pTmp645);
			else if (wID == 0x901f) //已经尝试了901f不OK,就不用再重复抄一次请求的901f了
			{
				DTRACE(DB_645, ("CDL645::AskItem1 end trying***: Pn=%d, m_fRd901f=%d, m_fRd9010=%d, m_AddrFilledData=%d\r\n", pMtrPara->wPn, pMtr645->fRd901f, pMtr645->fRd9010, pMtr645->bAddrByte));
				return iRet;
			}
		}

		DTRACE(DB_645, ("CDL645::AskItem1 end trying***: Pn=%d, m_fRd901f=%d, m_fRd9010=%d,  m_AddrFilledData=%d\r\n", pMtrPara->wPn, pMtr645->fRd901f, pMtr645->fRd9010, pMtr645->bAddrByte));
	}

	DTRACE(DB_645, ("CDL645::AskItem1 : Pn=%d, read meter=%x.\r\n", pMtrPara->wPn, wID)); 

	//wRdId = wID;
	//if (IsPnSPHTDMtr(pMtrPara->wPn) &&  wID==0x941f) //江苏单相分时表的上月ID不同
		//wRdId = 0xd120; 

	if ((iRet=DL645MakeAskData(pMtrPro, pTmp645, wID, bReTryTimes)) <= 0) 	
		return iRet;	

	//读取成功的ID再进行解析
	iLen = 0;
	if ((wID&0x000f)==0x000f 
		&& (pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xaa || pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xdd))//科陆表实测
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-3);
		iLen = pTmp645->wRxDataLen-3;
	}
	else
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-2);
		iLen = pTmp645->wRxDataLen-2;
	}
	bRxLen = iLen;
	bItemLen = Get645TypeLength(wID);
	memset(mBuf, m_bInvdData, 80);
	memcpy(mBuf, pbBuf, iLen);
	if ((wID&0xf) == 0xf) //费率转换
	{
		if ((wID&0xf000) == 0xa000)
		{
			eLen = 3;					
			CheckRate(pMtrPara->bRateTab, mBuf+eLen, eLen);		
			iLen = TOTAL_RATE_NUM*eLen; //长度过长的截掉，过短的补上	

			if (bRxLen == eLen) //若块只返回一个总量数据,则将总量也拷贝到费率1
				memcpy(mBuf+eLen, mBuf, eLen);

			//如果只有单费率,江苏的电表要求只回总数据
			if ((pMtrPara->bRateTab[0]==1 && pMtrPara->bRateTab[1]==0 
				&& pMtrPara->bRateTab[2]==0 && pMtrPara->bRateTab[3]==0) || bRxLen==eLen)
				memset(mBuf+eLen, m_bInvdData, iLen-eLen);

			memcpy(pbBuf, mBuf, iLen);			
		}
		else if ( (wID&0xf000)==0x9000 || (wID&0xff00)==0xb000
			|| (wID&0xff00)==0xb100 || (wID&0xff00)==0xb400
			|| (wID&0xff00)==0xb500 || (wID&0xff00)==0xb800
			|| (wID&0xff00)==0xb900 )
		{
			//AdjRateNumByRdEng(wID, (WORD)iLen); //根据电能数据返回长度调整费率数

			eLen = 4;					
			CheckRate(pMtrPara->bRateTab, mBuf+eLen, eLen);		
			iLen = TOTAL_RATE_NUM*eLen; //长度过长的截掉，过短的补上	

			if (bRxLen == eLen) //若块只返回一个总量数据,则将总量也拷贝到费率1
				memcpy(mBuf+eLen, mBuf, eLen);

			//如果只有单费率,江苏的电表要求只回总数据
			if ((pMtrPara->bRateTab[0]==1 && pMtrPara->bRateTab[1]==0 
				&& pMtrPara->bRateTab[2]==0 && pMtrPara->bRateTab[3]==0) || bRxLen==eLen)
				memset(mBuf+eLen, m_bInvdData, iLen-eLen);


			memcpy(pbBuf, mBuf, iLen);	
		}		
		else if (wID>=0xc32f && wID<=0xc3af)//费率
		{
			if (iLen != 42)//长度正常的	
			{
				iLen = 42;
				memcpy(pbBuf, mBuf, 42);
			}
		}
		else if (wID == 0xb63f)	
		{
			if (iLen == 16)//去掉后面的上下限ID	
			{
				iLen = 12;
				memcpy(pbBuf, mBuf, 12);
			}
			else if (iLen != 12)//长度不对需拆分
				return -1;
		}
		else if (wID==0xb61f || wID==0xb62f)
		{
			if (iLen != 6)//长度不对需拆分
				return -1;
		}
		else if (wID==0xb64f || wID==0xb65f)
		{
			if (iLen != 8)//长度不对需拆分
				return -1;
		}
	}
	else
	{
		memset(mBuf, 0, 80);
		memcpy(mBuf, pbBuf, iLen);
		iLen = bItemLen;
		memcpy(pbBuf, mBuf, iLen);
	}

	return iLen;
}

//描述：读取那些DL645有此数据标识的数据,只采用单ID,对块进行拆分
int AskItemSID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{
	BYTE i, j;
	BYTE bNum, bItemLen;
	BYTE bTemp[100];
	bool bfind=false;
	WORD wSubID=0; 	
	int iLen=0, iRv;

	T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	

	if ((wID&0x000F) == 0xF)
	{
		//if (IsRateId(wID) && !pMtr645->fRd901f && pMtr645->bAddrByte!=0)
		if (pMtr645->fRd9010 && pMtr645->bAddrByte!=0)
		{		
			//简化不必要的抄表
		}
		else
		{
			iLen = AskItem1SID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);
		}
		
		if (pMtr645->bAddrByte == 0)  //基本尝试都未成功，则不再进行拆分尝试
			return iLen;	

		if (!pMtr645->fRd901f && !pMtr645->fRd9010)	//如果9010以及901F均通信不上,则不再进行拆分尝试
			return iLen;

		
		wSubID = wID&0xFF00;
		if (IsRateId(wID) || wSubID==0xB600 || wSubID==0xB300)
		{
			bNum = GetBlockIdNum(wID);
			bItemLen = Get645TypeLength(wID);
			memset(pbBuf, m_bInvdData,  bNum*bItemLen);		
			
			iLen = 0;
			for (i=0; i<bNum; i++)
			{
				memset(bTemp, 0, sizeof(bTemp));
				if (wID==0xB61F || wID==0xB62F)
					j = i+1;			
				else
					j = i;
				iRv = AskItem1SID(pMtrPro, (wID&0xFFF0)+j, bTemp, TXRX_RETRYNUM);
				if (iRv > 0)
				{
					bfind = true;
					memcpy(pbBuf+iLen, bTemp, iRv);
					iLen += iRv;
				}
				else
				{	
					if (iRv < 0) //如果ID表示不支持（含通信返回不支持以及非通信检测不支持的情况），也算通信OK的，上次不予以补抄
						bfind = true;

					if (bNum == 5) //电量、需量、需量时间之类的数据
					{
						if (iRv<0 && i==0) //如果第一个子ID就不支持，则块ID视作不支持
							return -1;
						else if (iRv == -2) //非通信检测不支持的情况(费率调整或是不支持抄读的ID)
							iLen += bItemLen;
						else //if (iRv == -1) //注意需是通信返回的不支持,再停止抄后续子ID,否则单ID依据参数进行费率调整的时候,也会有不支持的,但要继续请求支持的子ID
						{
							iLen = bNum*bItemLen;
							break;
						}
					}
					else //瞬时量或断相
					{
						if (j == 0)	//总量不予判断
							iLen += bItemLen;
						else	//以A相为依据返回,后面的子ID不抄了
						{
							if (iRv<0 && j==1) //如果A相就不支持，则块ID视作不支持
							{
								if ( bfind ) //表示总已经回OK了
									iLen = bNum*bItemLen;
								else
									return -1;
							}
							else
								iLen = bNum*bItemLen;
							break;						
						}						
					}
				}			
			}

			if ( !bfind ) //子ID一个都没回答
			{
				iLen = 0;
				if (wID != 0x901f)	//本块ID不返回且901F支持,则视作本块ID不支持(江苏集抄需求)
				{
					if ( pMtr645->fRd9010 )
					{
						DTRACE(DB_645, ("CDL645::AskItem: wID=%x failed. test 0x9010\r\n", wID));
						if ( AskItem1SID(pMtrPro, 0x9010, pbBuf, 1) > 0)
							return -1;
						return 0;
					}
				}
			}
		}	
	}
	else
	{
		if (wID==0x9010 && pMtr645->bAddrByte!=0 && pMtr645->fRd9010)
		{
			iLen = AskItem1SID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);
		}
		else
		{
			iLen = AskItem1SID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);	
			if (iLen==0 && pMtr645->bAddrByte!=0)	//本ID不返回且关键ID支持,则视作本ID不支持(江苏集抄需求)
			{
				if (pMtr645->fRd9010 && wID!=0x9010)
				{
					DTRACE(DB_645, ("CDL645::AskItem: wID=%x failed. test 0x9010\r\n", wID));
					if ( AskItem1SID(pMtrPro, 0x9010, pbBuf, 1) > 0)
						return -1;
					return 0;
				}
			}
		}
	}

	if (iLen == -2) //转化配置不支持的ID的返回值
		return -1;

	return iLen;	
}

//描述：读取那些DL645有此数据标识的数据,只尝试单ID
int AskItem1SID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes)
{
	BYTE find = 0, i;
	int iRet = -1, iLen;
	WORD wtID;

	T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	//BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;

	//收帧解析的临时变量,每次处理重新开始
	T645Tmp tTmp645; 
	T645Tmp* pTmp645 = &tTmp645;
	memset(&tTmp645, 0, sizeof(T645Tmp));

	if ( !MtrProOpenComm(&pMtrPara->CommPara) )		return 0;

	DTRACE(DB_645, ("CDL645::AskItem1 : wID = %x.\r\n", wID)); 

	if ( Is645NotSuptId(wID) ) //分相电能及相位角、视在功率、零序电流等不支持	
		return -2;	

	//根据费率的参数,调整读取的费率的单ID
	//费率参数的意义,是将对应电表上来的费率顺序,依次对应到终端的费率位置去,比如参数i0=2;i1=3;i2=4;i3=0,表示电表回来的第一个费率数据(抄表ID9011),是对应终端的峰费率9012的位置
	//则需注意,读取峰费率,读取ID是9012,实际的抄表ID是9011,返回的数据认做9012
	if ( IsRateId(wID) )
	{
		if ((wID&0x000f)!=0x000f && (wID&0x000f)!=0x0000) //分费率转换
		{
			for (i=0; i<4; i++)
			{
				if (pMtrPara->bRateTab[i] == (wID&0x000f))
				{					
					wID = (wID&0xfff0)+i+1;
					find = 1;
					break;
				}
			}
			if (!find )	return -2;		
		}
	}

	//确定地址域不够6字节是填0还是填0xaa
	wtID = 0x901f;
	if (pMtr645->bAddrByte == 0) //没尝试过，需要进行尝试
	{			
		/*iRet = DL645MakeAskData(pMtrPro, pTmp645, wtID, 1);
		if (iRet > 0) //地址填0 明确的支持或不支持都表明通信OK	
			pMtr645->bAddrByte = 1;	
		else 
		{
			if (iRet < 0)	//当901f明确不支持时,还必须试下9010是否支持	
				pMtr645->bAddrByte = 1;	
			*/
			wtID = 0x9010;
			iRet = DL645MakeAskData(pMtrPro, pTmp645, wtID, 1);
			
			if (iRet!=0 || pMtr645->bAddrByte==1) //地址填0 明确的支持或不支持都表明通信OK	
				pMtr645->bAddrByte = 1;		
			else
			{		
				//wtID = 0x901f;
				pMtr645->bAddrByte = 2;
				//iRet = DL645MakeAskData(pMtrPro, wtID, 1);

				//if (iRet <= 0)	//当901f明确不支持时,还必须试下9010是否支持	
				//{
					wtID = 0x9010;				
					iRet = DL645MakeAskData(pMtrPro, pTmp645, wtID, 1);

					if (iRet == 0)	//下次再试	 
					{
						pMtr645->bAddrByte = 0;			
						return iRet;				
					}//else //地址填AA 明确的支持或不支持都表明通信OK							
				//}//else //地址填AA 明确的支持或不支持都表明通信OK						
			}	
		//}
		//else if (wtID == 0x9010) //支持单
		{
			if (iRet > 0)
				pMtr645->fRd9010 = true;
			else
				pMtr645->fRd9010 = false;
		}
	}


	if ((iRet=DL645MakeAskData(pMtrPro, pTmp645, wID, bReTryTimes)) <= 0) 	
		return iRet;	

	//读取成功的ID再进行解析
	iLen = 0;
	if ((wID&0x000f)==0x000f 
		&& (pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xaa || pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xdd))//科陆表实测
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-3);
		iLen = pTmp645->wRxDataLen-3;
	}
	else
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-2);
		iLen = pTmp645->wRxDataLen-2;
	}

	return iLen;
}

	
