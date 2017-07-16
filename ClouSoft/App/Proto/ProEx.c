#ifdef EN_PROEX
#include <stdio.h>
//#include "FaProto.h"
#include "ProEx.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "ExcTask.h"
#include "DrvCfg.h"
#include "LibDbStruct.h"
#include "SysDebug.h"
#include "Trace.h"
#include "ComAPI.h"


static bool g_fPro07 = true;
static bool g_fNewInMtrExc = false;
bool g_fMtrParaChg = false;
bool g_fWrInMtr = true;


const TMtrExcDesc g_wMtrExcDesc[] = {
	//bErc				//bNum	//wID
	{ERC_DISORDER,		2,		{0x126f, 0x9010} },
	{ERC_YXCHG,			0,		{0} }
};



bool ProExInit(TPro645Ex* pProEx, TCommPara* pCommPara)
{
	WORD i;
  	BYTE bConnectNew = 0;
 	BYTE bSignStrength = 0;

	pProEx->bMStep = 0;
    memset(pProEx->SendInfo, 0, sizeof(pProEx->SendInfo));
	memset(pProEx->SendPara, 0, sizeof(pProEx->SendPara));
	for (i=8; i<=MAX_ADD_TYPE; i++)//8~14不主动发送
		pProEx->SendInfo[i].fInit = 1;

	pProEx->bGprsSignal = 0;
	pProEx->dwSignalClick = 0;
	pProEx->bGprsConnectOld = 0;

	memset(pProEx->bInMtrAddr, 0xaa, 6);//表址址
	memset(pProEx->bEvtNum, 0, ADD645MAXEVT);
	memset(pProEx->bNewEvtNum, 0, ADD645MAXEVT);

	pProEx->pCommPara = pCommPara;
    
   	WriteItemEx(BN5, PN0, 0x5001, &bSignStrength);  //Modem信号强度
	WriteItemEx(BN5, PN0, 0x5002, &bConnectNew);

    ReadItemEx(BN5, PN0, 0x5007, (BYTE* )&pProEx->dwNeedSendFlag);	//上电初始化发送标志
	for (i=0; i<MAX_PARA_ID; i++)
	{
		if (pProEx->dwNeedSendFlag & (1<<i))
			SetSendParaFlag(&g_ProEx, i);
	}
}

void SetInMtrParaChg()
{
	g_fMtrParaChg = true; 
}


//返回该序号对应的bErc号
BYTE GetMtrErc(BYTE bEvtInx)
{
	if (bEvtInx == 14 || bEvtInx == 15)
		return ERC_DISORDER;
	else if (bEvtInx == 53 || bEvtInx == 54)
		return ERC_YXCHG;
	else
		return 0;
}

int GetMtrErcInx(BYTE bERC)
{
	WORD i;
	BYTE bNum = sizeof(g_wMtrExcDesc) / sizeof(TMtrExcDesc);

	for (i=0; i<bNum; i++)
	{
		if (bERC == g_wMtrExcDesc[i].bErc)
			return i;
	}

	return -1;
}

//触发类电表告警处理
bool DoInMtrExc(BYTE bEvtInx, bool fExcEstb)
{
	int iLen, nRead, iErcInx;
	BYTE bAlrType, bBitMask, bVal;
	WORD i, wPn;
	TTime tmNow;
	BYTE bAlrBuf[1024];
	TBankItem tbItem[MAX_MTR_EXC_ITEM];

	BYTE bERC = GetMtrErc(bEvtInx);
	if (!bERC)
	{
		//DTRACE(DB_FAPROTO, ("CFaProtoEx::HandleMtrExc Evt Index=%d, Erc not found!\r\n", bEvtInx));
		return false;
	}

	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset((BYTE* )tbItem, 0, sizeof(tbItem));
	iErcInx = GetMtrErcInx(bERC);
	if (iErcInx < 0)	//是否属于触发类电表告警
	{
		DTRACE(DB_METER_EXC, ("DoMtrExc: bERC=%d, not MtrExc!", bERC));
		return false;
	}

	bAlrType = GetErcType(bERC);
	if (bAlrType == 0)	//告警属性无效
	{
		DTRACE(DB_METER_EXC, ("DoMtrExc: bERC=%d Erc Type invalid, no need to Save alr rec.", bERC));
		return false;
	}

	wPn = GetInMtrPn();
	
	GetCurTime(&tmNow);
	if (bERC == ERC_YXCHG)
	{
		BYTE bYxPolar, bState=0;
		BYTE bBuf[10];

		memset(bBuf, 0, sizeof(bBuf));
		iLen = ReadItemEx(BN0, PN0, 0x00cf, bBuf);	//C4F12
		if (iLen > 0)
			bYxPolar = (bBuf[1]) & 0x0f;
		else
			bYxPolar = 0x0f;		

		if (bEvtInx == 53)	//开表盖
			i=0;
		else
			i=1;

		bBitMask = (0x1<<i);
		nRead = ReadItemEx(BN2, PN0, 0x1100, &bState);
		if (fExcEstb)	//发生
			bState |= bBitMask;
		else
			bState &= ~bBitMask;

		bVal = (bState^bYxPolar) & bBitMask;
		bState = (bState & (~bBitMask)) | bVal;

		WriteItemEx(BN2, PN0, 0x1100, &bState);
	}
	else
	{
		/*if (fExcEstb)	//发生
			bAlrBuf[0] |= 0x80;
		else	//恢复
			bAlrBuf[0] &= ~0x80;
		bAlrBuf[0] |= wPn & 0x3f;	

		BYTE bItemNum = g_wMtrExcDesc[iErcInx].bItemNum;
		for (i=0; i<bItemNum; i++)
		{
			tbItem[i].wBn = BN0;
			tbItem[i].wPn = wPn;
			tbItem[i].wID = g_wMtrExcDesc[iErcInx].wIDs[i];
		}

		int iLen = DirectReadMtr(tbItem, bItemNum, bAlrBuf+1);
		SaveAlrData( bERC, tmNow, bAlrBuf);*/
	}

	return true;
}

//DLT645-F003自定义协议,专用于内表,借助透明传输的方式发送
//返回值:-1错误或无返回； 0正确响应； 1异常响应
int CallInMeterDat(TPro645Ex* pProEx, BYTE  bId, BYTE *pbBuf, BYTE bTxBufLen, bool fAddrDefault)
{
	int iret,nEvt;
	BYTE *pbTx = pProEx->bTxBuf;	
	WORD i, wLen, wRxOffset, ret = 0;
	TCommPara* pCommPara = pProEx->pCommPara;
	static bool fEvtFileExist = false;
	BYTE *p;

	*pbTx++ = 0x68;	//组帧
	if(fAddrDefault)
		memset(pbTx, 0xaa, 6);
	else
		memcpy(pbTx, pProEx->bInMtrAddr, 6);
	pbTx += 6;
	*pbTx++ = 0x68;
	*pbTx++ = 0x01;	//cmd
	*pbTx++ = bTxBufLen + 3;//len
	*pbTx++ = 0x03;//DI=0xF003
	*pbTx++ = 0xF0;	
	*pbTx++ = bId;
	memcpy(pbTx, pbBuf, bTxBufLen);
	pbTx += bTxBufLen;
	for (p=pProEx->bTxBuf+10; p<pbTx; p++)
	{//换码
		*p += 0x33;
	}	
	//校验
	*pbTx++ = CheckSum(pProEx->bTxBuf, (WORD)bTxBufLen+13);
	*pbTx++ = 0x16;

	//(3)透明传输
	wLen = pbTx-pProEx->bTxBuf;
	i = 0;
	wRxOffset = 0;
//	if (IsDebugOn(DB_FAFRM))
//		TraceFrm("--> FaProtoEx::Send:", pProEx->bTxBuf, wLen);
	//ret = m_pComm->Write(pProEx->bTxBuf, wLen);	
	ret = CommWrite(pCommPara->wPort, pProEx->bTxBuf, wLen, 1000);	
	//ret = MtrFwdCmdSend(m_bTxBuf, wLen)
	//	SetToggleMtrTxDly(wLen);
	while (i < 8)
	{
		//ret = m_pComm->Read(pProEx->bRxBuf+wRxOffset, 512);
		ret = CommRead(pCommPara->wPort, pProEx->bRxBuf+wRxOffset, MAX_645_BUF, 1000);
//		if (ret>0 && IsDebugOn(DB_FAFRM))
//			TraceFrm("<-- FaProtoEx::Receive:", pProEx->bRxBuf+wRxOffset, ret);

		wRxOffset += ret;

		if (wRxOffset > 1024)//防溢出
			break;
		else if (wRxOffset > 0)
		{
			if (pProEx->bRxBuf[wRxOffset-1] == 0x16)
				break;
		}
		i++;
	}

	ret = wRxOffset;
	if (!ret)//(4)返回检测
		return -10;

	//	SetToggleMtrRxDly(ret);	
	iret = RcvBlockDL645(pProEx, pProEx->bRxBuf, ret);
	if(iret <= 0)
		return -10;

	ret = iret;
	if (pProEx->bRxBuf[8] == 0x81)
		iret = ret;
	else if (pProEx->bRxBuf[8] == 0xC1)
		iret = -1;
	else 		
		return -2;

	if (ret > 14)
	{		
		if (bId==ADDTYPEID_READ && pProEx->bRxBuf[8]==0x81)
		{
			memcpy(pbBuf, pProEx->bRxBuf, ret);
		}
		else
		{
			nEvt = ret-14;
			if (nEvt > ADD645MAXEVT)
				nEvt = ADD645MAXEVT;
			for (i=0; i<nEvt;i++)
				pProEx->bNewEvtNum[i] = pProEx->bRxBuf[i+12];

			if (!fEvtFileExist)
			{
				memcpy(pProEx->bEvtNum, pProEx->bNewEvtNum, ADD645MAXEVT);
				
				DoInMtrExc(EXC_DOOR_IDX, pProEx->bNewEvtNum[EXC_DOOR_IDX]&0x80);	//初始化端盖状态
				DoInMtrExc(EXC_REAR_IDX, pProEx->bNewEvtNum[EXC_REAR_IDX]&0x80);	//初始化端盖状态
				fEvtFileExist = true;
				//DoYX();
				//WriteEvtFile();
			}
			else
			{
				if (memcmp(pProEx->bEvtNum, pProEx->bNewEvtNum, ADD645MAXEVT) != 0)
				{
					g_fNewInMtrExc = true;
					//m_dwExcStartClick = GetClick();
				}
			}
		}
	}

	return iret;
}

//0为GR47, 1为SIM, 2为WAVECOM, 3为华为CDMA，4为MC39，5为以太网，6为MODEM拨号，默认为0
BYTE GetModemType()
{					
	BYTE bType=0;
	BYTE bModemType[] = {0, 1, 2, 3, 0, 4, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0};

	ReadItemEx(BN1, PN0, 0x2012, &bType);
	if (bType < 15)
		return bModemType[bType];
	else //if (bType == 5)
		return 0;	
}

extern BYTE g_bSoftVer[];

void GetProgramVerInfo(BYTE *pbBuf)
{	
	BYTE *p = &g_bSoftVer[12];

	*pbBuf++ = AsciiToByte(&p);	
	*pbBuf++ = AsciiToByte(&p); //主版本 2

	*pbBuf++ = 0;				//副版本 1

	*pbBuf++ = 0;				//地方版本 1
	*pbBuf++ = 0;				//地方版本 1

	*pbBuf++ = 0x20;			//20XX年
	*pbBuf++ = *(p+2);			//09年
	*pbBuf++ = *(p+1);			//MM
	*pbBuf++ = *p;				//DD
}

//F003的扩充支持//bId为F003的type id
int DoOneID(TPro645Ex* pProEx, BYTE bId)
{
/*	BYTE i,n,tmp,buf[200],evtbuf[30];
	WORD wNum, wDataLen;
	int ret = -1;
	DWORD clicks;
	bool f = false;
	BYTE bSignal=0, bGprsConnect=0;

	//DTRACE(DB_FAFRM, ("DoOneID bId=%d.\r\n", bId));

	if(!bId || bId > MAX_ADD_TYPE)
		return ret;

	switch( bId )
	{
	case 1://几秒检测一次，不适用flag
		ReadItemEx(BN5, PN0, 0x5001, &bSignal);	//Modem信号强度
		ReadItemEx(BN5, PN0, 0x5002, &bGprsConnect);	//在线标志
		buf[ 0 ] = bSignal;
		buf[ 1 ] = bGprsConnect;
		DTRACE(DB_FAFRM, ("DoOneID ID=%d, Signal=%d, m_bGprsConnect=%d.\r\n", bId, bSignal, bGprsConnect));   
		ret = CallInMeterDat(pProEx, bId, buf, 2, false);
		break;
	case 2:		
		buf[ 0 ] = GetModemType();
		ret = CallInMeterDat(pProEx, bId, buf, 1, false); 		
		break;

	case 3://自行给出m_bComuType
		buf[ 0 ] = 0;	//0:GPRS
		ret = CallInMeterDat(pProEx, bId, buf, 1, false);
		break;

	case 4:
		memset(buf, 0, 200);
		if (ReadItemEx(BN0, PN0, 0x003f, buf) > 0)
		{
			ret = CallInMeterDat(pProEx, bId, buf, 12, false); 	//主/备用IP及端口
		}
		break;

	case 5:
		memset(buf, 0, 200);
		if (ReadItemEx(BN0, PN0, 0x004f, buf) > 0)
		{
			ret = CallInMeterDat(pProEx, bId, buf, 16, false); 	
		}
		break;	

	case 6:
		if (ReadItemEx(BN10, PN0, 0xA040, buf)>0 && ReadItemEx(BN10, PN0, 0xA041, &buf[2])>0)
			ret = CallInMeterDat(pProEx, bId, buf, 4, false); 
		break;

	case 7:
		memset(buf, 0, 200);	
		GetProgramVerInfo(buf);		
		ret = CallInMeterDat(pProEx, bId, buf, 48, false); 
		break;

	case ADDTYPEID_PHASE:	//相角
		ret = CallInMeterDat(pProEx, bId, buf, 0, false);
		if (ret > 0)
			ret = WriteItemEx(BN0, PN0, 0xb66f, buf);

		break;

	case ADDTYPEID_READ:	//读模块参数	12
		buf[0] = 0;			
		buf[1] = 0;		//偏移地址	2字节
		buf[2] = Module_PARA_LEN;	//读取长度	1字节
		ret = CallInMeterDat( pProEx, bId, buf, 3, false );
		break;

	case ADDTYPEID_WRITE:	//写模块参数	13
		memset(buf, 0, sizeof(buf));
		wDataLen = MakeModuleParaFrm(0, Module_PARA_LEN, buf);
		ret = CallInMeterDat( pProEx, bId, buf, wDataLen, false);
		break;

	default:
		break;

	}

	return ret;*/
	return 0;
}


bool MasterToInMtrUpdate(TPro645Ex* pProEx)
{
	int ret;
	BYTE i,tmp;

NEXT:
	pProEx->bMStep++;
	if( pProEx->bMStep > MAX_ADD_TYPE )
		pProEx->bMStep = 1;//[1,MAX_ADD_TYPE]

	i = pProEx->bMStep;

	if((!pProEx->SendInfo[ i ].fInit || pProEx->SendInfo[ i ].fNeedSend) && (i<8))//|| i == 1 )//type=1属于常规循环类
	{
		ret = DoOneID(pProEx, i );

		if( ret > 0 )
		{
			pProEx->SendInfo[ i ].fInit = 1;
			pProEx->SendInfo[ i ].fNeedSend = 0;
			pProEx->SendInfo[ i ].SendErr = 0;
#ifndef SYS_WIN
			//SetLedCtrlMode(LED_LOCAL, LED_MODE_ON);
#endif
		}
		else 
		{
			pProEx->SendInfo[ i ].SendErr++;
			if( pProEx->SendInfo[ i ].SendErr >= 3 )
			{
				pProEx->SendInfo[ i ].fNeedSend = 0;
				pProEx->SendInfo[ i ].SendErr = 0;
#ifndef SYS_WIN
				//SetLedCtrlMode(LED_LOCAL, LED_MODE_OFF);
#endif
			}
		}
	}
	else
	{
		if( i < MAX_ADD_TYPE )
		{
			//Sleep(10);	//睡眠久了会影响抄表
			goto NEXT;
		}
	}

	return true;
}



//DLT645格式自定义,专用于内表,借助透明传输的方式发送
//返回值:-1错误或无返回； 0正确响应； 1异常响应
int CallInMeterDat1(TPro645Ex* pProEx, WORD  wID, BYTE *pbBuf, BYTE bTxBufLen, bool fAddrDefault)
{
	int iret,nEvt;
	BYTE bConfirmByte, bDenyByte;
	BYTE *pbTx = pProEx->bTxBuf;
	WORD i, wOffset, wRxOffset, wLen, ret = 0;
	DWORD dw07ID = 0;
	BYTE *p=pbBuf;
	TCommPara* pCommPara = pProEx->pCommPara;

	static const DWORD dwMapTo07ID[][2] = {
		{0xc010, 0x04000101},
		{0xc011, 0x04000102},
		{0xc034, 0x04000401},
	};
	
	if (g_fPro07)
	{
		for (i=0; i<sizeof(dwMapTo07ID)/(2*sizeof(DWORD)); i++)
		{
			if (wID == dwMapTo07ID[i][0])
			{
				dw07ID = dwMapTo07ID[i][1];
				break;
			}
		}
	}
	else
	{
		dw07ID = wID;
	}

	if (!dw07ID)
		return -10;

	*pbTx++ = 0x68;	//组帧
	if(fAddrDefault)
		memset(pbTx, 0xaa, 6);
	else
		memcpy(pbTx, pProEx->bInMtrAddr, 6);
	pbTx += 6;	
	*pbTx++ = 0x68;
	if (g_fPro07)
	{
		*pbTx++ = 0x11;	//cmd
		*pbTx++ = bTxBufLen + 4;//len
	}
	else
	{
		*pbTx++ = 0x01;	//cmd
		*pbTx++ = bTxBufLen + 2;//len
	}
	*pbTx++ = dw07ID&0xff;
	*pbTx++ = (dw07ID>>8)&0xff;
	if (g_fPro07)
	{
		*pbTx++ = (dw07ID>>16)&0xff;
		*pbTx++ = (dw07ID>>24)&0xff;
	}

	memcpy(pbTx, pbBuf, bTxBufLen);
	pbTx += bTxBufLen;
	for (p=pProEx->bTxBuf+10; p<pbTx; p++)
	{
		*p += 0x33;		//换码
	}

	if (g_fPro07)
		wOffset = 14;
	else
		wOffset = 12;
	//校验
	*pbTx++ = CheckSum(pProEx->bTxBuf, (WORD)bTxBufLen+wOffset);
	*pbTx++ = 0x16;

	//(3)透明传输
	wLen = pbTx-pProEx->bTxBuf;

	wRxOffset = 0;

//	if (IsDebugOn(DB_FAFRM))
//		TraceFrm("--> FaProtoEx::Send:", pProEx->bTxBuf, wLen);
	//ret = m_pComm->Write(pProEx->bTxBuf, wLen);
	ret = CommWrite(pCommPara->wPort, pProEx->bTxBuf, wLen, 1000);	
	//ret = MtrFwdCmdSend(pProEx->bTxBuf, wLen);	
	//	SetToggleMtrTxDly(wLen);
	i = 0;
	while (i < 4)
	{
		//ret = m_pComm->Read(m_bMtrFwdBuf+wRxOffset, 512, 1000);
		ret = CommRead(pCommPara->wPort, pProEx->bRxBuf+wRxOffset, MAX_645_BUF, 1000);
//		if (ret>0 && IsDebugOn(DB_FAFRM))
//			TraceFrm("<-- FaProtoEx::Receive:", pProEx->bRxBuf+wRxOffset, ret);

		wRxOffset += ret;	
		if (wRxOffset > 1024)//防溢出
			break;
		else if (wRxOffset > 0)
		{
			if (pProEx->bRxBuf[wRxOffset-1] == 0x16)
				break;
		}
		i++;
	}

	ret = wRxOffset;		
	if (!ret)//(4)返回检测
		return -10;

	//	SetToggleMtrRxDly(ret);	
	iret = RcvBlockDL645(pProEx, pProEx->bRxBuf, ret);	
	if(iret <= 0)
		return -10;

	ret = iret;	

	if (g_fPro07)	//07规约
	{
		bConfirmByte = 0x91;
		bDenyByte = 0xD1;
	}
	else
	{
		bConfirmByte = 0x81;
		bDenyByte = 0xC1;
	}

	if (pProEx->bRxBuf[8] == bConfirmByte)
		iret = ret;
	else if (pProEx->bRxBuf[8] == bDenyByte)
		iret = -1;
	else 		
		return -2;

	memcpy(pbBuf, pProEx->bRxBuf, ret);

	return iret;
}


int DoStdID(TPro645Ex* pProEx, WORD wId)
{
    WORD wOffset = 0;
	BYTE buf[300];
	int ret=-1;
	DWORD dwClick;
	BYTE bInnAddr[30];
	BYTE bBuf[100];
	static bool fAddrValid = false;
	static DWORD dwReadAddrClick=0;

	memset(bInnAddr, 0, sizeof(bInnAddr));
	switch (wId)
	{
	case 0xc034:
		dwClick = GetClick();
		if (!fAddrValid)
		{
			if( !dwReadAddrClick || dwClick - dwReadAddrClick > 5*60 )
			{
				dwReadAddrClick = dwClick;
				ret = CallInMeterDat1(pProEx, wId, buf, 0, true);
				if (ret > 0)
				{
					//ReadItemEx(BN5, PN0, 0x5003, bInnAddr);										
					memset(bBuf, 0, sizeof(bBuf));
					if (ReadItemEx(BN0, PN1, 0x8902, bBuf) <= 0)					
						memset(bInnAddr, 0xee, 6);
					else
						memcpy(bInnAddr, &bBuf[F10_SN_LEN+3], 6);					

				    if (g_fPro07)
				    	wOffset = 14;
				    else
				    	wOffset = 12;
					if (memcmp(bInnAddr, buf+wOffset, 6) != 0)
					{
						memcpy(pProEx->bInMtrAddr, buf+wOffset, 6);
						WriteItemEx(BN5, PN0, 0x5003, pProEx->bInMtrAddr);	//写入内表地址
						TrigerSavePara();
						//TrigerSaveBank(BN5, 0, -1);
						InitInMeterPn();	//默认内表测量点参数
					}
					fAddrValid = true;
				}
			}
		}	
		break;
	}
	
	return ret;
}


void MasterMainProcess(TPro645Ex* pProEx)
{
	int ret;
	BYTE i;	

	//循环请求2---std645
	WORD AskIDGrp[] = {0xc034, 0};
	i = 0;
	while( AskIDGrp[i] )
	{
		DoStdID(pProEx, AskIDGrp[i]);
		i++;	
	}

	//循环请求1---F003
	MasterToInMtrUpdate(pProEx);
}

bool VeryRxFrmAddr()
{
	return true;
}


//描述:接收一个数据块,判断是否接收到一个完整的通信帧
//返回:返回已经扫描过的字节数,如果收到一个完整的通信帧则返回正数,否则返回负数
int RcvBlockDL645(TPro645Ex* pProEx, BYTE* pbBlock, int nLen)
{
	int i, j;
	WORD wRxPtr645,RxCnt645,RxDataLen645;
	BYTE b,step=0; 

	wRxPtr645 = 0;
	RxCnt645 = 0;
	RxDataLen645 = 0;

	for (i=0; i<nLen; i++)
	{
		b = *pbBlock++;
		switch (step) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pProEx->bRxBuf[0] = 0x68;
				wRxPtr645 = 1;
				RxCnt645 = 10;       
				step = 1;
			}
			break;
		case 1:    //数据域前的数据
			pProEx->bRxBuf[wRxPtr645++] = b;
			RxCnt645--;
			if (RxCnt645 == 0)   //接收完，进行校验
			{
				RxDataLen645 = pProEx->bRxBuf[FAP_LEN_645];

				if (pProEx->bRxBuf[7]==0x68 && 
					RxDataLen645+FAP_FIX_LEN_645 < FAP_FRM_SIZE_645 &&
					(pProEx->bRxBuf[FAP_CMD_645] & FAP_CMD_DIR_645) == FAP_CMD_UP_645)  //防止接收到MODEM返回的自己发出去的帧
				{
					RxCnt645 = RxDataLen645 + 1;
					step = 2;
				}
				else
				{
					step = 0;
				}
			}
			break;

		case 2:     //数据 + 检验码 + 结束码
			pProEx->bRxBuf[wRxPtr645++] = b;
			RxCnt645--;
			if (RxCnt645 == 0)   //接收完，进行校验
			{
				step = 0;
				if (pProEx->bRxBuf[wRxPtr645-1]==0x16 && 
					pProEx->bRxBuf[wRxPtr645-2]==CheckSum(pProEx->bRxBuf, RxDataLen645+10))
				{
					if (VeryRxFrmAddr() == false)				
						return -1;

					for ( j=0; j<RxDataLen645; j++)
					{
						pProEx->bRxBuf[FAP_DATA_645+j] -= 0x33;
					}

					return (RxDataLen645+12);
				}
			}
			break;

		default:
			step = 0;
			break;
		} 
	}

	return -nLen;
}

void UpdateSendFlg(TPro645Ex* pProEx)
{
	BYTE bId;
	bool fFlg = false;
	for (bId=1; bId<=MAX_ADD_TYPE; bId++)
	{
		if (pProEx->SendInfo[bId].fNeedSend)
			fFlg = true;

		if (pProEx->SendInfo[bId].fInit == 0)
			fFlg = true;
	}

	g_fWrInMtr = fFlg;
}

void HandleMtrExc(TPro645Ex* pProEx)
{
	WORD i;
	BYTE bERC;
	bool fExcEstb;

	//判别有事件的话优先请求
	for (i=0; i<ADD645MAXEVT; i++)
	{
		if (pProEx->bNewEvtNum[i] != pProEx->bEvtNum[i])
		{
			//DTRACE(DB_FAFRM, ("CFaProtoEx::HandleMtrExc Evt Index=%d, Old = %x, New =%x.\r\n", i, m_EvtNum[i], m_NewEvtNum[i]));
			pProEx->bEvtNum[i] = pProEx->bNewEvtNum[i];
			fExcEstb = (pProEx->bNewEvtNum[i] & 0x80);

			DoInMtrExc(i, fExcEstb);
			//if (DoMtrExc(i, fExcEstb))			//TO DO:读取电表事件记录
				//WriteEvtFile();
		}
	}

	//DoYX();   //在2路门节点都检测完成后才执行
}

void SetSendFlag(TPro645Ex* pProEx, BYTE Id)
{
	if( !Id || Id > MAX_ADD_TYPE )
		return;

	pProEx->SendInfo[ Id ].fNeedSend = 1;
}

void SetSendParaFlag(TPro645Ex* pProEx, BYTE bIdx)
{
	if( bIdx > MAX_PARA_ID )
		return;
	
	pProEx->SendPara[ bIdx ].fNeedSend = 1;

	pProEx->dwNeedSendFlag |= (1<<bIdx);
	WriteItemEx(BN5, PN0, 0x5007, (BYTE* )&pProEx->dwNeedSendFlag);
}

void RfsMtrPara(TPro645Ex* pProEx)
{
	BYTE bBuf[100];
	BYTE bProp, bPort;
	TCommPara* pCommPara = pProEx->pCommPara;

	memset(pProEx->bInMtrAddr, 0xaa, sizeof(pProEx->bInMtrAddr));//表址址
	//memset(m_bInMtrDefaultAddr, SPECIAL_ADDR, sizeof(m_bInMtrDefaultAddr));

	pCommPara->wPort = COMM_METER; 
	pCommPara->dwBaudRate = INMTR_BAUD; 
	pCommPara->bByteSize = 8; 
	pCommPara->bStopBits = ONESTOPBIT;
	pCommPara->bParity = EVENPARITY;
	if (ReadItemEx(BN0, PN1, 0x8902, bBuf)>0 && ReadItemEx(BN0, PN1, 0x8901, &bProp)>0)
	{
		if (bProp == PN_PROP_METER)
		{
			memcpy(pProEx->bInMtrAddr, &bBuf[MTR_ADDR_OFFSET], sizeof(pProEx->bInMtrAddr));   //表址址
			bPort = MeterPortToPhy(bBuf[MTR_PORT_OFFSET]);
			if (bPort != COMM_METER)
			{
				bPort = COMM_METER;
				DTRACE(DB_FAFRM, ("CFaProtoEx::RfsMtrPara() : InitInMeterPn due to port diff. \n"));
				InitInMeterPn();
			}
		}
		else
		{
			DTRACE(DB_FAFRM, ("CFaProtoEx::RfsMtrPara() : InitInMeterPn due to bProp diff. \n"));
			InitInMeterPn();
		}
	}
}

int WriteParaToInnMeter(TPro645Ex* pProEx, int nOffset, WORD wDataLen, BYTE* bBuf)
{
/*	int nRet = 0;
	int nLen = MakeModuleParaFrm(nOffset, wDataLen, bBuf);
	if (nLen > 0)
		nRet = CallInMeterDat( pProEx, ADDTYPEID_WRITE, bBuf, nLen, true);

	return nRet;*/
	return 0;
}

void CheckModulePara(TPro645Ex* pProEx)
{
/*	WORD i;
	int ret = 0;    
	bool fNeedSave = false;
	BYTE bBuf[150];

	for (i=0; i<MAX_PARA_ID; i++)
	{
		if (pProEx->SendPara[ i ].fNeedSend == 1)
		{
			TModuleParaDesc* pDesc = GetParaDesc(i);
			if (pDesc == NULL)
				continue;

			ret = WriteParaToInnMeter(pProEx, pDesc->wOffset, pDesc->wLen, bBuf);
			if (ret > 0)
			{
				pProEx->SendPara[ i ].fInit = 1;	//写命令不放在循环内执行，触发即执行
				pProEx->SendPara[ i ].fNeedSend = 0;
				pProEx->SendPara[ i ].SendErr = 0;

				pProEx->dwNeedSendFlag &= ~(1<<i);
				WriteItemEx(BN5, PN0, 0x5007, (BYTE* )&pProEx->dwNeedSendFlag);
				fNeedSave = true;
			}
			else
			{
				pProEx->SendPara[ i ].SendErr++;
				if( pProEx->SendPara[ i ].SendErr >= 3 )
				{
					pProEx->SendPara[ i ].fNeedSend = 0;
					pProEx->SendPara[ i ].SendErr = 0;
				}
			}
		}
	}

	if (fNeedSave)
		TrigerSavePara();
		//TrigerSaveBank(BN5, 0, -1);*/
}

void DoFaProtoEx(TPro645Ex* pProEx)
{
/*	BYTE bSignal, bGprsConnect;
	WORD i;
	//static DWORD dwSignalClick=0;
	TCommPara* pCommPara = pProEx->pCommPara;

	if (g_fNewInMtrExc)
	{
		HandleMtrExc(pProEx);
		g_fNewInMtrExc = false;
	}

	//CheckModulePara(pProEx);	//for debug

	//if (GetClick()-m_dwLastCheckSiganlClick >= 1)
	{
//		CheckMtrEvtSignal();	//检测到电表事件握手信号	//for debug
		//	m_dwLastCheckSiganlClick = GetClick();
	}

	ReadItemEx(BN5, PN0, 0x5001, &bSignal);	//Modem信号强度
	ReadItemEx(BN5, PN0, 0x5002, &bGprsConnect);	//在线标志
	if (pProEx->bGprsConnectOld != bGprsConnect)	//在线标志有变化
	{
		SetSendFlag(pProEx, 1);
		pProEx->bGprsConnectOld = bGprsConnect;
	}
	else if (pProEx->bGprsSignal != bSignal && GetClick() - pProEx->dwSignalClick > 5)		//信号强度有变化且　比上次变化时超过５秒
	{
		SetSendFlag(pProEx, 1);
		pProEx->bGprsSignal = bSignal;
		pProEx->dwSignalClick = GetClick();
	}
	else if (GetClick() - pProEx->dwSignalClick > 60)	//120 刷新一次
	{
		SetSendFlag(pProEx, 1);
		pProEx->dwSignalClick = GetClick();
	}

	if (g_fMtrParaChg)
	{
		RfsMtrPara(pProEx);
		g_fMtrParaChg = false;
	}

	UpdateSendFlg(pProEx); //更新g_fWrInMtr
	if (g_fWrInMtr == true)//有数据要发给内部电表
	{
		//DTRACE(DB_FAFRM, ("CFaProtoEx::DoFaProtoEx g_fWrInMtr=true.\r\n"));
		if (!MtrProOpenComm(pCommPara))
		{
			DTRACE(DB_FAFRM, ("CFaProtoEx::DoFaProtoEx fail to open Comm!\r\n"));
			return;     			
		}

		MasterMainProcess(pProEx);
	}*/
}

#endif



