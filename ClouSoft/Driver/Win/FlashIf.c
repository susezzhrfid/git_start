/*********************************************************************************************************
* Copyright (c) 2010,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：Flash.c
* 摘    要：本文件主要用于对FLASH接口进行封装
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2010-12-28
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

	f = open(pszPathName, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE);  	//windows下必须使用O_BINARY,否则读出来的长度为空,其它系统下O_BINARY为空
	if (f < 0)
	{
		TraceOut("FlashWrSect: error : fail to open %s.\r\n", pszPathName);
		return -ERR_NOTEXIST;	//不存在
	}

	iErr = lseek(f, dwAddr, SEEK_SET);
	if (iErr == -1)
	{
		close(f);
		//unlink(pszPathName);   //删除
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

//描述:内部FLASH限定扇区的写
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	return FlashWrSect(USER_PARA_PATH"InFlash.dat", dwAddr, pbBuf, dwSectSize);
}

//描述:外部FLASH限定扇区的写
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

//描述:擦除外部Flash的指定块
//参数：@dwBlockAddr 擦除块的起始地址
//      @bBlockType  0-32K的块，1-64K的块
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
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
	int f = open(pszPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);  	//windows下必须使用O_BINARY,否则读出来的长度为空,其它系统下O_BINARY为空
	if (f < 0)
	{
		TraceOut("FlashRd: error : fail to open %s.\r\n", pszPathName);
		return -ERR_NOTEXIST;	//不存在
	}

	iErr = lseek(f, dwAddr, SEEK_SET);
	if (iErr == -1)
	{
		close(f);
		//unlink(pszPathName);   //删除
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

//描述:内部FLASH不限定扇区的读
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
	return FlashRd(USER_PARA_PATH"InFlash.dat", dwAddr, pbBuf, dwLen);
}

//描述:外部FLASH不限定扇区的读
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

//读取BOOTLOADER的版本号
BYTE GetBootVer(char *pcBuf, BYTE bBufSize)
{
    return 32;
}

//本函数可以编程任意内部FLASH区域，慎用
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen)
{
	return false;
}