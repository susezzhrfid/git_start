/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�M590.h
 * ժ    Ҫ�����ļ�ʵ����M590MODEM����
 * ��ǰ�汾��1.0
 * ��    �ߣ������᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ProIfCfg.h"
#include "FaCfg.h"
//#include "FaConst.h"
#include "SysArch.h"
#include "drivers.h"
#include "M590.h"
#include "ComAPI.h"
//#include "LibDbAPI.h"
#include "DrvConst.h"
//#include "SysAPI.h"
#include "SysDebug.h"
#include "GprsIf.h"
//#include "GbPro.h"
#include "ProPara.h"
#ifndef SYS_WIN
#include "ppp.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//M590˽�к궨��
#define M590_INVALID_SOCK	0xff	//��Ч��socketͨ��

////////////////////////////////////////////////////////////////////////////////////////////
//M590˽�г�Ա����

/////////////////////////////////////////////////////////////////////////////////////////////////////
//M590ʵ��

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

void M590Init(struct TModem* pModem)
{
	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��

	//����ṹ��Ա��ʼ��
	ModemInit(pModem);
    

	//����

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pModem->pfnResetModem = M590ResetModem;		//��λģ��
	pModem->pfnUpdateSignStrength = UpdateSignStrength;	//���³�ǿ
	pModem->pfnInitAPN = M590InitAPN;			//��ʼ��APN

	pModem->pfnOpenPpp = M590OpenPpp;			//PPP��������
	pModem->pfnClosePpp = M590ClosePpp;		//�Ͽ�PPP����

	pModem->pfnConnect = M590Connect; 		//��Ϊ�ͻ������ӷ�����
	pModem->pfnCloseCliSock = M590CloseCliSock;	//�رտͻ���socket

	pModem->pfnCliSend = M590CliSend; 	//��Ϊ�ͻ��˷�������
	pModem->pfnCliReceive = M590CliReceive; 	//��Ϊ�ͻ��˽�������
	pModem->pfnSpecHandle = M590SpecHandle; //��ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������

	pModem->pfnChkCliStatus = M590ChkCliStatus;	//���ͻ��˵�socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ

#ifdef EN_PROIF_SVR		//֧�ַ�����
	pModem->pfnListen = M590Listen;  		//��Ϊ�����������˿�
	pModem->pfnCloseListen = M590CloseListen;		//�رռ���
	pModem->pfnCloseSvrSock = M590CloseSvrSock;	//�رշ������Ѿ����ӵ�socket
	pModem->pfnSvrSend = M590SvrSend; 	//��Ϊ��������������
	pModem->pfnSvrReceive = M590SvrReceive; 	//��Ϊ��������������
	pModem->pfnIsSvrAcceptOne = M590IsSvrAcceptOne;	//��Ϊ�������Ƿ���յ�һ���ͻ��˵�����
	pModem->pfnChkSvrStatus = M590ChkSvrStatus;	//����������socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
#endif //EN_PROIF_SVR		//֧�ַ�����

	//����������ʼ��
	//pModem->pszCSQ = "AT+CSQ\r\n";
	pM590->bCliSock = 0;	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
	pM590->fPowOn = true;	//�Ƿ��Ǹ��ϵ�
	pM590->fCliUdp = false;	//�ͻ����Ƿ���UDP����
   
#ifdef EN_PROIF_SVR		//֧�ַ�����
	pM590->bSvrSock = M590_INVALID_SOCK;	//�������˵�socket�ţ�ֻ��Ϊ0~9����ֵ0xff��ʾ��Ч��
#endif //EN_PROIF_SVR		//֧�ַ�����
}
 
int M590InitAPN(struct TModem* pModem)
{    
	char szApnCmd[96];
	char szApn[32];
    char szUser[33];
    char szPsw[33];
	int  iRet;		
	WORD i;
    
    struct TProIf* pProIf = pModem->pProIf;
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	
	pModem->bStep = MODEM_STEP_SIM;
	//�������Ƿ�ע�����,ֻ��Ϊ�˷������        
	for (i=0; i<30; i++)
	{
		if (ATCommand(pModem, "AT+CPIN?\r\n", "OK", "ERROR", NULL, 3) == 1)  //�����Ƿ���
		{   
			break;
		}
		else
		{//���ǲ����Ȳ�εģ��������������ⲻ�����Ļ���û�б�Ҫ���������
       		if (ATCommand(pModem, "AT+CPIN?\r\n", "OK", "ERROR", NULL, 3) < 0)  //�����Ƿ���    ,����0û��Ӧ��
    			return MODEM_SIM_FAIL;
		}
		Sleep(1500);
	}
    if (i >= 30)
    {
        DTRACE(DB_FAPROTO, ("CM590::InitAPN : SIM no answer!\r\n"));
        return MODEM_SIM_FAIL;
    }
    
    for (i=0; i<30; i++)//�������Ų�ע�᲻�������ԭ��
    {
        iRet = UpdateSignStrength(pModem);
	    if (iRet>0 && iRet<=31)
	    {
            //SignLedCtrl(iRet);
        	break;
        }
        Sleep(3000);     
    }
    if (i >= 30)
    {
        DTRACE(DB_FAPROTO, ("CM590::InitAPN : Update SignStrength Fail!\r\n"));
        return MODEM_CSQ_FAIL;
    }
        
    for (i=0; i<30; i++)
    {
        pModem->bStep = MODEM_STEP_REG;
		iRet = ATCommand(pModem, "AT+CREG?\r\n", "+CREG: 0,1", "+CREG: 0,5", "+CREG: 0,0", 3); //+CREG: 0,3   ����������վ
		if (iRet==1 || iRet==-1)
			break;		
        Sleep(2000);
    }		
	if (i >= 30)//����ûע���,��ȥ����
	{
		DTRACE(DB_FAPROTO, ("CM590::InitAPN : Network Register Fail!\r\n"));
		return MODEM_REG_FAIL;
	}	
        
    if (pGprs->bCnMode == CN_MODE_SOCKET)
    {
        if (memcmp(pModem->tModemInfo.bManuftr, "FIBO", 4) == 0) //���ͨ��AT&D1
            ATCommand(pModem, "AT&D1\r\n", "OK", NULL, NULL, 3);//��������������ģʽ������ģʽ�໥�л�   //�ն�Э��ջ����Ҫ
        else //�з�����ԶAT&D2    
            ATCommand(pModem, "AT&D2\r\n", "OK", NULL, NULL, 3);
    }
    
    memset(szApn, 0, sizeof(szApn));
    GetApn(szApn);
    GetUserAndPsw(szUser, szPsw);    
    if (pGprs->bCnMode == CN_MODE_EMBED)
    {        
        ATCommand(pModem, "AT$MYTYPE?\r\n", "OK", "ERROR", NULL, 3);
        
        sprintf(szApnCmd, "AT$MYNETCON=0,\"APN\",\"%s\"\r\n", szApn);
        if (ATCommand(pModem, szApnCmd, "OK", "ERROR", NULL, 3) <= 0)
            return MODEM_OTHER_FAIL;
        
        //ATCommand(pModem, "AT$MYNETCON=0,\"USERPWD\",\"None,None\"\r\n", "OK", "ERROR", NULL, 3);// �û������������ã�����ʱΪ��
        sprintf(szApnCmd, "AT$MYNETCON=0,\"USERPWD\",\"%s,%s\"\r\n", szUser, szPsw);
        if (ATCommand(pModem, szApnCmd, "OK", "ERROR", NULL, 3) <= 0)
            return MODEM_OTHER_FAIL;
        if (ATCommand(pModem, "AT$MYNETCON=0,\"AUTH\",1\r\n", "OK", "ERROR", NULL, 3) <= 0)
            return MODEM_OTHER_FAIL;  //������֤��ʽ PAP
        if (ATCommand(pModem, "AT$MYIPFILTER=0,2\r\n", "OK", "ERROR", NULL, 3) <= 0)
            return MODEM_OTHER_FAIL;         //ɾ��ȫ��IP��֤ͨ��
        if (ATCommand(pModem, "AT$MYNETURC=1\r\n", "OK", "ERROR", NULL, 3) <= 0)
        //if (ATCommand(pModem, "AT$MYNETURC=0\r\n", "OK", "ERROR", NULL, 3) <= 0)
            return MODEM_OTHER_FAIL;             //������Э��ջ�����ϱ�
        if (ATCommand(pModem, "AT$MYNETACT=0,1\r\n", "OK", "ERROR", NULL, 5) <=0 ) //150
            return MODEM_OTHER_FAIL;//����  // ͨ��0����PDP    ����PPP ����
        
        for (i=0; i<30; i++)
        {
            Sleep(2000);
            iRet = ATCmdGetInfo(pModem, "AT$MYNETACT?\r\n", "OK", NULL, "$MYNETACT: 0,", szApnCmd, sizeof(szApnCmd), 4);
            if (iRet > 0)
            {
                if (szApnCmd[0] == '1')//�����Ѽ���    todo:IP��Ҫ���ô��
                    break;
            }
            else if (i>3 && iRet==0) //ģ�鱻�ε���
                return MODEM_OTHER_FAIL;
            Sleep(2000);
        }
        if (i >= 10)
            return MODEM_OTHER_FAIL;
    }
    else if (pGprs->bCnMode == CN_MODE_SOCKET)
    {
        sprintf(szApnCmd, "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", szApn);
        Sleep(500);
        if (ATCommand(pModem, szApnCmd, "OK", NULL, NULL, 3) <= 0)   //ʧ���˶���һ��
        {					//APNĿǰֻ�ڵ�¼GPRSʱ���ã�SMSû������
            ATCommand(pModem, szApnCmd, "OK", NULL, NULL, 3);
        }
        
        //�����֤   AT+XGAUTH=1,1,"gsm","1234"
        //sprintf(szApnCmd, "AT+XGAUTH=1,1,\"%s\",\"%s\"\r\n", szUser, szPsw);  //����376.3�鲻������ָ��
        //ATCommand(pModem, szApnCmd, "OK", "ERROR", NULL, 3);
    }
    //ATCommand(pModem, "AT#PSNT?\r\n", "OK", NULL, NULL, 3);
    
    //ATCommand(pModem, "AT+CFUN?\r\n", "OK", NULL, NULL, 3);
    
	return MODEM_NO_ERROR;
}

int M590ResetModem(struct TModem* pModem)
{
	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    struct TProIf* pProIf = pModem->pProIf;
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	WORD j;
	WORD wErrCnt = 0;
	WORD wRead = 0;
	BYTE bBuf[64];
    BYTE bCnType;
	
	pModem->bStep = MODEM_STEP_RST;
    pM590->fDataUpload = false;   //û�������ϱ�
    pM590->fLinkErr = false;
    pM590->fInitiativeErr = false;
	
    ModemPowerOn();  //Added by wing 2014/07/30
    Sleep(1000);		
    
    bCnType = GetModeState();
    bCnType &= 0x07;   //����GPRSģ��ʱΪ 010=0x02
    if (bCnType != 0x02) //bCnType!=0x06 && bCnType!=0x04) //����GPRSҲ������̫��,�ص�4V��Դ
    {
        pGprs->bSignStrength = 0;    //�ص��źŵ�        
        ModemPowerOff();
        DTRACE(DB_FAPROTO, ("M590ResetModem: Modem state is error.\r\n"));
        return MODEM_RST_FAIL;
    }

	while (1)
	{	
		ReadItemEx(2, 0, 0x10d3, &bCnType); //�����µ���������
        if (bCnType == CN_TYPE_ET) //��⵽����̫��,�Ǿ�����MODEM�ĳ�ʼ��������̫��
            return MODEM_RST_FAIL;
               
		//��λģ��	
		if (pM590->fPowOn == true)
		{
			pM590->fPowOn = false;
			ModemPowerOn();
		}
		else
		{
			//ATCommand(pModem, "ATH\r\n", "OK", NULL, NULL, 0);//�Ҷϵ绰��������ҵ��
			//ATCommand(pModem, "+++", "OK", NULL, NULL, 0);//�Ͽ���������   Note:+++���ܼ��κη���
			//Sleep(500);
			//ATCommand(pModem,"ATH\r\n", "OK", NULL, NULL, 0);
			//Sleep(500);
            DTRACE(DB_FAPROTO, ("M590::ResetGPRS: reset modem.\r\n"));
            ResetM590();
		}
        for (j=0; j<5; j++)
        {				
            Sleep(1000);
            memset(bBuf, 0, sizeof(bBuf));
            wRead = CommRead(pModem->bComm, bBuf, 63, 1000);
            if (wRead>=13 && bufbuf(bBuf, wRead, (BYTE *)"MODEM:STARTUP", 13)!=NULL)  //����������Ϣ
            {	
                DTRACE(DB_FAPROTO, ("ME590::ResetGPRS : rx MODEM:STARTUP.\r\n"));
                Sleep(2000);
                break;
            }
        }
		if (ATCmdTest(pModem, 3))
		{
			wErrCnt = 0;
			/*if (ATCommand(pModem, "AT+IPR?\r\n", "+IPR: 115200", NULL, NULL, 0) <= 0)  //����������Ӧ�İѲ������趨Ϊ115200
			{
				ATCommand(pModem, "AT+IPR=115200\r\n", "OK", NULL, NULL, 3);
				Sleep(500);
				ATCommand(pModem, "AT&W\r\n", "OK", NULL, NULL, 2);
				Sleep(500);
			}*/

            if (GetModemVer(pModem))
            {                
                GetModemCCID(pModem);
                UpdModemInfo(&pModem->tModemInfo); //���Ƶ�����߽���Ƶ��дFLASH  //CCID���²��ɹ�Ҳ����ģ����Ϣ�����ݿ�
            }            
			//Sleep(1000);			
			break;
		}

		wErrCnt++;	//����ʧ�ܴ����ۼ�
/*		if (wErrCnt%3 == 0)  //AT���ͨ��,�����޸Ĳ�����
		{
		    DTRACE(DB_FAPROTO, ("ME590::ResetGPRS: Test CBR_9600.\r\n"));
			CommSetBaudRate(pModem->bComm, CBR_9600);            
            
			Sleep(500);
			if (ATCmdTest(pModem, 3))
			{
			    DTRACE(DB_FAPROTO, ("ME590::ResetGPRS: Set Modem BaudRate to 115200.\r\n"));		
				ATCommand(pModem, "AT+IPR=115200\r\n", "OK", NULL, NULL, 0);
				Sleep(1000);
				CommSetBaudRate(pModem->bComm, CBR_115200);
				Sleep(500);
				if (ATCommand(pModem, "AT&W\r\n", "OK", NULL, NULL, 2) > 0)
					break;
			}
			else
			{
				CommSetBaudRate(pModem->bComm, CBR_115200);
				Sleep(500);
			}
		}*/

		//����ģ��,��Ҫ̫Ƶ����λģ��
		if	(wErrCnt > 3/*15*/)
		{
			//Sleep(60*1000);
			return MODEM_RST_FAIL;
		}
		else
		{
			Sleep(2000);
		}
	}
    
	return MODEM_NO_ERROR;
}

//��ѯ�ͻ����Ƿ������
WORD GetM590SendBufLeft(struct TModem* pModem, bool fSvr)
{
    char* p;
    WORD wLen;
    WORD wBufLen;
    BYTE bBuf[40];
	char szCmd[20];
	char szConnect[32];

   	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    if (fSvr== false)
    {
    	sprintf(szCmd, "AT+IPSTATUS=%d\r\n", pM590->bCliSock);		//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
         if (pM590->fCliUdp)
          	sprintf(szConnect, "+IPSTATUS:%d,CONNECT,UDP,", pM590->bCliSock);
        else
        	sprintf(szConnect, "+IPSTATUS:%d,CONNECT,TCP,", pM590->bCliSock);
       	DTRACE(DB_FAPROTO, ("M590ChkCliStatus : tx AT+IPSTATUS=%d.\r\n", pM590->bCliSock));
    }
    else
    {
        sprintf(szCmd, "AT+CLIENTSTATUS=%d\r\n", pM590->bSvrSock);		//�������˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
         if (pM590->fCliUdp)
          	sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT,UDP,", pM590->bSvrSock);
        else
        	sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT,TCP,", pM590->bSvrSock);
        
       	DTRACE(DB_FAPROTO, ("M590ChkSvrStatus : tx AT+CLIENTSTATUS=%d.\r\n", pM590->bSvrSock));
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
}

//��BCD���ASCII������ȡHEX����
//��0x32, 0x30, 0x34, 0x37��2,0,4,7����ӦBCDΪ2047��HEXΪ7FF
//���ڴ�ģ��Э��ջͨ��ʱȡ����
/*  //��SearchStrVal��ͬ
DWORD GetDataFromBcdAscii(BYTE *p, BYTE bLen)
{
    BYTE i;
    DWORD dwData;
    BYTE bBuf[4];
    if (bLen>8 || bLen==0)
        return 0;
    for (i=0; i<bLen; i++)
    {
        p[i] -= '0';  //ASCII -> BCD   
    }
    if ((bLen&0x01)==0x00) //ż��
    {
        for (i=0; i<bLen>>1; i++)
        {
            bBuf[i] = p[i*2+1] | (p[i*2]<<4);  //��λƴ��һ��BCD��,��λ��ǰ
        }
        Swap(bBuf, i);
        dwData = BcdToDWORD(bBuf, i);
    }
    else //����
    {
        for (i=0; i<((bLen>>1)+1); i++)
        {
            if (i==0)
                bBuf[i] = p[i];
            else
                bBuf[i] = p[(i-1)*2+2] | (p[(i-1)*2+1]<<4);  //��λƴ��һ��BCD��,��λ��ǰ
        }
        Swap(bBuf, i);
        dwData = BcdToDWORD(bBuf, i);  //BCD -> HEX
    }
    return dwData;
}*/

bool GetMyNetAck(struct TModem* pModem, DWORD *pdwNAckLen, DWORD *pdwBufLen)
{
    //AT$MYNETACK
    char szBuf[32];
    char szCmd[32];
    char szCmd2[32];
    char *p = szBuf;
    char *p2;
    BYTE i;
    int iRet;
    TM590* pM590 = (TM590* )pModem->pvModem;	
    
    if (pM590->fCliUdp)  //UDP   NETACK��UDP��Ч
    {
        *pdwNAckLen = 0;
        *pdwBufLen = 2047;
        return true;
    }
                
    sprintf(szCmd, "AT$MYNETACK=%d\r\n", pM590->bCliSock);//��ѯ�Ƿ���յ�ACKȷ�Ϻͷ��ͻ�������С
    sprintf(szCmd2, "$MYNETACK: %d,", pM590->bCliSock);
    iRet = ATCmdGetInfo(pModem, szCmd, "OK", "ERROR", szCmd2, szBuf, sizeof(szBuf), 2);
    if (iRet > 0)
    {
        p2 = szBuf;
        for (i=0; i<8; i++)
        {
            if (*p++ == ',')
                break;
        }
        //*pdwNAckLen = GetDataFromBcdAscii((BYTE *)p2,i);
        *pdwNAckLen = SearchStrVal(p2, p2+i);
        p2 = p;
        for (i=0; i<8; i++)
        {
            if (*p++ == '\r')
                break;
        }
        //*pdwBufLen = GetDataFromBcdAscii((BYTE *)p2,i); 
        *pdwBufLen = SearchStrVal(p2, p2+i);
        return true;
    }
    return false;
}

//��Ϊ�ͻ��˷�������
int M590CliSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen)
{
    TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    
    char szCmd[140];
    char szCmd2[24];    
    WORD wTxLen;
    BYTE *p = pbTxBuf;    
    static DWORD sdwNAckLen = 0;
    static DWORD sdwBufLen = 0;
    static DWORD dwClick = 0;    
    //static DWORD dwSendClick = 0;
    DWORD dwNAckLen;
    DWORD dwBufLen;    
    int iLen;
    
    //1���ȶ�ȡ�Ƿ����ϱ���Ϣ
    iLen = CommRead(pModem->bComm, (BYTE *)szCmd, sizeof(szCmd)-1, 0);
    if (iLen > 0)
    {
        M590SpecHandle(pModem, szCmd, iLen, sizeof(szCmd));
        if (pM590->fLinkErr)//��·�Ͽ��ˣ����÷������ˡ�
        {
            DTRACE(DB_FAPROTO, ("M590CliSend: link disconnet!\r\n"));            
            return -1;
        }
    }
    
    if (dwClick==0 && sdwNAckLen==0 && sdwBufLen==0) //��һ�β�
    {
        if (GetMyNetAck(pModem, &dwNAckLen, &dwBufLen)) //����֮ǰ�Ȳ��·��ͻ�����
        {
            dwClick = GetClick();
            sdwNAckLen = dwNAckLen;
            sdwBufLen = dwBufLen;
        }
    }
    else if ((dwClick!=0) && (GetClick()-dwClick>10))//���ǵ�һ��, �����ϴβ�ѯ������10S
    {
        if (GetMyNetAck(pModem, &dwNAckLen, &dwBufLen)) //����֮ǰ�Ȳ��·��ͻ�����
        {
            if (((dwNAckLen!=0) && (dwNAckLen>=sdwNAckLen)) && //����10S��δӦ��֡��û�м���
                ((dwBufLen!=2047) && (dwBufLen<=sdwBufLen)))   //���ͻ�����Խ��Խ���ˡ�        �п���ǰ��շ�����һ֡���������Ͼ����飬���ܲ鵽�������ˡ�
            {                
                //if ((dwSendClick!=0) && (GetClick()-dwSendClick>5))//�ϴη��͵����ڹ���5S��    //Ҳ������SLEEP(3000),Ȼ���ٲ�һ��ACK                
                sdwNAckLen = dwNAckLen;
                sdwBufLen = dwBufLen;
                Sleep(3000); //3S
                if (GetMyNetAck(pModem, &dwNAckLen, &dwBufLen))
                {
                    if (((dwNAckLen!=0) && (dwNAckLen>=sdwNAckLen)) && //����3S��δӦ��֡��û�м���
                        ((dwBufLen!=2047) && (dwBufLen<=sdwBufLen)))   //���ͻ�����Խ��Խ���ˡ�                    
                    {
                        DTRACE(DB_FAPROTO, ("M590CliSend: more than 10s data leave in sendbuf!\r\n"));
                        pM590->fInitiativeErr = true;
                        return -1;
                    }
                }
            }
            dwClick = GetClick(); //����ʱ��
            sdwNAckLen = dwNAckLen;
            sdwBufLen = dwBufLen;
        }        
    }                                  
                             
    while(wLen > 0)
    {
        //1���ȶ�ȡ�Ƿ����ϱ���Ϣ
        /*iLen = CommRead(pModem->bComm, (BYTE *)szCmd, sizeof(szCmd)-1, 0);
        if (iLen > 0)
        {
            sprintf(szCmd2, "$MYURCCLOSE: %d", pM590->bCliSock);//socket�Ͽ�        
            if (bufbuf((BYTE *)szCmd, iLen, (BYTE *)szCmd2, strlen(szCmd2)) != NULL)
                return -1;
            if (bufbuf((BYTE *)szCmd, iLen, "$MYURCACT: 0,0", strlen(szCmd2)) != NULL)//�������ӶϿ�
                return -1;
        }*/
        //GetMyNetAck(pModem, &dwNAckLen, &dwBufLen); //����֮ǰ�Ȳ��·��ͻ�����
        //2����������
    TRYAGIN:
        wTxLen = (wLen>1024)?1024:wLen;
        sprintf(szCmd, "AT$MYNETWRITE=%d,%d\r\n", pM590->bCliSock, wTxLen); //����wTxLen������
        //nStrLen = strlen(szCmd);
        //if (CommWrite(pModem->bComm, szCmd, nStrLen, 200) != wTxLen)
            //return 0;
        sprintf(szCmd2, "$MYNETWRITE: %d,%d", pM590->bCliSock, wTxLen);
        if (ATCommand(pModem, szCmd, szCmd2, "ERROR", NULL, 3) <= 0)
        {
            if (GetMyNetAck(pModem, &dwNAckLen, &dwBufLen))  //�����ǲ��Ƿ��ͻ���������
            {
                dwClick = GetClick(); //����ʱ��
                sdwNAckLen = dwNAckLen;
                sdwBufLen = dwBufLen;
                if (dwBufLen < wTxLen)
                {                    
                    Sleep(3000);//��3S
                    if (GetMyNetAck(pModem, &dwNAckLen, &dwBufLen))  //�����ǲ��Ƿ��ͻ���������
                    {
                        if (dwBufLen <= sdwBufLen) //����һ������Ҳû�з���
                        {
                            DTRACE(DB_FAPROTO, ("M590CliSend: Modem sendbuf is full,send fail!\r\n"));
                            pM590->fInitiativeErr = true;
                            return -1;
                        }
                        else if (dwBufLen >= wTxLen)
                            goto TRYAGIN;
                    }
                }
            }
            return 0;
        }
        if (CommWrite(pModem->bComm, p, wTxLen, 3000) == wTxLen)
        {
            wLen -= wTxLen;
            p += wTxLen;
            if (wLen != 0) //���к���֡
                Sleep(1000);
        }
        if (WaitModemAnswer(pModem, "OK", "ERROR", NULL, 3) <= 0) 
            return 0;
    }       

	return p-pbTxBuf;
}
/*
int M590RcvBlock(BYTE *pbRxBuf, WORD wBufSize)
{
    BYTE bBuf[128];
    BYTE *p;
    int iLen;
    BYTE bStep = 0;
    WORD i;
    BYTE bRxBuf[128];
    BYTE *pRx;
    
    pRx = bRxBuf;
    WORD wRxCnt;
    WORD wRxPtr;
    BYTE b;
    
    while(1)
    {
        iLen = CommRead(pModem->bComm, bBuf, sizeof(bBuf), 0);
        p = bBuf;
        for (i=0; i<iLen; i++)
        {       
            b = *p++;
            switch(bStep)
            {
            case 0: //��ͷ   $
                if (b == '$')
                {
                    *pRx++ = '$';
                    //wRxPtr = 1;
                    wRxCnt = 2;   //��һ���ȶ�2���ֽ�
                    bStep = 1;
                    break;
                }             
            case 1: //��   MY
                *pRx++ = b;
                wRxCnt--;
                if (wRxCnt == 0)  //�����������ֽ�
                {
                    if ((*(pRx-2)=='M') && (*(pRx-1)=='Y'))
                    {
                    }
                    //��ͷ������ͷ
                    p -= 2;  //��������
                    i -= 2;
                    bStep = 0;
                    pRx = bRxBuf;  //���ջ���Ҳ�ص�ԭ��
                }
                
                p = bufbuf(bBuf, iLen, "$MY", 3);
                if (p == NULL)//û�ҵ�ͷ������ȫ����ջ���
                {
                    
                }
            }
            
            
            sprintf(szCmd2, "$MYURCCLOSE: %d", pM590->bCliSock);//socket�Ͽ�        
            if (bufbuf((BYTE *)szBuf, iLen, (BYTE *)szCmd2, strlen(szCmd2)) != NULL)
                return -1;
            if (bufbuf((BYTE *)szBuf, iLen, "$MYURCACT: 0,0", strlen(szCmd2)) != NULL)//�������ӶϿ�
                return -1;
        }
        else  //û��������
            break;
    }
}*/
#include "opt.h"
#include "ppp.h"
extern BYTE outpacket_buf[NUM_PPP][PPP_MRU+PPP_HDRLEN]; 

//��������Ϊ�ͻ��˽�������,����ֱ�ӷ������ݣ�ֻ��ʹ�ûص��ķ�ʽ
int M590CliReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize)
{
    TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    BYTE *pbBufHead = outpacket_buf[0];
    const WORD wBufLen = sizeof(outpacket_buf[0]);
    static WORD wHead=0, wTail=0;
    WORD wRdLen;
    BYTE* p;
    char szCmd2[20];
    int iRet;    
    char *psz;
    char *psz2;
    int iLen;
    int iRdLen;
    BYTE i = 0;
    BYTE bszLen;
    static BYTE bFailCnt = 0;    
    
    
    if (wHead < wTail)//a�����������������ݣ�ֱ�ӷ��ظ�������
    {
        wRdLen = (wTail-wHead)>wBufSize ? wBufSize : wTail-wHead;
        memcpy(pbRxBuf, &pbBufHead[wHead], wRdLen);
        wHead += wRdLen;
        return wRdLen;
    }    
    else//b��������û�����ˣ�ȥM590���ȡ
    {
        wHead = 0;
        wTail = 0;
    }    
    
    //1���ȶ�ȡ�Ƿ����ϱ���Ϣ   Ӧ��һֱ������Ϊֹ
    iLen = 0;
    iRet = 0;
    
    if (pM590->fLinkErr)
    {
        DTRACE(DB_FAPROTO, ("M590CliReceive: link disconnet!\r\n"));
        pM590->fLinkErr = false;
        return -1;
    } 
    if (pM590->fInitiativeErr) //�������Ͽ�
    {
        DTRACE(DB_FAPROTO, ("M590CliReceive: Initiative disconnet!\r\n"));
        pM590->fInitiativeErr = false;
        return -1;
    } 
    
    wRdLen = CommRead(pModem->bComm, pbBufHead, wBufLen-1, 0);        
    if (wRdLen > 0)
    {   
        //DTRACE(DB_FAPROTO, ("M590CliReceive: rx len=%d.\r\n", wRdLen));
        pbBufHead[wRdLen] = 0;
             
        sprintf(szCmd2, "$MYURCCLOSE: %d", pM590->bCliSock);//socket�Ͽ�   
        bszLen = strlen(szCmd2);
        if (bufbuf(pbBufHead, wRdLen, (BYTE *)szCmd2, bszLen) != NULL)
        {
            DTRACE(DB_FAPROTO, ("M590CliReceive: socket disconnet!\r\n"));
            return -1;
        }
        sprintf(szCmd2, "$MYURCACT: 0,0");
        bszLen = strlen(szCmd2);
        if (bufbuf(pbBufHead, wRdLen, (BYTE *)szCmd2, bszLen) != NULL)//�������ӶϿ�
        {
            DTRACE(DB_FAPROTO, ("M590CliReceive: link disconnet!\r\n"));
            return -1;
        }
        
        //�յ������ϱ��ˣ��Ų�ѯ������Ƶ������ģ�������������
        sprintf(szCmd2, "$MYURCREAD: %d\r\n", pM590->bCliSock);
        bszLen = strlen(szCmd2);
        if (bufbuf(pbBufHead, wRdLen, (BYTE *)szCmd2, bszLen) != NULL)  //��������Ҫ��ȡ
        {
            pM590->fDataUpload = true;
            //dwClick = GetClick();
            DTRACE(DB_FAPROTO, ("M590CliReceive: M590 data upload.\r\n"));
            goto ReadData1;
        }
    }
    
    if (pM590->fDataUpload) //�������ϱ�
    {
        goto ReadData1;  
    }        
    return 0;//û�������ˡ�
    
    //2����ȡ����
ReadData1:        
    p = pbBufHead;
    sprintf((char *)p, "AT$MYNETREAD=%d,%d\r\n", pM590->bCliSock, 1460);//��1-1460������
    sprintf(szCmd2, "$MYNETREAD: %d,", pM590->bCliSock);  
    bszLen = strlen(szCmd2);
    //CommRead(pModem->bComm, NULL, 0, 0);    //��AT����ǰ�����崮�ڻ�����
	wRdLen = strlen((char *)p);
    if (CommWrite(pModem->bComm, p, wRdLen, 100) != wRdLen)
		return -2;
    
    iLen = 0;
    iRet = 0;
	for (i=0; i<30; i++)
    {                  
		wRdLen = CommRead(pModem->bComm, p+iLen, wBufLen-iLen-1, 20);
        iLen += wRdLen;
        if (iLen >= wBufLen)
            iLen = wBufLen-1;
		p[iLen] = 0;
        		
        psz = (char *)bufbuf(p, iLen, (BYTE *)szCmd2, bszLen);
        if (psz != NULL)//���յ���ȷ�ش�
		{
            iRet = iLen;
            bFailCnt = 0;
    		break; 
		}
		
        if (iLen ==wBufLen-1) //Buf ��
        {
            iLen = bszLen-1;
            memcpy(p, p+wBufLen-bszLen, iLen);//���һ���ݲ���������п���szBuf�Ŀռ䲻���ˣ���Ҫ�Ƴ�һЩ�ռ�            
        }
	}    
    if (i >= 30) //ģ��û����Ӧ
    {
        bFailCnt++;        
        DTRACE(DB_FAPROTO, ("M590CliReceive: read no answer.\r\n"));
        Sleep(1000);  //ģ��æ
        if (bFailCnt >= 3) //����3��û����Ӧ,ģ������ô��
        {
            bFailCnt = 0;
            return -1;
        }
        return 0;
    }
    iLen = 0;
    iRdLen = 0;        
    if (psz != NULL)
    {               
        iRet -= psz-(char *)p;//pszǰ�������ȫ����� //�п���szBuf�Ŀռ䲻���ˣ���Ҫ�Ƴ�һЩ�ռ�
        iRet -= bszLen;
        psz += bszLen;   
        memcpy(p, psz, iRet);  //ֻ�������ź�
        p[iRet] = 0;
        psz = (char *)p;
        for (i=0; i<10; i++)
        {
            psz2 = strstr(psz, "\r\n");
            if (psz2 != NULL)
            {                
                iRdLen = SearchStrVal(psz, psz2);
                DTRACE(DB_FAPROTO, ("M590CliReceive: read data len=%d.\r\n", iRdLen));
                if (iRdLen == 0) //ģ��û��������
                {
                    pM590->fDataUpload = false;
                    return 0;
                }                                
                psz2 += 2; //psz2 ָ��������
                break;
            }
            else
            {
                iLen = CommRead(pModem->bComm, (BYTE *)(p+iRet), wBufLen-iRet-1, 20);
                iRet += iLen;
                *(p+iRet) = 0;
            }
        } 
        if (i >= 10) //û�ҵ�\r\n
        {            
            DTRACE(DB_FAPROTO, ("M590CliReceive: read len info lost.\r\n"));
            return 0;
        }
        wRdLen = (((char *)p+iRet-psz2)>iRdLen) ? iRdLen : ((char *)p+iRet-psz2);
        //memcpy(p, psz2, wRdLen);
        wHead = (BYTE *)psz2-p;
        p = (BYTE *)psz2;
        p += wRdLen;
        iRdLen -= wRdLen;
        for (i=0; i<100; i++) //9600��1500���ֽڣ���Ҫ2S
        {
            if (iRdLen <= 0)
                break;
            if (iRdLen > wBufLen-(p-pbBufHead))
                break; //����

            iLen = CommRead(pModem->bComm, p, iRdLen, 20);
            if (iLen > 0)
            {
                iRdLen -= iLen;         
                p += iLen;                        
            }                    
        }
        //CommRead(pModem->bComm, szBuf, 4, 10);//����OK\r\n
    }
        
    wTail = p-pbBufHead;
    
    if (wHead != wTail)//���������������ݣ�ֱ�ӷ��ظ�������
    {
        wRdLen = (wTail-wHead)>wBufSize ? wBufSize : wTail-wHead;
        memcpy(pbRxBuf, &pbBufHead[wHead], wRdLen);
        wHead += wRdLen;
        return wRdLen;
    }
    return 0;    
    
#if 0
    TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    BYTE* p;
    char szCmd2[20];
    int iRet;
    char szBuf[130];        
    char *psz;
    char *psz2;
    int iLen;
    int iRdLen;
    BYTE i = 0;
    WORD wDatLen;
    BYTE bszLen;
    static BYTE bFailCnt = 0;    
        
    p = pbRxBuf;
    
    /*static DWORD dwClick = 0;
    if (dwClick == 0)
        dwClick = GetClick();*/
    
    //1���ȶ�ȡ�Ƿ����ϱ���Ϣ   Ӧ��һֱ������Ϊֹ
    iLen = 0;
    iRet = 0;
    for (i=0; i<4; i++)
    {
        wDatLen = CommRead(pModem->bComm, (BYTE *)szBuf+iLen, sizeof(szBuf)-iLen-1, 0);        
        if (wDatLen > 0)
        {   
            DTRACE(DB_FAPROTO, ("M590CliReceive: rx len=%d.\r\n", wDatLen));
            iLen += wDatLen;
            if (iLen >= sizeof(szBuf))
                iLen = sizeof(szBuf)-1;
		    szBuf[iLen] = 0;
            
            iRdLen = 0;   //�ҵ���Ĵ�            
            sprintf(szCmd2, "$MYURCCLOSE: %d", pM590->bCliSock);//socket�Ͽ�   
            bszLen = strlen(szCmd2);
            if (bufbuf((BYTE *)szBuf, iLen, (BYTE *)szCmd2, bszLen) != NULL)
            {
                DTRACE(DB_FAPROTO, ("M590CliReceive: socket disconnet!\r\n"));
                return -1;
            }
            if (bszLen > iRdLen)
                iRdLen = bszLen;
            sprintf(szCmd2, "$MYURCACT: 0,0");
            bszLen = strlen(szCmd2);
            if (bufbuf((BYTE *)szBuf, iLen, (BYTE *)szCmd2, bszLen) != NULL)//�������ӶϿ�
            {
                DTRACE(DB_FAPROTO, ("M590CliReceive: link disconnet!\r\n"));
                return -1;
            }
            if (bszLen > iRdLen)
                iRdLen = bszLen;
            if (pM590->fLinkErr)
            {
                pM590->fLinkErr = false;
                return -1;
            }
            
            //�յ������ϱ��ˣ��Ų�ѯ������Ƶ������ģ�������������
            sprintf(szCmd2, "$MYURCREAD: %d\r\n", pM590->bCliSock);
            bszLen = strlen(szCmd2);
            if (bufbuf((BYTE *)szBuf, iLen, (BYTE *)szCmd2, bszLen) != NULL)  //��������Ҫ��ȡ
            {
                pM590->fDataUpload = true;
                //dwClick = GetClick();
                DTRACE(DB_FAPROTO, ("M590CliReceive: M590 data upload.\r\n"));
                goto ReadData1;
            }
            if (bszLen > iRdLen)
                iRdLen = bszLen;
               
            sprintf(szCmd2, "$MYNETREAD: %d,", pM590->bCliSock); //�ϴ��������ڻ�����
            bszLen = strlen(szCmd2);
            if (bufbuf((BYTE *)szBuf, iLen, (BYTE *)szCmd2, bszLen) != NULL)
            {                
                iRet = iLen;
                DTRACE(DB_FAPROTO, ("M590CliReceive: read lasttime data.\r\n"));
                goto ReadData2;
            }
            if (bszLen > iRdLen)
                iRdLen = bszLen;
            
            if (iLen == sizeof(szBuf)-1) //Buf ��
            {
                iLen = iRdLen-1;
                memcpy(szBuf, szBuf+sizeof(szBuf)-iRdLen, iLen);//���һ���ݲ���������п���szBuf�Ŀռ䲻���ˣ���Ҫ�Ƴ�һЩ�ռ�            
            }
        }
        
        if (pM590->fDataUpload) //�������ϱ�
        {
            //dwClick = GetClick();
            goto ReadData1;  
        }
        
        /*if (GetClick()-dwClick > 2) //�����һ��
        {
            dwClick = GetClick();
            goto ReadData1; 
        }*/
        
        if (wDatLen <= 0) //û�������ˡ�
            return 0;
    }
    return 0;
    
    //2����ȡ����
ReadData1:        
    p = pbRxBuf;
    sprintf(szBuf, "AT$MYNETREAD=%d,%d\r\n", pM590->bCliSock, wBufSize);//��1-1460������
    sprintf(szCmd2, "$MYNETREAD: %d,", pM590->bCliSock);  
    bszLen = strlen(szCmd2);
    CommRead(pModem->bComm, NULL, 0, 0);    //��AT����ǰ�����崮�ڻ�����
	wDatLen = strlen(szBuf);
    if (CommWrite(pModem->bComm, (BYTE *)szBuf, wDatLen, 100) != wDatLen)
		return -2;
    
    iLen = 0;
    iRet = 0;
    memset(szBuf, 0, sizeof(szBuf));
	for (i=0; i<30; i++)
    {                  
		wDatLen = CommRead(pModem->bComm, (BYTE *)(szBuf+iLen), sizeof(szBuf)-iLen-1, 20);
        iLen += wDatLen;
        if (iLen >= sizeof(szBuf))
            iLen = sizeof(szBuf)-1;
		szBuf[iLen] = 0;
        		
        if (bufbuf((BYTE *)szBuf, iLen, (BYTE *)szCmd2, bszLen) != NULL)//���յ���ȷ�ش�
		{
            iRet = iLen;
            bFailCnt = 0;
    		break; 
		}
		
        if (iLen == sizeof(szBuf)-1) //Buf ��
        {
            iLen = bszLen-1;
            memcpy(szBuf, szBuf+sizeof(szBuf)-bszLen, iLen);//���һ���ݲ���������п���szBuf�Ŀռ䲻���ˣ���Ҫ�Ƴ�һЩ�ռ�            
        }
	}    
    if (i >= 30) //ģ��û����Ӧ
    {
        bFailCnt++;        
        DTRACE(DB_FAPROTO, ("M590CliReceive: read no answer.\r\n"));
        if (bFailCnt >= 6) //����6��û����Ӧ,ģ������ô��
        {
            bFailCnt = 0;
            return -1;
        }
        return 0;
    }
ReadData2:    
    iLen = 0;
    iRdLen = 0;    
    bszLen = strlen(szCmd2);
    if (iRet > 0)
    {       
        psz = (char *)bufbuf((BYTE *)szBuf, iRet, (BYTE *)szCmd2, bszLen);//�п���szBuf�Ŀռ䲻���ˣ���Ҫ�Ƴ�һЩ�ռ�
        iRet -= psz-szBuf;//pszǰ�������ȫ�����
        iRet -= bszLen;
        psz += bszLen;   
        memcpy(szBuf, psz, iRet);  //ֻ�������ź�
        szBuf[iRet] = 0;
        psz = szBuf;
        for (i=0; i<10; i++)
        {
            psz2 = strstr(psz, "\r\n");
            if (psz2 != NULL)
            {                
                iRdLen = SearchStrVal(psz, psz2);
                DTRACE(DB_FAPROTO, ("M590CliReceive: read data len=%d.\r\n", iRdLen));
                if (iRdLen == 0) //ģ��û��������
                {
                    pM590->fDataUpload = false;
                    return 0;
                }                                
                psz2 += 2; //psz2 ָ��������
                break;
            }
            else
            {
                iLen = CommRead(pModem->bComm, (BYTE *)(szBuf+iRet), sizeof(szBuf)-iRet-1, 20);
                iRet += iLen;
                *(szBuf+iRet) = 0;
            }
        } 
        if (i >= 10) //û�ҵ�\r\n
        {            
            DTRACE(DB_FAPROTO, ("M590CliReceive: read len info lost.\r\n"));
            return 0;
        }
        wDatLen = ((szBuf+iRet-psz2)>iRdLen) ? iRdLen : (szBuf+iRet-psz2);
        memcpy(p, psz2, wDatLen);
        p += wDatLen;
        iRdLen -= wDatLen;
        for (i=0; i<10; i++)
        {
            if (iRdLen <= 0)
                break;
            if (iRdLen > wBufSize-(p-pbRxBuf))
                return 0; //����

            iLen = CommRead(pModem->bComm, p, iRdLen, 50);
            if (iLen > 0)
            {
                iRdLen -= iLen;         
                p += iLen;                        
            }                    
        }
        //CommRead(pModem->bComm, szBuf, 4, 10);//����OK\r\n
    }
    //GprsIfRxFrm(pModem->pProIf, pbRxBuf, p-pbRxBuf, fSvr); //���������֡
    
    DoLedBurst(LED_REMOTE_RX);
    
    //goto ReadData1;  //һֱ����û������
    return p-pbRxBuf;
#endif
        
    //int iRxLen = (WORD )CommRead(pModem->bComm, pbRxBuf, wBufSize, 200);
    //if (iRxLen > 0)
        //M590SpecHandle(pModem, (char* )pbRxBuf, iRxLen, wBufSize); 
    //return iRxLen;
    
   /* 
	WORD wRxLen = (WORD )CommRead(pModem->bComm, pbRxBuf, wBufSize, 200);
	if (wRxLen > 0)
	{
		pbRxBuf[wRxLen] = 0;
		GL868SpecHandle(pModem, (char* )pbRxBuf, wRxLen, wBufSize);
	}

	return 0;	//����ֱ�ӷ������ݣ�ֻ��ʹ�ûص��ķ�ʽ
        */
}


//���ͻ��˵�socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
bool M590ChkCliStatus(struct TModem* pModem)	
{
	//TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	//char szCmd[20];
	//char szConnect[32];
	//char szDisConnect[32];

	//sprintf(szCmd, "AT+IPSTATUS=%d\r\n", pM590->bCliSock);		//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
	//sprintf(szConnect, "+IPSTATUS:%d,CONNECT", pM590->bCliSock);
	//sprintf(szDisConnect, "+IPSTATUS:%d,DISCONNECT", pM590->bCliSock);

	//if (ATCommand(pModem, "AT$MYNETOPEN?\r\n", "OK", NULL, NULL, 3) > 0)
		return true;
	//else
		//return false;
}

//��Ϊ�ͻ������ӷ�����
bool M590Connect(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort)
{
	DWORD dwClick = GetClick();
	char szCmd[64];	
    TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��

	pM590->fCliUdp = fUdp;
    if (fUdp)
		sprintf(szCmd, "AT$MYNETSRV=0,%d,2,0,\"%d.%d.%d.%d:%d\"\r\n", pM590->bCliSock, (dwRemoteIP>>24)&0xff, (dwRemoteIP>>16)&0xff, (dwRemoteIP>>8)&0xff, dwRemoteIP&0xff, wRemotePort);	
	else
		sprintf(szCmd, "AT$MYNETSRV=0,%d,0,0,\"%d.%d.%d.%d:%d\"\r\n", pM590->bCliSock, (dwRemoteIP>>24)&0xff, (dwRemoteIP>>16)&0xff, (dwRemoteIP>>8)&0xff, dwRemoteIP&0xff, wRemotePort);
		
	if (ATCommand(pModem, szCmd, "OK", "ERROR", NULL, 5) > 0) //20
	{            
        //ATCommand(pModem, "AT$MYNETSRV?\r\n", "OK", "ERROR", NULL, 20);            
        sprintf(szCmd, "AT$MYNETOPEN=%d\r\n", pM590->bCliSock);              
        if (ATCommand(pModem, szCmd, "OK", "ERROR", NULL, 6) > 0)//75
		    return true;
	}
 
	return false;
}

bool M590CloseCliSock(struct TModem* pModem)
{
	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	char szCmd[32];
	sprintf(szCmd, "AT$MYNETCLOSE=%d\r\n", pM590->bCliSock);	//�ͻ��˵�socket�ţ�Ŀǰ�̶�ֻʹ��0
	ATCommand(pModem, szCmd, "OK", "ERROR", NULL, 5);
	    
	return true; 
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
bool M590PPPOpenEmbed(struct TModem* pModem)
{
    //ATCommand(pModem, "$MYNETSRV=0,0,0,0,\r\n", "OK", NULL, NULL, 3)
    return true;
    
    /*
    
    
	if (pModem->bModuleVer == MODULE_ME590)
	{
		BYTE bBuf[128];
		int iReTry = 0;
	
TRYAGAIN:
		if (ATCommand(pModem, "AT+XISP=0\r\n", "OK", NULL, NULL, 3) > 0)
		{
			if (ATCommand(pModem, "AT+XIIC=1\r\n", "OK", NULL, NULL, 3) > 0)
			{
//				at+xiic? 
//				+XIIC:    1, 10.11.72.103 
//				OK 
				DWORD dwClick = GetClick();
				do
				{
					int j = 0;
					bool fStart = false;
					BYTE bNumber = 0;
					char *szXIIC = "AT+XIIC?\r\n";
					DWORD dwRead = 0;
					bool fValid = false;
					
					DTRACE(DB_FAPROTO, ("CGC864::PPPOpen : tx AT+XIIC?.\r\n"));
				    if (CommWrite(pModem->bComm, (BYTE *)szXIIC, strlen(szXIIC), 2000) != strlen(szXIIC)) 
				        return false;
					
					Sleep(500);  //����Read���յ���ǰ��ͨ�����ݶ���������				
					dwRead = CommRead(pModem->bComm, bBuf, sizeof(bBuf), 2000);
					bBuf[dwRead] = 0;
					DTRACE(DB_FAPROTO, ("CGC864::PPPOpen : rx %s.\r\n", bBuf));
				
					//+XIIC:
					char* p = strstr((char* )bBuf, "+XIIC:");
					if (p == NULL)
					{
				        DTRACE(DB_FAPROTO, ("CGC864::PPPOpen : sending AT+XIIC?, without answer.\r\n"));
						return false;
					}
					DTRACE(DB_FAPROTO, ("CGC864::PPPOpen : Get local ip address .\r\n"));
					for (int i=0; i<dwRead; i++)
					{
						if (bBuf[i] == ',')
						{
							fStart = true;
							j = 0;
						}
						if (fStart)
						{
							if (bBuf[i]>='0' && bBuf[i]<='9')
							{								
								bNumber *= (j*10);
								bNumber += bBuf[i]-'0';
								j++;
							}
							else if (bBuf[i] == ' ')						
							{
								
							}
							else if (bBuf[i] == '.')
							{
							//	DTRACE(DB_FAPROTO, ("%d.", bNumber));
								if (bNumber != 0)
									fValid = true;
								j = 0;
							}
							else if (bBuf[i] == '\r')
							{
							//	DTRACE(DB_FAPROTO, ("%d\r\n", bNumber));
								if (bNumber != 0)
									fValid = true;							
								break;
							}
						}
					}
					if (fValid)
						return true;
				}
				while (GetClick()-dwClick < 60);
			}
		}
		iReTry++;
		if (iReTry < 3)
			goto TRYAGAIN;
	}	
	return false;*/
}

bool M590PPPCloseEmbed(struct TModem* pModem)
{
    return true;
	/*if (pModem->bModuleVer == MODULE_ME590)
	{
		ATCommand(pModem, "AT+CGATT=0\r\n", "OK", NULL, NULL, 1);
		return true;
	}
	return false;*/
}

//����PPP����
bool M590OpenPpp(struct TModem* pModem)
{
    BYTE bBuf[64];
    struct TProIf* pProIf = pModem->pProIf;
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
    if (pGprs->bCnMode == CN_MODE_SOCKET)
    {        
#ifndef SYS_WIN        
        ReadItemEx(0, 0, 0x010f, bBuf);
	    //memcpy(pWorkerPara->szPppUser, &bBuf[0], 32);
	    //memcpy(pWorkerPara->szPppPsw, &bBuf[32], 32);
        //pppSetAuth(PPPAUTHTYPE_ANY, "CARD", "CARD");  //todo:liyan  ������������
        pppSetAuth(PPPAUTHTYPE_ANY, &bBuf[0], &bBuf[32]);
    
        DTRACE(DB_FAPROTO, ("M590OpenPpp: pppOpen\n"));
        pGprs->ipd = pppOpen(&pModem->bComm, NULL, NULL);
        if (pGprs->ipd >= 0)
            return true;
#else
       	return true;
#endif
    }
    else if (pGprs->bCnMode == CN_MODE_EMBED)
    {
        if (M590PPPOpenEmbed(pModem))
            return true;
    }
    
    DTRACE(DB_FAPROTO, ("M590OpenPpp: pppOpen failed\n"));
    return false;
}

//�Ͽ�PPP����
bool M590ClosePpp(struct TModem* pModem)
{
    struct TProIf* pProIf = pModem->pProIf;
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
    if (pGprs->bCnMode == CN_MODE_SOCKET)
    {
#ifndef SYS_WIN
        if (pGprs->ipd >= 0)
        {
            DTRACE(DB_FAPROTO, ("M590OpenPpp: pppClose\n"));
            if (pppClose(pGprs->ipd) == 0)
            {
                pGprs->ipd = -1;
                return true;
            }
        }
#else
		return true;
#endif
    }
    else if (pGprs->bCnMode == CN_MODE_EMBED)
    {
        return M590PPPCloseEmbed(pModem);
    }

	return false;
}

/*
//��������MODEM���յ������в����Ƿ���
char* M590IsRxIpFrm(struct TModem* pModem, char* pszRx, bool* pfSvr, WORD* pwLen)
{
//���յ���վ�����ݣ�+TCPRECV(S) 
//+TCPRECV(S):1,10,1234567899

//ָʾ���յ��� TCP ����
//+TCPRECV:<n>,<length>,<data>
//+TCPRECV:0,10,1234567890
	TM590* pM590 = (TM590* )pModem->pvModem;

	char* p = strstr(pszRx, "+TCPRECV");
	if (p == NULL)
		return NULL;

	if (strstr(p, "+TCPRECV(S)") == p)	//��ͷ�ľ��Ƿ�����ͷ
    {
		*pfSvr = true;        
        p += strlen("+TCPRECV(S):");
        if (*p>='0' && *p<='9')
			pM590->bSvrSock = *p - '0';
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


//��������ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������, 6.5.1�����ݵ��������ϱ� $MYURCREAD
bool M590SpecHandle(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize)
{    
    TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
    
    char szCmd2[20];
    int iRet;
    char szBuf[130];        
    char *psz;
    char *psz2;
    int iLen;
    int iRdLen;
    BYTE i = 0;
    WORD wDatLen;
    bool fSvr = false;
    BYTE bszLen;
        
    pszRxBuf[wRxLen] = 0;
    
    //1���ȶ�ȡ�Ƿ����ϱ���Ϣ   Ӧ��һֱ������Ϊֹ
    iLen = wRxLen;
    if (iLen > 0)
    {   
        //DTRACE(DB_FAPROTO, ("M590SpecHandle: rx len=%d.\r\n", iLen));
        
        sprintf(szCmd2, "$MYURCCLOSE: %d", pM590->bCliSock);//socket�Ͽ�        
        if (bufbuf((BYTE *)pszRxBuf, iLen, (BYTE *)szCmd2, strlen(szCmd2)) != NULL)
        {
            DTRACE(DB_FAPROTO, ("M590SpecHandle: socket disconnet!\r\n"));
            pM590->fLinkErr = true;
            return false;
        }
        sprintf(szCmd2, "$MYURCACT: 0,0");
        if (bufbuf((BYTE *)pszRxBuf, iLen, "$MYURCACT: 0,0", strlen(szCmd2)) != NULL)//�������ӶϿ�
        {
            DTRACE(DB_FAPROTO, ("M590SpecHandle: link disconnet!\r\n"));
            pM590->fLinkErr = true;
            return false;
        }
        sprintf(szCmd2, "$MYURCREAD: %d\r\n", pM590->bCliSock);
        bszLen = strlen(szCmd2);
        if (bufbuf((BYTE *)pszRxBuf, iLen, (BYTE *)szCmd2, bszLen) != NULL)  //��������Ҫ��ȡ
        {            
            pM590->fDataUpload = true;
            DTRACE(DB_FAPROTO, ("M590SpecHandle: M590 data upload.\r\n"));
        }
        sprintf(szCmd2, "$MYNETREAD: %d,", pM590->bCliSock); //�ϴ��������ڻ�����
        bszLen = strlen(szCmd2);
        if (bufbuf((BYTE *)pszRxBuf, iLen, (BYTE *)szCmd2, bszLen) != NULL)
        {
            iRet = iLen;
            DTRACE(DB_FAPROTO, ("M590SpecHandle: read lasttime data.\r\n"));
            goto ReadData;
        }       
    }
    return false;
    
ReadData:    
    iLen = 0;
    iRdLen = 0;    
    bszLen = strlen(szCmd2);
    if (iRet > 0)
    {       
        psz = (char *)bufbuf((BYTE *)pszRxBuf, iRet, (BYTE *)szCmd2, bszLen);//�п���szBuf�Ŀռ䲻���ˣ���Ҫ�Ƴ�һЩ�ռ�                
        iRet -= psz-pszRxBuf;//pszǰ�������ȫ�����
        iRet -= bszLen;
        psz += bszLen;   
        memcpy(szBuf, psz, iRet);  //���õ����ݿ���szBuf,pszRxBuf��������ݲ��ܶ����������������õ���
        szBuf[iRet] = 0;
        psz = szBuf;
        
        for (i=0; i<20; i++)
        {
            psz2 = strstr(psz, "\r\n");
            if (psz2 != NULL)
            {                
                iRdLen = SearchStrVal(psz, psz2);                
                if (iRdLen == 0) //ģ��û��������
                {
                    //DTRACE(DB_FAPROTO, ("M590SpecHandle: read data over.\r\n"));
                    return false;
                }
                psz2 += 2; //psz2 ָ��������
                break;
            }
            else
            {
                iLen = CommRead(pModem->bComm, (BYTE *)(szBuf+iRet), sizeof(szBuf)-iRet-1, 10);
                iRet += iLen;
                *(szBuf+iRet) = 0;
            }
        } 
        if (i >= 20) //û�ҵ�\r\n
        {            
            DTRACE(DB_FAPROTO, ("M590SpecHandle: read len info lost.\r\n"));
            return false;
        }
        wDatLen = ((szBuf+iRet-psz2)>iRdLen) ? iRdLen : (szBuf+iRet-psz2);
        
        GprsIfRxFrm(pModem->pProIf, (BYTE *)psz2, wDatLen, fSvr); //���������֡

        iRdLen -= wDatLen;
        for (i=0; i<100; i++)
        {
            if (iRdLen <= 0)
                break;
            if (iRdLen > sizeof(szBuf))
                break; //����
            iLen = CommRead(pModem->bComm, (BYTE *)szBuf, iRdLen, 20);
            if (iLen > 0)
            {
                GprsIfRxFrm(pModem->pProIf, (BYTE *)szBuf, sizeof(szBuf), fSvr); //���������֡
                iRdLen -= iLen;               
            }                    
        }
        //CommRead(pModem->bComm, szBuf, 4, 10);//����OK\r\n
    }
    return false;
}


#ifdef EN_PROIF_SVR		//֧�ַ�����
//��������Ϊ�����������˿�
bool M590Listen(struct TModem* pModem, bool fUdp, WORD wLocalPort)  
{
	char* p;
//	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	DWORD dwRead;
	//BYTE bSvrSock;	//�������˵�SOCK
	char szCmd[48];
	BYTE bBuf[128];

	M590CloseListen(pModem);

    Sleep(4000);
	sprintf(szCmd, "AT+TCPLISTEN=%d\r\n", wLocalPort);

	//DTRACE(DB_FAPROTO, ("M590Listen : tx %s", szCmd));
	if (CommWrite(pModem->bComm, (BYTE *)szCmd, strlen(szCmd), 1000) != strlen(szCmd)) 
		return false;

	Sleep(500);  //����Read���յ���ǰ��ͨ�����ݶ���������
	dwRead = CommRead(pModem->bComm, bBuf, sizeof(bBuf), 2000);
	bBuf[dwRead] = 0;
	//DTRACE(DB_FAPROTO, ("M590Listen : rx %s.\r\n", bBuf));
	
	pModem->pfnSpecHandle(pModem, (char* )bBuf, dwRead, 128); //��ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������

	//+TCPLISTEN:0,OK    ������������ʼ����
	p = strstr((char* )bBuf, "+TCPLISTEN:");
	if (p == NULL)
	{
		DTRACE(DB_FAPROTO, ("M590Listen: fail to listen\r\n"));
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
	}
}

//�������رշ������Ѿ����ӵ�socket
bool M590CloseSvrSock(struct TModem* pModem)
{
	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��

	pM590->bSvrSock = M590_INVALID_SOCK;
	return ATCommand(pModem, "AT+CLOSECLIENT\r\n", "+CLOSECLIENT:", NULL, NULL, 3);
}

//�������رռ���
bool M590CloseListen(struct TModem* pModem)
{
//	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	M590CloseSvrSock(pModem);

	//pM590->bSvrSock = M590_INVALID_SOCK;
	return ATCommand(pModem, "AT+CLOSELISTEN\r\n", "+CLOSELISTEN:", NULL, NULL, 3);
}

//��������Ϊ��������������,����ֱ�ӷ������ݣ�ֻ��ʹ�ûص��ķ�ʽ
int M590SvrReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize)
{
	return M590CliReceive(pModem, pbRxBuf, wBufSize);
}

//��Ϊ��������������
int M590SvrSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen)
{    
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
	return 0;
}

//����������socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
bool M590ChkSvrStatus(struct TModem* pModem)
{
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
	}
}

bool M590IsSvrAcceptOne(struct TModem* pModem)
{
	TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM��������,ָ�����ʵ��
	return pM590->bSvrSock != M590_INVALID_SOCK;
}

#endif //EN_PROIF_SVR		//֧�ַ�����
