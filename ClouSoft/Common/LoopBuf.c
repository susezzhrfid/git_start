/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LoopBuf.c
 * ժ    Ҫ�����ļ�ʵ��ѭ����������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��12��
 * ��    ע��
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

//����:�Ѵ��ڽ��յ������ݷŵ�����ѭ����������ȥ
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
	if (pTLoopBuf->m_wRxHead != pTLoopBuf->m_wRxTail)   //������Ϊ���ջ��������ڵ���ѭ��������
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
			//��������ݵ�ѭ���ѭ��������ת����һ�㻺������˳��
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

		//ȡ�߽���ѭ���������е����ݺ�,��������ѭ����������ͷβָ��,
		//��RcvBlock()�����������ѵ���ŰѸ����ݴӻ�������ɾ��
	}
	else
	{
		return 0;
	}
}

//����:ɾ���Ѿ�ɨ�������
void LoopBufDeleteFromBuf(TLoopBuf* pTLoopBuf, WORD wLen)
{
	WORD wLeft = LoopBufGetBufLen(pTLoopBuf);
    if (wLen > wLeft)
        wLen = wLeft;
            
	pTLoopBuf->m_wRxTail = (pTLoopBuf->m_wRxTail + wLen) % pTLoopBuf->m_wBufSize;	
}

//����:���ѭ�����������е�����
void LoopBufClrBuf(TLoopBuf* pTLoopBuf)
{
	pTLoopBuf->m_wRxHead = 0;
	pTLoopBuf->m_wRxTail = 0;	
}
