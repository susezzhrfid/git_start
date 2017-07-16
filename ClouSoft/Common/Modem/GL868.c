/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�GL868.h
 * ժ    Ҫ�����ļ�ʵ����GL868MODEM����
 * ��ǰ�汾��1.0
 * ��    �ߣ�����
 * ������ڣ�2012��5��
 *********************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ProIfCfg.h"
#include "FaCfg.h"
//#include "FaConst.h"
#include "SysArch.h"
#include "drivers.h"
#include "GL868.h"
#include "ComAPI.h"
//#include "LibDbAPI.h"
#include "DrvConst.h"
//#include "SysAPI.h"
#include "SysDebug.h"
#include "GprsIf.h"
#include "ProPara.h"
#ifndef SYS_WIN
#include "ppp.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//GL868˽�к궨��
#define GL868_INVALID_SOCK	0xff	//��Ч��socketͨ��    
#define BACKLOG     10              //���ͬʱ���ӵ�������
////////////////////////////////////////////////////////////////////////////////////////////
//GL868˽�г�Ա����


/////////////////////////////////////////////////////////////////////////////////////////////////////
//GL868ʵ��

WORD str2num(char** pp)
{
	char* p = *pp;
	WORD wNum = 0;
	while (1)
	{
		if (*p>='0' && *p<='9')
		{
			wNum *= 10;
			wNum += *p - '0';
		}
		else
		{
			break;
		}

		p++;
	}

	*pp = p;
	return wNum;
}



void GL868Init(struct TModem* pModem)
{
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��

	//����ṹ��Ա��ʼ��
	ModemInit(pModem);
    

	//����

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pModem->pfnResetModem = GL868ResetModem;		//��λģ��
	pModem->pfnUpdateSignStrength = UpdateSignStrength;	//���³�ǿ
	pModem->pfnInitAPN = GL868InitAPN;			//��ʼ��APN

	pModem->pfnOpenPpp = GL868OpenPpp;			//PPP��������
	pModem->pfnClosePpp = GL868ClosePpp;		//�Ͽ�PPP����

	pModem->pfnConnect = GL868Connect; 		//��Ϊ�ͻ������ӷ�����
	pModem->pfnCloseCliSock = GL868CloseCliSock;	//�رտͻ���socket

	pModem->pfnCliSend = GL868CliSend; 	//��Ϊ�ͻ��˷�������
	pModem->pfnCliReceive = GL868CliReceive; 	//��Ϊ�ͻ��˽�������
	pModem->pfnSpecHandle = GL868SpecHandle; //��ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������

	pModem->pfnChkCliStatus = GL868ChkCliStatus;	//���ͻ��˵�socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ

#ifdef EN_PROIF_SVR		//֧�ַ�����
	pModem->pfnListen = GL868Listen;  		//��Ϊ�����������˿�
	pModem->pfnCloseListen = GL868CloseListen;		//�رռ���
	pModem->pfnCloseSvrSock = GL868CloseSvrSock;	//�رշ������Ѿ����ӵ�socket
	pModem->pfnSvrSend = GL868SvrSend; 	//��Ϊ��������������
	pModem->pfnSvrReceive = GL868SvrReceive; 	//��Ϊ��������������
	pModem->pfnIsSvrAcceptOne = GL868IsSvrAcceptOne;	//��Ϊ�������Ƿ���յ�һ���ͻ��˵�����
	pModem->pfnChkSvrStatus = GL868ChkSvrStatus;	//����������socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
#endif //EN_PROIF_SVR		//֧�ַ�����

	//����������ʼ��
	//pModem->pszCSQ = "AT+CSQ\r\n";
	pGL868->bCliSock = 0;	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
	pGL868->fPowOn = true;	//�Ƿ��Ǹ��ϵ�
	pGL868->fCliUdp = false;	//�ͻ����Ƿ���UDP����
    
    pGL868->pSocket = &g_tSock;   
    SocketInit(pGL868->pSocket);

#ifdef EN_PROIF_SVR		//֧�ַ�����
	pGL868->bSvrSock = GL868_INVALID_SOCK;	//�������˵�socket�ţ�ֻ��Ϊ0~9����ֵ0xff��ʾ��Ч��
#endif //EN_PROIF_SVR		//֧�ַ�����
}

int GL868InitAPN(struct TModem* pModem)
{
	char szApnCmd[64];
	char szApn[32];
	int  iRet;		
	WORD i, m;
	
	pModem->bStep = MODEM_STEP_SIM;
	//�������Ƿ�ע�����,ֻ��Ϊ�˷������
	for (i=0; i<30; i++)
	{
		if (ATCommand(pModem, "AT+CPIN?\r\n", "OK", "ERROR", NULL, 3) == 1)  //�����Ƿ���
		{
			pModem->bStep = MODEM_STEP_REG;
			iRet = ATCommand(pModem, "AT+CREG?\r\n", "+CREG: 0,1", "+CREG: 0,5", "+CREG: 0,0", 3);
			if (iRet==1 || iRet==-1)
				break;
			if ((pModem->bModuleVer == MODULE_GC864) || (pModem->bModuleVer == MODULE_GL868))
			{
				iRet = ATCommand(pModem, "AT#STIA?\r\n", "#STIA: 0,2", "ERROR", NULL, 3);
				if (iRet <= 0)
				{
					for (m=0; m<5; m++)
					{
						iRet = ATCommand(pModem, "AT#STIA=2\r\n", "OK", "ERROR", NULL, 3);
						if (iRet > 0)
						{
							ATCommand(pModem, "AT&P\r\n", "OK", "ERROR", NULL, 0);
							ATCommand(pModem, "AT&W\r\n", "OK", "ERROR", NULL, 0);
							break;
						}
					}
				}
			}
		}
		else
		{//���ǲ����Ȳ�εģ��������������ⲻ�����Ļ���û�б�Ҫ���������
			return MODEM_SIM_FAIL;
		}
		Sleep(1000);
	}
	
	//����ûע���,��ȥ����
	if (i >= 30)
	{
		DTRACE(DB_FAPROTO, ("CGL868::InitAPN : Network Register Fail!\r\n"));
		return MODEM_REG_FAIL;
	}	

	//Sleep(300);

    memset(szApn, 0, sizeof(szApn));
    GetApn(szApn);
	sprintf(szApnCmd, "+CGDCONT: 1,\"IP\",\"%s\"", szApn);
	if (ATCommand(pModem, "AT+CGDCONT?\r\n", szApnCmd, NULL, NULL, 5) <= 0)   
	{
		sprintf(szApnCmd, "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", szApn);
		Sleep(500);
		if (ATCommand(pModem, szApnCmd, "OK", NULL, NULL, 3) <= 0)   //ʧ���˶���һ��
		{					//APNĿǰֻ�ڵ�¼GPRSʱ���ã�SMSû������
			ATCommand(pModem, szApnCmd, "OK", NULL, NULL, 3);
		}
	}
    
    //ATCommand(pModem, "AT#PSNT?\r\n", "OK", NULL, NULL, 3);
    
    //ATCommand(pModem, "AT+CFUN?\r\n", "OK", NULL, NULL, 3);
    
    if (ATCommand(pModem, "ATDT*99***1#\r\n", "CONNECT", NULL, NULL, 3) <= 0)
    {
        ATCommand(pModem, "ATDT*99***1#\r\n", "CONNECT", NULL, NULL, 3);
    }

	return MODEM_NO_ERROR;
}

int GL868ResetModem(struct TModem* pModem)
{
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	WORD i, j;
	WORD wErrCnt = 0;
	WORD wRead = 0;
	BYTE bBuf[64];
	int iLen;
	
	pModem->bStep = MODEM_STEP_RST;
       
	while (1)
	{	
		//m_wRunCnt++;
		//��λģ��	
		if (pGL868->fPowOn == true)
		{
			pGL868->fPowOn = false;
            
			ModemPowerOn();         //ģ��Դ���ж�
                            
			if (pModem->bModuleVer==MODULE_GC864 || pModem->bModuleVer==MODULE_EF0306)
			{
				DTRACE(DB_FAPROTO, ("GC864::ResetGPRS: power on reset\r\n"));
				ResetGC864();
			}            
            else if (pModem->bModuleVer==MODULE_GL868)
            {
                DTRACE(DB_FAPROTO, ("GL868::ResetGPRS: power on reset\r\n"));
                ResetGL868();
            }
			else
			{
				DTRACE(DB_FAPROTO, ("CFapME3000::ResetGPRS: power on reset\r\n"));
				ResetME3000();
			}
		}
		else
		{
			ATCommand(pModem, "ATH\r\n", "OK", NULL, NULL, 0);
			ATCommand(pModem, "+++\r\n", "OK", NULL, NULL, 0);
			Sleep(500);
			ATCommand(pModem,"ATH\r\n", "OK", NULL, NULL, 0);
			Sleep(500);
			if (wErrCnt!=0 && wErrCnt%6==0)  //��ģ���Դ
			{
				DTRACE(DB_FAPROTO, ("CGL868/ME3000::ResetGPRS: power down reset!\r\n"));
				
				ModemPowerOff();	//��ģ���Դ����Ҫ2������
				//Sleep(2000);
				ModemPowerOn();                    
				//Sleep(1000);
			}
			
			if (pModem->bModuleVer==MODULE_GC864 || pModem->bModuleVer==MODULE_EF0306)
			{
				DTRACE(DB_FAPROTO, ("CGC864::ResetGPRS: reset modem.\r\n"));
				ResetGC864();
			}
            else if (pModem->bModuleVer==MODULE_GL868)
            {
                DTRACE(DB_FAPROTO, ("GL868::ResetGPRS: reset modem.\r\n"));
                ResetGL868();
            }            
			else
			{
				DTRACE(DB_FAPROTO, ("CFapME3000::ResetGPRS: reset modem.\r\n"));
				ResetME3000();
			}
		}

/*		if (pModem->bModuleVer==MODULE_GC864 || pModem->bModuleVer==MODULE_EF0306 || pModem->bModuleVer==MODULE_GL868)
		{
			Sleep(1000);
		}*/

		if (pModem->bModuleVer == MODULE_ME3000)
		{	
			for (j=0; j<15; j++)
			{				
				Sleep(100);
				memset(bBuf, 0, sizeof(bBuf))	;
				wRead = CommRead(pModem->bComm, bBuf, 63, 1000);
				if (pModem->bModuleVer == MODULE_ME3000)
				{
					if (wRead>=8 && bufbuf(bBuf, wRead, (BYTE *)"+CFUN: 1", 8)!=NULL)  //����������Ϣ
					{	
						DTRACE(DB_FAPROTO, ("MODULE_ME3000::ResetGPRS : rx +CFUN: 1.\r\n"));
						Sleep(500);
						break;
					}
				}
			}
		}
		
		if (ATCmdTest(pModem, 3))
		{
		    if (pModem->bModuleVer == MODULE_EF0306)
		    {//�ն�Э��ջ������ȫ0������
		    	for (i=0; i<5; i++)
		    	{
		    		if (ATCommand(pModem, "AT&F\r\n", "OK", NULL, NULL, 0) > 0)
		    			break;
		    	}
		        ATCommand(pModem, "AT$GATEWAY=\"10.98.96.7\"\r\n", "OK", NULL, NULL, 3);
		    }
		    	
			wErrCnt = 0;
			if (ATCommand(pModem, "AT+IPR?\r\n", "+IPR: 115200", NULL, NULL, 0) <= 0)  //����������Ӧ�İѲ������趨Ϊ9600
			{
				ATCommand(pModem, "AT+IPR=115200\r\n", "OK", NULL, NULL, 3);
				Sleep(500);
				ATCommand(pModem, "AT&W\r\n", "OK", NULL, NULL, 2);
				Sleep(500);
			}

			if (pModem->bModuleVer == MODULE_GC864 || pModem->bModuleVer == MODULE_GL868)
			{
				ATCommand(pModem, "AT+CGMR\r\n", "OK", NULL, NULL, 0);
				
				for (i=0; i<3; i++)
				{
					if (ATCommand(pModem, "AT&K0\r\n", "OK", NULL, NULL, 0) > 0)
					{
						ATCommand(pModem, "AT+FLO?\r\n", "OK", NULL, NULL, 0);
						break;
					}
				}
			}
			
			//Sleep(1000);			
			break;
		}

		wErrCnt++;	//����ʧ�ܴ����ۼ�
		if (wErrCnt%3 == 0)  //AT���ͨ��,�����޸Ĳ�����
		{
		    DTRACE(DB_FAPROTO, ("CFapGC864/ME3000/GL868::ResetGPRS: Test CBR_9600.\r\n"));
			CommSetBaudRate(pModem->bComm, CBR_9600);            
            
			Sleep(500);
			if (ATCmdTest(pModem, 3))
			{
			    DTRACE(DB_FAPROTO, ("CFapGC864/ME3000/GL868::ResetGPRS: Set Modem BaudRate to 115200.\r\n"));		
				ATCommand(pModem, "AT+IPR=115200\r\n", "OK", NULL, NULL, 0);
				Sleep(1000);
				CommSetBaudRate(pModem->bComm, CBR_115200);
				Sleep(500);
				iLen = ATCommand(pModem, "AT&W\r\n", "OK", NULL, NULL, 2);
				if (iLen > 0)
					break;
			}
			else
			{
				CommSetBaudRate(pModem->bComm, CBR_115200);
				Sleep(500);
			}
		}

		//����ģ��,��Ҫ̫Ƶ����λģ��
		if	(wErrCnt > 15)
		{
			Sleep(60*1000);
			return MODEM_RST_FAIL;
		}
		else
		{
			Sleep(2000);
		}
	}
    
	return MODEM_NO_ERROR;
}
/*
//��ѯ�ͻ����Ƿ������
WORD GetGL868SendBufLeft(struct TModem* pModem, bool fSvr)
{
    char* p;
    WORD wLen;
    WORD wBufLen;
    BYTE bBuf[40];
	char szCmd[20];
	char szConnect[32];

   	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    if (fSvr== false)
    {
    	sprintf(szCmd, "AT+IPSTATUS=%d\r\n", pGL868->bCliSock);		//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
         if (pGL868->fCliUdp)
          	sprintf(szConnect, "+IPSTATUS:%d,CONNECT,UDP,", pGL868->bCliSock);
        else
        	sprintf(szConnect, "+IPSTATUS:%d,CONNECT,TCP,", pGL868->bCliSock);
       	DTRACE(DB_FAPROTO, ("GL868ChkCliStatus : tx AT+IPSTATUS=%d.\r\n", pGL868->bCliSock));
    }
    else
    {
        sprintf(szCmd, "AT+CLIENTSTATUS=%d\r\n", pGL868->bSvrSock);		//�������˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
         if (pGL868->fCliUdp)
          	sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT,UDP,", pGL868->bSvrSock);
        else
        	sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT,TCP,", pGL868->bSvrSock);
        
       	DTRACE(DB_FAPROTO, ("GL868ChkSvrStatus : tx AT+CLIENTSTATUS=%d.\r\n", pGL868->bSvrSock));
    }    
   
	wLen = strlen(szCmd);
    if (CommWrite(pModem->bComm, (BYTE *)szCmd, wLen, 1000) != wLen) 
        return 0;
	
	Sleep(500);  //����Read���յ���ǰ��ͨ�����ݶ���������
	
	WORD wRead = CommRead(pModem->bComm, bBuf, sizeof(bBuf), 2000);
	bBuf[wRead] = 0;

	//+CSQ:
	p = strstr((char* )bBuf, szConnect);
	if (p==NULL)
		return 0;
    
    p += strlen(szConnect);
    wBufLen = str2num(&p);
    return wBufLen;
}*/

//��Ϊ�ͻ��˷�������
int GL868CliSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen)
{
    //static DWORD dwLastTick=0;
    //static WORD wLastSnd=0;
    
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    
    //if ()//�ն�Э��ջ
    //{            
    if (SocketSend(pGL868->pSocket, pbTxBuf, wLen)) //todotodo:���SOCKET�Ͽ�����ʲô��
        return wLen;
    return 0;
    //}
    //else//ģ��Э��ջ
  /*   {
       
        if (pModem->bModuleVer == MODULE_ME590)
	{
		TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
		WORD nStrLen;
	 	int iGprsRet = -1;
		char szCmd[32];
		char szAnsOK[32];
		BYTE bBuf[10];
		
		//if (!ATCmdTest(pModem, 1))
		//	return 0;
        if (wLastSnd >= 300) //��һ֡����800�ֽڣ���Ҫ�ȴ�3��ģ�鷢�����ܽ��ŷ�
        {
            while (GetTick()-dwLastTick<=15000)
            {
                if (GetGL868SendBufLeft(pModem, false) > 1800) //2047Ϊ��󻺳���
                {
                    Sleep(100);
                    break;
                }

                Sleep(1200);
            }
        }
		
		if (pGL868->fCliUdp)
			sprintf(szCmd, "AT+UDPSEND=%d,%d\r\n", pGL868->bCliSock, wLen);	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
		else
			sprintf(szCmd, "AT+TCPSEND=%d,%d\r\n", pGL868->bCliSock, wLen);	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0

		nStrLen = strlen(szCmd);

		if (CommWrite(pModem->bComm, (BYTE *)szCmd, nStrLen, 1000) != nStrLen)
			return 0;
		
		if (WaitModemAnswer(pModem, ">", NULL, NULL, 3) <= 0)
   			return 0;

		if (CommWrite(pModem->bComm, pbTxBuf, wLen, 1000) != wLen)
			return 0;        
		
		bBuf[0] = '\r';
		if (CommWrite(pModem->bComm, bBuf, 1, 1000) != 1)
			return 0;
        
        wLastSnd = wLen;
		if (pGL868->fCliUdp)
		{
			sprintf(szAnsOK, "+UDPSEND:%d,%d", 0, wLen);
			iGprsRet = WaitModemAnswer(pModem, szAnsOK, "+UDPSEND:Error", NULL, 10);
		}
		else
		{
			sprintf(szAnsOK, "+TCPSEND:%d,%d", 0, wLen);
			iGprsRet = WaitModemAnswer(pModem, szAnsOK, "+TCPSEND:Error", "+TCPSEND:Buffer not enough", 10);
		}
        
        dwLastTick = GetTick();
		if (iGprsRet > 0)
		{	
			DTRACE(DB_FAPROTO, ("CGL868::Send : ggprs tx cnt=%d.\r\n", wLen));
			return wLen;
		}
		else if (iGprsRet == -1)
		{
			DTRACE(DB_FAPROTO, ("CGL868::Send : send failed.\r\n"));
			return 0;
		}		
	}
    //else if (pModem->bModuleVer==MODULE_GL868)
    //{
    //}
        
    }*/
	//return 0;
}

//��������Ϊ�ͻ��˽�������,����ֱ�ӷ������ݣ�ֻ��ʹ�ûص��ķ�ʽ
int GL868CliReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    int iRxLen = SocketReceive(pGL868->pSocket, pbRxBuf, wBufSize);
//    if (wRxLen > 0)
//        GL868SpecHandle(pModem, (char* )pbRxBuf, wRxLen, wBufSize);
    
   /* 
	WORD wRxLen = (WORD )CommRead(pModem->bComm, pbRxBuf, wBufSize, 200);
	if (wRxLen > 0)
	{
		pbRxBuf[wRxLen] = 0;
		GL868SpecHandle(pModem, (char* )pbRxBuf, wRxLen, wBufSize);
	}*/

	return iRxLen;	//����ֱ�ӷ������ݣ�ֻ��ʹ�ûص��ķ�ʽ        
}

//���ͻ��˵�socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
bool GL868ChkCliStatus(struct TModem* pModem)	
{    
    /*
    TGL868* pGL868 = (TGL868* )pModem->pvModem;
    	
    if (pGL868->pSocket->iSocket == INVALID_SOCKET)
        return false;
    
    return true;
    */
    
    BYTE bTxBuf[2];
    bTxBuf[0] = 0x55;
    
    if (GL868CliSend(pModem, bTxBuf, 1) == 0)
        return false;
    else
        return true;
    /*
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	char szCmd[20];
	char szConnect[32];
	char szDisConnect[32];

	sprintf(szCmd, "AT+IPSTATUS=%d\r\n", pGL868->bCliSock);		//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
	sprintf(szConnect, "+IPSTATUS:%d,CONNECT", pGL868->bCliSock);
	sprintf(szDisConnect, "+IPSTATUS:%d,DISCONNECT", pGL868->bCliSock);

	if (ATCommand(pModem, szCmd, szConnect, szDisConnect, NULL, 3) > 0)
		return true;
	else
		return false;*/    
}

//��Ϊ�ͻ������ӷ�����
bool GL868Connect(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    return SocketConnect(pGL868->pSocket);
    /*
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	DWORD dwClick = GetClick();
	char szCmd[48];
	int iReTry = 0;
	

	pGL868->fCliUdp = fUdp;
	
	if (pModem->bModuleVer == MODULE_ME590)
	{
		//AT+TCPSETUP=0,220.199.66.56,6800 
		//OK 
		//+TCPSETUP:0,OK 		
	TRYAGAIN:	
		if (fUdp)
		{
			sprintf(szCmd, "AT+UDPCLOSE=%d\r\n", pGL868->bCliSock);	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
			ATCommand(pModem, szCmd, "OK", NULL, NULL, 6);
			sprintf(szCmd, "AT+ZIPSETUPU=0,%d.%d.%d.%d,%d\r\n", (dwRemoteIP>>24)&0xff, (dwRemoteIP>>16)&0xff, (dwRemoteIP>>8)&0xff, dwRemoteIP&0xff, wRemotePort);	
		}
		else
		{
//			sprintf(szCmd, "AT+TCPCLOSE=%d\r\n", pGL868->bCliSock);
//			ATCommand(pModem, szCmd, "OK", NULL, NULL, 6);
			sprintf(szCmd, "AT+TCPSETUP=0,%d.%d.%d.%d,%d\r\n", (dwRemoteIP>>24)&0xff, (dwRemoteIP>>16)&0xff, (dwRemoteIP>>8)&0xff, dwRemoteIP&0xff, wRemotePort);
		}	
		
		if (ATCommand(pModem, szCmd, "+TCPSETUP:", "+TCPSETUP:0,FAIL", NULL, 20) > 0)
		{
			if (!fUdp)	
			{
				//AT+IPSTATUS=0 
				//+IPSTATUS:0,CONNECT,TCP,2047 
				if (ATCommand(pModem, "AT+IPSTATUS=0\r\n", "+IPSTATUS:0,CONNECT", "+IPSTATUS:0,DISCONNECT", NULL, 75) > 0)
					return true;
			}
			else
			{
				if (ATCommand(pModem, "AT+IPSTATUS=0\r\n", "+IPSTATUS:0,CONNECT,UDP", NULL, NULL, 75) > 0)
					return true;
			}
		}
				
		if (iReTry++ < 3)
			goto TRYAGAIN;
	}
    //else if (pModem->bModuleVer == MODULE_GL868)
    //{//�ն�Э��ջ����SOCKET��������
    //}
    
	return false; */
}

bool GL868CloseCliSock(struct TModem* pModem)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    return SocketClose(pGL868->pSocket);
    /*
	if (pModem->bModuleVer == MODULE_ME590)
	{
		TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
		char szCmd[48];
		sprintf(szCmd, "AT+TCPCLOSE=%d\r\n", pGL868->bCliSock);	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
		ATCommand(pModem, szCmd, "OK", "+TCPCLOSE:Error", NULL, 3);

		sprintf(szCmd, "AT+UDPCLOSE=%d\r\n", pGL868->bCliSock);	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
		ATCommand(pModem, szCmd, "OK", "+UDPCLOSE:Error", NULL, 3);
        
        //lwip_close(pGL868->bCliSock);
	}
    //else if (pModem->bModuleVer==MODULE_GL868)
    //{
    //}
	*/	
}

DWORD g_dwLocalIp = 0;	//����IP
void SetLocalAddr(DWORD dwIP)
{
	g_dwLocalIp = dwIP;
}

DWORD GetLocalAddr()
{
	return g_dwLocalIp;
}

//����PPP����
bool GL868OpenPpp(struct TModem* pModem)
{
    if (pModem->bModuleVer == MODULE_GL868 || pModem->bModuleVer == MODULE_GC864)
    {
#ifndef SYS_WIN
        pppSetAuth(PPPAUTHTYPE_ANY, "CARD", "CARD");
        
    	pModem->pd = pppOpen(&pModem->bComm, NULL, NULL);
        if (pModem->pd >= 0)
            return true;
#else
		return true;
#endif
    }
    
    return false;
}

//�Ͽ�PPP����
bool GL868ClosePpp(struct TModem* pModem)
{
	if (pModem->bModuleVer == MODULE_GL868 || pModem->bModuleVer == MODULE_GC864)
	{
#ifndef SYS_WIN
        if (pModem->pd >= 0)
        {
    		if (pppClose(pModem->pd) == 0)
        		return true;
            else
                return false;
        }
#endif
	}

	return true;
}

/*
//��������MODEM���յ������в����Ƿ���
char* GL868IsRxIpFrm(struct TModem* pModem, char* pszRx, bool* pfSvr, WORD* pwLen)
{
//���յ���վ�����ݣ�+TCPRECV(S) 
//+TCPRECV(S):1,10,1234567899

//ָʾ���յ��� TCP ����
//+TCPRECV:<n>,<length>,<data>
//+TCPRECV:0,10,1234567890
	TGL868* pGL868 = (TGL868* )pModem->pvModem;

	char* p = strstr(pszRx, "+TCPRECV");
	if (p == NULL)
		return NULL;

	if (strstr(p, "+TCPRECV(S)") == p)	//��ͷ�ľ��Ƿ�����ͷ
    {
		*pfSvr = true;        
        p += strlen("+TCPRECV(S):");
        if (*p>='0' && *p<='9')
			pGL868->bSvrSock = *p - '0';
        else
            return NULL;
    }
	else
    {
		*pfSvr = false;
    }

	p = strchr(p, ',');
	if (p == NULL)
		return NULL;

	p++;	//��������
	*pwLen = str2num(&p);
	return p;
}*/


//��������ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������
bool GL868SpecHandle(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize)
{
	//TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	//char* p;
	//BYTE* pbRxFrm;
	//DWORD dwTick;
	//WORD wLen, wFrmLen;
	bool fSvr = false;
    
    //pbRxFrm = NULL;

	//1.��������յ�TCP/UDP���ģ���������ֽڶ�����ŷ���
	//pbRxFrm = (BYTE* )GL868IsRxIpFrm(pModem, pszRxBuf, &fSvr, &wFrmLen); //�Ƿ���յ�TCP/UDP����
	//if (pbRxFrm != NULL)	//���յ�TCP/UDP����
	//{
		//if (pbRxFrm < (BYTE *)pszRxBuf+wRxLen)	//����ָ�����
		//{
			if (GprsIfRxFrm(pModem->pProIf, (BYTE *)pszRxBuf, wRxLen, fSvr)) //���������֡
				return true;
		//}

		//��û���������֡������£���������ʣ���ֽڲŷ���
		//dwTick = GetTick();
		//do
		//{
			//wLen = CommRead(pModem->bComm, (BYTE* )pszRxBuf, wBufSize, 500);
			//if (GprsIfRxFrm(pModem->pProIf, (BYTE* )pszRxBuf, wLen, fSvr)) //���������֡
				//return true;

			//if (wLen == 0)
				//Sleep(10);
		//} while (GetTick()-dwTick < 5000);

		//return false;
	//}
	
#ifdef EN_PROIF_SVR		//֧�ַ�����
	//2.���Ƿ����յ����Լ����˿ڵ�������
	//Connect AcceptSocket=1,ClientAddr=119.123.77.133
    
//    if (wRxLen>0)
//        DTRACE(DB_FAPROTO, ("GL868SpecHandle : rx %s, wRxLen=%d.\r\n", pszRxBuf, wRxLen));
/*	p = strstr(pszRxBuf, "Connect AcceptSocket=");
	if (p != NULL)
	{
		p += strlen("Connect AcceptSocket=");
		if (*p>='0' && *p<='9')
		{
			pGL868->bSvrSock = *p - '0';
			return true;
		}
	}*/
#endif //EN_PROIF_SVR		//֧�ַ�����

	return false;
}


#ifdef EN_PROIF_SVR		//֧�ַ�����
//��������Ϊ�����������˿�
bool GL868Listen(struct TModem* pModem, bool fUdp, WORD wLocalPort)  
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    return listen(pGL868->pSocket->iSocket, BACKLOG);
    /*
	char* p;
//	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	DWORD dwRead;
	//BYTE bSvrSock;	//�������˵�SOCK
	char szCmd[48];
	BYTE bBuf[128];

	GL868CloseListen(pModem);

    Sleep(4000);
	sprintf(szCmd, "AT+TCPLISTEN=%d\r\n", wLocalPort);

	//DTRACE(DB_FAPROTO, ("GL868Listen : tx %s", szCmd));
	if (CommWrite(pModem->bComm, (BYTE *)szCmd, strlen(szCmd), 1000) != strlen(szCmd)) 
		return false;

	Sleep(500);  //����Read���յ���ǰ��ͨ�����ݶ���������
	dwRead = CommRead(pModem->bComm, bBuf, sizeof(bBuf), 2000);
	bBuf[dwRead] = 0;
	//DTRACE(DB_FAPROTO, ("GL868Listen : rx %s.\r\n", bBuf));
	
	pModem->pfnSpecHandle(pModem, (char* )bBuf, dwRead, 128); //��ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������

	//+TCPLISTEN:0,OK    ������������ʼ����
	p = strstr((char* )bBuf, "+TCPLISTEN:");
	if (p == NULL)
	{
		DTRACE(DB_FAPROTO, ("GL868Listen: failed to listen\r\n"));
		return false;
	}
	
	p += 11;	//����+TCPLISTEN:

	if ((*p>='0' || *p<='3') && 
		*(p+1)==',' && *(p+2)=='O' && *(p+3)=='K')
	{
		return true;
	}
	else	//+TCPLISTEN:bind error  ��ʧ��
	{
		return false;
	}*/
}

//�������رշ������Ѿ����ӵ�socket
bool GL868CloseSvrSock(struct TModem* pModem)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    return SocketClose(pGL868->pSocket);
    /*
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	pGL868->bSvrSock = GL868_INVALID_SOCK;
	return ATCommand(pModem, "AT+CLOSECLIENT\r\n", "+CLOSECLIENT:", NULL, NULL, 3);*/
}

//�������رռ���
bool GL868CloseListen(struct TModem* pModem)
{/*
//	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	GL868CloseSvrSock(pModem);

	//pGL868->bSvrSock = GL868_INVALID_SOCK;
	return ATCommand(pModem, "AT+CLOSELISTEN\r\n", "+CLOSELISTEN:", NULL, NULL, 3);*/
    
    return true;
}

//��������Ϊ��������������,����ֱ�ӷ������ݣ�ֻ��ʹ�ûص��ķ�ʽ
int GL868SvrReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    return SocketReceive(pGL868->pSocket, pbRxBuf, wBufSize);
    
	//return M590CliReceive(pModem, pbRxBuf, wBufSize);
}

//��Ϊ��������������
int GL868SvrSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    
    //�ն�Э��ջ
   	if (SocketSend(pGL868->pSocket, pbTxBuf, wLen))
        return wLen;
    
    return 0;
    /*
    static WORD wLastSnd=0;
    static DWORD dwLastSndTick = 0;
	if (pModem->bModuleVer == MODULE_ME590)
	{
		TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
		WORD nStrLen;
	 	int iGprsRet = -1;
		char szCmd[32];
		char szAnsOK[32];
        char szAnsErr1[20];
		BYTE bBuf[10];
		
		if (pM590->bSvrSock == M590_INVALID_SOCK)	//������SOCK�Ѿ���ʹ��
			return 0;
        
//        if (GetM590SendBufLeft(pModem, true) <= 0)
//            return 0;

        if (wLastSnd > 300)
        {
            while (GetTick()-dwLastSndTick<=15000)
            {
                if (GetM590SendBufLeft(pModem, true) > 1800) //2047Ϊ��󻺳���
                {
                    Sleep(100);
                    break;
                }
    
                Sleep(1200);
            }
        }
        
		sprintf(szCmd, "AT+TCPSENDS=%d,%d\r\n", pM590->bSvrSock, wLen);

		nStrLen = strlen(szCmd);
		if (CommWrite(pModem->bComm, (BYTE *)szCmd, nStrLen, 1000) != nStrLen)
			return 0;
		
        iGprsRet = WaitModemAnswer(pModem, ">", "+TCPSENDS:Error", NULL, 3);
		if (iGprsRet <= 0)
        {
            if (iGprsRet == -1)
                Sleep(1);
            
			return 0;
        }
		
		if (CommWrite(pModem->bComm, pbTxBuf, wLen, 1000) != wLen)
			return 0;
		
		bBuf[0] = '\r';
		if (CommWrite(pModem->bComm, bBuf, 1, 1000) != 1)
			return 0;
		
        wLastSnd = wLen;
        dwLastSndTick = GetTick();
		sprintf(szAnsOK, "+TCPSENDS:%d,%d", pM590->bSvrSock, wLen);
   		sprintf(szAnsErr1, "+TCPSENDS:%d,-1", pM590->bSvrSock);
		iGprsRet = WaitModemAnswer(pModem, szAnsOK, szAnsErr1, "+TCPSENDS:Error", 10);
		if (iGprsRet > 0)
		{	
			DTRACE(DB_FAPROTO, ("CM590::Send : gprs tx cnt=%d.\r\n", wLen));
			return wLen;
		}
		else if (iGprsRet == -1)
		{
			DTRACE(DB_FAPROTO, ("CM590::Send : send ret = -1 failed.\r\n"));
			return 0;
		}else if (iGprsRet == -2)
        {
        	DTRACE(DB_FAPROTO, ("CM590::Send : send ret = -2 failed.\r\n"));
   			return 0;
        }
	}
    //else if (pModem->bModuleVer==MODULE_GL868)
    //{
    //}
	return 0;*/
}

//����������socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
bool GL868ChkSvrStatus(struct TModem* pModem)
{
    BYTE bTxBuf[2];
    bTxBuf[0] = 0x55;
    if (GL868SvrSend(pModem, bTxBuf, 1) == 0)  //����һ���ֽڲ�����·�Ƿ���Ч��
        return false;
    else
        return true;
    
    /*
	TM590* pM590 = (TM590* )pModem->pvModem;	//MODEM��������,ָ�����ʵ��
	char szCmd[24];
	char szConnect[32];
	char szDisConnect[32];

	if (pM590->bSvrSock != M590_INVALID_SOCK)	//������SOCK�Ѿ���ʹ��
	{			
		sprintf(szCmd, "AT+CLIENTSTATUS=%d\r\n", pM590->bSvrSock);
		sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT", pM590->bSvrSock);
		sprintf(szDisConnect, "+CLIENTSTATUS:%d,DISCONNECT", pM590->bSvrSock);

		if (ATCommand(pModem, szCmd, szConnect, szDisConnect, "+CLIENTSTATUS:Error", 3) > 0)
			return true;
		else
			return false;        
	}
	else	//������ͨ����û��ʹ��
	{
		return false;
	}*/
}

bool GL868IsSvrAcceptOne(struct TModem* pModem)
{
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	return pGL868->bSvrSock != GL868_INVALID_SOCK;
}

#endif //EN_PROIF_SVR		//֧�ַ�����
