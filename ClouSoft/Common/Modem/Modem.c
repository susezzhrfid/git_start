/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Modem.cpp
 * ժ    Ҫ�����ļ�ʵ����ͨ��MODEM�Ļ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע������ΪGPRS,CDMA�͵绰MODEM�Ļ���m
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

	CommRead(pModem->bComm, NULL, 0, 10);    //��AT����ǰ�����崮�ڻ�����
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


//����:���յ���ȷ�Ļش𷵻�1,���յ���һ������Ļش𷵻�-1,���յ��ڶ�������Ļش𷵻�-2,��ʱ����0
int WaitModemAnswer(struct TModem* pModem, char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds)
{
	BYTE bBuf[128];
	WORD i = 0;
	WORD wLen;
    WORD wAllLen = 0;
	
	do
	{
		if (i!=0 && i!=1)   //����nWaitSeconds==0�����,Read()����,��Sleep()
			Sleep(1000);
        
		wLen = CommRead(pModem->bComm, bBuf+wAllLen, sizeof(bBuf)-wAllLen, 1000);
        wAllLen += wLen;
        if (wAllLen >= sizeof(bBuf))
            wAllLen = sizeof(bBuf)-1;
		bBuf[wAllLen] = 0;

		//1.��ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������
		pModem->pfnSpecHandle(pModem, (char* )bBuf, wAllLen, sizeof(bBuf)); //

		//2.������AT�����
		if (strstr((char* )bBuf, pszAnsOK) != NULL)   //���յ���ȷ�ش�
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return 1;
		}

		if (pszAnsErr1!=NULL && strstr((char* )bBuf, pszAnsErr1)!=NULL)  //���յ�����ش�1
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -1;
		}
		
		if (pszAnsErr2!=NULL && strstr((char* )bBuf, pszAnsErr2)!=NULL)  //���յ�����ش�2
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -2;
		}
        
        if (wAllLen == sizeof(bBuf)-1) //Buf ��
            wAllLen = 0;  //��֮ǰ�յ����ӵ�

		i++;
		if (i == 1)    //����nWaitSeconds==0�����,���ٻ�������һ��
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


//����:���³�ǿ���ڲ�����m_wSignStrength
//����:������Ų����ش��ڽ��յ�������,�����ڸ��³�ǿ��ʱ��ʧ������ͨ��֡
//����:��ǿ,���ʧ���򷵻�-1
//��ע:������׶β����յ�TCP/UDP���ģ����ô���
int UpdateSignStrength(struct TModem* pModem)
{
	char* p;
	int len;
	DWORD dwRead;
	WORD wSignStrength;
	BYTE bBuf[128];
    char szCmd[16] = "AT+CSQ\r\n";

	//��AT+CSQ
	DTRACE(DB_FAPROTO, ("UpdateSignalStrength : tx AT+CSQ.\r\n"));
	len = strlen(szCmd);
    if (CommWrite(pModem->bComm, (BYTE *)szCmd, len, 1000) != len) 
        return 0;
	
	Sleep(500);  //����Read���յ���ǰ��ͨ�����ݶ���������
	
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
	if (m_wSignStrength >= 100)//TD��ģ���ź�ǿ�ȷ�ΧΪ100-199
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
    //SetLed(true, LED_SIGR);     //�����
    //SetLed(false, LED_SIGG);    //���̵�
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

	CommRead(pModem->bComm, NULL, 0, 10);    //��AT����ǰ�����崮�ڻ�����
	wLen = strlen(pszCmd);
    if (CommWrite(pModem->bComm, (BYTE *)pszCmd, wLen, 1000) != wLen)
		return -2;
    
    memset(bBuf, 0, sizeof(bBuf));
	do
	{
		if (i!=0 && i!=1)   //����nWaitSeconds==0�����,Read()����,��Sleep()
			Sleep(1000);
        
		wLen = CommRead(pModem->bComm, bBuf+wAllLen, sizeof(bBuf)-wAllLen-1, 1000);
        wAllLen += wLen;
        if (wAllLen >= sizeof(bBuf))
            wAllLen = sizeof(bBuf)-1;
		bBuf[wAllLen] = 0;
        
        //1.��ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������
		pModem->pfnSpecHandle(pModem, (char* )bBuf, wAllLen, sizeof(bBuf)); //
        
		//2.������AT�����
		if (strstr((char* )bBuf, pszAnsOK) != NULL)   //���յ���ȷ�ش�
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
            //ȡ����Ҫ�Ĳ���
            if (psBuf == NULL)
                return -3;
            
            if (psRxHead != NULL)
            {
                wLen = strlen(psRxHead);
                if (wLen >= sizeof(bBuf))//ͷ�Ȼ��滹��
                    return -4;
                p = strstr((char* )bBuf, psRxHead);   //���յ���ȷ�ش�
                if (p == NULL)//û���ҵ�ͷ
                    return -5;                
                p += wLen;
                wLen = strlen(p);
                if (wLen > wBufSize) //������̫С
                    return -6;
                strcpy(psBuf, p);
            }
            else
            {
                wLen = strlen((char *)bBuf);
                if (wLen > wBufSize) //������̫С
                    return -7;
                strcpy(psBuf, (char *)bBuf);
            }
                    
    		return wLen;  
		}

		if (pszAnsErr!=NULL && strstr((char* )bBuf, pszAnsErr)!=NULL)  //���յ�����ش�1
		{
			DTRACE(DB_FAPROTO, ("WaitModemAnswer : rx %s\r\n", bBuf));
			return -1;
		}
		
        if (wAllLen == sizeof(bBuf)-1) //Buf ��
            wAllLen = 0;  //��֮ǰ�յ����ӵ�

		i++;
		if (i == 1)    //����nWaitSeconds==0�����,���ٻ�������һ��
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
    iRet = ATCmdGetInfo(pModem, "AT$MYGMR\r\n", "OK", "ERROR", NULL, (char *)bBuf, sizeof(bBuf), 3);//��ȡ�汾��Ϣ
	//iRet = ATCmdGetInfo(pModem, "ATI\r\n", "OK", "ERROR", NULL, (char *)bBuf, sizeof(bBuf), 3);//��ȡ�汾��Ϣ
  	if (iRet < 46)
	{
		DTRACE(DB_FAPROTO, ("GetGMR : CGMR is to short %d.\r\n", iRet-2));
		return false;
	}
    else if (iRet >= sizeof(bBuf))
        return false;
    
    memset(&pModem->tModemInfo, 0, sizeof(TModemInfo));
        
        //1.���̴���	ASCII	4
	p = strstr((char* )bBuf, "\r\n");
	if (p==NULL || (BYTE* )p+44>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("GetGMR : fail to rd bManuftr.\r\n"));
		return false;
	}
	p += 2;
	memcpy(pModem->tModemInfo.bManuftr, p, 4); //���̴���	ASCII	4
        
	//2.ģ���ͺ�	ASCII	8
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+38>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bmodel.\r\n"));
		return false;
	}
	p += 2;
	CpyUntilCR(pModem->tModemInfo.bModel, (BYTE* )p, 8); 	//ģ���ͺ�	ASCII	8
											//memcpy(m_ModemInfo.bmodel, p, 8);
	//3.����汾��	ASCII	4
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+28>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bSoftVer.\r\n"));
		return false;
	}
	p += 2;
	memcpy(pModem->tModemInfo.bSoftVer, p, 4); //����汾��	ASCII	4
	
	//4.����������ڣ�������	����¼A.20	3
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+22>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bSoftDate.\r\n"));
		return false;
	}
	p += 2;
	AsciiStr2BCD((BYTE* )p, pModem->tModemInfo.bSoftDate, 3); //����������ڣ�������	����¼A.20	3

	//5.Ӳ���汾��	ASCII	4
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+14>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bHardVer.\r\n"));
		return false;
	}
	p += 2;
	memcpy(pModem->tModemInfo.bHardVer, p, 4); //Ӳ���汾��	ASCII	4

	//6.Ӳ���������ڣ�������	����¼A.20	3
	p = strstr(p, "\r\n");
	if (p==NULL || (BYTE* )p+8>&bBuf[iRet])
	{
		DTRACE(DB_FAPROTO, ("CGC864::GetGMR : fail to rd bHardDate.\r\n"));
		return false;
	}
	p += 2;
	AsciiStr2BCD((BYTE* )p, pModem->tModemInfo.bHardDate, 3);  //Ӳ���������ڣ�������	����¼A.20	3
    
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
			else	//��һ��'"'�Ϳ�ʼ������ֱ�����ֵڶ���'"'
			{
				if (*p == '"')
				{
					return true;	//�ֳ���'"'����ʹ����Ϊ�գ�����ģ�鷵�صľ��ǿ�
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
