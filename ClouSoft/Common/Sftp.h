/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Sftp.h
 * 摘    要：本文件实现了Sftp下载协议
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef SFTP_H
#define SFTP_H

#include "TypeDef.h"

#define PATHNAME_LEN	32

#define FILE_ERR      0
#define BOOTLOADER    1
#define FILE_APP      2
#define PARA_CFG      3

typedef struct{
	bool 	m_IsFinish;
	WORD	m_wTxLen;
	WORD	m_wRxLen;
	TTime 	m_tmLastRecv;

	WORD	m_wBlockSize;
	WORD	m_wPermission;
	BYTE	m_bTransType;
	DWORD 	m_dwFileID;
	DWORD	m_dwFileSize;
	BYTE*   m_pbDataBuf;
	char  	m_szPathName[PATHNAME_LEN+1];
} TSftp;

extern TSftp g_tSftpInfo;

BYTE IsSftpFwUpdate();
void SftpInit();
bool SftpHandleFrm(BYTE* pbRx, BYTE* pbTx);
int SftpReadFirst(BYTE* pbRx, BYTE* pbTx);
int SftpReadNext(BYTE* pbRx, BYTE* pbTx);
int SftpWriteFirst(BYTE* pbRx, BYTE* pbTx);
int SftpWriteNext(BYTE* pbRx, BYTE* pbTx);
int SftpTransferFinish(BYTE* pbRx, BYTE* pbTx);
int SftpTransferCancel(BYTE* pbRx, BYTE* pbTx);
void SftpClear();


#endif //SFTP_H