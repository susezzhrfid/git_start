/*********************************************************************************************************
* Copyright (c) 2010,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：Flash.h
* 摘    要：本文件主要用于对FLASH接口进行封装
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2010-12-28
*********************************************************************************************************/

#define IN_PARACFG_ADDR 0xF2900     //片内Flash参数配置文件地址

BYTE* ParaAddrConv(DWORD dwLogical);
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize); //内部FLASH限定扇区的写
int ExFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize);	//外部FLASH限定扇区的写
int ExFlashEraseBlock(DWORD dwBlockAddr, BYTE bBlockType);
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//内部FLASH不限定扇区的读
int ExFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//外部FLASH不限定扇区的读
bool ExFlashEraseSect(DWORD dwAddr);
bool InFlashEraseSect(DWORD dwAddr);
void ExFlashEraseChip();
bool InFlashFormat();
int GetFileLen(char* pszPathName);
BYTE GetBootVer(char *pcBuf, BYTE bBufSize);
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen);

