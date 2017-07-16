/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Modem.cpp
 * 摘    要：本文件实现了通信MODEM的基类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：可作为GPRS,CDMA和电话MODEM的基类m
 *********************************************************************************************************/
#include "ProIfCfg.h"
#include "FaCfg.h"
#include "Modem.h"
#include "SysDebug.h"
#include "ComAPI.h"
#include "sysarch.h"

BYTE ASCII2BCD(BYTE bLow, BYTE bHigh)
{
	return (bLow-'0') + ((bHigh-'0')<<4);
}

void AsciiStr2BCD(BYTE* pAscii, BYTE* pBcd, BYTE bBcdLen)
{
	WORD i;
	for (i=0; i<bBcdLen; i++)
	{
		*pBcd++ = ASCII2BCD(*(pAscii+1), *pAscii);
		pAscii += 2;
	}
}

WORD CpyUntilCR(BYTE* pDst, BYTE* pSrc, WORD wLen)
{
	WORD i=0;
	for (i=0; i<wLen; i++)
	{
		if (*pSrc!='\r' && *pSrc!='\n')
			*pDst++ = *pSrc++;
		else
			return i;
	}
	
	return i;
}

void ModemInit(struct TModem* pModem)
{	
	pModem->bStep = MODEM_STEP_INIT;    
	CommOpen(pModem->bComm, CBR_115200, 8, ONESTOPBIT, NOPARITY);    
}

int ATCommand(struct TModem* pModem, char* pszCmd, char* pszAnsOK,
			  char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds)
{
	WORD wLen;
	DTRACE(DB_FAPROTO, ("ATCommand : tx %s\r\n", pszCmd));

	CommRead(pModem->bComm, NULL, 0, 10);    //发AT命令前，先清串口缓冲区
	wLen = strlen(pszCmd);
#if 0
    while(1) {
      CommWrite(pModem->bComm, (BYTE *)pszCmd, wLen, 1000);
      Sleep(1000);
    }
#endif
    if (CommWrite(pModem->bComm, (BYTE *)pszCmd, wLen, 1000) != wLen)
		return false;

	return WaitModemAnswer(pModem, pszAnsOK, pszAnsErr1, pszAnsErr2, nWaitSeconds);
}


//返回:接收到正确的回答返回1,接收到第一个错误的回答返回-1,接收到第二个错误的回答返回-2,超时返回0
int WaitModemAnswer(struct TModem* pModem, char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds)
{
	BYTE bBuf[128];
	WORD i = 0;
	WORD wLen;
    WORD wAllLen = 0;
	
	do
	{
		if (i!=0 && i!=1)   //对于nWaitSeconds==0的情况,Read()两回,不Sleep()
			Sleep(1000);
        
		wLen = CommRead(pModem->bComm, bBuf+wAllLen, sizeof(bBuf)-wAllLen, 1000);
        wAllLen += wLen;
        if (wAllLen >= sizeof(bBuf))
            wAllLen = sizeof(bBuf)-1;
		bBuf[wAllLen] = 0;

		//1.对模块主动发过来的的数据进行特殊处理,如TCP/UDP报文、收到来自监听端口的新连接
		pModem->pfnSpecHandle(pModem, (char* )bBuf, wAllLen, sizeof(bBuf)); //

		//2.正常的AT命令处理
		if (strstr((char* )bBuf, pszAnsOK) != NULL)   //接收到正确回答
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return 1;
		}

		if (pszAnsErr1!=NULL && strstr((char* )bBuf, pszAnsErr1)!=NULL)  //接收到错误回答1
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -1;
		}
		
		if (pszAnsErr2!=NULL && strstr((char* )bBuf, pszAnsErr2)!=NULL)  //接收到错误回答2
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -2;
		}
        
        if (wAllLen == sizeof(bBuf)-1) //Buf 满
            wAllLen = 0;  //将之前收到的扔掉

		i++;
		if (i == 1)    //对于nWaitSeconds==0的情况,至少还得再来一次
		{
			continue;
		}
		else if (i >= nWaitSeconds)
		{
			break;
		}
	} while(1);

	if (wAllLen != 0)
	{
		bBuf[wAllLen] = 0;
		DTRACE(DB_FAPROTO, ("CModem::WaitModemAnswer : rx %s\r\n", bBuf));
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CModem::WaitModemAnswer : rx no answer.\r\n", bBuf));
	}

	return 0;
}

bool ATCmdTest(struct TModem* pModem, WORD wTimes)
{
	WORD i;
	for (i=0; i<wTimes; i++)
	{
		if (ATCommand(pModem, "ATE0\r\n", "OK", NULL, NULL, 0)>0 && 
			ATCommand(pModem, "ATE0\r\n", "OK", NULL, NULL, 0)>0)
		{
			return true;
		}
	}
	
	return false;
}


//描述:更新场强到内部变量m_wSignStrength
//参数:用来存放并返回串口接收到的数据,避免在更新场强的时候丢失正常的通信帧
//返回:场强,如果失败则返回-1
//备注:在这个阶段不会收到TCP/UDP报文，不用处理
int UpdateSignStrength(struct TModem* pModem)
{
	char* p;
	int len;
	DWORD dwRead;
	WORD wSignStrength;
	BYTE bBuf[128];
    char szCmd[16] = "AT+CSQ\r\n";

	//发AT+CSQ
	DTRACE(DB_FAPROTO, ("UpdateSignalStrength : tx AT+CSQ.\r\n"));
	len = strlen(szCmd);
    if (CommWrite(pModem->bComm, (BYTE *)szCmd, len, 1000) != len) 
        return 0;
	
	Sleep(500);  //避免Read接收到以前的通信数据而立即返回
	
    memset(bBuf, 0, sizeof(bBuf));
	dwRead = CommRead(pModem->bComm, bBuf, 120, 2000);
	//bBuf[dwRead] = 0;
	DTRACE(DB_FAPROTO, ("UpdateSignStrength : rx %s.\r\n", bBuf));
	//PutToLoopBuf(bBuf, dwRead);

	//+CSQ:
	p = strstr((char* )bBuf, "+CSQ:");
	if (dwRead<9 || p==NULL)
	{
        DTRACE(DB_FAPROTO, ("UpdateSignStrength : sending AT+CSQ, without answer.\r\n"));
		return 0;
	}

	wSignStrength = SearchStrVal(p+5, (char* )&bBuf[dwRead]);
	/*
	if (m_wSignStrength >= 100)//TD的模块信号强度范围为100-199
	{
		m_wSignStrength -= 100;		
		if (m_wSignStrength != 99)
		{
			m_wSignStrength = (m_wSignStrength)*31/98 + 10;
			if (m_wSignStrength > 31)
				m_wSignStrength = 31;
		}
	}*/
	return (int)wSignStrength;
}

/*
bool IsSignValid(WORD wSignStrength)
{
	if (wSignStrength==0 || wSignStrength==99
		|| wSignStrength==100 || wSignStrength==199)
	{
		return false;
	}

	return true;
}


void SignLedCtrl(BYTE bSignStrength)
{
    //SetLed(true, LED_SIGR);     //开红灯
    //SetLed(false, LED_SIGG);    //关绿灯
    if (IsSignValid(bSignStrength))
    {
        if (bSignStrength >= 20)
        {
            //SetLed(true, LED_SIGG);
            //SetLed(false, LED_SIGR);
        }
        else  if (bSignStrength >= 10)
        {
            //SetLed(true, LED_SIGR);
            //SetLed(true, LED_SIGG);
        }
    }
}*/

int ATCmdGetInfo(struct TModem* pModem, char* pszCmd, char* pszAnsOK,
			  char* pszAnsErr, char* psRxHead, char *psBuf, WORD wBufSize,
              WORD nWaitSeconds)
{           
    BYTE bBuf[128];
	WORD i = 0;
	WORD wLen;
    WORD wAllLen = 0;
    char *p;
    
	DTRACE(DB_FAPROTO, ("ATCommand : tx %s\r\n", pszCmd));

	CommRead(pModem->bComm, NULL, 0, 10);    //发AT命令前，先清串口缓冲区
	wLen = strlen(pszCmd);
    if (CommWrite(pModem->bComm, (BYTE *)pszCmd, wLen, 1000) != wLen)
		return -2;
    
    memset(bBuf, 0, sizeof(bBuf));
	do
	{
		if (i!=0 && i!=1)   //对于nWaitSeconds==0的情况,Read()两回,不Sleep()
			Sleep(1000);
        
		wLen = CommRead(pModem->bComm, bBuf+wAllLen, sizeof(bBuf)-wAllLen-1, 1000);
        wAllLen += wLen;
        if (wAllLen >= sizeof(bBuf))
            wAllLen = sizeof(bBuf)-1;
		bBuf[wAllLen] = 0;
        
        //1.对模块主动发过来的的数据进行特殊处理,如TCP/UDP报文、收到来自监听端口的新连接
		pModem->pfnSpecHandle(pModem, (char* )bBuf, wAllLen, sizeof(bBuf)); //
        
		//2.正常的AT命令处理
		if (strstr((char* )bBuf, pszAnsOK) != NULL)   //接收到正确回答
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
            //取出需要的部份
            if (psBuf == NULL)
                return -3;
            
            if (psRxHead != NULL)
            {
                wLen = strlen(psRxHead);
                if (wLen >= sizeof(bBuf))//头比缓存还长
                    return -4;
                p = strstr((char* )bBuf, psRxHead);   //接收到正确回答
                if (p == NULL)//没有找到头
                    return -5;                
                p += wLen;
                wLen = strlen(p);
                if (wLen > wBufSize) //缓存区太小
                    return -6;
                strcpy(psBuf, p);
            }
            else
            {
                wLen = strlen((char *)bBuf);
                if (wLen > wBufSize) //缓存区太小
                    return -7;
                strcpy(psBuf, (char *)bBuf);
            }
                    
    		return wLen;  
		}

		if (pszAnsErr!=NULL && strstr((char* )bBuf, pszAnsErr)!=NULL)  //接收到错误回答1
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -1;
		}
		
        if (wAllLen == sizeof(bBuf)-1) //Buf 满
            wAllLen = 0;  //将之前收到的扔掉

		i++;
		if (i == 1)    //对于nWaitSeconds==0的情况,至少还得再来一次
		{
			continue;
		}
		else if (i >= nWaitSeconds)
		{
			break;
		}
	} while(1);

	if (wAllLen != 0)
	{
		bBuf[wAllLen] = 0;
		DTRACE(DB_FAPROTO, ("CModem::WaitModemAnswer : rx %s\r\n", bBuf));
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CModem::WaitModemAnswer : rx no answer.\r\n", bBuf));
	}

	return 0;
}

bool GetModemVer(struct TModem* pModem)
{           
    BYTE bBuf[64];
    char *p = NULL;
    int iRet;
    iRet = ATCmdGetInfo(pModem, "AT$MYGMR\r\n", "OK", "ERROR", NULL, (char *)bBuf, sizeof(bBuf), 3);//获取版本信息
	//iRet = ATCmdGetInfo(pModem, "ATI\r\n", "OK", "ERROR", NULL, (char *)bBuf, sizeof(bBuf), 3);//获取版本信息
  	if (iRet < 46)
	{
		DTRACE(DB_FAPROTO, ("GetGMR : CGMR is to short %d.\r\n", iRet-2));
		return false;
	}
    else if (iRet >= sizeof(bBuf))
        return false;
    
    memset(&pModem->tModemInfo, 0, sizeof(TModemInfo));
        
        //1.厂商代号	ASCII	4
	p = strstr((char* )bBuf, "\r\n");
	if (p==NULL || (BYTE* )p+44>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("GetGMR : fail to rd bManuftr.\r\n"));
		return false;
	}
	p += 2;
	memcpy(pModem->tModemInfo.bManuftr, p, 4); //厂商代号	ASCII	4
        
	//2.模块型号	ASCII	8
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+38>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bmodel.\r\n"));
		return false;
	}
	p += 2;
	CpyUntilCR(pModem->tModemInfo.bModel, (BYTE* )p, 8); 	//模块型号	ASCII	8
											//memcpy(m_ModemInfo.bmodel, p, 8);
	//3.软件版本号	ASCII	4
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+28>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bSoftVer.\r\n"));
		return false;
	}
	p += 2;
	memcpy(pModem->tModemInfo.bSoftVer, p, 4); //软件版本号	ASCII	4
	
	//4.软件发布日期：日月年	见附录A.20	3
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+22>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bSoftDate.\r\n"));
		return false;
	}
	p += 2;
	AsciiStr2BCD((BYTE* )p, pModem->tModemInfo.bSoftDate, 3); //软件发布日期：日月年	见附录A.20	3

	//5.硬件版本号	ASCII	4
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+14>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bHardVer.\r\n"));
		return false;
	}
	p += 2;
	memcpy(pModem->tModemInfo.bHardVer, p, 4); //硬件版本号	ASCII	4

	//6.硬件发布日期：日月年	见附录A.20	3
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+8>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bHardDate.\r\n"));
		return false;
	}
	p += 2;
	AsciiStr2BCD((BYTE* )p, pModem->tModemInfo.bHardDate, 3);  //硬件发布日期：日月年	见附录A.20	3
    
    return true;
}

bool GetModemCCID(struct TModem* pModem)
{
    BYTE bBuf[32];
    BYTE *p = bBuf;
    int iRet;
    bool fStart = false;
    WORD i = 0;
    iRet = ATCmdGetInfo(pModem, "AT$MYCCID\r\n", "OK", "ERROR", "CCID:", (char *)bBuf, sizeof(bBuf), 2);//ICCID
    if (iRet >= 20)
    {        
		while (p<&bBuf[iRet] && i<20)	
		{
			if (fStart == false)
			{
				if (*p == '"')
					fStart = true;
			}
			else	//第一个'"'就开始拷贝，直到出现第二个'"'
			{
				if (*p == '"')
				{
					return true;	//又出现'"'，即使里面为空，当成模块返回的就是空
				}
				else
				{
					pModem->tModemInfo.bCCID[i++] = *p;
				}
			}
						
			p++;
		}
		
		if (i == 20)
			return true;        
    }
    memset(&pModem->tModemInfo.bCCID, 0, sizeof(pModem->tModemInfo.bCCID));
    return false;
}
