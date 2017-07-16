/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：GprsIf.c
 * 摘    要：本文件实现了GPRS通信接口类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
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
//GprsIf私有宏定义
/*
#define CN_MODE_SOCKET      0	//基于TCP/IP的通信模式
#define CN_MODE_SMS      	1	//短信
#define CN_MODE_EMBED     	2	//模块嵌入式协议栈
#define CN_MODE_COMM     	3	//串口通信模式
#define CN_MODE_CMD     	4	//命令模式*/

//客户端状态机
#define CLI_STATE_IDLE		0   //空闲:有主动上报需求->连接状态
#define CLI_STATE_CONNECT 	1	//连接:连接成功->登陆状态
#define CLI_STATE_LOGIN		2   //登录:登录成功->传输状态
#define CLI_STATE_TRANS		3   //传输:连续无通信规定时间->空闲状态

//服务器状态机
#define SVR_STATE_LISTEN 	0	//监听:设置监听端口->接收状态
#define SVR_STATE_ACCEPT	1   //接收:接收到新的连接->传输状态
#define SVR_STATE_TRANS		2   //传输:对方断开连接|超过心跳间隔->传输状态

#define RST_INTERV 			20	//复位间隔，单位秒
#define CONNECT_INTERV 		5   //5//60	//连接间隔，单位秒
#define LOGIN_INTERV 		5	//登陆间隔，单位秒
#define LISTEN_INTERV		30	//监听间隔，单位秒
#define SOCK_CHK_INTERV		200	//SOCKET检查间隔，单位秒
#define SVR_CLOSE_TO	    120	//混合模式服务器端无通信掉线时间

#define RST_NUM				5	//一次循环中允许的复位模块次数
#define BEAT_TEST_TIMES 	2	//心跳测试次数,   为0,表示不自动掉线,只周期发心跳
#define BEAT_TEST_TO		30	//心跳超时时间,单位秒

////////////////////////////////////////////////////////////////////////////////////////////
//GprsIf私有成员变量
static BYTE m_bFailCnt = 0;			//记录客户端各阶段的连续失败计数
static BYTE m_bSvrFailCnt = 0;		//记录服务器各阶段的连续失败计数
static BYTE m_bRstToDormanCnt = 0;	//复位到休眠的计数:任何情况的失败会导致模块复位，
									//当模块复位次数到达pGprs->bRstToDormanNum后，进入休眠状态
static BYTE m_bConnectFailCnt = 0;	//连接失败计数
static BYTE m_bLoginFailCnt = 0;		//登陆失败计数									
static DWORD m_dwDormanClick = 0;	//进入Dorman的时刻
static DWORD m_dwSvrChkClick = 0;	//上次服务器socket有效性的检查时刻
static DWORD m_dwCliChkClick = 0;	//上次客户端socket有效性的检查时刻
static DWORD m_dwSvrRxClick = 0;	//上次服务器socket接收时刻
static DWORD m_dwBeatClick = 0;
static DWORD m_dwDebugClick = 0;

////////////////////////////////////////////////////////////////////////////////////////////
//GprsIf私有函数定义
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
//GprsIf实现
bool GprsIfReInit(struct TProIf* pProIf)
{    
    TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;    
    struct TModem* pModem = pGprs->pModem;		//GPRS模块
    BYTE bCnType;
    BYTE bNetContTye = 0;
    
    ReadItemEx(BN2, PN0, 0x2050, &bCnType);//读取上次的连接类型
    //1、先关闭之前的连接
    if (bCnType == CN_TYPE_ET)
    {
        SocketIfDisConnect(pGprs->pSocketIf);    //将以太网的SOCKET关掉      
	#ifndef SYS_WIN
        ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //以太网连接方式
        if (bNetContTye == 1)//是PPPOE，要关闭
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
            pModem->pfnCloseCliSock(pModem);    //如果socket有效，先关闭socket        
        }
                    
        pModem->pfnClosePpp(pModem);    //如果PPP打开的,先关闭PPP          
    }
        
    ReadItemEx(BN2, PN0, 0x10d3, &bCnType); //读最新的连接类型
#ifndef SYS_WIN
	pGprs->bCnType = bCnType;     
#endif
	
    pGprs->pSocketIf->bCnType = pGprs->bCnType;
    
    if (pGprs->bCnType == CN_TYPE_ET)
    {        
        pGprs->bCnMode = CN_MODE_SOCKET;  //切换到以太网应该切入到SOCKET
        SetNetIfDefaultET(); //设置默认网关为以太网
        DTRACE(DB_FAPROTO, ("Shift to Ethernet!\r\n"));        
    }
    else //GPRS
    {          
        //以太网切换到GPRS时需要知道什么协议栈
        ReadItemEx(BN1, PN0, 0x2032, &bCnType);//TCP/IP协议库：0模块自带,1终端
        if (bCnType == 0)
            pGprs->bCnMode = CN_MODE_EMBED;
        else
            pGprs->bCnMode = CN_MODE_SOCKET;
        
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SetNetIfDefaultPPP(); //设置默认网关为PPP
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

	//基类结构成员初始化
	ProIfInit(pProIf);

	//类派生
	pProIf->pszName = "GprsIf";					//接口名称
	pProIf->wMaxFrmBytes = GPRS_MAX_BYTES;		//最大帧长度
	pProIf->bIfType = IF_GPRS;					//接口类型

	//虚函数，需要实例化为具体接口的对应函数
	pProIf->pfnDorman = GprsIfDorman;			//休眠
	pProIf->pfnReset = GprsIfReset;				//复位接口
	pProIf->pfnSend = GprsIfSend;				//发送函数
	pProIf->pfnReceive = GprsIfReceive;			//接收函数
	pProIf->pfnTrans = GprsIfTrans;				//传输状态函数
	pProIf->pfnOnResetOK = GprsIfOnResetOK;		//接口复位成功时的回调函数
	pProIf->pfnOnResetFail = GprsIfOnResetFail;	//接口复位失败时的回调函数
	pProIf->pfnOnRcvFrm = GprsIfOnRcvFrm;
	pProIf->pfnDoIfRelated = GprsIfDoIfRelated;	//接口相关特殊处理函数
   	pProIf->LoadUnrstPara = GprsIfLoadUnrstPara;	//接口相关特殊处理函数

	//派生类具体初始化1
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
    SetLedCtrlMode(LED_ONLINE, LED_MODE_ON);   //开灯
	SetLedCtrlMode(LED_RUN, LED_MODE_TOGGLE);//运行灯闪烁  
#endif    
    SocketIfOnLoginOK(pGprs->pSocketIf);
}

void OnGprsReset()
{
    //BYTE bConnectNew = 0;
    //WriteItemEx(BN5, 0, 0x5002, &bConnectNew);
    
#ifndef SYS_WIN
    SetLedCtrlMode(LED_ONLINE, LED_MODE_OFF);   //关灯
	SetLedCtrlMode(LED_RUN, LED_MODE_TOGGLE);//运行灯常亮 
#endif
}

//发送函数
bool GprsIfSend(struct TProIf* pProIf, BYTE* pbTxBuf, WORD wLen)
{
    int iRet = 0;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRS模块
    
    if (pGprs->bCnMode == CN_MODE_SOCKET) 
    {
        return SocketIfSend(pGprs->pSocketIf, pbTxBuf, wLen);        
    }
    else if (pGprs->bCnMode == CN_MODE_EMBED) 
    {
#ifdef EN_PROIF_SVR		//支持服务器
	    if (pGprs->fSvrTrans)
		    iRet = pModem->pfnSvrSend(pModem, pbTxBuf, wLen);
    	else
#endif //EN_PROIF_SVR
        {
	    	iRet = pModem->pfnCliSend(pModem, pbTxBuf, wLen);    
            if (iRet < 0) //链路出错，需要复位
            {                              
                DTRACE(DB_FAPROTO, ("GprsIfSend: send fail.\r\n"));
            }
        }
        if (iRet > 0)
        {
            DoLedBurst(LED_REMOTE_TX);
            if (pGprs->pSocketIf->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
    			AddFlux(iRet);
        }
    }
    else if (pGprs->bCnMode == CN_MODE_SMS)//暂时不支持
    {        
    }
    else
    {        
    }
    
    if (iRet == wLen)
        return true;
    return false;        
}

//接收函数
int GprsIfReceive(struct TProIf* pProIf, BYTE* pbRxBuf, WORD wBufSize)	
{
    int iLen = 0;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRS模块
    
    if (pGprs->bCnMode == CN_MODE_SOCKET) 
        return SocketIfReceive(pGprs->pSocketIf, pbRxBuf, wBufSize);
    else if (pGprs->bCnMode == CN_MODE_EMBED) 
    {
#ifdef EN_PROIF_SVR		//支持服务器
    	if (pGprs->fSvrTrans)
	    	iLen = pModem->pfnSvrReceive(pModem, pbRxBuf, wBufSize);
    	else
#endif //EN_PROIF_SVR
	    	iLen = pModem->pfnCliReceive(pModem, pbRxBuf, wBufSize);
        if (iLen > 0)
        {
            DoLedBurst(LED_REMOTE_RX);
            if (pGprs->pSocketIf->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
		        AddFlux((DWORD )iLen);
        }
    }
    else if (pGprs->bCnMode == CN_MODE_SMS)//暂时不支持
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
    if (pGprs->bCnType == CN_TYPE_ET)  //以太见网更新PING间隔
        UpdPingTime();
}

//描述：复位接口
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
			//m_dwDormanInterv = 0;	//临时设定的休眠间隔，单位秒
			pProIf->bState = IF_STATE_RST;
			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: wake up\n"));
		}		
	}
	else //m_dwDormanClick==0 
	{	 //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
		 //要退出这种状态,就要靠在DoIfRelated()中根据接口相关的
		 //情况把m_wState状态改变,比如从不在线时段切换到在线时段
		 //接口的插入等情况
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
	pProIf->bState = IF_STATE_DORMAN; //休眠
	m_dwDormanClick = GetClick();	 //进入休眠的开始时间
    if (m_dwDormanClick == 0)
    {
        Sleep(1000); //延时1S再取
        m_dwDormanClick = GetClick();
    }
	m_bRstToDormanCnt = 0;	//复位到休眠的计数:任何情况的失败会导致模块复位，
	//当模块复位次数到达pGprs->bRstToDormanNum后，进入休眠状态
	//GprsIfReset(pProIf);    模块一复位，又改变了pProIf->bState
}    		


//描述：本函数只在线程函数中调用，避免非主流流程改变接口状态
void GprsIfOnResetOK(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

	pGprs->bCliState = CLI_STATE_CONNECT;
	m_bFailCnt = 0;
	
	pGprs->bSvrState = SVR_STATE_LISTEN;	//if (pGprs->bOnlineMode == ONLINE_M_MIX) 	//客户机/服务器混合模式)
	m_bSvrFailCnt = 0;
	
	m_bRstToDormanCnt = 0;   //能复位成功，但登不上前置机，
}

//描述：本函数只在线程函数中调用，避免非主流流程改变接口状态
void GprsIfOnResetFail(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

	m_bFailCnt++;

	if (m_bFailCnt >= RST_NUM)	//当前阶段的连续失败计数到达限值，要决定复位模块或者休眠
	{				//一次循环中允许的复位模块次数	
		m_bFailCnt = 0;
        m_bRstToDormanCnt++;
		if (m_bRstToDormanCnt >= pGprs->bRstToDormanNum)	//复位到休眠的计数:模块真正复位的次数相当于RST_NUM*pGprs->bRstToDormanNum
		{	
			//任何情况的失败：
			//当模块复位次数没达到pGprs->bRstToDormanNum时，先去复位模块；
			//当模块复位次数到达pGprs->bRstToDormanNum后，进入休眠状态;
			
			m_bRstToDormanCnt = 0;
			if (pGprs->wDormanInterv != 0)
			{
				DTRACE(DB_FAPROTO, ("GprsIfOnResetFail: enter dorman\r\n"));
				GprsIfEnterDorman(pProIf);
			}
			else
			{
				Sleep(RST_INTERV*1000); //接口的连接间隔,单位秒
				pProIf->bState = IF_STATE_RST;
			}
		}
		else
		{
			Sleep(RST_INTERV*1000); //接口的连接间隔,单位秒
			pProIf->bState = IF_STATE_RST;
		}
	}
	else
	{
		Sleep(RST_INTERV*1000); //接口的连接间隔,单位秒
		pProIf->bState = IF_STATE_RST;
	}
}

//描述：复位接口
bool GprsIfReset(struct TProIf* pProIf)
{
	/*pProIf->bState = IF_STATE_TRANS;
#ifdef SYS_WIN
	return IF_RST_OK;
#endif //SYS_WIN

	WORD wTryCnt = 0;
	
again:
	ClosePpp();	//如果PPP打开的,先关闭PPP
	
	if (wCnMode==GPRS_MODE_IDLE || wCnMode==GPRS_MODE_PWROFF)	//可能别的接口要使用串口,这里先关闭,让出串口
	{		
		if (m_pWorkerPara->fEmbedProtocol)//模块协议栈的话就不跟别的地方共用串口了
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
	struct TModem* pModem = pGprs->pModem;		//GPRS模块
    static bool first = true;

//again:
//	SetLocalAddr(0);	//本机IP初始化为0
    
    OnGprsReset();
    if (!first)   //首次上电不存在SOCKET
    {
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SocketIfDisConnect(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
            pModem->pfnCloseCliSock(pModem);    //如果socket有效，先关闭socket        
        }
        first = false;
    }
        
    if (pGprs->bCnType == CN_TYPE_ET)//以太网
    {
	#ifndef SYS_WIN
		if (!pGprs->fCnSw)
        {            
            DTRACE(DB_FAPROTO, ("Reset mac and phy.\r\n"));
            SetInfo(INFO_MAC_RST);//复位PHY
            i = 30;
            while(i--) //
            {
                Sleep(1000);
                if (ETLinkUp())
                    break;
            }
        }
	#endif
        pGprs->bSignStrength = 0;    //关掉信号灯      
		pGprs->fCnSw = false;
        pProIf->bState = IF_STATE_TRANS;       
        
	#ifndef SYS_WIN
	    ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //以太网连接方式
        if (bNetContTye == 1) //1：pppoe拨号
        {
            //ClosePppoe(pGprs->ipd);   //先关闭之前打开的
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
        pModem->pfnClosePpp(pModem);    //如果PPP打开的,先关闭PPP
	    if (pModem->pfnResetModem(pModem) != MODEM_NO_ERROR)
    	{
    		pGprs->cLastErr = GPRS_ERR_RST;
    		return false;
    	}

    	//if (m_wState == IF_STATE_DORMAN)	//已经进入休眠模式，就此返回
    	//	return IF_RST_OK;
    /*
    	for (i=0; i<10; i++)   //不应该在这里更新场强，如果没插卡，将会在这里Sleep 20S
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
    
        pGprs->bSignStrength = pModem->pfnUpdateSignStrength(pModem);  //更新bSignStrength用于点灯
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
    
    			//GprsShedOthers();	//调度处于同一线程的其它任务
                pGprs->bSignStrength = pModem->pfnUpdateSignStrength(pModem);
    			Sleep(2000);
    		}
    	}        
        
        WriteItemEx(BN2, PN0, 0x1058, &pGprs->bSignStrength);
        
        if ((pGprs->bSignStrength==0) || (pGprs->bSignStrength>31)) //0用于关信号强度灯
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
	/*else if (pGprs->bCnMode == CN_MODE_SMS) 	 //短信
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

//描述:接口保活探测
void GprsIfKeepAlive(struct TProIf* pProIf)
{	
	DWORD dwClick, dwBrokenTime;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//通信协议
	DWORD dwBeatMinutes = pGprs->bCliBeatMin;		//客户端心跳间隔,单位分钟
	if (dwBeatMinutes == 0)
		return;
	
	dwClick = GetClick();
    dwBrokenTime = dwBeatMinutes*60 +	BEAT_TEST_TO*BEAT_TEST_TIMES;
	if (dwClick-pProIf->dwRxClick > dwBrokenTime)
	{	
		GprsIfDisConnectCli(pProIf); //重新初始化
		DTRACE(DB_FAPROTO, ("GprsIfKeepAlive: DisConnect at click %ld\r\n", dwClick));
	}
	else if (dwClick-pProIf->dwRxClick > dwBeatMinutes*60)//40 20秒还没收到过一帧，则进行心跳检测
	{	//刚开始时dwBeatClick为0，能马上发
		if (dwClick-m_dwBeatClick > BEAT_TEST_TO)
		{							//心跳超时时间,单位秒
			pPro->pfnBeat(pPro);//数据都发不出去了。
			m_dwBeatClick = dwClick;
			DTRACE(DB_FAPROTO, ("GprsIfKeepAlive: heart beat test at click %ld\r\n", dwClick));
		}
	}
}

//描述:在接口由连接转为断开的时候调用，不管是主动断开还是被动断开
bool GprsIfDisConnectCli(struct TProIf* pProIf)
{
    bool fRet = false;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRS模块
    
    if (pGprs->bCnMode == CN_MODE_SOCKET)
    {
        fRet = SocketIfDisConnect(pGprs->pSocketIf);
    }
    else if (pGprs->bCnMode == CN_MODE_EMBED)
    {
        fRet = pModem->pfnCloseCliSock(pModem);		//关闭潜在的客户端连接
    }
    else
    {
    }

	if (pGprs->bCliState > CLI_STATE_CONNECT)
		pGprs->bCliState = CLI_STATE_CONNECT;

	//心跳相关
	//m_dwRxClick = 0;	//不能清m_dwRxClick,不然正常运行m_pIfPara->dwNoRxRstAppInterv秒后,
						//只要有一次没有联网成功就会引起终端复位
	m_dwBeatClick = 0;

	//连接相关
	//m_wConnectFailCnt = 0; //断开连接不需要复位失败计数,因为有在socket
							 //方式下,每次连接前都主动地尝试断开之前的
							 //连接,清零会使连接失败计数失效,本计数只有
							 //在接口复位成功或者累计到重试次数后才清零
	return fRet;
}

//描述:检查是否需要激活,
//返回:如果需要激活则返回true,否则返回false
//备注:在下列情况下需要激活:1.接收激活消息(短信/振铃);2.主动上报
bool GprsIfCheckActivation(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//通信协议

	if (GetInfo(INFO_ACTIVE)) //收到了短信激活帧
	{
		DTRACE(DB_FAPROTO, ("GprsIfCheckActivation: switch to gprs mode due to rx activate info\n"));
		return true;
	}
	
	if (pGprs->fEnableAutoSendActive) //允许主动上报激活
	{
		if (pPro->pfnIsNeedAutoSend(pPro)) //需要主动上报
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
	if (tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)	//备用主站IP和端口
		bConnectNum *= 2;

	if (m_bFailCnt >= bConnectNum)	//当前阶段的连续失败计数到达限值，要决定复位模块或者休眠
	{
		m_bFailCnt = 0;
        m_bConnectFailCnt++;
		if (m_bConnectFailCnt >= pGprs->bRstToDormanNum)	//复位到休眠的计数:
		{	
			//任何情况的失败：
			//当模块复位次数没达到pGprs->bRstToDormanNum时，先去复位模块；
			//当模块复位次数到达pGprs->bRstToDormanNum后，进入休眠状态;
			
			m_bConnectFailCnt = 0;
			if (pGprs->wDormanInterv != 0)
			{
				DTRACE(DB_FAPROTO, ("GprsIfOnConnectFail: enter dorman\r\n"));
				GprsIfEnterDorman(pProIf);
			}
			else
			{
				//Sleep(CONNECT_INTERV*1000); //接口的连接间隔,单位秒
				pProIf->bState = IF_STATE_RST;
			}
		}
		else
		{
			//Sleep(CONNECT_INTERV*1000); //接口的连接间隔,单位秒
			pProIf->bState = IF_STATE_RST;
		}
	}
	else
	{
		Sleep(CONNECT_INTERV*1000); //接口的连接间隔,单位秒
	}
}

//描述:在协议登陆失败时调用,比如多少次失败后断开连接,连续3次登陆失败会重新CONNET切换主备，不需要复位模块
void GprsIfOnLoginFail(struct TProIf* pProIf)
{	
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;

    OnGprsReset();
    //SocketIfOnConnectFail(pGprs->pSocketIf);
	m_bFailCnt++;	
	if (m_bFailCnt >= pGprs->bLoginNum)	//当前阶段的连续失败计数到达限值，要决定复位模块或者休眠
	{					
		m_bFailCnt = 0; 
        pGprs->pSocketIf->wIPUseCnt = pGprs->pSocketIf->wConnectNum;  //登陆多次失败下次，标识当前IP使用次数到了，直接换一个IP连接。---liyan
        m_bLoginFailCnt++;
		if (m_bLoginFailCnt > pGprs->bRstToDormanNum)	//复位到休眠的计数
		{	
			//任何情况的失败：
			//当模块复位次数没达到pGprs->bRstToDormanNum时，先去复位模块；
			//当模块复位次数到达pGprs->bRstToDormanNum后，进入休眠状态;
		
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
			//Sleep(LOGIN_INTERV*1000); //接口的连接间隔,单位秒
			pProIf->bState = IF_STATE_RST;
		}
	}
	else
	{					//登录失败的次数还没到断开连接的次数
		Sleep(LOGIN_INTERV*1000); //登录间隔
	}
}

#ifdef EN_PROIF_SVR		//支持服务器
void GprsIfMixModeTrans(struct TProIf* pProIf)
{
	TMasterIp tMasterIp;
	TSvrPara tSvrPara;
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRS模块
	struct TPro* pPro = pProIf->pPro;	//通信协议
	DWORD dwClick = GetClick();
	bool fRes = false;
	BYTE bConnectNum;
    BYTE bBuf[120];

	//1.客户端处理
	pGprs->fSvrTrans = false;	//给发送/接收函数标识当前是在处理[客户端]
	switch (pGprs->bCliState)	//客户机状态机：连接->登陆->通信->主动断开->空闲
	{
	case CLI_STATE_IDLE:  	 	//空闲:有主动上报需求->连接状态
		break;

	case CLI_STATE_CONNECT:		//连接:连接成功->登陆状态
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SocketIfDisConnect(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
    		pModem->pfnCloseCliSock(pModem);		//关闭潜在的客户端连接
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
                fRes = SocketIfConnect(pGprs->pSocketIf);//todo:切换主站与备用主站
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
			m_bConnectFailCnt = 0;	//连接失败计数
			SocketIfOnConnectOK(pGprs->pSocketIf);
		}
		else
		{
			GprsIfOnConnectFail(pProIf);
		}
		break;

	case CLI_STATE_LOGIN:   	//登录:登录成功->传输状态
		if (pPro->pfnLogin(pPro))
		{
			pGprs->bCliState = CLI_STATE_TRANS;
			m_bFailCnt = 0;
            GprsOnLoginOk(pProIf);
            //SocketIfOnConnectOK(pGprs->pSocketIf); //连续3次登陆失败也要主备切换
		}
		else
		{         
			GprsIfOnLoginFail(pProIf);
		}
		break;

	case CLI_STATE_TRANS:   	//传输:连续无通信规定时间->空闲状态
		//接收帧处理
		if (pPro->pfnRcvFrm(pPro) < 0)//接收到的一帧,并已经对其进行处理
        {//返回错误，socket关闭
            GprsIfDisConnectCli(pProIf); //重新初始化
        } 

		//主动上送处理
		pPro->pfnAutoSend(pPro);

		//GprsIfKeepAlive(pProIf);  //混合模式不发心跳

		//定期查询socket是否还有效
		if (dwClick-pProIf->dwRxClick > SOCK_CHK_INTERV &&
			dwClick-m_dwCliChkClick > SOCK_CHK_INTERV)	//上次客户端socket有效性的检查时刻
		{
			m_dwCliChkClick = GetClick();
			if (!pModem->pfnChkCliStatus(pModem))	//socket已经断开
			{
				pGprs->bCliState = CLI_STATE_IDLE;
				m_bFailCnt = 0;
			}
		}
		break;
	}

	//2.服务器处理
	pGprs->fSvrTrans = true;	//给发送/接收函数标识当前是在处理[服务器]
	switch (pGprs->bSvrState)	//服务器状态机：监听->接收连接->服务器通信->
	{
	case SVR_STATE_LISTEN:		//监听:设置监听端口->接收状态
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
				Sleep(LISTEN_INTERV*1000); //监听间隔
			}
		}
		break;

	case SVR_STATE_ACCEPT:   	//接收:接收到新的连接->传输状态
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

		if (pModem->pfnIsSvrAcceptOne(pModem))	//接收到新的连接  //todo:
        {
			pGprs->bSvrState = SVR_STATE_TRANS;
            UpdateSvrRxClick();
        }

		break;

	case SVR_STATE_TRANS:   	//传输:对方断开连接|超过心跳间隔->传输状态
		//接收帧处理
		if (pPro->pfnRcvFrm(pPro) < 0)//接收到的一帧,并已经对其进行处理
        {//返回错误，socket关闭
            GprsIfDisConnectCli(pProIf); //重新初始化
        } 

		//定期检查服务器的socket连接是否依然有效
		if (dwClick-GetSvrRxClick() > SOCK_CHK_INTERV &&
			dwClick-m_dwSvrChkClick > SOCK_CHK_INTERV)	//上次客户端socket有效性的检查时刻
		{
			m_dwSvrChkClick = GetClick();
			if (!pModem->pfnChkSvrStatus(pModem))	//检查服务器的socket连接是否依然有效：要主动发命令查询
			{
				pGprs->bSvrState = SVR_STATE_LISTEN;	//重新侦听 
				m_bSvrFailCnt = 0;
			}
		}
		
		break;
    default:
        GprsIfDisConnectCli(pProIf); //重新初始化
        break;
	}

	pGprs->fSvrTrans = false;	//给发送/接收函数标识当前是在处理[客户端]
}
#endif //EN_PROIF_SVR		//支持服务器

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
	struct TPro* pPro = pProIf->pPro;	//通信协议
	DWORD dwClick = GetClick();

	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRS模块

	//1.客户端处理
	pGprs->fSvrTrans = false;	//给发送/接收函数标识当前是在处理[客户端]
	switch (pGprs->bCliState)	//客户机状态机：连接->登陆->通信->主动断开->空闲
	{
//	case CLI_STATE_IDLE:  	 	//空闲:有主动上报需求->连接状态
//		break;

	case CLI_STATE_CONNECT:		//连接:连接成功->登陆状态
        //SetLed(false, LED_ONLINE);   //关灯
        //g_fAlertLed = false;    //关灯
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {
            SocketIfDisConnect(pGprs->pSocketIf);
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
    		pModem->pfnCloseCliSock(pModem);		//关闭潜在的客户端连接
        }
		bConnectNum = pGprs->bConnectNum;
		GetMasterIp(&tMasterIp);
        
        if (pGprs->bCnMode == CN_MODE_SOCKET)
        {   
            fRes = SocketIfConnect(pGprs->pSocketIf);//todo:
        }
        else if (pGprs->bCnMode == CN_MODE_EMBED)
        {
			if (pGprs->pSocketIf->fBakIP	//当前要使用备用地址
				&& tMasterIp.dwBakIP!=0 && tMasterIp.wBakPort!=0)//备用IP端口有效
			{		//备用IP端口有效
				pGprs->pSocketIf->wIPUseCnt++;
		        if (pGprs->pSocketIf->wIPUseCnt > pGprs->pSocketIf->wConnectNum)
		        {
		            pGprs->pSocketIf->wIPUseCnt = 1;   //已经使用一次了
		            pGprs->pSocketIf->fBakIP = false;  //本回切换到主IP
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
		            pGprs->pSocketIf->wIPUseCnt = 1; //已经使用一次了
		           	pGprs->pSocketIf->fBakIP = true; //本切换到备用IP
					fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwBakIP, tMasterIp.wBakPort);
		        }
		        else
		        {
		        	pGprs->pSocketIf->fBakIP = false;  //本回用主IP
					fRes = pModem->pfnConnect(pModem, tMasterIp.fUdp, tMasterIp.dwRemoteIP, tMasterIp.wRemotePort);
		        }
			}
        }

		if (fRes)
		{
			pGprs->bCliState = CLI_STATE_LOGIN;
			m_bFailCnt = 0;
            m_bConnectFailCnt = 0;	//连接失败计数
            pProIf->dwRxClick = GetClick();//防止连上之后还没有通信，没有更新该值，之后的keepalive中断开
            //SocketIfOnConnectOK(pGprs->pSocketIf);
		}
		else
		{
			GprsIfOnConnectFail(pProIf);
		}
		break;

	case CLI_STATE_LOGIN:   	//登录:登录成功->传输状态
		if (pPro->pfnLogin(pPro))
		{
			pGprs->bCliState = CLI_STATE_TRANS;
			m_bFailCnt = 0;
            GprsOnLoginOk(pProIf);
            //SocketIfOnConnectOK(pGprs->pSocketIf); //连续3次登陆失败也要主备切换
		}
		else
		{
			GprsIfOnLoginFail(pProIf);
		}
		break;

	case CLI_STATE_TRANS:   	//传输:连续无通信规定时间->空闲状态
		//接收帧处理
		if (pPro->pfnRcvFrm(pPro) < 0)//接收到的一帧,并已经对其进行处理
		{//返回错误，socket关闭
			GprsIfDisConnectCli(pProIf); //重新初始化
            break;  //应该返回，否则马上会发送一次心跳
		}
		else if (dwClick-pProIf->dwRxClick < pGprs->bCliBeatMin*60)
		{
			//空闲时主动上送处理
			pPro->pfnAutoSend(pPro);
		}

		GprsIfKeepAlive(pProIf);

		//定期查询socket是否还有效
		if (dwClick-pProIf->dwRxClick > SOCK_CHK_INTERV &&
			dwClick-m_dwCliChkClick > SOCK_CHK_INTERV)	//上次客户端socket有效性的检查时刻
		{
			m_dwCliChkClick = GetClick();
            if (pGprs->bCnMode == CN_MODE_SOCKET)
            {   
            }
            else if (pGprs->bCnMode == CN_MODE_EMBED)
            {
                if (!pModem->pfnChkCliStatus(pModem))	//socket已经断开
    			{
       				pGprs->bCliState = CLI_STATE_CONNECT;   //CLI_STATE_IDLE;
    				m_bFailCnt = 0;
    			}
            }			
		}
		break;
    default:
        GprsIfDisConnectCli(pProIf);  //断开重新初始化
        break;
	}
	
}

void GprsIfTrans(struct TProIf* pProIf)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
//	struct TModem* pModem = pGprs->pModem;		//GPRS模块
	
	switch (pGprs->bOnlineMode)
	{
	case ONLINE_M_PERSIST:  //永久在线模式
		GprsIfPersistModeTrans(pProIf);
		break;

#ifdef EN_PROIF_SVR		//支持服务器
	case ONLINE_M_MIX:    	//客户机/服务器混合模式
		GprsIfMixModeTrans(pProIf);
		break;
#endif //EN_PROIF_SVR		//支持服务器
    default:
        pGprs->bOnlineMode = ONLINE_M_PERSIST;
        break;
	}
}

//描述:做一些各个接口相关的非标准的事情,
//		本接口要做的事情有:
//		1.模式切换,比如非连续在线方式下,GPRS和SMS间的切换
void GprsIfDoIfRelated(struct TProIf* pProIf)
{
	//非连续在线模式下,要实现的切换有:
	//GPRS一段时间没有通信,切换回SMS方式
	//SMS收到短信唤醒或电话唤醒,切换到GPRS方式
	
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TModem* pModem = pGprs->pModem;		//GPRS模块
	DWORD dwClick = GetClick();
    
    if (pGprs->dwNoRxRstAppInterv != 0) //无接收复位终端间隔,单位秒,0表示不复位
	{
		if (dwClick-pProIf->dwRxClick > pGprs->dwNoRxRstAppInterv)
		{
			DTRACE(DB_FAPROTO, ("DoIfRelated: GPRS/ET no rx reset app!\n"));
			SetInfo(INFO_APP_RST);	//CPU复位 INFO_APP_RST
			return;
		}
	}

#ifdef EN_PROIF_SVR		//支持服务器
	if (pGprs->bOnlineMode == ONLINE_M_MIX)
	{
		if (dwClick-GetSvrRxClick() > (DWORD )pGprs->bSvrBeatMin*60)	//服务器在规定时间内没有通信，则客户端重新登陆，服务器重新监听
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
    			pModem->pfnCloseSvrSock(pModem);		//关闭服务器连接
    			pModem->pfnCloseListen(pModem);		//关闭监听
            }

			if (pProIf->bState >= IF_STATE_TRANS)	//目前PPP还是处于连接状态，为了节省流量，
			{										//先直接尝试socket连接，不行的话在复位接口
				pGprs->bSvrState = SVR_STATE_LISTEN;	//服务器重新监听
				m_bSvrFailCnt = 0;

				pGprs->bCliState = CLI_STATE_CONNECT;	//客户端重新连接，如果客户端连接失败，再去重新拨号
				m_bFailCnt = 0;

				m_bRstToDormanCnt = 0;	//清复位到休眠的计数，保证在连接失败的情况下，有机会复位接口再来
			}
			else
			{
				pProIf->bState = IF_STATE_RST;
				m_bFailCnt = 0;
				m_bSvrFailCnt = 0;
			}
		}
		else if (GprsIfCheckActivation(pProIf))	//有主动上报需求
		{
            DTRACE(DB_FAPROTO, ("GprsIfDoIfRelated: Activation due to need auto send!\n"));
			if (pProIf->bState < IF_STATE_RST)	//目前PPP还是处于连接状态，为了节省流量
			{
				pProIf->bState = IF_STATE_RST;
				m_bFailCnt = 0;
				m_bSvrFailCnt = 0;
			}
			else if (pProIf->bState>=IF_STATE_TRANS && //目前PPP还是处于连接状态，可以直接socket连接
					 pGprs->bCliState<CLI_STATE_CONNECT) //处于空闲则要进行连接，其它状态继续原来的操作即可
			{
				pGprs->bCliState = CLI_STATE_CONNECT;
				m_bFailCnt = 0;
			}
		}
		else if (pProIf->bState==IF_STATE_TRANS)    //目前PPP还是处于连接状态
		{
		  	if (IsFluxOver())
			{
			  	if (pGprs->dwFluxOverClick == 0)
				{
					pGprs->dwFluxOverClick = GetClick();	//流量超标的起始时标
					GprsOnFluxOver();		//回调函数,用于生成告警记录等用途
				}
			}
            if (pGprs->bCliState==CLI_STATE_TRANS)    //客户端处于传输状态，无通信到达指定时间后断开连接
            {
                if (dwClick-pProIf->dwRxClick > pGprs->wActiveDropSec)  //客户端主动掉线时间
                {
                    DTRACE(DB_FAPROTO, ("GprsIfDoIfRelated: Close Cli Sock due to timeout %d secs!\n", pGprs->wActiveDropSec));
                    if (pGprs->bCnMode == CN_MODE_SOCKET)
                    {  
                        SocketIfDisConnect(pGprs->pSocketIf);
                    }
                    else if (pGprs->bCnMode == CN_MODE_EMBED)
                    {
                        pModem->pfnCloseCliSock(pModem);		//关闭潜在的客户端连接
                    }
                    pGprs->bCliState = CLI_STATE_IDLE;
                    m_bFailCnt = 0;
					pGprs->dwFluxOverClick = 0;
                }
            }
            
            if (pGprs->bSvrState==SVR_STATE_TRANS)     //服务器端处于传输状态，无通信到达指定时间后断开连接
            {
                if (dwClick-GetSvrRxClick() > SVR_CLOSE_TO)  //服务器无通信主动关闭连接时间
                {
                    DTRACE(DB_FAPROTO, ("GprsIfDoIfRelated: Close Svr Sock due to 120s timeout!\n"));
                    if (pGprs->bCnMode == CN_MODE_SOCKET)
                    {  
                        SocketIfDisConnect(pGprs->pSocketIf);
                    }
                    else if (pGprs->bCnMode == CN_MODE_EMBED)
                    {
                        pModem->pfnCloseSvrSock(pModem);		//关闭潜在的客户端连接
                    }
                    pGprs->bSvrState = SVR_STATE_LISTEN;    //模块需要重新listen后才能监视串口，直接进入SVR_STATE_ACCEPT模块会收不到主站数据
                    m_bSvrFailCnt = 0;
					pGprs->dwFluxOverClick = 0;
                }
            }                
		}
	}
	else
#endif //EN_PROIF_SVR		//支持服务器        
        if(pGprs->bOnlineMode == ONLINE_M_PERSIST)
	{
	  	if (IsFluxOver())
		{
			if (pGprs->dwFluxOverClick == 0)
			{
				pGprs->dwFluxOverClick = GetClick();
				GprsOnFluxOver();		//回调函数,用于生成告警记录等用途
			}
		}
		else
		{
		    pGprs->dwFluxOverClick = 0;
		}
	}
}

//描述：外部传入接收数据并处理帧
bool GprsIfRxFrm(struct TProIf* pProIf, BYTE* pbRxBuf, int iLen, bool fSvr)
{
	TGprsIf* pGprs = (TGprsIf* )pProIf->pvIf;
	struct TPro* pPro = pProIf->pPro;	//通信协议
	bool fRet = false;
	bool fSvrTrans; //用于保存旧的状态

	if (pProIf->bState != IF_STATE_TRANS)
		return false;
	
#ifdef EN_PROIF_SVR		//支持服务器		
	if (fSvr)
	{
		fSvrTrans = pGprs->fSvrTrans;	//用于保存旧的状态
		pGprs->fSvrTrans = true;

		fRet = pPro->pfnRxFrm(pPro, pbRxBuf, iLen);

		pGprs->fSvrTrans = fSvrTrans;	//恢复旧的状态
	}
	else
#endif //EN_PROIF_SVR		//支持服务器
	{
		fSvrTrans = pGprs->fSvrTrans;	//用于保存旧的状态
		pGprs->fSvrTrans = false;

		fRet = pPro->pfnRxFrm(pPro, pbRxBuf, iLen);

		pGprs->fSvrTrans = fSvrTrans;	//恢复旧的状态
	}

	return fRet;
}

//描述：装载非复位参数
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

//检查以太网的状态
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
        if (IsPowerOff()) //停电了
        {        
            if (!fPowerOff) //之前是有电的
            {
                Sleep(200);
                if (IsPowerOff()) 
                {
                    fPowerOff = true;
                    if (bCnTypeCurr == CN_TYPE_GPRS) //当前是GPRS
                    {
#ifndef SYS_WIN
                        PhyClose();   //节电
#endif
                        DTRACE(DB_FAPROTO, ("CheckNetStat: PHY power off\r\n"));
                    }
                }
            }
            return;   //停电了不检查状态了
        }
        else
        {
            if (fPowerOff) //之前是没电的
            {
                Sleep(200);
                if (!IsPowerOff()) 
                {
                    fPowerOff = false;
                    if (bCnTypeCurr == CN_TYPE_GPRS) //当前是GPRS
                    {
				#ifndef SYS_WIN
                        PhyReset();   //PHY复位
				#endif
                        DTRACE(DB_FAPROTO, ("CheckNetStat: PHY power on\r\n"));
                        Sleep(100);
                        return;  //本次也不检查状态了。
                    }
                }
            }
        }
    }
    
    if ((bCnTypeCurr == CN_TYPE_ET) && (GetGprsIfState()==IF_STATE_RST))//当前是以太网 而且正在复位
        return;
    
    if (ETLinkUp())//以太网连接了
    {
#ifndef PING_TEST
        BYTE bNetContTye = 0;
        ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //以太网连接方式
        if (!DhcpGetIpOver() && (bNetContTye==0))//DHCP获取IP结束  //0-以太网，1-PPPOE
            return;
#else
        CheckEthernet();
        bFailCnt = GetEthPingFailCnt();
        if (bFailCnt == 0)//PING通了
        {
#endif
            if (bCnTypeCurr == CN_TYPE_GPRS)//当前是GPRS连接
            {                
                DTRACE(DB_FAPROTO, ("Gprs to ET\r\n"));
                bCnTypeOld = bCnTypeCurr;
                WriteItemEx(BN2, PN0, 0x2050, &bCnTypeOld); //上次的连接类型
                bCnTypeCurr = CN_TYPE_ET;                
                //当前连接更新成以太网
                WriteItemEx(BN2, PN0, 0x10d3, &bCnTypeCurr); 
                SetInfo(INFO_DISCONNECT);
            }
#ifdef PING_TEST            
        }
        else if (bFailCnt > 6)//PING不通了   (次数）复位PHY
        {                   
            if (bCnTypeCurr == CN_TYPE_ET)//如果当前是以太网，则发断开消息切入GPRS
            {               
                DTRACE(DB_FAPROTO, ("ET to Gprs\r\n"));
                bCnTypeOld = bCnTypeCurr;
                WriteItemEx(BN2, PN0, 0x2050, &bCnTypeOld); //上次的连接类型
                bCnTypeCurr = CN_TYPE_GPRS;                
                WriteItemEx(BN2, PN0, 0x10d3, &bCnTypeCurr);
                SetInfo(INFO_DISCONNECT);
            }           
        }
#endif
    }
    else
    {
        if (bCnTypeCurr == CN_TYPE_ET)//如果当前是以太网，则发断开消息切入GPRS
        {            
            DTRACE(DB_FAPROTO, ("ET to Gprs\r\n"));
            bCnTypeOld = bCnTypeCurr;
            WriteItemEx(BN2, PN0, 0x2050, &bCnTypeOld); //上次的连接类型
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
