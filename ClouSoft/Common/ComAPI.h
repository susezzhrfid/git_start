/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ComAPI.h
 * 摘    要：本文件主要包含common目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：在common目录下放平台和应用无关的公用源文件和头文件
 *********************************************************************************************************/
#ifndef COMAPI_H
#define COMAPI_H

#include "ComConst.h"
#include "ComStruct.h"
#include "TypeDef.h"
#include "Info.h"
#include <string.h>

#ifdef SYS_WIN
#define BASETIME          1999
#else
#define BASETIME          1999
#endif
#define BASEWEEK		  5			//1999/1/1是星期5 用5表示

bool IsBcdCode(BYTE *p, WORD num);
BYTE BcdToByte(BYTE bcd);
BYTE ByteToBcd(BYTE b);
WORD BcdToWORD(BYTE * p);
DWORD BcdToDWORD(BYTE* p, WORD len);
DDWORD BcdToDDWORD(BYTE* p, WORD len);
WORD MbyteToWord(BYTE* pbBuf, WORD wLen);
DWORD ByteToDWORD(BYTE* pbBuf, WORD wLen);
void DWordToMbyte(DWORD val, BYTE* byte, WORD len);
WORD DWordToByte(DWORD dw, BYTE* p);
DWORD ByteToDWord(BYTE* p);
uint64 BcdToUint64(BYTE* p, WORD len);
int BcdToInt(BYTE* p, WORD len);
void IntToBCD(int val, BYTE* bcd, WORD len);
void DWORDToBCD(DWORD val, BYTE* bcd, WORD len);
void Uint64ToBCD(uint64 val, BYTE* bcd, WORD len);
void HexToASCII(BYTE* in, BYTE* out, WORD wInLen);
void ByteToASCII(BYTE b, BYTE** pp);
//void ByteXtoASCII(BYTE b, BYTE** pp);
BYTE AsciiToByte(BYTE** pp);
bool IsAllAByte(const BYTE* p, BYTE b, WORD len);
DWORD SearchStrVal(char* pStart, char* pEnd);
BYTE* bufbuf(BYTE* pbSrc, WORD wSrcLen, BYTE* pbSub, WORD wSubLen);
BYTE CheckSum(BYTE* p, DWORD dwLen);

void GetCurTime(TTime* pTime);
DWORD GetCurSec();
DWORD GetCurMinute();
DWORD GetCurHour();

DWORD DaysFrom2000(const TTime* pTm);
DWORD MinutesFrom2000(const TTime* ptPast);
DWORD MonthFrom2000(const TTime* ptPast);
DWORD MonthsPast(const TTime* ptPast, const TTime* ptNow);
int DaysPast(const TTime* ptPast, const TTime* ptNow);
DWORD HoursPast(const TTime* ptPast, const TTime* ptNow);
DWORD MinutesPast(const TTime* ptPast, const TTime* ptNow);
DWORD SecondsPast(const TTime* ptPast, const TTime* ptNow);
bool IsInvalidTime(const TTime* pTm);
DWORD TimeToSeconds(const TTime* pTm);
DWORD TimeToMinutes(const TTime* pTm);
DWORD TimeToHours(const TTime* pTm);
int dayOfWeek(int year,int month,int day);
void SecondsToTime(DWORD dwSeconds, TTime* pTime);
void MinutesToTime(DWORD dwMins, TTime* pTime);
void DaysToTime(DWORD dwDays, TTime* pTime);
void MonthsToTime(DWORD dwMonths, TTime* pTime);
bool IsTimeEmpty(const TTime* pTm);
bool IsDiffDay(const TTime* pTm1, const TTime* pTm2);
bool IsSameDay(const TTime* pTm1, const TTime* pTm2);
bool AddIntervs(TTime* pTm, BYTE bIntervU, int iIntervV);
int IntervsPast(const TTime* pPast, const TTime* pNow, BYTE bIntervU, BYTE bIntervV);
bool AddIntervsInTask(TTime* pTm, BYTE bIntervU, int iIntervV);
int TaskIntervsPast(const TTime* pPast, const TTime* pNow, BYTE bIntervU, BYTE bIntervV);

BYTE DayOfWeek(const TTime* pTm);
WORD ByteToWord(BYTE* p);
WORD WordToByte(WORD w, BYTE* p);
char* TimeToStr(TTime time, char* psz);
char* IntervUToStr(BYTE bIntervU, char* psz);
char* TaskTypeToStr(BYTE bType, char* psz);

int64 Pow(int iBase, WORD wExp);
WORD CalcuBitNum(const BYTE* pbBuf, WORD wSize);
int DoMtrFwdFunc(BYTE bPort, WORD wTestId, BYTE bMtrPro, BYTE* pbMtrAddr, BYTE* pbRxFrm, BYTE bRxBufSize);
BYTE Make645AskItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm);
WORD Make645Frm(BYTE* pbFrm, const BYTE* pbAddr, BYTE bCmd, BYTE *pbData,BYTE bDataLen);
WORD get_crc_16 (WORD start, BYTE *p, int n);

void Swap(BYTE *pbBuf, WORD wLen);
BYTE memrcpy(BYTE *pbDst,BYTE *pbSrc,WORD wLen);
int BufToAllTime(BYTE* pbBuf, TTime* time);
DWORD tmBufToSecond(BYTE* pbBuf);

char* MtrAddrToStr(const BYTE* pbAddr, char* psz);

#define ABS(x) ((x)>=0 ? (x) : -(x))

#endif //COMAPI_H

