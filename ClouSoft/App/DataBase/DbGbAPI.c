/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbGbAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�����ݽṹ����
 * ��ǰ�汾��1.0
 * ��    �ߣ�κ��
 * ������ڣ�2007��4��
 *********************************************************************************************************/
#include "FaCfg.h"
#include "DbConst.h"
#include "FaConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "GbPro.h"
#include "DbGbAPI.h"
#include "DbStruct.h"
#include "DbFmt.h"
#include "CommonTask.h"
#include "SysDebug.h"
#include "FlashMgr.h"
#include "MtrCtrl.h"
#include "SysApi.h"
#include "TermnExc.h"
#include "MtrAPIEx.h"
#include "ExcTask.h"
#include "DbSgAPI.h"

extern TBankCtrl g_Bank0Ctrl[SECT_NUM];
extern TBankCtrl g_BankCtrl[BANK_NUM];

//��������ȡϵͳ������
//������@ bFn �������ݸ��ඨ�����Ϣ����
//      @ bPtr ��ȡָ��
//      @ pbBuf �����������
//      @ pwLen ���ݳ���ָ��
//���أ����ڵ���0��ʾ�ɹ���С��0��ʾʧ��
//��ע�����չ������ʵ�ʵĳ���,����ʹ��ԭЭ��ӿڲ��ø���
int SgReadClass3(DWORD dwId, WORD wPn, DWORD dwStartm, DWORD dwEndtm, BYTE* pbBuf, WORD* wRdNum, WORD wTotal, int iTxBufLeft)
{
	int iRet;
	WORD wAlrLen = 0;
	TTime tm;
	DWORD dwSec = 0;
	DWORD dwAddr = 0;
	DWORD dwTempId = 0;
	WORD wTempPn = 0;
	WORD wRecTotalNum = 0;
	WORD wPtr = 0;
	WORD wReadLen = 0;
	BYTE* p = pbBuf;
//	BYTE bTestBuf[300];

	if(wPn < PN_NUM)
	{
		if (((dwId<0xE2000001)||((dwId>0xE200004e)&&(dwId!=0xE20010FF)&&(dwId!=0xE200FFFF)))
			&& !IsEventId(dwId))
		{
			*wRdNum = wTotal;
			return -1;
		}

		if ((dwStartm>dwEndtm)&&(dwId!=0xE20010FF))
		{
			*wRdNum = wTotal;
			return -1;
		}
		
		if(IsEventId(dwId))///////xzz
			dwAddr = FADDR_EVTREC;
		else if(dwId != 0xE20010FF)
			dwAddr = FADDR_ALRREC;
		else
			dwAddr = FADDR_RPTFAILREC;

		iRet = 0;
		
		/////////////////////////////////////for test//////////////////////////////////
		/*if(dwId != 0xE20001FF)
		{
			memset(bTestBuf, 0x00, 300);
 			GetCurTime(&tm);
			WordToByte(wPn, bTestBuf);
			DWordToByte(dwId, bTestBuf+2);
 			bTestBuf[6] = 1;
 			bTestBuf[12] = ByteToBcd(14);
 			bTestBuf[11] =  ByteToBcd(tm.nMonth);
 			bTestBuf[10] =  ByteToBcd(tm.nDay);
 			bTestBuf[9] =  ByteToBcd(tm.nHour);
 			bTestBuf[8] =  ByteToBcd(tm.nMinute);
 			bTestBuf[7] =  ByteToBcd(tm.nSecond);
 			wAlrLen = GetFmtARDLen(dwId)+6;
 			memset(&bTestBuf[13], 0x01, wAlrLen);
 			WriteAlrRec(dwAddr, bTestBuf, wAlrLen);
 		}*/



		//////////////////////////////////////test end/////////////////////////////////
		WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

		
		
		memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
		//��ȡ�ܹ��洢�ı���
		if (ExFlashRdData(dwAddr, NULL, g_ExFlashBuf, 4) < 0)
		{	
			*wRdNum = wTotal;
			SignalSemaphore(g_semExFlashBuf);
			return -1;
		}

		wRecTotalNum = ByteToWord(g_ExFlashBuf);
		wPtr = ByteToWord(g_ExFlashBuf+2);

		if(wRecTotalNum==0 || wRecTotalNum>wTotal || wPtr>wTotal)
		{	
			*wRdNum = wTotal;
			SignalSemaphore(g_semExFlashBuf);
			return -1;
		}

		if(wRecTotalNum >= wTotal)
			wRecTotalNum = wTotal;


		while(*wRdNum<wRecTotalNum)
		{
			memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);

			if(wRecTotalNum == wTotal)//��ֹ��ȡ��Ч����
				wPtr = (wPtr+(*wRdNum)) % wTotal;
			else
				wPtr = *wRdNum;

			(*wRdNum)++;
			iRet = ReadOneAlr(dwAddr, g_ExFlashBuf, wPtr);
			if(iRet == TDB_ERR_FLS_RD_FAIL)
			{
				SignalSemaphore(g_semExFlashBuf);
				*wRdNum = wTotal;
				return -1;
			}
			else if(iRet < 0)
			{
				continue;
			}

			dwTempId = ((DWORD)(g_ExFlashBuf[6] << 24) | (DWORD)(g_ExFlashBuf[5] << 16) | (DWORD)(g_ExFlashBuf[4] << 8) | (DWORD)g_ExFlashBuf[3]);	
			wTempPn = ByteToWord(g_ExFlashBuf+1);
			if(!IsEventId(dwId))
			{
				if(dwId!=0xE20010FF && dwId!=0xE200FFFF)
				{
					//�ж�ID	
					if(dwId != dwTempId)
						continue;
				}
				//�ж�PN
				if(wTempPn!=wPn && dwId!=0xE20010FF)
					continue;
				if(dwTempId == 0)
					continue;
				wAlrLen = GetFmtARDLen(dwTempId);
				if(wAlrLen<=0)
				{
					continue;
				}
			}
			else
			{
				if(wTempPn!=wPn)
					continue;
				if(dwTempId==0 || dwTempId!=dwId)
					continue;

				wAlrLen = GetEventLen(dwTempId);
				if(wAlrLen<=0)
				{
					continue;
				}
			}
			Fmt1ToTime(&g_ExFlashBuf[8], &tm);//CS + ID + PN + ALRSTATUS = 8
			dwSec = TimeToSeconds(&tm);
			if((dwSec>=dwStartm && dwSec<dwEndtm)//ǰ�պ�����
				|| (dwStartm==0xffffffff && dwEndtm==0xffffffff))
			{
				if(((wReadLen+wAlrLen+6) > iTxBufLeft) &&
					(!IsEventId(dwTempId) || memcmp(p, g_ExFlashBuf+7, 6)!=0))
				{
					(*wRdNum)--;//���ڿռ䲻���˳��󣬻Ὣ��ǰ�������ݶ������´ν����ض�
					SignalSemaphore(g_semExFlashBuf);
					//return GB_RDERR_NOROOM;
                    return wReadLen;    //xzz
				}


				IdPnToBytes(dwTempId, wTempPn, g_ExFlashBuf+ALRSECT_SIZE);
				if(IsEventId(dwTempId) && memcmp(p, g_ExFlashBuf+ALRSECT_SIZE, 6)==0  
					&& memcmp(p+6, g_ExFlashBuf+7, 6)==0 && IsAllAByte(p+12, INVALID_DATA, wAlrLen-6))
				{
					//�����¼����������лָ�ʱ�����ùܷ�����
					memcpy(p+6, g_ExFlashBuf+7, wAlrLen);
				}
				else
				{
					IdPnToBytes(dwTempId, wTempPn, pbBuf+wReadLen);
					memcpy(pbBuf+wReadLen+6, g_ExFlashBuf+7, wAlrLen);
					p = pbBuf + wReadLen;
					wReadLen += (wAlrLen+6);
				}	
				
			}
		}
		SignalSemaphore(g_semExFlashBuf);
		if(*wRdNum >= wRecTotalNum)
			*wRdNum = wTotal;

	}
	return wReadLen;
}


#if 0
//����ʱ��ID���
WORD IsDemandBlockId(DWORD dwId)
{
	WORD wBlockId = 0;
	switch(dwId)
	{
	case 0x0101ff00://��ǰ�����й������������ʱ�����ݿ�
		wBlockId = 0xb01f;
		break;
	case 0x0102ff00://��ǰ�����й������������ʱ�����ݿ�
		wBlockId = 0xb02f;
		break;
	case 0x0103ff00://��ǰ����޹�1�����������ʱ�����ݿ�
		wBlockId = 0xb11f;
		break;
	case 0x0104ff00://��ǰ����޹�2�����������ʱ�����ݿ�
		wBlockId = 0xb12f;
		break;
	case 0x0105ff00://��ǰ��һ���������������ʱ�����ݿ�
		wBlockId = 0xb13f;
		break;
	case 0x0106ff00://��ǰ�ڶ����������������ʱ�����ݿ�
		wBlockId = 0xb15f;
		break;
	case 0x0107ff00://��ǰ�������������������ʱ�����ݿ�
		wBlockId = 0xb16f;
		break;
	case 0x0108ff00://��ǰ�������������������ʱ�����ݿ�
		wBlockId = 0xb14f;
		break;
	case 0x0109ff00://��ǰ�������������������ʱ�����ݿ�
		wBlockId = 0xb03f;
		break;
	case 0x010aff00://��ǰ�������������������ʱ�����ݿ�
		wBlockId = 0xb04f;
		break;

	case 0x0101ff01://�¶��������й������������ʱ�����ݿ�
		wBlockId = 0xb41f;
		break;
	case 0x0102ff01://�¶��ᷴ���й������������ʱ�����ݿ�
		wBlockId = 0xb42f;
		break;
	case 0x0103ff01://�¶�������޹�1�����������ʱ�����ݿ�
		wBlockId = 0xb51f;
		break;
	case 0x0104ff01://�¶�������޹�2�����������ʱ�����ݿ�
		wBlockId = 0xb52f;
		break;
	case 0x0105ff01://�¶����һ���������������ʱ�����ݿ�
		wBlockId = 0xb53f;
		break;
	case 0x0106ff01://�¶���ڶ����������������ʱ�����ݿ�
		wBlockId = 0xb55f;
		break;
	case 0x0107ff01://�¶���������������������ʱ�����ݿ�
		wBlockId = 0xb56f;
		break;
	case 0x0108ff01://�¶���������������������ʱ�����ݿ�
		wBlockId = 0xb54f;
		break;
	case 0x0109ff01://�¶����������������������ʱ�����ݿ�
		wBlockId = 0xb43f;
		break;
	case 0x010aff01://�¶��ᷴ�����������������ʱ�����ݿ�
		wBlockId = 0xb44f;
		break;

	case 0x050609ff://�ն��������й������������ʱ�����ݿ�
		wBlockId = 0x9c8f;
		break;
	case 0x05060aff://�ն��ᷴ���й������������ʱ�����ݿ�
		wBlockId = 0x9caf;
		break;
	case 0x05060bff://�ն�������޹�1�����������ʱ�����ݿ�
		wBlockId = 0x9c9f;
		break;
	case 0x05060cff://�ն�������޹�2�����������ʱ�����ݿ�
		wBlockId = 0x9cbf;
		break;
	}

	return wBlockId;
}

//Ϊ����ʱ�������
bool AddDemandBlockYear(BYTE* pbBuf,WORD wId)
{
	BYTE bTimeBuf[25] = {0};
	BYTE* pbDataBuf = pbBuf+3;
	TTime tmTimeNow;
	BYTE i;
	GetCurTime(&tmTimeNow);

	if(pbBuf==NULL)
		return false;

	ReadItemEx(BN0,PN0,wId,bTimeBuf);
	// 	TraceBuf(DB_CRITICAL,"******** time buf is ",bTimeBuf,25);
	if(wId&0x000f == 0x000f)
	{
		for (i=0; i<5; i++)
		{
			if(0==BcdToDWORD(&pbBuf[8*i],3))
			{
				memset(pbDataBuf+8*i,0,5);
				continue;
			}
			if(IsAllAByte(bTimeBuf+4*i, 0, 4))
			{
				memset(pbBuf,0,5);
				continue;
			}

			memcpy(pbDataBuf+8*i,bTimeBuf+4*i,4);
			if(BcdToByte(pbDataBuf[3+8*i])<tmTimeNow.nMonth)
				pbDataBuf[4+8*i] = ByteToBcd(tmTimeNow.nYear%100-1);
			else
				pbDataBuf[4+8*i] = ByteToBcd(tmTimeNow.nYear%100);
		}
	}
	else
	{
		if(IsAllAByte(bTimeBuf, 0, 4))
		{
			memset(pbBuf,0,5);
			return true;
		}

		memcpy(pbBuf,bTimeBuf,4);
		if(BcdToByte(pbBuf[3])<tmTimeNow.nMonth)
			pbBuf[4] = ByteToBcd(tmTimeNow.nYear%100-1);
		else
			pbBuf[4] = ByteToBcd(tmTimeNow.nYear%100);
	}

	return true;
}
#endif
//��������ͨ��Э����õĶ�ȡ1�����ݽӿڣ�1�ε���ֻ�ܶ�ȡһ��������
//������@pbTx �������ض�ȡ������������
//		@iTxBufSize ���ͻ�����ʣ�µĴ�С
//���أ���ȷ�򷵻����ݳ��ȣ�һ������ֻ���������򷵻�0��
//		��ʧ���򷵻�-GB_RDERR_FAIL���ռ䲻���򷵻�-GB_RDERR_NOROOM
//��ע��1���Сʱ����ת������Ӧ��2��󣬵��ñ�����
int SgReadClass1(DWORD dwId, WORD wPn, BYTE* pbTx, int iTxBufSize, bool fRptState)
{
	int iLen, iRet;
	DWORD dwRdSec;
	TProIdInfo ProIdInfo = {0};
	const WORD* pwSubID;
	WORD wBN, wID;	//ת����ı�ʶ
	BYTE bRdMode;	//�䳤
	bool fMtrPn;
	WORD bNum,i;
	TTime now;
	int iThrId;

	
	//if(IsCctPn(wPn))
	//{
	//	wBn  =BN21;
	//}
	//else
	//{
		wBN = BN0;
	//}
	memset(&ProIdInfo,0,sizeof(TProIdInfo));
	if(!GetProIdInfo(dwId,&ProIdInfo,false))
		return -GB_RDERR_FAIL;;
	//��ȡ��dwID����Ϣ
	pwSubID = ProId2SubId(ProIdInfo.dwProId,&bNum);

	if(pwSubID!=NULL && bNum>0)
	{
		iLen = 0;
		fMtrPn = IsPnValid(wPn);
		for(i = 0; i<bNum; i++)
		{
			if(GetItemLen(wBN,pwSubID[i]) > 0)
			{
				if ((iLen+GetItemLen(wBN,pwSubID[i])) > iTxBufSize)
					return -GB_RDERR_NOROOM;
			}
			else
				continue;
			
			GetCurTime(&now); //ʱ��ͳһ�������ȡ����֤��д������ͬһ������
			wID = pwSubID[i];
			if (IsMtrID(wID) && !MeterSpecierId(dwId))
			{
				iRet = ReadItemEx(BN24, PN0, 0x4105, &bRdMode);
				if (iRet>0 && bRdMode==3 && !fRptState)	//ʵʱ��ȡ�������F38ͬ����Ч
				{							//�������Ͳ���ֱ�ӳ�������Ѽ���������ݵ������
					g_bDirRdStep = 1;
					iThrId = DoDirMtrRd(wBN, wPn, wID, now);
					g_bDirRdStep = 0;
					if (iThrId < 0)
						return -GB_RDERR_FAIL;
				}
				else
				{
					iThrId = GrabPnThread(wPn);	//GetPnThread
					SetDynPn(iThrId, wPn);	//����ϵͳ�⣬����ò����������
				}
			}

			if (fMtrPn && !IsPnDataLoaded(wPn))	//���������ݻ�û������ڴ�
			{
				if (!LoadPnDirData(wPn))	//���������ֱ������
				{
					DTRACE(DB_FAPROTO, ("GbReadClass1: fail to rd pn=%d's data!\r\n", wPn));
					return -GB_RDERR_FAIL;
				}
			}

			dwRdSec = GetCurIntervSec(wPn, &now);   //���ϵ�ǰ������ʱ��
			if (fRptState && dwRdSec==TimeToMinutes(&now)*60)
				dwRdSec -= GetMeterInterv(wPn)*60;

			if(MeterSpecierId(dwId))
			{
				dwRdSec = 0;
			}

			//if(GetItemLen(wBN,pwSubID[i]) > 0)
			//{
				ReadItemTm(wBN, wPn, pwSubID[i], pbTx+iLen, dwRdSec, INVALID_TIME);	//û���������ݶ��Ѿ��ó���Ч����
				iLen += GetItemLen(wBN,pwSubID[i]);
			//}
			
			
		}
#if 0		
		wID = IsDemandBlockId(ProIdInfo.dwProId);
		if (wBN==BN0 && wPn==PN0 && wID!=0)
		{
			// 				TraceBuf(DB_CRITICAL,"********* pre ReadBuf data is ",bBuf,43);
			AddDemandBlockYear(pbTx+1,wID);
			// 				TraceBuf(DB_CRITICAL,"*********     ReadBuf data is ",bBuf,43);
		}
#endif
		if (wBN==BN0 && (ProIdInfo.dwProId==0x020a01ff || ProIdInfo.dwProId==0x020a02ff || ProIdInfo.dwProId==0x020a03ff || ProIdInfo.dwProId==0x020b01ff || ProIdInfo.dwProId==0x020b02ff || ProIdInfo.dwProId==0x020b03ff))
		{///��г��������
			//BYTE *pTmpPtr = bBuf;
			//BYTE bTmpBuf[44];
			//memcpy(bTmpBuf, pTmpPtr+2, 42);
			memcpy(pbTx+2, pbTx+4, 40);
			if(ProIdInfo.dwId==ProIdInfo.dwProId)//20140627-1
				ProIdInfo.wDataLen -= GetItemLen(wBN,pwSubID[i]);//20140320-1
		}

		if(ProIdInfo.bIdx == INVALID_DATA && ProIdInfo.wOffset==0xFFFF) //��id����Ŀǰ��id��֧�ֵķ�Χ,û����ȷ��ƫ��Ĭ�ϳ���Ч����
			memset(pbTx,INVALID_DATA,ProIdInfo.wDataLen);
		else
			memcpy(pbTx,pbTx+ProIdInfo.wOffset,ProIdInfo.wDataLen);

		
		
		iRet = ProIdInfo.wDataLen;
	}

	if (iRet <= 0)
	{
		iRet = ProIdInfo.wDataLen;
		if(iRet >0)
		{
			if(bNum > 0 && pwSubID[0] == 0x8908 && ProIdInfo.wOffset==0x00)
			{
				memset(pbTx+6+1,INVALID_DATA,ProIdInfo.wDataLen-1);
			}
			else
				memset(pbTx+6,INVALID_DATA,ProIdInfo.wDataLen);

		}
	}
	return iRet;
}


//����ȱʡֵ!,�б����£�֮���������ʺ�г������ֵ��
//�ܽ�4���飺
//A: Td+����ʱ��+���� 1-4,9-12,17-20
//B: Td+���� 5-8,21-24,58-59,61-62
//C: Td+г������ 121-123
//D������
typedef struct 
{
	BYTE bFN;//��ʶ
	BYTE bDefaultStyle;//ȱʡֵ���Ͷ��� 0:ȫ0xee; 1:ȫ0; 2:����0
	WORD wOff;//����0����ʼλ��
	WORD wLen;//����0����������
}TC2DefaultValDesc;

const TC2DefaultValDesc C2DefaultValDesc[]=
{//��ʶ		ȱʡֵ����	��ʼλ��  ����
	{ 25,		2,			24,		8	},
	{ 27,		2,			0,		30	},
#ifdef PRO_698
	{ 28,		2,			0,		4	},
#else
	{ 28,		1,			0,		0	},
#endif
	{ 29,		2,			0,		14	},
	{ 30,		1,			0,		0	},
#ifdef PRO_698
	{ 32,		2,			5,		20	},
#else
	{ 31,		2,			5,		20	},
#endif
	{ 33,		2,			24,		8	},
	{ 35,		2,			0,		30	},
#ifdef PRO_698
	{ 36,		2,			0,		4	},
#else
	{ 36,		1,			0,		0	},
#endif
	{ 37,		2,			0,		14	},
	{ 38,		1,			0,		0	},
	{ 41,		1,			0,		0	},
	{ 42,		1,			0,		0	},
	{ 43,		1,			0,		0	},
	{ 44,		1,			0,		0	},
	{ 49,		1,			0,		0	},
	{ 50,		1,			0,		0	},
	{ 51,		1,			0,		0	},
	{ 52,		1,			0,		0	},
#ifdef PRO_698
	{ 53,		1,			0,		0	},
	{ 54,		1,			0,		0	},
#endif
	{ 57,		2,			10,		2	},
	{ 60,		2,			10,		2	},
	{ 65,		2,			0,		2	},
	{ 66,		2,			0,		2	},
	
	{ 113,		1,			0,		0	},
	{ 114,		1,			0,		0	},
	{ 115,		1,			0,		0	},
	{ 116,		1,			0,		0	},
	{ 117,		1,			0,		0	},
	{ 118,		1,			0,		0	},

	{ 121,		1,			0,		0	},
	{ 122,		1,			0,		0	},
	{ 123,		1,			0,		0	},
	{ 129,		2,			0,		4	},
	{ 130,		2,			0,		4	},
	{ 0,}
};

//�������������Ч���ݣ������ʲ�����Ч
void SetC2DefaultVal(BYTE bFN, WORD wPN, BYTE *pbBuf, WORD wItemNum, WORD wItemLen)
{
	BYTE *pTmp=pbBuf;
	WORD wBufLen;

	BYTE bRateNum;

	if (!wItemNum || !wItemLen)
		return;

	wBufLen = wItemNum * wItemLen;


	memset(pbBuf, INVALID_DATA, wBufLen); 


// 	bRateNum = RATE_NUM;
// 	if ((1 <= bFN && 8 >= bFN)
// 		|| (11 <= bFN && 20 >= bFN)
// 		|| (23 <= bFN && 26 >= bFN)) 
// 	{
// 		int i;
// 		for (i = 0; i < wItemNum; i++)
// 		{
// 			pbBuf[i * wItemLen-1] = bRateNum;
// 		}
// 		
// 	}
	
}


//��������ͨ��Э����õĶ�ȡ2�����ݽӿ�
//������@pbRx ����֡�����������������·�֡��Ҫ��FN�ĸ����������ݣ��綳��������ʱ�꣬
//			  ���������Σ�pbRx��ָ������ݲ���
//		@pbTx �������ض�ȡ������������
//		@iTxBufSize ���ͻ�����ʣ�µĴ�С
//		@wStart ���ζ�ȡ���ݵĿ�ʼ��������Զ�ζ�ȡ���������������������Ҫ������ݵ����У����ζ�ȡ�ӵ�wStart�㿪ʼ
//				���ڶ�ȡ�Ĺ����У��Զ��������´��ٽ�����ʱ������ϴε�
//���أ���ȷ�򷵻����ݳ��ȣ�һ������ֻ���������򷵻�0��
//		��ʧ���򷵻�-GB_RDERR_FAIL���ռ䲻���򷵻�-GB_RDERR_NOROOM
//��ע��1���Сʱ����ת������Ӧ��2��󣬵��ñ�����
int SgReadClass2(DWORD dwID, WORD wPn, BYTE* pbRx, BYTE* pbTx, int iTxBufSize, WORD* pwStart, bool fRptState)
{
	TProIdInfo m_tProIdInfo = {0};
	BYTE* pbTx0 = pbTx;
	const TCommTaskCtrl* pTaskCtrl;
	int iRdLen, iRet=0;
	WORD i, wRecDataLen;
	WORD wNumToRd = 1;
	TTime tmStart,tmEnd;
	BYTE bInterv;
	WORD wBN, wPoint, wID;	//ת����ı�ʶ
	BYTE bFn;
	BYTE bRate;
	BYTE bIntervU;
	int iIntervNum;
	BYTE bRateNum = RATE_NUM;

	bFn = FindFn(dwID);
	if (bFn == 0)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: FN is 0!\r\n"));
		return -GB_RDERR_FAIL;		//��ʧ��
	}
		

	if (wPn != 0)
	{
		bool fMtrPn = IsPnValid(wPn);
		if (!fMtrPn)
		{
			DTRACE(DB_TASKDB, ("TdbReadRec: PN is INVALID!\r\n"));
			return -GB_RDERR_FAIL;		//��ʧ��
		}
			
	}

	if (iTxBufSize < 0)
		return -GB_RDERR_NOROOM;

	pTaskCtrl = ComTaskFnToCtrl(bFn);
	if (pTaskCtrl==NULL)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: TaskCtrl is NULL!\r\n"));
		return -GB_RDERR_FAIL;		//��ʧ��
	}
		

	wRecDataLen = ComTaskGetDataLen(pTaskCtrl);

	memset(&tmStart, 0, sizeof(tmStart));
	memset(&tmEnd, 0, sizeof(tmEnd));
	if (DATA_TIME_LEN+wRecDataLen > iTxBufSize)
		return -GB_RDERR_NOROOM;  //�ռ䲻��
	BcdToTime(pbRx, &tmStart);    //Get Start Time
	BcdToTime(pbRx+6, &tmEnd);    //Get End Time
	iIntervNum = IntervsPast(&tmStart, &tmEnd, pTaskCtrl->bIntervU, 1);
	if (iIntervNum > 0)
		wNumToRd = iIntervNum;
	else
		wNumToRd = 0;

	if (!GetProIdInfo(dwID, &m_tProIdInfo, true))
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: Can not find ID!\r\n"));
		return -GB_RDERR_FAIL;   //��ʧ��
		//			TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx���ݺ��ʱ��
	}

	if (TdbOpenTable(bFn) != TDB_ERR_OK)
	{
		if (fRptState)	//�����ϱ���Ч���ݴ�����ֻ�ϱ�һ����Ч��¼
		{
			SetC2DefaultVal(bFn, wPn, pbTx, 1, wRecDataLen);
			if ((1 <= bFn && 8 >= bFn)
				|| (13 <= bFn && 20 >= bFn)
				|| (23 <= bFn && 26 >= bFn)) 
			{
				if (m_tProIdInfo.wOffset == 0)
				{
					pbTx[0] = bRateNum;
				}
			}
				
			TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx���ݺ��ʱ�� ��ʱ�겻����Ч����
			pbTx += wRecDataLen+6;
			(*pwStart)++;			
			iRet = pbTx - pbTx0;  
            TdbCloseTable(bFn);
			return iRet;
		}
        TdbCloseTable(bFn);
		DTRACE(DB_TASKDB, ("TdbReadRec: Can not open File!\r\n"));
		return -GB_RDERR_FAIL;		//��ʧ��
	}    

	for (i=0; i<wNumToRd; i++)
	{
		if (pbTx-pbTx0+wRecDataLen+6 > iTxBufSize)  //��2�ʺ󳬳��������Ѿ���֤��һ�ʲ��ᳬ��
			break;
		
		iRdLen = TdbReadRec(bFn, wPn, tmStart, pbTx);
		if (iRdLen > 0)
		{
				if(m_tProIdInfo.bIdx == INVALID_DATA && m_tProIdInfo.wOffset==0xFFFF) //��id����Ŀǰ��id��֧�ֵķ�Χ,û����ȷ��ƫ��Ĭ�ϳ���Ч����
				{
					memset(pbTx,INVALID_DATA,m_tProIdInfo.wDataLen);
					TimeToBcd(&tmStart,pbTx+m_tProIdInfo.wDataLen);//pbTx���ݺ��ʱ��
					pbTx += (m_tProIdInfo.wDataLen+6);                //���ݳ��ȼ�6���ֽ�ʱ��
				}
				else if (m_tProIdInfo.bIdType == 0)
				{
					//memcpy(pbTx,pbTx+(m_tProIdInfo.wOffset-1)*m_tProIdInfo.wDataLen+1,m_tProIdInfo.wDataLen);
					memcpy(pbTx,pbTx+m_tProIdInfo.wOffset,m_tProIdInfo.wDataLen);
					TimeToBcd(&tmStart,pbTx+m_tProIdInfo.wDataLen);//pbTx���ݺ��ʱ��
					pbTx += (m_tProIdInfo.wDataLen+6);                //���ݳ��ȼ�6���ֽ�ʱ��
				}
				else if(m_tProIdInfo.bIdType == T_BLK || m_tProIdInfo.bIdType == T_FEE_BLK)  
				{
					if (IsAllAByte(pbTx, INVALID_DATA, iRdLen) && (m_tProIdInfo.bIdType == T_FEE_BLK))
						pbTx[0] = 4;	//���ʸ���

					TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx���ݺ��ʱ��
					pbTx += (wRecDataLen+6);                //���ݳ��ȼ�6���ֽ�ʱ��
				}

		}
		else
		{
			if (m_tProIdInfo.bIdType == 0)                   //��ID
			{
				SetC2DefaultVal(bFn, wPn, pbTx, 1, m_tProIdInfo.wDataLen);
				TimeToBcd(&tmStart,pbTx+m_tProIdInfo.wDataLen);//pbTx���ݺ��ʱ��
				pbTx += (m_tProIdInfo.wDataLen+6);                //���ݳ��ȼ�6���ֽ�ʱ��
			}
			else  if(m_tProIdInfo.bIdType == T_BLK || m_tProIdInfo.bIdType == T_FEE_BLK)                                          //��ID
			{
				SetC2DefaultVal(bFn, wPn, pbTx, 1, wRecDataLen);
				if (m_tProIdInfo.bIdType == T_FEE_BLK)
					pbTx[0] = bRateNum;

				TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx���ݺ��ʱ��
				pbTx += (wRecDataLen+6);                //���ݳ��ȼ�6���ֽ�ʱ��
			}
		}
		
		AddIntervs(&tmStart, pTaskCtrl->bIntervU, 1);
		//pbTx += wRecDataLen+6;                //���ݳ��ȼ�6���ֽ�ʱ��
		(*pwStart)++;
	}
	iRet = pbTx - pbTx0;
	TdbCloseTable(bFn);

	return iRet;
}

