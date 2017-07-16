/*********************************************************************************************************
* Copyright (c) 2010,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：FileMgr.c
* 摘    要：本文件主要用于对文件管理接口进行封装
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2010-12-28
*********************************************************************************************************/
#include "TypeDef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "FileMgr.h"
#include "Trace.h"
#include "sysapi.h"
#include "DbConst.h"
#include "FlashIf.h"

#include "FlashMgr.h"
#include "SysDebug.h"

//文件扇区分配的说明：
//1、扇区的大小保持跟实际存储设备保持一致，分别为INSECT_SIZE(片内FLASH扇区大小)和EXSECT_SIZE(片外FLASH扇区大小)
//2、页的大小不管是片内FLASH还是片外FLASH，统一为256个字节
//3、对于大的文件按照扇区来分配，对于小于扇区大小一半的文件建议按照页来分配。
//4、每个文件都是按照扇区或者页的整数倍进行分配
//5、为了简化编程，对于按照页进行分配的文件，文件不能超过扇区的结尾

//文件读写的说明：
//1、对于一般的随机读文件，提供按照文件号和文件内部偏移的读
//2、对于写文件前的读，提供按照扇区号的整个扇区读，应用程序可以修改相应部分，其它部分保持不变，
//	 然后整个扇区写回FLASH


static const TFileCfg g_InFileCfg[] = {	//片内FLASH文件配置结构
//起始的扇区号	起始的页号		占用页或扇区个数
	{0,				-1,					1},		//0--测量点动态映射对照表文件(193*2+2)*2
	{1,				0,					2},		//1--终端保持参数，304
	{1,				2,					1},		//2--BANK1扩展参数，40	
	{2,				-1,					1},		//3--BANK10扩展参数，462
	{3,				-1,					1},		//4--BANK24扩展参数，523
	{4,				-1,					1},		//5--sect1终端参数 540
	{5,				-1,					13},	//6--sect4测量点参数 13115=13k,裕留3k //注意此处不能为16，否则上电会校验不过
	{21,			-1,					3},		//7--BN11数据   1386	
	{24,			0,					2},		//8--BANK25校准参数，328
};


TFileCfg g_ExFileCfg[] = {	//片外FLASH文件配置结构
//起始的扇区号	起始的页号		占用扇区个数
	{0,				-1,					1},		//掉电保存数据 135
	{1,				-1,					57},	//sect5终端扩展参数229597, 57个扇区  
	{58,            -1,                 2},     //存储身份认证任务
	{60,            -1,                 2},     //存储对时任务
	{62,            -1,                 1},     //存储处理表号情况
};

const TFileCfg* File2Cfg(WORD wFile)
{
	if (wFile < 0x80)
	{
		if (wFile >= sizeof(g_InFileCfg)/sizeof(TFileCfg))
			return NULL;

		return &g_InFileCfg[wFile];
	}
	else
	{
		wFile -= 0x80;
		if (wFile >= sizeof(g_ExFileCfg)/sizeof(TFileCfg))
			return NULL;

		return &g_ExFileCfg[wFile];
	}
}

//描述：是否是按页分配的文件
bool IsPageFile(WORD wFile)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	if (pFileCfg->nPage >= 0)	//按页分配
		return true;
	else							//按扇区分配
		return false;
}

//描述：取得文件起始偏移
int GetFileOffset(WORD wFile, DWORD dwSect)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return -ERR_DEV;	//设备有问题

	if (pFileCfg->nPage >= 0)	//按页分配
	{	
		dwSect = pFileCfg->nSect;	//就是它所在扇区
		return PAGE_SIZE*pFileCfg->nPage;	//返回文件起始偏移
	}
	else						//按扇区分配
	{	
		dwSect += pFileCfg->nSect;	//起始扇区后的第dwSect个扇区
		return 0;	//返回文件起始偏移
	}
}

//描述：读取文件所在的第dwSect个扇区
//参数：@dwSect 文件的第几个扇区，对于按页分配的，置为0
//		@piFileOff	用来返回文件起始偏移
//返回：如果正确返回0，错误则返回负数
int ReadFileSect(WORD wFile, DWORD dwSect, BYTE* pbData, int* piFileOff)
{
	int iRet;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return -ERR_DEV;	//设备有问题

	if (pFileCfg->nPage >= 0)	//按页分配
	{	
		dwSect = pFileCfg->nSect;	//就是它所在扇区
		*piFileOff = PAGE_SIZE*pFileCfg->nPage;	//返回文件起始偏移
	}
	else						//按扇区分配
	{	
		dwSect += pFileCfg->nSect;	//起始扇区后的第dwSect个扇区
		*piFileOff = 0;	//返回文件起始偏移
	}

	if (wFile < 0x80)
	{
		iRet = InFlashRd(INSECT_SIZE*dwSect, pbData, INSECT_SIZE);
		if (iRet == INSECT_SIZE)
			return 0;
		else if (iRet == -ERR_NOTEXIST)	//文件不存在
		{
			memset(pbData, 0, INSECT_SIZE);
			return 0;
		}
		else 
			return -ERR_DEV;	//设备有问题
	}
	else
	{
		iRet = ExFlashRd(EXSECT_SIZE*dwSect+EXFLASH_PARA_OFFSET, pbData, EXSECT_SIZE);	//外部FLASH不限定扇区的读
		if (iRet == EXSECT_SIZE)
			return 0;
		else if (iRet == -ERR_NOTEXIST)	//文件不存在
		{
			memset(pbData, 0, EXSECT_SIZE);
			return 0;
		}
		else 
			return -ERR_DEV;	//设备有问题
	}	
}

//描述：随机读文件，
//参数：@wFile 文件号
// 		@dwOffset 文件内部偏移，包含了文件标志和校验和
//		@pbData 接收缓冲
// 		@iLen 读取长度, -1表示按默认长度读，对于按页分配的读取整个文件，对于按扇区分配的读一个扇区
//备注：$因为读某个ID时是不需要校验的，而扇区读的时候又需要，这里只负责把指定地址的数据读出来，不负责校验，
//		校验交由应用处理
//		$文件读取分3种情况：
//		1、上电系统库扫描时，整个扇区读，dwOffset和dwLen跟扇区对齐
//		2、程序运行时，读某个ID，非扇区对齐地读，用本函数也可以
//		3、程序运行时，写某个ID，必须整个扇区先读出来，dwOffset和dwLen跟扇区对齐
bool readfile(WORD wFile, DWORD dwOffset, BYTE* pbData, int iLen)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	if (pFileCfg->nPage >= 0)	//对按页分配的，纠正起始偏移
		dwOffset += PAGE_SIZE*pFileCfg->nPage;

	if (iLen < 0)	//-1表示按默认长度读
	{
		if (pFileCfg->nPage >= 0)	//对于按页分配的读取整个文件
			iLen = PAGE_SIZE*pFileCfg->wSectNum;
		else						//对于按扇区分配的读一个扇区
			iLen = GetSectSize(wFile);
	}

	if (wFile < 0x80)
    {
		return InFlashRd(INSECT_SIZE*pFileCfg->nSect+dwOffset, pbData, iLen) == iLen;	//内部FLASH不限定扇区的读
    }
	else
    {
        dwOffset += EXFLASH_PARA_OFFSET;  //外部参数偏移
		return ExFlashRd(EXSECT_SIZE*pFileCfg->nSect+dwOffset, pbData, iLen) == iLen;	//外部FLASH不限定扇区的读
    }
}

//描述：写文件
//参数：@wFile 文件号
//		@pbData 要写的数据缓冲
// 		@dwSect 文件的第几个扇区，对于按页分配的，置为0
// 		@dwLen 写长度
//备注：$文件有效标志和校验由应用算好，本程序只负责把数据写到指定地址
//		$文件写必须整个扇区地写，dwOffset和dwLen跟扇区对齐，数据之前就有上层缓存好
bool writefile(WORD wFile, DWORD dwSect, BYTE* pbData)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	if (pFileCfg->nPage >= 0)	//对于按页分配的，文件的第几个扇区置为0
		dwSect = 0;

	if (wFile < 0x80)
    {
		return InFlashWrSect(INSECT_SIZE*pFileCfg->nSect+INSECT_SIZE*dwSect, pbData, INSECT_SIZE) == INSECT_SIZE;	//内部FLASH限定扇区的写
    }
	else
    {        
		return ExFlashWrSect(EXSECT_SIZE*pFileCfg->nSect+EXSECT_SIZE*dwSect+EXFLASH_PARA_OFFSET, pbData, EXSECT_SIZE) == EXSECT_SIZE;	//外部FLASH限定扇区的写
    }
}

//擦除片内文件
bool EraseInFile(WORD wFile)
{            
	DWORD dwUsedSize;
	WORD wSect, wSectNum;
    int iFileOff;
    int iRet;
    
    const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;
    
    if (wFile > 0x80)  //片外的可以用擦除命令
        return true;
        
    wSectNum = GetFileSectNum(wFile);	//取得文件保存的扇区数量
	dwUsedSize = GetSectUsedSize(wFile);//取得文件实际使用的扇区大小
    
    WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
    
    for (wSect=0; wSect<wSectNum; wSect++)
	{
		memset(g_bDBBuf, 0, DBBUF_SIZE);
		iRet = ReadFileSect(wFile, wSect, g_bDBBuf, &iFileOff);	//读取文件所在的第dwSect个扇区
		if (iRet < 0)
			goto ret_err;

		memset(g_bDBBuf+iFileOff, 0, dwUsedSize+2); //将文件或扇区校验也清掉
		//MakeFile(pBankCtrl->wFile, g_bDBBuf);	//给文件扇区加上文件标志和校验
		if (!writefile(wFile, wSect, g_bDBBuf))
			goto ret_err;
	}    

    SignalSemaphore(g_semDbBuf);
    return true;
    
ret_err:	
    SignalSemaphore(g_semDbBuf);
    DTRACE(DB_CRITICAL, ("fail to EraseFile.\r\n"));
    return false;
}

//擦除所有片内文件,不包括校准系数
bool EraseAllInFile(void)
{
    bool fRet = true;
	BYTE i;
	for (i=0; i<sizeof(g_InFileCfg)/sizeof(TFileCfg); i++)
    {
        if (i == FILE_BN25_PARA)  //交采参数
        {
            continue;
        }
        
        fRet &= EraseInFile(i);
    }
    return fRet;
}

bool EraseAllInSpace(void)
{
#ifndef SYS_WIN
	DWORD dwAddr = INPROG_ADDR;

	//WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
	memset(g_bDBBuf, 0, DBBUF_SIZE);
	while (dwAddr < INFLASH_END)
	{
		InFlashWrSect(dwAddr, g_bDBBuf, INSECT_SIZE);
		dwAddr += INSECT_SIZE;
	}
#endif //SYS_WIN
	//SignalSemaphore(g_semDbBuf);
	return true;
}

//描述：取得文件的直接读地址
//参数：@wFile 文件号
// 		@dwOffset 文件内部纯内容的偏移
BYTE* GetFileDirAddr(WORD wFile, DWORD dwOffset)
{
	DWORD dwSect, dwSectOff, dwUsedSize;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return NULL;

	dwUsedSize = GetSectUsedSize(wFile);//取得文件实际使用的扇区大小
	dwSect = dwOffset / dwUsedSize;
	dwSectOff = dwOffset % dwUsedSize;

	if (pFileCfg->nPage >= 0)	//对按页分配的，纠正起始偏移
		dwSectOff += PAGE_SIZE*pFileCfg->nPage;

	if (wFile < 0x80)
#ifndef SYS_WIN
	  	return ParaAddrConv(INSECT_SIZE*pFileCfg->nSect + INSECT_SIZE*dwSect + dwSectOff);
#else
		return (BYTE* )(INSECT_SIZE*pFileCfg->nSect + INSECT_SIZE*dwSect + dwSectOff);	//内部FLASH不限定扇区的读
#endif
	else
		return NULL;	//外部FLASH不限定扇区的读
}

//描述：取得文件保存的扇区大小
DWORD GetSectSize(WORD wFile)
{
	if (wFile < 0x80)
		return INSECT_SIZE;		//片内FLASH扇区大小
	else
		return EXSECT_SIZE;		//片外FLASH扇区大小
}


//描述：取得文件实际使用的扇区大小
DWORD GetSectUsedSize(WORD wFile)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return 0;

	if (pFileCfg->nPage >= 0)	//对按页分配的，纠正起始偏移
		return PAGE_SIZE*pFileCfg->wSectNum - 2;  //减去了校验
	else
		return GetSectSize(wFile) - 2;
}

//描述：取得文件保存的扇区数量
WORD GetFileSectNum(WORD wFile)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return 0;

	if (pFileCfg->nPage >= 0)	//按页分配
		return 1;
	else							//按扇区分配
		return pFileCfg->wSectNum;
}

//描述：检验文件全部或者某个扇区的合法性
//参数：@iFileOff 文件在pbFile中的偏移
//备注：假定文件都是单独读出来的，不是按照整个扇区读出来的
bool CheckFile(WORD wFile, BYTE* pbFile, int iFileOff)
{
	DWORD dwEnd, dwUsedSize;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	dwUsedSize = GetSectUsedSize(wFile);

	if (pFileCfg->nPage >= 0)	//按页分配
		dwEnd = iFileOff + PAGE_SIZE*pFileCfg->wSectNum;
	else							//按扇区分配
		dwEnd = GetSectSize(wFile);

	if (pbFile[dwEnd-2] != 0x55)
		return false;

	if (pbFile[dwEnd-1] != CheckSum(pbFile+iFileOff, (WORD)dwUsedSize))
		return false;

	return true;
}

//描述：给文件扇区加上文件标志和校验
//备注：假定文件都是按照整个扇区进行的
void MakeFile(WORD wFile, BYTE* pbFile)
{
	int iFileOff;
	DWORD dwEnd, dwUsedSize;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return;

	dwUsedSize = GetSectUsedSize(wFile);
	if (pFileCfg->nPage >= 0)	//按页分配
	{
		iFileOff = PAGE_SIZE*pFileCfg->nPage;
		dwEnd = PAGE_SIZE*pFileCfg->nPage + PAGE_SIZE*pFileCfg->wSectNum;
	}
	else							//按扇区分配
	{	
		iFileOff = 0;
		dwEnd = GetSectSize(wFile);
	}

	pbFile[dwEnd-2] = 0x55;
	pbFile[dwEnd-1] = CheckSum(pbFile+iFileOff, (WORD)dwUsedSize);
}

//备注：涉及到操作外部FLASH的，统一申请g_semExFlashBuf信号量，用g_ExFlashBuf
bool ReadPwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen)
{
	BYTE i;
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	for (i=0; i<2; i++)
	{
		if (readfile(FILE_PWROFF_TMP, 0, g_ExFlashBuf, -1))
		{
			if (CheckFile(FILE_PWROFF_TMP, g_ExFlashBuf, 0))
				break;
		}
	}

	if (i >= 2)
	{
		memset(pPwrOffTmp, 0, nLen);
		SignalSemaphore(g_semExFlashBuf);
		//InitPoweroffTmp();
		
		return false;
	}

	memcpy(pPwrOffTmp, g_ExFlashBuf, nLen);

	SignalSemaphore(g_semExFlashBuf);
	return true;
}

//备注：涉及到操作外部FLASH的，统一申请g_semExFlashBuf信号量
bool WritePwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen)
{
	BYTE i;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	memset(g_ExFlashBuf, 0, DBBUF_SIZE);
	memcpy(g_ExFlashBuf, pPwrOffTmp, nLen);

	MakeFile(FILE_PWROFF_TMP, g_ExFlashBuf);

	for (i=0; i<2; i++)
	{
		if (writefile(FILE_PWROFF_TMP, 0, g_ExFlashBuf))
		{
			break;
		}
	}

	SignalSemaphore(g_semExFlashBuf);
	if (i < 2)
		return true;
	else
		return false;
}
