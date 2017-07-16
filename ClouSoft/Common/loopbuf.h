/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LoopBuf.h
 * 摘    要：本文件实现循环缓冲区类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef LOOPBUF_H
#define LOOPBUF_H
#include "Typedef.h"
#include "Comm.h"

#define CCT_LOOPBUF_SIZE 256	//循环缓冲区的大小

typedef struct 
{
	WORD  m_wBufSize;
	WORD  m_wRxHead;
	WORD  m_wRxTail;
	BYTE*  m_pbLoopBuf;    //循环缓冲区
}TLoopBuf;

bool LoopBufInit(TLoopBuf* pTLoopBuf);
void LoopBufPutToBuf(TLoopBuf* pTLoopBuf, BYTE* pbBuf, WORD wLen);
WORD LoopBufRxFromBuf(TLoopBuf* pTLoopBuf, BYTE* pbRxBuf, WORD wBufSize);
void LoopBufDeleteFromBuf(TLoopBuf* pTLoopBuf, WORD wLen);
WORD LoopBufGetBufLen(TLoopBuf* pTLoopBuf);
void LoopBufClrBuf(TLoopBuf* pTLoopBuf);

#endif //LOOPBUF_H
