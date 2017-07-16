/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LoopBuf.c
 * 摘    要：本文件实现循环缓冲区类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年12月
 * 备    注：
 *********************************************************************************************************/
#include "string.h"
#include "LoopBuf.h"
#include "Trace.h"
#include "SysDebug.h"
#include "FaCfg.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CLoopBuf

bool LoopBufInit(TLoopBuf* pTLoopBuf)
{
	pTLoopBuf->m_wRxHead = 0;
	pTLoopBuf->m_wRxTail = 0;	
	return true;
}

WORD LoopBufGetBufLen(TLoopBuf* pTLoopBuf)
{
	if (pTLoopBuf->m_wRxHead >= pTLoopBuf->m_wRxTail)
		return pTLoopBuf->m_wRxHead - pTLoopBuf->m_wRxTail;
	else
		return pTLoopBuf->m_wRxHead + pTLoopBuf->m_wBufSize - pTLoopBuf->m_wRxTail;
}

//描述:把串口接收到的数据放到接收循环缓冲区中去
void LoopBufPutToBuf(TLoopBuf* pTLoopBuf, BYTE* pbBuf, WORD wLen)
{
	WORD WBuffLen=0;
	if (wLen > pTLoopBuf->m_wBufSize)
	{
		DTRACE(DB_FAPROTO, ("CLoopBuf::PutToLoopBuf over,wLen>m_wBufSize.\r\n"));
		return;
	}
	WBuffLen=(pTLoopBuf->m_wRxHead+pTLoopBuf->m_wBufSize-pTLoopBuf->m_wRxTail)%pTLoopBuf->m_wBufSize;
	if (wLen+WBuffLen > pTLoopBuf->m_wBufSize)
	{
		DTRACE(DB_FAPROTO, ("CLoopBuf::PutToLoopBuf over,(wLen+WBuffLen)>m_wBufSize.\r\n"));
		return;
	}		

	if (pTLoopBuf->m_wRxHead+wLen <= pTLoopBuf->m_wBufSize)
	{
		memcpy(&pTLoopBuf->m_pbLoopBuf[pTLoopBuf->m_wRxHead], pbBuf, wLen);
	}
	else
	{
		memcpy(&pTLoopBuf->m_pbLoopBuf[pTLoopBuf->m_wRxHead], pbBuf, pTLoopBuf->m_wBufSize - pTLoopBuf->m_wRxHead);
		memcpy(pTLoopBuf->m_pbLoopBuf, pbBuf + pTLoopBuf->m_wBufSize - pTLoopBuf->m_wRxHead, wLen-(pTLoopBuf->m_wBufSize - pTLoopBuf->m_wRxHead));
	}

	pTLoopBuf->m_wRxHead += wLen;
	if (pTLoopBuf->m_wRxHead >= pTLoopBuf->m_wBufSize)
		pTLoopBuf->m_wRxHead -= pTLoopBuf->m_wBufSize;
}


WORD LoopBufRxFromBuf(TLoopBuf* pTLoopBuf, BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wRetLen;
	if (pTLoopBuf->m_wRxHead != pTLoopBuf->m_wRxTail)   //现在认为接收缓冲区大于等于循环缓冲区
	{
		if (pTLoopBuf->m_wRxHead > pTLoopBuf->m_wRxTail)
		{
			wRetLen = pTLoopBuf->m_wRxHead - pTLoopBuf->m_wRxTail;
			if (wRetLen > wBufSize)
				wRetLen = wBufSize;

			memcpy(pbRxBuf, &pTLoopBuf->m_pbLoopBuf[pTLoopBuf->m_wRxTail], wRetLen);
			return wRetLen;
		}
		else
		{
			//这里把数据的循序从循环缓冲区转换成一般缓冲区的顺序
			wRetLen = pTLoopBuf->m_wBufSize - pTLoopBuf->m_wRxTail;
			if (wRetLen >= wBufSize)
			{
				memcpy(pbRxBuf, &pTLoopBuf->m_pbLoopBuf[pTLoopBuf->m_wRxTail], wBufSize);
				return wBufSize;
			}
			else
			{
				memcpy(pbRxBuf, &pTLoopBuf->m_pbLoopBuf[pTLoopBuf->m_wRxTail], wRetLen);

				if (wRetLen + pTLoopBuf->m_wRxHead > wBufSize)
				{
					memcpy(pbRxBuf+wRetLen, pTLoopBuf->m_pbLoopBuf, wBufSize-wRetLen);
					return wBufSize;
				}
				else
				{
					memcpy(pbRxBuf+wRetLen, pTLoopBuf->m_pbLoopBuf, pTLoopBuf->m_wRxHead);
					return wRetLen + pTLoopBuf->m_wRxHead;
				}
			}
		}

		//取走接收循环缓冲区中的数据后,还不更新循环缓冲区的头尾指针,
		//等RcvBlock()函数真正消费掉后才把该数据从缓冲区中删除
	}
	else
	{
		return 0;
	}
}

//描述:删除已经扫描的数据
void LoopBufDeleteFromBuf(TLoopBuf* pTLoopBuf, WORD wLen)
{
	WORD wLeft = LoopBufGetBufLen(pTLoopBuf);
    if (wLen > wLeft)
        wLen = wLeft;
            
	pTLoopBuf->m_wRxTail = (pTLoopBuf->m_wRxTail + wLen) % pTLoopBuf->m_wBufSize;	
}

//描述:清除循环缓冲区现有的数据
void LoopBufClrBuf(TLoopBuf* pTLoopBuf)
{
	pTLoopBuf->m_wRxHead = 0;
	pTLoopBuf->m_wRxTail = 0;	
}
