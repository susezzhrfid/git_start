#include <stdio.h>
#include "Trace.h"
#include "Comm.h"
#include "DbAPI.h"
#include <string.h>
#include "SysDebug.h"
#include "DrvCfg.h"

bool g_fDbInited = false;
bool g_fTraceEnable = true; 
TDataItem g_diDB;
extern TSem   g_semDebug;


bool InitDebug()
{
	BYTE bPortFun = PORT_FUN_DEBUG;
	g_diDB = GetItemEx(BN1, PN0, 0x1002);  //0x1002 16 各项的调试输出开关

	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
	//3口不用于调试输出
	if (bPortFun != PORT_FUN_DEBUG)
	{
		g_fTraceEnable = false;
		//g_fDbInited = false;
		//CommClose(COMM_DEBUG);
		return false;
	}
    
	g_fTraceEnable = true;
	g_fDbInited = true;
	return true;
}

bool IsDebugOn(BYTE bType)
{    
	BYTE flag;
	BYTE bBuf[16];
	BYTE* pbDbItem = NULL;
    
    //if ((bType >= MAXDBMASK) || (sizeof(bBuf)*8 < MAXDBMASK))
    //    return false;

	if (!g_fTraceEnable) //对于本地通讯口和调试输出口重叠的情况,使用本接口来开关
		return false;
		
	if (g_fDbInited == false)
		return true;

	pbDbItem = GetItemRdAddrDI(&g_diDB, bBuf);

	if (pbDbItem == NULL)//有些在g_pbDbItem没有初始化的时候打印会死机
		return false;

	if ((*pbDbItem & 0x01) == 0)     //总开关关闭
		return false;

	flag = pbDbItem[bType>>3];
	if ((flag & (1<<(bType&7))) == 0)    //该项不输出调试信息
		return false;

	/*ReadItemEx(BN2, PN0, 0x2110, &flag);
	if (flag == 1)
	{
		TTime now, startnow;
		GetCurTime(&now);
		ReadItemEx(BN0, PN0, 0x8910, bBuf);
		Fmt15ToTime(bBuf, &startnow);	
		if (MinutesPast(&startnow, &now) >= 1440)
		{
			flag = 0;
			WriteItemEx(BN2, PN0, 0x2110, &flag);
		}
		return false;
	}*/

	return true;
}


//描述:对于本地通讯口和调试输出口重叠的情况,使用本接口来开关
void EnableTrace(bool fEnable)
{
	g_fTraceEnable = fEnable;
}

bool IsTraceEnable() 
{ 
    return g_fTraceEnable;
}


#ifdef SYS_WIN
#define DB_BUF_LEN	(1024*16)
BYTE g_bDbBuf[DB_BUF_LEN];	//使用公共缓冲区，不用每个线程都消耗那么大的堆栈

/*WORD PrintBuf(BYTE* out, BYTE* in, WORD wInLen)
{
	int i;
    for (i=0; i<wInLen; i++)
    {
		BYTE b = *in++;
		BYTE hi = b >> 4;
		BYTE lo = b & 0x0f;
		*out++ = ' ';
		if (hi >= 0x0a)
			*out++ = hi - 0x0a + 'A';
		else
			*out++ = hi + '0';

		if (lo >= 0x0a)
			*out++ = lo - 0x0a + 'A';
		else
			*out++ = lo + '0';
	}

	*out++ = 0;
	return wInLen*3;
}*/


WORD PrintBuf(BYTE* out, WORD wOutLen, BYTE* in, WORD wInLen)
{
	WORD i;
	BYTE b, hi, lo;
    for (i=0; i<wInLen; i++)
    {
		if ((i+1)*3 > wOutLen)
			return i;

		b = *in++;
		hi = b >> 4;
		lo = b & 0x0f;
		*out++ = ' ';
		if (hi >= 0x0a)
			*out++ = hi - 0x0a + 'A';
		else
			*out++ = hi + '0';

		if (lo >= 0x0a)
			*out++ = lo - 0x0a + 'A';
		else
			*out++ = lo + '0';
	}

	return i;
}


void TraceBuf(WORD wSwitch, char* szHeadStr, BYTE* p, WORD wLen)
{
	WORD wStrLen, wPrinted, n;
	if (!IsDebugOn(wSwitch))
		return ;

	WaitSemaphore(g_semDebug, SYS_TO_INFINITE);
	
	wStrLen = strlen(szHeadStr);
	memcpy(g_bDbBuf, szHeadStr, wStrLen);

	for (wPrinted=0; wPrinted<wLen; )
	{
		n = PrintBuf(&g_bDbBuf[wStrLen], 1000-wStrLen, p, wLen-wPrinted);
		p += n;
		wPrinted += n;
		wStrLen += n*3;
		if (n==0 || wStrLen>1000)   //已经往bBuf[]里塞满了,则先把当前的数据输出
		{
			g_bDbBuf[wStrLen++] = '\r';
			g_bDbBuf[wStrLen++] = '\n';
			g_bDbBuf[wStrLen++] = 0;
			STRACE(wSwitch, (char*)g_bDbBuf, wStrLen);
			wStrLen = 0;
		}
	}

	if (wStrLen > 0)
	{
		g_bDbBuf[wStrLen++] = '\r';
		g_bDbBuf[wStrLen++] = '\n';
		g_bDbBuf[wStrLen++] = 0;
		STRACE(wSwitch, (char*)g_bDbBuf, wStrLen);
	}
	
	SignalSemaphore(g_semDebug);
}

void TraceFrm(char* pszHeader, BYTE* pbBuf, WORD wLen)
{
	if (!IsDebugOn(DB_FAFRM))
		return ;

	if (wLen > 0)
	{
		char szBuf[64];
		sprintf(szBuf, "%s %d\n", pszHeader, wLen);
		TraceBuf(DB_FAFRM, szBuf, pbBuf, wLen);
	}
}
#endif
