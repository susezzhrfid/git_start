/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ComAPI.cpp
 * 摘    要：本文件主要包含common目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：在common目录下放平台和应用无关的公用源文件和头文件
 *********************************************************************************************************/
#include <string.h>
#include <stdio.h>
#include "SysCfg.h"
#include "ComAPI.h"
#include "FaAPI.h"
#include "SysDebug.h"
#include "SysAPI.h"
#include "FaCfg.h"

bool IsBcdCode(BYTE *p, WORD num)
{
	int i;
	BYTE ch;
	for(i=0; i<num; i++)
	{
		ch = p[i]&0xf;
		if (ch > 9)
			return false;

		ch = p[i]>>4;
		if (ch > 9)
			return false;
	}
	return true;
}

BYTE BcdToByte(BYTE bcd) 
{
	return ((bcd >> 4) & 0x0f) * 10 + (bcd & 0x0f);
}

BYTE ByteToBcd(BYTE b)
{
	return (b/10<<4) + b%10;
}

WORD ByteToWord(BYTE* p)
{
	WORD val = 0;
	memcpy(&val, p, 2);
	return val;
}

WORD WordToByte(WORD w, BYTE* p)
{
	*p++ = (BYTE )(w & 0xff);
	*p = (BYTE )(w >> 8);

	return 2;
}

WORD BcdToWORD(BYTE * p) 
{
	WORD val;
	BYTE* p0 = p;
	p0 += 1;

	val = 0;
	for (; p<=p0; p++)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	return val;
}

DWORD BcdToDWORD(BYTE* p, WORD len)
{
	DWORD val;
	BYTE* p0 = p;
	p += len - 1;

	val = 0;
	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	return val;
}

DDWORD BcdToDDWORD(BYTE* p, WORD len)
{
	DDWORD val;
	BYTE* p0 = p;

	p += len - 1;

	val = 0;
	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	return val; 
}

WORD MbyteToWord(BYTE* pbBuf, WORD wLen)
{
	WORD val = 0;
	if (wLen > 2)
		wLen = 2;

	memcpy(&val, pbBuf, wLen);
	return val;
}

DWORD ByteToDWORD(BYTE* pbBuf, WORD wLen)
{
	DWORD val = 0;
	if (wLen > 4)
		wLen = 4;
		
	memcpy(&val, pbBuf, wLen);
	return val;
}

uint64 BcdToUint64(BYTE* p, WORD len)
{
	uint64 val;
	BYTE* p0 = p;
	p += len - 1;

	val = 0;
	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	return val;
}

//参数：@len bcd的字节长度
int BcdToInt(BYTE* p, WORD len)
{
	int val;
	BYTE* p0 = p;
	bool fNeg;

	p += len - 1;
	val = (*p & 0x0f);

	
	if ((*p&0xf0) != 0)
		fNeg = true;
	else
		fNeg = false;

	p--;

	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	if (fNeg)
	    return -val;
	else
		return val;
}

void IntToBCD(int val, BYTE* bcd, WORD len)
{
	int power;
	WORD i;
	bool fNeg = false;
	if (val < 0)
	{
		val = - val;
		fNeg = true;
	}

	power = 1;
	for (i=0; i<len-1; i++)
	{
		power *= 100; 
	}

	power *= 10;  
    if (power == 0)
        return;
	val %= power;

	power /= 10;

    if (power == 0)
        return;
	if (fNeg)
		bcd[len - 1] = 0x10 + val / power;
	else
		bcd[len - 1] = val / power;

	len--;

	for (; len>0; len--)
	{
		BYTE bHigh, bLow;
	    val %= power;
	    power /= 10;
        if (power == 0)
            return;
		bHigh = val / power;

	    val %= power;
	    power /= 10;
        if (power == 0)
            return;
		bLow = val / power;
		bcd[len - 1] = (bHigh << 4) | bLow;
	}
}

void DWORDToBCD(DWORD val, BYTE* bcd, WORD len)
{
	DWORD power;
	WORD i;
	BYTE bHigh, bLow;

	memset(bcd,0,len);
	if (len > 4)//处理最高字节
	{
		power = 1000000000;
		bHigh = (BYTE )(val / power);
	    val %= power;

		power = 100000000;
		bLow = (BYTE )(val / power);
	    val %= power;
		bcd[4] = (bHigh << 4) | bLow;

		len = 4;
	}

	power = 1;
	for (i=0; i<len; i++)
	{
		power *= 100; 
	}

    if (power == 0)
        return;
    
	for (; len>0; len--)
	{
	    val %= power;
	    power /= 10;
        if (power == 0)
            return;
		bHigh = (BYTE )(val / power);

	    val %= power;
	    power /= 10;
        if (power == 0)
            return;
		bLow = (BYTE )(val / power);
		bcd[len - 1] = (bHigh << 4) | bLow;
	}
}

void DWordToMbyte(DWORD val, BYTE* byte, WORD len)
{
	if( len > 4 ) len = 4;
	for(; len>0; byte++, len--)
	{
		*byte = (BYTE)val%0x100;
		val = val/0x100;
	}
}

WORD DWordToByte(DWORD dw, BYTE* p)
{
	*p++ = dw & 0xff;
	*p++ = (dw >> 8) & 0xff;
	*p++ = (dw >> 16) & 0xff;
	*p = (dw >> 24) & 0xff;
	return 4;
}

DWORD ByteToDWord(BYTE* p)
{
	DWORD dw = 0;
	memcpy(&dw, p, 4);
	return dw;
	//return *p + (DWORD )*(p + 1)*0x100 + (DWORD )*(p + 2)*0x10000 + (DWORD )*(p + 3)*0x1000000;
}

void Uint64ToBCD(uint64 val, BYTE* bcd, WORD len)
{
	uint64 power;
	WORD i;
	BYTE bHigh, bLow;

	memset(bcd, 0, len);

	power = 1;
	for (i=0; i<len; i++)
	{
		power *= 100; 
	}
    
    if (power == 0)
        return;
    
	for (; len>0; len--)
	{
	    val %= power;
	    power /= 10;
        if (power == 0)
            return;
		bHigh = (BYTE )(val / power);

	    val %= power;
	    power /= 10;
        if (power == 0)
            return;
		bLow = (BYTE )(val / power);
		bcd[len - 1] = (bHigh << 4) | bLow;
	}
}

void HexToASCII(BYTE* in, BYTE* out, WORD wInLen)
{
	WORD i;
    for (i=0; i<wInLen; i++)
    {
		BYTE b = *in++;
		BYTE hi = b >> 4;
		BYTE lo = b & 0x0f;
		if (hi >= 0x0a)
			*out++ = hi - 0x0a + 'A';
		else
			*out++ = hi + '0';

		if (lo >= 0x0a)
			*out++ = lo - 0x0a + 'A';
		else
			*out++ = lo + '0';
	}
}

void ByteToASCII(BYTE b, BYTE** pp)
{
	BYTE* p = *pp;

	BYTE hi = b >> 4;
	BYTE lo = b & 0x0f;
	if (hi >= 0x0a)
		*p++ = hi - 0x0a + 'A';
	else
		*p++ = hi + '0';

	if (lo >= 0x0a)
		*p++ = lo - 0x0a + 'A';
	else
		*p++ = lo + '0';

	*pp = p;
}
/*
void ByteXtoASCII(BYTE b, BYTE** pp)
{
	BYTE* p = *pp;

	BYTE lo = b >> 4;
	BYTE hi = b & 0x0f;

	if (hi >= 0x0a)
		*p++ = hi - 0x0a + 'A';
	else
		*p++ = hi + '0';

	if (lo >= 0x0a)
		*p++ = lo - 0x0a + 'A';
	else
		*p++ = lo + '0';

	*pp = p;
}*/

BYTE AsciiToByte(BYTE** pp)
{
	BYTE* p = *pp;
	BYTE hi = 0, lo = 0;

	if (*p>='A' && *p<='F')
		hi = *p - 'A' + 0x0a;
	else if (*p>='a' && *p<='f')
		hi = *p - 'a' + 0x0a;
	else if (*p>='0' && *p<='9')
		hi = *p - '0';
	else
		return 0;

	p++;

	if (*p>='A' && *p<='F')
		lo = *p - 'A' + 0x0a;
	else if (*p>='a' && *p<='f')
		lo = *p - 'a' + 0x0a;
	else if (*p>='0' && *p<='9')
		lo = *p - '0'; 
	else
		return hi;

	p++;

	*pp = p;

	return (hi << 4) | lo;
}

bool IsAllAByte(const BYTE* p, BYTE b, WORD len)
{
	WORD i;
	for (i=0; i<len; i++)
	{
		if (*p++ != b)
			return false;
	}

	return true;
}

DWORD SearchStrVal(char* pStart, char* pEnd)
{
	bool fGetFirst = false;
	DWORD val = 0;
	while (pStart < pEnd)
	{
		char c = *pStart++;
		if (!fGetFirst)
		{
			if (c>='0' && c<='9')
			{
				fGetFirst = true;
			}
		}

		if (fGetFirst)
		{
			if (c>='0' && c<='9')
			{
				val = val*10 + c - '0';
			}
			else
			{
				break;
			}
		}

	}

	return val;
}

BYTE* bufbuf(BYTE* pbSrc, WORD wSrcLen, BYTE* pbSub, WORD wSubLen)
{
	BYTE* pbSrcEnd = pbSrc + wSrcLen;
	while (pbSrc+wSubLen <= pbSrcEnd)
	{
		if (memcmp(pbSrc, pbSub, wSubLen) == 0)
			return pbSrc;

		pbSrc++;
	}

	return NULL;
}

BYTE CheckSum(BYTE* p, DWORD dwLen)
{
	BYTE bSum = 0;	
	for (; dwLen > 0; dwLen--)
	{
		bSum += *p++;
	}	
	return bSum;
}

void GetCurTime(TTime* pTime)
{
	GetSysTime(pTime);

	/*printf("GetCurTime : %02d/%02d/%02d %02d::%02d::%02d.\r\n", 
							pTime->nYear, pTime->nMonth, pTime->nDay, 
					  		pTime->nHour, pTime->nMinute, pTime->nSecond);*/

	if (IsInvalidTime(pTime))
	{
		DTRACE(DB_GLOBAL, ("GetCurTime : error time %d-%d-%d %d:%d:%d.\r\n", 
					  		pTime->nYear, pTime->nMonth, pTime->nDay, 
					  		pTime->nHour, pTime->nMinute, pTime->nSecond));
	}
}

//描述：返回当前距离2000/1/1 00:00:00 的秒数//1999-1-1 00:00:00
DWORD GetCurSec()
{
	TTime now;
	GetCurTime(&now);

	return DaysFrom2000(&now)*60*60*24 +
		   (DWORD )(now.nHour)*60*60 +
		   (DWORD )now.nMinute*60 + now.nSecond;
}

//描述：返回当前距离2000/1/1 00:00:00 的分钟数
DWORD GetCurMinute()
{
	TTime now;
	GetCurTime(&now);

	return DaysFrom2000(&now)*60*24 +
		   (DWORD )(now.nHour)*60 +
		   now.nMinute;
}

//描述：返回当前距离2000/1/1 00:00:00 的小时数
DWORD GetCurHour()
{
	TTime now;
	GetCurTime(&now);

	return DaysFrom2000(&now)*24 + now.nHour;
}

const WORD g_wDaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                         //1   2   3   4   5   6   7   8   9   10  11  12   

//描述：计算从2000/1/1到year/month/day的天数
DWORD DaysFrom2000(const TTime* pTm)
{
	int y, m, d, days;
	if (IsTimeEmpty(pTm))
	{
		return 0;
	}
	
	y=BASETIME;
	m=1;
	d=1;

	days = 0;
	while (y < pTm->nYear)
	{
		if ((y%4==0 && y%100!=0) || y%400==0)   //闰年
			days += 366;
		else
			days += 365;

		y++;
	}

	//现在：y == year
	while (m < pTm->nMonth)
	{
		if (m == 2)
		{
			if ((y%4==0 && y%100!=0) || y%400==0)   //闰年
				days += 29;
			else
				days += 28;

		}
		else
		{
			days += g_wDaysOfMonth[m-1];
		}

		m++;
	}

	//现在：m == month
	days += pTm->nDay - d;

	return days;
}

DWORD MinutesFrom2000(const TTime* ptPast)
{
	DWORD dwPast = DaysFrom2000(ptPast) * 60 * 24;
	dwPast += (DWORD )(ptPast->nHour)*60 + ptPast->nMinute;

	return dwPast;
}

DWORD MonthFrom2000(const TTime* ptPast)
{
	return (ptPast->nYear-BASETIME)*12+ptPast->nMonth-1;
}

//一年的月数：0x1D5'5600
DWORD MonthsPast(const TTime* ptPast, const TTime* ptNow)
{
	DWORD dwPast = (ptPast->nYear - BASETIME) * 12 + ptPast->nMonth - 1;
	DWORD dwNow = (ptNow->nYear - BASETIME) * 12 + ptNow->nMonth - 1;
	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}

//一年的秒数：0x1D5'5600
int DaysPast(const TTime* ptPast, const TTime* ptNow)
{
	int iPast = (int )DaysFrom2000(ptPast);
	int iNow = (int )DaysFrom2000(ptNow);
	return iNow - iPast;
}

//一年的秒数：0x1D5'5600
DWORD HoursPast(const TTime* ptPast, const TTime* ptNow)
{
	DWORD dwNow;
	DWORD dwPast = DaysFrom2000(ptPast) * 24;
	dwPast += ptPast->nHour;

	dwNow = DaysFrom2000(ptNow) * 24;
	dwNow += ptNow->nHour;

	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}

//一年的秒数：0x1D5'5600
DWORD MinutesPast(const TTime* ptPast, const TTime* ptNow)
{
	DWORD dwNow;
	DWORD dwPast = DaysFrom2000(ptPast) * 60 * 24;
	dwPast += (DWORD )(ptPast->nHour)*60 + ptPast->nMinute;

	dwNow = DaysFrom2000(ptNow) * 60 * 24;
	dwNow += (DWORD )(ptNow->nHour)*60 + ptNow->nMinute;

	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}

/*
int MunitesSub(const TTime& time1, const TTime& time2)
{
	int t1 = DaysFrom2000(time1) * 60 * 24;
	t1 += (int )(pTm1->nHour)*60 + pTm1->nMinute;

	int t2 = DaysFrom2000(time2) * 60 * 24;
	t2 += (int )(pTm2->nHour)*60 + pTm2->nMinute;
	
	return t1 - t2;
}*/

DWORD SecondsPast(const TTime* ptPast, const TTime* ptNow)
{
	DWORD dwNow;
	DWORD dwPast = DaysFrom2000(ptPast) * 60 * 60 * 24;
	dwPast += (DWORD )(ptPast->nHour)*60*60 + (DWORD )ptPast->nMinute*60 + ptPast->nSecond;

	dwNow = DaysFrom2000(ptNow) * 60 * 60 * 24;
	dwNow += (DWORD )(ptNow->nHour)*60*60 + (DWORD )ptNow->nMinute*60 + ptNow->nSecond;

	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}

bool IsInvalidTime(const TTime* pTm)
{
	if (pTm->nYear<2000 || pTm->nYear>2100 ||
		pTm->nMonth<1 || pTm->nMonth>12 ||
		pTm->nDay<1 || pTm->nDay>31 ||
		pTm->nHour>23 ||
		pTm->nMinute >= 60 ||
		pTm->nSecond >= 60)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SecondsToTime(DWORD dwSeconds, TTime* pTime)
{
	int year=BASETIME, month=1, day=1, hour=0, minute=0, second=0;
	DWORD delta;

	while (dwSeconds > 0)
	{
		if ((year%4==0 && year%100!=0) || year%400==0)   //闰年
			delta = 366*24*60*60;
		else
			delta = 365*24*60*60;

		if (dwSeconds < delta)
		{
			break;
		}
		else
		{
			dwSeconds -= delta;
			year++;
		}
	}	

	while (dwSeconds > 0)
	{
		if (month == 2)
		{
			if ((year%4==0 && year%100!=0) || year%400==0)   //闰年
				delta = 29*24*60*60;
			else
				delta = 28*24*60*60;
		}
		else
		{
			delta = g_wDaysOfMonth[month-1]*24*60*60;
		}
		
		if (dwSeconds < delta)
		{
			break;
		}
		else
		{
			dwSeconds -= delta;
			month++;
		}
	}

	if (dwSeconds > 0)
	{
		day = dwSeconds / (24*60*60);
		
		dwSeconds -= day * 24 * 60 * 60;
		
		day++;
		
		if (dwSeconds > 0)
		{
			hour = dwSeconds / (60*60);
			
			dwSeconds -= hour * 60 * 60;
			
			if (dwSeconds > 0)
			{
				minute = dwSeconds / 60;
				second = dwSeconds - minute * 60;
			}
		}
	}
	
	pTime->nYear = year;
	pTime->nMonth = month;
	pTime->nDay = day;
	pTime->nHour = hour;
	pTime->nMinute = minute;
	pTime->nSecond = second;
//	pTime->nWeek = dayOfWeek(year, month, day) + 1;
	pTime->nWeek = DayOfWeek(pTime);
	
}

void MinutesToTime(DWORD dwMins, TTime* pTime)
{
	SecondsToTime(dwMins*60, pTime);
}

void DaysToTime(DWORD dwDays, TTime* pTime)
{
	SecondsToTime( dwDays * 24 * 3600, pTime );
}

void MonthsToTime(DWORD dwMonths, TTime* pTime)
{
	pTime->nYear = (WORD)( ( dwMonths / 12 ) + BASETIME );
	pTime->nMonth = (BYTE)( ( dwMonths % 12 ) + 1 );
	pTime->nDay = 1;
	pTime->nHour = 0;
	pTime->nMinute = 0;
	pTime->nSecond = 0;
}

/*
int dayOfWeek(int year,int month,int day)
{   
	int _month[12]={31,0,31,30,31,30,31,31,30,31,30,31};
    if (year%4==0 && year%100!=0 || year%400==0)
       _month[1]=29;
    else _month[1]=28;
    int C=0;
    for (int i=0;i<month-1;++i)
      C+=_month[i];
    C+=day;
    int S=year-1+(year-1)/4-(year-1)/100+(year-1)/400+C;
    return S%7;
}
*/

DWORD TimeToSeconds(const TTime* pTm)
{
	return DaysFrom2000(pTm)*60*60*24 +
		   (DWORD )(pTm->nHour)*60*60 +
		   (DWORD )pTm->nMinute*60 + pTm->nSecond;
}

DWORD TimeToMinutes(const TTime* pTm)
{
	return DaysFrom2000(pTm)*60*24 +
		   (DWORD )(pTm->nHour)*60 +
		   pTm->nMinute;
}

DWORD TimeToHours(const TTime* pTm)
{
	return DaysFrom2000(pTm)*24 + pTm->nHour;
}

bool IsTimeEmpty(const TTime* pTm)
{
	if (pTm->nYear==0 && pTm->nMonth==0 && pTm->nDay==0 && 
		pTm->nHour==0 && pTm->nMinute==0 && pTm->nSecond==0)
		return true;
	else
		return false;
}

//描述:是否是不同的一天
bool IsDiffDay(const TTime* pTm1, const TTime* pTm2)
{
	if (pTm1->nDay!=pTm2->nDay || pTm1->nMonth!=pTm2->nMonth || pTm1->nYear!=pTm2->nYear)
		return true;
	else
		return false;
}

//描述:是否是同一天
bool IsSameDay(const TTime* pTm1, const TTime* pTm2)
{
	if (pTm1->nDay==pTm2->nDay && pTm1->nMonth==pTm2->nMonth && pTm1->nYear==pTm2->nYear)
		return true;
	else
		return false;
}

//DayOfWeek()的返回 1 = Sunday, 2 = Monday, ..., 7 = Saturday
BYTE DayOfWeek(const TTime* pTm)
{
	DWORD dwDays = DaysFrom2000(pTm);
	WORD nWeek = (WORD )(dwDays % 7);

	nWeek = (nWeek + BASEWEEK) % 7;	
	return nWeek + 1;
}

//描述:
//		@iIntervV 递增或递减的时间间隔,取决于正负
bool AddIntervs(TTime* pTm, BYTE bIntervU, int iIntervV)
{
	int nYear, nMonth;
	DWORD dwSec = TimeToSeconds(pTm);
	switch (bIntervU)
	{
		case TIME_UNIT_MINUTE:
			dwSec += (DWORD )60*iIntervV;
			SecondsToTime(dwSec, pTm);
			return true;
		
		case TIME_UNIT_HOUR:
			dwSec += (DWORD )60*60*iIntervV;
			SecondsToTime(dwSec, pTm);
			return true;
		
		case TIME_UNIT_DAY:
			dwSec += (DWORD )24*60*60*iIntervV;
			SecondsToTime(dwSec, pTm);
			return true;
		
		case TIME_UNIT_MONTH: 
			nYear = iIntervV / 12;
			pTm->nYear = pTm->nYear + nYear;
			
			iIntervV = iIntervV % 12;
			nMonth = iIntervV + pTm->nMonth;   //先把month当有符号数看待
			if (nMonth > 12)
			{
				pTm->nYear++;
				nMonth -= 12;
			}
			else if (nMonth < 1)
			{
				pTm->nYear--;
				nMonth += 12;
			}
			
			pTm->nMonth = nMonth;
			
			return true;
			
		default: return false;
	}
	
	//return false;
}


//描述:
//		@iIntervV 递增或递减的时间间隔,取决于正负
bool AddIntervsInTask(TTime* pTm, BYTE bIntervU, int iIntervV)
{
	int nYear, nMonth;
	DWORD dwSec = TimeToSeconds(pTm);
	switch (bIntervU)
	{
		case TIME_UNIT_MINUTE_TASK:
			dwSec += (DWORD )60*iIntervV;
			SecondsToTime(dwSec, pTm);
			return true;
		
		case TIME_UNIT_HOUR_TASK:
			dwSec += (DWORD )60*60*iIntervV;
			SecondsToTime(dwSec, pTm);
			return true;
		
		case TIME_UNIT_DAY_TASK:
			dwSec += (DWORD )24*60*60*iIntervV;
			SecondsToTime(dwSec, pTm);
			return true;
		
		case TIME_UNIT_MONTH_TASK: 
			nYear = iIntervV / 12;
			pTm->nYear = pTm->nYear + nYear;
			
			iIntervV = iIntervV % 12;
			nMonth = iIntervV + pTm->nMonth;   //先把month当有符号数看待
			if (nMonth > 12)
			{
				pTm->nYear++;
				nMonth -= 12;
			}
			else if (nMonth < 1)
			{
				pTm->nYear--;
				nMonth += 12;
			}
			
			pTm->nMonth = nMonth;
			
			return true;
			
		default: return false;
	}
	
	//return false;
}


//描述:求现在的时间pNow相对过去的时间pPast已经消逝的采样间隔个数,本函数会把间隔单位以下
//		的时间忽略掉,比如如果间隔时间是天,则把时分秒忽略掉
int IntervsPast(const TTime* pPast, const TTime* pNow, BYTE bIntervU, BYTE bIntervV)
{
	int iSign;
	DWORD dwPast, dwNow;
	TTime now, past;

	if (bIntervV == 0)
		return 0;
		
	dwPast = TimeToSeconds(pPast);
	dwNow = TimeToSeconds(pNow);
	iSign = 1;
	if (dwNow == dwPast)
	{
		return 0;
	}
	else if (dwNow > dwPast)
	{
		now = *pNow;
		past = *pPast;
	}
	else
	{
		iSign = -1;
		now = *pPast;
		past = *pNow;
	}
		
	switch (bIntervU)
	{
		case TIME_UNIT_MINUTE:
			return iSign * (int )(MinutesPast(&past, &now) / bIntervV);
		
		case TIME_UNIT_HOUR:
			return iSign * (int )(HoursPast(&past, &now) / bIntervV);
		
		case TIME_UNIT_DAY:
			return iSign * (int )(DaysPast(&past, &now) / bIntervV);
		
		case TIME_UNIT_MONTH:
			return iSign * (int )(MonthsPast(&past, &now) / bIntervV);
		default: return 0;
	}
	
	//return 0;
}

int TaskIntervsPast(const TTime* pPast, const TTime* pNow, BYTE bIntervU, BYTE bIntervV)
{
	int iSign;
	DWORD dwPast, dwNow;
	TTime now, past;

	if (bIntervV == 0)
		return 0;
		
	dwPast = TimeToSeconds(pPast);
	dwNow = TimeToSeconds(pNow);
	iSign = 1;
	if (dwNow == dwPast)
	{
		return 0;
	}
	else if (dwNow > dwPast)
	{
		now = *pNow;
		past = *pPast;
	}
	else
	{
		iSign = -1;
		now = *pPast;
		past = *pNow;
	}
		
	switch (bIntervU)
	{
		case TIME_UNIT_MINUTE_TASK:
			return iSign * (int )(SecondsPast(&past, &now) / (bIntervV*60));
		
		case TIME_UNIT_HOUR_TASK:
			return iSign * (int )(SecondsPast(&past, &now) / (bIntervV*60*60));
		
		case TIME_UNIT_DAY_TASK:
			return iSign * (int )(SecondsPast(&past, &now) / (bIntervV*60*60*24));
		
		case TIME_UNIT_MONTH_TASK:
			if ((now.nDay-1)*86400+now.nHour*3600+now.nMinute*60+now.nSecond >= (past.nDay-1)*86400+past.nHour*3600+past.nMinute*60+past.nSecond)
				return iSign * (int )(MonthsPast(&past, &now) / bIntervV);
			else
				return iSign * (int )(MonthsPast(&past, &now) / bIntervV)-1;
			
		default: return 0;
	}
	
	//return 0;
}

//描述:把时间转换成可以显示的字符串
//参数:@time 待转换的时间
//	   @psz 用来存放转换后的字符串
//返回:转换后得到的字符串
char* TimeToStr(TTime time, char* psz)
{
	sprintf(psz, "%04d-%02d-%02d %02d:%02d:%02d", 
			time.nYear, time.nMonth, time.nDay, 
			time.nHour, time.nMinute, time.nSecond);
			
	return psz;
}

//描述:把时间间隔单位转换成可以显示的字符串
//参数:@bIntervU	待转换的时间间隔单位
//	   @psz		用来存放转换后的字符串
//返回:转换后得到的字符串
char* IntervUToStr(BYTE bIntervU, char* psz)
{
	switch (bIntervU)
	{
		case TIME_UNIT_MINUTE:
			sprintf(psz, "Minute");
			break;

		case TIME_UNIT_HOUR:
			sprintf(psz, "Hour");
			break;

		case TIME_UNIT_DAY:
			sprintf(psz, "Day");
			break;

		case TIME_UNIT_MONTH:
			sprintf(psz, "Month");
			break;

		case TIME_UNIT_DAYFLG:
			sprintf(psz, "DayFlg");
			break;

		case TIME_UNIT_MTR:
			sprintf(psz, "Class1");
			break;

		case TIME_UNIT_STA:
			sprintf(psz, "Stat");
			break;

		default:
			sprintf(psz, "Unkown");
			break;
	}

	return psz;
}

//描述:把任务类型转换成可以显示的字符串
//参数:@bType	待转换的任务类型
//	   @psz		用来存放转换后的字符串
//返回:转换后得到的字符串
char* TaskTypeToStr(BYTE bType, char* psz)
{
	switch (bType)
	{
		case TYPE_FRZ_TASK:
			sprintf(psz, "DayMonTask");
			break;

		case TYPE_COMM_TASK:
			sprintf(psz, "ComTask");
			break;

		case TYPE_FWD_TASK:
			sprintf(psz, "FwdTask");
			break;

		case TYPE_ALR_EVENT:
			sprintf(psz, "AlrEvt");
			break;

		default:
			sprintf(psz, "Unkown");
			break;
	}

	return psz;
}

int64 Pow(int iBase, WORD wExp)
{
	int64 iVal=1;
	WORD i;
	for (i=0; i<wExp; i++)
	{
		iVal *= iBase;
	}
	
	return iVal;
}

//描述:计算一个缓冲区中的比特为1的位的个数
WORD CalcuBitNum(const BYTE* pbBuf, WORD wSize)
{
	const static WORD wBitNumArr[16] = {0, 1, 1, 2, 1, 2, 2, 3, //0~7
								  1, 2, 2, 3, 2, 3, 3, 4};//8~15
	
	WORD i;
	WORD wBitNum = 0;
	BYTE b;
	for (i=0; i<wSize; i++)
	{
		b = *pbBuf++;
		wBitNum += wBitNumArr[b&0x0f];
		wBitNum += wBitNumArr[(b>>4)&0x0f];
	}
	
	return wBitNum;
}

//485端口转发函数
int DoMtrFwdFunc(BYTE bPort, WORD wTestId, BYTE bMtrPro, BYTE* pbMtrAddr, BYTE* pbRxFrm, BYTE bRxBufSize)
{
	WORD wRxLen;
	int iPort;
	BYTE bFrmLen;
	TCommPara CommPara;
	BYTE bBuf[128];
	DWORD dwID, dwTmpClick, dwLen;

	iPort = MeterPortToPhy(bPort);
	if (iPort < 0)
		return -1;
	CommPara.wPort = (WORD)iPort;
	if (bMtrPro == CCT_MTRPRO_97)
		CommPara.dwBaudRate = CBR_1200;
	else
		CommPara.dwBaudRate = CBR_2400;	
	CommPara.bParity =  EVENPARITY;
	CommPara.bByteSize = 8;
	CommPara.bStopBits = ONESTOPBIT;

	if (bMtrPro == CCT_MTRPRO_07)
	{
		if (wTestId == 0x9010)
			dwID = 0x00010000;
		else 
			dwID = 0x0001ff00;
	}
	else	
		dwID = wTestId;

	//GetDirRdCtrl();	//取得直抄的控制权
	if ( !MtrProOpenComm(&CommPara) )
	{
		//ReleaseDirRdCtrl(); //释放直抄的控制权
		return -1;
	}

	CommRead(CommPara.wPort, NULL, 0, 300); //先清除一下串口

	bFrmLen = Make645AskItemFrm(bMtrPro, pbMtrAddr, dwID, bBuf);
	if (CommWrite(CommPara.wPort, bBuf, bFrmLen, 1000) != bFrmLen)
	{
		DTRACE(DB_FAPROTO, ("DoMtrFwdFunc: fail to write comm.\r\n")); 
		//ReleaseDirRdCtrl(); //释放直抄的控制权
		return -1;
	}

#ifndef SYS_WIN	
	if ( IsDebugOn(DB_FAPROTO) )
	{
		char szBuf[40];
		sprintf(szBuf, "DoMtrFwdFunc: Port%d Mtr Fwd Frm--> ", bPort);
		TraceBuf(DB_FAFRM, szBuf, bBuf, bFrmLen);
	}
#endif	

	wRxLen = 0;
	dwTmpClick = GetTick();	
	while (GetTick()-dwTmpClick < 2000)    //n次尝试读取数据
	{
		dwLen = CommRead(CommPara.wPort, bBuf, sizeof(bBuf), 200);

		if ((wRxLen+dwLen) > bRxBufSize)  //这里比较的应该是pbRxFrm的大小，而不是bBuf的
		{
			DTRACE(DB_FAPROTO, ("DoMtrFwdFunc: CommRead Buffer not enough!\r\n"));
			break;
		}
		else
			memcpy(pbRxFrm+wRxLen, bBuf, dwLen);

		if (dwLen > 0)
		{			
			wRxLen += (WORD)dwLen;
		}
	}
	//ReleaseDirRdCtrl(); //释放直抄的控制权

#ifndef SYS_WIN	
	if ( IsDebugOn(DB_FAPROTO) )
	{
		char szBuf[40];
		sprintf(szBuf, "DoMtrFwdFunc: Port%d Mtr Fwd Frm<--", bPort);
		TraceBuf(DB_FAFRM, szBuf, pbRxFrm, wRxLen);
	}
#endif	
	if (wRxLen == 0)
		return -2;
	return wRxLen;
}
WORD Make645Frm(BYTE* pbFrm, const BYTE* pbAddr, BYTE bCmd, BYTE *pbData,BYTE bDataLen)
{
	WORD i;
	pbFrm[0] = 0x68;
	memcpy(&pbFrm[1], pbAddr, 6);
	pbFrm[7] = 0x68;
	pbFrm[8] = bCmd;
	pbFrm[9] = bDataLen;

	memcpy(pbFrm+10,pbData,bDataLen);

	for (i=10; i<(WORD)bDataLen+10; i++)
	{
		pbFrm[i] += 0x33;
	}	 

	pbFrm[10+(WORD)bDataLen] = CheckSum(pbFrm, (WORD)bDataLen+10);
	pbFrm[11+(WORD)bDataLen] = 0x16;

	return bDataLen+12;
}  
//描述:组645抄读数据帧
BYTE Make645AskItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm)
{ 
	WORD i;
	//WORD wPn = PlcAddrToPn(pbAddr, NULL);
	BYTE bDataLen, bCS = 0;

    pbFrm[0] = 0x68;
	memcpy(pbFrm+1, pbAddr, 6);
    pbFrm[7] = 0x68;

	if (bMtrPro == CCT_MTRPRO_07)	
	{
		pbFrm[8] = 0x11;		//命令
		memcpy(pbFrm+10, &dwID, 4);
		bDataLen = 4;
	}
	else
    {
		pbFrm[8] = 0x01;		//命令
		memcpy(pbFrm+10, &dwID, 2);
		bDataLen = 2;
	}

	//bDataLen = MeterIdToBuf(wPn, dwID, pbFrm+10, m_pStdPara->RdrPara.bMtrPro);
	pbFrm[9] = bDataLen;

	for (i=0; i<bDataLen; i++)
		pbFrm[10+i] += 0x33;

	for (i=0; i<10+bDataLen; i++)
		bCS += pbFrm[i];

	pbFrm[10+bDataLen] = bCS;
	pbFrm[11+bDataLen] = 0x16;

	return 12+bDataLen;
}

const static unsigned short crc_16_table[16] = {
  0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
  0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};
  
WORD get_crc_16 (WORD start, BYTE *p, int n)
{ 
	WORD crc = start;
	register WORD r;
	/* while there is more data to process */
	while (n-- > 0) 
	{
		/* compute checksum of lower four bits of *p */
		r = crc_16_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r ^ crc_16_table[*p & 0xF];

		/* now compute checksum of upper four bits of *p */
		r = crc_16_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r ^ crc_16_table[(*p >> 4) & 0xF];

		/* next... */
		p++;
	}
	return(crc);
}

//将一维数组元素倒序
void Swap(BYTE *pbBuf, WORD wLen)
{
    BYTE bTemp;
    WORD wSwapTimes = wLen>>1;
	WORD i;
    for (i=0; i<wSwapTimes; i++)
    {
        bTemp = pbBuf[i];
        pbBuf[i] = pbBuf[wLen-i-1];
        pbBuf[wLen-i-1] = bTemp;
    }
}

//倒序拷贝
BYTE memrcpy(BYTE *pbDst,BYTE *pbSrc,WORD wLen)
{
	int i;
	for (i=0;i<wLen;i++) 
	{
		pbDst[i] = pbSrc[wLen-1-i];
	}
	return wLen;
}

int BufToAllTime(BYTE* pbBuf, TTime* time)
{
	time->nYear = BcdToByte(*pbBuf++)*100 + BcdToByte(*pbBuf++);
	time->nMonth = BcdToByte(*pbBuf++);
	time->nDay = BcdToByte(*pbBuf++);
	time->nHour = BcdToByte(*pbBuf++);
	time->nMinute = BcdToByte(*pbBuf++);
	time->nSecond = 0;

	return 6;
}

DWORD tmBufToSecond(BYTE* pbBuf)
{
	TTime tm;
	DWORD dwSec = 0;
	BufToAllTime(pbBuf, &tm);
	dwSec = TimeToSeconds(&tm);

	return dwSec;
}

char* MtrAddrToStr(const BYTE* pbAddr, char* psz)
{
	sprintf(psz, "%02x%02x%02x%02x%02x%02x", 
		pbAddr[5], pbAddr[4], pbAddr[3], pbAddr[2], pbAddr[1], pbAddr[0]);

	return psz;
}