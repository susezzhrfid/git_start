/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：GL868.h
 * 摘    要：本文件实现了GL868MODEM子类
 * 当前版本：1.0
 * 作    者：李焱
 * 完成日期：2012年5月
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
//GL868私有宏定义
#define GL868_INVALID_SOCK	0xff	//无效的socket通道    
#define BACKLOG     10              //最大同时连接的请求数
////////////////////////////////////////////////////////////////////////////////////////////
//GL868私有成员变量


/////////////////////////////////////////////////////////////////////////////////////////////////////
//GL868实现

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
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例

	//基类结构成员初始化
	ModemInit(pModem);
    

	//参数

	//虚函数，需要实例化为具体接口的对应函数
	pModem->pfnResetModem = GL868ResetModem;		//复位模块
	pModem->pfnUpdateSignStrength = UpdateSignStrength;	//更新场强
	pModem->pfnInitAPN = GL868InitAPN;			//初始化APN

	pModem->pfnOpenPpp = GL868OpenPpp;			//PPP拨号上网
	pModem->pfnClosePpp = GL868ClosePpp;		//断开PPP连接

	pModem->pfnConnect = GL868Connect; 		//作为客户端连接服务器
	pModem->pfnCloseCliSock = GL868CloseCliSock;	//关闭客户端socket

	pModem->pfnCliSend = GL868CliSend; 	//作为客户端发送数据
	pModem->pfnCliReceive = GL868CliReceive; 	//作为客户端接收数据
	pModem->pfnSpecHandle = GL868SpecHandle; //对模块主动发过来的的数据进行特殊处理,如TCP/UDP报文、收到来自监听端口的新连接

	pModem->pfnChkCliStatus = GL868ChkCliStatus;	//检查客户端的socket连接是否依然有效：要主动发命令查询

#ifdef EN_PROIF_SVR		//支持服务器
	pModem->pfnListen = GL868Listen;  		//作为服务器监听端口
	pModem->pfnCloseListen = GL868CloseListen;		//关闭监听
	pModem->pfnCloseSvrSock = GL868CloseSvrSock;	//关闭服务器已经连接的socket
	pModem->pfnSvrSend = GL868SvrSend; 	//作为服务器发送数据
	pModem->pfnSvrReceive = GL868SvrReceive; 	//作为服务器接收数据
	pModem->pfnIsSvrAcceptOne = GL868IsSvrAcceptOne;	//作为服务器是否接收到一个客户端的连接
	pModem->pfnChkSvrStatus = GL868ChkSvrStatus;	//检查服务器的socket连接是否依然有效：要主动发命令查询
#endif //EN_PROIF_SVR		//支持服务器

	//派生类具体初始化
	//pModem->pszCSQ = "AT+CSQ\r\n";
	pGL868->bCliSock = 0;	//客户端的socket号，目前固定只使用0
	pGL868->fPowOn = true;	//是否是刚上电
	pGL868->fCliUdp = false;	//客户端是否是UDP连接
    
    pGL868->pSocket = &g_tSock;   
    SocketInit(pGL868->pSocket);

#ifdef EN_PROIF_SVR		//支持服务器
	pGL868->bSvrSock = GL868_INVALID_SOCK;	//服务器端的socket号，只能为0~9，赋值0xff表示无效。
#endif //EN_PROIF_SVR		//支持服务器
}

int GL868InitAPN(struct TModem* pModem)
{
	char szApnCmd[64];
	char szApn[32];
	int  iRet;		
	WORD i, m;
	
	pModem->bStep = MODEM_STEP_SIM;
	//看网络是否注册好了,只是为了方便调试
	for (i=0; i<30; i++)
	{
		if (ATCommand(pModem, "AT+CPIN?\r\n", "OK", "ERROR", NULL, 3) == 1)  //看卡是否插好
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
		{//卡是不能热插拔的，所以这里如果检测不到卡的话就没有必要继续检测了
			return MODEM_SIM_FAIL;
		}
		Sleep(1000);
	}
	
	//网络没注册好,不去拨号
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
		if (ATCommand(pModem, szApnCmd, "OK", NULL, NULL, 3) <= 0)   //失败了多试一次
		{					//APN目前只在登录GPRS时设置，SMS没有设置
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
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
	WORD i, j;
	WORD wErrCnt = 0;
	WORD wRead = 0;
	BYTE bBuf[64];
	int iLen;
	
	pModem->bStep = MODEM_STEP_RST;
       
	while (1)
	{	
		//m_wRunCnt++;
		//复位模块	
		if (pGL868->fPowOn == true)
		{
			pGL868->fPowOn = false;
            
			ModemPowerOn();         //模电源被切断
                            
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
			if (wErrCnt!=0 && wErrCnt%6==0)  //关模块电源
			{
				DTRACE(DB_FAPROTO, ("CGL868/ME3000::ResetGPRS: power down reset!\r\n"));
				
				ModemPowerOff();	//关模块电源至少要2秒以上
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
					if (wRead>=8 && bufbuf(bBuf, wRead, (BYTE *)"+CFUN: 1", 8)!=NULL)  //接收启动信息
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
		    {//终端协议栈不接收全0的网关
		    	for (i=0; i<5; i++)
		    	{
		    		if (ATCommand(pModem, "AT&F\r\n", "OK", NULL, NULL, 0) > 0)
		    			break;
		    	}
		        ATCommand(pModem, "AT$GATEWAY=\"10.98.96.7\"\r\n", "OK", NULL, NULL, 3);
		    }
		    	
			wErrCnt = 0;
			if (ATCommand(pModem, "AT+IPR?\r\n", "+IPR: 115200", NULL, NULL, 0) <= 0)  //波特率自适应的把波特率设定为9600
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

		wErrCnt++;	//连续失败次数累计
		if (wErrCnt%3 == 0)  //AT命令不通过,尝试修改波特率
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

		//保护模块,不要太频繁复位模块
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
//查询客户端是否发送完成
WORD GetGL868SendBufLeft(struct TModem* pModem, bool fSvr)
{
    char* p;
    WORD wLen;
    WORD wBufLen;
    BYTE bBuf[40];
	char szCmd[20];
	char szConnect[32];

   	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    if (fSvr== false)
    {
    	sprintf(szCmd, "AT+IPSTATUS=%d\r\n", pGL868->bCliSock);		//客户端的socket号，目前固定只使用0
         if (pGL868->fCliUdp)
          	sprintf(szConnect, "+IPSTATUS:%d,CONNECT,UDP,", pGL868->bCliSock);
        else
        	sprintf(szConnect, "+IPSTATUS:%d,CONNECT,TCP,", pGL868->bCliSock);
       	DTRACE(DB_FAPROTO, ("GL868ChkCliStatus : tx AT+IPSTATUS=%d.\r\n", pGL868->bCliSock));
    }
    else
    {
        sprintf(szCmd, "AT+CLIENTSTATUS=%d\r\n", pGL868->bSvrSock);		//服务器端的socket号，目前固定只使用0
         if (pGL868->fCliUdp)
          	sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT,UDP,", pGL868->bSvrSock);
        else
        	sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT,TCP,", pGL868->bSvrSock);
        
       	DTRACE(DB_FAPROTO, ("GL868ChkSvrStatus : tx AT+CLIENTSTATUS=%d.\r\n", pGL868->bSvrSock));
    }    
   
	wLen = strlen(szCmd);
    if (CommWrite(pModem->bComm, (BYTE *)szCmd, wLen, 1000) != wLen) 
        return 0;
	
	Sleep(500);  //避免Read接收到以前的通信数据而立即返回
	
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

//作为客户端发送数据
int GL868CliSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen)
{
    //static DWORD dwLastTick=0;
    //static WORD wLastSnd=0;
    
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    
    //if ()//终端协议栈
    //{            
    if (SocketSend(pGL868->pSocket, pbTxBuf, wLen)) //todotodo:如果SOCKET断开返回什么？
        return wLen;
    return 0;
    //}
    //else//模块协议栈
  /*   {
       
        if (pModem->bModuleVer == MODULE_ME590)
	{
		TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
		WORD nStrLen;
	 	int iGprsRet = -1;
		char szCmd[32];
		char szAnsOK[32];
		BYTE bBuf[10];
		
		//if (!ATCmdTest(pModem, 1))
		//	return 0;
        if (wLastSnd >= 300) //上一帧超过800字节，需要等待3秒模块发完后才能接着发
        {
            while (GetTick()-dwLastTick<=15000)
            {
                if (GetGL868SendBufLeft(pModem, false) > 1800) //2047为最大缓冲区
                {
                    Sleep(100);
                    break;
                }

                Sleep(1200);
            }
        }
		
		if (pGL868->fCliUdp)
			sprintf(szCmd, "AT+UDPSEND=%d,%d\r\n", pGL868->bCliSock, wLen);	//客户端的socket号，目前固定只使用0
		else
			sprintf(szCmd, "AT+TCPSEND=%d,%d\r\n", pGL868->bCliSock, wLen);	//客户端的socket号，目前固定只使用0

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

//描述：作为客户端接收数据,不能直接返回数据，只能使用回调的方式
int GL868CliReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
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

	return iRxLen;	//不能直接返回数据，只能使用回调的方式        
}

//检查客户端的socket连接是否依然有效：要主动发命令查询
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
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
	char szCmd[20];
	char szConnect[32];
	char szDisConnect[32];

	sprintf(szCmd, "AT+IPSTATUS=%d\r\n", pGL868->bCliSock);		//客户端的socket号，目前固定只使用0
	sprintf(szConnect, "+IPSTATUS:%d,CONNECT", pGL868->bCliSock);
	sprintf(szDisConnect, "+IPSTATUS:%d,DISCONNECT", pGL868->bCliSock);

	if (ATCommand(pModem, szCmd, szConnect, szDisConnect, NULL, 3) > 0)
		return true;
	else
		return false;*/    
}

//作为客户端连接服务器
bool GL868Connect(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    return SocketConnect(pGL868->pSocket);
    /*
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
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
			sprintf(szCmd, "AT+UDPCLOSE=%d\r\n", pGL868->bCliSock);	//客户端的socket号，目前固定只使用0
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
    //{//终端协议栈，从SOCKET发送数据
    //}
    
	return false; */
}

bool GL868CloseCliSock(struct TModem* pModem)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    return SocketClose(pGL868->pSocket);
    /*
	if (pModem->bModuleVer == MODULE_ME590)
	{
		TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
		char szCmd[48];
		sprintf(szCmd, "AT+TCPCLOSE=%d\r\n", pGL868->bCliSock);	//客户端的socket号，目前固定只使用0
		ATCommand(pModem, szCmd, "OK", "+TCPCLOSE:Error", NULL, 3);

		sprintf(szCmd, "AT+UDPCLOSE=%d\r\n", pGL868->bCliSock);	//客户端的socket号，目前固定只使用0
		ATCommand(pModem, szCmd, "OK", "+UDPCLOSE:Error", NULL, 3);
        
        //lwip_close(pGL868->bCliSock);
	}
    //else if (pModem->bModuleVer==MODULE_GL868)
    //{
    //}
	*/	
}

DWORD g_dwLocalIp = 0;	//本机IP
void SetLocalAddr(DWORD dwIP)
{
	g_dwLocalIp = dwIP;
}

DWORD GetLocalAddr()
{
	return g_dwLocalIp;
}

//建立PPP连接
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

//断开PPP连接
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
//描述：从MODEM接收的数据中查找是否有
char* GL868IsRxIpFrm(struct TModem* pModem, char* pszRx, bool* pfSvr, WORD* pwLen)
{
//接收到主站的数据：+TCPRECV(S) 
//+TCPRECV(S):1,10,1234567899

//指示接收到的 TCP 数据
//+TCPRECV:<n>,<length>,<data>
//+TCPRECV:0,10,1234567890
	TGL868* pGL868 = (TGL868* )pModem->pvModem;

	char* p = strstr(pszRx, "+TCPRECV");
	if (p == NULL)
		return NULL;

	if (strstr(p, "+TCPRECV(S)") == p)	//开头的就是服务器头
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

	p++;	//跳过逗号
	*pwLen = str2num(&p);
	return p;
}*/


//描述：对模块主动发过来的的数据进行特殊处理,如TCP/UDP报文、收到来自监听端口的新连接
bool GL868SpecHandle(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize)
{
	//TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
	//char* p;
	//BYTE* pbRxFrm;
	//DWORD dwTick;
	//WORD wLen, wFrmLen;
	bool fSvr = false;
    
    //pbRxFrm = NULL;

	//1.如果意外收到TCP/UDP报文，则把所有字节都收完才返回
	//pbRxFrm = (BYTE* )GL868IsRxIpFrm(pModem, pszRxBuf, &fSvr, &wFrmLen); //是否接收到TCP/UDP报文
	//if (pbRxFrm != NULL)	//接收到TCP/UDP报文
	//{
		//if (pbRxFrm < (BYTE *)pszRxBuf+wRxLen)	//返回指针合理
		//{
			if (GprsIfRxFrm(pModem->pProIf, (BYTE *)pszRxBuf, wRxLen, fSvr)) //组成了完整帧
				return true;
		//}

		//在没有组成完整帧的情况下，继续收完剩余字节才返回
		//dwTick = GetTick();
		//do
		//{
			//wLen = CommRead(pModem->bComm, (BYTE* )pszRxBuf, wBufSize, 500);
			//if (GprsIfRxFrm(pModem->pProIf, (BYTE* )pszRxBuf, wLen, fSvr)) //组成了完整帧
				//return true;

			//if (wLen == 0)
				//Sleep(10);
		//} while (GetTick()-dwTick < 5000);

		//return false;
	//}
	
#ifdef EN_PROIF_SVR		//支持服务器
	//2.看是否有收到来自监听端口的新连接
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
#endif //EN_PROIF_SVR		//支持服务器

	return false;
}


#ifdef EN_PROIF_SVR		//支持服务器
//描述：作为服务器监听端口
bool GL868Listen(struct TModem* pModem, bool fUdp, WORD wLocalPort)  
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    return listen(pGL868->pSocket->iSocket, BACKLOG);
    /*
	char* p;
//	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
	DWORD dwRead;
	//BYTE bSvrSock;	//服务器端的SOCK
	char szCmd[48];
	BYTE bBuf[128];

	GL868CloseListen(pModem);

    Sleep(4000);
	sprintf(szCmd, "AT+TCPLISTEN=%d\r\n", wLocalPort);

	//DTRACE(DB_FAPROTO, ("GL868Listen : tx %s", szCmd));
	if (CommWrite(pModem->bComm, (BYTE *)szCmd, strlen(szCmd), 1000) != strlen(szCmd)) 
		return false;

	Sleep(500);  //避免Read接收到以前的通信数据而立即返回
	dwRead = CommRead(pModem->bComm, bBuf, sizeof(bBuf), 2000);
	bBuf[dwRead] = 0;
	//DTRACE(DB_FAPROTO, ("GL868Listen : rx %s.\r\n", bBuf));
	
	pModem->pfnSpecHandle(pModem, (char* )bBuf, dwRead, 128); //对模块主动发过来的的数据进行特殊处理,如TCP/UDP报文、收到来自监听端口的新连接

	//+TCPLISTEN:0,OK    服务器侦听开始启动
	p = strstr((char* )bBuf, "+TCPLISTEN:");
	if (p == NULL)
	{
		DTRACE(DB_FAPROTO, ("GL868Listen: failed to listen\r\n"));
		return false;
	}
	
	p += 11;	//跳过+TCPLISTEN:

	if ((*p>='0' || *p<='3') && 
		*(p+1)==',' && *(p+2)=='O' && *(p+3)=='K')
	{
		return true;
	}
	else	//+TCPLISTEN:bind error  绑定失败
	{
		return false;
	}*/
}

//描述：关闭服务器已经连接的socket
bool GL868CloseSvrSock(struct TModem* pModem)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    return SocketClose(pGL868->pSocket);
    /*
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
	pGL868->bSvrSock = GL868_INVALID_SOCK;
	return ATCommand(pModem, "AT+CLOSECLIENT\r\n", "+CLOSECLIENT:", NULL, NULL, 3);*/
}

//描述：关闭监听
bool GL868CloseListen(struct TModem* pModem)
{/*
//	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
	GL868CloseSvrSock(pModem);

	//pGL868->bSvrSock = GL868_INVALID_SOCK;
	return ATCommand(pModem, "AT+CLOSELISTEN\r\n", "+CLOSELISTEN:", NULL, NULL, 3);*/
    
    return true;
}

//描述：作为服务器接收数据,不能直接返回数据，只能使用回调的方式
int GL868SvrReceive(struct TModem* pModem, BYTE* pbRxBuf, WORD wBufSize)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    return SocketReceive(pGL868->pSocket, pbRxBuf, wBufSize);
    
	//return M590CliReceive(pModem, pbRxBuf, wBufSize);
}

//作为服务器发送数据
int GL868SvrSend(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen)
{
    TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
    
    //终端协议栈
   	if (SocketSend(pGL868->pSocket, pbTxBuf, wLen))
        return wLen;
    
    return 0;
    /*
    static WORD wLastSnd=0;
    static DWORD dwLastSndTick = 0;
	if (pModem->bModuleVer == MODULE_ME590)
	{
		TM590* pM590 = (TM590* )pModem->pvModem;		//MODEM子类数据,指向具体实例
		WORD nStrLen;
	 	int iGprsRet = -1;
		char szCmd[32];
		char szAnsOK[32];
        char szAnsErr1[20];
		BYTE bBuf[10];
		
		if (pM590->bSvrSock == M590_INVALID_SOCK)	//服务器SOCK已经在使用
			return 0;
        
//        if (GetM590SendBufLeft(pModem, true) <= 0)
//            return 0;

        if (wLastSnd > 300)
        {
            while (GetTick()-dwLastSndTick<=15000)
            {
                if (GetM590SendBufLeft(pModem, true) > 1800) //2047为最大缓冲区
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

//检查服务器的socket连接是否依然有效：要主动发命令查询
bool GL868ChkSvrStatus(struct TModem* pModem)
{
    BYTE bTxBuf[2];
    bTxBuf[0] = 0x55;
    if (GL868SvrSend(pModem, bTxBuf, 1) == 0)  //发送一个字节测试链路是否有效。
        return false;
    else
        return true;
    
    /*
	TM590* pM590 = (TM590* )pModem->pvModem;	//MODEM子类数据,指向具体实例
	char szCmd[24];
	char szConnect[32];
	char szDisConnect[32];

	if (pM590->bSvrSock != M590_INVALID_SOCK)	//服务器SOCK已经在使用
	{			
		sprintf(szCmd, "AT+CLIENTSTATUS=%d\r\n", pM590->bSvrSock);
		sprintf(szConnect, "+CLIENTSTATUS:%d,CONNECT", pM590->bSvrSock);
		sprintf(szDisConnect, "+CLIENTSTATUS:%d,DISCONNECT", pM590->bSvrSock);

		if (ATCommand(pModem, szCmd, szConnect, szDisConnect, "+CLIENTSTATUS:Error", 3) > 0)
			return true;
		else
			return false;        
	}
	else	//服务器通道还没在使用
	{
		return false;
	}*/
}

bool GL868IsSvrAcceptOne(struct TModem* pModem)
{
	TGL868* pGL868 = (TGL868* )pModem->pvModem;		//MODEM子类数据,指向具体实例
	return pGL868->bSvrSock != GL868_INVALID_SOCK;
}

#endif //EN_PROIF_SVR		//支持服务器
