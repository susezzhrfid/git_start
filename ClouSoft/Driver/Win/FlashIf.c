/*********************************************************************************************************
* Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�Flash.c
* ժ    Ҫ�����ļ���Ҫ���ڶ�FLASH�ӿڽ��з�װ
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2010-12-28
*********************************************************************************************************/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <direct.h>
#include <errno.h>
#include "TypeDef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "FlashIf.h"
#include "Trace.h"
#include "sysapi.h"
#include "SysDebug.h"
#include "FileMgr.h"

#ifdef SYS_WIN
#pragma warning(disable:4996)
#endif

BYTE* ParaAddrConv(DWORD dwLogical)
{
	return ((BYTE *)dwLogical);
}

int FlashWrSect(const char* pszPathName, DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	int iRet = dwSectSize;
	int f, iErr;
	TraceOut("FlashWrSect: wr %s [0x%08x~0x%08x) .\r\n", pszPathName, dwAddr, dwAddr+dwSectSize);

	f = open(pszPathName, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE);  	//windows�±���ʹ��O_BINARY,����������ĳ���Ϊ��,����ϵͳ��O_BINARYΪ��
	if (f < 0)
	{
		TraceOut("FlashWrSect: error : fail to open %s.\r\n", pszPathName);
		return -ERR_NOTEXIST;	//������
	}

	iErr = lseek(f, dwAddr, SEEK_SET);
	if (iErr == -1)
	{
		close(f);
		//unlink(pszPathName);   //ɾ��
		TraceOut("FlashWrSect: error : fail to lseek %s.\r\n", pszPathName);
		return -ERR_FAIL;
	}

	if (write(f, pbBuf, dwSectSize) != (int )dwSectSize)
	{
		TraceOut("FlashWrSect: error:  fail to write %s .\r\n", pszPathName);
		return -ERR_FAIL;
	}

	close(f);

	return iRet;
}

//����:�ڲ�FLASH�޶�������д
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	return FlashWrSect(USER_PARA_PATH"InFlash.dat", dwAddr, pbBuf, dwSectSize);
}

//����:�ⲿFLASH�޶�������д
int ExFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	return FlashWrSect(USER_PARA_PATH"ExFlash.dat", dwAddr, pbBuf, dwSectSize);
}

bool InFlashEraseSect(DWORD dwAddr)
{
	BYTE bBuf[INSECT_SIZE];
	memset(bBuf, 0xff, INSECT_SIZE);
	return FlashWrSect(USER_PARA_PATH"InFlash.dat", dwAddr, bBuf, INSECT_SIZE);
}

bool ExFlashEraseSect(DWORD dwAddr)
{
	BYTE bBuf[4096];
	memset(bBuf, 0xff, 4096);
	return FlashWrSect(USER_PARA_PATH"ExFlash.dat", dwAddr, bBuf, EXSECT_SIZE);
}

//����:�����ⲿFlash��ָ����
//������@dwBlockAddr ���������ʼ��ַ
//      @bBlockType  0-32K�Ŀ飬1-64K�Ŀ�
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashEraseBlock(DWORD dwBlockAddr, BYTE bBlockType)
{
	return 0;
}

int GetFileLen(char* pszPathName)
{
	int iFileSize = -1;
    int f = open(pszPathName, O_RDWR, S_IREAD|S_IWRITE); //, S_IREAD|S_IWRITE
	if (f >= 0)
	{
		iFileSize = lseek(f, 0, SEEK_END);
		close(f);
	}
	
	return iFileSize;
}

int FlashRd(const char* pszPathName, DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
	int iRet, iErr;
	int f = open(pszPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);  	//windows�±���ʹ��O_BINARY,����������ĳ���Ϊ��,����ϵͳ��O_BINARYΪ��
	if (f < 0)
	{
		TraceOut("FlashRd: error : fail to open %s.\r\n", pszPathName);
		return -ERR_NOTEXIST;	//������
	}

	iErr = lseek(f, dwAddr, SEEK_SET);
	if (iErr == -1)
	{
		close(f);
		//unlink(pszPathName);   //ɾ��
		TraceOut("FlashRd: error : fail to lseek %s.\r\n", pszPathName);
		return -ERR_FAIL;
	}

	iRet = read(f, pbBuf, dwLen);
	if (iRet != (int )dwLen)
	{
		if (iRet == 0)
		{
			memset(pbBuf, 0, dwLen);
			//iRet = dwLen;
		}
		else
		{
			TraceOut("FlashRd: error:  fail to read %s, err=%s .\r\n", pszPathName, strerror(errno));
		}
	}

	close(f);

	return iRet;
}

//����:�ڲ�FLASH���޶������Ķ�
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
	return FlashRd(USER_PARA_PATH"InFlash.dat", dwAddr, pbBuf, dwLen);
}

//����:�ⲿFLASH���޶������Ķ�
int ExFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
	return FlashRd(USER_PARA_PATH"ExFlash.dat", dwAddr, pbBuf, dwLen);
}

void ExFlashEraseChip()
{
	return unlink(USER_PARA_PATH"ExFlash.dat");
}

bool InFlashFormat()
{
	return unlink(USER_PARA_PATH"InFlash.dat");
}

//��ȡBOOTLOADER�İ汾��
BYTE GetBootVer(char *pcBuf, BYTE bBufSize)
{
    return 32;
}

//���������Ա�������ڲ�FLASH��������
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen)
{
	return false;
}