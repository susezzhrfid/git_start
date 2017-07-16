#include <stdio.h>
#include <ctype.h> 
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "Sysarch.h"
#include "DrvCfg.h"
//#include "SysApi.h"
#include "Comm.h"
#include "SysDebug.h"
#include "FaCfg.h"

#define DBG_BUF_LEN 256
//#define USER_DEF_PRINT
static TSem   g_semDebug;
static BYTE   g_bDebugBuf[DBG_BUF_LEN];//g_bDBBuf

//描述:初始化不同系统下调试最低层的部分
bool SysDebugInit()
{
	g_semDebug = NewSemaphore(1, 1);	
	CommOpen(COMM_DEBUG, 9600, 8, ONESTOPBIT, EVENPARITY);
	return true; 
}

int UsrSprintf(BYTE* pbBuf, const char *fmt, ...)
{
    va_list varg;
    char *p, *sval;
    int iVal,j;
    WORD i=0;
    DWORD dwVal;
    char bBuf[12] = {0};
    
    va_start(varg, fmt );
    for (p=(char *)fmt; *p; p++)
    {
        if (*p != '%')
        {
            pbBuf[i++] = *p;
            continue;
        }
        switch(*++p)
        {
        case 'd':
            iVal = va_arg(varg, int);
            if (iVal == 0)
                pbBuf[i++] = '0';
            else
            {
                if (iVal<0)
                {
                    iVal = -iVal;
                    pbBuf[i++] = '-';
                }
                j=0;
                memset(bBuf, 0, sizeof(bBuf));
                while(iVal>0)
                {
                    bBuf[j++] = iVal%10 + '0';
                    iVal /= 10;
                }
                
                for (j=11; j>=0; j--)
                {
                    if (bBuf[j] != 0)
                        pbBuf[i++] = bBuf[j];
                }
            }
            break;
            
        case 'l':
            dwVal = va_arg(varg, long);
            if (dwVal == 0)
                pbBuf[i++] = '0';
            else
            {
                j=0;
                memset(bBuf, 0, sizeof(bBuf));
                while(dwVal>0)
                {
                    bBuf[j++] = dwVal%10 + '0';
                    dwVal /= 10;
                }
                
                for (j=11;j>=0;j--)
                {
                    if (bBuf[j] != 0)
                        pbBuf[i++] = bBuf[j];
                }
            }
            p++;//去掉之后的d
            break;
            
        case 's':
            for (sval = va_arg(varg, char *); *sval; sval++)
                pbBuf[i++] = *sval;
            break;
            
        default:
            pbBuf[i++] = *p;
            break;
        }
    }
    va_end(varg);
    return 0;
}

//TODO:缓冲区是不是存在溢出的问题
void DebugPrintf(const char *fmt, ...)
{
    va_list varg;
#ifdef USER_DEF_PRINT
    char *p, *sval;
    int iVal,j;
    WORD i=0, wVal;
    DWORD dwVal;
    char bBuf[12] = {0};
#endif
    
    //TODO:没有信号量立刻返回是不是可能造成当前待输出的调试信息
    //没有机会输出  xiao
    WaitSemaphore(g_semDebug, 0);   
    
    memset(g_bDebugBuf, 0, sizeof(g_bDebugBuf));    
#ifdef USER_DEF_PRINT
    //sprintf((char*)g_bDebugBuf, fmt);
    //UsrSprintf((char* )g_bDebugBuf, fmt);
    va_start(varg, fmt );
    for (p=(char *)fmt; *p; p++)
    {
        if (i >= sizeof(g_bDebugBuf))
                break;
        if (*p != '%')
        {            
            g_bDebugBuf[i++] = *p;
            continue;
        }
        switch(*++p)
        {
        case 'd':
            iVal = va_arg(varg, int);
            if (iVal == 0)
                g_bDebugBuf[i++] = '0';
            else
            {
                if (iVal<0)
                {
                    iVal = -iVal;
                    g_bDebugBuf[i++] = '-';
                }
                j=0;
                memset(bBuf, 0, sizeof(bBuf));
                while(iVal>0)
                {
                    bBuf[j++] = iVal%10 + '0';
                    iVal /= 10;
                }
                
                for (j=11; j>=0; j--)
                {
                    if (bBuf[j] != 0)
                        g_bDebugBuf[i++] = bBuf[j];
                }
            }
            break;
            
        case 'x':
            wVal = va_arg(varg, unsigned int);
            if (wVal == 0)
                g_bDebugBuf[i++] = '0';
            else
            {
                g_bDebugBuf[i++] = '0';
                g_bDebugBuf[i++] = 'x';
                for (j=3; j>=0; j--)
                {
                    g_bDebugBuf[i] = (wVal>>(j*4))&0x0f;
                    if (g_bDebugBuf[i] > 9)
                        g_bDebugBuf[i] += 55;
                    else
                        g_bDebugBuf[i] += 48;
                    i++;
                }
            }
            break;
            
        case 'l':
            dwVal = va_arg(varg, unsigned long);
            if (dwVal == 0)
                g_bDebugBuf[i++] = '0';
            else
            {
                j=0;
                memset(bBuf, 0, sizeof(bBuf));
                while(dwVal>0)
                {
                    bBuf[j++] = dwVal%10 + '0';
                    dwVal /= 10;
                }
                for (j=11;j>=0;j--)
                {
                    if (bBuf[j] != 0)
                        g_bDebugBuf[i++] = bBuf[j];
                }
            }
            p++;
            break;
            
        case 's':
            for (sval = va_arg(varg, char *); *sval; sval++)
                g_bDebugBuf[i++] = *sval;
            break;
            
        default:
            g_bDebugBuf[i++] = *p;
            break;
        }
    }
    va_end(varg);
    
    g_bDebugBuf[DBG_BUF_LEN-1] = 0x00;
#else
    va_start(varg, fmt);
    vsnprintf((char*)g_bDebugBuf, sizeof(g_bDebugBuf), fmt, varg);
    va_end(varg);
#endif       
        
    CommWrite(COMM_DEBUG, g_bDebugBuf, strlen((const char *)g_bDebugBuf), 500);//9600波特率每秒可以打印800字节
    SignalSemaphore(g_semDebug); 
}

//#define DB_BUF_LEN	(1024)
//BYTE g_bTraceBuf[DB_BUF_LEN];	//使用公共缓冲区，不用每个线程都消耗那么大的堆栈

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
    
    if (!IsDebugOn(wSwitch)) //效率更高
        return;
    
    wStrLen = strlen(szHeadStr);
    if (wStrLen > DBG_BUF_LEN)
        return;
    
	WaitSemaphore(g_semDebug, SYS_TO_INFINITE);	
	
	memcpy(g_bDebugBuf, szHeadStr, wStrLen);

	for (wPrinted=0; wPrinted<wLen; )
	{
		n = PrintBuf(&g_bDebugBuf[wStrLen], DBG_BUF_LEN-wStrLen, p, wLen-wPrinted);
		p += n;
		wPrinted += n;
		wStrLen += n*3;
		if (n==0 || ((wStrLen+3)>=DBG_BUF_LEN))   //已经往bBuf[]里塞满了,则先把当前的数据输出
		{
			g_bDebugBuf[wStrLen++] = '\r';
			g_bDebugBuf[wStrLen++] = '\n';
			g_bDebugBuf[wStrLen++] = 0;
			//STRACE(wSwitch, (char*)g_bTraceBuf, wStrLen);
            
            //if (IsDebugOn(wSwitch))
            CommWrite(COMM_DEBUG, g_bDebugBuf, wStrLen, 500);
            
			wStrLen = 0;
		}
	}

	if (wStrLen > 0)
	{
		g_bDebugBuf[wStrLen++] = '\r';
		g_bDebugBuf[wStrLen++] = '\n';
		g_bDebugBuf[wStrLen++] = 0;
		//STRACE(wSwitch, (char*)g_bTraceBuf, wStrLen);
        
        //if (IsDebugOn(wSwitch))
        CommWrite(COMM_DEBUG, g_bDebugBuf, wStrLen, 500);
	}
	
	SignalSemaphore(g_semDebug);
}

void TraceFrm(char* pszHeader, BYTE* pbBuf, WORD wLen)
{
    if (!IsDebugOn(DB_FAFRM)) //效率更高
        return;
	if (wLen > 0)
	{
		char szBuf[64];
		sprintf(szBuf, "%s %d\n", pszHeader, wLen);
		TraceBuf(DB_FAFRM, szBuf, pbBuf, wLen);
	}
}
