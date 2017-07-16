/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterFmt.cpp
 * 摘    要：本文件主要实现跟协议相关的抄表数据项的格式转换、
 × 			 数据保存、ID转换问题
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2008年3月
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
	//组11
	{81,	0x3681, 0xb630},
	{82,	0x3682, 0xb631},
	{83,	0x3683, 0xb632},
	{84,	0x3684, 0xb633},
	{85,	0x3685, 0xb640},
	{86,	0x3686, 0xb641},
	{87,	0x3687, 0xb642},
	{88,	0x3688, 0xb643},	
	//组12			
	{89,	0x3689, 0xb611},
	{90,	0x3690, 0xb612},
	{91,	0x3691, 0xb613},
	{92,	0x3692, 0xb621},
	{93,	0x3693, 0xb622},
	{94,	0x3694, 0xb623},	
	//组12		
	{101,	0x3701, 0x9010},
	{102,	0x3702, 0x9110},
	{103,	0x3703, 0x9020},
	{104,	0x3704, 0x9210},
	//组19
	{145,	0x3745, 0x9130},
	{146,	0x3746, 0x9140},
	{147,	0x3747, 0x9150},
	{148,	0x3748, 0x9160}
};

const WORD g_DayFrzIdTo97Cfg[][2] = //非07表的日冻结ID转换描述
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

//描述：从通用ID向协议格式数据库的ID的转换
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
	//99版645的电表数据转变为07版645的数据
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
	case 0xc810:	//变成0xd8XX
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
	case 0xc9d2:	//变成0xd040
		wDstId = 0xd040;
		break;
	default:
		break;	
	}

	return wDstId;
}


//描述：从通用类型向协议格式数据的转换
//参数：@wDstId  要转换为的目标ID
//		@pbDst	 要转换为的目标格式值
//		@wSrcId	 通用数据原始ID	
//		@pbSrc	 通用数据格式的原始数据
//		@wSrcLen 通用数据格式的原始长度
//返回：大于零则转换OK,返回转换后的数据格式的长度
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
	
	//换成常规ID来转换
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

	if (wSrcId == 0xd800) //相位角特殊ID
		wTmpSrcId = 0xb66f;

	switch (wTmpSrcId>>8)
	{		
		case 0x90:
		case 0x94://有功电能		
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
		case 0x95://无功电能	
				iLen = sizeof(uint64);				
				for (i=0; i<wSrcLen/iLen; i++)
				{	
					memcpy((BYTE*)&iVal64, pbSrc+i*iLen, iLen);
					if (iVal64 == (uint64)INVALID_VAL64)
						memset(pbDst+tLen, INVALID_DATA, 4);
					else
					{
						//iVal64 /= 100;	//-2小数位
						Uint64ToBCD(iVal64, pbDst+tLen, 4);		
					}
					tLen += 4;					
				}	
				iRet = tLen;			
				break;		
		case 0xa0:
		case 0xa1:
		case 0xa4:
		case 0xa5://需量
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
				if (wTmpSrcId == 0xb6a0) //零序电流不支持
					wTmpSrcId = 0xb620; //零序电流同普通电流处理

				iLen = sizeof(int32);				
				for (i=0; i<wSrcLen/iLen; i++)
				{	
					memcpy((BYTE*)&iVal32, pbSrc+i*iLen, iLen);
					if ((wTmpSrcId&0xf0)==0x10 || (wTmpSrcId&0xf0)==0x50 || (wTmpSrcId&0xf0)==0x60) //2字节数据
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
					else if ((wTmpSrcId&0xf0) == 0x20)	//电流格式GB2005和698有差异
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
					case 0xb210:	//编程时间		
					case 0xb211:	//需量清零时间
						pbDst[0] = 0; //年
						memcpy(pbDst+1, pbSrc, wSrcLen); //月、日、时、分					
						pbDst[wSrcLen+1] = 0;
						iRet = wSrcLen+2; //秒 					
						break;
					case 0xb212:	//编程次数				
					case 0xb213:	//需量清零次数
					case 0xb214:	//电池时间				
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
					//if (pbSrc[0] == 0) //电表库出口的周日＝0
					//	pbSrc[0] = 7;
						pbDst[0] = pbSrc[1]; //日
						pbDst[1] = ((pbSrc[0]&0x7f)<<5)+pbSrc[2]; //周、月
						pbDst[2] = pbSrc[3]; //年
						iRet = 3;					
						break;
					case 0xc011:
						memcpy(pbDst, pbSrc, 3); //秒、分、时
						iRet = 3;				
						break;
					case 0xc01f:
					if (pbSrc[3] == 0) //电表库出口的周日＝0
						pbSrc[3] = 7;
						memcpy(pbDst, pbSrc, 3); //秒、分、时
						pbDst[3] = pbSrc[4]; //日
						pbDst[4] = ((pbSrc[3]&0x7f)<<5)+pbSrc[5]; //周、月
						pbDst[5] = pbSrc[6]; //年
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
				if (wTmpSrcId == 0xc117) //自动抄表日（结算日）
				{
					memcpy(pbDst, pbSrc, 2);
					iRet = 2;	
				}
				break;
		case 0xc8: //2007版645扩展抄表ID
				switch (wTmpSrcId>>4)
				{
					case 0xc89: //铜损铁损
					case 0xc8a:
						if (IsAllAByte(pbSrc, bInvdData, wSrcLen)) //无效数据
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
							if (IsAllAByte(pbSrc, bInvdData, wSrcLen)) //无效数据
								memset(pbDst, bInvdData, 2);
							else							
								memcpy(pbDst, pbSrc, 2);							
							iRet = 2;
						}
						else if (wTmpSrcId==0xc811 || wTmpSrcId==0xc813 || wTmpSrcId==0xc815 || wTmpSrcId==0xc851 || wTmpSrcId==0xc853) //去掉秒
						{
							if (IsAllAByte(pbSrc, bInvdData, wSrcLen)) //无效数据
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
			if (wTmpSrcId==0xc9c4 || wTmpSrcId==0xc9c5||  wTmpSrcId==0xc9b1 //购电后剩余金额	
				|| wTmpSrcId==0xc9d2) //4字节故障电量转成另一个5字节故障电量
			{
				pbDst[0] = 0x00;
				memcpy(pbDst+1, pbSrc, 4);
				iRet = 5;	
			}
			break;	
		case 0xd0:				
			if (wTmpSrcId==0xd000 || wTmpSrcId==0xd010) //结算电量1
			{
				pbDst[0] = 0x00;
				memcpy(pbDst+1, pbSrc, 4);
				iRet = 5;	
			}
			else if (wTmpSrcId==0xd02f || wTmpSrcId==0xd03f) //结算电量2
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

//描述：按协议格式将抄表的一个ID的数据入库
//		特殊需要转换的ID在处理转换后入库
//参数：@wPn  测量点号
//		@wID	数据ID
//		@pbBuf	要写入的数据缓冲区	
//		@wLen	要写入的数据长度
//		@dwTime	要写入的数据时标
//		@bErr	错误代码
//返回：成功则返回true
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
			if (wID1 == wID)	//原始ID要写转换数据
			{
				WriteItemTm(BN0, wPn, wID, bBuf, dwTime); //写原始ID	
				SaveLastRdMtrData(wID, wPn, bBuf);
			}
			else			//原始ID要写旧数据
			{
				WriteItemTm(BN0, wPn, wID, pbBuf, dwTime); //写原始ID
				WriteItemTm(BN0, wPn, wID1, bBuf, dwTime); //还需要写转换ID
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

				//对于07表没有总的最近一次断相起始时刻，需要根据分相的来构建
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
	            else if (bCnt == 3)//分相全为0时，最近一次也为0
	            	memset(pbBuf, 0, 4);
				else  //分相无效时最近一次断相也应是无效
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
		if (wID1 != wID)	//转换ID也要写出错信息
			UpdItemErr(BN0, wPn, wID1, bErr, dwTime);
	}

	return true;
}

//描述：按协议格式将抄表曲线数据直接入任务库
//		同时置上该点已入库的标志
//参数：@wPn  测量点号
//		@wID	数据ID
//		@pbBuf	要写入的数据缓冲区	
//		@wLen	要写入的数据长度
//		@dwTime	要写入的数据时标
//		@bErr	错误代码
//返回：成功则返回true
//备注：此处入口的数据为07协负荷曲线数据，内容为有效条数＋每笔（5字节时间＋数据内容）
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
	tLast.nYear = bIsSaveFlg[5]+2000;//记录的年
	tLast.nMonth = bIsSaveFlg[4];	//记录的月
	tLast.nDay = bIsSaveFlg[3];		//记录的日
	if ( IsDiffDay(&tNow, &tLast) ) //如果不是当天的曲线记录时标，则清零
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

		if ( IsDiffDay(&tNow, &time) ) //如果不是当天的曲线，则不存
		{
            if (pbBuf[0] != 0)
    			bLen += ((wLen-1)/pbBuf[0]); //每笔记录的实际长度
			continue;
		}

		bit = ((WORD)time.nHour*60 + time.nMinute)/bIntvMin; //按曲线保存间隔归整到当前相应的时间点 

		if ((bIsSaveFlg[(bit>>3)]&(1<<(bit&0x07))) == 0) //还未入库 只判本数据块的起始ID即可,因为子项数据都是一起写入的		
		{				
			time.nMinute = time.nMinute/bIntvMin*bIntvMin; //时间按曲线保存间隔归整
			TimeToFmt15(&time, mBuf);		//Fmt15 5字节
			memcpy(mBuf+5, &wPn, 1);		//测量点号 1字节	

			//以下是数据内容
			if (wID>=0x3701 && wID<=0x3704)	//正反相有无功电能要调整顺序
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
			else if (wID>=0x3745 && wID<=0x3748) //四象限要调整顺序
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
			else if (wID>=0x3681 && wID<=0x3688)//功率
			{				
				if ((bFn=GetFnFromCurveId(0x3681))==0xff)
					return false;

				for (j=0; j<8; j++)
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+5+j*3], 3);				
					fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);					
				}
			}
			else if (wID>=0x3689 && wID<=0x3694)//电压、电流
			{				
				if ((bFn=GetFnFromCurveId(0x3689))==0xff)
					return false;

				for (j=0; j<6; j++)
				{
					if (j < 3)
					{
						memcpy(mBuf+6, &pbBuf[bLen+1+5+(j<<1)], 2); //去掉频率
						fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 8);
					}
					else
					{
						memcpy(mBuf+6, &pbBuf[bLen+1+11+(j-3)*3], 3); //去掉频率				
						fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 9);
					}
				}
			}
			else if (wID>=0x3705 && wID<=0x3708)//功率因素
			{		
				if ((bFn=GetFnFromCurveId(0x3705))==0xff)
					return false;

				for (j=0; j<4; j++)
				{
					memcpy(mBuf+6, &pbBuf[bLen+1+5+(j<<1)], 2); //去掉频率				
					fSave = PipeAppend(TYPE_FRZ_TASK, bFn+j, mBuf, 8);
				}
			}	

			if (fSave)
			{
				bIsSaveFlg[(bit>>3)] = bIsSaveFlg[(bit>>3)] | (1<<(bit&0x07)); //每个块回来的子项数据都是一样的

				memcpy(g_ProfFrzFlg+n*6, bIsSaveFlg, sizeof(bIsSaveFlg));

				DTRACE(DB_TASK, ("CCommonTask::SaveMeterTask: wID =%02x time=%d-%d-%d  %2x-%2x-%2x\r\n", wID, bIsSaveFlg[5], bIsSaveFlg[4], bIsSaveFlg[3], bIsSaveFlg[2], bIsSaveFlg[1], bIsSaveFlg[0]));	
			}
			else 
				DTRACE(DB_TASK, ("CCommonTask::SaveMeterTask: id=%02x is SaveTask Fail\r\n", wID));		
		}
        if (pbBuf[0] != 0)
    		bLen += ((wLen-1)/pbBuf[0]); //每笔记录的实际长度
	}

	return true;
}


//描述：将抄表数据入库,对特殊ID做调整
//参数：@pMtrReq  相同需求ID队列的指针
//		@wNum	相同需求ID队列的个数
//		@pbBuf	要写入的数据缓冲区	
//		@wLen	要写入的数据长度
//		@dwCurT	要写入的数据时标
//		@bErr	错误代码
//返回：MTR_SAVE_OK/MTR_SAVE_RECLAIM
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
	
	//时标归整
	dwIntvT = GetMeterInterv(wPn)*60;
    if (dwIntvT == 0)
        return -1;
	dwSec = dwTime/dwIntvT*dwIntvT;//*60;

	if (wID==0x9a00) //上次日冻结时间
	{
		if (bErr==ERR_APP_OK)
		{
			Fmt15ToTime(pbBuf, &time);
			if (IsDiffDay(&time, &now))
			{
				DTRACE(DB_METER, ("SaveSameMeterItem: wPn=%d, ID=0x05060001, frz time mismatch, %d-%d-%d\n", 
					wPn, time.nYear, time.nMonth, time.nDay));
				SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, ERR_UNSUP);
				return -1;		//数据内容有错,需要对数据项进行回收重抄;
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
			SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, ERR_APP_OK);	//南网台体不支持此ID，为了过台体，这里直接取终端时间作为9a00
			DTRACE(DB_METER, ("SaveSameMeterItem : wPn=%d, ID=0x05060001 unsupport,  get term time, %d-%d-%d\n", 
				wPn, now.nYear, now.nMonth, now.nDay));
		}
	}
	else if (wID == 0xd800) //网络表的电压电流相位角ID,同时存到b66f
	{
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);
		SaveOneMtrItem(wPn, 0xb66f, pbBuf, wLen, dwSec, bErr);
	}
	else if (wID==0xd000 || wID==0xd010) //结算电量
	{			
		if (bErr != ERR_APP_OK)
			SaveOneMtrItem(wPn, wID+0xf, pbBuf, wLen, dwSec, bErr);
		else //写入结算块信息
		{
			bBuf[0] = 0;
			memcpy(bBuf+1, pbBuf, wLen);
			memset(bBuf+wLen+1, bInvdData, 25-1-wLen);
			SaveOneMtrItem(wPn, wID+0xf, bBuf, 25, dwSec, bErr);
		}
	}
	else if (wID == 0xc020)	//97版协议电表不支持07版645的其他电表状态字,这里同时写入以避免引起阻塞超时
	{				
		if (ReadItemEx(BN0, wPn, 0xc020, bFlg)>0 && bFlg[0]!=bInvdData) 	//这里只判断电表运行状态字1
			bBuf[0] = bFlg[0]^pbBuf[0];			
		else
			bBuf[0] = 0;

		//写入0xc020、0xc860、0xc880，其他不支持
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);
		SaveOneMtrItem(wPn, 0xc880, bBuf, 2, dwSec, bErr);			
	}
	else if ((wID>>4) == 0xc86)	//2007版645的电表状态字,这里同时判断电表状态字变位	
	{					
		if ((wID&0xf) == 0xf)
			bNum = wLen>>1;

		if (bErr != ERR_APP_OK)
			bNum = 0;
		
		for (n=0; n<bNum; n++)
		{			
			memset(bFlg, 0, sizeof(bFlg));
			if (ReadItemEx(BN0, wPn, (wID&0xfff0)+n, bFlg)>0 && !IsAllAByte(bFlg, bInvdData, 2)) //判断电表状态字变位	
			{
				bBuf[2*n] = bFlg[0]^pbBuf[2*n];
				bBuf[1+2*n] = bFlg[1]^pbBuf[1+2*n];
			}
			else
				memset(&bBuf[2*n], 0, 2);				
		}
		//写入电表状态字变位
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);
		SaveOneMtrItem(wPn, wID+0x20, bBuf, wLen, dwSec, bErr);		
	}		
	else if ( IsCurveId(wID))	//这里一定是07表的返回，因为非07的表的ID在提交时已经换成当前ID了
	{			
		//曲线的返回 内容为有效条数＋每笔（5字节时间＋数据内容）
		SaveMeterTask(wPn, wID, pbBuf, wLen, dwSec, bErr);	
		SaveOneMtrItem(wPn, wID, pbBuf, 1, dwSec, bErr);
	}
	else
	{
		SaveOneMtrItem(wPn, wID, pbBuf, wLen, dwSec, bErr);		
	}

	return 0;	//无错误		
}

