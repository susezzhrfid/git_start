/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbGbAPI.cpp
 * 摘    要：本文件主要实现数据库的数据结构定义
 * 当前版本：1.0
 * 作    者：魏红
 * 完成日期：2007年4月
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

//描述：读取系统库数据
//参数：@ bFn 国标数据各类定义的信息类型
//      @ bPtr 读取指针
//      @ pbBuf 数据输出缓存
//      @ pwLen 数据长度指针
//返回：大于等于0表示成功，小于0表示失败
//备注：按照国标给出实际的长度,可以使得原协议接口不用更改
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
		//获取总共存储的笔数
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

			if(wRecTotalNum == wTotal)//防止读取无效数据
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
					//判断ID	
					if(dwId != dwTempId)
						continue;
				}
				//判断PN
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
			if((dwSec>=dwStartm && dwSec<dwEndtm)//前闭后开区间
				|| (dwStartm==0xffffffff && dwEndtm==0xffffffff))
			{
				if(((wReadLen+wAlrLen+6) > iTxBufLeft) &&
					(!IsEventId(dwTempId) || memcmp(p, g_ExFlashBuf+7, 6)!=0))
				{
					(*wRdNum)--;//由于空间不足退出后，会将当前读到数据丢掉，下次进来重读
					SignalSemaphore(g_semExFlashBuf);
					//return GB_RDERR_NOROOM;
                    return wReadLen;    //xzz
				}


				IdPnToBytes(dwTempId, wTempPn, g_ExFlashBuf+ALRSECT_SIZE);
				if(IsEventId(dwTempId) && memcmp(p, g_ExFlashBuf+ALRSECT_SIZE, 6)==0  
					&& memcmp(p+6, g_ExFlashBuf+7, 6)==0 && IsAllAByte(p+12, INVALID_DATA, wAlrLen-6))
				{
					//对于事件，发生且有恢复时，不用管发生了
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
//需量时间ID检测
WORD IsDemandBlockId(DWORD dwId)
{
	WORD wBlockId = 0;
	switch(dwId)
	{
	case 0x0101ff00://当前正向有功最大需量发生时间数据块
		wBlockId = 0xb01f;
		break;
	case 0x0102ff00://当前反向有功最大需量发生时间数据块
		wBlockId = 0xb02f;
		break;
	case 0x0103ff00://当前组合无功1最大需量发生时间数据块
		wBlockId = 0xb11f;
		break;
	case 0x0104ff00://当前组合无功2最大需量发生时间数据块
		wBlockId = 0xb12f;
		break;
	case 0x0105ff00://当前第一象限最大需量发生时间数据块
		wBlockId = 0xb13f;
		break;
	case 0x0106ff00://当前第二象限最大需量发生时间数据块
		wBlockId = 0xb15f;
		break;
	case 0x0107ff00://当前第三象限最大需量发生时间数据块
		wBlockId = 0xb16f;
		break;
	case 0x0108ff00://当前第四象限最大需量发生时间数据块
		wBlockId = 0xb14f;
		break;
	case 0x0109ff00://当前正向视在最大需量发生时间数据块
		wBlockId = 0xb03f;
		break;
	case 0x010aff00://当前反向视在最大需量发生时间数据块
		wBlockId = 0xb04f;
		break;

	case 0x0101ff01://月冻结正向有功最大需量发生时间数据块
		wBlockId = 0xb41f;
		break;
	case 0x0102ff01://月冻结反向有功最大需量发生时间数据块
		wBlockId = 0xb42f;
		break;
	case 0x0103ff01://月冻结组合无功1最大需量发生时间数据块
		wBlockId = 0xb51f;
		break;
	case 0x0104ff01://月冻结组合无功2最大需量发生时间数据块
		wBlockId = 0xb52f;
		break;
	case 0x0105ff01://月冻结第一象限最大需量发生时间数据块
		wBlockId = 0xb53f;
		break;
	case 0x0106ff01://月冻结第二象限最大需量发生时间数据块
		wBlockId = 0xb55f;
		break;
	case 0x0107ff01://月冻结第三象限最大需量发生时间数据块
		wBlockId = 0xb56f;
		break;
	case 0x0108ff01://月冻结第四象限最大需量发生时间数据块
		wBlockId = 0xb54f;
		break;
	case 0x0109ff01://月冻结正向视在最大需量发生时间数据块
		wBlockId = 0xb43f;
		break;
	case 0x010aff01://月冻结反向视在最大需量发生时间数据块
		wBlockId = 0xb44f;
		break;

	case 0x050609ff://日冻结正向有功最大需量发生时间数据块
		wBlockId = 0x9c8f;
		break;
	case 0x05060aff://日冻结反向有功最大需量发生时间数据块
		wBlockId = 0x9caf;
		break;
	case 0x05060bff://日冻结组合无功1最大需量发生时间数据块
		wBlockId = 0x9c9f;
		break;
	case 0x05060cff://日冻结组合无功2最大需量发生时间数据块
		wBlockId = 0x9cbf;
		break;
	}

	return wBlockId;
}

//为需量时间添加年
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
//描述：给通信协议调用的读取1类数据接口，1次调用只能读取一个数据项
//参数：@pbTx 用来返回读取到的数据内容
//		@iTxBufSize 发送缓冲区剩下的大小
//返回：正确则返回数据长度，一切正常只是无数据则返回0，
//		读失败则返回-GB_RDERR_FAIL，空间不够则返回-GB_RDERR_NOROOM
//备注：1类的小时数据转换成相应的2类后，调用本函数
int SgReadClass1(DWORD dwId, WORD wPn, BYTE* pbTx, int iTxBufSize, bool fRptState)
{
	int iLen, iRet;
	DWORD dwRdSec;
	TProIdInfo ProIdInfo = {0};
	const WORD* pwSubID;
	WORD wBN, wID;	//转换后的标识
	BYTE bRdMode;	//变长
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
	//先取出dwID的信息
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
			
			GetCurTime(&now); //时间统一在这里获取，保证读写数据在同一抄表间隔
			wID = pwSubID[i];
			if (IsMtrID(wID) && !MeterSpecierId(dwId))
			{
				iRet = ReadItemEx(BN24, PN0, 0x4105, &bRdMode);
				if (iRet>0 && bRdMode==3 && !fRptState)	//实时读取电表，参数F38同样无效
				{							//主动上送不用直接抄，必须把间隔抄的数据导入进来
					g_bDirRdStep = 1;
					iThrId = DoDirMtrRd(wBN, wPn, wID, now);
					g_bDirRdStep = 0;
					if (iThrId < 0)
						return -GB_RDERR_FAIL;
				}
				else
				{
					iThrId = GrabPnThread(wPn);	//GetPnThread
					SetDynPn(iThrId, wPn);	//设置系统库，缓存该测量点的数据
				}
			}

			if (fMtrPn && !IsPnDataLoaded(wPn))	//测量点数据还没导入进内存
			{
				if (!LoadPnDirData(wPn))	//导入测量点直抄数据
				{
					DTRACE(DB_FAPROTO, ("GbReadClass1: fail to rd pn=%d's data!\r\n", wPn));
					return -GB_RDERR_FAIL;
				}
			}

			dwRdSec = GetCurIntervSec(wPn, &now);   //贴上当前抄表间隔时标
			if (fRptState && dwRdSec==TimeToMinutes(&now)*60)
				dwRdSec -= GetMeterInterv(wPn)*60;

			if(MeterSpecierId(dwId))
			{
				dwRdSec = 0;
			}

			//if(GetItemLen(wBN,pwSubID[i]) > 0)
			//{
				ReadItemTm(wBN, wPn, pwSubID[i], pbTx+iLen, dwRdSec, INVALID_TIME);	//没读到的数据都已经置成无效数据
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
		{///读谐波块数据
			//BYTE *pTmpPtr = bBuf;
			//BYTE bTmpBuf[44];
			//memcpy(bTmpBuf, pTmpPtr+2, 42);
			memcpy(pbTx+2, pbTx+4, 40);
			if(ProIdInfo.dwId==ProIdInfo.dwProId)//20140627-1
				ProIdInfo.wDataLen -= GetItemLen(wBN,pwSubID[i]);//20140320-1
		}

		if(ProIdInfo.bIdx == INVALID_DATA && ProIdInfo.wOffset==0xFFFF) //子id超出目前块id所支持的范围,没赋正确的偏移默认成无效数据
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


//修正缺省值!,列表如下，之后修正费率和谐波次数值！
//总结4个组：
//A: Td+抄表时间+费率 1-4,9-12,17-20
//B: Td+费率 5-8,21-24,58-59,61-62
//C: Td+谐波次数 121-123
//D：其它
typedef struct 
{
	BYTE bFN;//标识
	BYTE bDefaultStyle;//缺省值类型定义 0:全0xee; 1:全0; 2:部分0
	WORD wOff;//部分0的起始位置
	WORD wLen;//部分0的连续长度
}TC2DefaultValDesc;

const TC2DefaultValDesc C2DefaultValDesc[]=
{//标识		缺省值类型	起始位置  长度
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

//针对数据区填无效数据，但费率不填无效
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


//描述：给通信协议调用的读取2类数据接口
//参数：@pbRx 接收帧缓冲区，用来传递下发帧中要读FN的附加数据内容，如冻结类数据时标，
//			  如果进来多次，pbRx所指向的内容不变
//		@pbTx 用来返回读取到的数据内容
//		@iTxBufSize 发送缓冲区剩下的大小
//		@wStart 本次读取数据的开始索引，针对多次读取的情况，比如曲线类数据要求的数据点数中，本次读取从第wStart点开始
//				它在读取的过程中，自动递增，下次再进来的时候接着上次的
//返回：正确则返回数据长度，一切正常只是无数据则返回0，
//		读失败则返回-GB_RDERR_FAIL，空间不够则返回-GB_RDERR_NOROOM
//备注：1类的小时数据转换成相应的2类后，调用本函数
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
	WORD wBN, wPoint, wID;	//转换后的标识
	BYTE bFn;
	BYTE bRate;
	BYTE bIntervU;
	int iIntervNum;
	BYTE bRateNum = RATE_NUM;

	bFn = FindFn(dwID);
	if (bFn == 0)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: FN is 0!\r\n"));
		return -GB_RDERR_FAIL;		//读失败
	}
		

	if (wPn != 0)
	{
		bool fMtrPn = IsPnValid(wPn);
		if (!fMtrPn)
		{
			DTRACE(DB_TASKDB, ("TdbReadRec: PN is INVALID!\r\n"));
			return -GB_RDERR_FAIL;		//读失败
		}
			
	}

	if (iTxBufSize < 0)
		return -GB_RDERR_NOROOM;

	pTaskCtrl = ComTaskFnToCtrl(bFn);
	if (pTaskCtrl==NULL)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: TaskCtrl is NULL!\r\n"));
		return -GB_RDERR_FAIL;		//读失败
	}
		

	wRecDataLen = ComTaskGetDataLen(pTaskCtrl);

	memset(&tmStart, 0, sizeof(tmStart));
	memset(&tmEnd, 0, sizeof(tmEnd));
	if (DATA_TIME_LEN+wRecDataLen > iTxBufSize)
		return -GB_RDERR_NOROOM;  //空间不够
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
		return -GB_RDERR_FAIL;   //读失败
		//			TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx数据后加时标
	}

	if (TdbOpenTable(bFn) != TDB_ERR_OK)
	{
		if (fRptState)	//主动上报无效数据处理，且只上报一条无效记录
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
				
			TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx数据后加时标 ，时标不填无效数据
			pbTx += wRecDataLen+6;
			(*pwStart)++;			
			iRet = pbTx - pbTx0;  
            TdbCloseTable(bFn);
			return iRet;
		}
        TdbCloseTable(bFn);
		DTRACE(DB_TASKDB, ("TdbReadRec: Can not open File!\r\n"));
		return -GB_RDERR_FAIL;		//读失败
	}    

	for (i=0; i<wNumToRd; i++)
	{
		if (pbTx-pbTx0+wRecDataLen+6 > iTxBufSize)  //第2笔后超长，上面已经保证第一笔不会超长
			break;
		
		iRdLen = TdbReadRec(bFn, wPn, tmStart, pbTx);
		if (iRdLen > 0)
		{
				if(m_tProIdInfo.bIdx == INVALID_DATA && m_tProIdInfo.wOffset==0xFFFF) //子id超出目前块id所支持的范围,没赋正确的偏移默认成无效数据
				{
					memset(pbTx,INVALID_DATA,m_tProIdInfo.wDataLen);
					TimeToBcd(&tmStart,pbTx+m_tProIdInfo.wDataLen);//pbTx数据后加时标
					pbTx += (m_tProIdInfo.wDataLen+6);                //数据长度加6个字节时标
				}
				else if (m_tProIdInfo.bIdType == 0)
				{
					//memcpy(pbTx,pbTx+(m_tProIdInfo.wOffset-1)*m_tProIdInfo.wDataLen+1,m_tProIdInfo.wDataLen);
					memcpy(pbTx,pbTx+m_tProIdInfo.wOffset,m_tProIdInfo.wDataLen);
					TimeToBcd(&tmStart,pbTx+m_tProIdInfo.wDataLen);//pbTx数据后加时标
					pbTx += (m_tProIdInfo.wDataLen+6);                //数据长度加6个字节时标
				}
				else if(m_tProIdInfo.bIdType == T_BLK || m_tProIdInfo.bIdType == T_FEE_BLK)  
				{
					if (IsAllAByte(pbTx, INVALID_DATA, iRdLen) && (m_tProIdInfo.bIdType == T_FEE_BLK))
						pbTx[0] = 4;	//费率个数

					TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx数据后加时标
					pbTx += (wRecDataLen+6);                //数据长度加6个字节时标
				}

		}
		else
		{
			if (m_tProIdInfo.bIdType == 0)                   //子ID
			{
				SetC2DefaultVal(bFn, wPn, pbTx, 1, m_tProIdInfo.wDataLen);
				TimeToBcd(&tmStart,pbTx+m_tProIdInfo.wDataLen);//pbTx数据后加时标
				pbTx += (m_tProIdInfo.wDataLen+6);                //数据长度加6个字节时标
			}
			else  if(m_tProIdInfo.bIdType == T_BLK || m_tProIdInfo.bIdType == T_FEE_BLK)                                          //块ID
			{
				SetC2DefaultVal(bFn, wPn, pbTx, 1, wRecDataLen);
				if (m_tProIdInfo.bIdType == T_FEE_BLK)
					pbTx[0] = bRateNum;

				TimeToBcd(&tmStart,pbTx+wRecDataLen);//pbTx数据后加时标
				pbTx += (wRecDataLen+6);                //数据长度加6个字节时标
			}
		}
		
		AddIntervs(&tmStart, pTaskCtrl->bIntervU, 1);
		//pbTx += wRecDataLen+6;                //数据长度加6个字节时标
		(*pwStart)++;
	}
	iRet = pbTx - pbTx0;
	TdbCloseTable(bFn);

	return iRet;
}

