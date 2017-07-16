/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProThread.c
 * 摘    要：本文件主要实现接口的标准通信线程
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
//#include "FaAPI.h"
#include "FaCfg.h"
#include "ProAPI.h"
#include "Pro.h"
#include "SysArch.h" 
#include "ThreadMonitor.h"
#include "SysDebug.h"

//描述:接口的标准通信线程
TThreadRet StdProThread(void* pvArg)
{
	//DWORD dwClick;
	int iMonitorID;
	struct TPro* pPro = (struct TPro* )pvArg;
	struct TProIf* pIf = pPro->pIf;		//通信接口
	
	//DTRACE(DB_FAPROTO, ("StdProThread : If (%s) started!\n", pIf->pszName));
	
	iMonitorID = ReqThreadMonitorID(pIf->pszName, 60*60);	//申请线程监控ID,更新间隔为一个小时

	pIf->fExitDone = false;
	while (1)
	{
		Sleep(100);
		UpdThreadRunClick(iMonitorID);
		if (pIf->fExit)
			break;

		pIf->LoadUnrstPara(pIf); 	//非复位参数发生改变
		//pProto->LoadUnrstPara(); 

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
				if (pIf->pfnReset(pIf))
                {
					pIf->pfnOnResetOK(pIf);
                }
				else
                {
					pIf->pfnOnResetFail(pIf);
                }
				break;
					
			case IF_STATE_TRANS:  //传输
				//if (pIf->bIfType==IF_GPRS || pIf->bIfType==IF_SOCKET)
					//pPro->pfnAutoSend(pPro);

				pIf->pfnTrans(pIf); //接收到的一帧,并已经对其进行处理
				break;
				
			default:
				DTRACE(DB_FAPROTO, ("StdProtoThread : enter unkown state!\n"));	
                pIf->bState = IF_STATE_RST;  //直接复位
				//Sleep(5000);
				break;
		}
	}

	ReleaseThreadMonitorID(iMonitorID);
	pIf->fExitDone = true;

	return 0;
}
