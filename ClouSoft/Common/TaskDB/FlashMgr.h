/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FlashMgr.h
 * 摘    要：Flash空间管理（数据存放规划、存放数据的结构定义）
 *			 提供读写Flash各部分的接口（包含三部分：固定存储空间的中间数据部分、固定存储空间的事件记录部分、动态分配部分）
 *
 * 当前版本：1.0.0
 * 作    者：吴若文
 * 完成日期：2011-03-23
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
 * 备    注：1、对Flash读写的进一步封装，应用程序要将中间数据保存在Flash中的时候需要在此定义具体结构等信息
 *			 2、本程序将Flash分为了三部分（固定存储空间的中间数据部分、固定存储空间的事件记录部分、动态分配部分）
 *				由于各部分存储的数据结构以及方式都有异，因此有不同的读写接口，应用需要严格按照相应的接口操作
************************************************************************************************************/
#pragma once
#ifndef _FLASHMGR_H
#define _FLASHMGR_H

#include "TypeDef.h"
#include "SysArch.h"
#include "MtrStruct.h"
#include "FileMgr.h"
#include "FlashIf.h"
#include "GbPro.h"

extern TSem	g_semExFlashBuf;//外部Flash读写信号量
extern BYTE	*g_ExFlashBuf;//外部Flash读写缓冲区
extern TSem    m_semTaskCfg;
//extern BYTE g_ExFlashBuf[EXSECT_SIZE];

#define INSECT_NUM	50	//32
//#define EXFLASH_SIZE	(4*1024*1024)	//4M

#define FLASH_SECT_NUM (EXFLASH_SIZE/EXSECT_SIZE)

//事件
#define MAX_ALR_LEN	(72+1)	//37是事件长度，1是指1个字节的校验

//升级程序保留的与片内Flash一样大小的空间 256K == 64个扇区
#define UDP_STR_SECT_OFFSET	0

//文件分配表起始位置的扇区偏移
#define MT_STR_SECT_OFFSET		0


// Flash空间中间数据保存区    数据定义   260K
#define TD_STR_SECT_OFFSET	0

// Flash空间事件记录保存区   数据结构定义 19072Byte
// 重要事件
//重要事件数据起始位置的扇区偏移
#define VA_STR_SECT_OFFSET		0	//上一块中间数据块占用了整数倍扇区


//普通事件块起始位置的扇区偏移
#define CA_STR_SECT_OFFSET		VITALALR_REC_SIZE%EXSECT_SIZE
// 普通事件,因为中间数据是整扇区的用的，所以重要事件的起始点也一定是整扇区头，利用这点计算偏移

#define ALRREC_SIZE		(EXSECT_SIZE*33)	
#define EVTREC_SIZE		(EXSECT_SIZE*5)

#define EXTPARA_SIZE    (EXSECT_SIZE*64)    //外部参数空间133922Bytes, 占用33个扇区, 现开64个扇区，预留31个扇区空间。

//告警类型
#define VIT_ALR		0x01
#define COM_ALR		0x02

#define EXC_DATA_OFFSET     4		//告警内容偏移(2 总笔数 + 2 当前笔数)
#define EXC_MAX_ALR_NUM     500		//告警记录最大笔数
#define EXC_MAX_EVENT_NUM   10		//事件记录最大笔数
#define ERR_MAX_ALR_NUM     10		//主动上报失败告警记录最大笔数

//文件分配表相关
#define FN_DAYSTAT			1						//日数据起始FN号
#define FN_MONSTAT			11						//月数据起始FN号
#define FN_CURSTAT			21						//曲线数据起始FN号
#define FN_COMSTAT			30						//普通任务数据起始FN号
#define FN_FWDSTAT			94						//中继任务数据起始FN号
#define FN_MAX				158						//最大的FN号
#define FAT_NUM				158						//文件分配表的个数

#define FAT_SIZE 			1024						//每个文件的分配表大小
#define FAT_FLG_SIZE 		1021						//分配表中用于大小
#define FAT_PER_SECT 	(EXSECT_SIZE/FAT_SIZE)		//每个扇区的文件分配表个数

#define FAT_TOTAL_SIZE		((FN_MAX+62)*FAT_SIZE)//0x37000		//总的文件分配表大小
								//文件分配表总长度为0x36C00，共占54个扇区又0xC00个字节，为其分配55个扇区	//216*1024 216个Fn，每个1024个字节-----同上
								//文件分配表要独立做出来，因为这一块不实行扇区校验，而是实行文件分配表校验

#define UPDINFO_LEN			28  //!!!!!!! 2014/09/12 和bootloader对应  

#define FADDR_UPDINFO		EXFLASH_ADDR
#define FADDR_UPDFW			(EXFLASH_ADDR+UPDINFO_LEN)	//24个字节的文件信息和标志位
#define FADDR_F1_INFO       (EXFLASH_ADDR+EXSECT_SIZE*127)//文件传输中F1所表示的信息以及相关自定义的信息
#define FADDR_FAT			(EXFLASH_ADDR+EXSECT_SIZE*128)	//简化后的计算公式
#define FADDR_PNTMP			(FADDR_FAT+FAT_TOTAL_SIZE)		//(FADDR_UPDFW+EXSECT_SIZE*64)	//简化后的计算公式
#define FADDR_ALRREC		(FADDR_PNTMP+EXSECT_SIZE*PN_NUM*8)    //告警记录
#define FADDR_EVTREC		(FADDR_ALRREC+ALRREC_SIZE)      //事件记录
#define FADDR_EXTYKPARA		(FADDR_EVTREC+EVTREC_SIZE)      //遥控拉合闸参数
#define FADDR_RPTFAILREC	(FADDR_EXTYKPARA+EXSECT_SIZE)   //主动上报失败记录
#define FADDR_EXTPARA		(FADDR_RPTFAILREC+EXSECT_SIZE)
#define FADDR_DYN			(FADDR_EXTPARA+EXTPARA_SIZE)

#define STA_SECT_NUM	(FADDR_DYN/EXSECT_SIZE)
#define DYN_SECT_NUM	((EXFLASH_ADDR+EXFLASH_SIZE-FADDR_DYN)/EXSECT_SIZE) //7741

#define DYN_SECT_START	(FADDR_DYN/EXSECT_SIZE)//451


#define EXFLASH_PARA_OFFSET	    FADDR_EXTPARA		//片外Flash扩展参数起始偏移地址

//----------------------------------------------------------------------------------
// 以下函数为FlashMgr为应用提供的接口

bool UpdFat();

//描述：上电时扫描Flash，获取全局文件分配表g_TdbCtrl.bGlobalFat，也就是扫描文件分配表
//参数：NONE
//返回：如果正确，返回true,否则返回false
bool FlashInit();

//描述：将g_ExFlashBuf设置为默认值
//参数：NONE
//返回：NONE
void MakeDefaultSect(BYTE *pbBuf, WORD wSize);

//描述：从外部Flash读取需要升级的程序数据
//参数：@*pbBuf 升级程序文件的数据buf，安全起见，pbBuf不允许使用g_ExFlashBuf
//		@nLen 本次读取的长度
//		@dwStartOffset 从离文件头dwStartOffset偏移的位置开始读取
//返回：如果正确读取到所需数据则返回true
//      否则返回flash
//注意：函数不检查长度，应用程序需要自己保证接收的缓冲区足够大
bool ReadUpdateProg(BYTE* pbBuf, DWORD nLen, DWORD dwStartOffset);

//描述：升级用的程序写到外部Flash里
//参数：@*pbBuf 写到外部Flash的程序的buffet
//		@nLen 本次写Flash的长度
//		@dwStartOffset 从离文件头dwStartOffset偏移的位置开始写Flash
//返回：如果正确写入数据则返回true
//      否则返回false
//注意：函数不检查长度，应用程序需要自己保证接收的缓冲区足够大
bool WriteUpdateProg(BYTE* pbBuf, DWORD nLen, DWORD dwStartOffset);

//描述：写升级程序的校验及添加升级标志
//参数：@wCrc16 crc校验值
//返回：如果校验正确返回true，否则返回falsh
//bool WriteUpdProgCrc(DWORD dwUpdLen, WORD wCrc16);
bool WriteUpdProgCrc(char *szPfile, DWORD dwUpdLen, WORD wCrc16);

//描述：清除测量点bPn的中间数据项
//参数：@wPn需要读取数据的测量点号，F10中配置的测量点号
//返回：如果正确清除了数据则返回true
//      否则返回false
bool ClrPnTmpData(BYTE bPn);

//描述：读取测量点wPn的bFileNum中间数据项
//参数：@bPn需要读取数据的测量点号，F10中配置的测量点号
//	    @*pPnTmp 测量点中间数据缓存
//		@wNum PnTmp的个数
//返回：如果正确读取到所需数据则返回true
//      否则返回false
//注意：函数不检查长度，应用程序需要自己保证接收的缓冲区足够大
bool ReadPnTmp(BYTE bPn, const TPnTmp* pPnTmp, WORD wNum);

//描述：保存测量点bPn的bFileNum中间数据项
//参数：@bPn需要读取数据的测量点号，F10中配置的测量点号
//	    @*pPnTmp 测量点中间数据缓存
//		@wNum PnTmp的个数
//返回：如果正确保存了数据则返回true
//      否则返回false
bool WritePnTmp(BYTE bPn, const TPnTmp* pPnTmp, WORD wNum);

//描述：读取bAlrType类型事件的事件指针
//参数：@bAlrType事件类型，普通事件or重要事件
//返回：如果正确读取，返回事件记录条数，否则返回相应的错误码（错误码是负数）
int GetAlrRecPtr(BYTE bAlrType);

//描述：保存bAlrType类型的事件pbBuf
//参数：@bAlrType事件类型，普通事件or重要事件
//返回：如果正确读取，返回事件记录条数，否则返回相应的错误码（错误码是负数）
int GetAlrRecNum(BYTE bAlrType);

//描述：读取bAlrType类型的事件pbBuf
//参数：@bAlrType事件类型，普通事件or重要事件
//		@bAlrPtr 需要读取的记录指针0~255
//	    @*pbBuf保存数据缓冲区
//返回：如果正确读取了数据则返回数据的长度
//      否则返回相应的错误码（都是小于零的负数）
//int ReadAlrRec(DWORD dwId, BYTE bPn, BYTE* pbBuf, WORD* wRdNum, WORD wTotal, WORD* wAlrLen);
int ReadOneAlr(DWORD dwAddr, BYTE* pbBuf, WORD wRdNum);

//描述：保存bAlrType类型的事件pbBuf
//参数：@bAlrType事件类型，普通事件or重要事件
//	    @*pbBuf保存数据缓冲区
//		@nLen 保存数据长度
//返回：如果正确保存了数据则返回保存数据的长度
//      否则返回相应的错误码（都是小于零的负数）
int WriteAlrRec(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);

//描述：保存电能表拉合闸的配置参数；备注：pbBuf为TYkCtrl的指针，
//		在读写的时候已单个结构体大小字节TYkCtrl读写
//参数：@bPn测量点
//		@*pbBuf为保存的数据
//返回：如果正确保存了数据，则返回保存数据的长度
//		否则返回相应的错误码（都是小于零的负数）
int ExFlashReadPnCtrlPara(WORD wPn, TYkCtrl *tYkCtrl);

//描述：保存电能表拉合闸的配置参数；备注：pbBuf为TYkCtrl的指针，
//		在读写的时候已单个结构体大小字节TYkCtrl读写
//参数：@bPn测量点
//		@*pbBuf为保存的数据
//		@nLen为保存的长度
//返回：如果正确保存了数据，则返回保存数据的长度
//		否则返回相应的错误码（都是小于零的负数）
int ExFlashWritePnCtrlPara(WORD wPn, TYkCtrl *tYkCtrl, WORD nLen);

//供外部使用的接口到此结束
//----------------------------------------------------------------------------------

//描述：检查g_ExFlashBuf
//参数：NONE
//返回：如果正确，返回true,否则返回false
bool CheckSect(BYTE *pbBuf, WORD wSize);

//描述：给g_ExFlashBuf上文件标志以及算校验
//参数：NONE
//返回：NONE
void MakeSect(BYTE *pbBuf, WORD wSize);

//描述：检查pbMacTab是否正确
//参数：@pbMacTab 需要检查的文件分配表
//返回：如果正确，返回true,否则返回false
bool CheckFat(BYTE *pbMacTab);

//描述：给pbMacTab上校验
//参数：@pbMacTab 需要计算校验的文件分配表
//返回：NONE
void MakeFat(BYTE *pbMacTab);

//描述：检查事件记录pbRec是否正确
//参数：@pbRec 需要计算校验的记录
//返回：如果正确，返回true,否则返回false
bool CheckAlrRec(BYTE *pbRec, DWORD dwLen);

//描述：给事件记录pbRec上校验
//参数：@pbRec需要计算校验的事件记录
//返回：NONE
void MakeAlrRec(BYTE *pbRec, DWORD dwLen);

//描述：检查历史记录pbRec是否正确
//参数：@pbRec 需要计算校验的记录
//		@nLen 历史记录的长度
//返回：如果正确，返回true,否则返回false
bool CheckData(BYTE *pbRec, WORD nLen);

//描述：给历史记录pbRec上校验
//参数：@pbRec需要计算校验的历史记录
//		@nLen 历史记录的长度
//返回：NONE
void MakeData(BYTE *pbRec, WORD nLen);


/*****************************************
 *  有扇区校验的Flash读写接口
******************************************/

//描述：读Flash
//参数：@dwAddr 写入的地址
//		@*pbFat 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashRdData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//描述：读Flash，无校验
//参数：@dwAddr 写入的地址
//		@*pbFat 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashRdDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//描述：写Flash
//参数：@dwAddr 写入的地址
//		@*pbFat 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashWrData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//描述：写Flash，无校验
//参数：@dwAddr 写入的地址
//		@*pbMacTab 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashWrDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//   Flash读完接口完毕
/************************************************************/

//描述：将TPnTmp的数据设置成0，出错时使用
//参数：@*pPnTmp测量点中间数据缓存
//		@wNum PnTmp的个数
//返回：无
void ClrPnTmp(const TPnTmp* pPnTmp, WORD wNum);

//描述：读取bFn的文件分配表到pbFat
//参数：@bFn 需要读取的文件分配表号
//	    @*pbFat 读出来文件分配表存放缓存
//返回：如果正确返回true，否则返回false
int ReadFat(BYTE bFn, BYTE *pbFat);

//描述：将bFn的文件分配表pbFat写入到Flash相应位置
//参数：@bFn 需要写入的文件分配表号
//	    @*pbFat 文件分配表缓存
//返回：如果正确返回true，否则返回false
bool WriteFat(BYTE bFn, BYTE *pbFat);

//描述：检查文件分配表是否有效
//参数：@*pbFat 文件分配表
//返回：如果分配表无效返回true，有效返回false
bool FatIsInValid(BYTE *pbFat);

//描述：清除bFn的文件分配表信息
//参数：@bFn 需要清除的文件分配表号
//返回：如果成功清除则返回true,否则返回false
bool ClrFat(BYTE bFn, BYTE* pbFat);

//描述：清除所有文件分配表信息
//参数：NONE
//返回：如果成功清除则返回true,否则返回false
bool ResetAllFat();

//描述：程序为MakeDataInSect服务，计算长度为wLen，内容全部为bDefault值的缓冲空间的校验值
//参数：@wLen 数据长度---指的有效数据长度，不换手校验
//		@bDefault 默认值
//返回：校验值，校验值为wLen个bDefault总和模余0xff的值取反
BYTE CalcChkSum(WORD wLen, BYTE bDefault);

//描述：将g_ExFlashBuf从wStart开始按每wPeriod长度添加校验，校验值添加在wPeriod位置
//参数：@wStart 起始位置
//		@wPeriod 每一段的长度
//返回：返回最后一段完整周期后剩余的空间
WORD MakeDataInSect(WORD wStart, WORD wPeriod, BYTE *pbBuf, WORD wSize);

//描述：清除pbFat所占用的Flash空间及信息,只负责清除Flash，不清除文件分配表
//参数：@bFn  需要清除的文件分配表所属的bFn
//		@pbFat 需要清除的文件分配表信息
//返回：如果正确，返回true,否则返回false
bool CleanFlash(BYTE bFn, BYTE *pbFat);

//描述：找到文件分配表pbFat首个动态分配空间的扇区偏移
//参数：@pbFat 文件分配表
//		@nSect 需要查找的第nSect个有效扇区
//返回：返回第1个动态分配扇区的偏移号,第1个扇区的偏移为0
WORD SchSectInFat(BYTE *pbFat, WORD nSect);

//描述：计算wSectOffset偏移的扇区地址
//参数：@wSectOffset 扇区偏移量,从0开始
//返回：如果正确计算，返回Flash地址，否则返回-1
DWORD SectionToPhysicalAddr(WORD wSectOffset);

//描述：对直抄1类测量点数据进行校验
WORD CheckPnData(const TPnTmp* pPnTmp, WORD wNum);

//描述：取得直抄1类数据导入测量点数据的大小
WORD GetPnDataSize(const TPnTmp* pPnTmp, WORD wNum);

//描述：直抄1类数据导入测量点数据
bool LoadPnData(WORD wPn, const TPnTmp* pPnTmp, WORD wNum);

#endif	//_FLASHMGR_H