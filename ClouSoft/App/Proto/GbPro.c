#include <stdio.h>
//#include "Sysapi.h"
#include "GbPro.h"
#include "ProHook.h"
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
#include "Sftp.h"
#include "CommonTask.h"
#include "SysApi.h"
#include "FileMgr.h"
#include "FlashMgr.h"
#include "MtrAPI.h"
#include "MeterPro.h"
#include "ParaMgr.h"
#include "DrvCfg.h"
#include "AutoSend.h"
#ifndef SYS_WIN
#include "Sample.h"
#include "EsamCmd.h"
#include "Typedef.h"
#endif


////////////////////////////////////////////////////////////////////////////////////////////
//GbPro˽�г�Ա����

////////////////////////////////////////////////////////////////////////////////////////////
BYTE g_bFrmHandle = 0x00;
bool m_fRxGoOnDB;   //���յ������������������
BYTE g_bEnergyClrPnt = 0;

//���ڽ��յı�ʶתFnPn
TSgIdPn			m_RxIdPnGrp[POINT_NUM];//���յ���һ��DiDaDg��ת��Ϊһ��IdPn
//TSgIdPn			m_TxIdPnGrp[64];//�����ϱ���һ��DiDaDg��ת��Ϊһ��IdPn
char			m_szCmdLine[100];
//TRANSMIT_FILE_INFO g_TransmitFileInfo;

//ȫ�ֺ���
BYTE GetFnPnFromDaDt(BYTE bDA1, BYTE bDA2, BYTE bDI1, BYTE bDI2, BYTE bDI3, BYTE bDI4,
					 TGbFnPn* pIdPnGrp, BYTE bGrpSize)
{	
	BYTE i,k;
	WORD pn[POINT_NUM];
	WORD pnnum;
	DWORD dwID = 0;

	memset((BYTE*)pn, 0, sizeof(pn));

	//���pn��
	if (!bDA1 || !bDA2)
	{
		pn[0] = 0;
		pnnum = 1;
	}
	else
	{
		if (0xff==bDA1 && 0xff==bDA2)		
		{
		}
		else
		{
			WORD startpn = (bDA2<<3) - 7;

			for (i=0, pnnum=0; i < 8; i++,bDA1>>=1)
			{
				if (bDA1 & 1)
					pn[pnnum++] = startpn + i;
			}
		}
	}

	//���ID��
	dwID = (DWORD)((DWORD)(bDI4 << 24) | (DWORD)(bDI3 << 16) | (DWORD)(bDI2 << 8) | (DWORD)bDI1);

	//���fn,pn�飬������pn��fn
	k = 0;
	for (i = 0; i < pnnum; i++)
	{

		if (k >= bGrpSize)
			return bGrpSize;

		pIdPnGrp[k].bPn = pn[i];
		pIdPnGrp[k].dwId = dwID;
		k++;

	}

	return k;
}

BYTE GetFnPn(BYTE *p, TGbFnPn* pFnPnGrp, BYTE bGrpSize)
{	
	return GetFnPnFromDaDt(p[0], p[1], p[2], p[3], p[4], p[5], pFnPnGrp, bGrpSize);
}

bool GetOneId(BYTE *p, DWORD* dwId)
{	
	//*dwId = 0;
	//���ID��
	*dwId = (DWORD)((DWORD)(p[5] << 24) | (DWORD)(p[4] << 16) | (DWORD)(p[3] << 8) | (DWORD)p[2]);
	if(*dwId > 0)
		return true;

	return false;
}

bool PutToDaDt(BYTE bIdx, BYTE* p, TSgDaDt* pDaDt, BYTE bSize)
{
	if (bIdx >= bSize)
		return false;

	pDaDt[bIdx].bDA1 = p[0];
	pDaDt[bIdx].bDA2 = p[1];
	pDaDt[bIdx].bDI1 = p[2];
	pDaDt[bIdx].bDI2 = p[3];
	pDaDt[bIdx].bDI3 = p[4];
	pDaDt[bIdx].bDI4 = p[5];
	pDaDt[bIdx].bErr = 0xff;//����Ӧ�ó�ʼ��bErr������һ����Э���goto��Errȥ���֡����ȷ֡
	return true;
}

void PutReturnValue(TSgDaDt* pDaDt, BYTE bErr)
{
	BYTE bDaDtNum = 0;

	pDaDt[bDaDtNum].bErr = bErr;
	//pDaDt[bDaDtNum].fItemOK = false;

	bDaDtNum++;
}

WORD GetIdPnFromDiDa(BYTE bDA,BYTE bDAG,BYTE bDI1,BYTE bDI2,BYTE bDI3,BYTE bDI4,TSgIdPn *pIDPNGrp,WORD bIDPNGrpNum)
{	
	DWORD i,j,k;
	WORD pn[POINT_NUM];
	WORD pnnum;
	DWORD dwID = 0;

	memset((BYTE*)pn, 0, sizeof(pn));

	//���pn��
	if (!bDA || !bDAG)
	{
		pn[0] = 0;
		pnnum = 1;
	}
	else
	{
		if (0xff==bDA && 0xff==bDAG)		//20131224-3
		{
			for (i=1, pnnum=0; i < PN_NUM; i++)//20140321-4
				pn[pnnum++] = i;
		}
		else
		{
			WORD startpn = (bDAG<<3) - 7;

			for (i=0, pnnum=0; i < 8; i++,bDA>>=1)
			{
				if (bDA & 1)
					pn[pnnum++] = startpn + i;
			}
		}
	}

	//���ID��
	dwID = (DWORD)((DWORD)(bDI4 << 24) | (DWORD)(bDI3 << 16) | (DWORD)(bDI2 << 8) | (DWORD)bDI1);

	//���fn,pn�飬������pn��fn
	k = 0;
	for (i = 0; i < pnnum; i++)
	{

		if (k >= bIDPNGrpNum)
			return bIDPNGrpNum;

		pIDPNGrp[k].wPN = pn[i];
		pIDPNGrp[k].dwID = dwID;
		k++;

	}

	return k;
}

//inline WORD Rx_GetIDPN(BYTE *p)
WORD Rx_GetIDPN(BYTE *p)
{
	return GetIdPnFromDiDa(p[0], p[1], p[2], p[3], p[4], p[5], m_RxIdPnGrp,	sizeof(m_RxIdPnGrp)/sizeof(TSgIdPn));
}

//����:��ID,PNת����ͨ��Э���ʽ��6���ֽ�
void IdPnToBytes(DWORD dwID, WORD wPn, BYTE *pbBuf)
{
	if (!wPn)
	{
		pbBuf[0] = 0;
		pbBuf[1] = 0;
	}
	else
	{
		wPn--;
		pbBuf[0] = 1 << (wPn&7);
		pbBuf[1] = (wPn>>3) + 1;
	}

	pbBuf[2] = (BYTE) dwID & 0xff;
	pbBuf[3] = (BYTE) (dwID>>8) & 0xff;
	pbBuf[4] = (BYTE) (dwID>>16) & 0xff;
	pbBuf[5] = (BYTE) (dwID>>24) & 0xff;
}


bool ReExtCmd(struct TPro* pPro, BYTE bErrCode)
{
   	WORD wRxDataLen;  //�û��Զ��峤��
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;

	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DATA];
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DATA];

	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //���̱��

	
	wRxDataLen = 4;
	*pbTx++ = wRxDataLen & 0xff;
	*pbTx++ = (wRxDataLen >> 8) & 0xff;
	pbRx += 2; // ��������2�ֽ�

	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;	//��½ʶ����

	*pbTx++ = *pbRx++;  //��չ������

	*pbTx++ = bErrCode;

	pGbPro->wTxPtr += (WORD)(pbTx - &pGbPro->pbTxBuf[SG_LOC_DATA]);

	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;

	SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_USERDEF, pGbPro->wTxPtr);

	return true;
}

bool ReExtCmd1(struct TPro* pPro, BYTE *pbSftp, WORD len)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	WORD wRxDataLen;
	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DATA];
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DATA];

	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //���̱��

	wRxDataLen = 3 + len;  //�û��Զ��峤��
	*pbTx++ = wRxDataLen & 0xff;
	*pbTx++ = (wRxDataLen >> 8) & 0xff;
	pbRx += 2; // ��������2�ֽ�

	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;  //��½ʶ����

	*pbTx++ = *pbRx++;  //��չ������

	memcpy(pbTx, pbSftp, len);

	pbTx += len;

	pGbPro->wTxPtr += (WORD)(pbTx - &pGbPro->pbTxBuf[SG_LOC_DATA]);

	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;

	SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_USERDEF, pGbPro->wTxPtr);

	return true;
}

bool ReExtErr(struct TPro* pPro, BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	*pbTxBuf++ = bErrCode;
	pGbPro->wTxPtr += (WORD)(pbTxBuf - &pGbPro->pbTxBuf[SG_LOC_DATA]);
	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;
	SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_USERDEF, pGbPro->wTxPtr);//�˴����������ӿ�

	return true;
}

bool ReadDataEx(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE bISEQ = 0;
	int nRead;
	WORD wFrmLen;
	bool fFirstFrm = true;
	WORD wRxDataLen;
	WORD wPn;

	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DATA];
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DATA];


	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //���̱��

	wRxDataLen = ByteToWord(pbRx);
	*pbTx++ = *pbRx++;  //�Զ������ݳ���
	*pbTx++ = *pbRx++;	//�Զ������ݳ���

	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;	//��½ʶ����

	*pbTx++ = *pbRx++;  //��չ������

	//BYTE bPoint = *pbRx++;
	//*pbTx++ = bPoint;

	wPn = ByteToWord(pbRx);
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;


	while (pbRx < &pGbPro->pbRxBuf[SG_LOC_DATA+wRxDataLen+2])
	{
		BYTE bBank = *pbRx;
		WORD wID = ByteToWord(pbRx+1);

		*pbTx++ = *pbRx++;   //Bank
		*pbTx++ = *pbRx++;   //��������
		*pbTx++ = *pbRx++;
		
		nRead = ReadItemEx(bBank, wPn, wID, pbTx);
		if (nRead < 0)
		{
			ReExtErr(pPro,-nRead,pbRx,pbTx);
			return false;
		}
		
		 //����
		pbTx += nRead;
	}

	if (pbTx == &pGbPro->pbTxBuf[SG_LOC_DATA+9])    //������û��������
	{
		ReExtErr(pPro,ERR_ITEM,pbRx,pbTx);
		return false;
	}

	wRxDataLen = (WORD)(pbTx - &pGbPro->pbTxBuf[SG_LOC_DATA]) - 4;

	pGbPro->pbTxBuf[SG_LOC_DATA+2] = wRxDataLen & 0xff;
	pGbPro->pbTxBuf[SG_LOC_DATA+3] = (wRxDataLen >> 8) & 0xff;

	pGbPro->wTxPtr += (WORD)(pbTx - &pGbPro->pbTxBuf[SG_LOC_DATA]);

	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;

	SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_USERDEF, pGbPro->wTxPtr);//�˴����������ӿ�
	
	return true;
}

//����:д��չ����
bool WriteDataEx(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	int nWritten;
	WORD wRxDataLen;
	WORD wPn;
	BYTE bPerm;
	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DATA];
	BYTE* pbTx = &pGbPro->pbTxBuf[SG_LOC_DATA];

	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //���̱��

	wRxDataLen = ByteToWord(pbRx);
	*pbTx++ = *pbRx++;  //�Զ������ݳ���
	*pbTx++ = *pbRx++;	//�Զ������ݳ���


	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;	//��½ʶ����

	*pbTx++ = *pbRx++;  //��չ������

	//BYTE bPoint = *pbRx++;
	wPn = ByteToWord(pbRx);
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;


	bPerm = *pbRx++;
	pbRx += 3;   //��������

	//*pbTx++ = bPoint;

	while (pbRx < &pGbPro->pbRxBuf[SG_LOC_DATA+wRxDataLen+2])
	{
		BYTE bBank = *pbRx;
		WORD wID = ByteToWord(pbRx+1);

		*pbTx++ = *pbRx++;   //Bank
		*pbTx++ = *pbRx++;   //��������
		*pbTx++ = *pbRx++;

		nWritten = WriteItemEx(bBank, wPn, wID, pbRx);  //��չЭ�������룿
		if (nWritten < 0)
		{
			nWritten = -nWritten;
			if (nWritten == ERR_ITEM)   //��ϵͳ��֧�ֵ���������ݳ��Ȳ�������������������û������
			{
				*pbTx++ = (BYTE )nWritten;  //���������ý��
				break;
			}
			else   //ϵͳ�е������ֻ��Ȩ�޵�ûͨ��
			{
				*pbTx++ = (BYTE )(nWritten & 0xff);  //���������ý��
				nWritten = nWritten >> 8;
			}
		}
		else   //���óɹ�
		{
			if ( 0x2012== wID && BN1 == bBank && PN0 == wPn)//����GPRSģ���ͺ�	//20131225-2
			{
				WriteItemEx(BN10, PN0, 0xa1c5, pbRx);
			}
			if ( 0xa1c5== wID && BN10 == bBank && PN0 == wPn)
			{
				WriteItemEx(BN1, PN0, 0x2012, pbRx);
			}

			*pbTx++ = SG_ERR_OK;              //���������ý��
		}

		pbRx += nWritten;   //��������
	}

	wRxDataLen = (WORD)(pbTx - &pGbPro->pbTxBuf[SG_LOC_DATA]) - 4;

	pGbPro->pbTxBuf[SG_LOC_DATA+2] = wRxDataLen & 0xff;
	pGbPro->pbTxBuf[SG_LOC_DATA+3] = (wRxDataLen >> 8) & 0xff;

	pGbPro->wTxPtr += (WORD)(pbTx - &pGbPro->pbTxBuf[SG_LOC_DATA]);

	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;

	SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_USERDEF, pGbPro->wTxPtr);//�˴����������ӿ�

	TrigerSavePara();//20131212-1

	return true;
}

bool RunCmd(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	WORD wCmdLen;
	memset(m_szCmdLine, '\0',100);
	wCmdLen = ByteToWord(&pGbPro->pbRxBuf[FAP_DATA_EX+3]);
	DTRACE(DB_FAPROTO, ("CFaProto::RunCmd : len = %x\r\n",wCmdLen));

	memcpy(m_szCmdLine, &pGbPro->pbRxBuf[FAP_DATA_EX+5], wCmdLen);
	DTRACE(DB_FAPROTO, ("CFaProto::RunCmd : %s\r\n",m_szCmdLine));
#ifdef SYS_LINUX
	system(m_szCmdLine);
#endif
	return true;
}

/*int TrigerAdj(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	bool fRet = false;	

#ifdef SYS_LINUX
	if (TERMINAL_TYPE_CONCENTRATOR != GetTermType())//�ն������Ǽ�����
	{
		fRet = g_AcSample.TrigerAdj(&pbRxBuf[FAP_DATA_EX+3]);
		if (pbRxBuf[FAP_DATA_EX+3] == 0)
		{
			BYTE bBuf[128];
			memset(bBuf, 0, sizeof(bBuf));
			WriteItemEx(BN25, PN0, 0x5005, bBuf);  //0x5005 72 ATT7022У������
		}
	}
#endif

	if (fRet)
	{
		TrigerSavePara();
		return 0;
	}
	else
	{
		return -1;
	}
}*/

//0:��ȷ ERR_OK  04��ʧ�� ERR_ITEM
bool ClearConfigFile(struct TPro* pPro)
{
#if 0
	char szList[20][64];
	int iCnt = 0;
	bool bFail = false;
	//iCnt = ListDefault((char *)&szList[0], 20, ".dft");
	if(iCnt <= 0)
		return ReExtCmd(ERR_OK);
	char szFileName[64] = USER_PARA_PATH;	

	for (int i = 0; i<iCnt; i++)
	{
#ifdef SYS_VDK
		strcpy(szFileName, "/root/user/para/");
#else
		strcpy(szFileName, "/mnt/data/cfg/");
#endif
		if (unlink(szFileName) == -1)//ɾ�� 
		{
			DTRACE(DB_FAPROTO, ("CFaProto::ClearConfigFile : Fail To Delete Config File-->%s.\r\n",szList[i]));
			bFail = true;
		}
	}
	if(bFail == true)
		return ReExtCmd(ERR_ITEM);
	else
#endif
		return ReExtCmd(pPro, SG_ERR_OK);
}

bool UserDef(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE* pbRx = NULL;
	char  szPathNameEx[PATHNAME_LEN+1] = { 0 };
	WORD wLen = 0;
	WORD wNameLen = 0;
	TTime now;
    BYTE bRet = 0;
	//SG_LOC_DATA+0  //���̱���ƫ��  2�ֽ�
	//SG_LOC_DATA+2  //���̱������ݳ���ƫ�� 2�ֽ�

	if (pGbPro->wRxFrmLen==0x09 && pGbPro->pbRxBuf[SG_LOC_DATA+4]==0x21 && pGbPro->pbRxBuf[SG_LOC_DATA+5]==0x01)
	{	//���Ż�������,�л���GPRS/CDMAͨ��
		DTRACE(DB_FAPROTO, ("UserDef : rx sms wakeup.\r\n"));
		SetInfo(INFO_ACTIVE);
		return true;
	}

	if (pGbPro->pbRxBuf[SG_LOC_DATA+4]!=0x3d || pGbPro->pbRxBuf[SG_LOC_DATA+5]!=0x5a)
		return false;

	switch (pGbPro->pbRxBuf[SG_LOC_DATA+6])    //��չ������
	{
		//case 0x01:
		//return ReadDataEx();
		//case 0x02:
		//return WriteDataEx();
	case 0x03:					//��ʽ��Ӳ��
		if (!PswCheck(DI_HIGH_PERM|DI_PRG_PERM, &pGbPro->pbRxBuf[FAP_DATA_EX]))
			return ReExtCmd1(pPro, &pGbPro->pbRxBuf[FAP_DATA_EX], ERR_PERM);

		g_dwExtCmdClick = GetClick();
		g_dwExtCmdFlg = FLG_FORMAT_DISK;

		return ReExtCmd(pPro, SG_ERR_OK);
	case 0x04:					//Ĭ������
		if (!PswCheck(DI_HIGH_PERM, &pGbPro->pbRxBuf[FAP_DATA_EX]))
			return ReExtCmd(pPro, ERR_PERM);

		//			DlDefaultPra(true);

		g_dwExtCmdClick = GetClick();
		g_dwExtCmdFlg = FLG_DEFAULT_CFG;
		g_bDefaultCfgID = pGbPro->pbRxBuf[FAP_DATA_EX+3];

		//SetBeep();
		return ReExtCmd(pPro, SG_ERR_OK);
	case 0x05:
		m_fRxGoOnDB = true;   //���յ������������������
		return ReExtCmd(pPro, SG_ERR_OK);
	case 0x06:
		if (!PswCheck(DI_HIGH_PERM|DI_PRG_PERM, &pGbPro->pbRxBuf[FAP_DATA_EX]))
			return ReExtCmd(pPro, ERR_PERM);

		g_dwExtCmdClick = GetClick();
		g_dwExtCmdFlg = FLG_ENERGY_CLR;
		g_bEnergyClrPnt = pGbPro->pbRxBuf[FAP_DATA_EX+3];
		GetCurTime(&now);
		//g_TaskManager.SaveEventData(16, 1, now);//����ִ��ʱ���棬���ܱ���������
		// 		Sleep(2000);

		//EnergyClear(m_bRxBuf[FAP_DATA_EX]);
		return ReExtCmd(pPro, SG_ERR_OK);	
	case 0x07:			
		if (!PswCheck(DI_HIGH_PERM, &pGbPro->pbRxBuf[FAP_DATA_EX]))
			return ReExtCmd(pPro, ERR_PERM);

		g_dwExtCmdClick = GetClick();
		//    ֪ͨGPRS���������µ�IP�Ͷ˿�

		if (!(pGbPro->pbRxBuf[FAP_DATA_EX+3]==0 && pGbPro->pbRxBuf[FAP_DATA_EX+4]==0 &&
			pGbPro->pbRxBuf[FAP_DATA_EX+5]==0 && pGbPro->pbRxBuf[FAP_DATA_EX+6]==0))
		{			//Զ����������ķ�����IP��ַ

			BYTE bBuf2[32];
			if (!(pGbPro->pbRxBuf[FAP_DATA_EX+3]==pGbPro->pbRxBuf[FAP_DATA_EX+4]&&
				pGbPro->pbRxBuf[FAP_DATA_EX+3]==pGbPro->pbRxBuf[FAP_DATA_EX+5]&&
				pGbPro->pbRxBuf[FAP_DATA_EX+3]==pGbPro->pbRxBuf[FAP_DATA_EX+6]))
			{
				bBuf2[0] = pGbPro->pbRxBuf[FAP_DATA_EX+3+7];
				bBuf2[1] = pGbPro->pbRxBuf[FAP_DATA_EX+3+6];
				bBuf2[2] = pGbPro->pbRxBuf[FAP_DATA_EX+3+3];
				bBuf2[3] = pGbPro->pbRxBuf[FAP_DATA_EX+3+2];
				bBuf2[4] = pGbPro->pbRxBuf[FAP_DATA_EX+3+1];
				bBuf2[5] = pGbPro->pbRxBuf[FAP_DATA_EX+3];
				WriteItemEx(BN3, PN0, 0x3203, bBuf2);
				g_fDownLoad = true;
				SetInfo(INFO_REMOTEDOWN);
			}
			else
			{
				memset(bBuf2, 0,32);
				WriteItemEx(BN3, PN0, 0x3203, &pGbPro->pbRxBuf[FAP_DATA_EX+3]);
			}
		}
		return ReExtCmd(pPro, SG_ERR_OK);
	case 0x10://Ӧ�õط���������
		pbRx = &pGbPro->pbRxBuf[SG_LOC_DATA+2];
		if (!PswCheck(DI_HIGH_PERM, &pGbPro->pbRxBuf[FAP_DATA_EX]))
			return ReExtCmd(pPro, ERR_PERM);		
		DTRACE(DB_FAPROTO, ("CFaProto::UserDef : rx apply default para.\r\n"));
		wLen = ByteToWord(pbRx);
		pbRx += 2;
		wNameLen = wLen-SG_FAP_DATA_EX;  	//�ļ�������
		pbRx += SG_FAP_DATA_EX;
		memcpy(szPathNameEx, pbRx, wNameLen);

#ifndef SYS_WIN
		if (!ParaMgrParse())
			bRet = -1;
#endif

		//�˴��д��޸�
		//if (g_pmParaMgr.LoadPara(szPathNameEx) == 0)
		//{
		//	if (g_pmParaMgr.Parse())
		//	{
		//		ReExtCmd(SG_ERR_OK);//Ӧ��
		//		Sleep(100);
		//		FaClose();
		//		return true;
		//	}
		//}
		return ReExtCmd(pPro, ERR_ITEM);

	case 0x0c:
		SetInfo(INFO_CLR_DEMAND);
		return ReExtCmd(pPro, SG_ERR_OK);

	case 0x0E:

		//SftpHandleFrm(&pGbPro->pbRxBuf[FAP_DATA_EX+6], g_tSftpInfo.m_pbDataBuf);
		//ZJSftpDataEx(&pGbPro->pbRxBuf[FAP_DATA_EX+6],wRxDataLen, pbTxBuf);
		ZJHandleFrm(&pGbPro->pbRxBuf[LEN_OF_HEAD], &pGbPro->pbTxBuf[LEN_OF_HEAD]);
		return ReExtCmd1(pPro, &pGbPro->pbTxBuf[LEN_OF_HEAD+LEN_TO_DATA-LEN_OF_PWD], g_tSftpInfo.m_wTxLen);
//		return true;

	case 0x11:
		return ReadDataEx(pPro);
	case 0x12:
		return WriteDataEx(pPro);

	case 0x13:
		RunCmd(pPro);
		return ReExtCmd(pPro, SG_ERR_OK);

	case 0x16://ɾ�������ļ�
		if (!PswCheck(DI_HIGH_PERM, &pGbPro->pbRxBuf[FAP_DATA_EX]))
			return ReExtCmd(pPro, ERR_PERM);
		return ClearConfigFile(pPro);
	case 0x20://ģ����У׼
#ifndef SYS_WIN
#ifdef g_dcSample
		g_dcSample.TrigerAdj(pGbPro->pbRxBuf+0x12);
#endif
		return ReExtCmd(pPro, SG_ERR_OK);
#endif
		break;

	case 0x21:
		ZJDealTestCmd(&pGbPro->pbRxBuf[SG_LOC_DATA+7], 6, &pGbPro->pbTxBuf[LEN_OF_HEAD]);//������ָ��
		return ReExtCmd(pPro, SG_ERR_OK);

	default:
		return false;
	}

	return false;
}

/*
//����:��FN,PNת����ͨ��Э���ʽ��4���ֽ�
void FnPnToBytes(BYTE bFn, WORD wPn, BYTE *pbBuf)
{
	if (!wPn)
	{
		pbBuf[0] = 0;
		pbBuf[1] = 0;
	}
	else
	{
			#ifdef PRO_698
		wPn--;
		pbBuf[0] = 1 << (wPn&7);
		pbBuf[1] = (wPn>>3) + 1;
			#else
		wPn--;
		pbBuf[0] = 1 << ((BYTE)wPn&7);
		pbBuf[1] = 1 << ((BYTE)wPn>>3);
			#endif
	}

	if (!bFn)
	{
		pbBuf[2] = 0;
		pbBuf[3] = 0;
	}
	else
	{
		bFn--;
		pbBuf[2] = 1 << (bFn&7);
		pbBuf[3] = bFn >> 3;
	}
}

//����:��FN,PNת����ͨ��Э���ʽ��4���ֽ�
void IdPnToBytes(DWORD dwId, WORD wPn, BYTE *pbBuf)
{
	if (!wPn)
	{
		pbBuf[0] = 0;
		pbBuf[1] = 0;
	}
	else
	{
#ifdef PRO_698
		wPn--;
		pbBuf[0] = 1 << (wPn&7);
		pbBuf[1] = (wPn>>3) + 1;
#else
		wPn--;
		pbBuf[0] = 1 << ((BYTE)wPn&7);
		pbBuf[1] = 1 << ((BYTE)wPn>>3);
#endif
	}

	pbBuf[2] = dwId & 0xff;
	pbBuf[3] = (dwId>>8) & 0xff;
	pbBuf[4] = (dwId>>16) & 0xff;
	pbBuf[5] = (dwId>>24) & 0xff;
}*/



BYTE ReadEC1()
{ 
	BYTE bEc1;
	ReadItemEx(BN0, PN0, 0x1060, &bEc1);
	return bEc1;
}

BYTE ReadEC2()
{ 
	BYTE bEc2;
	ReadItemEx(BN0, PN0, 0x1061, &bEc2);
	return bEc2;
}

WORD ReadEC1Num()
{ 
	WORD wEc1Num;
	ReadItemEx(BN0, PN0, 0x1400, (BYTE*)&wEc1Num);
	return wEc1Num;
}

WORD ReadEC2Num()
{ 
	WORD wEc2Num;
	ReadItemEx(BN0, PN0, 0x1401, (BYTE*)&wEc2Num);
	return wEc2Num;
}

WORD GetValidEcStartEnd(BYTE bEc, WORD wEcNum, BYTE *pEcStart, BYTE *pEcEnd)
{
	BYTE bStart = *pEcStart;
	BYTE bEnd = *pEcEnd;

	if (!wEcNum)//�յ�
		return 0;

	if (wEcNum >= 256)//����
		return (bStart<=bEnd)? bEnd-bStart: 256+bEnd-bStart;
	
	if (bStart < bEc && bEnd < bEc && bEnd >= bStart)//������������Ч����
		return (bStart<=bEnd)? bEnd-bStart: 256+bEnd-bStart;

	if (bStart < bEc && bEnd >= bEc)// && bEnd >= bStart)
	{
		*pEcEnd = bEc;
		bEnd = *pEcEnd;
		return (bStart<=bEnd)? bEnd-bStart: 256+bEnd-bStart;
	}

	if (bStart >= bEc && bEnd >= bEc && bEnd >= bStart)
		return 0;

	if (bStart < bEc && bEnd < bEc && bEnd < bStart)
		return (bStart<=bEnd)? bEnd-bStart: 256+bEnd-bStart;

	if (bStart >= bEc && bEnd < bEc)// && bEnd < bStart)
	{
		*pEcStart = 0;
		return bEnd;
	}

	if (bStart >= bEc && bEnd >= bEc && bEnd < bStart)
	{
		*pEcStart = 0;
		*pEcEnd = bEc;
		return *pEcEnd;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////
//GbProʵ��

//������ͨ�Žӿڳ�ʼ��
void SgInit(struct TPro* pPro, BYTE* pbRxBuf, BYTE* pbTxBuf, bool fLocal)
{
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;

	memset(pSgPro, 0, sizeof(TSgPro));
	  
	//����ṹ��Ա��ʼ��
	ProInit(pPro);

	//����������ʼ��
	pSgPro->pbRxBuf = pbRxBuf;
	pSgPro->pbTxBuf = pbTxBuf;

	pSgPro->dwAddr1 = 0x00332211;
	pSgPro->dwAddr2 = 0x00665544;
	//ReadItemEx(BN10, PN0, 0xa040, (BYTE*)&pSgPro->dwAddr1);
	//ReadItemEx(BN10, PN0, 0xa041, (BYTE*)&pSgPro->dwAddr2);
	ReadItemEx(BN0, PN0, 0x8020, (BYTE*)&pSgPro->dwAddr1);
	ReadItemEx(BN0, PN0, 0x8021, (BYTE*)&pSgPro->dwAddr2);

	pSgPro->fLocal = fLocal;
	pSgPro->fAutoSend = false;

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pPro->pfnRcvBlock = SgRcvBlock;	//��֡
	pPro->pfnHandleFrm = SgHandleFrm;	//����֡

	pPro->pfnLogin = SgLogin;			//��½
	pPro->pfnLogoff = SgLogoff;			//��½�˳�
	pPro->pfnBeat = SgBeat;			//����
	pPro->pfnAutoSend = GbAutoSend;		//��������
	pPro->pfnIsNeedAutoSend = SgIsNeedAutoSend;	//�Ƿ���Ҫ��������
	pPro->pfnLoadUnrstPara = SgLoadUnrstPara;	//װ�طǸ�λ����
	pPro->pfnDoProRelated = SgDoProRelated;	//��һЩЭ����صķǱ�׼������

	//����������
	pSgPro->wRxStep = 0;
	pSgPro->wRxPtr = 0;
	pSgPro->wRxCnt = 0;
	pSgPro->wRxFrmLen = 0;
	pSgPro->wRxtry = 0;
	pSgPro->bRxFrmFlg = 0;	//���յ�֡��־λ

	pSgPro->dwDayFlux = 0;
	pSgPro->dwMonFlux = 0;

	SgInitECVal(pSgPro);
}

void SgInitECVal(TSgPro* pSgPro)
{
	//pGbPro->bEC1 = ReadEC1();
	ReadItemEx(BN0, PN0, 0x345f, &pSgPro->bEC1);
}

//��֡����
//����:����һ�����ݿ�,�ж��Ƿ���յ�һ��������ͨ��֡
//����:�����Ѿ�ɨ������ֽ���,����յ�һ��������ͨ��֡�򷵻�����,���򷵻ظ���
int SgRcvBlock(struct TPro* pPro, BYTE* pbBlock, int nLen)
{
	BYTE b;
	int i,j,k;
	WORD num;

	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	struct TProIf* pIf = pPro->pIf;

	if (nLen>0 && !pGbPro->fRptState)
		pGbPro->dwRxClick = GetClick();

	if (pGbPro->wRxStep!=0 && pGbPro->dwFrmStartClick!=0 &&
		GetClick()-pGbPro->dwFrmStartClick>40)	//̨���ط���60S,����60s������Ч��
	{
	  	pGbPro->wRxStep = 0;
		pGbPro->dwFrmStartClick = 0;
	}

	for (i = 0; i < nLen; i++)
	{
        b = *pbBlock++;

        switch (pGbPro->wRxStep) 
		{
        case 0:   //0x68
			if (b == 0x68)
			{
				//memset(pGbPro->pbRxBuf, 0, PRO_FRM_SIZE);
				pGbPro->pbRxBuf[0] = 0x68;
				pGbPro->wRxPtr = 1;
				pGbPro->wRxCnt = 5;
				pGbPro->wRxStep = 1;
				pGbPro->dwFrmStartClick = GetClick();
			}
			break;

		case 1:    //��ַ��ǰ������
			pGbPro->pbRxBuf[pGbPro->wRxPtr++] = b;
			pGbPro->wRxCnt--;
			if (pGbPro->wRxCnt == 0)   //�����꣬����У��
			{
				pGbPro->wRxFrmLen = ((WORD)pGbPro->pbRxBuf[2]<<8) | pGbPro->pbRxBuf[1];
				pGbPro->wRxFrmLen += 8;

				if (pGbPro->pbRxBuf[5] == 0x68 && 
					pGbPro->pbRxBuf[1] == pGbPro->pbRxBuf[3] && pGbPro->pbRxBuf[2] == pGbPro->pbRxBuf[4] &&
					pGbPro->wRxFrmLen < GB_FRM_SIZE )
				{
					pGbPro->wRxCnt = pGbPro->wRxFrmLen - 6;
					pGbPro->wRxStep = 2;
				}
				else
				{
					if (pGbPro->pbRxBuf[1]==0x68 && pGbPro->pbRxBuf[2]==pGbPro->pbRxBuf[4] && pGbPro->pbRxBuf[3]==pGbPro->pbRxBuf[5] &&
						((((WORD)pGbPro->pbRxBuf[3]<<8)|pGbPro->pbRxBuf[2])>>2)+8 < GB_FRM_SIZE )  
						j = 1;
					else if (pGbPro->pbRxBuf[2]==0x68 && pGbPro->pbRxBuf[3]==pGbPro->pbRxBuf[5] &&
						((((WORD)pGbPro->pbRxBuf[4]<<8)|pGbPro->pbRxBuf[3])>>2)+8 < GB_FRM_SIZE )  
						j = 2;
					else if (pGbPro->pbRxBuf[3] == 0x68 &&
						((((WORD)pGbPro->pbRxBuf[5]<<8)|pGbPro->pbRxBuf[4])>>2)+8 < GB_FRM_SIZE )  
						j = 3;
					else if (pGbPro->pbRxBuf[4] == 0x68)
						j = 4;
					else if (pGbPro->pbRxBuf[5] == 0x68)
						j = 5;
					else
						j = 6;

					num = (WORD)(6 - j);
					for (k=0; k<num; k++)
						pGbPro->pbRxBuf[k] = pGbPro->pbRxBuf[j+k];
					pGbPro->wRxPtr = num;
					pGbPro->wRxStep = (num)?1:0;
					pGbPro->wRxCnt = 6 - num;
				}
			}
			break;

		case 2:     //��ַ + ���� + ������ + ������
			pGbPro->pbRxBuf[pGbPro->wRxPtr++] = b;
			pGbPro->wRxCnt--;
			if (pGbPro->wRxCnt == 0)   //�����꣬����У��
			{
				pGbPro->wRxStep = 0;
				if (pGbPro->pbRxBuf[pGbPro->wRxPtr-1] == 0x16)
				{	
					if ( SgVeryRxFrm(pGbPro) )
					{
                        DTRACE(DB_FAPROTO, ("Rx valid frm at click = %d.\r\n", GetClick()));
						pGbPro->wRxtry=0;
						pGbPro->dwFrmStartClick = 0;
						return i+1;
					}
				}
			}
			break;

		default:
			pGbPro->wRxStep = 0;
			break;
		} //switch (wRxStep) 
	}
	
	pGbPro->wRxtry++;
	if (pGbPro->wRxtry > 50) //��֡���ն�ν�����֡
	{
		pGbPro->wRxtry = 0;
		pGbPro->wRxStep = 0;//������Ч֡
	}
	//DTRACE(DB_FAPROTO, ("GbRcvBlock:err ,nLen=%d,wRxCnt=%d,wRxStep=%d.\r\n",nLen,pGbPro->wRxCnt,pGbPro->wRxStep));

	return -nLen;
}

bool SgVeryRxFrm(TSgPro* pSgPro)
{
	DWORD dwRxA1A2, dwRxB1B2;
	//BYTE bRxAFN;
	WORD i, wDataLen;
	WORD wGrpAddr[8];
	bool fGrpValid = false;

	pSgPro->dwAddr1 = 0x332211;	//����������A1
	pSgPro->dwAddr2 = 0x665544;	//�ն˵�ַA2
	//ReadItemEx(BN10, PN0, 0xa040, (BYTE*)&pGbPro->dwAddr1);
	//ReadItemEx(BN10, PN0, 0xa041, (BYTE*)&pGbPro->dwAddr2);	

	ReadItemEx(BN0, PN0, 0x8020, (BYTE*)&pSgPro->dwAddr1);
	ReadItemEx(BN0, PN0, 0x8021, (BYTE*)&pSgPro->dwAddr2);


	wDataLen = ByteToWord(pSgPro->pbRxBuf + SG_LOC_LLEN1);

	dwRxA1A2 = ByteToDWORD(pSgPro->pbRxBuf + SG_LOC_ADDA1, 3);
	dwRxB1B2 = ByteToDWORD(pSgPro->pbRxBuf + SG_LOC_ADDB1, 3);

	//bRxAFN = pGbPro->pbRxBuf[SG_LOC_AFN];

	//����У���
	if (CheckSum(pSgPro->pbRxBuf+SG_LOC_CONTROL, wDataLen) != pSgPro->pbRxBuf[SG_LOC_CONTROL+wDataLen])
	{
		DTRACE(DB_FAPROTO, ("GbVeryRxFrm: CheckSum failure\n"));
		return false; 
	}

	/*//�����޳�״̬
	BYTE buf[64];
	buf[0] = 0;
	GBReadItemEx(SG_DATACLASS1, 5, 0, buf, &num);	//�ն˿�������״̬
	if (buf[0]&2) //�޳�Ͷ��Ļ������ܹ㲥���鲥�����ʱ���⣩
	{
		if ((pGbPro->pbRxBuf[SG_LOC_ADDB3] & 1) || //���ַ
			(wRxB1B2==GBADDR_BROADCAST) )	//�㲥
		{	//buf=05 seq 00 00 40 03��ʾ��ʱ
			if (!(bRxAFN==SG_AFUN_CONTROLCMD &&
				pGbPro->pbRxBuf[SG_LOC_DA1]==0 && pGbPro->pbRxBuf[SG_LOC_DA2]==0 &&
				pGbPro->pbRxBuf[SG_LOC_DT1]==0x40 && pGbPro->pbRxBuf[SG_LOC_DT2]==0x03)) //F31��ʱ����
			{
				DTRACE(DB_FAPROTO, ("GbVeryRxFrm: Forbit involve.Broad commamd,Is not verify Time command.\r\n"));
				return false;
			}
		}
	}*/

	//if ((pGbPro->pbRxBuf[SG_LOC_ADDB3]&1) && wRxB1B2!=SGADDR_BROADCAST)//���ַ
	//{
		//if (ReadItemEx(BN0, PN0, 0x006f, (BYTE*)wGrpAddr) <= 0)
		//	memset(wGrpAddr, 0, sizeof(wGrpAddr));

		//for (i=0; i<8; i++)
		//{
		//	if (wGrpAddr[i] && wGrpAddr[i] == wRxB1B2)
		//	{
		//		fGrpValid = true;
		//		break;
		//	}
		//}

		//if (fGrpValid == false)
		//{
		//	DTRACE(DB_FAPROTO, ("GbVeryRxFrm: grp addr mismatch\n"));
		//	return false;
		//}
	//}
	 if (dwRxB1B2 == SGADDR_BROADCAST)	//����ַ+�㲥��ַ
	{
		/*if ((pGbPro->pbRxBuf[SG_LOC_ADDB3]&1) != 0x1) //�Ǹ�A3��D0λ��1����Ϊ�ǹ㲥���̨�ӷ�����û��1��
		{
			DTRACE(DB_FAPROTO, ("CFaProto::VeryRxFrm: GBADDR_BROADCAST ERR.\n"));
			return false; 
		}*/
	}
	else  //����ַ+�㲥��ַ
	{
		if (pSgPro->fLocal == false) //�Ǳ��ؿ��жϵ�ַ
		{
			if (dwRxB1B2==SGADDR_INVALID ||dwRxB1B2!=pSgPro->dwAddr2 || dwRxA1A2!=pSgPro->dwAddr1)
			{
				DTRACE(DB_FAPROTO, ("GbVeryRxFrm: remote port address wrong, Do forward\n"));
				//DoForward();
				return false; 
			}
		}
		else //�͵�ά���˿��жϵ�ַ���Ͽ�Ĭ�ϲ���
		{
			if ((dwRxA1A2!=0x332211 && dwRxA1A2!=SGADDR_INVALID) || 
				(dwRxB1B2!=0x665544 && dwRxB1B2!=SGADDR_BROADCAST))
			{
				if (/*m_GbCommInfo.wRxB1B2 == GBADDR_INVALID ||*/
					dwRxB1B2!=pSgPro->dwAddr2 || dwRxA1A2!=pSgPro->dwAddr1)
				{
					DTRACE(DB_FAPROTO, ("GbVeryRxFrm: local address wrong, Do forward\n"));
					//���Ӹ��жϣ��ǲ������նˣ���Ҫת������.
					//DoForward();
					return false; 
				}
			}
		}
	}

	if ((pSgPro->pbRxBuf[SG_LOC_CONTROL] & SG_CTL_DIR) != 0)	//�ն˷��������б���
	{
		DTRACE(DB_FAPROTO, ("GbVeryRxFrm: Direction Wrong\n"));
		return false; 
	}

	return true;
}

//����:֡����
//����:�����֡����ɹ��򷵻�true,���򷵻�ʧ��
bool SgHandleFrm(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	int iRet;
	TSgDaDt tDaDt[1];
	bool fOk;

	DTRACE(DB_FAPROTO, ("GbHandleFrm: RxProcess.Receive a message.\r\n"));

	//TVPУ��
	if ( !SgCheckTVP(pGbPro) )
	{
		DTRACE(DB_FAPROTO, ("GbProHandleFrm:RxProcess.CheckTVP failure.\r\n"));
		return false;
	}

	//process
	SgPrintCmd(pGbPro->pbRxBuf[SG_LOC_AFN], false);

	switch (pGbPro->pbRxBuf[SG_LOC_AFN])
	{
	case SG_AFUN_CONFIRM:	
		fOk = (SgRxConfirm(pGbPro) > 0) ? true : false;
		if (fOk)
			pGbPro->bRxFrmFlg |= FRM_CONFIRM;	//���յ�֡��־λ

		return fOk;

    case SG_AFUN_CHECKLINK:
		return SgAnsConfirm(pPro, tDaDt, 0xff);
	case SG_AFUN_SETPARA:
		return SgRxSetPara(pPro);		
		break;		
	case SG_AFUN_ASKPARA:
		return SgRxAskPara(pPro);
	case SG_AFUN_ASKCLASS1:
		return SgRxCallClass1(pPro);		
	case SG_AFUN_ASKCLASS2:
		return SgRxCallClass2(pPro);		
	case SG_AFUN_ASKCLASS3:
		return SgRxCallClass3(pPro, SG_AFUN_ASKCLASS3);
	case SG_AFUN_ASKALARM:
		return SgRxCallClass3(pPro, SG_AFUN_ASKALARM);		
	case SG_AFUN_MTRFWD:
		iRet = SgRxMtrFwdCmd(pPro);
		break;
	case SG_AFUN_ASKTASK:
		iRet = ReadTask(pPro);
		break;
    case SG_AFUN_TRANSFILE:
       return AnsTransFile(pPro, SgRxCmd_TransFile(pPro));
        //break;
	case SG_AFUN_USERDEF:				//�û��Զ�������
		iRet = UserDefData(pPro);
		break;

	default:
		return false;
	}

	if (iRet < 0)
		return SgAnsConfirm(pPro, tDaDt, 0);
	return true;	//��֡����ʧ��
}

int SgRxConfirm(TSgPro* pGbPro)
{	
	DWORD dwId;
	if ( !GetOneId(pGbPro->pbRxBuf+SG_LOC_DA1, &dwId) )
		return -1;

	return 1000;
}

int SgRxReset(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	TSgDaDt tDaDt[1];
	DWORD bFn;
	int	iRet;

	if (!SgGetRxSeq(pGbPro))
		return -1;

	if ((iRet=SgVeryPsw(pPro)) <= 0)
		return iRet;

	memset(tDaDt, 0, sizeof(tDaDt));
	if ( !GetOneId(pGbPro->pbRxBuf+SG_LOC_DA1, &bFn) )
	{
		SgAnsConfirm(pPro, tDaDt, 0);
		iRet = -1;
	}
	else
	{
		switch( bFn )
		{
		case 1://Ӳ����ʼ��
			iRet = SgAnsConfirm(pPro, tDaDt, 1);
			Sleep(300);
			SetInfo(INFO_APP_RST); 
			break;
		case 2://��������ʼ��
			iRet = SgAnsConfirm(pPro, tDaDt, 1);
			Sleep(300);
			SetInfo(INFO_RST_DATA);  
			break;
#ifdef PRO_698
		case 3://������ȫ����������ʼ��
			iRet = SgAnsConfirm(pPro, tDaDt, 1);
			Sleep(300);
			SetInfo(INFO_RST_ALLPARA);  
			break;		
#else
		case 3://������ȫ����������ʼ��
#endif
		case 4:
			iRet = SgAnsConfirm(pPro, tDaDt, 1);
			Sleep(300);
			SetInfo(INFO_RST_PARA);  
			break;
		default: 
			iRet = -1;
			break;
		}		
	}
	
	return iRet;
}

int WritePointCtrlPara(WORD wPn,DWORD dwID,BYTE *pbRx)
{
	BYTE bControlType[] = {0x1A,0x1B,0x3A,0x3B};//��բ,��բ,����,������
	BYTE bMtrAddr[6], bDstAddr[6], bOptPwd[4], bOptCode[4], bCtrlValidTm, bType, bRelayType;
	TTime tmTime;
	BYTE bData[30];
	TYkCtrl YkCtrl;
	DWORD dwTime = 0;
	int iRet = 0;
	WORD wPn1 = 0;

	dwTime = GetCurSec();
	//�Ե��ܱ�ң����բ,��բ,����,������
	//�Ȳ��жϵ���Ƿ����բ �ſ�������
	if(dwID == 0xe0001101 || dwID == 0xe0001100)
	{
		BYTE bFwdOpen;
		ReadItemEx(BN0,wPn,0x8907,&bFwdOpen);
		if(0 == bFwdOpen)
		{
			iRet = 0;
			return iRet;
		}
		//���ܱ��ַ
		memcpy(bMtrAddr,pbRx,6);
		//���ܱ��������
		memcpy(bOptPwd,pbRx+6,4);
		//�����ߴ���
		memcpy(bOptCode,pbRx+10,4);
		//������Чʱ��,��λ����
		bCtrlValidTm = BcdToByte(*(pbRx+14));
		dwTime += bCtrlValidTm*60;
		SecondsToTime(dwTime, &tmTime);
		iRet = 15;
		if(dwID == 0xe0001101) //�Ե��ܱ�ң�غ�բ
		{
			//��բ����
			bType = BcdToByte(*(pbRx+15)) & 0x01;
			iRet += 1;
		}
		//����Ҫɾ���Ĳ������ַ
		ReadItemEx(BN0,wPn,0x8902,bDstAddr);

		if(IsPnValid(wPn) && memcmp(bMtrAddr,bDstAddr,6) == 0)
		{
			//���ݲ������Լ���� ��ң����բ,��բ,����,����������
			bRelayType = bControlType[dwID&SG_DWORD_GET_LOW_BYTE];

			if((bRelayType ==0x1A) || (bRelayType ==0x1B))  //ֻ������բ�Ž�ȥ
			{
				if(bRelayType ==0x1B) //��բ�������բ0 ֱ�Ӻ�բ1
					bRelayType += bType;
				YkCtrl.wYkPn = wPn;
				memcpy(YkCtrl.bYkMtrAddr, bMtrAddr, 6);
				memcpy(YkCtrl.bYKOptCode, bOptCode, 4);
				memcpy(YkCtrl.bYkOptPwd, bOptPwd, 4);
				YkCtrl.bYKValDly = bCtrlValidTm;
				YkCtrl.bYKCtrlType = bRelayType;

				//������բ�Ĳ����㼰����д��Flash
			iRet = ExFlashWritePnCtrlPara(wPn, &YkCtrl, sizeof(YkCtrl));
				//!!!!!!�ȴ��ӿ�: SetCctAlrPnMask(0xE200003C, 0x8001, wPn, false);//��澯��ɱ�־
			}
		}
	}
	else if (dwID == 0xe0001104)	//ɾ������բ����
	{		
		wPn1 = ByteToWord(pbRx);
		pbRx += 2;

		ReadItemEx(BN0, wPn1, 0x8902, bDstAddr);

		//���ܱ��ַ
		memcpy(bMtrAddr, pbRx, 6);
		pbRx += 6;
		
		iRet = 8;
		if (IsPnValid(wPn1) && memcmp(bDstAddr, bMtrAddr, 6) == 0)	//if (IsPnValid(wPn) && wPn1 == wPn && memcmp(bDstAddr, bMtrAddr, 6) == 0)
		{
			memset(&YkCtrl, 0, sizeof(YkCtrl));
			ExFlashWritePnCtrlPara(wPn1, &YkCtrl, sizeof(YkCtrl));

			memset(bData, 0, sizeof(bData));
			WriteItemEx(BN0, wPn1, 0x891f,bData);
			WriteItemEx(BN0, wPn1, 0x890f,bData);
		}
		else
		{
			iRet = 0;
		}
	}

	return iRet;
}


int WritePointAdjTimePara(WORD wPn,DWORD dwID,BYTE *pbRx)
{
	
	int iRet = 0;
	BYTE bMtrAddr[6];
	BYTE bOptPwd[4];
	BYTE bOptCode[4];
	BYTE bWriteID[4];
	BYTE bCtrlValidTm;
	TTime tmTime;
	DWORD dwTime = 0;
	BYTE bType;
	WORD wDelPn;
	BYTE bDstAddr[6];
	BYTE bTxFrm[50];
	BYTE bRxFrm[50];
	BYTE bData[50]; 
	BYTE bCmd = SCHED_07_CMD_ALARM;
	BYTE bDataLen = 6;
	TMtrFwdMsg tFwdMsg;
	BYTE bPort;
	BYTE bFunc = 0;
	DWORD dwLen = 0;
	BYTE bProId;
	BYTE bMtrFwdBuf[4];
	WORD wTmCheck;
	BYTE bRetLen = 0;
	int i = 0;
	BYTE bPos;
	
	WORD wDstPn = 0;
	if(*pbRx==0xFF && *(pbRx+1)==0xFF)
	{
		wDstPn = 0xFFFF;
	}
	else
		wDstPn = BcdToDWORD(pbRx,2);

	GetCurTime(&tmTime);
	dwTime = TimeToSeconds(&tmTime);
	
	if ((pbRx[3]&0x80) == 0x00)
	{
		pbRx[3] &= 0x7f;
		wTmCheck = BcdToDWORD(pbRx+2,2);
		SecondsToTime((dwTime+wTmCheck), &tmTime);//У��
	}
	else
	{
		pbRx[3] &= 0x7f;
		wTmCheck = BcdToDWORD(pbRx+2,2);
		SecondsToTime((dwTime-wTmCheck), &tmTime);//У��
	}
	
	if(wDstPn == 0xFFFF) //�����в�����㲥��ʱ
	{
		iRet = 4;

		//��㲥��ʱ����
		bDataLen = 6;
		bCmd = 0x08;
		memset(bMtrAddr,0x99,6);
		
		bData[0] = ByteToBcd(tmTime.nSecond);
		bData[1] = ByteToBcd(tmTime.nMinute);
		bData[2] = ByteToBcd(tmTime.nHour);
		bData[3] = ByteToBcd(tmTime.nDay);
		bData[4] = ByteToBcd(tmTime.nMonth);
		bData[5] = ByteToBcd(tmTime.nYear%100) ;
		
		//ȡ��·485�ڹ���
		for(bPort = 1; bPort <= 2; bPort++)//�߼��˿�
		{
			dwLen = Make645Frm(bTxFrm,bMtrAddr,bCmd,bData,bDataLen);
			bFunc = GetMeterPortFunc(bPort);
			if(bFunc == COM_FUNC_METER)
			{
				//�����
				memset(&tFwdMsg, 0, sizeof(tFwdMsg));
				//tFwdMsg.pbTxBuf = bTxFrm;
				tFwdMsg.pbRxBuf = bRxFrm;
				tFwdMsg.wRxBufSize = sizeof(bRxFrm);
				tFwdMsg.wTxLen = dwLen;

				tFwdMsg.CommPara.wPort = bPort-1;//Э��˿�
				tFwdMsg.CommPara.dwBaudRate = ValToBaudrate(8); //������2400
				tFwdMsg.CommPara.bParity = ValToParity(1);      //ż����
				tFwdMsg.CommPara.bByteSize = ValToByteSize(8);	//����λ8
				tFwdMsg.CommPara.bStopBits = ValToStopBits(0);  //ֹͣλ0

				tFwdMsg.wFrmTimeout = 2*1000;
				tFwdMsg.wByteTimeout = 2*10;

				tFwdMsg.dwClick = GetClick();
				
				DTRACE(DB_FAPROTO, ("CFaProto::WritePointAdjTimePara: wPort=%d, dwBaudRate=%ld, bParity=%d, bByteSize=%d, bStopBits=%d, wFrmTimeout=%d\n",
					tFwdMsg.CommPara.wPort, tFwdMsg.CommPara.dwBaudRate, 
					tFwdMsg.CommPara.bParity, tFwdMsg.CommPara.bByteSize, 
					tFwdMsg.CommPara.bStopBits, tFwdMsg.wFrmTimeout));
				TraceBuf(DB_FAPROTO, "Mtr fwd frm->", bTxFrm, tFwdMsg.wTxLen);
				
				ComTransmit645Cmd(tFwdMsg.CommPara, bTxFrm, tFwdMsg.wTxLen, tFwdMsg.pbRxBuf, &bRetLen, 0);
				tFwdMsg.wRxLen = bRetLen;
				//MtrDoFwd(&tFwdMsg);
				
				
				tFwdMsg.CommPara.dwBaudRate = ValToBaudrate(4); //������1200
				tFwdMsg.dwClick = GetClick();

				DTRACE(DB_FAPROTO, ("CFaProto::WritePointAdjTimePara: wPort=%d, dwBaudRate=%ld, bParity=%d, bByteSize=%d, bStopBits=%d, wFrmTimeout=%d\n",
					tFwdMsg.CommPara.wPort, tFwdMsg.CommPara.dwBaudRate, 
					tFwdMsg.CommPara.bParity, tFwdMsg.CommPara.bByteSize, 
					tFwdMsg.CommPara.bStopBits, tFwdMsg.wFrmTimeout));
				TraceBuf(DB_FAPROTO, "Mtr fwd frm->", bTxFrm, tFwdMsg.wTxLen);

				bRetLen = 0;
				dwLen = Make645Frm(bTxFrm,bMtrAddr,bCmd,bData,bDataLen);
				ComTransmit645Cmd(tFwdMsg.CommPara, bTxFrm, tFwdMsg.wTxLen, tFwdMsg.pbRxBuf, &bRetLen, 0);
				tFwdMsg.wRxLen = bRetLen;
				//MtrDoFwd(&tFwdMsg);
			}
		}

#ifdef EN_CCT
		bRetLen = 0;
		dwLen = Make645Frm(bTxFrm,bMtrAddr,bCmd,bData,bDataLen);
		tFwdMsg.CommPara.wPort = PORT_CCT_PLC;
		ComTransmit645Cmd(tFwdMsg.CommPara, bTxFrm, dwLen, bRxFrm, &bRetLen, 0);
#endif
	}
	else
	{
		iRet = 4;
		//�鵥���ʱ����
		if (PN0 == wDstPn || !IsPnValid(wDstPn))//��Ч�����㲻��ʱ
		{
			return 0;
		}
		ReadItemEx(BN0,wDstPn,0x8903,&bProId);
		for (i = 0; i<2; i++)
		{
			if (0x01 == bProId)//07Э��
			{
				bCmd = 0x14;
				//���ܱ��ַ
				ReadItemEx(BN0,wDstPn,0x8902,bMtrAddr);
				//���ܱ��������
				ReadItemEx(BN0,wDstPn,0x8930,bData+4);
				//�����ߴ���
				ReadItemEx(BN0,wDstPn,0x8931,bData+8);
				if (0 == i)//ʱ��
				{
					bDataLen = 12+3;
					bWriteID[0] = 0x02;
					bWriteID[1] = 0x01;
					bWriteID[2] = 0x00;
					bWriteID[3] = 0x04;
					memcpy(bData, bWriteID, 4);
					bData[12] = ByteToBcd(tmTime.nSecond);
					bData[13] = ByteToBcd(tmTime.nMinute);
					bData[14] = ByteToBcd(tmTime.nHour);
				}
				else//����
				{
					bDataLen = 12+4;
					bWriteID[0] = 0x01;
					bWriteID[1] = 0x01;
					bWriteID[2] = 0x00;
					bWriteID[3] = 0x04;
					memcpy(bData, bWriteID, 4);
					bData[12] = ByteToBcd(tmTime.nWeek);
					bData[13] = ByteToBcd(tmTime.nDay);
					bData[14] = ByteToBcd(tmTime.nMonth);
					bData[15] = ByteToBcd(tmTime.nYear%100);
				}
				
			}
			else
			{
				bCmd = 0x04;
				//���ܱ��ַ
				ReadItemEx(BN0,wDstPn,0x8902,bMtrAddr);
				if (0 == i)//ʱ��
				{
					bDataLen = 2+3;
					bWriteID[0] = 0x11;
					bWriteID[1] = 0xc0;
					memcpy(bData, bWriteID, 2);
					bData[2] = ByteToBcd(tmTime.nSecond);
					bData[3] = ByteToBcd(tmTime.nMinute);
					bData[4] = ByteToBcd(tmTime.nHour);
				}
				else//����
				{
					bDataLen = 2+4;
					bWriteID[0] = 0x10;
					bWriteID[1] = 0xc0;
					memcpy(bData, bWriteID, 2);
					bData[2] = ByteToBcd(tmTime.nWeek);
					bData[3] = ByteToBcd(tmTime.nDay);
					bData[4] = ByteToBcd(tmTime.nMonth);
					bData[5] = ByteToBcd(tmTime.nYear%100);
				}
			}
			dwLen = Make645Frm(bTxFrm,bMtrAddr,bCmd,bData,bDataLen);
			
			ReadItemEx(BN0,wDstPn,0x890a,&bPort);
			if(bPort == 0x1f || bPort == 0x20)
			{
				//�ز��ڷ��Ͷ�ʱ����
#ifdef EN_CCT
				iRet = DirectTransmit645Cmd(wDstPn, PORT_CCT_PLC, bTxFrm, dwLen, bRxFrm, &bRetLen, 0);
#endif
				return iRet;
			}
			bFunc = GetMeterPortFunc(bPort+1);
			if(bFunc == COM_FUNC_METER)
			{
				//�����
				memset(&tFwdMsg, 0, sizeof(tFwdMsg));
				//tFwdMsg.pbTxBuf = bTxFrm;
				tFwdMsg.pbRxBuf = bRxFrm;
				tFwdMsg.wRxBufSize = sizeof(bRxFrm);
				tFwdMsg.wTxLen = dwLen;

				ReadItemEx(BN0,wDstPn, 0x890b, bMtrFwdBuf);

				tFwdMsg.CommPara.wPort = bPort+1;
				tFwdMsg.CommPara.dwBaudRate = ValToBaudrate(bMtrFwdBuf[0]);
				tFwdMsg.CommPara.bParity = ValToParity(bMtrFwdBuf[1]);
				tFwdMsg.CommPara.bByteSize = ValToByteSize(bMtrFwdBuf[2]);
				tFwdMsg.CommPara.bStopBits = ValToStopBits(bMtrFwdBuf[3]);

				tFwdMsg.wFrmTimeout = 2* 1000;
				tFwdMsg.wByteTimeout = 2*10;

				tFwdMsg.dwClick = GetClick();
				
				DTRACE(DB_FAPROTO, ("CFaProto::WritePointAdjTimePara: wPort=%d, dwBaudRate=%ld, bParity=%d, bByteSize=%d, bStopBits=%d, wFrmTimeout=%d\n",
					tFwdMsg.CommPara.wPort, tFwdMsg.CommPara.dwBaudRate, 
					tFwdMsg.CommPara.bParity, tFwdMsg.CommPara.bByteSize, 
					tFwdMsg.CommPara.bStopBits, tFwdMsg.wFrmTimeout));
				TraceBuf(DB_FAPROTO, "Mtr fwd frm->", bTxFrm, tFwdMsg.wTxLen);
				
				if (DirectTransmit645Cmd(wDstPn, bPort-1, bTxFrm, tFwdMsg.wTxLen, tFwdMsg.pbRxBuf, &bRetLen, 2) <= 0)
					return 0;
				tFwdMsg.wRxLen = bRetLen;

				//��Ӧ��֡
				//pTxBuf = m_bTxBuf + SG_LOC_DA1;
				//IdPnToBytes(dwID,wPn,pTxBuf);
				//pTxBuf += 6;

				if (tFwdMsg.wRxLen > 0)
				{
					for (bPos=0; bPos<tFwdMsg.wRxLen; bPos++)
					{
						if (bRxFrm[bPos] == 0x68)
							break;
					}
					if (bPos < tFwdMsg.wRxLen)
						memcpy(bData, &bRxFrm[bPos], tFwdMsg.wRxLen-bPos); 	//ȥ��ǰ���ַ�

					iRet = 0;
					if (bData[SCHED_07_RESPONSE_BIT]==SCHED_07_RESPONSE_OK_TIME)	
					{
		  				 //*pTxBuf++ = SG_ERR_RIGHT;				//��ȷ  
		  				 iRet = 4;
					}
					else if (bData[SCHED_07_RESPONSE_BIT]==SCHED_07_RESPONSE_ABNORMITY_TIME )	
					{
						iRet = 0;
					}
				}
				else
				{
					iRet = 0;
				}
				return iRet;
			}
		}
	}
	return iRet;
	
}


bool SgRxSetPara(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	int iRet;
	BYTE  bDaDtNum = 0, bPortFun=0;
	TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
	TSgDaDt tDaDt[SG_DADT_GRP_SZ];

	#define STORAGE_PARA_CHG_LENTH	8
	WORD 	i;
	WORD 	FpnGrpNum;
	BYTE* pbTx;
	bool 	fFirst=true, fLastSend=false, fHasOneFrm=false;
	BYTE *pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	BYTE *pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];
	WORD    wPIFMaxFrmBytes =pProIf->wMaxFrmBytes; //�ӿ�һ֡�����������

	//[chao
	BYTE dwBufDI[32];
	DWORD dwLenDI = 0;
	TTime ParaChgTime = {0};
	//endchao]

	int iWritten;
	bool fCtrlOk = false;
	BYTE bData;
	BYTE* pbChargeRate = NULL;

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= 5;

	pbRxTail -= 16;	//password

	if (pbRxTail <= pbRx) //�쳣
		goto RxWritePara_err;

	if (SgGetRxSeq(pGbPro) == false) //�ж�֡������
		goto RxWritePara_err;

	pbTx = pGbPro->pbTxBuf + SG_LOC_DA1;
	pGbPro->wTxPtr = SG_LOC_DA1;

	GetCurTime(&ParaChgTime);
	while (pbRx < pbRxTail)
	{
		if(!PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ))
			goto RxWritePara_err;  //�����ܽ��մ���DIDA�������


		FpnGrpNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);
		//[chao :�ݴ���������¼����Ҫ�����ݱ�ʶ����
		memcpy(dwBufDI+dwLenDI,pbRx+2,4);
		dwLenDI += 4;
		if(STORAGE_PARA_CHG_LENTH == dwLenDI/4)//���ID�ı�������ﵽ8��,�ʹ�һ�μ�¼
		{
			IsTerminalParaChgRec(dwBufDI,dwLenDI,ParaChgTime);//������������¼
			dwLenDI = 0;
		}
		//-------------------------------------------------------endchao]
		pbRx += 6;    //DA 2�ֽ� DI 4�ֽ��ܹ�6�ֽ�
		if (!FpnGrpNum)
			goto RxWritePara_err;

		for (i=0; i<FpnGrpNum; i++)
		{
			if(tFnPn[i].dwId >= 0xe0000230 && tFnPn[i].dwId <= 0xe0000250)
			{
				//д�������������Ϣ
				iRet = TaskCheck(pbRx, pbRxTail, tFnPn[i].dwId);
				if (iRet > 0)
				{
					iWritten = WriteTaskConfig(tFnPn[i].dwId, pbRx, iRet, 0 , NULL);
					iRet = iWritten;		//Ϊ�������óɹ�������ж�
				}

			}
			else if((tFnPn[i].dwId>= 0xe0000301 && tFnPn[i].dwId <= 0xe00003fe)
				|| (tFnPn[i].dwId >= 0xe0000401 && tFnPn[i].dwId<= 0xe00004fe))
			{
				//д��ͨ/�м�����������Ϣ
				iRet = TaskCheck(pbRx, pbRxTail, tFnPn[i].dwId);
				if (iRet > 0)
				{
					iWritten = WriteTaskConfig(tFnPn[i].dwId, pbRx, iRet);
					iRet = iWritten;		//Ϊ�������óɹ�������ж�
				}
			}
			else if(tFnPn[i].dwId >= 0xe0001100 && tFnPn[i].dwId <= 0xe0001104)
			{
				//д��������Ʋ���
				iRet  = WritePointCtrlPara(tFnPn[i].bPn,tFnPn[i].dwId,pbRx);
				fCtrlOk = iRet > 0 ? true : false;
				iRet = abs(iRet);
			}
			else if(tFnPn[i].dwId == 0xe0000131)
			{
				iRet = WritePointAdjTimePara(tFnPn[i].bPn,tFnPn[i].dwId,pbRx);
			}
			else if((tFnPn[i].dwId >= 0xe1800001 && tFnPn[i].dwId <= 0xe1800018) ||
					(tFnPn[i].dwId >= 0xe0000b00 && tFnPn[i].dwId <= 0xe0000b02))
			{
				//�ն��������ݲ�֧��д,ֻ�ܶ�
				//�ն˰汾����,ֻ�ܶ�
				iRet = 0;
			}
			else if(tFnPn[i].dwId == 0xe0800205) 
			{
				iRet = WriteItemDw(BN0,tFnPn[i].bPn,tFnPn[i].dwId,pbRx);//��Լ���ѽ��������ع̶�Ϊ9����ֵ

			}
			else if(tFnPn[i].dwId==0xe0800000)
			{
				if ((GetMtrClassNum(0x8906, tFnPn[i].bPn, 1)>=10 && *pbRx==1) || (GetMtrClassNum(0x890a, tFnPn[i].bPn, 31) >= MAX_CCT_METER && pbRx[0]==31))
					iRet = -1;
				else
					iRet = WriteItemDw(BN0,tFnPn[i].bPn,tFnPn[i].dwId,pbRx);
			}
			else if(tFnPn[i].dwId==0xe0800008 || tFnPn[i].dwId==0xe080000f)//20140225-1
			{
				if(tFnPn[i].dwId==0xe080000f && pbRx[0] == 1 && ( (GetMtrClassNum(0x8906, tFnPn[i].bPn, 1)>=MAX_VIP_METER && pbRx[11]==1) || 
					 (GetMtrClassNum(0x890a, tFnPn[i].bPn, 31) >= MAX_CCT_METER && pbRx[20]==31) ))	//�ص㻧��10�� �� �ز���32�� 
				{
					iRet = -1;
				}
				else
				{
#define CHARGE_RATE_PLACE	13
					pbChargeRate = pbRx;
					if (tFnPn[i].dwId==0xe080000f)
						pbChargeRate = pbRx+CHARGE_RATE_PLACE;

					bData = BcdToByte(*pbChargeRate);
					if(bData>RATE_NUM)
						*pbChargeRate = ByteToBcd(RATE_NUM);
					iRet = WriteItemDw(BN0,tFnPn[i].bPn,tFnPn[i].dwId,pbRx);

				}
			}
			else if((tFnPn[i].dwId>=0xe0000500 && tFnPn[i].dwId<=0xe00008ff) ||
				(tFnPn[i].dwId>=0xe0000a20 && tFnPn[i].dwId<=0xe0000a26) )

			{
				//�ն˲�����������II�Ͳ�֧��
				iRet = -1;
			}
			else if(tFnPn[i].dwId == 0xe0000a01)
            {                
                iRet = WriteItemDw(BN0,tFnPn[i].bPn,tFnPn[i].dwId,pbRx);
                
                if (pbRx[0] == 0)   //485-2����Ϊ�����
               	    bPortFun = 0;
                else    //485-2����Ϊά��/���Կ�
               	    bPortFun = 0xff;
                WriteItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
            }
			else if((tFnPn[i].dwId&0xf0000000) != 0xe0000000)
            {
  				iRet = -1;
            }
            else
			{
				iRet = WriteItemDw(BN0,tFnPn[i].bPn,tFnPn[i].dwId,pbRx);
			}

			if (iRet <= 0)
			{
				tDaDt[bDaDtNum].bErr = SG_ERR_DATAERR;
				goto RxWritePara_err;
			}

			IdPnToBytes(tFnPn[i].dwId, tFnPn[i].bPn, pbTx); //��ID,PNת����ͨ��Э���ʽ��6���ֽ�
			pbTx += 6;
			pGbPro->wTxPtr += 6;
			
			if(tFnPn[i].dwId >= 0xe0001100 && tFnPn[i].dwId <= 0xe0001104) //��������բ�������
			{
				*pbTx++ = fCtrlOk ? 0x00 : 0x01; //д�ɹ�
			}
			else
			{
				*pbTx++ = 0x00; //д�ɹ�
			}
			pGbPro->wTxPtr += 1;


			//�Ѿ���ȷ�����ˣ����⴦��һ�¿�����
			if(tFnPn[i].dwId==0xe0000c0f || tFnPn[i].dwId==0xe0000c2f)
				iRet = 15;

			if (pGbPro->wTxPtr>=wPIFMaxFrmBytes || pGbPro->wTxPtr>=1000)
			{					//TODO:GB_MAXDATASIZEӦ������Ӧ����ӿڵ�������ֽ�
				if ( fFirst )
				{
					fFirst = false;
					pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
				}
				else
					pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

				if ((pbRx>=pbRxTail && i+1==FpnGrpNum))
				{
					fLastSend = true;
					pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;
				}
				pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;

				//����
				SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_SETPARA, pGbPro->wTxPtr);
				Sleep(2000);

				//����
				fHasOneFrm = true;
				pbTx = pGbPro->pbTxBuf + SG_LOC_DA1;
				pGbPro->wTxPtr = SG_LOC_DA1;

			}
		}	//end for (i=0; i<FpnGrpNum; i++)
		pbRx += iRet;   //��������
	}

	//[chao
	if(dwLenDI != 0)
	{
		//memset(dwBufDI+dwLenDI,INVALID_DATA,(STORAGE_PARA_CHG_LENTH-dwLenDI/4)*4);
		//IsTerminalParaChgRec(dwBufDI,STORAGE_PARA_CHG_LENTH*4,ParaChgTime);//������������¼
		IsTerminalParaChgRec(dwBufDI,dwLenDI,ParaChgTime);//������������¼
	}
	//endchao]

	if (fLastSend == false)
	{
		if (pbTx != &pGbPro->pbTxBuf[SG_LOC_DA1])
		{
			if ( fFirst )
				pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
			else
				pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

			//if (pbRx >= pbRxTail)
			pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;

			fHasOneFrm = true;
			pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;
			SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_SETPARA, pGbPro->wTxPtr);
		}
		else
		{
			goto RxWritePara_err;
		}
	}

	if (fHasOneFrm == false)
		goto RxWritePara_err; //һ֡��û�У���ȷ������


	TrigerSavePara();//20131212-1
	return true;
RxWritePara_err:
	return SgAnsConfirm(pPro, tDaDt, bDaDtNum);
}

bool SgRxAskPara(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;	
	WORD i;
	BYTE bDaDtNum = 0;
	bool fFnPnOkOnce;	//��FNPN��ɹ���һ��
	TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
	TSgDaDt tDaDt[SG_DADT_GRP_SZ];
	int  	iRet;
	WORD 	FpnGrpNum,nFailNum;
	bool 	fFirst=true, fLastSend=false, fHasOneFrm=false;
	WORD    wPIFMaxFrmBytes =pProIf->wMaxFrmBytes; //�ӿ�һ֡�����������

	BYTE *pbTx;
	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	BYTE* pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= 5;

	if (SgGetRxSeq(pGbPro) == false)
		goto RxAskPara_err;


	nFailNum = 0;
	pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;

	while (pbRx < pbRxTail)
	{
		
		if(!PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ))
			goto RxAskPara_err;  //�����ܽ��մ���DIDA�������


		FpnGrpNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);
		pbRx += 6;    //DA 2�ֽ� DI 4�ֽ��ܹ�6�ֽ�
		if (!FpnGrpNum)
			goto RxAskPara_err;

		for (i=0; i<FpnGrpNum; i++)
		{
			if((tFnPn[i].dwId >= 0xe0000230 && tFnPn[i].dwId <= 0xe0000250) || (tFnPn[i].dwId == 0xe0000221))
			{
				//���������������Ϣ
				if(tFnPn[i].dwId == 0xe0000221)
				{
					*(pbTx+6) = ByteToBcd(GetRdMtrTaskValidNum());
					iRet = 1;
				}
				else
				{
					iRet = ReadTaskConfig(tFnPn[i].dwId, pbTx+6);
				}
			}
			else if((tFnPn[i].dwId >= 0xe0000300 && tFnPn[i].dwId <= 0xe00003fe) ||
				(tFnPn[i].dwId >= 0xe0000400 && tFnPn[i].dwId <= 0xe00004fe))
			{
				//����ͨ����/�м�����������Ϣ
				if(0x0300 == (tFnPn[i].dwId & SG_DWORD_GET_LOW_WORD))
				{
					iRet = ReadItemEx(BN0, tFnPn[i].bPn,0x8100,pbTx+6);		//����ͨ��������
				}
				else if(0x0400 == (tFnPn[i].dwId & SG_DWORD_GET_LOW_WORD))
				{
					iRet = ReadItemEx(BN0, tFnPn[i].bPn,0x8101,pbTx+6);		//���м���������
				}
				else
				{
					iRet = ReadTaskConfig(tFnPn[i].dwId, pbTx+6);
				}
			}
			else if(tFnPn[i].dwId == 0xe0800205) 
			{
				iRet = ReadItemDw(BN0,tFnPn[i].bPn,tFnPn[i].dwId,pbTx+6);//��Լ���ѽ��������ع̶�Ϊ9����ֵ

			}
			else if((tFnPn[i].dwId>=0xe0000500 && tFnPn[i].dwId<=0xe00008ff) ||
				(tFnPn[i].dwId>=0xe0000a20 && tFnPn[i].dwId<=0xe0000a26) )

			{
				//�ն˲�����������II�Ͳ�֧��
				iRet = -1;
			}
			else
			{
				iRet = ReadItemDw(BN0,tFnPn[i].bPn,tFnPn[i].dwId,pbTx+6);
			}
			if (iRet <= 0)
			{
				nFailNum++;
				continue;
			}

			IdPnToBytes(tFnPn[i].dwId, tFnPn[i].bPn, pbTx); //��ID,PNת����ͨ��Э���ʽ��6���ֽ�
			pbTx += iRet + 6;
			pGbPro->wTxPtr += iRet + 6;

			/*if(tFnPn[i].dwId==0xe0000c0f || tFnPn[i].dwId==0xe0000c1f)	//���ݿ��Ѿ�����15���ֽ���
			{
				//�������������⴦��
				memset(pbTx, 0x00, 15-iRet);
				pbTx += (15-iRet);
				pGbPro->wTxPtr += (15-iRet);
			}*/
			if (pGbPro->wTxPtr>=wPIFMaxFrmBytes || pGbPro->wTxPtr>=1000)
			{					//TODO:GB_MAXDATASIZEӦ������Ӧ����ӿڵ�������ֽ�
				if ( fFirst )
				{
					fFirst = false;
					pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
				}
				else
					pGbPro->pbTxBuf[SG_LOC_SEQ] =pGbPro->bHisSEQ;

				if (pbRx>=pbRxTail && i+1==FpnGrpNum)
				{
					fLastSend = true;
					pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;
				}
				pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;

				//����
				SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_ASKPARA, pGbPro->wTxPtr);
				Sleep(2000);

				//����
				fHasOneFrm = true;
				pbTx = pGbPro->pbTxBuf + SG_LOC_DA1;
				pGbPro->wTxPtr = SG_LOC_DA1;

			}
		}	//end for (i=0; i<FpnGrpNum; i++)


		//tDaDt[bDaDtNum].bItemERR = (nFailNum==FpnGrpNum)? SG_ERR_NOVALIDDATA : SG_ERR_OK;
		tDaDt[bDaDtNum].bErr = (nFailNum==FpnGrpNum)? SG_ERR_DATAERR : SG_ERR_OK;
		bDaDtNum++;	
	}

	if (fLastSend == false)
	{
		if (pbTx != &pGbPro->pbTxBuf[SG_LOC_DA1])
		{
			if ( fFirst )
				pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
			else
				pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

			//if (pbRx >= pbRxTail)
			pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;

			fHasOneFrm = true;
			pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;
			SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_ASKPARA, pGbPro->wTxPtr);
		}
		else
		{
			goto RxAskPara_err;
		}
	}

	if (fHasOneFrm == false)
		goto RxAskPara_err; //һ֡��û�У���ȷ������

	return true;

RxAskPara_err:
	return SgAnsConfirm(pPro, tDaDt, bDaDtNum);
}


//������ ��֡��������ȡ(Id,Pn)��ÿ����һ����ȡһ����Ч��(Id,Pn)��ֱ��ȫ��ȡ��
//������ @bFrmType       ֡���ͣ�����
//      @ppbFrm         ��һ��DADI����ʼ��ַָ�룬˫��
//      @pbFrmEnd       ���һ��DADI��֮��ĵ�ַ������
//      @pwDaIdx        ��Ϣ��Ԫ��ʼƫ������һ���ʼ��Ϊ0���룬˫��
//      @pdwID          ID�����
//      @pwPn           �����㣬���
//      @ppbData        DADI���������ݵĵ�ַָ�룬���
//���أ� �ɹ�������true�����򣬷���false
// bool GetIdPnFromFrm(BYTE bFrmType, BYTE** ppbFrm, BYTE* pbFrmEnd, 
// 							  WORD* pwDaIdx, DWORD* pdwID, WORD* pwPn, 
// 							  BYTE** ppbData)
// {
// 	BYTE bStepLen = 0;
// 	
// 	bool fNextDADI = false;
// 	WORD wBasePn = 0;
// 	//���������Ч��������Ч��Pn��ȡ��
// 	if (!ppbFrm || !(*ppbFrm) || !pbFrmEnd || !pwDaIdx || !pdwID || !pwPn ||
// 		!ppbData || *ppbFrm >= pbFrmEnd)
// 	{
// 		return false;
// 	}
// 
// 	//����֡����ȡ��DADI�鲽������
// 	
// 	switch (bFrmType)
// 	{
// 	case FRM_TYPE_NO_DATA:
// 		bStepLen = DADI_LEN;
// 		break;
// 	case FRM_TYPE_TIME_ONLY:
// 		bStepLen = DADI_LEN + DATA_TIME_LEN * 2;
// 		break;
// 	case FRM_TYPE_TIME_DENSITY:
// 		bStepLen = DADI_LEN + DATA_TIME_LEN * 2 + DATA_DENSITY_LEN;
// 		break;
// 	default:
// 		return false;
// 	}
// 
// 	
// 
// 	//ȡ����һ����Ч��Pn
// 	if (!(**ppbFrm) || !(*(*ppbFrm+1)))
// 	{
// 		*pwPn = 0;
// 		fNextDADI = true;
// 
// 	}
// 	else
// 	{
// 		if (0xff == (**ppbFrm) && 0xff == (*(*ppbFrm+1)))		//20131224-3
// 		{
// 			if (0 == *pwDaIdx)//���в�����ȥ���ն˱���
// 				*pwDaIdx = 1;
// 
// 			*pwPn = *pwDaIdx;
// 			(*pwDaIdx)++;
// 			if (*pwDaIdx >= POINT_NUM)//������
// 				fNextDADI = true;
// 		}
// 		else
// 		{
// 			//Ѱ����ЧPn
// 			while (!(1 & (**ppbFrm >> (*pwDaIdx)++)));
// 
// 			wBasePn = (*(*ppbFrm+1) << 3) - 7;
// 			*pwPn = wBasePn + *pwDaIdx - 1;
// 
// 			if (!(**ppbFrm>>*pwDaIdx) || (*pwPn+1>=POINT_NUM))                       
// 				fNextDADI = true;
// 		}
// 	}
// 
// 	//ȡ��ID
// 	*pdwID = ByteToDWORD(*ppbFrm+2, 4);
// 
// 	//֡������ʼ��ַ(����ʱ�����ַ)������������ָ����һ��DADI��ַ
// 	*ppbData = *ppbFrm + DADI_LEN;
// 
// 	//һ��DADI�ѱ����꣬ת����һ��DADI
// 	if (fNextDADI)
// 	{
// 		*ppbFrm += bStepLen;
// 		*pwDaIdx = 0;
// 	}
// 
// 	return true;
// }

bool MeterSpecierId(DWORD dwId)
{
	if(dwId>=0x03100000 && dwId<=0x03100300)
		return true;
	if(dwId>=0xe1000010 && dwId<=0xe1004036)//20140624-1
		return true;

	if((dwId&0xFFFFFF00)==0xe1800000)
		return true;

	return false;
}

int SgRxCallClass1(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	BYTE* pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];
	BYTE *pbTx;
	int iRet, iTxBufLeft;
	WORD i;
	BYTE bFnPnNum, bDaDtNum = 0;
	bool fFirst=true, fLastSend=false, fHaveOneFrm=false, fTxFrmThisRnd, fFirstRnd;
	bool fFnPnOkOnce;	//��FNPN��ɹ���һ��
	TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
	TSgDaDt tDaDt[SG_DADT_GRP_SZ];
	bool fAllPn = false;
	bool fGetDirRdCtrl = false;
	WORD wReadPn = 0;
	BYTE bPort=0;

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= 5;

	if (!SgGetRxSeq(pGbPro))
		goto GbRxCallClass1_err;

	pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;

	//
	while (pbRx < pbRxTail)
	{
		fAllPn = false;
		if (*pbRx == 0xff && *(pbRx+1)==0xff) 
		{
			fAllPn = true;

			//��һ����Ч�Ĳ�����
			while(wReadPn < POINT_NUM)
			{
				if(IsPnValid(wReadPn))
					break;

				wReadPn++;
			}	

			//��������Ч������һ��һ������������������fn��pn��ȡ�ķ�ʽ��ģ��ʵ��0xffһ�ζ�ȡ���в�����
			PnToBytes(wReadPn, pbRx);  //�����ٱ��0xff 0x00
		}
		//get each FN/PN data cyclely
		PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ);
		bFnPnNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);
		if (!fAllPn)
			pbRx += 6;

		if (bFnPnNum == 0)
			goto GbRxCallClass1_err;

		fFnPnOkOnce = false;	//��FNPN��ɹ���һ��
		for (i=0; i<bFnPnNum; i++)
		{
			fFirstRnd = true;		//��1��ѭ��
			do
			{
				fTxFrmThisRnd = false;	//�����Ƿ�Ҫ����һ֡
				iTxBufLeft = (int )pProIf->wMaxFrmBytes - (pbTx - pGbPro->pbTxBuf) - 6 - 10;
				//6��DADT�Ŀռ䣬10�Ƕ�Ԥ���Ŀռ�
				//if(MeterSpecierId(tFnPn[i].dwId)) //�ն��������ݶ��� ����Ҫ�ύ���� ֱ�Ӷ�����
				//{
					
				//}
				//else
				{
					if (!fGetDirRdCtrl)	 //�ڵ�һ��������ȡ�ó������Ȩ��ֱ����֡��������ͷſ���Ȩ��
					{					 //���ⷴ���������������
   						GetDirRdCtrl(PORT_GB485);	//ȡ��ֱ���Ŀ���Ȩ
						GetDirRdCtrl(PORT_CCT_PLC);	//ȡ��ֱ���Ŀ���Ȩ
						fGetDirRdCtrl = true;
					}
					iRet = SgReadClass1(tFnPn[i].dwId, tFnPn[i].bPn, pbTx+6, iTxBufLeft, pGbPro->fRptState);
				}
                
				if (iRet > 0)	//ȫ���򲿷ֶ�����Ӧ������
				{
					fFnPnOkOnce = true;	//��FNPN��ɹ���һ��
					IdPnToBytes(tFnPn[i].dwId, tFnPn[i].bPn, pbTx); //��ID,PNת����ͨ��Э���ʽ��6���ֽ�
					pbTx += iRet + 6;
				}
				else if (iRet == -GB_RDERR_NOROOM)	//�ռ䲻��,һ������������󣬶�������û���κ���Ч������
				{
					fTxFrmThisRnd = true;	//����Ҫ����һ֡
				}
				else if (iRet == -GB_RDERR_FAIL)	//��ʧ��
				{
					break;	//�˳����������ѭ�����������κζ���
				}

				fFirstRnd = false;	//��1��ѭ��

				if (fTxFrmThisRnd)	//����Ҫ����һ֡
				{
					//�ȷ��ͱ�֡
					if (fFirst)
					{
						fFirst = false;
						pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
					}
					else
						pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

					if ((!fAllPn && pbRx>=pbRxTail && i+1==bFnPnNum) || (fAllPn && wReadPn>=POINT_NUM))
					{
						fLastSend = true;
						pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;
					}
					pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;

					SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_ASKCLASS1, pbTx-pGbPro->pbTxBuf);

					//���³�ʼ������
					bDaDtNum = 0;
					fHaveOneFrm = true;
					pbTx = pGbPro->pbTxBuf + SG_LOC_DA1;
					pGbPro->wTxPtr = SG_LOC_DA1;
				}
			} while (iRet == -GB_RDERR_NOROOM);
		}	//end for (i=0; i<bFnPnNum; i++) = false;	

		if (!fAllPn || (fAllPn && wReadPn>=POINT_NUM))
		{
			tDaDt[bDaDtNum].bErr = fFnPnOkOnce ? SG_ERR_OK : SG_ERR_NOVALIDDATA;
			bDaDtNum++;
			if (bDaDtNum >= SG_DADT_GRP_SZ)
			{
				DTRACE(DB_FAPROTO, ("SgRxCallClass1: ##### err! DaDtNum>SG_DADT_GRP_SZ!\r\n"));
				break;
			}
		}
		
		if(fAllPn) //�ж�һ�º����Ƿ񻹴�����Ч�Ĳ�����
		{
			*pbRx = 0xff;
			*(pbRx+1) = 0xff;

			wReadPn++;

			while(wReadPn<POINT_NUM)
			{
				if(IsPnValid(wReadPn))
					break;

				wReadPn++;
			}
			if(wReadPn>=POINT_NUM) //�Ѿ������һ����������
			{
				pbRx += 6;
				wReadPn = 0;
			}
			
		}

	}

	if (!fLastSend)
	{
		if (pbTx != &pGbPro->pbTxBuf[SG_LOC_DA1])
		{
			if (fFirst)
				pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
			else
				pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

			pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;

			fHaveOneFrm = true;
			pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;
			SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_ASKCLASS1, pbTx-pGbPro->pbTxBuf);
		}
		else
		{
			goto GbRxCallClass1_err;
		}
	}

	if (fHaveOneFrm == false)
		goto GbRxCallClass1_err; //һ֡��û�У���ȷ������

	if (fGetDirRdCtrl)
    {
		ReleaseDirRdCtrl(PORT_GB485); //�ͷ�ֱ���Ŀ���Ȩ
   		ReleaseDirRdCtrl(PORT_CCT_PLC); //�ͷ�ֱ���Ŀ���Ȩ
    }

	return true;

GbRxCallClass1_err:
	if (fGetDirRdCtrl)
    {
   		ReleaseDirRdCtrl(PORT_GB485); //�ͷ�ֱ���Ŀ���Ȩ
   		ReleaseDirRdCtrl(PORT_CCT_PLC); //�ͷ�ֱ���Ŀ���Ȩ
    }

	if (pGbPro->fRptState)
		return true;
	else
		return SgAnsConfirm(pPro, tDaDt, bDaDtNum);
}
const BYTE Data2TimeByteLen[] = {0, 0, 7, 3, 2, 3};
bool SgRxCallClass2(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	BYTE* pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];
	BYTE *pbTx;
	int iRet, iTxBufLeft;
	WORD i, wStart;
	TTime tmStart, tmEnd, tmRxStart;
	//DWORD dwMinStart;
	WORD wToRdRecNum;
	BYTE bFnPnNum, bDaDtNum = 0;
	BYTE bIntervU;
	BYTE bInterv, bRate, bTmLen;
	bool fFirst=true, fLastSend=false, fHaveOneFrm=false, fTxFrmThisRnd;
	bool fFnPnOkOnce;	//��FNPN��ɹ���һ��
	TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
	TSgDaDt tDaDt[SG_DADT_GRP_SZ];
	WORD wReadPn, wCurPn, wTotalPn, n;
	bool fAllPn = false;
	int iIntervNum=0;
	const TCommTaskCtrl* pTaskCtrl=NULL;

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= TP_LEN;

	if ( !SgGetRxSeq(pGbPro) )
		goto GbRxCallClass2_err;

	pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;

	while (pbRx < pbRxTail)
	{
		if (*pbRx == 0xff && *(pbRx+1)==0xff)	//ȫ����Ч������
		{
			wReadPn=0;
			wCurPn = 0;
			wTotalPn = 0;
			fAllPn = true;
			*pbRx = 0x00;

			for(n=0; n<POINT_NUM; n++)
			{
				if(IsPnValid(n))
					wTotalPn++;
			}

			DTRACE(DB_CCT,("RxCmd_CallClass2: Read Pn Total=%d.\r\n", wTotalPn));
		}
		else
		{
			//get each FN/PN data cyclely
			PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ);
		}
		
		bFnPnNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);
		pbRx += DADI_LEN;
		if (bFnPnNum == 0)
			goto GbRxCallClass2_err;

		BcdToTime(pbRx, &tmStart);
		BcdToTime(pbRx+DATA_TIME_LEN, &tmEnd);
		bRate = pbRx[DATA_TIME_LEN<<1];		//����
		bTmLen = (DATA_TIME_LEN<<1) + 1;	//����ʱ�곤��=13 ��ʼʱ��6 + ����ʱ��6 + �ܶ�1			

		tmRxStart = tmStart;    //��¼��ȡ���ʱ��
		fFnPnOkOnce = false;	//��FNPN��ɹ���һ��
		for (i=0; i<bFnPnNum; i++)
		{
			wReadPn=0;
			wCurPn = 0;

			pTaskCtrl = ComTaskIdToCtrl(tFnPn[i].dwId);
			if (pTaskCtrl==NULL)
			{
				pbRx += bTmLen;
				continue;	//goto GbRxCallClass2_err;
			}

			bIntervU = pTaskCtrl->bIntervU;
			if (bIntervU == TIME_UNIT_HOUR)
			{								
				tmRxStart.nMinute = 0;

				tmEnd.nMinute = 0;
			}
			else if (bIntervU == TIME_UNIT_DAY)
			{																
				tmRxStart.nHour = 0;	
				tmRxStart.nMinute = 0;

				tmEnd.nHour = 0;	
				tmEnd.nMinute = 0;
			}
			else if (bIntervU == TIME_UNIT_MONTH)
			{
				tmRxStart.nDay = 1;
				tmRxStart.nHour = 0;	
				tmRxStart.nMinute = 0;

				tmEnd.nDay = 1;
				tmEnd.nHour = 0;	
				tmEnd.nMinute = 0;
			}

			tmStart = tmRxStart;    //��¼��ȡ���ʱ��	    //ÿ��ID����Э��ȡ ��¼�����Ķ�ȡ���ʱ�䣨tmStart��֡���������޸�, ����ȡtmRxStart��
            TimeToBcd(&tmStart, pbRx);
			TimeToBcd(&tmEnd, pbRx+DATA_TIME_LEN);
			iIntervNum = IntervsPast(&tmStart, &tmEnd, bIntervU, 1);
			if (iIntervNum > 0)
				wToRdRecNum = iIntervNum;
			else
				wToRdRecNum = 0;				

			do 
			{
				wStart = 0;

				if(fAllPn)
				{
					while(wCurPn<POINT_NUM )
					{
						wCurPn++;
						if(IsPnValid(wCurPn))
						{
							tFnPn[i].bPn = wCurPn;
							break;
						}
					}
					
					wReadPn++;
					if(wReadPn>=wTotalPn)
					{
						wCurPn = POINT_NUM;
						break;
					}

					tmStart = tmRxStart;					
					TimeToBcd(&tmStart, pbRx);
				}

				while (wStart < wToRdRecNum)
				{
					fTxFrmThisRnd = false;	//�����Ƿ�Ҫ����һ֡
					iTxBufLeft = (int )pProIf->wMaxFrmBytes - (pbTx - pGbPro->pbTxBuf) - DADI_LEN - 10;	//6��DADI�Ŀռ䣬10�Ƕ�Ԥ���Ŀռ�
					iRet = SgReadClass2(tFnPn[i].dwId, tFnPn[i].bPn, pbRx, pbTx+DADI_LEN, iTxBufLeft, &wStart, pGbPro->fRptState);
					if (iRet > 0)	//ȫ���򲿷ֶ�����Ӧ������
					{
						fFnPnOkOnce = true;	//��FNPN��ɹ���һ��
						IdPnToBytes(tFnPn[i].dwId, tFnPn[i].bPn, pbTx); //��ID,PNת����ͨ��Э���ʽ��6���ֽ�
						pbTx += iRet + DADI_LEN;
					}
					else if (iRet == -GB_RDERR_NOROOM)	//�ռ䲻��,һ������������󣬶�������û���κ���Ч������
					{
						fTxFrmThisRnd = true;	//����Ҫ����һ֡
						/*if(fAllPn)  //�ܵ�������˵����������Ч����������������
						{
							wCurPn--;
							wReadPn--;
						}*/
					}
					else if (iRet == -GB_RDERR_FAIL)	//��ʧ��
					{
						break;	//�˳����������ѭ�����������κζ���
					}
		
					if (fTxFrmThisRnd)	//����Ҫ����һ֡
					{
						//�ȷ��ͱ�֡
						if (fFirst)
						{
							fFirst = false;
							pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
						}
						else
							pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;
		
						if (pbRx+bTmLen>=pbRxTail && i+1==bFnPnNum)
						{
							if ((!fAllPn || (fAllPn && wCurPn>=PN_NUM)) && wStart>=wToRdRecNum)
							{
								fLastSend = true;
								pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;
							}
							
						}
						pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;
		
						SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_ASKCLASS2, pbTx-pGbPro->pbTxBuf);
		
						//���³�ʼ������
						bDaDtNum = 0;
						fHaveOneFrm = true;
						pbTx = pGbPro->pbTxBuf + SG_LOC_DA1;
						pGbPro->wTxPtr = SG_LOC_DA1;
						//�������ʱ��
						if (wStart < wToRdRecNum)
						{
							tmStart = tmRxStart;
							AddIntervs(&tmStart, bIntervU, wStart);
							TimeToBcd(&tmStart, pbRx);
						}
					}
				} //while (wStart < wToRdRecNum)

			}while (fAllPn && wCurPn<POINT_NUM);						
		}	//end for (i=0; i<bFnPnNum; i++) = false;	

		if (!pGbPro->fRptState || (pGbPro->fRptState && (i+1==bFnPnNum)))
		{
			pbRx += bTmLen;
		}

		tDaDt[bDaDtNum].bErr = fFnPnOkOnce ? SG_ERR_OK : SG_ERR_NOVALIDDATA;
		bDaDtNum++;
		if (bDaDtNum >= SG_DADT_GRP_SZ)
		{
			DTRACE(DB_FAPROTO, ("GbRxCallClass2: ##### err! DaDtNum>SG_DADT_GRP_SZ!\r\n"));
			break;
		}
	}

	if (!fLastSend)
	{
		if (pbTx != &pGbPro->pbTxBuf[SG_LOC_DA1])
		{
			if (fFirst)
				pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
			else
				pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

			pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;

			fHaveOneFrm = true;
			pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;
			SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_ASKCLASS2, pbTx-pGbPro->pbTxBuf);
		}
		else
		{
			goto GbRxCallClass2_err;
		}
	}

	if (fHaveOneFrm == false)
		goto GbRxCallClass2_err; //һ֡��û�У���ȷ������

	return true;

GbRxCallClass2_err:
	if (pGbPro->fRptState)	//�����ϱ� ���ϱ�����֡
		return true;
	else
		return SgAnsConfirm(pPro, tDaDt, bDaDtNum);
}

bool SgRxCallClass3(struct TPro* pPro, BYTE bAFn)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	BYTE* pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];
	BYTE *pbTx;
	int iRet, iTxBufLeft;
	WORD i;
	BYTE bFnPnNum, bDaDtNum = 0;
	WORD wReadNum = 0;
	bool fFirst=true, fLastSend=false, fHaveOneFrm=false, fTxFrmThisRnd;
	bool fFnPnOkOnce;	//��FNPN��ɹ���һ��
	TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
	TSgDaDt tDaDt[SG_DADT_GRP_SZ];
	bool fAllPn = false;
	bool fGetDirRdCtrl = false;
	WORD wReadPn = 0;
	DWORD dwTmStr = 0;
	DWORD dwTmEnd = 0;
	WORD	wTotal = 0;
	bool  fGetData = false;

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= 5;

	if (!SgGetRxSeq(pGbPro))
		goto RxReadAlrData_err;

	pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;
	while (pbRx < pbRxTail)
	{
		fAllPn = false;
		if (*pbRx == 0xff && *(pbRx+1)==0xff) 
		{
			fAllPn = true;

			//��һ����Ч�Ĳ�����
			while(wReadPn < POINT_NUM)
			{
				if(IsPnValid(wReadPn))
					break;
				wReadPn++;
			}	
			//��������Ч������һ��һ������������������fn��pn��ȡ�ķ�ʽ��ģ��ʵ��0xffһ�ζ�ȡ���в�����
			PnToBytes(wReadPn, pbRx);  //�����ٱ��0xff 0x00
		}
		PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ);
		bFnPnNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);

		if(IsAllAByte(pbRx+6, INVALID_DATA, 12))
		{
			dwTmStr = dwTmEnd = 0xffffffff;//10��ȫ��
		}
		else
		{
			//�����ȡ
			dwTmStr = tmBufToSecond(pbRx+6);/////xzzzzzzzz
			dwTmEnd = tmBufToSecond(pbRx+12);
		}

		if (!fAllPn)
			pbRx += (6+6+6);

		if (bFnPnNum == 0)
			goto RxReadAlrData_err;

		fFnPnOkOnce = false;	//��FNPN��ɹ���һ��
		for (i=0; i<bFnPnNum; i++)
		{
			wReadNum = 0;
			wTotal = 500;
			if(tFnPn[i].dwId == 0xE20001FF)
				wTotal = 10;
			else if(IsEventId(tFnPn[i].dwId))
				wTotal = 50;
			do
			{
				fTxFrmThisRnd = false;	//�����Ƿ�Ҫ����һ֡
				iTxBufLeft = (int )pProIf->wMaxFrmBytes - (pbTx - pGbPro->pbTxBuf) - 6 - 10;
				//4��DADT�Ŀռ䣬10�Ƕ�Ԥ���Ŀռ�

				iRet = SgReadClass3(tFnPn[i].dwId, tFnPn[i].bPn, dwTmStr, dwTmEnd, pbTx+6, &wReadNum, wTotal, iTxBufLeft);


				if (iRet > 0)	//ȫ���򲿷ֶ�����Ӧ������
				{
					fGetData = true;
					break;
				}

				//���³�ʼ������
				bDaDtNum = 0;
			} while (wReadNum < wTotal);//��ȡһ��PNID��10�ʼ�¼��ȡ�ʺ�Ҫ�������

			if(fGetData)
				break;//ȷ���п������ݣ�ֱ���˳���ѯ
		}	//end for (i=0; i<bFnPnNum; i++) = false;	

		if(fGetData)
			break;//ȷ���п������ݣ�ֱ���˳���ѯ

		if(fAllPn) //�ж�һ�º����Ƿ񻹴�����Ч�Ĳ�����
		{
			*pbRx = 0xff;
			*(pbRx+1) = 0xff;

			wReadPn++;

			while(wReadPn<POINT_NUM)
			{
				if(IsPnValid(wReadPn))
					break;
				wReadPn++;
			}
			if(wReadPn>=POINT_NUM) //�Ѿ������һ����������
			{
				pbRx += (6+6+6);
				wReadPn = 0;
			}
		}
	}

	/////ǰ���whileֻ����֪���Ƿ��к������������ݣ�������е�PN��ID��û�з���������������Ҫ�ظ���Ч��

	if(!fGetData)
		goto RxReadAlrData_err;

	bDaDtNum = 0;
	pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];
	pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;
	wReadPn = 0;
	while (pbRx < pbRxTail)
	{
		fAllPn = false;
		if (*pbRx == 0xff && *(pbRx+1)==0xff) 
		{
			fAllPn = true;

			//��һ����Ч�Ĳ�����
			while(wReadPn < POINT_NUM)
			{
				if(IsPnValid(wReadPn))
					break;
				wReadPn++;
			}	
			//��������Ч������һ��һ������������������fn��pn��ȡ�ķ�ʽ��ģ��ʵ��0xffһ�ζ�ȡ���в�����
			PnToBytes(wReadPn, pbRx);  //�����ٱ��0xff 0x00
		}
		PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ);
		bFnPnNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);

		if(IsAllAByte(pbRx+6, INVALID_DATA, 12))
		{
			dwTmStr = dwTmEnd = 0xffffffff;//10��ȫ��
		}
		else
		{
			//�����ȡ
			dwTmStr = tmBufToSecond(pbRx+6);
			dwTmEnd = tmBufToSecond(pbRx+12);
		}

		if (!fAllPn)
			pbRx += (6+6+6);
		
		if (bFnPnNum == 0)
			goto RxReadAlrData_err;

		fFnPnOkOnce = false;	//��FNPN��ɹ���һ��
		for (i=0; i<bFnPnNum; i++)
		{
			wReadNum = 0;
			fGetData = false;
			wTotal = 500;
			if(tFnPn[i].dwId == 0xE20001FF)
				wTotal = 10;
			else if(IsEventId(tFnPn[i].dwId))
				wTotal = 50;

			do
			{
				fTxFrmThisRnd = false;	//�����Ƿ�Ҫ����һ֡
				iTxBufLeft = (int )pProIf->wMaxFrmBytes - (pbTx - pGbPro->pbTxBuf) - 6 - 10;
				//4��DADT�Ŀռ䣬10�Ƕ�Ԥ���Ŀռ�
				
				iRet = SgReadClass3(tFnPn[i].dwId, tFnPn[i].bPn, dwTmStr, dwTmEnd, pbTx, &wReadNum, wTotal, iTxBufLeft);


				if (iRet > 0)	//ȫ���򲿷ֶ�����Ӧ������
				{
					fGetData = true;
					fFnPnOkOnce = true;	//��FNPN��ɹ���һ��
					//IdPnToBytes(tFnPn[i].dwId, tFnPn[i].bPn, pbTx); //��FN,PNת����ͨ��Э���ʽ��4���ֽ�
					pbTx += iRet;
                    if(wReadNum<wTotal && wReadNum>0)
                        fTxFrmThisRnd = true;	//����Ҫ����һ֡
                        
				}

				if (iRet==0 && fGetData==false && wReadNum==wTotal)//û�ж�Ӧ����ʱ,ȫ����FF
		 		{
		 			//������Ӧ��FnPn�����ͻ�����
		 			IdPnToBytes(tFnPn[i].dwId, tFnPn[i].bPn, pbTx);
		 			pGbPro->wTxPtr += 6;
		 			pbTx += 6;

 					memset(pbTx,INVALID_DATA,GetFmtARDLen(tFnPn[i].dwId));
 					pGbPro->wTxPtr += GetFmtARDLen(tFnPn[i].dwId);
 					pbTx += GetFmtARDLen(tFnPn[i].dwId);
		 		}

				if (iRet == -GB_RDERR_NOROOM)	//�ռ䲻��,һ������������󣬶�������û���κ���Ч������
				{
					fTxFrmThisRnd = true;	//����Ҫ����һ֡
				}

				if (fTxFrmThisRnd)	//����Ҫ����һ֡
				{
					//�ȷ��ͱ�֡
					if (fFirst)
					{
						fFirst = false;
						pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
					}
					else
						pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

					if (wReadNum>=wTotal &&//����
						((!fAllPn && pbRx>=pbRxTail && i+1==bFnPnNum) || (fAllPn && wReadPn>=POINT_NUM)))
					{
						fLastSend = true;
						pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;
					}
					pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;

					SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, bAFn, pbTx-pGbPro->pbTxBuf);
                    SetLedCtrlMode(LED_ALARM, LED_MODE_OFF);   //�澯��

					//���³�ʼ������
					bDaDtNum = 0;
					fHaveOneFrm = true;
					pbTx = pGbPro->pbTxBuf + SG_LOC_DA1;
					pGbPro->wTxPtr = SG_LOC_DA1;
				}
			} while (wReadNum < wTotal);//��ȡһ��PNID��10�ʼ�¼��ȡ�ʺ�Ҫ�������
		}	//end for (i=0; i<bFnPnNum; i++) = false;	

		if (!fAllPn || (fAllPn && wReadPn>=POINT_NUM))
		{
			tDaDt[bDaDtNum].bErr = fFnPnOkOnce ? SG_ERR_OK : SG_ERR_NOVALIDDATA;
			bDaDtNum++;
			if (bDaDtNum >= SG_DADT_GRP_SZ)
			{
				DTRACE(DB_FAPROTO, ("GbRxCallClass1: ##### err! DaDtNum>GB_DADT_GRP_SZ!\r\n"));
				break;
			}
		}

		if(fAllPn) //�ж�һ�º����Ƿ񻹴�����Ч�Ĳ�����
		{
			*pbRx = 0xff;
			*(pbRx+1) = 0xff;

			wReadPn++;

			while(wReadPn<POINT_NUM)
			{
				if(IsPnValid(wReadPn))
					break;
				wReadPn++;
			}
			if(wReadPn>=POINT_NUM) //�Ѿ������һ����������
			{
				pbRx += (6+6+6);
				wReadPn = 0;
			}
		}
	}

	if (!fLastSend)
	{
		if (pbTx != &pGbPro->pbTxBuf[SG_LOC_DA1])
		{
			if (fFirst)
				pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
			else
				pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

			pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;

			fHaveOneFrm = true;
			pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;
			SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, bAFn, pbTx-pGbPro->pbTxBuf);
            SetLedCtrlMode(LED_ALARM, LED_MODE_OFF);   //�澯��
		}
		else
		{
			goto RxReadAlrData_err;
		}
	}

	if (fHaveOneFrm == false)
		goto RxReadAlrData_err; //һ֡��û�У���ȷ������

	return true;

RxReadAlrData_err:
	if (pGbPro->fRptState)	//�����ϱ� ���ϱ�����֡
		return true;
	else
		return SgAnsConfirm(pPro, tDaDt, bDaDtNum);

}

///////////////////////////////////////////
void TransFileInit()
{
    TRANSMIT_FILE_INFO tTransFInfo;
    memset(&tTransFInfo, 0x00, sizeof(tTransFInfo));
    memset(tTransFInfo.bFileFlg, 0xff, sizeof(tTransFInfo.bFileFlg));
    WriteUpdateProg((BYTE *)&tTransFInfo, sizeof(tTransFInfo), FADDR_F1_INFO);  
}

bool AnsTransFile(struct TPro* pPro, BYTE bFD)
{
    TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;

	if(0xFF != bFD)//FD3
	{
		pGbPro->pbTxBuf[pGbPro->wTxPtr] = bFD;
		pGbPro->wTxPtr += 1;

		SgMakeTxFrm(pPro, false, SG_LFUN_CONFIRM, 0x0F, pGbPro->wTxPtr);
	}
	return true;
}
//�������ļ�����
//������
int SgRxCmd_TransFile(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	BYTE* pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	BYTE* pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];
	BYTE *pbTx;
    BYTE bNum = 0;
	WORD wHead;
	WORD wTail;
	WORD wNum = 0;
    DWORD dwoffest;
	WORD wFileDataLen, wCrc;
	DWORD dwRxSubsection, dwCheckSubsection;
	bool fFirst = true;
	TRANSMIT_FILE_INFO  tTransFInfo;
	TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
    BYTE i,j,k;
	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= 5;
	pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;
	memcpy(pbTx, pbRx, 6);//DA��ID
	pbTx += 6;
	pGbPro->wTxPtr += 6;
	if (SgGetRxSeq(pGbPro) == false)
		return 0x04; //�޴�������
	bNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);
	if(bNum <= 0)
		return 0x04;//DA ID ��Ч �޴�������
	pbRx += 6;
	ReadUpdateProg((BYTE *)&tTransFInfo, sizeof(tTransFInfo), FADDR_F1_INFO);

	for(i=0; i<bNum; i++)//ͨ�����bNum=1
	{
		if(tFnPn[i].bPn != 0)
			return 0x04;//�޴�������
		StopMtrRd(2);//ֹͣ����2s
		switch(tFnPn[i].dwId)
		{
		case 0xE3010001:
			{
				DTRACE(DB_FAPROTO, ("GbPro:: SgRxCmd_TransFile: Get a NewFile!!!\r\n"));
				memset(&tTransFInfo, 0x00, sizeof(tTransFInfo));
				WriteUpdateProg((BYTE *)&tTransFInfo, sizeof(tTransFInfo), FADDR_F1_INFO);
				tTransFInfo.wFileProp = ByteToWord(pbRx);
				pbRx += 2;
				tTransFInfo.wFileTotalSec = ByteToWord(pbRx);
				pbRx += 2;
				tTransFInfo.dwFileTotalLen = ByteToDWord(pbRx);
				pbRx += 4;
				memcpy(tTransFInfo.chFileName, pbRx, 32);
#ifdef SYS_WIN
				//memset(chszFName, 0, sizeof(chszFName));
				//sprintf(chszFName, "%s%s", USER_DATA_PATH, g_TransmitFileInfo.chFileName);
				//DeleteFile(chszFName);
#endif
				pbRx += 32;
				tTransFInfo.wFileCRC = ByteToWord(pbRx);
				pbRx += 2;

				WriteUpdateProg((BYTE *)&tTransFInfo, sizeof(tTransFInfo), FADDR_F1_INFO);
			}
			break;
		case 0xE3010002:
			tTransFInfo.wFileCurSec = ByteToWord(pbRx);
			pbRx += 2;
			wFileDataLen = ByteToWord(pbRx);
			if(tTransFInfo.wFileCurSecLen==0)
				tTransFInfo.wFileCurSecLen = wFileDataLen;

			pbRx += 2;
			wCrc = get_crc_16(0, pbRx, wFileDataLen);
			if(wCrc != ByteToWord(pbRx+wFileDataLen))
			{
				DTRACE(DB_FAPROTO, ("GbPro::SgRxCmd_TransFile.Rcv FD2 Sec_CRC Err.\r\n"));
				return 0x07;//CRCУ��ʧ��
			}
			
			if(tTransFInfo.wFileCurSec < tTransFInfo.wFileTotalSec)
			{

				DTRACE(DB_FAPROTO, ("CFaProto::Rx_TransFile.Rcv FD2 Sec_CRC Right.\r\n"));
				j = tTransFInfo.wFileCurSec&7;//ȡ����ʾ��֡��ĳ�ֽڵ�bit
				k = tTransFInfo.wFileCurSec>>3;//ȡ�����ʾ��֡�ڱ�־λ�ĵڼ��ֽ�
				dwoffest = tTransFInfo.wFileCurSecLen*tTransFInfo.wFileCurSec;
				if(WriteUpdateProg(pbRx, tTransFInfo.wFileCurSecLen, dwoffest))
				{
					tTransFInfo.bFileFlg[k] |=  0x01<<j;	
				}
				else
				{
					tTransFInfo.bFileFlg[k] &=  ~(0x01<<j);
					WriteUpdateProg((BYTE *)&tTransFInfo, sizeof(tTransFInfo), FADDR_F1_INFO);
					return 0x06;//Ŀ���ַ������
				}

			}
			else
			{
				DTRACE(DB_FAPROTO, ("CFaProto::Rx_TransFile.Rcv FD2 Sec_Num Err.\r\n"));
				return 0x06;//Ŀ���ַ������
			}

			WriteUpdateProg((BYTE *)&tTransFInfo, sizeof(tTransFInfo), FADDR_F1_INFO);
			if((tTransFInfo.wFileCurSec+1) == tTransFInfo.wFileTotalSec)
			{
				if(UpdataFile(&tTransFInfo) > 0)//ʵ�ֺ���
				{
					DTRACE(DB_FAPROTO, ("CFaProto::RxCmd_TransFile:UpdFile OK !!!\r\n"));
#ifndef SYS_WIN
					g_dwExtCmdClick = GetClick();
					g_dwExtCmdFlg = FLG_APP_RST;
#endif
				}
			}

			break;
		case 0xE3010003:
			WordToByte(tTransFInfo.wFileProp, pbTx);
			pbTx += 2;
			WordToByte(tTransFInfo.wFileTotalSec, pbTx);
			pbTx += 2;
			DWordToByte(tTransFInfo.dwFileTotalLen, pbTx);
			pbTx += 4;
			memcpy(pbTx, tTransFInfo.chFileName, 32);
			pbTx += 32;
			WordToByte(tTransFInfo.wFileCRC, pbTx);
			pbTx += 2;
			wHead = ByteToWord(pbRx+42);
			wTail = ByteToWord(pbRx+44);
			if(wTail > tTransFInfo.dwFileTotalLen)
				wTail = tTransFInfo.dwFileTotalLen;
			//memcpy(pbTx, pbRx+42, 4);//��ѯ����ʼ�����κ�
			if(IsAllAByte((BYTE *)&tTransFInfo, 0x00, sizeof(TRANSMIT_FILE_INFO))
				|| wHead>wTail)
			{
				memset(pbTx, 0x00, 6);
				pbTx += 6;
				pGbPro->wTxPtr += 48;//pbTx ָ���ƶ��� 2��2��4��32��2��6 ��48
			}
			else
			{
				//pbTx += 4;
				pGbPro->wTxPtr += 42;//pbTx ָ���ƶ��� 2��2��4��32��2��4 ��46

				memcpy(pbTx, pbRx+42, 2);//��ѯ����ʼ�κ�
				pbTx += 2;
				pGbPro->wTxPtr += 2;

				for(;wHead<=wTail; wHead++)
				{
					j = wHead&7;//ȡ����ʾ��֡��ĳ�ֽڵ�bit
					k = wHead>>3;//ȡ�����ʾ��֡�ڱ�־λ�ĵڼ��ֽ�
					if( !(tTransFInfo.bFileFlg[k] & (0x01<<j)) )
					{
						WordToByte(wHead, pbTx+4+wNum*2);
						pGbPro->wTxPtr += 2;
						wNum++;


						if( ((pbTx+4+wNum*2)-&pGbPro->pbTxBuf[SG_LOC_DA1]) > pProIf->wMaxFrmBytes-40)
						{
							if(fFirst)
							{
								fFirst = false;
								pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR|pGbPro->bHisSEQ;
							}
							else
							{
								pGbPro->pbTxBuf[ SG_LOC_SEQ ] = pGbPro->bHisSEQ;
							}
							WordToByte(wHead, pbTx);
							pGbPro->wTxPtr += 2;
							WordToByte(wNum, pbTx+2);
							pGbPro->wTxPtr += 2;

							pGbPro->bHisSEQ = (pGbPro->bHisSEQ + 1) & SG_SEQ_SEQ;
							SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_TRANSFILE, pGbPro->wTxPtr);


							pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
							pGbPro->wTxPtr = SG_LOC_DA1;
							memcpy(pbTx, pbRx-6, 6);//DA��ID
							pbTx += 6;
							pGbPro->wTxPtr += 6;
							WordToByte(tTransFInfo.wFileProp, pbTx);
							pbTx += 2;
							WordToByte(tTransFInfo.wFileTotalSec, pbTx);
							pbTx += 2;
							DWordToByte(tTransFInfo.dwFileTotalLen, pbTx);
							pbTx += 4;
							memcpy(pbTx, tTransFInfo.chFileName, 32);
							pbTx += 32;
							WordToByte(tTransFInfo.wFileCRC, pbTx);
							pbTx += 2;
							pGbPro->wTxPtr += 42;

							wNum = 0;

							WordToByte(wHead+1, pbTx);
							pbTx += 2;
							pGbPro->wTxPtr += 2;
						}
					}

				}
				WordToByte(wTail, pbTx);
				pGbPro->wTxPtr += 2;
				WordToByte(wNum, pbTx+2);
				pGbPro->wTxPtr += 2;
			}


			if ( fFirst )
			{
				fFirst = false;
				pGbPro->pbTxBuf[ SG_LOC_SEQ ] = SG_SEQ_FIR|pGbPro->bHisSEQ;
			}
			else
				pGbPro->pbTxBuf[ SG_LOC_SEQ ] = pGbPro->bHisSEQ;

			pGbPro->pbTxBuf[ SG_LOC_SEQ ] |= SG_SEQ_FIN;
			pGbPro->bHisSEQ = ( pGbPro->bHisSEQ + 1 ) & SG_SEQ_SEQ;

			SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_TRANSFILE, pGbPro->wTxPtr);

			return 0xFF;//FD3 ��֡û��Err����
		default:
			return 0x04;
		}
	}	
	return 0;           
}
 int UpdataFile(TRANSMIT_FILE_INFO* tTransFInfo)
 {
	 BYTE pbBuf[256];
	 WORD wRcrc = 0;
	 WORD i;
	 for(i=0; i<tTransFInfo->dwFileTotalLen>>8; i++)
	 {
		 ReadUpdateProg(pbBuf, 256, i<<8);
		 //ExFlashRdDataNoChk(0x17+i*256, NULL, pbBuf, 256);
		 wRcrc = get_crc_16(wRcrc, pbBuf, 256);
	 }

	 if(tTransFInfo->dwFileTotalLen%256 > 0)
	 {
		 ReadUpdateProg(pbBuf, tTransFInfo->dwFileTotalLen%256, tTransFInfo->dwFileTotalLen-tTransFInfo->dwFileTotalLen%256);
		 //dwoffest = g_TransmitFileInfo.dwFileTotalLenthg-g_TransmitFileInfo.dwFileTotalLenthg%256;
		 //ExFlashRdDataNoChk(0x17+dwoffest, NULL, pbBuf, g_TransmitFileInfo.dwFileTotalLenthg%256);
		 wRcrc = get_crc_16(wRcrc, pbBuf, tTransFInfo->dwFileTotalLen%256);
	 }
	 if(wRcrc != tTransFInfo->wFileCRC)
	 {
		 DTRACE(DB_FAPROTO, ("GBRxCmd_TransFile : CRC Check Error\n"));
		 DTRACE(DB_FAPROTO, ("GBRxCmd_TransFile : Recv CRC = %x\n",tTransFInfo->wFileCRC));
		 DTRACE(DB_FAPROTO, ("GBRxCmd_TransFile : My CRC = %x\n",wRcrc));
		 return -1;
	 }
	 DTRACE(DB_FAPROTO, ("GBRxCmd_TransFile : CRC Check OK\n"));
	 WriteUpdProgCrc(tTransFInfo->chFileName,tTransFInfo->dwFileTotalLen, wRcrc);
	 ExFlashRd(0, pbBuf, 16);//��仰��֪��ʲô��˼���Ǵ�Sftp����������һֱ������
	 return 1;

 }
//����:��[����ת��:F1͸��ת��]�Ĵ���
//����:@pbTxBuf ֱ��ָ���͵����ݵ�Ԫ
//����:@pbRxBuf ֱ��ָ����յ����ݵ�Ԫ
int SgMtrFwdFrmCmd(BYTE *pbTxBuf,BYTE *pbRxBuf)
{
	WORD wTxLen,wRxLen,wTimeOut;
	WORD wErrCnt = 30;
	bool fBegin = false;
	int iPort;
	BYTE i,bComSet, bParity;
	TCommPara CommPara;
	BYTE bBuf[300];
	DWORD dwTmpClick, dwLen;
	BYTE bPort=0;

	//CommPara.wPort = *(pbTxBuf+1)+1;

	bPort = *(pbTxBuf+1);
	iPort = MeterPortToPhy(*(pbTxBuf+1));
	if (iPort < 0)
		return -1;

	CommPara.wPort = (WORD)iPort;
	CommPara.dwBaudRate = ValToBaudrate(*(pbTxBuf+2));
	CommPara.bParity = ValToParity(*(pbTxBuf+3));
	CommPara.bByteSize = ValToByteSize(*(pbTxBuf+4));
	CommPara.bStopBits = ValToStopBits(*(pbTxBuf+5));

	wTimeOut = *(pbTxBuf+6);
	wTxLen = *(pbTxBuf+7);

	GetDirRdCtrl(bPort);	//ȡ��ֱ���Ŀ���Ȩ
	if ( !MtrProOpenComm(&CommPara) )
	{
		ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
		return -1;
	}

	CommRead(CommPara.wPort, NULL, 0, 300); //�����һ�´���

	if (CommWrite(CommPara.wPort, &pbTxBuf[8], wTxLen, 1000) != wTxLen)
	{
		DTRACE(DB_FAPROTO, ("GbMtrFwdFrmCmd: first fail to write comm.\r\n")); 
		if (CommWrite(CommPara.wPort, &pbTxBuf[8], wTxLen, 1000) != wTxLen)
		{
			DTRACE(DB_FAPROTO, ("GbMtrFwdFrmCmd: second fail to write comm.\r\n")); 
			ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
			return -1;
		}
	}

//#ifdef SYS_WIN	
	if ( IsDebugOn(DB_FAPROTO) )
		TraceBuf(DB_FAFRM, "GbMtrFwdFrmCmd: Mtr Fwd Frm-->", &pbTxBuf[8], wTxLen);
//#endif	

	i = 0;
	wRxLen = 0;
	dwTmpClick = GetClick();	
	pbRxBuf[0] = pbTxBuf[0];
	while (GetClick()-dwTmpClick < wTimeOut)    //n�γ��Զ�ȡ����
	{
		dwLen = CommRead(CommPara.wPort, bBuf, sizeof(bBuf), 300);

		if ((wRxLen+dwLen) >= 300)
		{
			DTRACE(DB_FAPROTO, ("GbMtrFwdFrmCmd: CommRead Buffer not enough!\r\n"));
			break;
		}
		else
			memcpy(pbRxBuf+2+wRxLen, bBuf, dwLen);

		if (dwLen > 0)
		{
		    i = 0;
			wRxLen += (WORD)dwLen;
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

			if ((fBegin && i>wErrCnt) || (!fBegin && (GetClick()-dwTmpClick>200)))
				break;
		}					
	}
	ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ

	*(pbRxBuf+1) = (BYTE)wRxLen;
//#ifdef SYS_WIN	
	if ( IsDebugOn(DB_FAPROTO) )
		TraceBuf(DB_FAFRM, "GbMtrFwdFrmCmd: Mtr Fwd Frm<--", &pbRxBuf[2], wRxLen);
//#endif	
	return wRxLen+2;
}

//����:��[����ת��:F9ת����վֱ�ӶԵ��ĳ�����������]�Ĵ���
//����:@pbTxBuf ֱ��ָ���͵����ݵ�Ԫ
//����:@pbRxBuf ֱ��ָ����յ����ݵ�Ԫ
int SgMtrFwdDirIDCmd(BYTE *pbTxBuf,BYTE *pbRxBuf)
{
	BYTE* pbRx;
	int iDataLen, iPort;
	bool fBegin = false;
	DWORD dwID;
	WORD wErrCnt = 30;
	BYTE i, bFwdDepth, bType, bMtrPro, bFrmLen;
	WORD wRetLen = 0;
	BYTE bAddr[6];
	TCommPara CommPara;
	BYTE bCmdFrm[50];
	BYTE bRetFrm[256];
	DWORD dwTmpClick, dwLen;

	BYTE bLogPort = pbTxBuf[0];
	iPort = MeterPortToPhy(pbTxBuf[0]);
	if (iPort < 0)
		return -1;
	pbTxBuf++;
	bFwdDepth = *pbTxBuf++;			//�м̼���
	pbTxBuf += (DWORD )bFwdDepth * 6;	//Ŀǰ���Ե�1~n��ת���м̵�ַ
	memcpy(bAddr, pbTxBuf, 6);		//ת��Ŀ���ַ
	pbTxBuf += 6;
	bType = *pbTxBuf++;				//ֱ�ӳ��������ݱ�ʶ����
	bMtrPro = bType==0 ? CCT_MTRPRO_97 : CCT_MTRPRO_07;
	dwID = ByteToDWord(pbTxBuf);
	bFrmLen = Make645AskItemFrm(bMtrPro, bAddr, dwID, bCmdFrm);

	CommPara.wPort = (WORD)iPort;
	wRetLen = MtrAddrToPn(bAddr); //ͨ�����ַת��Ϊ�������
	ReadItemEx(BN0, wRetLen, 0x8902, bRetFrm);
	CommPara.dwBaudRate = GbValToBaudrate(bRetFrm[MTR_BAUD_OFFSET]>>5);
	if (CommPara.dwBaudRate==0 || wRetLen==0) //��������0��δ���ò����㰴Ĭ��ת��
	{
		if (bMtrPro == CCT_MTRPRO_97)
			CommPara.dwBaudRate = CBR_1200;
		else
			CommPara.dwBaudRate = CBR_2400;	
	}
	CommPara.bParity =  EVENPARITY;	
	CommPara.bByteSize = 8;
	CommPara.bStopBits = ONESTOPBIT;	

	GetDirRdCtrl(bLogPort);	//ȡ��ֱ���Ŀ���Ȩ
	if ( !MtrProOpenComm(&CommPara) )
	{
		ReleaseDirRdCtrl(bLogPort); //�ͷ�ֱ���Ŀ���Ȩ
		return -1;
	}

	CommRead(CommPara.wPort, NULL, 0, 300); //�����һ�´���

	if (CommWrite(CommPara.wPort, bCmdFrm, bFrmLen, 1000) != bFrmLen)
	{
		DTRACE(DB_FAPROTO, ("GbMtrFwdDirIDCmd: first fail to write comm.\r\n")); 
		if (CommWrite(CommPara.wPort, bCmdFrm, bFrmLen, 1000) != bFrmLen)
		{
			DTRACE(DB_FAPROTO, ("GbMtrFwdDirIDCmd: second fail to write comm.\r\n"));
			ReleaseDirRdCtrl(bLogPort); //�ͷ�ֱ���Ŀ���Ȩ
			return -1;
		}
	}

//#ifdef SYS_WIN	
	if ( IsDebugOn(DB_FAPROTO) )
		TraceBuf(DB_FAPROTO, "GbMtrFwdDirIDCmd: Mtr fwd ID frm->", bCmdFrm, bFrmLen);
//#endif	

	i = 0;
	wRetLen = 0;
	dwTmpClick = GetTick();
	while (GetTick()-dwTmpClick < 10000)    //n�γ��Զ�ȡ����
	{
		dwLen = CommRead(CommPara.wPort, bRetFrm+wRetLen, 256-wRetLen, 200);

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
	ReleaseDirRdCtrl(bLogPort); //�ͷ�ֱ���Ŀ���Ȩ

	if (wRetLen == 0)
		return -1;

//#ifdef SYS_WIN	
	if ( IsDebugOn(DB_FAPROTO) )
		TraceBuf(DB_FAFRM, "GbMtrFwdDirIDCmd: Mtr fwd ID frm<--", bRetFrm, wRetLen);
//#endif	

	//��Ӧ��֡
	pbRx = pbRxBuf;
	*pbRx++ = bLogPort;			//�ն�ͨ�Ŷ˿ں�
	memcpy(pbRx, bAddr, 6);	//ת��Ŀ���ַ
	pbRx += 6;

	iDataLen = 0;
	if (wRetLen > 0)
	{
		for (i=0; i<wRetLen; i++)
		{
			if (bRetFrm[i] == 0x68)
				break;
		}
		if (i!=0 && i<wRetLen)
			memcpy(bRetFrm, &bRetFrm[i], wRetLen-i); //ȥ��ǰ���ַ�
		if (bMtrPro==CCT_MTRPRO_07 && (bRetFrm[8]==0x91 || bRetFrm[8]==0xb1))
		{
			//bIdLen = 4;
			iDataLen = bRetFrm[9];

			iDataLen -= 4;
			if (iDataLen > 0)
				memcpy(bCmdFrm, &bRetFrm[14], iDataLen);
		}
		else if (bMtrPro==CCT_MTRPRO_97 && (bRetFrm[8]==0x81 || bRetFrm[8]==0xa1))
		{
			//bIdLen = 2;
			iDataLen = bRetFrm[9];
		
			iDataLen -= 2;
			if (iDataLen > 0)
				memcpy(bCmdFrm, &bRetFrm[12], iDataLen);
		}
	}

	if (iDataLen > 0)
	{
		*pbRx++ = 5;			//ת�������־:ת����������
		*pbRx++ = (BYTE )(iDataLen + 4);	//ת��ֱ�ӳ��������������ֽ���k+4
		memcpy(pbRx, (BYTE*)&dwID, 4);
		pbRx += 4;
		for (i=0; i<iDataLen; i++)
		{
			pbRx[i] = bCmdFrm[i] - 0x33;
		}
		//memcpy(pbRx, bCmdFrm, iDataLen);
		pbRx += iDataLen;
	}
	else
	{
		*pbRx++ = 1;			//ת�������־:ת�����ճ�ʱ
		*pbRx++ = 4;			//ת��ֱ�ӳ��������������ֽ���k+4
		memcpy(pbRx, (BYTE*)&dwID, 4);
		pbRx += 4;
	}

	return iDataLen+13;
}

static WORD iMakeForwordFrm(BYTE *pbTxBuf,BYTE *pbRxBuf)//20131226-3
{
	int i;
	BYTE bLenth = 8;
	BYTE bLenthC = 11;
	BYTE *bRxBuf = pbRxBuf+8;

	memcpy(pbRxBuf,pbTxBuf,8);
	*bRxBuf++ = 0xD1;
	*bRxBuf++ = 0x01;
	*bRxBuf++ = 0x01;
	*bRxBuf = 0;
	for (i = 0; i<bLenthC; i++)
	{
		*bRxBuf += pbRxBuf[i];
	}
	*bRxBuf++;
	*bRxBuf = 0x16;

	return (bRxBuf-pbRxBuf+1);
}

DWORD ValToBaudrate(BYTE val)
{
	switch (val)
	{
	case 1:
		return CBR_300;
	case 2:
		return CBR_600;
	case 4:   
		return CBR_1200;
	case 8:   
		return CBR_2400;
	case 16:
		return CBR_4800;
	case 32:
		return CBR_9600;
		//		case 48:
		//			return CBR_14400;
	case 64:
		return CBR_19200;
	case 128:
		return CBR_38400;
	case 255:
		return CBR_115200;
	default:
		return CBR_1200;
	}

	return CBR_1200;
}

BYTE BaudrateToVal(DWORD val)
{
	switch (val)
	{
	case CBR_300:
		return 1;
	case CBR_600:
		return 2;
	case CBR_1200:   
		return 4;
	case CBR_2400:   
		return 8;
	case CBR_4800:
		return 16;
	case CBR_9600:
		return 32;
		//		case 48:
		//			return CBR_14400;
	case CBR_19200:
		return 64;
	case CBR_38400:
		return 128;
	case CBR_115200:
		return 255;
	default:
		return 4;
	}

	return 4;
}

BYTE ValToParity(BYTE val)
{
	static BYTE bParityTab[] = {NOPARITY, EVENPARITY,ODDPARITY}; 

	if (val < 3)
		return bParityTab[val];
	else	
		return EVENPARITY;
}

BYTE ParityToVal(BYTE val)
{
	switch(val)
	{
	case NOPARITY:
		return 0;
	case EVENPARITY:
		return 1;
	case ODDPARITY:
		return 2;
	default:
		return 1;
	}
}

BYTE ValToStopBits(BYTE val)
{
	static BYTE bStopBitsTab[] = {ONESTOPBIT, TWOSTOPBITS, TWOSTOPBITS};
	if (val < 3)
		return bStopBitsTab[val];
	else
		return ONESTOPBIT;
}

BYTE StopBitsToVal(BYTE val)
{
	switch(val)
	{
	case ONESTOPBIT:
		return 0;
	case TWOSTOPBITS:
		return 1;
	default:
		return 0;
	}
}

BYTE ValToByteSize(BYTE val)
{
	if (val>=5 && val<=8)
		return val;
	else
		return 8;
}

BYTE ByteSizeToVal(BYTE val)
{
	if (val>=5 && val<=8)
		return val;
	else
		return 8;
}

int SgRxMtrFwdCmd(struct TPro* pPro)
{
#define BAUD_RATE_CH_VAL 300
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	BYTE* pbRx = &pSgPro->pbRxBuf[SG_LOC_DA1];
	int		iRet;
	TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
	TSgDaDt tDaDt[SG_DADT_GRP_SZ];
	BYTE 	*pTail,*p,*pTxBuf,seq,*pMtrFwdBuf;
	BYTE 	*pTrans;
	WORD 	i,nFrm, wNum, wMtrFwdLen;
	DWORD	dwSendClick;
	BYTE	bForwardType;
	BYTE	bDaDtNum=0;
	int iCodeBufLen;
	BYTE bRetLen = 0;
	
	TMtrFwdMsg tFwdMsg;
	BYTE bRxFrm[MTRFWD_BUFSIZE];
	BYTE bFwFrm[MTRFWD_BUFSIZE];
	BYTE bSaveDI1,bSaveDI2,bSaveDI3,bSaveDI4;
	WORD wLen = 0;
	WORD wMaxBytes = 512;
	BYTE bPort=0;
	pMtrFwdBuf = bFwFrm;

	p = pSgPro->pbRxBuf + SG_LOC_DATA;
	pTail = pSgPro->pbRxBuf + pSgPro->wRxFrmLen - 2;

	bSaveDI1 = pSgPro->pbRxBuf[SG_LOC_DI1];
	bSaveDI2 = pSgPro->pbRxBuf[SG_LOC_DI2];
	bSaveDI3 = pSgPro->pbRxBuf[SG_LOC_DI3];
	bSaveDI4 = pSgPro->pbRxBuf[SG_LOC_DI4];

	PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ);
	if (pTail <= p)		//�쳣
	{
		SgAnsConfirm(pPro, tDaDt, 1);

		return -100;	
	}

	if ( !SgGetRxSeq(pSgPro) )
	{
		SgAnsConfirm(pPro, tDaDt, 1);
		return -1;
	}

	if (p[0] > 4)		//�м����ʹ���04  ��Ч
	{
		SgAnsConfirm(pPro, tDaDt, 1);
		return -11;
	}

	if(p[1] > 0x20 )	//������˿ںŴ���0x20,��û�ж�Ӧͨ��
	{
		SgAnsConfirm(pPro, tDaDt, 1);
		return p[1];
	}

	wNum = pSgPro->pbRxBuf[SG_FORWARD_CMD_LEN];

	if (wNum > SG_MTRFWDSIZE)
	{
		SgAnsConfirm(pPro, tDaDt, 1);
		DTRACE(DB_FAPROTO, ("SgRxMtrFwdCmd: data forward wNum=%ld, too large!\n", wNum));
		return 0;		//����
	}

	seq = pSgPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_SEQ;
	if (pSgPro->pbRxBuf[SG_LOC_SEQ] & SG_SEQ_FIR)
	{
		pSgPro->bHisSEQ = seq;
		memcpy(pMtrFwdBuf, p, 7);
		wMtrFwdLen = 7;
		memcpy(pMtrFwdBuf+wMtrFwdLen, p+FWR_DATA_OFFSET, wNum);	
		wMtrFwdLen += wNum;
	}
	else 
	{
		if (seq == ((pSgPro->bHisSEQ+1)&0xf))
		{
			pSgPro->bHisSEQ = seq;
			if (wMtrFwdLen + wNum > SG_MTRFWDSIZE)
			{
				SgAnsConfirm(pPro, tDaDt, 1);
				return 0;//����
			}
			memcpy(pMtrFwdBuf+wMtrFwdLen, p+FWR_DATA_OFFSET, wNum);
			wMtrFwdLen += wNum;
		}
	}

	if (!(pSgPro->pbRxBuf[SG_LOC_SEQ] & SG_SEQ_FIN))
	{
		SgAnsConfirm(pPro, tDaDt, 1);
		return 0;
	}

	if (wMtrFwdLen <= 7)
	{
		SgAnsConfirm(pPro, tDaDt, 1);
		return -11;
	}

	//�����յ����ݷ���������ȥ
	memset(&tFwdMsg, 0, sizeof(tFwdMsg));
	tFwdMsg.pbRxBuf = bRxFrm;
	tFwdMsg.wRxBufSize = MTRFWD_BUFSIZE;
	tFwdMsg.wTxLen = wMtrFwdLen - 7;

	bForwardType = pMtrFwdBuf[0];

	if ( 0x1f == pMtrFwdBuf[1] || 0x20 == pMtrFwdBuf[1])
	{
		tFwdMsg.CommPara.wPort = 0x1f;
	}
	else
		tFwdMsg.CommPara.wPort = pMtrFwdBuf[1]+1;
	tFwdMsg.CommPara.dwBaudRate = ValToBaudrate(pMtrFwdBuf[2]);
	tFwdMsg.CommPara.bParity = ValToParity(pMtrFwdBuf[3]);
	tFwdMsg.CommPara.bByteSize = ValToByteSize(pMtrFwdBuf[4]);
	tFwdMsg.CommPara.bStopBits = ValToStopBits(pMtrFwdBuf[5]);

	tFwdMsg.wFrmTimeout = pMtrFwdBuf[6] * 1000;
	tFwdMsg.wByteTimeout = 2*10;

	dwSendClick = GetClick();
	tFwdMsg.dwClick = dwSendClick;
	if (tFwdMsg.wTxLen > MTRFWD_BUFSIZE)
	{
		DTRACE(DB_FAPROTO, ("SgRxMtrFwdCmd: tFwdMsg.wTxLen=%ld, TrsMsg Buf not enough, discard %d Bytes!\n", tFwdMsg.wTxLen, tFwdMsg.wTxLen-MTRFWD_BUFSIZE));
		tFwdMsg.wTxLen = MTRFWD_BUFSIZE;
	}

	DTRACE(DB_FAPROTO, ("SgRxMtrFwdCmd: wPort=%d, dwBaudRate=%ld, bParity=%d, bByteSize=%d, bStopBits=%d, wFrmTimeout=%d\n",
		tFwdMsg.CommPara.wPort, tFwdMsg.CommPara.dwBaudRate, 
		tFwdMsg.CommPara.bParity, tFwdMsg.CommPara.bByteSize, 
		tFwdMsg.CommPara.bStopBits, tFwdMsg.wFrmTimeout));

	if(!(pMtrFwdBuf[1] < 0x1f) || IsAcqLogicPort(pMtrFwdBuf[1]))								//�ز�ͨ�����м�(������)
	{
		bPort = pMtrFwdBuf[1];
		GetDirRdCtrl(bPort);	//ȡ��ֱ���Ŀ���Ȩ
		CctDirectTransmit645Cmd(pMtrFwdBuf+7, tFwdMsg.wTxLen, tFwdMsg.pbRxBuf, &bRetLen, 0);
		ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
		iCodeBufLen = bRetLen;

		*pMtrFwdBuf = *p;
		*(pMtrFwdBuf+1) = iCodeBufLen;
		iCodeBufLen += 2;
		//�ӽ��ն�����ȡ���ش������
		memcpy(pMtrFwdBuf+2, tFwdMsg.pbRxBuf, iCodeBufLen);
	}
	else									//485�˿��м�
	{
		TraceBuf(DB_FAPROTO, "Mtr fwd frm->", pMtrFwdBuf+7, tFwdMsg.wTxLen);

		iCodeBufLen = 0;
		iCodeBufLen = SgMtrFwdFrmCmd(p, tFwdMsg.pbRxBuf);

		if (iCodeBufLen <= 0)
		{
			BYTE bModeTest = 0;
			ReadItemEx(BN1,PN0,0x2113,&bModeTest);	//20131227-3
			if(0x01 == bModeTest)			//����ģʽ��D1����
			{
				tFwdMsg.wRxLen = iMakeForwordFrm(p+8, tFwdMsg.pbRxBuf);//20131226-3
			}
			else
			{
				SgAnsConfirm(pPro, tDaDt, 1);
				return -13;
			}
		}

		//�ӽ��ն�����ȡ���ش������
		memcpy(pMtrFwdBuf, tFwdMsg.pbRxBuf, iCodeBufLen);
	}

	//��֡���ͳ�ȥ
	
	if (!iCodeBufLen)
		nFrm = 1;
	else
		nFrm = (iCodeBufLen + wMaxBytes - 1) / wMaxBytes;
	for (i=0,pTrans=pMtrFwdBuf; i<nFrm; i++,pTrans+=wMaxBytes)
	{
		pTxBuf = pSgPro->pbTxBuf + SG_LOC_DA1;

		*pTxBuf++ = 0;	//֡��ʽ�涨DA = 0;
		*pTxBuf++ = 0;
		*pTxBuf++ = bSaveDI1;
		*pTxBuf++ = bSaveDI2;
		*pTxBuf++ = bSaveDI3;
		*pTxBuf++ = bSaveDI4;

		if (i == nFrm - 1) 
			wLen = (!iCodeBufLen)? 0 : iCodeBufLen - (nFrm-1)*wMaxBytes;
		else 
			wLen = wMaxBytes;

		memcpy(pTxBuf, pTrans, wLen);
		pTxBuf += wLen;

		pSgPro->pbTxBuf[SG_LOC_SEQ] = pSgPro->bHisSEQ;
		pSgPro->bHisSEQ = (pSgPro->bHisSEQ + 1) & SG_SEQ_SEQ;
		if (!i) 
			pSgPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIR;
		if (i == nFrm - 1)
			pSgPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;

		pSgPro->wTxPtr = (WORD)(pTxBuf - pSgPro->pbTxBuf);
		SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_MTRFWD, pSgPro->wTxPtr);
		Sleep(2000);
	}

	return 0;
}

bool ReadTask(struct TPro* pPro)
{
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	int nRead, nRead1;
	BYTE bSEQ = (pSgPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_SEQ)|SG_SEQ_FIR;
	
	DWORD dwRecCnt = 0;
	DWORD dwRecsFound = 0;
	WORD wFrmLen;
	int iFrmSize;
	
	DWORD dwNextRecCnt = 0;
	BYTE bTestRxBuf[128];	//FOR TEST
	BYTE bTestTxBuf[128];	//FOR TEST
	bool fReadData=false;
	TSgDaDt tDaDt[1];
	BYTE bDaDtNum = 0;
	int iRet, i=0;
	DWORD dwLastClick=0;

	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	WORD wPIFMaxFrmBytes =pProIf->wMaxFrmBytes; //�ӿ�һ֡�����������

	if (pSgPro->fRptState)
		bSEQ |= SG_SEQ_CON|SG_SEQ_FIR|SG_SEQ_FIN;

	if (SgGetRxSeq(pSgPro) == false)
		return false;

	while (1)
	{
		nRead = ReadTaskData(&pSgPro->pbRxBuf[SG_LOC_DA1], &pSgPro->pbTxBuf[SG_LOC_DA1], &dwRecCnt, (int)(wPIFMaxFrmBytes-22), true);
		if (nRead <= 0)   //��������¼��
		{
			if (fReadData == false)   //һֱ��û����
			{
				tDaDt[0].bErr = SG_RCQ_FRM_NONE;
				bDaDtNum = 1;

				if (!pSgPro->fRptState)
					SgAnsConfirm(pPro, tDaDt, bDaDtNum);
			}
			return true;
		}
		else
		{
			fReadData = true;
			dwNextRecCnt = dwRecCnt;

			if (dwRecCnt==0)
				nRead1 = -2;
			else
			{
				memset(bTestRxBuf, 0, sizeof(bTestRxBuf));
				memset(bTestTxBuf, 0, sizeof(bTestTxBuf));
				memcpy(bTestRxBuf, pSgPro->pbRxBuf, pSgPro->wRxFrmLen);
				nRead1 = ReadTaskData(&bTestRxBuf[SG_LOC_DA1], &bTestTxBuf[SG_LOC_DA1], &dwNextRecCnt, (int)(wPIFMaxFrmBytes-22), false);//�鿴��һ�����Ƿ���Ч����֤����ʱ�н���֡
				if (nRead1 <= 0)
					nRead1 = -2;
			}
			
		}

		if (nRead1 <= 0)
			bSEQ |= SG_SEQ_FIN;

REPORT_TASKDATA_FRM:
		pSgPro->pbTxBuf[SG_LOC_SEQ] = bSEQ;
		pSgPro->wTxPtr = nRead+SG_LOC_DATA;
		if (!(
			 ((ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) >= 0xE0000301) && (ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) <= 0xE00003FE)) ||
			 ((ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) >= 0xE0000401) && (ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4) <= 0xE00004FE))
			))
		{
			DTRACE(DB_FAPROTO, ("ReadTask auotosend id=%x wrong, exit!\r\n", ByteToDWORD(&pSgPro->pbTxBuf[SG_LOC_DI1], 4)));
			pSgPro->fRptState = false;									//�澯������������״̬������false
			return false;
		}

		if (pSgPro->fRptState)
			pSgPro->bRxFrmFlg &= ~FRM_CONFIRM;	//���ϱ�ǰ������ȷ��֡��־λ

		SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_ASKTASK, pSgPro->wTxPtr);
		if (pSgPro->fRptState)
		{
			pSgPro->fRptState = false;									//��������״̬������false
			i++;
			dwLastClick = GetClick();

			do
			{
				iRet = pPro->pfnRcvFrm(pPro);
				if (iRet > 0)
				{
					/*if ((pSgPro->pbRxBuf[SG_LOC_AFN]==SG_AFUN_CONFIRM) && 
						((bSEQ&0x0f) == (pSgPro->pbRxBuf[SG_LOC_SEQ]&0x0f)))
					{
						i = 3;
						break;
					}*/
					if (pSgPro->bRxFrmFlg & FRM_CONFIRM)	//���յ�ȷ��֡
					{
						DTRACE(DB_FAPROTO, ("ReadTask : rx confirm.\n"));
						i = 3;
						break;
					}
				}
			}while (GetClick()-dwLastClick < 20);	//20��ȴ�ȷ��ʱ��

			if (i<3)								//δ�յ�ȷ�ϣ������ϱ�����
			{
				pSgPro->fRptState = true;									//��������״̬������false
				goto REPORT_TASKDATA_FRM;
			}
		}

		if(!(bSEQ&SG_SEQ_FIN))
		{
			bSEQ++;
			bSEQ &= SG_SEQ_SEQ;
		}

		if (nRead1 < 0)  //û�к���֡
			return true;
		else
			Sleep(3000);  //�к���֡,����ǰ���֡�ȷ��ͳ�ȥ

		//TODO������ʧ��
	}

	return true;
}

bool UserDefData(struct TPro* pPro)
{
	TSgDaDt tDaDt[SG_DADT_GRP_SZ];
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	WORD FpnGrpNum;
	BYTE* pbTx;
	BYTE bDaDtNum = 0;
	BYTE *pbRx = &pSgPro->pbRxBuf[SG_LOC_DA1];
	BYTE *pbRxTail = &pSgPro->pbRxBuf[pSgPro->wRxFrmLen - 2];
	struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	WORD wPIFMaxFrmBytes =pProIf->wMaxFrmBytes; //�ӿ�һ֡�����������

	memset(tDaDt, 0, sizeof(tDaDt));
	if ((pSgPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= 5;

	//pbRxTail -= 16;	//password
	PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ);

	if (pbRxTail <= pbRx) //�쳣
	{
		PutReturnValue(tDaDt, 4);
		return SgAnsConfirm(pPro, tDaDt, 1);
	}

	if (SgGetRxSeq(pSgPro) == false)//�ж�֡������
	{
		PutReturnValue(tDaDt, 4);
		return SgAnsConfirm(pPro, tDaDt, 1);
	}

	pbTx = pSgPro->pbTxBuf + SG_LOC_DA1;
	pSgPro->wTxPtr = SG_LOC_DA1;

	if(pbRx < pbRxTail)
	{
		if (!PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ))//�����ܽ��մ���DIDA�������
		{
			PutReturnValue(tDaDt, 4);
			return SgAnsConfirm(pPro, tDaDt, 1);
		}

		FpnGrpNum = Rx_GetIDPN(pbRx);
		pbRx += 6;    //DA 2�ֽ� DI 4�ֽ��ܹ�6�ֽ�
		if (FpnGrpNum == 1 && m_RxIdPnGrp[0].wPN == 0 && m_RxIdPnGrp[0].dwID == 0xe0002000)
		{
			IdPnToBytes(m_RxIdPnGrp[0].dwID, m_RxIdPnGrp[0].wPN, pbTx); //��ID,PNת����ͨ��Э���ʽ��6���ֽ�
			pbTx += 6;
			pSgPro->wTxPtr += 6;

			UserDef(pPro);
		}
		else
		{
			PutReturnValue(tDaDt, 4);
			return SgAnsConfirm(pPro, tDaDt, 1);
		}
	}

	return true;
}

bool SgGetRxSeq(TSgPro* pGbPro)
{
	BYTE bRxSeq = pGbPro->pbRxBuf[SG_LOC_SEQ];

	//Ŀǰ����֧�ֵ��ֶ����󣬴����ļ��޴��ֽڣ�

	//��ʽ1��ȫ���ռ������Ժ󴫸�Ӧ�ò�
	if ((bRxSeq & SG_SEQ_FIR) && (bRxSeq & SG_SEQ_FIN))
	{ //if only one frame,not copy 	
		pGbPro->bHisSEQ = bRxSeq & SG_SEQ_SEQ;
		return true;
	}

	return true;	//���б�seq
}

//������ȡ����Ϣ��֤����
void SgGetAuthPara(BYTE* pbAuthType, WORD* pwAuthPara)
{
	BYTE bBuf[3];
	if (ReadItemEx(BN0, PN0, 0x005f, bBuf) >= 0)
	{
		*pbAuthType = bBuf[0];
		*pwAuthPara = bBuf[1] | ((WORD)bBuf[2]<<8);
	}
	else
	{
		*pbAuthType = 0xff;
		*pwAuthPara = 0x8967;
	}
}

int SgVeryPsw(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	DWORD dwPass;
	BYTE *pMsg;
	BYTE *pbRxTail;
	WORD wAuthPara;
	BYTE bAuthType; 
	BYTE	bPW[16];
	memset(bPW, 0, sizeof(bPW));

    //pbRx = pGbPro->pbRxBuf + SG_LOC_AFN;
	pbRxTail = pGbPro->pbRxBuf + pGbPro->wRxFrmLen - 2;

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1) 
		pbRxTail -= 6;

	SgGetAuthPara(&bAuthType, &wAuthPara);	//ȡ����Ϣ��֤����

	//�ж�
	dwPass = SgMakeDirPsw(pGbPro);
	
	if (pGbPro->fLocal)
	{	
		if (dwPass == 0x8967)
			return 1;
	}

	switch (bAuthType)
	{
		case 0:	
			return 1;
		case 1:
			pMsg = pGbPro->pbRxBuf + SG_LOC_CONTROL;
			if (dwPass == SgMakeCrcPsw(pMsg))
				return 1;
			break;
		case 255:
			{
		#ifdef EN_ESAM
                int iRv = -1;				
    			if ((iRv=VeryMacPwd(pPro)) <= 0) //��֤ʧ��
    			{
    				//��������¼
    				memcpy(bPW, (BYTE*)&dwPass, sizeof(dwPass));
    				//PushEvt_PassErr(bPW);
    				return iRv;
    			}			
    			else 
    				return 1;
		#else
				if (dwPass == wAuthPara)
					return 1;
                break;
		#endif				
			}
		default:		
			return 1; //break;//ȱʡΪ���봦��
		
	}

	//��������¼
	memcpy(bPW, (BYTE*)&dwPass, sizeof(dwPass));
	SgSaveAlrPswErr(bPW, pGbPro->pbRxBuf[SG_LOC_ADDB3]>>1);
	return -1;
}

DWORD SgMakeDirPsw(TSgPro* pGbPro)
{
	BYTE *pTail = pGbPro->pbRxBuf + pGbPro->wRxFrmLen - 2;
	BYTE bPLen;
	DWORD dwPass;

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pTail -= 6;

	bPLen = 0;
	ReadItemEx(BN10, PN0, 0xa165, &bPLen);
	if (bPLen==0 || bPLen==0xff || bPLen==INVALID_DATA)
	{
		bPLen = 2;	//password
	}
	pTail -= bPLen;	//password

#ifdef EN_ESAM
	dwPass = ((DWORD)pTail[3]<<24) | ((DWORD)pTail[2]<<16) | ((DWORD)pTail[1]<<8) | pTail[0];	
#else
	dwPass = ((WORD)pTail[1]<<8) | pTail[0];	
#endif
	return dwPass;
}

WORD SgMakeCrcPsw(BYTE *pdata)
{
	int j, k;
	WORD wRet = 0;
	WORD wAuthPara;
	BYTE bAuthType, Cl, Ch; 
	BYTE crc16Lo = 0x00;
	BYTE crc16Hi = 0x00;
	BYTE len = 12;

	SgGetAuthPara(&bAuthType, &wAuthPara);	//ȡ����Ϣ��֤����
	Cl = wAuthPara&0xFF;
	Ch = (wAuthPara&0xFF00)>>8;
	
	for (j = 0; j<len; j++) 
	{ 
		crc16Lo ^= pdata[j];
		for (k = 0; k < 8; k++)
		{
			BYTE save16Hi = crc16Hi; 
			BYTE save16Lo = crc16Lo;
			crc16Lo = crc16Lo>>1; 
			crc16Hi = crc16Hi>>1; 
			if (save16Hi & 0x01)
			{ 
				crc16Lo = crc16Lo | 0x80;
			}
			if (save16Lo & 0x01) 
			{ 
				crc16Hi ^= Ch;
				crc16Lo ^= Cl;
			}
		}
 	}
    wRet = (crc16Hi<<8)+crc16Lo;
    return wRet;
}


bool SgCheckTVP(TSgPro* pGbPro)
{
	BYTE *p, bDelay;
	DWORD dwNow, dwRxTagSec;
	TTime tmRxTag;

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP0) 
		return true;

	p = pGbPro->pbRxBuf + pGbPro->wRxFrmLen - 8;
	bDelay = p[4];
	if (bDelay == 0) 
		return true;

	GetCurTime(&tmRxTag);
	tmRxTag.nSecond = BcdToByte(*p++);
	tmRxTag.nMinute = BcdToByte(*p++);
	tmRxTag.nHour = BcdToByte(*p++);
	tmRxTag.nDay = BcdToByte(*p++);

	dwNow = GetCurSec();
	dwRxTagSec = TimeToSeconds(&tmRxTag);

	//���������еĶ�ʱ����TVP���
	//if (pGbPro->pbRxBuf[SG_LOC_AFN]==SG_AFUN_CONTROLCMD && 
	//	pGbPro->pbRxBuf[SG_LOC_DI1]==0x40 && pGbPro->pbRxBuf[SG_LOC_DI2]==0x03)
	//{
	//	return true;
	//}

	//�б�ʱ��
	if (dwNow > dwRxTagSec+(WORD )bDelay*60 || 
		dwNow < dwRxTagSec-(WORD )bDelay*60) 
		return false;
	else 
		return true;
}

bool GetACD(TSgPro* pGbPro)
{
	return (ReadEC1() != pGbPro->bEC1) ? true : false;
}

WORD GetLeftBufSize(WORD wTxPtr)
{
	return (GB_MAXDATASIZE >= (wTxPtr+4)) ? GB_MAXDATASIZE - (wTxPtr+4) : 0;
}

bool SgAnsConfirm(struct TPro* pPro, TSgDaDt* pDaDt, BYTE bDaDtNum)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx;
	DWORD dwRxB1B2;
	BYTE i, nTrue, nFalse, bReturn;

	//get bReturn
	dwRxB1B2 = ByteToDWORD(pGbPro->pbRxBuf + SG_LOC_ADDB1, 3);
	if (dwRxB1B2==SGADDR_BROADCAST && bDaDtNum!=0 && bDaDtNum!=0xff)	//�������վ�������������������ǹ㲥��ַ��
		return 0;


	nTrue = nFalse = 0;
	if (bDaDtNum == 0xff)
	{
		bReturn = AFN00_ALLOK;	//����ǵ�½������֡����û�����ݲ���
	}
	else
	{
		for (i=0; i<bDaDtNum; i++)
		{
			if (pDaDt[i].bErr == SG_ERR_OK)
				nTrue++;
			else
				nFalse++;
		}

		if (!bDaDtNum)
			bReturn = AFN00_ALLERR;
		else if (nTrue == bDaDtNum)
			bReturn = AFN00_ALLOK;
		else if (nFalse == bDaDtNum)
			bReturn = AFN00_ALLERR;
		else
			bReturn = AFN00_EVERYITEM; 
	}	

	//make frame
	pbTx = pGbPro->pbTxBuf + SG_LOC_SEQ;
	
	*pbTx++ = SG_SEQ_FIR | SG_SEQ_FIN | pGbPro->bHisSEQ;

	///PN
	*pbTx++ = 0;
	*pbTx++ = 0;

	//ID+����
	*pbTx++ = 0;
	*pbTx++ = 0;
	*pbTx++ = 0;
	*pbTx++ = 0xE0;
	*pbTx++ = (bReturn==AFN00_ALLOK? 0 : 1);

	if (bReturn == AFN00_EVERYITEM)
	{
		*pbTx++ = pGbPro->pbRxBuf[SG_LOC_AFN];
		for (i=0; i<bDaDtNum; i++)
		{
			*pbTx++ = pDaDt[i].bDA1;
			*pbTx++ = pDaDt[i].bDA2;
			*pbTx++ = 0;
			*pbTx++ = 0;
			*pbTx++ = 0;
			*pbTx++ = 0xE0;
			*pbTx++ = (pDaDt[i].bErr==SG_ERR_OK) ? 0 : 1;
		}
	}

	SgMakeTxFrm(pPro, false, SG_LFUN_CONFIRM, SG_AFUN_CONFIRM, (WORD)(pbTx-pGbPro->pbTxBuf));
	return true;
}

//����:�鷢��֡
WORD SgMakeTxFrm(struct TPro* pPro, bool fPRM, BYTE bCmd, BYTE bAFN, WORD wTxPtr)
{
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = pSgPro->pbTxBuf + wTxPtr;
	WORD wDLen, wDLen1;//,wRxA1A2,wRxB1B2,wOneGrpAddr; 
	BYTE bRS232Mode = 0, bPortFun = PORT_FUN_DEBUG;

	//if (!pSgPro->fLocal)	//�����ͨѶ�̹߳���Զ��ͨ��
	//	WaitSemaphore(g_semRemoteIf, SYS_TO_INFINITE);

#ifndef SYS_WIN
	if (!pSgPro->fLocal)	//Զ��ͨ��
	{
		DWORD dwClick = GetClick();
		while (pSgPro->fRptState && GetMemAvaible()<2048)
		{
			Sleep(100);
			if (GetClick() - dwClick > 60)
			{
				DTRACE(DB_FAPROTO, ("SgMakeTxFrm : GetMemAvaible()=%ld Memey Not Enough!", GetMemAvaible()));
				return 0;
			}
		}
	}
#endif

	//PRM��ACD
	if (fPRM || pSgPro->fRptState)	//�����ϱ�״̬
	{
		bCmd |= SG_CTL_PRM;
	}
	else
	{
		//Get ACD
		//if ( GetACD(pGbPro) )
		//{
		//	bCmd |= SG_CTL_ACD;
		//	*pbTx++ = ReadEC1(); //EC1
		//	*pbTx++ = ReadEC2(); //EC2
		//}
	}
	
	//�Ƿ���Ҫʱ���ǩ
	if (bAFN!=SG_AFUN_CHECKLINK && ((pSgPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1))
	{
		pSgPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_TVP;
		memcpy(pbTx, pSgPro->pbRxBuf + pSgPro->wRxFrmLen - 8, 5);
   	    pbTx = pbTx + 5;
	}

	wTxPtr = (WORD)(pbTx - pSgPro->pbTxBuf);
	wDLen = wTxPtr - 6;


	pSgPro->pbTxBuf[SG_LOC_START1] = pSgPro->pbTxBuf[SG_LOC_START2] = 0x68;
	pSgPro->pbTxBuf[SG_LOC_LLEN1] = pSgPro->pbTxBuf[SG_LOC_LLEN2] = wDLen & 0xff;
	pSgPro->pbTxBuf[SG_LOC_HLEN1] = pSgPro->pbTxBuf[SG_LOC_HLEN2] = wDLen>> 8;
	
	pSgPro->pbTxBuf[SG_LOC_CONTROL] = bCmd | SG_CTL_DIR;
//////////////////////////
	pSgPro->pbTxBuf[SG_LOC_ADDA1] = pSgPro->dwAddr1 & 0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDA2] = (pSgPro->dwAddr1 >> 8)&0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDA3] = (pSgPro->dwAddr1 >> 16)&0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDB1] = pSgPro->dwAddr2 & 0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDB2] = (pSgPro->dwAddr2 >> 8)&0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDB3] = (pSgPro->dwAddr2 >> 16)&0xff;
////////////////////////////

	if (fPRM || pSgPro->fRptState)
		pSgPro->pbTxBuf[SG_LOC_ADDB4] = 0;
	else
		pSgPro->pbTxBuf[SG_LOC_ADDB4] = pSgPro->pbRxBuf[SG_LOC_ADDB4];
	pSgPro->pbTxBuf[SG_LOC_AFN] = bAFN;
	pSgPro->pbTxBuf[wTxPtr++] = CheckSum(pSgPro->pbTxBuf+SG_LOC_CONTROL, wDLen);
	pSgPro->pbTxBuf[wTxPtr++] = 0x16;

	pPro->pfnSend(pPro, pSgPro->pbTxBuf, wTxPtr);
	//if (!pSgPro->fLocal)
	//{
	//	ReadItemEx(BN2, PN0, 0x2110, &bRS232Mode);
	//	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
	//	if (bRS232Mode==1 && bPortFun==PORT_FUN_DEBUG)
	//		CommWrite(COMM_DEBUG, pSgPro->pbTxBuf, wTxPtr, 500); //9600������ÿ����Դ�ӡ800�ֽ�
	//	SignalSemaphore(g_semRemoteIf);
	//}

	return wTxPtr;
}

//����:����վ��������֡
WORD SgMakeMasterReqFrm(struct TPro* pPro, bool fPRM, BYTE bCmd, BYTE bAFN, WORD wTxPtr)
{
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = pSgPro->pbTxBuf + wTxPtr;
	WORD wDLen, wDLen1;//,wRxA1A2,wRxB1B2,wOneGrpAddr; 
	BYTE bRS232Mode = 0, bPortFun = PORT_FUN_DEBUG;

	//if (!pSgPro->fLocal)	//�����ͨѶ�̹߳���Զ��ͨ��
	//	WaitSemaphore(g_semRemoteIf, SYS_TO_INFINITE);

	//PRM��ACD
	if (fPRM || pSgPro->fRptState)	//�����ϱ�״̬
	{
		bCmd |= SG_CTL_PRM;
	}
	else
	{
		//Get ACD
		//if ( GetACD(pGbPro) )
		//{
		//	bCmd |= SG_CTL_ACD;
		//	*pbTx++ = ReadEC1(); //EC1
		//	*pbTx++ = ReadEC2(); //EC2
		//}
	}
	
	//�Ƿ���Ҫʱ���ǩ
	if (bAFN!=SG_AFUN_CHECKLINK && ((pSgPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1))
	{
		pSgPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_TVP;
		memcpy(pbTx, pSgPro->pbRxBuf + pSgPro->wRxFrmLen - 8, 5);
		pbTx = pbTx + 5;
	}
	pSgPro->pbTxBuf[SG_LOC_SEQ] |= (pSgPro->bMySEQ & 0x0f);

	wTxPtr = (WORD)(pbTx - pSgPro->pbTxBuf);
	wDLen = wTxPtr - 6;


	pSgPro->pbTxBuf[SG_LOC_START1] = pSgPro->pbTxBuf[SG_LOC_START2] = 0x68;
	pSgPro->pbTxBuf[SG_LOC_LLEN1] = pSgPro->pbTxBuf[SG_LOC_LLEN2] = wDLen & 0xff;
	pSgPro->pbTxBuf[SG_LOC_HLEN1] = pSgPro->pbTxBuf[SG_LOC_HLEN2] = wDLen>> 8;
	
	pSgPro->pbTxBuf[SG_LOC_CONTROL] = bCmd;
//////////////////////////
	pSgPro->pbTxBuf[SG_LOC_ADDA1] = pSgPro->dwAddr1 & 0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDA2] = (pSgPro->dwAddr1 >> 8)&0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDA3] = (pSgPro->dwAddr1 >> 16)&0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDB1] = pSgPro->dwAddr2 & 0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDB2] = (pSgPro->dwAddr2 >> 8)&0xff;
	pSgPro->pbTxBuf[SG_LOC_ADDB3] = (pSgPro->dwAddr2 >> 16)&0xff;
////////////////////////////

	if (fPRM || pSgPro->fRptState)
		pSgPro->pbTxBuf[SG_LOC_ADDB4] = 0;
	else
		pSgPro->pbTxBuf[SG_LOC_ADDB4] = pSgPro->pbRxBuf[SG_LOC_ADDB4];
	pSgPro->pbTxBuf[SG_LOC_AFN] = bAFN;
	pSgPro->pbTxBuf[wTxPtr++] = CheckSum(pSgPro->pbTxBuf+SG_LOC_CONTROL, wDLen);
	pSgPro->pbTxBuf[wTxPtr++] = 0x16;

	//pPro->pfnSend(pPro, pSgPro->pbTxBuf, wTxPtr);
	//if (!pSgPro->fLocal)
	//{
	//	ReadItemEx(BN2, PN0, 0x2110, &bRS232Mode);
	//	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
	//	if (bRS232Mode==1 && bPortFun==PORT_FUN_DEBUG)
	//		CommWrite(COMM_DEBUG, pSgPro->pbTxBuf, wTxPtr, 500); //9600������ÿ����Դ�ӡ800�ֽ�
	//	SignalSemaphore(g_semRemoteIf);
	//}

	return wTxPtr;
}

int GbMakeLoginFrm(struct TPro* pPro, BYTE bFlag)
{
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	BYTE* pbTx = &pSgPro->pbTxBuf[SG_LOC_SEQ];

	*pbTx++ = SG_SEQ_CON | SG_SEQ_FIN | SG_SEQ_FIR;
	//PN
	*pbTx++ = 0;
	*pbTx++ = 0;
	///////////ID///////////////////
	if(bFlag == SG_LNKTST_LOGIN)
	{	
		*pbTx++ = 0x00;		
	}
	if (bFlag == SG_LNKTST_BEAT)
	{
		*pbTx++ = 0x01;	
	}
	if(bFlag == SG_LNKTST_LOGOUT)
	{
		*pbTx++ = 0x02;
	}
	*pbTx++ = 0x10;
	*pbTx++ = 0x00;
	*pbTx++ = 0xe0;
	///////////////////////////////

	if(bFlag == SG_LNKTST_LOGIN)
	{
		//ID���� ��Լ�汾��
		*pbTx++ = 0x00;
		*pbTx++ = 0x01;
	}
	
	return SgMakeTxFrm(pPro, true, SG_LFUN_ASKLINKSTATUS, SG_AFUN_CHECKLINK, (WORD)(pbTx - pSgPro->pbTxBuf));
}


bool SgLogin(struct TPro* pPro)
{
	TSgPro* pSgPro = pPro->pvPro;
	DWORD dwOldClick;
	const BYTE bRetConfirmBuf[] = {0x00, 0x00, 0x00, 0xE0, 0x00};
    int iRet;

	pSgPro->bRxFrmFlg &= ~FRM_CONFIRM;	//�ڷ��͵�½֡ǰ����ȷ��֡��־λ

	GbMakeLoginFrm(pPro, SG_LNKTST_LOGIN); //ȷ��������֡�ǵ�½֡
	Sleep(160);

	dwOldClick = GetClick();
	DTRACE(DB_FAPROTO, ("GbLogin : at click %ld\n", dwOldClick));
	do
	{
		if (pSgPro->bRxFrmFlg & FRM_CONFIRM)	//���յ�ȷ��֡
		{
			DTRACE(DB_FAPROTO, ("GbLogin : login ok\n"));
			return true;
		}
        iRet = pPro->pfnRcvFrm(pPro);
		if (iRet > 0)
		{
			if ((pSgPro->pbRxBuf[SG_LOC_AFN]==SG_AFUN_CONFIRM && 
				memcmp(&pSgPro->pbRxBuf[SG_LOC_DI1], bRetConfirmBuf, sizeof(bRetConfirmBuf))==0)) //����ȫȷ��
			{
				DTRACE(DB_FAPROTO, ("GbLogin : login ok\n"));
				Sleep(1000);
				SgBeat(pPro); //ɽ����վ������������������ljx 13-09-28 
				return true;
			}
		}
        else if (iRet < 0) //��·������
            break;
		Sleep(100);
	} while (GetClick()-dwOldClick < 12);   //12����ط�

	DTRACE(DB_FAPROTO, ("GbLogin : login fail\n"));
	return false;
}

bool SgBeat(struct TPro* pPro)
{
	if (GbMakeLoginFrm(pPro, SG_LNKTST_BEAT) <= 0)
        return false;
	return true;
}

bool SgLogoff(struct TPro* pPro)
{
	TSgPro* pSgPro = (TSgPro* )pPro->pvPro;
	DWORD dwOldClick;
	const BYTE bRetConfirmBuf[] = {0x00, 0x00, 0x00, 0xE0, 0x00};
    int iRet;
	BYTE i;
    
	for(i=0; i<4; i++)
	{
		GbMakeLoginFrm(pPro, SG_LNKTST_LOGOUT); //ȷ��������֡�ǵ�½֡
		Sleep(160);

		dwOldClick = GetClick();
		DTRACE(DB_FAPROTO, ("GbLogoff : at click %ld\n", dwOldClick));
		do
		{
			iRet = pPro->pfnRcvFrm(pPro);
			if (iRet > 0)
			{
				if ((pSgPro->pbRxBuf[SG_LOC_AFN]==SG_AFUN_CONFIRM && 
					memcmp(&pSgPro->pbRxBuf[SG_LOC_DI1], bRetConfirmBuf, sizeof(bRetConfirmBuf))==0)) //����ȫȷ��
				{
					DTRACE(DB_FAPROTO, ("SgLogoff : Logoff ok\n"));
					return true;
				}
			}
			else if (iRet < 0)//��·������
				break;
			Sleep(100);
		} while (GetClick()-dwOldClick < 12);   //12����ط�

	}
	
	DTRACE(DB_FAPROTO, ("GbLogoff : Logoff fail\n"));
	return false;
}

void SgLoadUnrstPara(struct TPro* pPro)
{
}

//����:��һЩЭ����صķǱ�׼������
void SgDoProRelated(struct TPro* pPro)	
{
	//DoNoComuTimeout();
}

//����:��ӡ֡����
void SgPrintCmd(BYTE bCmd, bool fTx)
{
#ifdef SYS_WIN
	char* szGbCmdName[] = {
		"ȷ��֡","��λ����","��·�������","�м�վ����","���ò���",
		"��������","����6","����7","����8","����9",
		"��ѯ������4�����ݣ�","�������ݲ�ѯ����","����1������","����2������","����3������",
		"�ļ�����","����ת��"}; 
#else
	char* szGbCmdName[] = {
		"confirm frame","reset command","link status checking","relay command","set parameter command",
		"control command","unuse cmd6","unuse cmd7","unuse cmd8","unuse cmd9",
		"call class4(parameters) command","call task data command","call class1 data command","call class2 data command","call class3 data command",
		"transmit file command","transmit data command"}; 
#endif
	if (bCmd > 0x10)
	{
		if (fTx)
			DTRACE(DB_FAPROTO, ("GbPrintCmd : tx a Extent Function frame.\r\n"));
		else
			DTRACE(DB_FAPROTO, ("GbPrintCmd : rx a Extent Function frame.\r\n"));
		return;
	}

	if (fTx)
        DTRACE(DB_FAPROTO, ("GbPrintCmd : tx a %s frame.\r\n", szGbCmdName[bCmd]));
    else
        DTRACE(DB_FAPROTO, ("GbPrintCmd : rx a %s frame.\r\n", szGbCmdName[bCmd]));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////���⴦��1�� ������Э����ʵ�ֵ�3���¼� ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SgSaveAlrParaChg(TSgDaDt* pDaDt, BYTE bDaDtNum, BYTE bRxMstAddr)
{
	BYTE* p;
	TTime t;
	BYTE i;
	BYTE bErcData[100];

	//p = bErcData;
	//*p++ = bRxMstAddr;	
	//for (i=0; i<bDaDtNum; i++)
	//{
	//	*p++ = pDaDt[i].bDA1;
	//	*p++ = pDaDt[i].bDA2;
	//	*p++ = pDaDt[i].bDI1;
	//	*p++ = pDaDt[i].bDI2;
	//}
	//GetCurTime(&t);
	//SaveAlrData(ERC_PARACHG, t, bErcData, (int)(p-bErcData), 0);
}

void SgSaveAlrPswErr(BYTE* pbPass, BYTE bRxMstAddr)
{
	TTime t;
	BYTE bErcData[17];

	memcpy(bErcData, pbPass, 16);
	bErcData[16] = bRxMstAddr;
	GetCurTime(&t);
// 	SaveAlrData(ERC_AUTH, t, bErcData, 0, 0);
}



//�����Ƿ����
bool IsTxComplete(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	return GetACD(pGbPro);
}

//�Ƿ���ҪGPRS������½
bool SgIsNeedAutoSend(struct TPro* pPro)
{
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	BYTE bAdmitRpt;
	ReadItemEx(BN0, PN0, 0x103f, &bAdmitRpt);
	if ((bAdmitRpt&0x03) == 1)
		return GetACD(pGbPro);

	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////�����㽭�����չЭ��֡����/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ZJHandleFrm(BYTE* pbRxBuf, BYTE* pbTxBuf)
{
	int iResult = -1;
	WORD wRxDataLen = (WORD)pbRxBuf[LEN_TO_LEN+1]*0x100 
		                  + pbRxBuf[LEN_TO_LEN];

	switch (pbRxBuf[FAP_CMD] & FAP_CMD_GET)
	{
	case FAP_CMD_USERDEF:
		iResult = ZJUserDef(pbRxBuf, wRxDataLen, pbTxBuf);
		break;
		
	default:
		break;
	} //switch (pbRxBuf[FAP_CMD] & FAP_CMD_GET)

	return iResult;
}

int ZJUserDef(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
    int iResult; 
	if (pbRxBuf[LEN_TO_LEN+LEN_OF_LEN]!=0x3d || pbRxBuf[LEN_TO_LEN+LEN_OF_LEN+1]!=0x5a)
		return -1;

	switch (pbRxBuf[LEN_OF_CL+LEN_TO_LEN+LEN_OF_LEN])    //��չ������
	{
	case 0x01:
		return ZJReadDataEx(pbRxBuf, wRxDataLen, pbTxBuf, false);
		
	case 0x02:
		return ZJWriteDataEx(pbRxBuf, wRxDataLen, pbTxBuf, false);
		
	case 0x03:					//��ʽ��Ӳ��
		if (!PswCheck(DI_HIGH_PERM, &pbRxBuf[FAP_DATA_EX]))
			return ZJReExtCmd(ERR_PERM, pbRxBuf, pbTxBuf);
		
		g_dwExtCmdClick = GetClick();
		g_dwExtCmdFlg = FLG_FORMAT_DISK;
		return ZJReExtCmd(ERR_APP_OK, pbRxBuf, pbTxBuf);

	case 0x07:
		g_dwExtCmdClick = GetClick();
		g_dwExtCmdFlg = FLG_REMOTE_DOWN;
		g_PowerOffTmp.bRemoteDownIP[0] = pbRxBuf[FAP_DATA_EX+3]; 
		g_PowerOffTmp.bRemoteDownIP[1] = pbRxBuf[FAP_DATA_EX+4];
		g_PowerOffTmp.bRemoteDownIP[2] = pbRxBuf[FAP_DATA_EX+5];
		g_PowerOffTmp.bRemoteDownIP[3] = pbRxBuf[FAP_DATA_EX+6];			
		g_PowerOffTmp.bRemoteDownIP[4] = pbRxBuf[FAP_DATA_EX+7];
		g_PowerOffTmp.bRemoteDownIP[5] = pbRxBuf[FAP_DATA_EX+8];
		g_PowerOffTmp.bRemoteDownIP[6] = pbRxBuf[FAP_DATA_EX+9];
		g_PowerOffTmp.bRemoteDownIP[7] = pbRxBuf[FAP_DATA_EX+10];
		return ZJReExtCmd(ERR_APP_OK, pbRxBuf, pbTxBuf);
    
	case 0x0E:
		return ZJSftpDataEx(pbRxBuf,wRxDataLen, pbTxBuf);//����
		
	case 0x11:
		return ZJReadDataEx(pbRxBuf, wRxDataLen, pbTxBuf, true);
		
	case 0x12:
		return ZJWriteDataEx(pbRxBuf, wRxDataLen, pbTxBuf, true);
		
	case 0x13:
  		ZJRunCmd(pbRxBuf, wRxDataLen, pbTxBuf);
		return ZJReExtCmd(ERR_APP_OK, pbRxBuf, pbTxBuf);
	
	case 0x15:	//��������չ����
		if (!PswCheck(DI_HIGH_PERM, &pbRxBuf[FAP_DATA_EX]))
			return ZJReExtCmd(ERR_PERM, pbRxBuf, pbTxBuf);

	case 0x10:
		return ZJLoadParaFile(pbRxBuf,wRxDataLen, pbTxBuf);//����		
    case 0x20:      //����У׼
	#ifndef SYS_WIN
		iResult = OnCalibrate(&pbRxBuf[FAP_DATA_EX+3]); //3�ֽ�����
	#endif
		if (iResult >= 0)
            return ZJReExtCmd(ERR_APP_OK, pbRxBuf, pbTxBuf);
		else 
			return ZJReExtCmd(-iResult, pbRxBuf, pbTxBuf);
	case 0x21:
		return ZJDealTestCmd(pbRxBuf, wRxDataLen, pbTxBuf);//������ָ��
	}

	return -1;
}


//������ͨ��֡�������Ѿ��ź���pָ���ͨ�Ż��������������������������ͨ���ֶ�
//���أ�ͨ��֡�ĳ���
int ZJMakeFrm(WORD wDataLen, BYTE* pbRxBuf, BYTE* pbTxBuf, bool fErr)
{

	pbTxBuf[LEN_TO_LEN] = wDataLen & 0xff;
	pbTxBuf[LEN_TO_LEN+1] = (wDataLen >> 8) & 0xff;

	return wDataLen + FAP_FIX_LEN;
}	


int ZJReplyErr(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf)
{
	pbTxBuf[FAP_DATA] = bErrCode;
    return ZJMakeFrm(1, pbRxBuf, pbTxBuf, true);
}


int ZJReExtCmd(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf)
{
	BYTE* pbRx = &pbRxBuf[FAP_DATA];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];

	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //��չ������
	*pbTx++ = bErrCode;
	
    return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA), pbRxBuf, pbTxBuf, false);
}

int ZJReadDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn)
{
	int nRead;
	BYTE* pbRx = &pbRxBuf[FAP_DATA];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];
	WORD wPn;

	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //��չ������

	if (fWordPn)
	{
		wPn = ByteToWord(pbRx);
		*pbTx++ = *pbRx++;
		*pbTx++ = *pbRx++;
	}
	else
	{
		wPn = *pbRx;
		*pbTx++ = *pbRx++;
	}

	while (pbRx < &pbRxBuf[FAP_DATA+wRxDataLen])
	{
		BYTE bBank = *pbRx;
		WORD wID = ByteToWord(pbRx+1);

		*pbTx++ = *pbRx++;   //Bank
		*pbTx++ = *pbRx++;   //��������
		*pbTx++ = *pbRx++;
				   
        nRead = ReadItemEx(bBank, wPn, wID, pbTx);
		if (nRead < 0)
		{
			return ZJReplyErr(-nRead, pbRxBuf, pbTxBuf);
		}
		else   //����
		{
			pbTx += nRead;
		}
	}

	if (pbTx == &pbTxBuf[FAP_DATA+5])    //������û��������
	{
		return ZJReplyErr(ERR_ITEM, pbRxBuf, pbTxBuf);
	}

    return ZJMakeFrm((WORD)(pbTx-&pbTxBuf[FAP_DATA]), pbRxBuf, pbTxBuf, false);
}

//����:д��չ����
int ZJWriteDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn)
{
	int nWritten/*, nIdx*/;
	BYTE* pbRx = &pbRxBuf[FAP_DATA];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];
	WORD wPn;
	WORD wPswOffset;
	BYTE bPerm;

	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //��չ������

	if (fWordPn)
	{
		wPn = ByteToWord(pbRx);
		*pbTx++ = *pbRx++;
		*pbTx++ = *pbRx++;
		wPswOffset = 7;
	}
	else
	{
		wPn = *pbRx;
		*pbTx++ = *pbRx++;
		wPswOffset = 6;
	}

	bPerm = *pbRx++;
	pbRx += 3;   //��������

	
	while (pbRx < &pbRxBuf[FAP_DATA+wRxDataLen])
	{
		BYTE bBank = *pbRx;
		WORD wID = ByteToWord(pbRx+1);

		*pbTx++ = *pbRx++;   //Bank
		*pbTx++ = *pbRx++;   //��������
		*pbTx++ = *pbRx++;

		nWritten = WriteItemEx(bBank, wPn, wID, pbRx);	//bPerm, &pbRxBuf[FAP_DATA+wPswOffset]
		if (nWritten < 0)
		{
			nWritten = -nWritten;
			if (nWritten == ERR_ITEM)   //��ϵͳ��֧�ֵ���������ݳ��Ȳ�������������������û������
			{
				*pbTx++ = (BYTE )nWritten;  //���������ý��
				break;
			}
			else   //ϵͳ�е������ֻ��Ȩ�޵�ûͨ��
			{
				*pbTx++ = (BYTE )(nWritten & 0xff);  //���������ý��
				nWritten = nWritten >> 8;
			}
		}
		else   //���óɹ�
		{
			*pbTx++ = ERR_APP_OK;              //���������ý��
			//nIdx = GetModuleParaIndex(bBank, wID);
			//if (nIdx >= 0)
			//	SetSendParaFlag(&g_ProEx, nIdx);
		}

		pbRx += nWritten;   //��������
	}

    TrigerSavePara();
    return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA), pbRxBuf, pbTxBuf, false);
}

int ZJSftpDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	BYTE* pbRx;
	BYTE* pbTx;
	BYTE bSftpBuf[128];
	int i;
	pbRx = &pbRxBuf[FAP_CMD];
	pbTx = &pbTxBuf[FAP_CMD];
 	for(i=0; i<LEN_TO_LEN;i++)
 	{
 		*pbTx++ = *pbRx++;
 	}

//	memcpy(pbTx, pbRx, LEN_TO_LEN);

	pbRx += 2;   //��������
	pbTx += 2;   //��������

	*pbTx++ = *pbRx++;  //��½��ʶ��
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //��չ������
	pbRx += 3;    //����PW

	memset(bSftpBuf, 0, sizeof(bSftpBuf));

	if (SftpHandleFrm(pbRx, bSftpBuf))
	{
		memcpy(pbTx, bSftpBuf, g_tSftpInfo.m_wTxLen);
		return ZJMakeFrm((WORD)(pbTx-pbTxBuf+g_tSftpInfo.m_wTxLen), pbRxBuf, pbTxBuf, false);
	}
	else
	{
		return -1;
	}
}

int ZJLoadParaFile(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	BYTE* pbRx = &pbRxBuf[FAP_LEN];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];
    BYTE bRet = 0;
	/*char  szPathName[PATHNAME_LEN+1];
	BYTE bRet = 0;
	WORD wLen = 0;
	WORD wNameLen = 0;
	
	wLen = ByteToWord(pbRx);  */
	
	pbRx += 2;   
	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //��չ������

	/*pbRx += 3;   //��������
	
	wNameLen = wLen-FAP_DATA_EX;  	//�ļ�������
	
	if (wNameLen > PATHNAME_LEN)
	{
		return 2;
	}
	memcpy(szPathName, pbRx, wNameLen);
	bRet = g_pmParaMgr.LoadPara(szPathName);
	if (bRet == 0)
	{
		if (!g_pmParaMgr.Parse())
			bRet = -1;
		else
            TrigerSave();
	}
	
	memcpy(pbTx, &bRet, sizeof(bRet));
	return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA+sizeof(bRet)), pbRxBuf, pbTxBuf, false);*/

#ifndef SYS_WIN
    if (!ParaMgrParse())
        bRet = -1;
#endif

    memcpy(pbTx, &bRet, sizeof(bRet));
	return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA+sizeof(bRet)), pbRxBuf, pbTxBuf, false);
    
	//return -1;
}

int ZJDealTestCmd(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	BYTE* pbRx = &pbRxBuf[FAP_LEN];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];
	//BYTE bRet = 0;
	//WORD wLen = 0;
	//WORD wNameLen = 0;

	//wLen = ByteToWord(pbRx);  

	pbRx += 2;   
	*pbTx++ = *pbRx++;  //���̱��
	*pbTx++ = *pbRx++;  //��½ʶ����
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //��չ������

	pbRx += 3;   //��������

	switch (*pbRx)	//0x00 ��ʾ���Կ��Ź�
	{
	case 0x00:
		SetInfo(INFO_STOP_FEED_WDG);
		*pbTx++ = *pbRx++;	//��������
		*pbTx++ = *pbRx++;	//��λ�ĸ������ݳ���
		*pbTx++ = *pbRx++;
		break;
  	case 0x01:     //�Ʋ�������     
        g_fRxLedTestCmd = true;
   		*pbTx++ = *pbRx++;	//��������
		*pbTx++ = *pbRx++;	//��λ�ĸ������ݳ���
		*pbTx++ = *pbRx++;
   		break;
  	case 0x02:    //�˳��Ʋ�������
        g_fRxLedTestCmd = false;
   		*pbTx++ = *pbRx++;	//��������
		*pbTx++ = *pbRx++;	//��λ�ĸ������ݳ���
		*pbTx++ = *pbRx++;
   		break;    
	}

	return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA), pbRxBuf, pbTxBuf, false);
}

bool ZJRunCmd(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	return true;
}


#ifndef SYS_WIN
#if 0   //û��ESAM
//��������Կ��֤ �����Ƕ��FNPN��Ϣ��״��
int RxCmd_VerifyPwd(struct TPro* pPro)
{
    TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
    struct TProIf* pProIf = (struct TProIf* )pPro->pIf;
	int  	iRv = -1;
	WORD 	i, wLen=0, wRet=0;
	BYTE 	bPLen, nFailNum/*, bUpdType, bP1*/; 
	BYTE 	*pbRxTail,*pbRx,*pbTx;
	bool 	fFirst=true, fLastSend=false, fHasOneFrm=false;
	BYTE	bErrBuf[17];
    BYTE	bErr = 0;
    
    BYTE bFnPnNum, bDaDtNum = 0;
    TGbFnPn tFnPn[SG_FNPN_GRP_SZ];
    TSgDaDt tDaDt[SG_DADT_GRP_SZ];

    pbRx = &pGbPro->pbRxBuf[SG_LOC_DA1];
	pbRxTail = &pGbPro->pbRxBuf[pGbPro->wRxFrmLen - 2];
	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1)
		pbRxTail -= 6;

	bPLen = 0;
	ReadItemEx(BN10, PN0, 0xa165, &bPLen);
	if (bPLen==0 || bPLen==0xff || bPLen==INVALID_DATA)
		bPLen = 2;

	pbRxTail -= bPLen;	
	
	if (!GbGetRxSeq(pGbPro))
		goto RxAskCfg_err;

	nFailNum = 0;
	pbTx = &pGbPro->pbTxBuf[SG_LOC_DA1];
	pGbPro->wTxPtr = SG_LOC_DA1;

	while (pbRx < pbRxTail)
	{
		//get each FN/PN data cyclely
        PutToDaDt(bDaDtNum, pbRx, tDaDt, SG_DADT_GRP_SZ);
        bFnPnNum = GetFnPn(pbRx, tFnPn, SG_FNPN_GRP_SZ);
		pbRx += 4;
		if (!bFnPnNum)
			goto RxAskCfg_err;

		for (i=0; i<bFnPnNum; i++)
		{	
			if (tFnPn[i].bPn != PN0)
				return -1;

			wRet = 0;
			switch(tFnPn[i].bFn)
			{
			case 11: //��ȡ�ն���Ϣ 					
				iRv = EsamGetTermInfo(pbTx+4);
				if (iRv <= 0)
					goto RxAskCfg_err;

				wRet += iRv;
				break;	
		    case 12: //�Ự��ʼ��/�ָ�
				wLen = ByteToWord(pbRx);
				iRv = EsamInitSession(pbRx+2, wLen, pbTx+4+2);
				if (iRv <= 0)
					goto RxAskCfg_err;
				
				WordToByte((WORD )iRv, pbTx+4);
				wRet += iRv + 2;
				pbRx += (ByteToWord(pbRx)+2);
				break;
		    case 13://�ỰЭ��
				wLen = ByteToWord(pbRx);
				iRv = EsamNegotiateKey(pbRx+2, wLen, pbTx+4);
				if (iRv <= 0)
					goto RxAskCfg_err;

				wRet += iRv;
				pbRx += (ByteToWord(pbRx)+2);
				break;
			case 14://�Գ���Կ����
				//TODO:��¼��Կ�汾
				if (EsamUpdSymKey(*(pbRx+1), pbRx+2))
				{					//���صĹ�����ΪAFN00ȷ��/����
					bErr = SG_ERR_OK;
				}
				else
				{
					bErr = SG_ERR_PASSERR;
					bErrBuf[0] =  ESAM_ERR_KEY;
					memset(bErrBuf+1, 0xff, 16);
				}
				pbRx++; //��Կ�汾
				pbRx += ((*pbRx)*32+1);
				return AnsCmd_Reset(pPro, bErr, bErrBuf, 17);

			case 15://�ն�֤�����
				iRv = EsamUpdCert(pbTx+4);
				if (iRv <= 0)
					goto RxAskCfg_err;

				wRet += iRv;
				break;

			case 16://CA֤�����
				wLen = ByteToWord(pbRx);
				if (EsamUpdCA(pbRx+2, wLen))
				{					//���صĹ�����ΪAFN00ȷ��/����
					bErr = SG_ERR_OK;
				}
				else
				{
					bErr = SG_ERR_PASSERR;
					bErrBuf[0] =  ESAM_ERR_KEY;
					memset(bErrBuf+1, 0xff, 16);
				}
				pbRx += (ByteToWord(pbRx)+2);
				return AnsCmd_Reset(pPro, bErr, bErrBuf, 17);

				//break;
			case 17://�ڲ���֤
				iRv = EsamIntCert(pbRx, 16, pbTx+4);
				if (iRv <= 0)
					goto RxAskCfg_err;

				wRet += iRv;
				pbRx += 16;
				break;

			case 18://�ⲿ��֤
				iRv = EsamExtCert(pbRx, 16, pbTx+4);
				if (iRv <= 0)
					goto RxAskCfg_err;

				wRet += iRv;
				pbRx += 16;
				break;

			case 19://״̬�л�
				if (EsamSwitchState(*pbRx, pbRx+1, 20))
				{					//���صĹ�����ΪAFN00ȷ��/����
					bErr = SG_ERR_OK;
				}
				else
				{
					bErr = SG_ERR_PASSERR;
					bErrBuf[0] =  ESAM_ERR_KEY;
					memset(bErrBuf+1, 0xff, 16);
				}
				pbRx += 1+16+4;
				return AnsCmd_Reset(pPro, bErr, bErrBuf, 17);

			case 20://�����߼�����
				if (EsamSetOfflineCnt(pbRx, 20))
				{					//���صĹ�����ΪAFN00ȷ��/����
					bErr = SG_ERR_OK;
				}
				else
				{
					bErr = SG_ERR_PASSERR;
					bErrBuf[0] =  ESAM_ERR_KEY;
					memset(bErrBuf+1, 0xff, 16);
				}
				pbRx += 20;
				return AnsCmd_Reset(pPro, bErr, bErrBuf, 17);

			case 21://ת������Ȩ
				if (EsamTransEncrAuth(pbRx, 32))
				{					//���صĹ�����ΪAFN00ȷ��/����
					bErr = SG_ERR_OK;
					//ת������Ȩ�ɹ�����������
					WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
					memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
					
					g_ExFlashBuf[0] = 0x00;
					g_ExFlashBuf[2048] = 0x01;
					MakeFile(FILE_BATTASK_STATUS, g_ExFlashBuf);
					writefile(FILE_BATTASK_STATUS, 0, g_ExFlashBuf);
					
					memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
					MakeFile(FILE_BATTASK0, g_ExFlashBuf);
					writefile(FILE_BATTASK0, 0, g_ExFlashBuf);
					writefile(FILE_BATTASK0, 1, g_ExFlashBuf);
					
					memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
					MakeFile(FILE_BATTASK1, g_ExFlashBuf);
					writefile(FILE_BATTASK1, 0, g_ExFlashBuf);
					writefile(FILE_BATTASK1, 1, g_ExFlashBuf);
					memset(g_PowerOffTmp.bBatTaskInfo, 0x00, sizeof(g_PowerOffTmp.bBatTaskInfo));
					SignalSemaphore(g_semExFlashBuf);
				}
				else
				{
					bErr = SG_ERR_PASSERR;
					bErrBuf[0] =  ESAM_ERR_KEY;
					memset(bErrBuf+1, 0xff, 16);
				}
				pbRx += 32;
				return AnsCmd_Reset(pPro, bErr, bErrBuf, 17);

//----------------------------------------------------------------------------
			default: 			
				break;
			}		
		
			if (wRet == 0)
			{
				nFailNum++;
				continue;
			}

			//FnPnToBytes(tFnPn[i].bFn, tFnPn[i].bPn, pbTx); //��FN,PNת����ͨ��Э���ʽ��4���ֽ�			
			pbTx += wRet + 4;
			pGbPro->wTxPtr += wRet + 4; 

			if (pGbPro->wTxPtr >= (int )pProIf->wMaxFrmBytes) 
			{					//TODO:SG_MAXDATASIZEӦ������Ӧ����ӿڵ�������ֽ�
				if ( fFirst )
				{
					fFirst = false;
					pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
				}
				else
					pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

				if (pbRx>=pbRxTail && i+1==bFnPnNum)
				{
					fLastSend = true;
					pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;
				}
				pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;

				//����
				SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_VERIFYPWD, pGbPro->wTxPtr);

				//����
				fHasOneFrm = true;
				pbTx = pGbPro->pbTxBuf + SG_LOC_DA1;
				pGbPro->wTxPtr = SG_LOC_DA1;
			}
		}	//end for (i=0; i<bFnPnNum; i++)
		
        tDaDt[bDaDtNum].bErr = (nFailNum==bFnPnNum)? SG_ERR_PASSERR : SG_ERR_OK;
		bDaDtNum++;
	}	

	if (fLastSend == false)
	{
		if (pbTx != &pGbPro->pbTxBuf[SG_LOC_DA1])
		{
			if ( fFirst )
				pGbPro->pbTxBuf[SG_LOC_SEQ] = SG_SEQ_FIR | pGbPro->bHisSEQ;
			else
				pGbPro->pbTxBuf[SG_LOC_SEQ] = pGbPro->bHisSEQ;

			pGbPro->pbTxBuf[SG_LOC_SEQ] |= SG_SEQ_FIN;

			fHasOneFrm = true;
			pGbPro->bHisSEQ = (pGbPro->bHisSEQ+1) & SG_SEQ_SEQ;
			SgMakeTxFrm(pPro, false, SG_LFUN_DATAREPLY, SG_AFUN_VERIFYPWD, pGbPro->wTxPtr);
		}
		else
		{
			goto RxAskCfg_err;
		}
	}

	if (fHasOneFrm == false)
		goto RxAskCfg_err; //һ֡��û�У���ȷ������
	return 1;

RxAskCfg_err:
	return GbAnsConfirm(pPro, tDaDt, bDaDtNum);
}
#endif

#else
int RxCmd_VerifyPwd(struct TPro* pPro)
{
	return 0;
}
#endif

#ifndef SYS_WIN
int VeryMacPwd(struct TPro* pPro)
{
	int iRv = -1;
    BYTE *pbRx, *pbRxTail;
    TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
    pbRx = pGbPro->pbRxBuf + SG_LOC_AFN;
    pbRxTail = pGbPro->pbRxBuf + pGbPro->wRxFrmLen - 2;
    WORD wRxB1B2 = ByteToWord(pGbPro->pbRxBuf + SG_LOC_ADDB1);
	BYTE bRxA3 = pGbPro->pbRxBuf[SG_LOC_ADDB3] & 1;
	//BYTE bSeq = m_bRxBuf[SG_LOC_SEQ];
	WORD wRet = 0, wOneGrpAddr;
	BYTE mBuf[20], bGrpAddr[20];
	BYTE i;
	//DWORD dwTimeout = 5*1000;
	memset(mBuf, 0xff, 17);	

	if ((pGbPro->pbRxBuf[SG_LOC_SEQ]&SG_SEQ_TVP) == SG_SEQ_TVP1) //�˴������¼���ǩ�Ĵ���
		pbRxTail -= 6;

	if (bRxA3 != 0)		
	{	
		memset(bGrpAddr, 0, sizeof(bGrpAddr));
		ReadItemEx(BN0, PN0, 0x006f, bGrpAddr); //��ȡ���ַ
		if (wRxB1B2 == 0xffff) ///return 1;
		////////////////////////���ӹ㲥��ַ/////////////////////////////
		{
			wOneGrpAddr = wRxB1B2;
		}
		else
		{
			for (i=0; i<8; i++)
			{
				wOneGrpAddr = bGrpAddr[2*i] + (((WORD)bGrpAddr[2*i+1])<<8);			
				if (wOneGrpAddr!=0 && wRxB1B2==wOneGrpAddr)	
				{
					break;
				}		
			}
			if (i == 8) //���ַ����ȷ
				return -1;
		}
		//ESAMģ����㲥У��:����У����
		if (EsamGrpBroadcastVerifyMac(pGbPro->pbRxBuf[SG_LOC_AFN], wOneGrpAddr, pbRx, pbRxTail-pbRx-12, &mBuf[1]))
			return 1;
		else
		{	
			mBuf[wRet++] = ESAM_ERR_MAC; //MAC��֤ʧ��
							
			wRet += 16;		
			
			//��������¼
			//PushEvt_PassErr(pbRxTail-16);
			DTRACE(DB_FAPROTO, ("CFaProto::VerifyMac  bErrBuf[0]=%d\n",  mBuf[0]));
			//if(wRxB1B2 != 0xffff)
			//{
				//if (AnsCmd_Reset(pPro, SG_ERR_PASSERR, mBuf, wRet) > 0)
					return 0;
				//else
					//return -1;
			//}
			//else
			  //return 0;
		}
	}
	//ESAMģ��У��:����У���� //����:�������ݡ����ĳ��ȡ�MAC���ݡ���ʱʱ��
	iRv = EsamVerifyMac(pGbPro->pbRxBuf[SG_LOC_AFN], pbRx, pbRxTail-pbRx-16, pbRxTail-16, &mBuf[1]);			
	if (iRv > 0) //MACУ��ʧ��
	{	
		mBuf[wRet++] = ESAM_ERR_MAC; //MAC��֤ʧ��
		wRet += 16;		

		//��������¼
		//PushEvt_PassErr(pbRxTail-16);
		DTRACE(DB_FAPROTO, ("CFaProto::VerifyMac  bErrBuf[0]=%d\n",  mBuf[0]));
		if (AnsCmd_Reset(pPro, SG_ERR_PASSERR, mBuf, wRet) > 0)
			return 0;
		else
			return -1;
	}
	else if (iRv < 0)
		return -1;

	return 1;
}
#else
int VeryMacPwd(TSgPro* pGbPro)
{
	return 0;
}
#endif

//�������˺���Ϊ��λ�����ȷ�ϻ���ϻش���Ҳ���������ذ�ȫ��֤�ķ��ϻش��������֤ʧ�ܵ���Ϣ��
int AnsCmd_Reset(struct TPro* pPro, BYTE bErr, BYTE* pbData, BYTE bDataLen)
{
	//make frame
	BYTE *pTxBuf;
	TSgPro* pGbPro = (TSgPro* )pPro->pvPro;
	pGbPro->wTxPtr = SG_LOC_SEQ;    
	pTxBuf = pGbPro->pbTxBuf + pGbPro->wTxPtr;
	
	*pTxBuf++ = SG_SEQ_FIR|SG_SEQ_FIN|pGbPro->bHisSEQ;
	*pTxBuf++ = 0;
	*pTxBuf++ = 0;
	if (bErr == SG_ERR_PASSERR) //��֤ʧ��
		*pTxBuf++ = AFN00_EASMERR;//FN
	else
		*pTxBuf++ = (bErr==SG_ERR_OK)?AFN00_ALLOK:AFN00_ALLERR;//FN
	*pTxBuf++ = 0;

	if (bErr == SG_ERR_PASSERR) //��֤ʧ��,�������֤ʧ�ܵ���Ϣ
	{
		memcpy(pTxBuf, pbData, bDataLen);
		pTxBuf += bDataLen;
	}

	pGbPro->wTxPtr = (WORD)(pTxBuf - pGbPro->pbTxBuf);
	return SgMakeTxFrm(pPro, false, SG_LFUN_CONFIRM, SG_AFUN_CONFIRM, pGbPro->wTxPtr);
}

void PnToBytes(WORD wPn, BYTE *pbBuf)
{
	if (!wPn)
	{
		pbBuf[0] = 0;
		pbBuf[1] = 0;
	}
	else
	{
#ifdef PRO_698
		wPn--;
		pbBuf[0] = 1 << (wPn&7);
		pbBuf[1] = (wPn>>3) + 1;
#else
		wPn--;
		pbBuf[0] = 1 << ((BYTE)wPn&7);
		pbBuf[1] = 1 << ((BYTE)wPn>>3);
#endif
	}
}
