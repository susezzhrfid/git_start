/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProPara.c
 * ժ    Ҫ�����ļ���Ҫ�����Ѹ�Э�鲻ͬ�Ĳ���װ�ص���ͬ�Ĳ����ṹ��ȥ,
 *			 ��TSocket,TGprs��,ʹ���õ�ͨ�Ŵ��벻��ֱ����Ը���
 *			 Э��Ĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#include "ProStruct.h"
#include "Pro.h"
#include "DrvCfg.h"
#include "ProIf.h"
//#include "GbPro.h"
#include "Modem.h"
#include "M590.h"
#include "CommIf.h"
#include "GprsIf.h"
#include "DbFmt.h"
#include "ComAPI.h"
#include "DbAPI.h"
#include "SysDebug.h"
#include "SocketIf.h"
#include "ProPara.h"
#include "ctrl.h"

#define FAP_TYPE_OFFSET  		0
#define FAP_IP_OFFSET  			2
#define FAP_DNS_OFFSET  		33
#define FAP_FRONT_PORT_OFFSET  	0
#define FAP_CONCENT_PORT_OFFSET	39
#define FAP_APN_OFFSET  		24
#define FAP_APN_USER_OFFSET  	0
#define FAP_APN_PSW_OFFSET  	16
#define FAP_NETMAST_OFFSET  	89
#define FAP_GATEWAY_OFFSET  	93

void LoadGprsPara(TGprsIf* pGprs)
{
    static bool first = true;
	int iScale;
	BYTE bMode;
	//BYTE bType = 1;
	BYTE bBuf[64];
    WORD wSvrBeatMin;
	bMode = 1;
    
    ReadItemEx(BN1, PN0, 0x2032, bBuf);//TCP/IPЭ��⣺0ģ���Դ�,1�ն�
    if (bBuf[0] == 0)
        pGprs->bCnMode = CN_MODE_EMBED;
    else
        pGprs->bCnMode = CN_MODE_SOCKET;
    
#ifdef SYS_WIN
	pGprs->bCnType = CN_TYPE_ET;
#else
	pGprs->bCnType = CN_TYPE_GPRS;
#endif
    WriteItemEx(BN2, PN0, 0x10d3, &pGprs->bCnType); //����������GPRS����̫���л�,��ʶ��ǰͨ������
    
    if (first)
    {
        pGprs->ipd = -1;  //���ز���ʱ���������¸�ipd��ֵ��
        first = false;
    }
    
    LoadSockPara(pGprs->pSocketIf);
    
    pGprs->dwNoRxRstAppInterv = 0;
    ReadItemEx(BN10, PN0, 0xa142, (BYTE* )&pGprs->dwNoRxRstAppInterv); //��ͨѶ��λ�ն�ʱ��,��λ����,HEX,Ĭ��4��
    if (pGprs->dwNoRxRstAppInterv == 0) //����0�����𷴸���λ
        pGprs->dwNoRxRstAppInterv = 5760;
	pGprs->dwNoRxRstAppInterv *= 60;

	//һ�����
	pGprs->bRstToDormanNum = 2;	//��λ�����ߵĴ���:�κ������ʧ�ܻᵼ��ģ�鸴λ����ģ�鸴λ����������������󣬽�������״̬
	pGprs->bConnectNum = 2;		//����ʧ���������ԵĴ���
	pGprs->bLoginNum = 2; 		//��¼ʧ���������ԵĴ���

	ReadItemEx(BN0, PN0, 0x8017, bBuf);
	if (bBuf[0] == 0) 
		bBuf[0] = 0x15;
	pGprs->bCliBeatMin = BcdToByte(bBuf[0]);
    
	pGprs->wDormanInterv = 0;   //����ʱ����, ��λ��, ,0��ʾ��ֹ����ģ
    
    pGprs->bSignStrength = 0;  //�ź�ǿ�ȳ�ʼ����0
	
	//ģʽ����
	if (bMode == 0)	//����ǻ��ģʽ�����ǰ����ϱ�
	{
		DTRACE(DB_FAPROTO, ("LoadGprsPara:: GPRS is in mix-mode\r\n"));
		pGprs->bOnlineMode = ONLINE_M_MIX;
		pGprs->fEnableAutoSendActive = true; 	//���������ϱ�����
		pGprs->wActiveDropSec = 60; 	 //����������ģʽ���Զ�����ʱ��,��λ��

   		ReadItemEx(BN24, PN0, 0x4108, bBuf);
		iScale = Fmt6ToVal(bBuf, 2);
		wSvrBeatMin = iScale * pGprs->bCliBeatMin / 100;	//Fmt6 format as SNN.NN
        pGprs->bSvrBeatMin = (wSvrBeatMin > 255)? 255:wSvrBeatMin;
		return;
	}
	else //if (bType == 1)	//��������
	{
		DTRACE(DB_FAPROTO, ("LoadGprsPara:: GPRS is in persisit-mode\r\n"));

		pGprs->bOnlineMode = ONLINE_M_PERSIST;
		pGprs->fEnableAutoSendActive = false; 	//���������ϱ�����
		pGprs->wActiveDropSec = 0; 	 //����������ģʽ���Զ�����ʱ��,��λ��
	}
}

void LoadSockPara(TSocketIf* pSocket)
{
    BYTE bBuf[1];
	//����
	pSocket->fSvr = false;	//�Ƿ��Ƿ�����ģʽ
	//pSocket->fUdp = false;	//�Ƿ���UDPͨ�ŷ�ʽ

	ReadItemEx(BN0, PN0, 0x801a, bBuf);
	if (bBuf[0] == 1)
		pSocket->fUdp = true;
	else
		pSocket->fUdp = false;
    
	pSocket->fEnableFluxStat = true;	//�Ƿ�������������,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��	
	pSocket->bCnType = CN_TYPE_GPRS;
	pSocket->fBakIP = false;
	pSocket->wIPUseCnt = 0;
	//pSocket->wConnectFailCnt = 0;
	pSocket->wConnectNum = 2;
}

bool GetMasterIp(TMasterIp* pMasterIp)
{
	BYTE bBuf[32];

	memset(pMasterIp, 0, sizeof(TMasterIp));
	ReadItemEx(BN0, PN0, 0x801a, bBuf);
	if (bBuf[0] == 1)
		pMasterIp->fUdp = true;
	else
		pMasterIp->fUdp = false;

	//ReadItemEx(BN0, PN0, 0x003f, bBuf);
	ReadItemEx(BN0, PN0, 0x8010, bBuf);

#ifdef SYS_WIN
    bBuf[2] = 1;	//IP��ַ
	bBuf[3] = 0;
	bBuf[4] = 0;
	bBuf[5] = 127;

	WordToByte(9200, bBuf);	//�˿ں�
#endif

	pMasterIp->dwRemoteIP = ((DWORD )bBuf[FAP_IP_OFFSET]) + ((DWORD )bBuf[FAP_IP_OFFSET+1]<<8) + 
							((DWORD )bBuf[FAP_IP_OFFSET+2]<<16) + (bBuf[FAP_IP_OFFSET+3]<<24);

	pMasterIp->wRemotePort = ByteToWord(&bBuf[FAP_FRONT_PORT_OFFSET]);
	//pPara->dwLocalIP = 0; //INADDR_ANY;
	//pPara->wLocalPort = 8000;

	ReadItemEx(BN0, PN0, 0x8011, bBuf);
	if (bBuf[8] == 0x02 || bBuf[8] == 0x04)//����ͨ��
	{
		pMasterIp->dwBakIP = ((DWORD )bBuf[FAP_IP_OFFSET]) + ((DWORD )bBuf[FAP_IP_OFFSET+1]<<8) + 
			((DWORD )bBuf[FAP_IP_OFFSET+2]<<16) + (bBuf[FAP_IP_OFFSET+3]<<24);

		pMasterIp->wBakPort = ByteToWord(&bBuf[FAP_FRONT_PORT_OFFSET]);
	}
	else
	{
		pMasterIp->dwBakIP = 0;
		pMasterIp->wBakPort = 0;
	}

	if (pMasterIp->dwBakIP == 0 || bBuf[8] == 0xff)
	{
		ReadItemEx(BN0, PN0, 0x8012, bBuf);
		if (bBuf[8] == 0x02 || bBuf[8] == 0x04)//����ͨ��
		{
			pMasterIp->dwBakIP = ((DWORD )bBuf[FAP_IP_OFFSET]) + ((DWORD )bBuf[FAP_IP_OFFSET+1]<<8) + 
				((DWORD )bBuf[FAP_IP_OFFSET+2]<<16) + (bBuf[FAP_IP_OFFSET+3]<<24);

			pMasterIp->wBakPort = ByteToWord(&bBuf[FAP_FRONT_PORT_OFFSET]);
		}
		else
		{
			pMasterIp->dwBakIP = 0;
			pMasterIp->wBakPort = 0;
		}
	}

//	pPara->dwBakIP = 0;
//	pPara->wBakPort = 0;
	/*DTRACE(DB_FAPROTO, ("SocketLoadPara:remote ip=%d.%d.%d.%d port=%d, local port=%d.\n",
						(pPara->dwRemoteIP>>24)&0xff, (pPara->dwRemoteIP>>16)&0xff, (pPara->dwRemoteIP>>8)&0xff, pPara->dwRemoteIP&0xff, 
						pPara->wRemotePort, pPara->wLocalPort)); */
	return true;
}

//������ȡ���ն��������������
bool GetSvrPara(TSvrPara* pSvrPara)
{
	
    
	pSvrPara->fUdp = false;
	
	pSvrPara->wLocalPort = 5100;

    DTRACE(DB_FAPROTO, ("GetSvrPara:: Svr wLocalPort=%d.\r\n", pSvrPara->wLocalPort));
//	pSvrPara->wLocalPort = 8000;		//�󶨵ı��ض˿ں�
	return true;
}

bool GetApn(char* pszApn)
{
	BYTE bBuf[16];
	ReadItemEx(BN0, PN0, 0x8014, bBuf);	
	if(IsAllAByte(bBuf,0x00,16))
	{
		strcpy(pszApn, "CMNET");
	}
	else
	{
		memcpy(pszApn,bBuf,16);
	}
	pszApn[16] = 0;
	return true;
}

bool GetUserAndPsw(char *psUser, char *psPsw)
{
	BYTE bBuf[32];
	ReadItemEx(BN0, PN0, 0x8015, bBuf);
	memcpy(psUser,bBuf,32);

	psUser[32] = 0;
	ReadItemEx(BN0, PN0, 0x8016, bBuf);
	memcpy(psPsw,bBuf,32);
	psPsw[32] = 0;
		
	if (psUser[0]=='\0' && psPsw[0]=='\0')
	{
		strcpy(psUser, "CARD");
		strcpy(psPsw, "CARD");
	}   
    return true;
}

//����232ά���ڳ�ʼ��
void LoadLocal232Para(TCommIf* pCommIf)
{
	pCommIf->wPort = COMM_TEST;
	pCommIf->dwBaudRate = CBR_9600;
	pCommIf->bByteSize = 8;
	pCommIf->bStopBits = ONESTOPBIT;
	pCommIf->bParity = EVENPARITY;
	pCommIf->fDebug = true;
}

//���غ���ά���ڳ�ʼ��
void LoadLocalIrPara(TCommIf* pCommIf)
{
	pCommIf->wPort = COMM_LOCAL;
	pCommIf->dwBaudRate = CBR_1200;
	pCommIf->bByteSize = 8;
	pCommIf->bStopBits = ONESTOPBIT;
	pCommIf->bParity = EVENPARITY;
	pCommIf->fDebug = false;
}
