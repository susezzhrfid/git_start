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
	g_diDB = GetItemEx(BN1, PN0, 0x1002);  //0x1002 16 ����ĵ����������

	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
	//3�ڲ����ڵ������
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

	if (!g_fTraceEnable) //���ڱ���ͨѶ�ں͵���������ص������,ʹ�ñ��ӿ�������
		return false;
		
	if (g_fDbInited == false)
		return true;

	pbDbItem = GetItemRdAddrDI(&g_diDB, bBuf);

	if (pbDbItem == NULL)//��Щ��g_pbDbItemû�г�ʼ����ʱ���ӡ������
		return false;

	if ((*pbDbItem & 0x01) == 0)     //�ܿ��عر�
		return false;

	flag = pbDbItem[bType>>3];
	if ((flag & (1<<(bType&7))) == 0)    //������������Ϣ
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


//����:���ڱ���ͨѶ�ں͵���������ص������,ʹ�ñ��ӿ�������
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
BYTE g_bDbBuf[DB_BUF_LEN];	//ʹ�ù���������������ÿ���̶߳�������ô��Ķ�ջ

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
		if (n==0 || wStrLen>1000)   //�Ѿ���bBuf[]��������,���Ȱѵ�ǰ���������
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
