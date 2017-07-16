/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Sftp.h
 * ժ    Ҫ�����ļ�ʵ����Sftp����Э��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 * ��    ע��
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