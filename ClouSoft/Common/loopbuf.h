/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LoopBuf.h
 * ժ    Ҫ�����ļ�ʵ��ѭ����������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��12��
 * ��    ע��
 *********************************************************************************************************/
#ifndef LOOPBUF_H
#define LOOPBUF_H
#include "Typedef.h"
#include "Comm.h"

#define CCT_LOOPBUF_SIZE 256	//ѭ���������Ĵ�С

typedef struct 
{
	WORD  m_wBufSize;
	WORD  m_wRxHead;
	WORD  m_wRxTail;
	BYTE*  m_pbLoopBuf;    //ѭ��������
}TLoopBuf;

bool LoopBufInit(TLoopBuf* pTLoopBuf);
void LoopBufPutToBuf(TLoopBuf* pTLoopBuf, BYTE* pbBuf, WORD wLen);
WORD LoopBufRxFromBuf(TLoopBuf* pTLoopBuf, BYTE* pbRxBuf, WORD wBufSize);
void LoopBufDeleteFromBuf(TLoopBuf* pTLoopBuf, WORD wLen);
WORD LoopBufGetBufLen(TLoopBuf* pTLoopBuf);
void LoopBufClrBuf(TLoopBuf* pTLoopBuf);

#endif //LOOPBUF_H
