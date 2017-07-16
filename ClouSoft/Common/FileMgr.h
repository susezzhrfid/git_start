/*********************************************************************************************************
* Copyright (c) 2010,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：FileMgr.h
* 摘    要：本文件主要用于对文件管理接口进行封装
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2010-12-28
*********************************************************************************************************/
#ifndef FILEMGR_H
#define FILEMGR_H
#include "SysCfg.h"
#include "TypeDef.h"
#include "FaStruct.h"

//文件号定义:[0~0x80)片内FLASH
#define FILE_PNMAP		0		//测量点动态映射对照表文件
#define FILE_KEEP_PARA	1		//终端保持参数
#define FILE_BN1_PARA	2		//BANK1扩展参数
#define FILE_BN10_PARA	3		//BANK10扩展参数
#define FILE_BN24_PARA	4		//BANK24扩展参数
#define FILE_TERM_PARA	5		//终端参数
#define FILE_PN_PARA	6		//测量点参数
#define FILE_BN11_DATA	7		//BANK11中间数据
#define FILE_BN25_PARA	8		//校准参数

//文件号定义:[0x80~0xff]片外FLASH
#define FILE_PWROFF_TMP		0x80	//掉电变量
#define FILE_EXT_TERM_PARA	0x81	//终端扩展参数
#define FILE_BATTASK0       0x82    //身份认证任务
#define FILE_BATTASK1       0x83    //对时任务
#define FILE_BATTASK_STATUS 0x84    //处理表号情况
#define FILE_SCHMTR_STATUS 0x85    //搜表状态信息存储

#define FILE_NULL		0xffff	//空文件

typedef struct{
	int 	nSect;		//起始的扇区号
	int 	nPage;		//起始的页号，-1表示使用整个扇区；页每个256个字节
	WORD 	wSectNum;	//占用扇区个数：如果nPage>=0,则表示占用页个数；如果nPage<0,则表示占用扇区个数
}TFileCfg;	//文件配置结构

#ifdef SYS_WIN
#define EXFLASH_ADDR	0
#define EXFLASH_SIZE    0x2000000   //32M
#endif

#define INSECT_SIZE		1024	//2048	//片内FLASH扇区大小
#define EXSECT_SIZE		4096	//片外FLASH扇区大小

#define PAGE_SIZE	256		//子扇区大小
#define ALRSECT_SIZE	270		//告警存储单元 1（CS）+2（Pn）+ 4（ID） + 1（告警状态）+ 6（时间） +256

const TFileCfg* File2Cfg(WORD wFile);
bool IsPageFile(WORD wFile);	//是否是按页分配的文件
int GetFileOffset(WORD wFile, DWORD dwSect);
int ReadFileSect(WORD wFile, DWORD dwSect, BYTE* pbData, int* piFileOff);	//读取文件所在的第dwSect个扇区
bool readfile(WORD wFile, DWORD dwOffset, BYTE* pbData, int iLen);
bool writefile(WORD wFile, DWORD dwSect, BYTE* pbData);
BYTE* GetFileDirAddr(WORD wFile, DWORD dwOffset);	//取得文件的直接读地址

DWORD GetSectSize(WORD wFile);	//取得文件保存的扇区大小
DWORD GetSectUsedSize(WORD wFile);	//取得文件实际使用的扇区大小
WORD GetFileSectNum(WORD wFile);	//取得文件保存的扇区数量
bool CheckFile(WORD wFile, BYTE* pbFile, int iFileOff);		//检验文件全部或者某个扇区的合法性
void MakeFile(WORD wFile, BYTE* pbFile);	//给文件扇区加上文件标志和校验

bool ReadPwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen);
bool WritePwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen);



#endif //FILEMGR_H
