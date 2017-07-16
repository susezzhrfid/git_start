/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�GprsIf.c
 * ժ    Ҫ�����ļ�ʵ����GPRSͨ�Žӿ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#include <stdio.h>
#include "ProIfCfg.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "sysarch.h" 
//#include "sysapi.h"
#include "GprsIf.h"
//#include "ProHook.h"
#include "ProIfConst.h"
#include "Info.h"
#include "ProStruct.h"
#include "ProPara.h"
#include "SysDebug.h"
//#include "Drivers.h"
#include "FaAPI.h"
#include "FaCfg.h"
#include "LibDbAPI.h"
#include "ping.h"
#include "ctrl.h"

////////////////////////////////////////////////////////////////////////////////////////////
//GprsIf˽�к궨��
/*
#define CN_MODE_SOCKET      0	//����TCP/IP��ͨ��ģʽ
#define CN_MODE_SMS      	1	//����
#define CN_MODE_EMBED     	2	//ģ��Ƕ��ʽЭ��ջ
#define CN_MODE_COMM     	3	//����ͨ��ģʽ
#define CN_MODE_CMD     	4	//����ģʽ*/

//�ͻ���״̬��
#define CLI_STATE_IDLE		0   //����:�������ϱ�����->����״̬
#define CLI_STATE_CONNECT 	1	//����:���ӳɹ�->��½״̬
#define CLI_STATE_LOGIN		2   //��¼:��¼�ɹ�->����״̬
#define CLI_STATE_TRANS		3   //����:������ͨ�Ź涨ʱ��->����״̬

//������״̬��
#define SVR_STATE_LISTEN 	0	//����:���ü����˿�->����״̬
#define SVR_STATE_ACCEPT	1   //����:���յ��µ�����->����״̬
#define SVR_STATE_TRANS		2   //����:�Է��Ͽ�����|�����������->����״̬

#define RST_INTERV 			20	//��λ�������λ��
#define CONNECT_INTERV 		5   //5//60	//���Ӽ������λ��
#define LOGIN_INTERV 		5	//��½�������λ��
#define LISTEN_INTERV		30	//�����������λ��
#define SOCK_CHK_INTERV		200	//SOCKET���������λ��
#define SVR_CLOSE_TO	    120	//���ģʽ����������ͨ�ŵ���ʱ��

#define RST_NUM				5	//һ��ѭ��������ĸ�λģ�����
#define BEAT_TEST_TIMES 	2	//�������Դ���,   Ϊ0,��ʾ���Զ�����,ֻ���ڷ�����
#define BEAT_TEST_TO		30	//������ʱʱ��,��λ��

////////////////////////////////////////////////////////////////////////////////////////////
//GprsIf˽�г�Ա����
static BYTE m_bFailCnt = 0;			//��¼�ͻ��˸��׶ε�����ʧ�ܼ���
static BYTE m_bSvrFailCnt = 0;		//��¼���������׶ε�����ʧ�ܼ���
static BYTE m_bRstToDormanCnt = 0;	//��λ�����ߵļ���:�κ������ʧ�ܻᵼ��ģ�鸴λ��
									//��ģ�鸴λ��������pGprs->bRstToDormanNum�󣬽�������״̬
static BYTE m_bConnectFailCnt = 0;	//����ʧ�ܼ���
static BYTE m_bLoginFailCnt = 0;		//��½ʧ�ܼ���									
static DWORD m_dwDormanClick = 0;	//����Dorman��ʱ��
static DWORD m_dwSvrChkClick = 0;	//�ϴη�����socket��Ч�Եļ��ʱ��
static DWORD m_dwCliChkClick = 0;	//�ϴοͻ���socket��Ч�Եļ��ʱ��
static DWORD m_dwSvrRxClick = 0;	//�ϴη�����socket����ʱ��
static DWORD m_dwBeatClick = 0;
static DWORD m_dwDebugClick = 0;

////////////////////////////////////////////////////////////////////////////////////////////
//GprsIf˽�к�������
bool GprsIfReset(struct TProIf* pProIf);
bool GprsIfDisConnectCli(struct TProIf* pProIf);
bool GprsIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen);
int GprsIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize);
bool GprsIfDorman(struct TProIf* pProIf);
void GprsIfTrans(struct TProIf* pProIf);
void GprsIfOnResetOK(struct TProIf* pProIf);
void GprsIfOnResetFail(struct TProIf* pProIf);
void GprsIfOnRcvFrm(struct TProIf* pProIf);
void GprsIfDoIfRelated(struct TProIf* pProIf);
void GprsIfLoadUnrstPara(struct TProIf* pProIf);
//void SocketIfTrans(struct TProIf* pProIf);
void GprsIfDoIfShift(struct TProIf** pProIf);

////////////////////////////////////////////////////////////////////////////////////////////
//GprsIfʵ��
bool GprsIfReInit(struct TProIf* pProIf)
{    
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;    
    struct TModem* pModem = pGprs->pModem;		//GPRSģ��
    BYTE bCnType;
    BYTE bNetContTye = 0;
    
    ReadItemEx(BN2, PN0, 0x2050, &bCnType);//��ȡ�ϴε���������
    //1���ȹر�֮ǰ������
    if (bCnType == CN_TYPE_ET)
    {
        SocketIfDisConnect(pGprs->pSocketIf);    //����̫����SOCKET�ص�      
	#ifndef SYS_WIN
        ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //��̫�����ӷ�ʽ
        if (bNetContTye == 1)//��PPPOE��Ҫ�ر�
        {
            //if (ClosePppoe(pGprs->ipd) >= 0)
            if (pppClose(pGprs->ipd) >= 0)
                pGprs->ipd = -1;
        }
	#endif
    }
    else //GPRS
    {
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SocketIfDisConnect(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
            pModem->pfnCloseCliSock(pModem);    //���socket��Ч���ȹر�socket        
        }
                    
        pModem->pfnClosePpp(pModem);    //���PPP�򿪵�,�ȹر�PPP          
    }
        
    ReadItemEx(BN2, PN0, 0x10d3, &bCnType); //�����µ���������
#ifndef SYS_WIN
	pGprs->bCnType = bCnType;     
#endif
	
    pGprs->pSocketIf->bCnType = pGprs->bCnType;
    
    if (pGprs->bCnType == CN_TYPE_ET)
    {        
        pGprs->bCnMode = CN_MODE_SOCKET;  //�л�����̫��Ӧ�����뵽SOCKET
        SetNetIfDefaultET(); //����Ĭ������Ϊ��̫��
        DTRACE(DB_FAPROTO, ("Shift to Ethernet!\r\n"));        
    }
    else //GPRS
    {          
        //��̫���л���GPRSʱ��Ҫ֪��ʲôЭ��ջ
        ReadItemEx(BN1, PN0, 0x2032, &bCnType);//TCP/IPЭ��⣺0ģ���Դ�,1�ն�
        if (bCnType == 0)
            pGprs->bCnMode = CN_MODE_EMBED;
        else
            pGprs->bCnMode = CN_MODE_SOCKET;
        
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SetNetIfDefaultPPP(); //����Ĭ������ΪPPP
            DTRACE(DB_FAPROTO, ("Shift to GPRS!\r\n"));
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)	
        {
        }
    }
	return true;
}

bool GprsIfInit(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

	//����ṹ��Ա��ʼ��
	ProIfInit(pProIf);

	//������
	pProIf->pszName = "GprsIf";					//�ӿ�����
	pProIf->wMaxFrmBytes = GPRS_MAX_BYTES;		//���֡����
	pProIf->bIfType = IF_GPRS;					//�ӿ�����

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	pProIf->pfnDorman = GprsIfDorman;			//����
	pProIf->pfnReset = GprsIfReset;				//��λ�ӿ�
	pProIf->pfnSend = GprsIfSend;				//���ͺ���
	pProIf->pfnReceive = GprsIfReceive;			//���պ���
	pProIf->pfnTrans = GprsIfTrans;				//����״̬����
	pProIf->pfnOnResetOK = GprsIfOnResetOK;		//�ӿڸ�λ�ɹ�ʱ�Ļص�����
	pProIf->pfnOnResetFail = GprsIfOnResetFail;	//�ӿڸ�λʧ��ʱ�Ļص�����
	pProIf->pfnOnRcvFrm = GprsIfOnRcvFrm;
	pProIf->pfnDoIfRelated = GprsIfDoIfRelated;	//�ӿ�������⴦����
   	pProIf->LoadUnrstPara = GprsIfLoadUnrstPara;	//�ӿ�������⴦����

	//����������ʼ��1
	pProIf->bState = IF_STATE_RST;
    
    if (pGprs->bCnMode == CN_MODE_SOCKET)
	{
        SocketIfInit(pGprs->pSocketIf);
    }

	return true;
}

void GprsOnLoginOk(struct TProIf* pProIf)
{
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
    //BYTE bConnectNew = 1;

    //WriteItemEx(BN5, 0, 0x5002, &bConnectNew);
    m_bLoginFailCnt = 0;
    
#ifndef SYS_WIN
    SetLedCtrlMode(LED_ONLINE, LED_MODE_ON);   //����
	SetLedCtrlMode(LED_RUN, LED_MODE_TOGGLE);//���е���˸  
#endif    
    SocketIfOnLoginOK(pGprs->pSocketIf);
}

void OnGprsReset()
{
    //BYTE bConnectNew = 0;
    //WriteItemEx(BN5, 0, 0x5002, &bConnectNew);
    
#ifndef SYS_WIN
    SetLedCtrlMode(LED_ONLINE, LED_MODE_OFF);   //�ص�
	SetLedCtrlMode(LED_RUN, LED_MODE_TOGGLE);//���еƳ��� 
#endif
}

//���ͺ���
bool GprsIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen)
{
    int iRet = 0;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRSģ��
    
    if (pGprs->bCnMode == CN_MODE_SOCKET) 
    {
        return SocketIfSend(pGprs->pSocketIf, pbTxBuf, wLen);        
    }
    else if (pGprs->bCnMode == CN_MODE_EMBED) 
    {
#ifdef EN_PROIF_SVR		//֧�ַ�����
	    if (pGprs->fSvrTrans)
		    iRet = pModem->pfnSvrSend(pModem, pbTxBuf, wLen);
    	else
#endif //EN_PROIF_SVR
        {
	    	iRet = pModem->pfnCliSend(pModem, pbTxBuf, wLen);    
            if (iRet < 0) //��·������Ҫ��λ
            {                              
                DTRACE(DB_FAPROTO, ("GprsIfSend: send fail.\r\n"));
            }
        }
        if (iRet > 0)
        {
            DoLedBurst(LED_REMOTE_TX);
            if (pGprs->pSocketIf->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
    			AddFlux(iRet);
        }
    }
    else if (pGprs->bCnMode == CN_MODE_SMS)//��ʱ��֧��
    {        
    }
    else
    {        
    }
    
    if (iRet == wLen)
        return true;
    return false;        
}

//���պ���
int GprsIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize)	
{
    int iLen = 0;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRSģ��
    
    if (pGprs->bCnMode == CN_MODE_SOCKET) 
        return SocketIfReceive(pGprs->pSocketIf, pbRxBuf, wBufSize);
    else if (pGprs->bCnMode == CN_MODE_EMBED) 
    {
#ifdef EN_PROIF_SVR		//֧�ַ�����
    	if (pGprs->fSvrTrans)
	    	iLen = pModem->pfnSvrReceive(pModem, pbRxBuf, wBufSize);
    	else
#endif //EN_PROIF_SVR
	    	iLen = pModem->pfnCliReceive(pModem, pbRxBuf, wBufSize);
        if (iLen > 0)
        {
            DoLedBurst(LED_REMOTE_RX);
            if (pGprs->pSocketIf->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
		        AddFlux((DWORD )iLen);
        }
    }
    else if (pGprs->bCnMode == CN_MODE_SMS)//��ʱ��֧��
    {
        Sleep(1000);  
    }
    else
    {
        Sleep(1000);    
    }
    
    return iLen;
}

DWORD GetSvrRxClick()
{
    return m_dwSvrRxClick;
}

void UpdateSvrRxClick()
{
    m_dwSvrRxClick = GetClick();
}

void GprsIfOnRcvFrm(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	if (pGprs->fSvrTrans)
        UpdateSvrRxClick();
	else
		pProIf->dwRxClick = GetClick();
    if (pGprs->bCnType == CN_TYPE_ET)  //��̫��������PING���
        UpdPingTime();
}

//��������λ�ӿ�
bool GprsIfDorman(struct TProIf* pProIf)
{
	DWORD dwClick = GetClick();
    static DWORD dwLastClick = 0;
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

	if (m_dwDormanClick != 0)
	{
		if (dwClick-m_dwDormanClick < pGprs->wDormanInterv)
		{
            if (dwLastClick != dwClick)
            {
                dwLastClick = dwClick;
    			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: remain %ld\n", pGprs->wDormanInterv + m_dwDormanClick - dwClick));
            }
		}
		else
		{
			m_dwDormanClick = 0;
			//m_dwDormanInterv = 0;	//��ʱ�趨�����߼������λ��
			pProIf->bState = IF_STATE_RST;
			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: wake up\n"));
		}		
	}
	else //m_dwDormanClick==0 
	{	 //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
		 //Ҫ�˳�����״̬,��Ҫ����DoIfRelated()�и��ݽӿ���ص�
		 //�����m_wState״̬�ı�,����Ӳ�����ʱ���л�������ʱ��
		 //�ӿڵĲ�������
		if (dwClick-m_dwDebugClick > IF_DEBUG_INTERV)
		{
			m_dwDebugClick = dwClick;
			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: if(%s)in idle mode\n", pProIf->pszName));
		}
	}

	return true;
}

void GprsIfEnterDorman(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

	SocketIfEnterDorman(pGprs->pSocketIf);
	DTRACE(DB_FAPROTO, ("GprsIfEnterDorman: for %dS \r\n", pGprs->wDormanInterv));
	//DisConnect();
	pProIf->bState = IF_STATE_DORMAN; //����
	m_dwDormanClick = GetClick();	 //�������ߵĿ�ʼʱ��
    if (m_dwDormanClick == 0)
    {
        Sleep(1000); //��ʱ1S��ȡ
        m_dwDormanClick = GetClick();
    }
	m_bRstToDormanCnt = 0;	//��λ�����ߵļ���:�κ������ʧ�ܻᵼ��ģ�鸴λ��
	//��ģ�鸴λ��������pGprs->bRstToDormanNum�󣬽�������״̬
	//GprsIfReset(pProIf);    ģ��һ��λ���ָı���pProIf->bState
}    		


//������������ֻ���̺߳����е��ã�������������̸ı�ӿ�״̬
void GprsIfOnResetOK(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

	pGprs->bCliState = CLI_STATE_CONNECT;
	m_bFailCnt = 0;
	
	pGprs->bSvrState = SVR_STATE_LISTEN;	//if (pGprs->bOnlineMode == ONLINE_M_MIX) 	//�ͻ���/���������ģʽ)
	m_bSvrFailCnt = 0;
	
	m_bRstToDormanCnt = 0;   //�ܸ�λ�ɹ������ǲ���ǰ�û���
}

//������������ֻ���̺߳����е��ã�������������̸ı�ӿ�״̬
void GprsIfOnResetFail(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

	m_bFailCnt++;

	if (m_bFailCnt >= RST_NUM)	//��ǰ�׶ε�����ʧ�ܼ���������ֵ��Ҫ������λģ���������
	{				//һ��ѭ��������ĸ�λģ�����	
		m_bFailCnt = 0;
        m_bRstToDormanCnt++;
		if (m_bRstToDormanCnt >= pGprs->bRstToDormanNum)	//��λ�����ߵļ���:ģ��������λ�Ĵ����൱��RST_NUM*pGprs->bRstToDormanNum
		{	
			//�κ������ʧ�ܣ�
			//��ģ�鸴λ����û�ﵽpGprs->bRstToDormanNumʱ����ȥ��λģ�飻
			//��ģ�鸴λ��������pGprs->bRstToDormanNum�󣬽�������״̬;
			
			m_bRstToDormanCnt = 0;
			if (pGprs->wDormanInterv != 0)
			{
				DTRACE(DB_FAPROTO, ("GprsIfOnResetFail: enter dorman\r\n"));
				GprsIfEnterDorman(pProIf);
			}
			else
			{
				Sleep(RST_INTERV*1000); //�ӿڵ����Ӽ��,��λ��
				pProIf->bState = IF_STATE_RST;
			}
		}
		else
		{
			Sleep(RST_INTERV*1000); //�ӿڵ����Ӽ��,��λ��
			pProIf->bState = IF_STATE_RST;
		}
	}
	else
	{
		Sleep(RST_INTERV*1000); //�ӿڵ����Ӽ��,��λ��
		pProIf->bState = IF_STATE_RST;
	}
}

//��������λ�ӿ�
bool GprsIfReset(struct TProIf* pProIf)
{
	/*pProIf->bState = IF_STATE_TRANS;
#ifdef SYS_WIN
	return IF_RST_OK;
#endif //SYS_WIN

	WORD wTryCnt = 0;
	
again:
	ClosePpp();	//���PPP�򿪵�,�ȹر�PPP
	
	if (wCnMode==GPRS_MODE_IDLE || wCnMode==GPRS_MODE_PWROFF)	//���ܱ�Ľӿ�Ҫʹ�ô���,�����ȹر�,�ó�����
	{		
		if (m_pWorkerPara->fEmbedProtocol)//ģ��Э��ջ�Ļ��Ͳ�����ĵط����ô�����
		{
			if (wCnMode == GPRS_MODE_PWROFF)
			{
				m_pModem->PowerOff();
				m_fModemPwrOn = false;
				//Sleep(5000);
			}
			return IF_RST_OK;	
		}
			
		if (m_Comm.IsOpen())
		{	
			DTRACE(DB_FAPROTO, ("CGprsWorker::ResetGPRS: close comm in idle\n"));
			m_Comm.Close();
		}

		if (wCnMode == GPRS_MODE_PWROFF)
		{
			m_pModem->PowerOff();
			m_fModemPwrOn = false;
			//Sleep(5000);
		}
		
		return IF_RST_OK;
	}

	if (!m_fModemPwrOn)
	{
		m_pModem->PowerOn();
		m_fModemPwrOn = true;
		//Sleep(5000);
	}
		
	if (!m_Comm.IsOpen())
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetGPRS: re open comm for at\n"));
		m_Comm.Open();	
	}*/
	WORD i;
//	WORD wTryCnt = 0;
	BYTE bNetContTye = 0;
    BYTE bBuf[64];
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRSģ��
    static bool first = true;

//again:
//	SetLocalAddr(0);	//����IP��ʼ��Ϊ0
    
    OnGprsReset();
    if (!first)   //�״��ϵ粻����SOCKET
    {
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SocketIfDisConnect(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
            pModem->pfnCloseCliSock(pModem);    //���socket��Ч���ȹر�socket        
        }
        first = false;
    }
        
    if (pGprs->bCnType == CN_TYPE_ET)//��̫��
    {
	#ifndef SYS_WIN
		if (!pGprs->fCnSw)
        {            
            DTRACE(DB_FAPROTO, ("Reset mac and phy.\r\n"));
            SetInfo(INFO_MAC_RST);//��λPHY
            i = 30;
            while(i--) //
            {
                Sleep(1000);
                if (ETLinkUp())
                    break;
            }
        }
	#endif
        pGprs->bSignStrength = 0;    //�ص��źŵ�      
		pGprs->fCnSw = false;
        pProIf->bState = IF_STATE_TRANS;       
        
	#ifndef SYS_WIN
	    ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //��̫�����ӷ�ʽ
        if (bNetContTye == 1) //1��pppoe����
        {
            //ClosePppoe(pGprs->ipd);   //�ȹر�֮ǰ�򿪵�
            pppClose(pGprs->ipd);
            ReadItemEx(BN0, PN0, 0x010f, bBuf);
            pppSetAuth(PPPAUTHTYPE_ANY, &bBuf[0], &bBuf[32]);

            pGprs->ipd = OpenPppoe();
            if (pGprs->ipd < 0)
            {
                DTRACE(DB_FAPROTO, ("Open pppoe is failed.\r\n"));
                return false;
            }
        }
	#endif

        DTRACE(DB_FAPROTO, ("EthernetIf enter trans state.\r\n"));
        return true;
    }
    else if (pGprs->bCnType == CN_TYPE_GPRS)//GPRS
    {
        pGprs->fCnSw = false;        
        pModem->pfnClosePpp(pModem);    //���PPP�򿪵�,�ȹر�PPP
	    if (pModem->pfnResetModem(pModem) != MODEM_NO_ERROR)
    	{
    		pGprs->cLastErr = GPRS_ERR_RST;
    		return false;
    	}

    	//if (m_wState == IF_STATE_DORMAN)	//�Ѿ���������ģʽ���ʹ˷���
    	//	return IF_RST_OK;
    /*
    	for (i=0; i<10; i++)   //��Ӧ����������³�ǿ�����û�忨������������Sleep 20S
    	{
    		pGprs->bSignStrength = pModem->pfnUpdateSignStrength(pModem);
    
    		if (pGprs->bSignStrength!=0 && pGprs->bSignStrength!=99 && 
    			pGprs->bSignStrength!=100 && pGprs->bSignStrength!=199)
    		{
    			break;
    		}
    		Sleep(2000);
    	}*/
        
    //    SignLedCtrl(pGprs->bSignStrength);    //debug
        
    	if (pModem->pfnInitAPN(pModem) != MODEM_NO_ERROR)
    	{
    		pGprs->cLastErr = GPRS_ERR_REG;
    		return false;
    	}
    
        pGprs->bSignStrength = pModem->pfnUpdateSignStrength(pModem);  //����bSignStrength���ڵ��
    	if (pGprs->bSignStrength<=0 || pGprs->bSignStrength>31)
    	{
    		for (i=0; i<5; i++)
    		{
    			if (pGprs->bSignStrength!=0 && pGprs->bSignStrength!=99 && 
    				pGprs->bSignStrength!=100 && pGprs->bSignStrength!=199)
    			{
                    //SignLedCtrl(pGprs->bSignStrength);
    				break;
    			}
    
    			//GprsShedOthers();	//���ȴ���ͬһ�̵߳���������
                pGprs->bSignStrength = pModem->pfnUpdateSignStrength(pModem);
    			Sleep(2000);
    		}
    	}        
        
        WriteItemEx(BN2, PN0, 0x1058, &pGprs->bSignStrength);
        
        if ((pGprs->bSignStrength==0) || (pGprs->bSignStrength>31)) //0���ڹ��ź�ǿ�ȵ�
            pGprs->bSignStrength = 1;               
    
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            if (ATCommand(pModem, "ATDT*99***1#\r\n", "CONNECT", NULL, NULL, 5) <= 0)
            {
                ATCommand(pModem, "ATDT*99***1#\r\n", "CONNECT", NULL, NULL, 5);
            }
        }
    	
    	//m_dwSignClick = m_dwSmsOverflowClick = GetClick();
    }

    //if (pGprs->bCnMode == CN_MODE_SOCKET)
	//{
		if(pModem->pfnOpenPpp(pModem))
		{
			pProIf->bState = IF_STATE_TRANS;
			return true;
		}
		else
		{
			pGprs->cLastErr = GPRS_ERR_PPP;
			return false;
		}
	//}
	/*else if (pGprs->bCnMode == CN_MODE_SMS) 	 //����
	{
		if (pModem->ResetSMS() == true)
		{
			pProIf->bState = IF_STATE_TRANS;
			return true;
		}
		else
			return false;
	}*/
	//else
	//{
		//return false;
	//}
/*	
	wTryCnt++;
	if (wTryCnt >= 2)
		return false;
		 
	goto again;

	return false;*/
}

//����:�ӿڱ���̽��
void GprsIfKeepAlive(struct TProIf* pProIf)
{	
	DWORD dwClick, dwBrokenTime;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��
	DWORD dwBeatMinutes = pGprs->bCliBeatMin;		//�ͻ����������,��λ����
	if (dwBeatMinutes == 0)
		return;
	
	dwClick = GetClick();
    dwBrokenTime = dwBeatMinutes*60 +	BEAT_TEST_TO*BEAT_TEST_TIMES;
	if (dwClick-pProIf->dwRxClick > dwBrokenTime)
	{	
		GprsIfDisConnectCli(pProIf); //���³�ʼ��
		DTRACE(DB_FAPROTO, ("GprsIfKeepAlive: DisConnect at click %ld\r\n", dwClick));
	}
	else if (dwClick-pProIf->dwRxClick > dwBeatMinutes*60)//40 20�뻹û�յ���һ֡��������������
	{	//�տ�ʼʱdwBeatClickΪ0�������Ϸ�
		if (dwClick-m_dwBeatClick > BEAT_TEST_TO)
		{							//������ʱʱ��,��λ��
			pPro->pfnBeat(pPro);//���ݶ�������ȥ�ˡ�
			m_dwBeatClick = dwClick;
			DTRACE(DB_FAPROTO, ("GprsIfKeepAlive: heart beat test at click %ld\r\n", dwClick));
		}
	}
}

//����:�ڽӿ�������תΪ�Ͽ���ʱ����ã������������Ͽ����Ǳ����Ͽ�
bool GprsIfDisConnectCli(struct TProIf* pProIf)
{
    bool fRet = false;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRSģ��
    
    if (pGprs->bCnMode == CN_MODE_SOCKET)
    {
        fRet = SocketIfDisConnect(pGprs->pSocketIf);
    }
    else if (pGprs->bCnMode == CN_MODE_EMBED)
    {
        fRet = pModem->pfnCloseCliSock(pModem);		//�ر�Ǳ�ڵĿͻ�������
    }
    else
    {
    }

	if (pGprs->bCliState > CLI_STATE_CONNECT)
		pGprs->bCliState = CLI_STATE_CONNECT;

	//�������
	//m_dwRxClick = 0;	//������m_dwRxClick,��Ȼ��������m_pIfPara->dwNoRxRstAppInterv���,
						//ֻҪ��һ��û�������ɹ��ͻ������ն˸�λ
	m_dwBeatClick = 0;

	//�������
	//m_wConnectFailCnt = 0; //�Ͽ����Ӳ���Ҫ��λʧ�ܼ���,��Ϊ����socket
							 //��ʽ��,ÿ������ǰ�������س��ԶϿ�֮ǰ��
							 //����,�����ʹ����ʧ�ܼ���ʧЧ,������ֻ��
							 //�ڽӿڸ�λ�ɹ������ۼƵ����Դ����������
	return fRet;
}

//����:����Ƿ���Ҫ����,
//����:�����Ҫ�����򷵻�true,���򷵻�false
//��ע:�������������Ҫ����:1.���ռ�����Ϣ(����/����);2.�����ϱ�
bool GprsIfCheckActivation(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��

	if (GetInfo(INFO_ACTIVE)) //�յ��˶��ż���֡
	{
		DTRACE(DB_FAPROTO, ("GprsIfCheckActivation: switch to gprs mode due to rx activate info\n"));
		return true;
	}
	
	if (pGprs->fEnableAutoSendActive) //���������ϱ�����
	{
		if (pPro->pfnIsNeedAutoSend(pPro)) //��Ҫ�����ϱ�
		{
			DTRACE(DB_FAPROTO, ("GprsIfCheckActivation: switch to gprs mode due to auto send\n"));
			return true;
		}
	}	

	return false;
}

void GprsIfOnConnectFail(struct TProIf* pProIf)
{
	BYTE bConnectNum;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	TMasterIp tMasterIp;
	GetMasterIp(&tMasterIp);

	SocketIfOnConnectFail(pGprs->pSocketIf);
	m_bFailCnt++;
	bConnectNum = pGprs->bConnectNum;
	if (tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)	//������վIP�Ͷ˿�
		bConnectNum *= 2;

	if (m_bFailCnt >= bConnectNum)	//��ǰ�׶ε�����ʧ�ܼ���������ֵ��Ҫ������λģ���������
	{
		m_bFailCnt = 0;
        m_bConnectFailCnt++;
		if (m_bConnectFailCnt >= pGprs->bRstToDormanNum)	//��λ�����ߵļ���:
		{	
			//�κ������ʧ�ܣ�
			//��ģ�鸴λ����û�ﵽpGprs->bRstToDormanNumʱ����ȥ��λģ�飻
			//��ģ�鸴λ��������pGprs->bRstToDormanNum�󣬽�������״̬;
			
			m_bConnectFailCnt = 0;
			if (pGprs->wDormanInterv != 0)
			{
				DTRACE(DB_FAPROTO, ("GprsIfOnConnectFail: enter dorman\r\n"));
				GprsIfEnterDorman(pProIf);
			}
			else
			{
				//Sleep(CONNECT_INTERV*1000); //�ӿڵ����Ӽ��,��λ��
				pProIf->bState = IF_STATE_RST;
			}
		}
		else
		{
			//Sleep(CONNECT_INTERV*1000); //�ӿڵ����Ӽ��,��λ��
			pProIf->bState = IF_STATE_RST;
		}
	}
	else
	{
		Sleep(CONNECT_INTERV*1000); //�ӿڵ����Ӽ��,��λ��
	}
}

//����:��Э���½ʧ��ʱ����,������ٴ�ʧ�ܺ�Ͽ�����,����3�ε�½ʧ�ܻ�����CONNET�л�����������Ҫ��λģ��
void GprsIfOnLoginFail(struct TProIf* pProIf)
{	
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

    OnGprsReset();
    //SocketIfOnConnectFail(pGprs->pSocketIf);
	m_bFailCnt++;	
	if (m_bFailCnt >= pGprs->bLoginNum)	//��ǰ�׶ε�����ʧ�ܼ���������ֵ��Ҫ������λģ���������
	{					
		m_bFailCnt = 0; 
        pGprs->pSocketIf->wIPUseCnt = pGprs->pSocketIf->wConnectNum;  //��½���ʧ���´Σ���ʶ��ǰIPʹ�ô������ˣ�ֱ�ӻ�һ��IP���ӡ�---liyan
        m_bLoginFailCnt++;
		if (m_bLoginFailCnt > pGprs->bRstToDormanNum)	//��λ�����ߵļ���
		{	
			//�κ������ʧ�ܣ�
			//��ģ�鸴λ����û�ﵽpGprs->bRstToDormanNumʱ����ȥ��λģ�飻
			//��ģ�鸴λ��������pGprs->bRstToDormanNum�󣬽�������״̬;
		
			m_bLoginFailCnt = 0;
			if (pGprs->wDormanInterv != 0)
			{
				DTRACE(DB_FAPROTO, ("GprsIfOnConnectFail: enter dorman\r\n"));
				GprsIfEnterDorman(pProIf);
			}
			else
			{
				pProIf->bState = IF_STATE_RST;
			}
		}
		else 
		{
			//Sleep(LOGIN_INTERV*1000); //�ӿڵ����Ӽ��,��λ��
			pProIf->bState = IF_STATE_RST;
		}
	}
	else
	{					//��¼ʧ�ܵĴ�����û���Ͽ����ӵĴ���
		Sleep(LOGIN_INTERV*1000); //��¼���
	}
}

#ifdef EN_PROIF_SVR		//֧�ַ�����
void GprsIfMixModeTrans(struct TProIf* pProIf)
{
	TMasterIp tMasterIp;
	TSvrPara tSvrPara;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRSģ��
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��
	DWORD dwClick = GetClick();
	bool fRes = false;
	BYTE bConnectNum;
    BYTE bBuf[120];

	//1.�ͻ��˴���
	pGprs->fSvrTrans = false;	//������/���պ�����ʶ��ǰ���ڴ���[�ͻ���]
	switch (pGprs->bCliState)	//�ͻ���״̬��������->��½->ͨ��->�����Ͽ�->����
	{
	case CLI_STATE_IDLE:  	 	//����:�������ϱ�����->����״̬
		break;

	case CLI_STATE_CONNECT:		//����:���ӳɹ�->��½״̬
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SocketIfDisConnect(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
    		pModem->pfnCloseCliSock(pModem);		//�ر�Ǳ�ڵĿͻ�������
        }
		bConnectNum = pGprs->bConnectNum;
		GetMasterIp(&tMasterIp);
		if (m_bFailCnt < bConnectNum)
        {
            if (pGprs->bCnMode == CN_MODE_SOCKET)
            {
                fRes = SocketIfConnect(pGprs->pSocketIf); //todo:
            }
            else if (pGprs->bCnMode == CN_MODE_EMBED)
            {
    			fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwRemoteIP, tMasterIp.wRemotePort);
            }
        }
		else
        {
            if (pGprs->bCnMode == CN_MODE_SOCKET)
            {
                fRes = SocketIfConnect(pGprs->pSocketIf);//todo:�л���վ�뱸����վ
            }
            else if (pGprs->bCnMode == CN_MODE_EMBED)
            {
    			fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwBakIP, tMasterIp.wBakPort);
            }
        }

		if (fRes)
		{
			pGprs->bCliState = CLI_STATE_LOGIN;
			m_bFailCnt = 0;
			m_bConnectFailCnt = 0;	//����ʧ�ܼ���
			SocketIfOnConnectOK(pGprs->pSocketIf);
		}
		else
		{
			GprsIfOnConnectFail(pProIf);
		}
		break;

	case CLI_STATE_LOGIN:   	//��¼:��¼�ɹ�->����״̬
		if (pPro->pfnLogin(pPro))
		{
			pGprs->bCliState = CLI_STATE_TRANS;
			m_bFailCnt = 0;
            GprsOnLoginOk(pProIf);
            //SocketIfOnConnectOK(pGprs->pSocketIf); //����3�ε�½ʧ��ҲҪ�����л�
		}
		else
		{         
			GprsIfOnLoginFail(pProIf);
		}
		break;

	case CLI_STATE_TRANS:   	//����:������ͨ�Ź涨ʱ��->����״̬
		//����֡����
		if (pPro->pfnRcvFrm(pPro) < 0)//���յ���һ֡,���Ѿ�������д���
        {//���ش���socket�ر�
            GprsIfDisConnectCli(pProIf); //���³�ʼ��
        } 

		//�������ʹ���
		pPro->pfnAutoSend(pPro);

		//GprsIfKeepAlive(pProIf);  //���ģʽ��������

		//���ڲ�ѯsocket�Ƿ���Ч
		if (dwClick-pProIf->dwRxClick > SOCK_CHK_INTERV &&
			dwClick-m_dwCliChkClick > SOCK_CHK_INTERV)	//�ϴοͻ���socket��Ч�Եļ��ʱ��
		{
			m_dwCliChkClick = GetClick();
			if (!pModem->pfnChkCliStatus(pModem))	//socket�Ѿ��Ͽ�
			{
				pGprs->bCliState = CLI_STATE_IDLE;
				m_bFailCnt = 0;
			}
		}
		break;
	}

	//2.����������
	pGprs->fSvrTrans = true;	//������/���պ�����ʶ��ǰ���ڴ���[������]
	switch (pGprs->bSvrState)	//������״̬��������->��������->������ͨ��->
	{
	case SVR_STATE_LISTEN:		//����:���ü����˿�->����״̬
		GetSvrPara(&tSvrPara);
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            fRes = SocketIfListen(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
            fRes = pModem->pfnListen(pModem, tSvrPara.fUdp, tSvrPara.wLocalPort);
        }
		if (fRes)
		{
			m_bSvrFailCnt = 0;
			pGprs->bSvrState = SVR_STATE_ACCEPT;
		}
		else
		{
			m_bSvrFailCnt++;
			if (m_bSvrFailCnt >= 3)
			{					
				m_bSvrFailCnt = 0;
				GprsIfEnterDorman(pProIf);
			}
			else
			{
				Sleep(LISTEN_INTERV*1000); //�������
			}
		}
		break;

	case SVR_STATE_ACCEPT:   	//����:���յ��µ�����->����״̬
		if (pGprs->bCliState != CLI_STATE_TRANS)
        {
            if (pGprs->bCnMode == CN_MODE_SOCKET)
            {
                SocketIfReceive(pGprs->pSocketIf, bBuf, 120);
            }
            else if (pGprs->bCnMode == CN_MODE_EMBED)
            {
    			pModem->pfnSvrReceive(pModem, bBuf, 120);
            }
        }

		if (pModem->pfnIsSvrAcceptOne(pModem))	//���յ��µ�����  //todo:
        {
			pGprs->bSvrState = SVR_STATE_TRANS;
            UpdateSvrRxClick();
        }

		break;

	case SVR_STATE_TRANS:   	//����:�Է��Ͽ�����|�����������->����״̬
		//����֡����
		if (pPro->pfnRcvFrm(pPro) < 0)//���յ���һ֡,���Ѿ�������д���
        {//���ش���socket�ر�
            GprsIfDisConnectCli(pProIf); //���³�ʼ��
        } 

		//���ڼ���������socket�����Ƿ���Ȼ��Ч
		if (dwClick-GetSvrRxClick() > SOCK_CHK_INTERV &&
			dwClick-m_dwSvrChkClick > SOCK_CHK_INTERV)	//�ϴοͻ���socket��Ч�Եļ��ʱ��
		{
			m_dwSvrChkClick = GetClick();
			if (!pModem->pfnChkSvrStatus(pModem))	//����������socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
			{
				pGprs->bSvrState = SVR_STATE_LISTEN;	//�������� 
				m_bSvrFailCnt = 0;
			}
		}
		
		break;
    default:
        GprsIfDisConnectCli(pProIf); //���³�ʼ��
        break;
	}

	pGprs->fSvrTrans = false;	//������/���պ�����ʶ��ǰ���ڴ���[�ͻ���]
}
#endif //EN_PROIF_SVR		//֧�ַ�����

bool IsOnlineState(struct TProIf* pProIf)
{    
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
    return (pGprs->bCliState==CLI_STATE_TRANS);
}

void GprsIfPersistModeTrans(struct TProIf* pProIf)
{
	bool fRes;
	BYTE bConnectNum;
	TMasterIp tMasterIp;
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��
	DWORD dwClick = GetClick();

	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRSģ��

	//1.�ͻ��˴���
	pGprs->fSvrTrans = false;	//������/���պ�����ʶ��ǰ���ڴ���[�ͻ���]
	switch (pGprs->bCliState)	//�ͻ���״̬��������->��½->ͨ��->�����Ͽ�->����
	{
//	case CLI_STATE_IDLE:  	 	//����:�������ϱ�����->����״̬
//		break;

	case CLI_STATE_CONNECT:		//����:���ӳɹ�->��½״̬
        //SetLed(false, LED_ONLINE);   //�ص�
        //g_fAlertLed = false;    //�ص�
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SocketIfDisConnect(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
    		pModem->pfnCloseCliSock(pModem);		//�ر�Ǳ�ڵĿͻ�������
        }
		bConnectNum = pGprs->bConnectNum;
		GetMasterIp(&tMasterIp);
        
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {   
            fRes = SocketIfConnect(pGprs->pSocketIf);//todo:
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
			if (pGprs->pSocketIf->fBakIP	//��ǰҪʹ�ñ��õ�ַ
				&& tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)//����IP�˿���Ч
			{		//����IP�˿���Ч
				pGprs->pSocketIf->wIPUseCnt++;
		        if (pGprs->pSocketIf->wIPUseCnt > pGprs->pSocketIf->wConnectNum)
		        {
		            pGprs->pSocketIf->wIPUseCnt = 1;   //�Ѿ�ʹ��һ����
		            pGprs->pSocketIf->fBakIP = false;  //�����л�����IP
		            fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwRemoteIP, tMasterIp.wRemotePort);
		        }
		        else
		        {
					fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwBakIP, tMasterIp.wBakPort);
			    }
			}
			else
			{		
		        pGprs->pSocketIf->wIPUseCnt++;
		        if (pGprs->pSocketIf->wIPUseCnt>pGprs->pSocketIf->wConnectNum &&
		        	tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)
		        {
		            pGprs->pSocketIf->wIPUseCnt = 1; //�Ѿ�ʹ��һ����
		           	pGprs->pSocketIf->fBakIP = true; //���л�������IP
					fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwBakIP, tMasterIp.wBakPort);
		        }
		        else
		        {
		        	pGprs->pSocketIf->fBakIP = false;  //��������IP
					fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwRemoteIP, tMasterIp.wRemotePort);
		        }
			}
        }

		if (fRes)
		{
			pGprs->bCliState = CLI_STATE_LOGIN;
			m_bFailCnt = 0;
            m_bConnectFailCnt = 0;	//����ʧ�ܼ���
            pProIf->dwRxClick = GetClick();//��ֹ����֮��û��ͨ�ţ�û�и��¸�ֵ��֮���keepalive�жϿ�
            //SocketIfOnConnectOK(pGprs->pSocketIf);
		}
		else
		{
			GprsIfOnConnectFail(pProIf);
		}
		break;

	case CLI_STATE_LOGIN:   	//��¼:��¼�ɹ�->����״̬
		if (pPro->pfnLogin(pPro))
		{
			pGprs->bCliState = CLI_STATE_TRANS;
			m_bFailCnt = 0;
            GprsOnLoginOk(pProIf);
            //SocketIfOnConnectOK(pGprs->pSocketIf); //����3�ε�½ʧ��ҲҪ�����л�
		}
		else
		{
			GprsIfOnLoginFail(pProIf);
		}
		break;

	case CLI_STATE_TRANS:   	//����:������ͨ�Ź涨ʱ��->����״̬
		//����֡����
		if (pPro->pfnRcvFrm(pPro) < 0)//���յ���һ֡,���Ѿ�������д���
		{//���ش���socket�ر�
			GprsIfDisConnectCli(pProIf); //���³�ʼ��
            break;  //Ӧ�÷��أ��������ϻᷢ��һ������
		}
		else if (dwClick-pProIf->dwRxClick < pGprs->bCliBeatMin*60)
		{
			//����ʱ�������ʹ���
			pPro->pfnAutoSend(pPro);
		}

		GprsIfKeepAlive(pProIf);

		//���ڲ�ѯsocket�Ƿ���Ч
		if (dwClick-pProIf->dwRxClick > SOCK_CHK_INTERV &&
			dwClick-m_dwCliChkClick > SOCK_CHK_INTERV)	//�ϴοͻ���socket��Ч�Եļ��ʱ��
		{
			m_dwCliChkClick = GetClick();
            if (pGprs->bCnMode == CN_MODE_SOCKET)
            {   
            }
            else if (pGprs->bCnMode == CN_MODE_EMBED)
            {
                if (!pModem->pfnChkCliStatus(pModem))	//socket�Ѿ��Ͽ�
    			{
       				pGprs->bCliState = CLI_STATE_CONNECT;   //CLI_STATE_IDLE;
    				m_bFailCnt = 0;
    			}
            }			
		}
		break;
    default:
        GprsIfDisConnectCli(pProIf);  //�Ͽ����³�ʼ��
        break;
	}
	
}

void GprsIfTrans(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
//	struct TModem* pModem = pGprs->pModem;		//GPRSģ��
	
	switch (pGprs->bOnlineMode)
	{
	case ONLINE_M_PERSIST:  //��������ģʽ
		GprsIfPersistModeTrans(pProIf);
		break;

#ifdef EN_PROIF_SVR		//֧�ַ�����
	case ONLINE_M_MIX:    	//�ͻ���/���������ģʽ
		GprsIfMixModeTrans(pProIf);
		break;
#endif //EN_PROIF_SVR		//֧�ַ�����
    default:
        pGprs->bOnlineMode = ONLINE_M_PERSIST;
        break;
	}
}

//����:��һЩ�����ӿ���صķǱ�׼������,
//		���ӿ�Ҫ����������:
//		1.ģʽ�л�,������������߷�ʽ��,GPRS��SMS����л�
void GprsIfDoIfRelated(struct TProIf* pProIf)
{
	//����������ģʽ��,Ҫʵ�ֵ��л���:
	//GPRSһ��ʱ��û��ͨ��,�л���SMS��ʽ
	//SMS�յ����Ż��ѻ�绰����,�л���GPRS��ʽ
	
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRSģ��
	DWORD dwClick = GetClick();
    
    if (pGprs->dwNoRxRstAppInterv != 0) //�޽��ո�λ�ն˼��,��λ��,0��ʾ����λ
	{
		if (dwClick-pProIf->dwRxClick > pGprs->dwNoRxRstAppInterv)
		{
			DTRACE(DB_FAPROTO, ("DoIfRelated: GPRS/ET no rx reset app!\n"));
			SetInfo(INFO_APP_RST);	//CPU��λ INFO_APP_RST
			return;
		}
	}

#ifdef EN_PROIF_SVR		//֧�ַ�����
	if (pGprs->bOnlineMode == ONLINE_M_MIX)
	{
		if (dwClick-GetSvrRxClick() > (DWORD )pGprs->bSvrBeatMin*60)	//�������ڹ涨ʱ����û��ͨ�ţ���ͻ������µ�½�����������¼���
		{
            DTRACE(DB_FAPROTO, ("GprsIfDoIfRelated: Disconnect svr due to rx timeout for %ld secs!\n", pGprs->bSvrBeatMin));
			UpdateSvrRxClick();
            
            if (pGprs->bCnMode == CN_MODE_SOCKET)
            {   
                SocketIfDisConnect(pGprs->pSocketIf);
                SocketIfCloseListen(pGprs->pSocketIf);
            }
            else if (pGprs->bCnMode == CN_MODE_EMBED)
            {
    			pModem->pfnCloseSvrSock(pModem);		//�رշ���������
    			pModem->pfnCloseListen(pModem);		//�رռ���
            }

			if (pProIf->bState >= IF_STATE_TRANS)	//ĿǰPPP���Ǵ�������״̬��Ϊ�˽�ʡ������
			{										//��ֱ�ӳ���socket���ӣ����еĻ��ڸ�λ�ӿ�
				pGprs->bSvrState = SVR_STATE_LISTEN;	//���������¼���
				m_bSvrFailCnt = 0;

				pGprs->bCliState = CLI_STATE_CONNECT;	//�ͻ����������ӣ�����ͻ�������ʧ�ܣ���ȥ���²���
				m_bFailCnt = 0;

				m_bRstToDormanCnt = 0;	//�帴λ�����ߵļ�������֤������ʧ�ܵ�����£��л��Ḵλ�ӿ�����
			}
			else
			{
				pProIf->bState = IF_STATE_RST;
				m_bFailCnt = 0;
				m_bSvrFailCnt = 0;
			}
		}
		else if (GprsIfCheckActivation(pProIf))	//�������ϱ�����
		{
            DTRACE(DB_FAPROTO, ("GprsIfDoIfRelated: Activation due to need auto send!\n"));
			if (pProIf->bState < IF_STATE_RST)	//ĿǰPPP���Ǵ�������״̬��Ϊ�˽�ʡ����
			{
				pProIf->bState = IF_STATE_RST;
				m_bFailCnt = 0;
				m_bSvrFailCnt = 0;
			}
			else if (pProIf->bState>=IF_STATE_TRANS && //ĿǰPPP���Ǵ�������״̬������ֱ��socket����
					 pGprs->bCliState<CLI_STATE_CONNECT) //���ڿ�����Ҫ�������ӣ�����״̬����ԭ���Ĳ�������
			{
				pGprs->bCliState = CLI_STATE_CONNECT;
				m_bFailCnt = 0;
			}
		}
		else if (pProIf->bState==IF_STATE_TRANS)    //ĿǰPPP���Ǵ�������״̬
		{
		  	if (IsFluxOver())
			{
			  	if (pGprs->dwFluxOverClick == 0)
				{
					pGprs->dwFluxOverClick = GetClick();	//�����������ʼʱ��
					GprsOnFluxOver();		//�ص�����,�������ɸ澯��¼����;
				}
			}
            if (pGprs->bCliState==CLI_STATE_TRANS)    //�ͻ��˴��ڴ���״̬����ͨ�ŵ���ָ��ʱ���Ͽ�����
            {
                if (dwClick-pProIf->dwRxClick > pGprs->wActiveDropSec)  //�ͻ�����������ʱ��
                {
                    DTRACE(DB_FAPROTO, ("GprsIfDoIfRelated: Close Cli Sock due to timeout %d secs!\n", pGprs->wActiveDropSec));
                    if (pGprs->bCnMode == CN_MODE_SOCKET)
                    {  
                        SocketIfDisConnect(pGprs->pSocketIf);
                    }
                    else if (pGprs->bCnMode == CN_MODE_EMBED)
                    {
                        pModem->pfnCloseCliSock(pModem);		//�ر�Ǳ�ڵĿͻ�������
                    }
                    pGprs->bCliState = CLI_STATE_IDLE;
                    m_bFailCnt = 0;
					pGprs->dwFluxOverClick = 0;
                }
            }
            
            if (pGprs->bSvrState==SVR_STATE_TRANS)     //�������˴��ڴ���״̬����ͨ�ŵ���ָ��ʱ���Ͽ�����
            {
                if (dwClick-GetSvrRxClick() > SVR_CLOSE_TO)  //��������ͨ�������ر�����ʱ��
                {
                    DTRACE(DB_FAPROTO, ("GprsIfDoIfRelated: Close Svr Sock due to 120s timeout!\n"));
                    if (pGprs->bCnMode == CN_MODE_SOCKET)
                    {  
                        SocketIfDisConnect(pGprs->pSocketIf);
                    }
                    else if (pGprs->bCnMode == CN_MODE_EMBED)
                    {
                        pModem->pfnCloseSvrSock(pModem);		//�ر�Ǳ�ڵĿͻ�������
                    }
                    pGprs->bSvrState = SVR_STATE_LISTEN;    //ģ����Ҫ����listen����ܼ��Ӵ��ڣ�ֱ�ӽ���SVR_STATE_ACCEPTģ����ղ�����վ����
                    m_bSvrFailCnt = 0;
					pGprs->dwFluxOverClick = 0;
                }
            }                
		}
	}
	else
#endif //EN_PROIF_SVR		//֧�ַ�����        
        if(pGprs->bOnlineMode == ONLINE_M_PERSIST)
	{
	  	if (IsFluxOver())
		{
			if (pGprs->dwFluxOverClick == 0)
			{
				pGprs->dwFluxOverClick = GetClick();
				GprsOnFluxOver();		//�ص�����,�������ɸ澯��¼����;
			}
		}
		else
		{
		    pGprs->dwFluxOverClick = 0;
		}
	}
}

//�������ⲿ����������ݲ�����֡
bool GprsIfRxFrm(struct TProIf* pProIf, BYTE* pbRxBuf, int iLen, bool fSvr)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//ͨ��Э��
	bool fRet = false;
	bool fSvrTrans; //���ڱ���ɵ�״̬

	if (pProIf->bState != IF_STATE_TRANS)
		return false;
	
#ifdef EN_PROIF_SVR		//֧�ַ�����		
	if (fSvr)
	{
		fSvrTrans = pGprs->fSvrTrans;	//���ڱ���ɵ�״̬
		pGprs->fSvrTrans = true;

		fRet = pPro->pfnRxFrm(pPro, pbRxBuf, iLen);

		pGprs->fSvrTrans = fSvrTrans;	//�ָ��ɵ�״̬
	}
	else
#endif //EN_PROIF_SVR		//֧�ַ�����
	{
		fSvrTrans = pGprs->fSvrTrans;	//���ڱ���ɵ�״̬
		pGprs->fSvrTrans = false;

		fRet = pPro->pfnRxFrm(pPro, pbRxBuf, iLen);

		pGprs->fSvrTrans = fSvrTrans;	//�ָ��ɵ�״̬
	}

	return fRet;
}

//������װ�طǸ�λ����
void GprsIfLoadUnrstPara(struct TProIf* pProIf)
{
   	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

    if (GetInfo(INFO_COMM_RLD))
    {
        DTRACE(DB_FAPROTO, ("GprsIfLoadUnrstPara: gprs para change, Reload!\r\n"));
        LoadGprsPara(pGprs);
       	pProIf->bState = IF_STATE_RST;
		m_bFailCnt = 0;
    	m_bSvrFailCnt = 0;
    }
    if (GetInfo(INFO_DISCONNECT))
    {
        GprsIfReInit(pProIf);
        pProIf->bState = IF_STATE_RST;
        pGprs->fCnSw = true;
        m_bFailCnt = 0;
    	m_bSvrFailCnt = 0;
    }
    if (GetInfo(INFO_COMM_RST))
    {
        Sleep(3000);
        DTRACE(DB_FAPROTO, ("GprsIfLoadUnrstPara: reset pProIf->bState!\r\n"));
        pProIf->bState = IF_STATE_RST;
		m_bFailCnt = 0;
    	m_bSvrFailCnt = 0;
    }
}

BYTE GetSign(struct TProIf* pProIf)
{
    BYTE bSign = 0;
    if (pProIf != NULL)
    {
       	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
        if (pGprs != NULL)
        {
            if (pGprs->bSignStrength <=31)
                bSign = pGprs->bSignStrength;
        }
    }

    return bSign;            
}

//#define PING_TEST

//�����̫����״̬
void CheckNetStat(void)
{
    BYTE bCnTypeOld;
    BYTE bCnTypeCurr;
#ifdef PING_TEST
    BYTE bFailCnt;
#endif
    static bool fPowerOff = false;
    ReadItemEx(BN2, PN0, 0x10d3, &bCnTypeCurr); 

#ifndef SYS_WIN
    if (IsAcCalib())
#endif
    { //todo:
        if (IsPowerOff()) //ͣ����
        {        
            if (!fPowerOff) //֮ǰ���е��
            {
                Sleep(200);
                if (IsPowerOff()) 
                {
                    fPowerOff = true;
                    if (bCnTypeCurr == CN_TYPE_GPRS) //��ǰ��GPRS
                    {
#ifndef SYS_WIN
                        PhyClose();   //�ڵ�
#endif
                        DTRACE(DB_FAPROTO, ("CheckNetStat: PHY power off\r\n"));
                    }
                }
            }
            return;   //ͣ���˲����״̬��
        }
        else
        {
            if (fPowerOff) //֮ǰ��û���
            {
                Sleep(200);
                if (!IsPowerOff()) 
                {
                    fPowerOff = false;
                    if (bCnTypeCurr == CN_TYPE_GPRS) //��ǰ��GPRS
                    {
				#ifndef SYS_WIN
                        PhyReset();   //PHY��λ
				#endif
                        DTRACE(DB_FAPROTO, ("CheckNetStat: PHY power on\r\n"));
                        Sleep(100);
                        return;  //����Ҳ�����״̬�ˡ�
                    }
                }
            }
        }
    }
    
    if ((bCnTypeCurr == CN_TYPE_ET) && (GetGprsIfState()==IF_STATE_RST))//��ǰ����̫�� �������ڸ�λ
        return;
    
    if (ETLinkUp())//��̫��������
    {
#ifndef PING_TEST
        BYTE bNetContTye = 0;
        ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //��̫�����ӷ�ʽ
        if (!DhcpGetIpOver() && (bNetContTye==0))//DHCP��ȡIP����  //0-��̫����1-PPPOE
            return;
#else
        CheckEthernet();
        bFailCnt = GetEthPingFailCnt();
        if (bFailCnt == 0)//PINGͨ��
        {
#endif
            if (bCnTypeCurr == CN_TYPE_GPRS)//��ǰ��GPRS����
            {                
                DTRACE(DB_FAPROTO, ("Gprs to ET\r\n"));
                bCnTypeOld = bCnTypeCurr;
                WriteItemEx(BN2, PN0, 0x2050, &bCnTypeOld); //�ϴε���������
                bCnTypeCurr = CN_TYPE_ET;                
                //��ǰ���Ӹ��³���̫��
                WriteItemEx(BN2, PN0, 0x10d3, &bCnTypeCurr); 
                SetInfo(INFO_DISCONNECT);
            }
#ifdef PING_TEST            
        }
        else if (bFailCnt > 6)//PING��ͨ��   (��������λPHY
        {                   
            if (bCnTypeCurr == CN_TYPE_ET)//�����ǰ����̫�����򷢶Ͽ���Ϣ����GPRS
            {               
                DTRACE(DB_FAPROTO, ("ET to Gprs\r\n"));
                bCnTypeOld = bCnTypeCurr;
                WriteItemEx(BN2, PN0, 0x2050, &bCnTypeOld); //�ϴε���������
                bCnTypeCurr = CN_TYPE_GPRS;                
                WriteItemEx(BN2, PN0, 0x10d3, &bCnTypeCurr);
                SetInfo(INFO_DISCONNECT);
            }           
        }
#endif
    }
    else
    {
        if (bCnTypeCurr == CN_TYPE_ET)//�����ǰ����̫�����򷢶Ͽ���Ϣ����GPRS
        {            
            DTRACE(DB_FAPROTO, ("ET to Gprs\r\n"));
            bCnTypeOld = bCnTypeCurr;
            WriteItemEx(BN2, PN0, 0x2050, &bCnTypeOld); //�ϴε���������
            bCnTypeCurr = CN_TYPE_GPRS;            
            WriteItemEx(BN2, PN0, 0x10d3, &bCnTypeCurr); 
            SetInfo(INFO_DISCONNECT);
        }
    }
}

void PppoeLinkdown(void)
{
    DTRACE(DB_FAPROTO, ("PppoeLinkdown\r\n"));
    SetInfo(INFO_COMM_RST);
}
