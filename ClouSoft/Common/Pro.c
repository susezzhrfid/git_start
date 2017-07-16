/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Pro.c
 * 摘    要：本文件实现了通信协议基类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "Pro.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "SysDebug.h"

#define PRO_FRM_SIZE	128		//64

int ProRcvBlock(struct TPro* pPro, BYTE* pbBlock, int nLen)
{
	return 0;
}

bool ProHandleFrm(struct TPro* pPro)
{
	return true;
}

bool ProLogin(struct TPro* pPro)
{
	return true;
}

bool ProLogoff(struct TPro* pPro)
{
	return true;
}

bool ProBeat(struct TPro* pPro)
{
	return true;
}

bool ProAutoSend(struct TPro* pPro)
{
	return true;
}

bool ProIsNeedAutoSend(struct TPro* pPro)
{
	return false;
}

void ProLoadUnrstPara(struct TPro* pPro)	//装载非复位参数
{
}

void ProDoProRelated(struct TPro* pPro)	//做一些协议相关的非标准的事情
{
}

bool ProSend(struct TPro* pPro, BYTE* pbTxBuf, WORD wLen)
{
	struct TProIf* pIf = pPro->pIf;		//通信接口
#ifndef SYS_WIN
    TraceBuf(DB_FAFRM, "tx<---", pbTxBuf, wLen);
#endif
        
    DTRACE(DB_FAPROTO, ("ProSend: tx from %s, len=%d, click=%d\n", pIf->pszName, wLen, GetClick())); 
        
	return pIf->pfnSend(pIf, pbTxBuf, wLen);
}

int ProReceive(struct TPro* pPro, BYTE* pbRxBuf, WORD wBufSize)
{
	struct TProIf* pIf = pPro->pIf;		//通信接口
	return pIf->pfnReceive(pIf, pbRxBuf, wBufSize);
}
	
//描述：主动接收并处理帧
int ProRcvFrm(struct TPro* pPro)
{
	struct TProIf* pIf = pPro->pIf;		//通信接口
	int iRet = 0;
	int len = 0;
    WORD wOffset;
	BYTE bBuf[PRO_FRM_SIZE];
    BYTE i=0;
	BYTE bMode = 0;
    int nScanLen = 0;
	ReadItemEx(BN2, PN0, 0x2040, &bMode);  //todo

    for (i=0; i<128; i++)
    {
        len = pPro->pfnReceive(pPro, bBuf, PRO_FRM_SIZE-1);
        if (len > 0)
        {
            DTRACE(DB_FAPROTO, ("RcvFrm: rx from %s, len=%d\n", pIf->pszName, len)); 
#ifndef SYS_WIN
            TraceBuf(DB_FAFRM, "rx--->", bBuf, len);
#endif
            //找启动测试帧。启动测试帧是一个645帧。
			if (bMode != 0)
				DoTest(pPro, bBuf, len);
        }
    
        if (len <= 0)
        {
            #ifdef SYS_WIN
                Sleep(300); //如果不加,在VC socket方式下不睡眠会导致CPU利用率高达99%,原因待查
            #endif //SYS_WIN
            if (len < 0)
                return -1; //出错
            else
                return 0;  //没有数据
        }
        
        wOffset=0;
        while (len > 0)
        {
            nScanLen = pPro->pfnRcvBlock(pPro, bBuf+wOffset, len);	//组帧
            if (nScanLen > 0)   //成功组成一帧
            {
                pPro->pfnHandleFrm(pPro);   //帧处理
                
                len = len - nScanLen;
                wOffset += nScanLen;
                pIf->pfnOnRcvFrm(pIf);	//在通信协议收到正确帧时调用,主要更新链路状态,比如心跳等
                iRet = 1;                
            }
            else if (nScanLen < 0)   //不全的报文
            {
                break;
            }
        }
        if (iRet > 0)
            return iRet;        
    }
	
	return 0; //没有正确的帧
}

//描述：外部传入接收数据并处理帧
bool ProRxFrm(struct TPro* pPro, BYTE* pbRxBuf, int iLen)
{
	struct TProIf* pIf = pPro->pIf;		//通信接口
	bool fRet = false;
	while (iLen > 0)
	{
		int nScanLen = pPro->pfnRcvBlock(pPro, pbRxBuf, iLen);	//组帧
		if (nScanLen > 0)   //成功组成一帧
		{
			pPro->pfnHandleFrm(pPro);   //帧处理

			iLen = iLen - nScanLen;
			pIf->pfnOnRcvFrm(pIf);	//在通信协议收到正确帧时调用,主要更新链路状态,比如心跳等
			fRet = true;
		}
		else if (nScanLen < 0)   //不全的报文
		{
			break;
		}
	}

	return fRet;
}

void ProInit(struct TPro* pPro)
{
	//虚函数，需要实例化为具体协议的对应函数
	pPro->pfnRcvFrm = ProRcvFrm;		//接收帧
	pPro->pfnRxFrm = ProRxFrm;			//外部传入接收数据并处理帧
	pPro->pfnRcvBlock = ProRcvBlock;	//组帧
	pPro->pfnHandleFrm = ProHandleFrm;	//处理帧

	pPro->pfnLogin = ProLogin;			//登陆
	pPro->pfnLogoff = ProLogoff;			//登陆退出
	pPro->pfnBeat = ProBeat;			//心跳
	pPro->pfnAutoSend = ProAutoSend;		//主动上送
	pPro->pfnIsNeedAutoSend = ProIsNeedAutoSend;	//是否需要主动上送
	pPro->pfnLoadUnrstPara = ProLoadUnrstPara;	//装载非复位参数
	pPro->pfnDoProRelated = ProDoProRelated;	//做一些协议相关的非标准的事情
	pPro->pfnSend = ProSend;	//对pIf->pIfpfnSend()的二次封装，可重载成特殊函数
	pPro->pfnReceive = ProReceive; //对pIf->pfnReceive()的二次封装，可重载成特殊函数
}
