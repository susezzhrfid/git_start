/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FapAPI.c
 * 摘    要：本文件主要包含FaProto目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "Pro.h"
#include "DrvCfg.h"
#include "ProIf.h"
#include "Modem.h"
#include "M590.h"
#include "CommIf.h"
#include "GprsIf.h"
#include "ProPara.h"
#include "ProAPI.h"
#include "GbPro.h"
#include "CommIf.h"
#include "SysDebug.h"
#include "ThreadMonitor.h"

//#include "socketIf.h"


//struct TProIf g_ifSock;	//Socket通信接口基类结构
TSocketIf g_SocketIf;	//Socket接口子类

struct TProIf g_ifGprs;	//GPRS通信接口基类结构
TGprsIf g_GprsIf;	//GPRS通信接口子类结构
struct TModem g_Modem;	//MODEM基类
TM590 g_M590;		//GC864子类

#define REMOTE_BUF_MAX			1860
#define LOCAL_BUF_MAX			1100

struct TProIf g_ifCommLocal;	//本地通信接口基类结构
TCommIf g_Comm232If;		//串行通信接口子类结构
TCommIf g_CommIrIf;		//串行通信接口子类结构


struct TPro g_proMaster;	//主站通信协议基类
TSgPro g_gbMaster;		//主站通信协议子类

struct TPro g_proLocal;		//本地维护通信协议基类
TSgPro g_gbLocal;			//本地维护通信协议子类

/*
struct TPro g_proLocalIr;		//本地红外口维护通信协议基类
TGbPro g_gbLocalIr;			//本地红外口维护通信协议子类*/

BYTE g_bRemoteRxBuf[REMOTE_BUF_MAX];		//远程通讯接收缓冲区
BYTE g_bRemoteTxBuf[REMOTE_BUF_MAX];		//远程通讯发送缓冲区

BYTE g_bLocalRxBuf[LOCAL_BUF_MAX];		//本地通讯接收缓冲区
BYTE g_bLocalTxBuf[LOCAL_BUF_MAX];		//本地通讯发送缓冲区

void InitMasterPro()
{
	//Socket接口子类
	memset(&g_SocketIf, 0, sizeof(g_SocketIf));
	//LoadSockPara(&g_SocketIf);

	//Socket通信接口基类结构
	//g_ifSock.pvIf = &g_SocketIf;
		
	//SocketIfInit(&g_ifSock);
	//SetMaxFrmBytes(&g_ifSock, REMOTE_BUF_MAX);
	
	//协议与接口的挂接	
	//g_ifSock.pPro = &g_proMaster;	
    //g_proMaster.pIf = &g_ifSock;	

	memset(&g_Modem, 0, sizeof(g_Modem));
	memset(&g_M590, 0, sizeof(g_M590));
	memset(&g_GprsIf, 0, sizeof(g_GprsIf));

	//MODEM基类参数
	g_Modem.bComm = COMM_GPRS;		//串口号
	g_Modem.pProIf = &g_ifGprs;	//指向GPRS接口的指针
	g_Modem.pvModem = &g_M590;		//MODEM子类数据,指向具体实例
	g_Modem.bModuleVer = MODULE_ME590;

	//GC864子类参数：无
#ifndef SYS_WIN
	M590Init(&g_Modem); //Wing 2014/07/31
	//GL868Init(&g_Modem);
#endif

    g_GprsIf.pSocketIf = &g_SocketIf;
    
	//GPRS通信接口子类结构
	LoadGprsPara(&g_GprsIf);
	g_GprsIf.pModem = &g_Modem;

	//GPRS通信接口基类结构
	g_ifGprs.pvIf = &g_GprsIf;

	GprsIfInit(&g_ifGprs);
	//SetMaxFrmBytes(&g_ifGprs, REMOTE_BUF_MAX);

	//协议与接口的挂接
    g_ifGprs.pPro = &g_proMaster;
	g_proMaster.pIf = &g_ifGprs;		

	//协议初始化
	g_proMaster.pvPro = &g_gbMaster;
	SgInit(&g_proMaster, g_bRemoteRxBuf, g_bRemoteTxBuf, false);   //由于这里没有初始化会引起死机。
}

//本地维护口协议初始化
void InitLocalPro()
{
    memset(&g_CommIrIf, 0, sizeof(g_CommIrIf));
	LoadLocalIrPara(&g_CommIrIf);
	g_ifCommLocal.pvIf = &g_CommIrIf;
	CommIfInit(&g_ifCommLocal);

	memset(&g_Comm232If, 0, sizeof(g_Comm232If));	
	LoadLocal232Para(&g_Comm232If);
    g_ifCommLocal.pvIf = &g_Comm232If;
	CommIfInit(&g_ifCommLocal);

	//SetMaxFrmBytes(&g_ifCommLocal, LOCAL_BUF_MAX);

	//协议与接口的挂接
	g_proLocal.pIf = &g_ifCommLocal;
	g_ifCommLocal.pPro = &g_proLocal;
	
	//协议初始化
	g_proLocal.pvPro = &g_gbLocal;
	SgInit(&g_proLocal, g_bLocalRxBuf, g_bLocalTxBuf, true);
}


void InitProto()
{
	InitMasterPro();

//#ifndef SYS_WIN
	InitLocalPro();    
//#endif
	
	SftpInit();
	TransFileInit();
}

void NewProThread()
{
	NewThread("STD", StdProThread, &g_proMaster, 768/*1024*/, THREAD_PRIORITY_NORMAL);  //1024
        
    //sys_thread_new("StdProThread",  (pdTASK_CODE)StdProThread, &g_proMaster, 1024, THREAD_PRIORITY_BELOW_NORMAL);//通信线程必须将线程句柄加入一超时链表,因此用LWIP提供的函数来创建
        
//#ifndef SYS_WIN
//    NewThread("LOCAL", LocalThread, &g_proLocal, 1024, THREAD_PRIORITY_BELOW_NORMAL);    
//#endif
} 

BYTE GetSignStrength()
{
#ifndef SYS_WIN
    return GetSign(&g_ifGprs);    
#else
	return 20;
#endif
}
        
    
//本地维护线程
bool DoLocalService(struct TPro* pPro, TCommIf* pCommIf)
{
    //struct TProIf* pIf = &g_ifCommLocal;
    struct TProIf* pIf = pPro->pIf;     //基类通道
	pIf->pvIf = pCommIf;		//先绑定实际通信接口
    
	pIf->pfnDoIfRelated(pIf);	//做一些各个接口相关的非标准的事情
						//比如非连续在线方式下,GPRS和SMS间的切换
	pPro->pfnDoProRelated(pPro);	//做一些协议相关的非标准的事情
	
	//接口状态机:休眠->复位(MODEM复位、PPP拨号)->传输(客户端状态机：连接->登陆->通信->主动断开->空闲)
	switch (pIf->bState)  //接口的状态机
	{
		case IF_STATE_DORMAN:  //休眠
			pIf->pfnDorman(pIf);
			break;
		
		case IF_STATE_RST:  //复位
			if (pIf->pfnReset(pIf) == IF_RST_OK)
			{
				pIf->pfnOnResetOK(pIf);
			}
			else
			{
				pIf->pfnOnResetFail(pIf);
			}
			break;
				
		case IF_STATE_TRANS:  //传输
			pIf->pfnTrans(pIf); //接收到的一帧,并已经对其进行处理
			break;
			
		default:
			DTRACE(DB_FAPROTO, ("StdProtoThread : enter unkown state!\n"));	
			Sleep(5000);
			break;
	}

	return true;
}


//描述:本地维护的标准通信线程(红外口和232维护口共用此线程)
void LocalThread()
{
//  struct TPro g_proLocal;		//本地维护通信协议基类
	//int iMonitorID;
	//struct TPro* pPro = (struct TPro* )pvArg;
	//struct TProIf* pIf = pPro->pIf;		//通信接口
	
	//DTRACE(DB_FAPROTO, ("LocalThread : if(%s) started!\n", pIf->pszName));
	//iMonitorID = ReqThreadMonitorID(pIf->pszName, 60*60);	//申请线程监控ID,更新间隔为一个小时

	//pIf->fExitDone = false;
	//while (1)
	{
        //if (IsDownSoft())            
        //    Sleep(10);
        //else
    		//Sleep(100);    //不需要Sleep,CommRead里已经有了

		//UpdThreadRunClick(iMonitorID);

        DoLocalService(&g_proLocal, &g_Comm232If);
        if (!IsDownSoft())    
            DoLocalService(&g_proLocal, &g_CommIrIf);
  		//if (pIf->fExit)
		//	break;
	}

	//ReleaseThreadMonitorID(iMonitorID);
	//pIf->fExitDone = true;
	
	return ;
}

BYTE GetGprsIfState(void)
{
    return g_ifGprs.bState;
}