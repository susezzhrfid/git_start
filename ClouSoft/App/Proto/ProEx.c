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
	for (i=8; i<=MAX_ADD_TYPE; i++)//8~14����������
		pProEx->SendInfo[i].fInit = 1;

	pProEx->bGprsSignal = 0;
	pProEx->dwSignalClick = 0;
	pProEx->bGprsConnectOld = 0;

	memset(pProEx->bInMtrAddr, 0xaa, 6);//��ַַ
	memset(pProEx->bEvtNum, 0, ADD645MAXEVT);
	memset(pProEx->bNewEvtNum, 0, ADD645MAXEVT);

	pProEx->pCommPara = pCommPara;
    
   	WriteItemEx(BN5, PN0, 0x5001, &bSignStrength);  //Modem�ź�ǿ��
	WriteItemEx(BN5, PN0, 0x5002, &bConnectNew);

    ReadItemEx(BN5, PN0, 0x5007, (BYTE* )&pProEx->dwNeedSendFlag);	//�ϵ��ʼ�����ͱ�־
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


//���ظ���Ŷ�Ӧ��bErc��
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

//��������澯����
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
	if (iErcInx < 0)	//�Ƿ����ڴ�������澯
	{
		DTRACE(DB_METER_EXC, ("DoMtrExc: bERC=%d, not MtrExc!", bERC));
		return false;
	}

	bAlrType = GetErcType(bERC);
	if (bAlrType == 0)	//�澯������Ч
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

		if (bEvtInx == 53)	//�����
			i=0;
		else
			i=1;

		bBitMask = (0x1<<i);
		nRead = ReadItemEx(BN2, PN0, 0x1100, &bState);
		if (fExcEstb)	//����
			bState |= bBitMask;
		else
			bState &= ~bBitMask;

		bVal = (bState^bYxPolar) & bBitMask;
		bState = (bState & (~bBitMask)) | bVal;

		WriteItemEx(BN2, PN0, 0x1100, &bState);
	}
	else
	{
		/*if (fExcEstb)	//����
			bAlrBuf[0] |= 0x80;
		else	//�ָ�
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

//DLT645-F003�Զ���Э��,ר�����ڱ�,����͸������ķ�ʽ����
//����ֵ:-1������޷��أ� 0��ȷ��Ӧ�� 1�쳣��Ӧ
int CallInMeterDat(TPro645Ex* pProEx, BYTE  bId, BYTE *pbBuf, BYTE bTxBufLen, bool fAddrDefault)
{
	int iret,nEvt;
	BYTE *pbTx = pProEx->bTxBuf;	
	WORD i, wLen, wRxOffset, ret = 0;
	TCommPara* pCommPara = pProEx->pCommPara;
	static bool fEvtFileExist = false;
	BYTE *p;

	*pbTx++ = 0x68;	//��֡
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
	{//����
		*p += 0x33;
	}	
	//У��
	*pbTx++ = CheckSum(pProEx->bTxBuf, (WORD)bTxBufLen+13);
	*pbTx++ = 0x16;

	//(3)͸������
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

		if (wRxOffset > 1024)//�����
			break;
		else if (wRxOffset > 0)
		{
			if (pProEx->bRxBuf[wRxOffset-1] == 0x16)
				break;
		}
		i++;
	}

	ret = wRxOffset;
	if (!ret)//(4)���ؼ��
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
				
				DoInMtrExc(EXC_DOOR_IDX, pProEx->bNewEvtNum[EXC_DOOR_IDX]&0x80);	//��ʼ���˸�״̬
				DoInMtrExc(EXC_REAR_IDX, pProEx->bNewEvtNum[EXC_REAR_IDX]&0x80);	//��ʼ���˸�״̬
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

//0ΪGR47, 1ΪSIM, 2ΪWAVECOM, 3Ϊ��ΪCDMA��4ΪMC39��5Ϊ��̫����6ΪMODEM���ţ�Ĭ��Ϊ0
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
	*pbBuf++ = AsciiToByte(&p); //���汾 2

	*pbBuf++ = 0;				//���汾 1

	*pbBuf++ = 0;				//�ط��汾 1
	*pbBuf++ = 0;				//�ط��汾 1

	*pbBuf++ = 0x20;			//20XX��
	*pbBuf++ = *(p+2);			//09��
	*pbBuf++ = *(p+1);			//MM
	*pbBuf++ = *p;				//DD
}

//F003������֧��//bIdΪF003��type id
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
	case 1://������һ�Σ�������flag
		ReadItemEx(BN5, PN0, 0x5001, &bSignal);	//Modem�ź�ǿ��
		ReadItemEx(BN5, PN0, 0x5002, &bGprsConnect);	//���߱�־
		buf[ 0 ] = bSignal;
		buf[ 1 ] = bGprsConnect;
		DTRACE(DB_FAFRM, ("DoOneID ID=%d, Signal=%d, m_bGprsConnect=%d.\r\n", bId, bSignal, bGprsConnect));   
		ret = CallInMeterDat(pProEx, bId, buf, 2, false);
		break;
	case 2:		
		buf[ 0 ] = GetModemType();
		ret = CallInMeterDat(pProEx, bId, buf, 1, false); 		
		break;

	case 3://���и���m_bComuType
		buf[ 0 ] = 0;	//0:GPRS
		ret = CallInMeterDat(pProEx, bId, buf, 1, false);
		break;

	case 4:
		memset(buf, 0, 200);
		if (ReadItemEx(BN0, PN0, 0x003f, buf) > 0)
		{
			ret = CallInMeterDat(pProEx, bId, buf, 12, false); 	//��/����IP���˿�
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

	case ADDTYPEID_PHASE:	//���
		ret = CallInMeterDat(pProEx, bId, buf, 0, false);
		if (ret > 0)
			ret = WriteItemEx(BN0, PN0, 0xb66f, buf);

		break;

	case ADDTYPEID_READ:	//��ģ�����	12
		buf[0] = 0;			
		buf[1] = 0;		//ƫ�Ƶ�ַ	2�ֽ�
		buf[2] = Module_PARA_LEN;	//��ȡ����	1�ֽ�
		ret = CallInMeterDat( pProEx, bId, buf, 3, false );
		break;

	case ADDTYPEID_WRITE:	//дģ�����	13
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

	if((!pProEx->SendInfo[ i ].fInit || pProEx->SendInfo[ i ].fNeedSend) && (i<8))//|| i == 1 )//type=1���ڳ���ѭ����
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
			//Sleep(10);	//˯�߾��˻�Ӱ�쳭��
			goto NEXT;
		}
	}

	return true;
}



//DLT645��ʽ�Զ���,ר�����ڱ�,����͸������ķ�ʽ����
//����ֵ:-1������޷��أ� 0��ȷ��Ӧ�� 1�쳣��Ӧ
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

	*pbTx++ = 0x68;	//��֡
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
		*p += 0x33;		//����
	}

	if (g_fPro07)
		wOffset = 14;
	else
		wOffset = 12;
	//У��
	*pbTx++ = CheckSum(pProEx->bTxBuf, (WORD)bTxBufLen+wOffset);
	*pbTx++ = 0x16;

	//(3)͸������
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
		if (wRxOffset > 1024)//�����
			break;
		else if (wRxOffset > 0)
		{
			if (pProEx->bRxBuf[wRxOffset-1] == 0x16)
				break;
		}
		i++;
	}

	ret = wRxOffset;		
	if (!ret)//(4)���ؼ��
		return -10;

	//	SetToggleMtrRxDly(ret);	
	iret = RcvBlockDL645(pProEx, pProEx->bRxBuf, ret);	
	if(iret <= 0)
		return -10;

	ret = iret;	

	if (g_fPro07)	//07��Լ
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
						WriteItemEx(BN5, PN0, 0x5003, pProEx->bInMtrAddr);	//д���ڱ��ַ
						TrigerSavePara();
						//TrigerSaveBank(BN5, 0, -1);
						InitInMeterPn();	//Ĭ���ڱ���������
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

	//ѭ������2---std645
	WORD AskIDGrp[] = {0xc034, 0};
	i = 0;
	while( AskIDGrp[i] )
	{
		DoStdID(pProEx, AskIDGrp[i]);
		i++;	
	}

	//ѭ������1---F003
	MasterToInMtrUpdate(pProEx);
}

bool VeryRxFrmAddr()
{
	return true;
}


//����:����һ�����ݿ�,�ж��Ƿ���յ�һ��������ͨ��֡
//����:�����Ѿ�ɨ������ֽ���,����յ�һ��������ͨ��֡�򷵻�����,���򷵻ظ���
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
		case 1:    //������ǰ������
			pProEx->bRxBuf[wRxPtr645++] = b;
			RxCnt645--;
			if (RxCnt645 == 0)   //�����꣬����У��
			{
				RxDataLen645 = pProEx->bRxBuf[FAP_LEN_645];

				if (pProEx->bRxBuf[7]==0x68 && 
					RxDataLen645+FAP_FIX_LEN_645 < FAP_FRM_SIZE_645 &&
					(pProEx->bRxBuf[FAP_CMD_645] & FAP_CMD_DIR_645) == FAP_CMD_UP_645)  //��ֹ���յ�MODEM���ص��Լ�����ȥ��֡
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

		case 2:     //���� + ������ + ������
			pProEx->bRxBuf[wRxPtr645++] = b;
			RxCnt645--;
			if (RxCnt645 == 0)   //�����꣬����У��
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

	//�б����¼��Ļ���������
	for (i=0; i<ADD645MAXEVT; i++)
	{
		if (pProEx->bNewEvtNum[i] != pProEx->bEvtNum[i])
		{
			//DTRACE(DB_FAFRM, ("CFaProtoEx::HandleMtrExc Evt Index=%d, Old = %x, New =%x.\r\n", i, m_EvtNum[i], m_NewEvtNum[i]));
			pProEx->bEvtNum[i] = pProEx->bNewEvtNum[i];
			fExcEstb = (pProEx->bNewEvtNum[i] & 0x80);

			DoInMtrExc(i, fExcEstb);
			//if (DoMtrExc(i, fExcEstb))			//TO DO:��ȡ����¼���¼
				//WriteEvtFile();
		}
	}

	//DoYX();   //��2·�Žڵ㶼�����ɺ��ִ��
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

	memset(pProEx->bInMtrAddr, 0xaa, sizeof(pProEx->bInMtrAddr));//��ַַ
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
			memcpy(pProEx->bInMtrAddr, &bBuf[MTR_ADDR_OFFSET], sizeof(pProEx->bInMtrAddr));   //��ַַ
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
				pProEx->SendPara[ i ].fInit = 1;	//д�������ѭ����ִ�У�������ִ��
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
//		CheckMtrEvtSignal();	//��⵽����¼������ź�	//for debug
		//	m_dwLastCheckSiganlClick = GetClick();
	}

	ReadItemEx(BN5, PN0, 0x5001, &bSignal);	//Modem�ź�ǿ��
	ReadItemEx(BN5, PN0, 0x5002, &bGprsConnect);	//���߱�־
	if (pProEx->bGprsConnectOld != bGprsConnect)	//���߱�־�б仯
	{
		SetSendFlag(pProEx, 1);
		pProEx->bGprsConnectOld = bGprsConnect;
	}
	else if (pProEx->bGprsSignal != bSignal && GetClick() - pProEx->dwSignalClick > 5)		//�ź�ǿ���б仯�ҡ����ϴα仯ʱ��������
	{
		SetSendFlag(pProEx, 1);
		pProEx->bGprsSignal = bSignal;
		pProEx->dwSignalClick = GetClick();
	}
	else if (GetClick() - pProEx->dwSignalClick > 60)	//120 ˢ��һ��
	{
		SetSendFlag(pProEx, 1);
		pProEx->dwSignalClick = GetClick();
	}

	if (g_fMtrParaChg)
	{
		RfsMtrPara(pProEx);
		g_fMtrParaChg = false;
	}

	UpdateSendFlg(pProEx); //����g_fWrInMtr
	if (g_fWrInMtr == true)//������Ҫ�����ڲ����
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



