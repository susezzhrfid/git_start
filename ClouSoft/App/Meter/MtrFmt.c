/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterFmt.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ָ�Э����صĳ���������ĸ�ʽת����
 �� 			 ���ݱ��桢IDת������
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#include "ComAPI.h"
#include "FaAPI.h"
#include "MtrAPI.h"
#include "MtrFmt.h"
#include "DbConst.h"
#include "FaConst.h"
#include "MtrCtrl.h"
#include "DbHook.h"
#include "DbFmt.h"
#include "SysDebug.h"
#include "LibDbAPI.h"

const WORD g_wCurveFnToId[][3]=
{
	//��11
	{81,	0x3681, 0xb630},
	{82,	0x3682, 0xb631},
	{83,	0x3683, 0xb632},
	{84,	0x3684, 0xb633},
	{85,	0x3685, 0xb640},
	{86,	0x3686, 0xb641},
	{87,	0x3687, 0xb642},
	{88,	0x3688, 0xb643},	
	//��12			
	{89,	0x3689, 0xb611},
	{90,	0x3690, 0xb612},
	{91,	0x3691, 0xb613},
	{92,	0x3692, 0xb621},
	{93,	0x3693, 0xb622},
	{94,	0x3694, 0xb623},	
	//��12		
	{101,	0x3701, 0x9010},
	{102,	0x3702, 0x9110},
	{103,	0x3703, 0x9020},
	{104,	0x3704, 0x9210},
	//��19
	{145,	0x3745, 0x9130},
	{146,	0x3746, 0x9140},
	{147,	0x3747, 0x9150},
	{148,	0x3748, 0x9160}
};

const WORD g_DayFrzIdTo97Cfg[][2] = //��07����ն���IDת������
{
	{0x9a1f, 0x901f},
	{0x9a2f, 0x902f},
	{0x9b1f, 0x911f},
	{0x9b2f, 0x912f},
	{0x9b3f, 0x913f},
	{0x9b4f, 0x914f},
	{0x9b5f, 0x915f},
	{0x9b6f, 0x916f},
	{0x9c0f, 0xa01f},
	{0x9c1f, 0xa11f},
	{0x9c2f, 0xa02f},
	{0x9c3f, 0xa12f},
	{0x9c8f, 0xb01f},
	{0x9c9f, 0xb11f},
	{0x9caf, 0xb02f},
	{0x9cbf, 0xb12f}
};

WORD GetV97DayFrzId(WORD wId)
{ 
	WORD i;

	for (i=0; i<sizeof(g_DayFrzIdTo97Cfg)/(sizeof(WORD)*2); i++)
	{
		if (g_DayFrzIdTo97Cfg[i][0] == wId)
			return g_DayFrzIdTo97Cfg[i][1];
	}

	return 0xffff;
}


BYTE GetFnFromCurveId(WORD wId)
{ 
	WORD i;

	for (i=0; i<sizeof(g_wCurveFnToId)/(sizeof(WORD)*3); i++)
	{
		if (g_wCurveFnToId[i][1] == wId)
			return (BYTE)g_wCurveFnToId[i][0];
	}

	return 0xff;
}

WORD GetV97IdFromCurveId(WORD wId)
{ 
	WORD i;

	for (i=0; i<sizeof(g_wCurveFnToId)/(sizeof(WORD)*3); i++)
	{
		if (g_wCurveFnToId[i][1] == wId)
			return g_wCurveFnToId[i][2];
	}

	return 0xffff;
}

BYTE GetInstCurveType(WORD wId)
{
	if (wId>=0x3681 && wId<=0x3688)
		return 0;
	else if (wId>=0x3689 && wId<=0x3694)
		return 1;
	else if (wId>=0x3701 && wId<=0x3704)
		return 2;
	else if (wId>=0x3745 && wId<=0x3748)
		return 3;	
	else if (wId>=0x3705 && wId<=0x3708)
		return 4;	
	else 
		return 0xff;
};

//��������ͨ��ID��Э���ʽ���ݿ��ID��ת��
WORD CommIdToProId(WORD wCommId)
{
	WORD wDstId = wCommId;

	switch (wCommId)
	{		
	case 0xc010:		
		wDstId = 0xc700;
		break;
	case 0xc011:		
		wDstId = 0xc701;
		break;
	case 0xc01f:	
		wDstId = 0xc70f;
		break;
	//99��645�ĵ������ת��Ϊ07��645������
	case 0xb210:	
		wDstId = 0xc811;
		break;
	case 0xb211:
		wDstId = 0xc831;
		break;
	case 0xb212:
		wDstId = 0xc810;
		break;
	case 0xb213:
		wDstId = 0xc830;
		break;
	case 0xb214:
		wDstId = 0xc870;
		break;
	case 0xc020:	
		wDstId = 0xc860;
		break;
	case 0xc117:	
		wDstId = 0xc871;
		break;
	case 0xc810:	//���0xd8XX
	case 0xc811:
	case 0xc812:
	case 0xc813:
	case 0xc814:
	case 0xc815:
	case 0xc850:	
	case 0xc851:
	case 0xc852:
	case 0xc853:
		wDstId += 0x1000;
		break;
	case 0xc9d2:	//���0xd040
		wDstId = 0xd040;
		break;
	default:
		break;	
	}

	return wDstId;
}


//��������ͨ��������Э���ʽ���ݵ�ת��
//������@wDstId  Ҫת��Ϊ��Ŀ��ID
//		@pbDst	 Ҫת��Ϊ��Ŀ���ʽֵ
//		@wSrcId	 ͨ������ԭʼID	
//		@pbSrc	 ͨ�����ݸ�ʽ��ԭʼ����
//		@wSrcLen ͨ�����ݸ�ʽ��ԭʼ����
//���أ���������ת��OK,����ת��������ݸ�ʽ�ĳ���
int CommToProType(WORD* pwDstId, BYTE* pbDst, WORD wSrcId, BYTE* pbSrc, WORD wSrcLen)
{
	int iRet = -1;
	int i;	
	BYTE tLen=0;
	BYTE iLen=0;
	DWORD dwVal;
	int32 iVal32;
	uint64 iVal64;
	BYTE bInvdData = GetInvalidData(ERR_APP_OK);
	WORD wTmpSrcId;

	*pwDstId = CommIdToProId(wSrcId);
	
	//���ɳ���ID��ת��
	if ( IsDayFrzId(wSrcId) )
	{
		wTmpSrcId = GetV97DayFrzId(wSrcId);
	}
	else if ( IsCurveId(wSrcId) )
	{		
		wTmpSrcId = GetV97IdFromCurveId(wSrcId);		
	}
	else
		wTmpSrcId = wSrcId;

	if (wSrcId == 0xd800) //��λ������ID
		wTmpSrcId = 0xb66f;

	switch (wTmpSrcId>>8)
	{		
		case 0x90:
		case 0x94://�й�����		
				iLen = sizeof(uint64);				
				for (i=0; i<wSrcLen/iLen; i++)
				{	
					memcpy((BYTE*)&iVal64, pbSrc+i*iLen, iLen);
					if (iVal64 == (uint64)INVALID_VAL64)
						memset(pbDst+tLen, INVALID_DATA, 4);
					else
						Uint64ToBCD(iVal64, pbDst+tLen, 4);				
					tLen += 4;					
				}	
				iRet = tLen;			
				break;
		case 0x91:
		case 0x95://�޹�����	
				iLen = sizeof(uint64);				
				for (i=0; i<wSrcLen/iLen; i++)
				{	
					memcpy((BYTE*)&iVal64, pbSrc+i*iLen, iLen);
					if (iVal64 == (uint64)INVALID_VAL64)
						memset(pbDst+tLen, INVALID_DATA, 4);
					else
					{
						//iVal64 /= 100;	//-2С��λ
						Uint64ToBCD(iVal64, pbDst+tLen, 4);		
					}
					tLen += 4;					
				}	
				iRet = tLen;			
				break;		
		case 0xa0:
		case 0xa1:
		case 0xa4:
		case 0xa5://����
			iLen = sizeof(DWORD);				
			for (i=0; i<wSrcLen/iLen; i++)
			{	
				memcpy((BYTE*)&dwVal, pbSrc+i*iLen, iLen);
				if (dwVal == (DWORD)INVALID_VAL)
					memset(pbDst+tLen, INVALID_DATA, 3);
				else					
					DWORDToBCD(dwVal, pbDst+tLen, 3);				
				tLen += 3;					
			}	
			iRet = tLen;	
			break;
		case 0xb6:
				if (wTmpSrcId == 0xb6a0) //���������֧��
					wTmpSrcId = 0xb620; //�������ͬ��ͨ��������

				iLen = sizeof(int32);				
				for (i=0; i<wSrcLen/iLen; i++)
				{	
					memcpy((BYTE*)&iVal32, pbSrc+i*iLen, iLen);
					if ((wTmpSrcId&0xf0)==0x10 || (wTmpSrcId&0xf0)==0x50 || (wTmpSrcId&0xf0)==0x60) //2�ֽ�����
					{
						if (iVal32 == INVALID_VAL)
						{
							memset(pbDst+tLen, INVALID_DATA, 2);
						}
						else	
						{
							if ((wTmpSrcId&0xf0) == 0x10)
								ValToFmt7(iVal32, pbDst+tLen, 2);		
							else
								ValToFmt5(iVal32, pbDst+tLen, 2);
						}
						
						tLen += 2;	
					}
					else if ((wTmpSrcId&0xf0) == 0x20)	//������ʽGB2005��698�в���
					{
						if (iVal32 == INVALID_VAL)
							memset(pbDst+tLen, INVALID_DATA, ILEN);
						else
							//ValToFmt(iVal32/MTRPRO_TO_IFMT_SCALE, pbDst+tLen, IFMT, ILEN);
							ValToFmt25(iVal32/MTRPRO_TO_IFMT_SCALE, pbDst+tLen, ILEN);
							
						tLen += ILEN;
					}	
					else if ((wTmpSrcId&0xf0)==0x30 || (wTmpSrcId&0xf0)==0x40 || (wTmpSrcId&0xf0)==0x70)
					{
						if (iVal32 == INVALID_VAL)
							memset(pbDst+tLen, INVALID_DATA, 3);
						else		
							ValToFmt9(iVal32, pbDst+tLen, 3);				
						tLen += 3;			
					}							
				}	
				iRet = tLen;			
				break;
		case 0xb2:
				switch (wTmpSrcId)
				{
					case 0xb210:	//���ʱ��		
					case 0xb211:	//��������ʱ��
						pbDst[0] = 0; //��
						memcpy(pbDst+1, pbSrc, wSrcLen); //�¡��ա�ʱ����					
						pbDst[wSrcLen+1] = 0;
						iRet = wSrcLen+2; //�� 					
						break;
					case 0xb212:	//��̴���				
					case 0xb213:	//�����������
					case 0xb214:	//���ʱ��				
						memcpy(pbDst, pbSrc, wSrcLen); 
						pbDst[wSrcLen] = 0;
						iRet = wSrcLen+1;
						break;	
					default:
						break;
				}
				break;
		case 0xc0:
				switch (wTmpSrcId)
				{
					case 0xc010:
					//if (pbSrc[0] == 0) //������ڵ����գ�0
					//	pbSrc[0] = 7;
						pbDst[0] = pbSrc[1]; //��
						pbDst[1] = ((pbSrc[0]&0x7f)<<5)+pbSrc[2]; //�ܡ���
						pbDst[2] = pbSrc[3]; //��
						iRet = 3;					
						break;
					case 0xc011:
						memcpy(pbDst, pbSrc, 3); //�롢�֡�ʱ
						iRet = 3;				
						break;
					case 0xc01f:
					if (pbSrc[3] == 0) //������ڵ����գ�0
						pbSrc[3] = 7;
						memcpy(pbDst, pbSrc, 3); //�롢�֡�ʱ
						pbDst[3] = pbSrc[4]; //��
						pbDst[4] = ((pbSrc[3]&0x7f)<<5)+pbSrc[5]; //�ܡ���
						pbDst[5] = pbSrc[6]; //��
						iRet = 6;					
						break;
					case 0xc020:	
						pbDst[0] = pbSrc[0];
						pbDst[1] = 0;
						iRet = 2;					
						break;
					default:
						break;
				}
				break;
		case 0xc1:
				if (wTmpSrcId == 0xc117) //�Զ������գ������գ�
				{
					memcpy(pbDst, pbSrc, 2);
					iRet = 2;	
				}
				break;
		case 0xc8: //2007��645��չ����ID
				switch (wTmpSrcId>>4)
				{
					case 0xc89: //ͭ������
					case 0xc8a:
						if (IsAllAByte(pbSrc, bInvdData, wSrcLen)) //��Ч����
							memset(pbDst, bInvdData, 5);
						else
						{
							pbDst[0] = 0;
							memcpy(pbDst+1, pbSrc, 4);
						}
						iRet = 5;
						break;
					case 0xc81: 
					case 0xc85:
						if (wTmpSrcId==0xc810 || wTmpSrcId==0xc812 || wTmpSrcId==0xc814 || wTmpSrcId==0xc850 || wTmpSrcId==0xc852)
						{
							if (IsAllAByte(pbSrc, bInvdData, wSrcLen)) //��Ч����
								memset(pbDst, bInvdData, 2);
							else							
								memcpy(pbDst, pbSrc, 2);							
							iRet = 2;
						}
						else if (wTmpSrcId==0xc811 || wTmpSrcId==0xc813 || wTmpSrcId==0xc815 || wTmpSrcId==0xc851 || wTmpSrcId==0xc853) //ȥ����
						{
							if (IsAllAByte(pbSrc, bInvdData, wSrcLen)) //��Ч����
								memset(pbDst, bInvdData, 5);
							else							
								memcpy(pbDst, pbSrc+1, 5);
							iRet = 5;
						}					
						break;
					default:		
						break;
				}
				break;
		case 0xc9:
			if (wTmpSrcId==0xc9c4 || wTmpSrcId==0xc9c5||  wTmpSrcId==0xc9b1 //�����ʣ����	
				|| wTmpSrcId==0xc9d2) //4�ֽڹ��ϵ���ת����һ��5�ֽڹ��ϵ���
			{
				pbDst[0] = 0x00;
				memcpy(pbDst+1, pbSrc, 4);
				iRet = 5;	
			}
			break;	
		case 0xd0:				
			if (wTmpSrcId==0xd000 || wTmpSrcId==0xd010) //�������1
			{
				pbDst[0] = 0x00;
				memcpy(pbDst+1, pbSrc, 4);
				iRet = 5;	
			}
			else if (wTmpSrcId==0xd02f || wTmpSrcId==0xd03f) //�������2
			{
				for (i=0; i<5; i++)
				{
					pbDst[i*5] = 0x00;
					memcpy(pbDst+i*5+1, pbSrc+i*4, 4);
					iRet += 5;	
				}
			}
			break;	
		default:		
			break;

	}
	return iRet;

}

//��������Э���ʽ�������һ��ID���������
//		������Ҫת����ID�ڴ���ת�������
//������@wPn  �������
//		@wID	����ID
//		@pbBuf	Ҫд������ݻ�����	
//		@wLen	Ҫд������ݳ���
//		@dwTime	Ҫд�������ʱ��
//		@bErr	�������
//���أ��ɹ��򷵻�true
bool SaveOneMtrItem(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, DWORD dwTime, BYTE bErr)
{
    TTime tmNow;
	WORD wID1 = 0;
	BYTE bBuf[256];
	int i, iLen;
	BYTE bCnt;
	DWORD dwTotal;
	DWORD dwLastTime;
	DWORD dwTmp;
	
	memset(bBuf, 0, sizeof(bBuf));
	if (bErr == ERR_APP_OK)
	{		
		if (CommToProType(&wID1, bBuf, wID, pbBuf, wLen) > 0)
		{
			if (wID1 == wID)	//ԭʼIDҪдת������
			{
				WriteItemTm(BN0, wPn, wID, bBuf, dwTime); //дԭʼID	
				SaveLastRdMtrData(wID, wPn, bBuf);
			}
			else			//ԭʼIDҪд������
			{
				WriteItemTm(BN0, wPn, wID, pbBuf, dwTime); //дԭʼID
				WriteItemTm(BN0, wPn, wID1, bBuf, dwTime); //����Ҫдת��ID
			}
		}
		else if (wID==0xb31f || wID==0xb32f)
		{
			if (IsV07Mtr(wPn))
			{
				iLen = 3;
				dwTotal = 0;
				bCnt = 0;
				for (i=0; i<3; i++)
				{
					if (IsAllAByte(pbBuf+iLen+i*iLen, 0, iLen))
						bCnt++;
					if (!IsAllAByte(pbBuf+iLen+i*iLen, INVALID_DATA, iLen))
						dwTotal += BcdToDWORD(pbBuf+iLen+i*iLen, iLen);
				}
				if (dwTotal>0 || bCnt==3)
					DWORDToBCD(dwTotal, pbBuf, iLen);
				else
					memset(pbBuf, INVALID_DATA, iLen);
			}
			else if (wID == 0xb31f)
			{
				iLen = 2;
				memset(bBuf, 0, sizeof(bBuf));
				for(i=0; i<4; i++)
				{
					memcpy(bBuf+i*(iLen+1), pbBuf+i*iLen, iLen);
				}
				memcpy(pbBuf, bBuf, 12);
			}
			WriteItemTm(BN0, wPn, wID, pbBuf, dwTime);
		}
		else if (wID==0xb33f || wID==0xb34f)
		{
			if (IsV07Mtr(wPn))
		    {
				dwLastTime = 0;
				bCnt = 0;

				//����07��û���ܵ����һ�ζ�����ʼʱ�̣���Ҫ���ݷ����������
				for (i=0; i<3; i++)
				{	
					dwTmp = 0;									
					if (IsAllAByte(pbBuf+4+i*4, 0, 4))					
						bCnt++;
					else if (!IsAllAByte(pbBuf+4+i*4, INVALID_DATA, 4)) 
					{
						tmNow.nYear = 2000;
						tmNow.nMonth = BcdToByte(pbBuf[3+4+i*4]);
						tmNow.nDay = BcdToByte(pbBuf[2+4+i*4]);
						tmNow.nHour= BcdToByte(pbBuf[1+4+i*4]);
						tmNow.nMinute = BcdToByte(pbBuf[0+4+i*4]);
						tmNow.nSecond = 0;
						dwTmp = TimeToSeconds(&tmNow);
					}
				
					if (dwTmp > dwLastTime)
						dwLastTime = dwTmp;
				}
	
	            if (dwLastTime > 0) 
				{
					SecondsToTime(dwLastTime, &tmNow);
					pbBuf[0] = ByteToBcd(tmNow.nMinute);
					pbBuf[1] = ByteToBcd(tmNow.nHour);
					pbBuf[2] = ByteToBcd(tmNow.nDay);
					pbBuf[3] = ByteToBcd(tmNow.nMonth);
	            }
	            else if (bCnt == 3)//����ȫΪ0ʱ�����һ��ҲΪ0
	            	memset(pbBuf, 0, 4);
				else  //������Чʱ���һ�ζ���ҲӦ����Ч
					memset(pbBuf, INVALID_DATA, 4);								
		    }
		    WriteItemTm(BN0, wPn, wID, pbBuf, dwTime);
		}
		else
			WriteItemTm(BN0, wPn, wID, pbBuf, dwTime);	
	}
	else
	{
		UpdItemErr(BN0, wPn, wID, bErr, dwTime);

		wID1 = CommIdToProId(wID);
		if (wID1 != wID)	//ת��IDҲҪд������Ϣ
			UpdItemErr(BN0, wPn, wID1, bErr, dwTime);
	}

	return true;
}

//��������Э���ʽ��������������ֱ���������
//		ͬʱ���ϸõ������ı�־
//������@wPn  �������
//		@wID	����ID
//		@pbBuf	Ҫд������ݻ�����	
//		@wLen	Ҫд������ݳ���
//		@dwTime	Ҫд�������ʱ��
//		@bErr	�������
//���أ��ɹ��򷵻�true
//��ע���˴���ڵ�����Ϊ07Э�����������ݣ�����Ϊ��Ч������ÿ�ʣ�5�ֽ�ʱ�䣫�������ݣ�
bool SaveMeterTask(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, DWORD dwTime, BYTE bErr)
{
	BYTE mBuf[20];
	BYTE bLen = 0, bit = 0;
	bool fSave = false;
	BYTE bIntvMin, bFn, i,j, n;
	BYTE bIsSaveFlg[6], bThrId;	

	TTime time,tNow,tLast;	
	memset((BYTE*)&time, 0, sizeof(TTime));
	GetCurTime(&tNow);

	if (bErr != ERR_APP_OK)
		return false; 

	if ((n=GetInstCurveType(wID)) == 0xff)
		return false;
	bThrId = GetPnThread(wPn);
	if (bThrId >= DYN_PN_NUM)
		return false;
	memcpy(bIsSaveFlg, &g_ProfFrzFlg[bThrId][n*6], sizeof(bIsSaveFlg));

	if ( GetCurveInterv() == TIME_UNIT_HOUR)
		bIntvMin = 60;
	else if ( GetCurveInterv() == TIME_UNIT_MINUTE)
		bIntvMin = 15;

	memset((BYTE*)&tLast, 0, sizeof(TTime));
	tLast.nYear = bIsSaveFlg[5]+2000;//��¼����
	tLast.nMonth = bIsSaveFlg[4];	//��¼����
	tLast.nDay = bIsSaveFlg[3];		//��¼����
	if ( IsDiffDay(&tNow, &tLast) ) //������ǵ�������߼�¼ʱ�꣬������
	{
		memset(bIsSaveFlg, 0, sizeof(bIsSaveFlg));
		bIsSaveFlg[5] = tNow.nYear-2000; 
		bIsSaveFlg[4] = tNow.nMonth;
		bIsSaveFlg[3] = tNow.nDay;
	}

	for (i=0; i<pbBuf[0]; i++)
	{   
		time.nMinute = BcdToByte(pbBuf[bLen+1]);
		time.nHour = BcdToByte(pbBuf[bLen+2]);
		time.nDay  = BcdToByte(pbBuf[bLen+3]);
		time.nMonth = BcdToByte(pbBuf[bLen+4]);
		time.nYear = BcdToByte(pbBuf[bLen+5])+2000;		
		if ( IsInvalidTime(&time) )
			break;

		if ( IsDiffDay(&tNow, &time) ) //������ǵ�������ߣ��򲻴�
		{
            if (pbBuf[0] != 0)
    			bLen += ((wLen-1)/pbBuf[0]); //ÿ�ʼ�¼��ʵ�ʳ���
			continue;
		}

		bit = ((WORD)time.nHour*60 + time.nMinute)/bIntvMin; //�����߱�������������ǰ��Ӧ��ʱ��� 

		if ((bIsSaveFlg[(bit>>3)]&(1<<(bit&0x07))) == 0) //��δ��� ֻ�б����ݿ����ʼID����,��Ϊ�������ݶ���һ��д���		
		{				
			time.nMinute = time.nMinute/bIntvMin*bIntvMin; //ʱ�䰴���߱���������
			TimeToFmt15(&time, mBuf);		//Fmt15 5�ֽ�
			memcpy(mBuf+5, &wPn, 1);		//������� 1�ֽ�	

			//��������������
			if (wID>=0x3701 && wID<=0x3704)	//���������޹�����Ҫ����˳��
			{
				if ((bFn=GetFnFromCurveId(0x3701))==0xff)
					return false;

				memcpy(mBuf+6, &pbBuf[bLen+1+5], 4);
				PipeAppend(TYPE_FRZ_TASK, bFn, mBuf, 10);				

				memcpy(mBuf+6, &pbBuf[bLen+1+13], 4);
				PipeAppend(TYPE_FRZ_TASK, bFn+1, mBuf, 10);				

				memcpy(mBuf+6, &pbBuf[bLen+1+9], 4);
				PipeAppend(TYPE_FRZ_TASK, bFn+2, mBuf, 10);			

				memcpy(mBuf+6, &pbBuf[bLen+1+17], 4);		
				fSave = PipeAppend(TYPE_FRZ_TASK, bFn+3, mBuf, 10);

			}
			else if (wID>=0x3745 && wID<=0x3748) //������Ҫ����˳��
			{
				if ((bFn=GetFnFromCurveId(0x3745))==0xff)
					return false;

				memcpy(mBuf+6, &pbBuf[bLen+1+5], 4);
				PipeAppend(TYPE_FRZ_TASK, bFn, mBuf, 10);						

				memcpy(mBuf+6, &pbBuf[bLen+1+17], 4);
				PipeAppend(TYPE_FRZ_TASK, bFn+1, mBuf, 10);		

				memcpy(mBuf+6, &pbBuf[bLen+1+9], 4);
				PipeAppend(TYPE_FRZ_TASK, bFn+2, mBuf, 10);		

				memcpy(mBuf+6, &pbBuf[bLen+1+13], 4);					
				fSave = PipeAppend(TYPE_FRZ_TASK, bFn+3, mBuf, 10);		
			}
			else if (wID>=0x3681 && wID<=0x3688)//����
			{				
				if ((bFn=GetFnFromCurveId(0x3681))==0xff)
					return false;

				for (j=0; j<8; j++)
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+5+j*3], 3);				
					fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);					
				}
			}
			else if (wID>=0x3689 && wID<=0x3694)//��ѹ������
			{				
				if ((bFn=GetFnFromCurveId(0x3689))==0xff)
					return false;

				for (j=0; j<6; j++)
				{
					if (j < 3)
					{
						memcpy(mBuf+6, &pbBuf[bLen+1+5+(j<<1)], 2); //ȥ��Ƶ��
						fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 8);
					}
					else
					{
						memcpy(mBuf+6, &pbBuf[bLen+1+11+(j-3)*3], 3); //ȥ��Ƶ��				
						fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);
					}
				}
			}
			else if (wID>=0x3705 && wID<=0x3708)//��������
			{		
				if ((bFn=GetFnFromCurveId(0x3705))==0xff)
					return false;

				for (j=0; j<4; j++)
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+5+(j<<1)], 2); //ȥ��Ƶ��				
					fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 8);
				}
			}	

			if (fSave)
			{
				bIsSaveFlg[(bit>>3)] = bIsSaveFlg[(bit>>3)] | (1<<(bit&0x07)); //ÿ����������������ݶ���һ����

				memcpy(g_ProfFrzFlg+n*6, bIsSaveFlg, sizeof(bIsSaveFlg));

				DTRACE(DB_TASK, ("CCommonTask::SaveMeterTask: wID =%02x time=%d-%d-%d  %2x-%2x-%2x\r\n", wID, bIsSaveFlg[5], bIsSaveFlg[4], bIsSaveFlg[3], bIsSaveFlg[2], bIsSaveFlg[1], bIsSaveFlg[0]));	
			}
			else 
				DTRACE(DB_TASK, ("CCommonTask::SaveMeterTask: id=%02x is SaveTask Fail\r\n", wID));		
		}
        if (pbBuf[0] != 0)
    		bLen += ((wLen-1)/pbBuf[0]); //ÿ�ʼ�¼��ʵ�ʳ���
	}

	return true;
}


//�������������������,������ID������
//������@pMtrReq  ��ͬ����ID���е�ָ��
//		@wNum	��ͬ����ID���еĸ���
//		@pbBuf	Ҫд������ݻ�����	
//		@wLen	Ҫд������ݳ���
//		@dwCurT	Ҫд�������ʱ��
//		@bErr	�������
//���أ�MTR_SAVE_OK/MTR_SAVE_RECLAIM
int SaveMeterItem(WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen, DWORD dwTime, BYTE bErr)
{
	DWORD dwIntvT = 0;
	DWORD dwSec = 0;
	BYTE bBuf[256];	
	BYTE bInvdData = GetInvalidData(ERR_APP_OK);	
	BYTE bFlg[2] = {0, 0};	
	BYTE n = 0, bNum = 1;	
	TTime now, time;
	GetCurTime(&now);
		
	memset(bBuf, 0, sizeof(bBuf));
	
	//ʱ�����
	dwIntvT = GetMeterInterv(wPn)*60;
    if (dwIntvT == 0)
        return -1;
	dwSec = dwTime/dwIntvT*dwIntvT;//*60;

	if (wID==0x9a00) //�ϴ��ն���ʱ��
	{
		if (bErr==ERR_APP_OK)
		{
			Fmt15ToTime(pbBuf, &time);
			if (IsDiffDay(&time, &now))
			{
				DTRACE(DB_METER, ("SaveSameMeterItem: wPn=%d, ID=0x05060001, frz time mismatch, %d-%d-%d\n", 
					wPn, time.nYear, time.nMonth, time.nDay));
				SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, ERR_UNSUP);
				return -1;		//���������д�,��Ҫ����������л����س�;
			}
			else
			{
				SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);
				DTRACE(DB_METER, ("SaveSameMeterItem : wPn=%d, ID=0x05060001, frz time ok, %d-%d-%d\n", 
					wPn, time.nYear, time.nMonth, time.nDay));
			}
		}
		else if (bErr==ERR_UNSUP)
		{
			TimeToFmt15(&now, pbBuf);
			SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, ERR_APP_OK);	//����̨�岻֧�ִ�ID��Ϊ�˹�̨�壬����ֱ��ȡ�ն�ʱ����Ϊ9a00
			DTRACE(DB_METER, ("SaveSameMeterItem : wPn=%d, ID=0x05060001 unsupport,  get term time, %d-%d-%d\n", 
				wPn, now.nYear, now.nMonth, now.nDay));
		}
	}
	else if (wID == 0xd800) //�����ĵ�ѹ������λ��ID,ͬʱ�浽b66f
	{
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);
		SaveOneMtrItem(wPn, 0xb66f, pbBuf, wLen, dwSec, bErr);
	}
	else if (wID==0xd000 || wID==0xd010) //�������
	{			
		if (bErr != ERR_APP_OK)
			SaveOneMtrItem(wPn, wID+0xf, pbBuf, wLen, dwSec, bErr);
		else //д��������Ϣ
		{
			bBuf[0] = 0;
			memcpy(bBuf+1, pbBuf, wLen);
			memset(bBuf+wLen+1, bInvdData, 25-1-wLen);
			SaveOneMtrItem(wPn, wID+0xf, bBuf, 25, dwSec, bErr);
		}
	}
	else if (wID == 0xc020)	//97��Э����֧��07��645���������״̬��,����ͬʱд���Ա�������������ʱ
	{				
		if (ReadItemEx(BN0, wPn, 0xc020, bFlg)>0 && bFlg[0]!=bInvdData) 	//����ֻ�жϵ������״̬��1
			bBuf[0] = bFlg[0]^pbBuf[0];			
		else
			bBuf[0] = 0;

		//д��0xc020��0xc860��0xc880��������֧��
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);
		SaveOneMtrItem(wPn, 0xc880, bBuf, 2, dwSec, bErr);			
	}
	else if ((wID>>4) == 0xc86)	//2007��645�ĵ��״̬��,����ͬʱ�жϵ��״̬�ֱ�λ	
	{					
		if ((wID&0xf) == 0xf)
			bNum = wLen>>1;

		if (bErr != ERR_APP_OK)
			bNum = 0;
		
		for (n=0; n<bNum; n++)
		{			
			memset(bFlg, 0, sizeof(bFlg));
			if (ReadItemEx(BN0, wPn, (wID&0xfff0)+n, bFlg)>0 && !IsAllAByte(bFlg, bInvdData, 2)) //�жϵ��״̬�ֱ�λ	
			{
				bBuf[2*n] = bFlg[0]^pbBuf[2*n];
				bBuf[1+2*n] = bFlg[1]^pbBuf[1+2*n];
			}
			else
				memset(&bBuf[2*n], 0, 2);				
		}
		//д����״̬�ֱ�λ
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);
		SaveOneMtrItem(wPn, wID+0x20, bBuf, wLen, dwSec, bErr);		
	}		
	else if ( IsCurveId(wID))	//����һ����07��ķ��أ���Ϊ��07�ı��ID���ύʱ�Ѿ����ɵ�ǰID��
	{			
		//���ߵķ��� ����Ϊ��Ч������ÿ�ʣ�5�ֽ�ʱ�䣫�������ݣ�
		SaveMeterTask(wPn, wID, pbBuf, wLen, dwSec, bErr);	
		SaveOneMtrItem(wPn, wID, pbBuf, 1, dwSec, bErr);
	}
	else
	{
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);		
	}

	return 0;	//�޴���		
}

