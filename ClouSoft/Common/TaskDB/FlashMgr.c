/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FlashMgr.c
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
 * 版本信息:
---2011-03-14:---V1.01----吴若文---	
	1、
************************************************************************************************************/
#include <stdio.h>
#include "TypeDef.h"
#include "ComAPI.h"
#include "FlashMgr.h"
#include "Trace.h"
#include "FlashIf.h"
#include "DataManager.h"
#include "TaskDB.h"
#include "DbConst.h"
#include "LibDbAPI.h"
#include "SysDebug.h"
#include "CommonTask.h"

#define FM_VERSION			"1.0.1"

TSem	g_semExFlashBuf;//用于保护g_ExFlashBuf
TSem    m_semTaskCfg;
//BYTE g_ExFlashBuf[EXSECT_SIZE];

#pragma section = "NFCRAM"
BYTE *g_ExFlashBuf;//外部Flash读写缓冲区        这个缓存区定义在NFC RAM里

#if EXSECT_SIZE > 0x1000
#error:EXSECT_SIZE is beyond the size! 
#endif

BYTE *GetNfcRamAddr(void)
{
#ifdef SYS_WIN
	return (BYTE*)malloc(EXSECT_SIZE);
#else
    pmc_enable_periph_clk( 9 ) ;//ID_SMC
	return __section_begin("NFCRAM");
#endif
}

//描述：参数F39有更新，检查更新，修改相应的Flash动态配置
//参数：NONE
//返回：更改参数的个数
bool UpdFat()
{
	const TCommTaskCtrl* pTaskCtrl;
	WORD wNeedSects, wSectCnt;
	bool fRet = true;
	bool fFnSup;
    WORD i;
	BYTE bFn, bPos, bMask;
	BYTE bFnValidFlg[(FAT_NUM+7)/8];
	//BYTE bFat[FAT_SIZE];
    
    BYTE bOrder = 0;    
    //添加本参数主要原因是FLASH格式化太慢，台体测试过不去，只好每次打乱分配空间的顺序达到格式化的目的
    //用这种方法替代格式化并不严谨，但也是无奈之举。
    //任务库和数据库本来是相对独立的，因为格式化后会断电，这里必须要从数据库取一个参数。其弊端二。
    //有一个好处就是这样打乱顺序在一定程序上例似于均衡算法，延长了FLASH的寿命。
    if (ReadItemEx(BN24, PN0, 0x5025, &bOrder) < 0)//0-正序分配空间，1-反序分配空间        //todo:liyan
		return false;

	TdbWaitSemaphore();

	//1、扫描所有文件分配表，把OK的FN分配表置位到全局分配表，否则当不存在
	memset(g_TdbCtrl.bGlobalFat, 0x00, sizeof(g_TdbCtrl.bGlobalFat));
	memset(bFnValidFlg, 0, sizeof(bFnValidFlg));
	memset(g_TdbCtrl.bFat, 0, FAT_SIZE);  //有信号量保护可以借用，g_TdbCtrl.bFat

	for (bFn=1; bFn<=FN_MAX; bFn++)
	{
		pTaskCtrl = ComTaskFnToCtrl(bFn);
		if (pTaskCtrl==NULL && bFn<FN_COMSTAT)
			continue;

		if (ReadFat(bFn, g_TdbCtrl.bFat) != TDB_ERR_OK)
			continue;

		if (!CheckFat(g_TdbCtrl.bFat))
			continue;

		fFnSup = IsFnSupByPn(bFn);
		if (fFnSup)
			wNeedSects = CalcFnNeedSects(bFn);
		else
			wNeedSects = 0;

		wSectCnt = CalcuBitNum(g_TdbCtrl.bFat, FAT_FLG_SIZE);
		if (wSectCnt != wNeedSects)	//支持的FN扇区分配数量不等 || 不支持的FN扇区分配数量不等0
		{
			TdbFree(bFn, g_TdbCtrl.bFat);
			continue;
		}

		if (!fFnSup)
			continue;

		for (i=0; i<FAT_FLG_SIZE; i++)
		{
			if ((g_TdbCtrl.bGlobalFat[i] & g_TdbCtrl.bFat[i]) != 0x00)	//有重复分配的情况
				break;
		}

		if (i < FAT_FLG_SIZE) //有重复分配的情况
		{
			TdbFree(bFn, g_TdbCtrl.bFat);
			continue;
		}

		//以下完全正确
		for (i=0; i<FAT_FLG_SIZE; i++)
		{
			g_TdbCtrl.bGlobalFat[i] |= g_TdbCtrl.bFat[i];
		}

		bPos = (bFn-1)>>3;
		bMask = 1<<((bFn-1)&7);
		bFnValidFlg[bPos] |= bMask;
	}

	//2、为没分配成功的FN分配空间
	for (bFn=1; bFn<=FN_MAX; bFn++)
	{
		bPos = (bFn-1)>>3;
		bMask = 1<<((bFn-1)&7);

		if (IsFnSupByPn(bFn) && (bFnValidFlg[bPos] & bMask)==0) //没分配成功的FN
		{	
			pTaskCtrl = ComTaskFnToCtrl(bFn);
			if (pTaskCtrl==NULL && bFn<FN_COMSTAT)
				continue;

			if (!TdbMalloc(bFn, g_TdbCtrl.bFat, bOrder))
				fRet = false;            
		}
	}

	TdbSignalSemaphore();
        
	return fRet;
}


//描述：上电时扫描Flash，获取全局文件分配表g_TdbCtrl.bGlobalFat，也就是扫描文件分配表
//参数：NONE
//返回：如果正确，返回true,否则返回false
//总结：
bool FlashInit()
{
#ifdef SYS_WIN
	DWORD i;
	int iFileLen = GetFileLen(USER_PARA_PATH"InFlash.dat");
	if (iFileLen != INSECT_SIZE*INSECT_NUM)
	{
		for (i=0; i<INSECT_NUM; i++)
		{
			//if (InFlashRd(i*INSECT_SIZE, g_ExFlashBuf, INSECT_SIZE) != INSECT_SIZE)	//上电第一次读失败，即文件不存在，创建
			{
				InFlashEraseSect(i*INSECT_SIZE);
			}
		}
	}
	/*for (i=0; i<INSECT_NUM; i++)
	{
		if (InFlashRd(i*INSECT_SIZE, g_ExFlashBuf, INSECT_SIZE) != INSECT_SIZE)	//上电第一次读失败，即文件不存在，创建
		{
			InFlashEraseSect(i*INSECT_SIZE);
		}
	}*/

	//windows下需要先将文件初始化一遍
	iFileLen = GetFileLen(USER_PARA_PATH"ExFlash.dat");
	if (iFileLen != EXSECT_SIZE*FLASH_SECT_NUM)
	{
		for (i=0; i<FLASH_SECT_NUM; i++)
		{
			//if (ExFlashRd(i*EXSECT_SIZE, g_ExFlashBuf, EXSECT_SIZE) != EXSECT_SIZE)	//上电第一次读失败，即文件不存在，创建
			{
				ExFlashEraseSect(i*EXSECT_SIZE);
			}
		}
	}
	/*for (i=0; i<FLASH_SECT_NUM; i++)
	{
		if (ExFlashRd(i*EXSECT_SIZE, g_ExFlashBuf, EXSECT_SIZE) != EXSECT_SIZE)	//上电第一次读失败，即文件不存在，创建
		{
			ExFlashEraseSect(i*EXSECT_SIZE);
		}
	}*/
#endif

	g_ExFlashBuf = GetNfcRamAddr();
	g_semExFlashBuf = NewSemaphore(1, 1);	//信号量初始化
	m_semTaskCfg = NewSemaphore(1, 1);

	DTRACE(DB_TASKDB, ("Flash::Init OK! Ver %s\r\n", FM_VERSION));
	return true;
}

//描述：将g_ExFlashBuf设置为默认值
//参数：NONE
//返回：NONE
static void MakeDefaultSect(BYTE *pbBuf, WORD wSize)
{
	memset(pbBuf, 0xff, wSize);
	MakeSect(pbBuf, wSize);
}

//描述：程序为MakeDataInSect服务，计算长度为wLen，内容全部为bDefault值的缓冲空间的校验值
//参数：@wLen 数据长度---指的有效数据长度，不换手校验
//		@bDefault 默认值
//返回：校验值，校验值为wLen个bDefault总和模余0xff的值取反
BYTE CalcChkSum(WORD wLen, BYTE bDefault)
{
	WORD i;
	BYTE bValue = 0;

	for (i=0; i<wLen; i++)
	{
		bValue += bDefault;
	}

	return ~bValue;
}

//描述：将g_ExFlashBuf从wStart开始按每wPeriod长度添加校验，校验值添加在wPeriod位置
//参数：@wStart 起始位置
//		@wPeriod 每一段的长度
//返回：返回最后一段完整周期后剩余的空间
WORD MakeDataInSect(WORD wStart, WORD wPeriod, BYTE *pbBuf, WORD wSize)
{
	WORD wOffset = wStart;	//起始计算的偏移
	BYTE bChkSum = CalcChkSum(wPeriod-1, 0xee);

	while (wOffset <= wSize-2)
	{
		pbBuf[wOffset-1] = bChkSum;
		wOffset += wPeriod;
	}

	return wOffset-(wSize-2)-1;
}

//描述：清除pbFat所占用的动态Flash空间及信息,只负责清除Flash，不清除文件分配表
//参数：@bFn 需要清除的文件分配表所属的bFn
//		@pbFat 需要清除的文件分配表信息
//返回：如果正确，返回true,否则返回false
bool CleanFlash(BYTE bFn, BYTE *pbFat)
{
	BYTE bBit = 0;
	WORD wByte = 0;
	DWORD dwSect;

	for(wByte=0; wByte<FAT_FLG_SIZE; wByte++)
	{
		if (pbFat[wByte] == 0)
			continue;

		for (bBit=0; bBit<8; bBit++)
		{
			dwSect = (wByte<<3) + bBit;
			if (dwSect >= DYN_SECT_NUM)	//已经遍历完了动态分配区，该退出了
				return true;

			if (pbFat[wByte] & (0x01<<bBit)) //此扇区被占用
			{
				ExFlashEraseSect(FADDR_DYN+EXSECT_SIZE*dwSect);
			}
		}
	}
	
	return true;
}

//描述：检查g_ExFlashBuf
//参数：NONE
//返回：如果正确，返回true,否则返回false
static bool CheckSect(BYTE *pbBuf, WORD wSize)
{
	BYTE bCheckVal;
	bCheckVal = ~CheckSum(pbBuf, wSize-2);
	
	if (pbBuf[wSize-2]==0x55 && pbBuf[wSize-1]==bCheckVal)
		return true;
	
	return false;
}

//描述：给g_ExFlashBuf上文件标志以及算校验
//参数：NONE
//返回：NONE
static void MakeSect(BYTE *pbBuf, WORD wSize)
{
	pbBuf[wSize-2] = 0x55;
	pbBuf[wSize-1] = (BYTE)(~CheckSum(pbBuf, wSize-2));
	
	return;
}

//描述：检查pbFat是否正确
//参数：@pbFat 需要检查的文件分配表
//返回：如果正确，返回true,否则返回false
bool CheckFat(BYTE* pbFat)
{
	BYTE bCheckVal;
	bCheckVal = ~CheckSum(pbFat, FAT_SIZE-3);

	if ((pbFat[FAT_SIZE-2]==0x55) && (pbFat[FAT_SIZE-3]==bCheckVal))
		return true;
	else
		return false;
}

//描述：给pbFat上校验
//参数：@pbFat 需要计算校验的文件分配表
//返回：NONE
void MakeFat(BYTE *pbFat)
{
	pbFat[FAT_SIZE-3] = (BYTE)(~CheckSum(pbFat, FAT_SIZE-3));	//FAT的校验
	pbFat[FAT_SIZE-2] = 0x55;
	pbFat[FAT_SIZE-1] = 0x00;	//扇区的校验

	return;
}

//描述：检查事件记录pbRec是否正确
//参数：@pbRec 需要计算校验的记录
//返回：如果正确，返回true,否则返回false
bool CheckAlrRec(BYTE *pbRec, DWORD dwLen)
{
	return (pbRec[0] == ((BYTE)(~CheckSum(pbRec+1, dwLen-1))));
}

//描述：给事件记录pbRec上校验
//参数：@pbRec需要计算校验的事件记录
//返回：NONE
void MakeAlrRec(BYTE *pbRec, DWORD dwLen)
{
	pbRec[0] = (BYTE)(~CheckSum(pbRec+1, dwLen));

	return;
}

//描述：检查历史记录pbRec是否正确
//参数：@pbRec 需要计算校验的记录
//		@nLen 历史记录的长度，包括校验
//返回：如果正确，返回true,否则返回false
bool CheckData(BYTE *pbRec, WORD nLen)
{
	return (pbRec[nLen-1] == ((BYTE)(~CheckSum(pbRec, nLen-1))));
}

//描述：给历史记录pbRec上校验
//参数：@pbRec需要计算校验的历史记录
//		@nLen 历史记录的长度，包括校验
//返回：NONE
void MakeData(BYTE *pbRec, WORD nLen)
{
	pbRec[nLen-1] = (BYTE)(~CheckSum(pbRec, nLen-1));
}

//描述：计算wSectOffset偏移的扇区地址
//参数：@wSectOffset 扇区偏移量,从0开始
//返回：如果正确计算，返回Flash地址，否则返回-1
DWORD SectToPhyAddr(WORD wSect)
{
	return FADDR_DYN + wSect*EXSECT_SIZE;
}

//描述：计算pbFnMacTab分配表wByteOffset, bBitOffset偏移的扇区的地址
//参数：@wByteOffset 文件分配表的字节偏移
//		@bBitOffset 字节偏移下的位偏移
//返回：返回对应的Flash地址
DWORD FatToPhyAddr(WORD wByte, BYTE bBit)
{
	return SectToPhyAddr((wByte<<3)+bBit);
}

//描述：寻找pbFnMacTab表里dwAddr所在扇区的下一个有效扇区地址
//参数：@*pbFnMacTab 文件分配表
//	    @dwAddrSect 当前扇区地址，返回的是下一个有效扇区地址
//返回：如果正确查找，返回下一扇区的地址，否则返回0
WORD SchNextSect(BYTE *pbFat, WORD wSect)
{
	WORD wByte;
	BYTE bBit;

	//如果pbFnMacTab为空，表示是在固定存储空间内，不需要查找，直接返回Flash下一个扇区地址
	if (pbFat == NULL)
		return wSect+1;

	//以下是动态分配区
	if (wSect < DYN_SECT_START)
		return 0xffff;

	wSect++;	//从下一个扇区开始找
	wSect -= DYN_SECT_START;	//转换为动态分配的第一个扇区从0开始算
	wByte = wSect>>3;
	bBit = wSect&7;

	for (; wByte<FAT_FLG_SIZE; wByte++)
	{
		for (; bBit<8; bBit++)
		{
			if ((wByte<<3)+bBit >= DYN_SECT_NUM)	//已经遍历完了动态分配区，该退出了
				return 0xffff;

			if ((pbFat[wByte]&(0x01<<bBit)) != 0x00)
			{
				return (DYN_SECT_START+(wByte<<3)+bBit);
			}
		}

		bBit = 0;
	}

	//到这了，说明已经从dwAddrSect开始找完整个pbFnMacTab了，找不到，只好返回0
	return 0xffff;
}

//描述：写Flash
//参数：@dwAddr 写入的地址
//		@*pbMacTab 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashWrData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen, wLen0 = wLen;
	BYTE i, j;

	wSect = dwAddr / EXSECT_SIZE;
	dwSectOff = dwAddr % EXSECT_SIZE;
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    
	for (i=0; i<2; i++)	//最多跨一个扇区
	{
		//先读出整个扇区
		for (j=0; j<2; j++)
		{
			if (ExFlashRd(EXSECT_SIZE*wSect, g_ExFlashBuf, EXSECT_SIZE) != EXSECT_SIZE)
				continue;
			
			if (CheckSect(g_ExFlashBuf, EXSECT_SIZE)) //检查标志和校验
				break;
		}
		
		if (j >= 2)
		{
			MakeDefaultSect(g_ExFlashBuf, EXSECT_SIZE);
			//return TDB_ERR_FLS_RD_FAIL;
		}

		if (dwSectOff+wLen > EXSECT_SIZE-2)	//跨扇区
        {
            if (dwSectOff < EXSECT_SIZE - 2)
    			wCpyLen = EXSECT_SIZE - 2 - dwSectOff;
            else
                wCpyLen = 0;
        }
		else
			wCpyLen = wLen;

		memcpy(&g_ExFlashBuf[dwSectOff], pbBuf, wCpyLen);

		MakeSect(g_ExFlashBuf, EXSECT_SIZE);

		if (ExFlashWrSect(EXSECT_SIZE*wSect, g_ExFlashBuf, EXSECT_SIZE) < 0)
		{
			if (ExFlashWrSect(EXSECT_SIZE*wSect, g_ExFlashBuf, EXSECT_SIZE) < 0)
			{
                SignalSemaphore(g_semExFlashBuf);
				return TDB_ERR_FLS_WR_FAIL;
			}
		}

		wLen -= wCpyLen;
		if (wLen == 0)
        {
            SignalSemaphore(g_semExFlashBuf);
			return wLen0;
        }

		pbBuf += wCpyLen;
		wSect = SchNextSect(pbFat, wSect);
		if (wSect == 0xffff)
        {
            SignalSemaphore(g_semExFlashBuf);
			return TDB_ERR_FLS_WR_FAIL;
        }

		dwSectOff = 0;
	}
    
    SignalSemaphore(g_semExFlashBuf);

	return TDB_ERR_FLS_WR_FAIL;
}

//描述：写Flash，无校验
//参数：@dwAddr 写入的地址
//		@*pbMacTab 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashWrDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen, wLen0 = wLen;
	BYTE i, j;

	wSect = dwAddr / EXSECT_SIZE;
	dwSectOff = dwAddr % EXSECT_SIZE;
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    
	for (i=0; i<2; i++)	//最多跨一个扇区
	{
		//先读出整个扇区
		for (j=0; j<2; j++)
		{
			if (ExFlashRd(EXSECT_SIZE*wSect, g_ExFlashBuf, EXSECT_SIZE) == EXSECT_SIZE)
				break;
		}
		
		if (j >= 2)
		{
			MakeDefaultSect(g_ExFlashBuf, EXSECT_SIZE);
            SignalSemaphore(g_semExFlashBuf);
			return TDB_ERR_FLS_RD_FAIL;
		}

		if (dwSectOff+wLen > EXSECT_SIZE)	//跨扇区
			wCpyLen = EXSECT_SIZE - dwSectOff;
		else
			wCpyLen = wLen;

		memcpy(&g_ExFlashBuf[dwSectOff], pbBuf, wCpyLen);

		if (ExFlashWrSect(EXSECT_SIZE*wSect, g_ExFlashBuf, EXSECT_SIZE) < 0)
		{
			if (ExFlashWrSect(EXSECT_SIZE*wSect, g_ExFlashBuf, EXSECT_SIZE) < 0)
			{
                SignalSemaphore(g_semExFlashBuf);
				return TDB_ERR_FLS_WR_FAIL;
			}
		}

		wLen -= wCpyLen;
		if (wLen == 0)
        {
            SignalSemaphore(g_semExFlashBuf);
			return wLen0;
        }

		pbBuf += wCpyLen;
		wSect = SchNextSect(pbFat, wSect);
		if (wSect == 0xffff)
        {
            SignalSemaphore(g_semExFlashBuf);
			return TDB_ERR_FLS_WR_FAIL;
        }

		dwSectOff = 0;
	}
    
    SignalSemaphore(g_semExFlashBuf);

	return TDB_ERR_FLS_WR_FAIL;
}

//描述：读Flash
//参数：@dwAddr 写入的地址
//		@*pbFat 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashRdData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen;
	
	dwSectOff = dwAddr % EXSECT_SIZE;
	if (dwSectOff+wLen > EXSECT_SIZE-2)	//跨扇区
	{
		wSect = dwAddr / EXSECT_SIZE;
        if (dwSectOff < EXSECT_SIZE - 2)
    		wCpyLen = EXSECT_SIZE - 2 - dwSectOff;
        else
            wCpyLen = 0;

		if (ExFlashRd(dwAddr, pbBuf, wCpyLen) != wCpyLen)
			if (ExFlashRd(dwAddr, pbBuf, wCpyLen) != wCpyLen)
				return -1;

		wSect = SchNextSect(pbFat, wSect);
		if (wSect == 0xffff)
			return -1;

		if (ExFlashRd(EXSECT_SIZE*wSect, pbBuf+wCpyLen, wLen-wCpyLen) != wLen-wCpyLen)
			if (ExFlashRd(EXSECT_SIZE*wSect, pbBuf+wCpyLen, wLen-wCpyLen) != wLen-wCpyLen)
				return -1;
	}
	else
	{
		if (ExFlashRd(dwAddr, pbBuf, wLen) != wLen)
			if (ExFlashRd(dwAddr, pbBuf, wLen) != wLen)
				return -1;
	}

	return wLen;
}

//描述：读Flash，无校验
//参数：@dwAddr 写入的地址
//		@*pbFat 需要写的Fn的文件分配表，如果没有，则用NULL，该参数为NULL的时候连续写扇区
//	    @*pbBuf 需要写入Flash的数据内容，此pbBuf不允许使用g_ExFlashBuf做接口
//		@nLen 写入Flash的数据长度
//返回：如果正确写入，返回true,否则返回false
//注意：以扇区为操作对象，函数里必须要检查检验和标志
int ExFlashRdDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen;
	
	dwSectOff = dwAddr % EXSECT_SIZE;
	if (dwSectOff+wLen > EXSECT_SIZE)	//跨扇区
	{
		wSect = dwAddr / EXSECT_SIZE;
		wCpyLen = EXSECT_SIZE - dwSectOff;
		if (ExFlashRd(dwAddr, pbBuf, wCpyLen) != wCpyLen)
			if (ExFlashRd(dwAddr, pbBuf, wCpyLen) != wCpyLen)
				return -1;

		wSect = SchNextSect(pbFat, wSect);
		if (wSect == 0xffff)
			return -1;

		if (ExFlashRd(EXSECT_SIZE*wSect, pbBuf+wCpyLen, wLen-wCpyLen) != wLen-wCpyLen)
			if (ExFlashRd(EXSECT_SIZE*wSect, pbBuf+wCpyLen, wLen-wCpyLen) != wLen-wCpyLen)
				return -1;
	}
	else
	{
		if (ExFlashRd(dwAddr, pbBuf, wLen) != wLen)
			if (ExFlashRd(dwAddr, pbBuf, wLen) != wLen)
				return -1;
	}

	return wLen;
}



//描述：将TPnTmp的数据设置成0，出错时使用
//参数：@*pPnTmp测量点中间数据缓存
//		@wNum PnTmp的个数
//返回：无
void ClrPnTmp(const TPnTmp* pPnTmp, WORD wNum)
{
	WORD i;

	for (i=0; i<wNum; i++)
	{
		memset(pPnTmp->pbData, 0x00, pPnTmp->wLen);
		pPnTmp++;
	}
}

//描述：从外部Flash读取需要升级的程序数据
//参数：@*pbBuf 升级程序文件的数据buf，安全起见，pbBuf不允许使用g_ExFlashBuf
//		@nLen 本次读取的长度
//		@dwOffset 从离文件头dwStartOffset偏移的位置开始读取
//返回：如果正确读取到所需数据则返回true
//      否则返回flash
//注意：函数不检查长度，应用程序需要自己保证接收的缓冲区足够大
bool ReadUpdateProg(BYTE* pbBuf, DWORD nLen, DWORD dwOffset)
{
	DWORD dwAddr;

	dwAddr = FADDR_UPDFW + dwOffset;

	if (ExFlashRdDataNoChk(dwAddr, NULL, pbBuf, nLen) < 0)
	{
		return false;
	}

	return true;
}

//描述：升级用的程序写到外部Flash里
//参数：@*pbBuf 写到外部Flash的程序的buffet
//		@nLen 本次写Flash的长度
//		@dwStartOffset 从离文件头dwStartOffset偏移的位置开始写Flash
//返回：如果正确写入数据则返回true
//      否则返回false
//注意：函数不检查长度，应用程序需要自己保证接收的缓冲区足够大
bool WriteUpdateProg(BYTE* pbBuf, DWORD nLen, DWORD dwOffset)
{
	DWORD dwAddr;

	dwAddr = FADDR_UPDFW + dwOffset;
	
	if (ExFlashWrDataNoChk(dwAddr, NULL, pbBuf, nLen) < 0)
		return false;

	return true;
}

//描述：写升级程序的校验及添加升级标志
//参数：@wCrc16 crc校验值
//返回：如果校验正确返回true，否则返回falsh
bool WriteUpdProgCrc(char *szPfile, DWORD dwUpdLen, WORD wCrc16)
{
	DWORD dwAddr = FADDR_UPDINFO;
	BYTE bBuf[UPDINFO_LEN];	//升级程序信息缓冲区
	
	bBuf[0] = 0x55;
	bBuf[1] = 0xaa;
	memcpy(bBuf+2, (BYTE*)szPfile, 16);
	
	DWordToByte(dwUpdLen, &bBuf[18]);
	WordToByte(wCrc16, &bBuf[22]);
		
	if (ExFlashWrDataNoChk(dwAddr, NULL, bBuf, UPDINFO_LEN) < 0)
		return false;
	
	return true;
}

//描述：清除测量点bPn的bFileNum中间数据项
//参数：@wPn需要读取数据的测量点号，F10中配置的测量点号
//返回：如果正确清除了数据则返回true
//      否则返回false
bool ClrPnTmpData(BYTE bPn)
{
	BYTE bFlag;
	int iPn = -1;
	DWORD dwAddr = 0;

#if MTRPNMAP!=PNUNMAP
	if ((iPn=SearchPnMap(MTRPNMAP,bPn)) < 0)
		return false;
#else
	iPn = bPn;
#endif

    if (iPn > PN_VALID_NUM)
        return false;
		
	//判断文件头标志位
	if (ReadItemEx(BN24, PN0, 0x410a, &bFlag) < 0)
		return false;
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    
    dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//对应的4K空间的首地址
	memset(g_ExFlashBuf, 0, EXSECT_SIZE);
    
	g_ExFlashBuf[0] = bFlag;	//写版本号
	
	MakeSect(g_ExFlashBuf, EXSECT_SIZE);//添加文件标志和校验
	
	//改写完将扇区重新写入
	if (ExFlashWrSect (dwAddr, g_ExFlashBuf, EXSECT_SIZE) < 0)
	{
		if (ExFlashWrSect (dwAddr, g_ExFlashBuf, EXSECT_SIZE) < 0)
		{
			SignalSemaphore(g_semExFlashBuf);
			return false;
		}
	}
	SignalSemaphore(g_semExFlashBuf);
		
	return true;
}


//描述：读取测量点wPn的bFileNum中间数据项
//参数：@wPn需要读取数据的测量点号，F10中配置的测量点号
//	    @*pPnTmp 测量点中间数据缓存
//		@wNum PnTmp的个数
//返回：如果正确读取到所需数据则返回true
//      否则返回false
//注意：函数不检查长度，应用程序需要自己保证接收的缓冲区足够大
bool ReadPnTmp(BYTE bPn, const TPnTmp* pPnTmp, WORD wNum)
{
	BYTE bFlag;
	BYTE *pPtrTmp = g_ExFlashBuf;
	int iPn = -1;
	DWORD dwAddr = 0;
	const TPnTmp *pTmp = pPnTmp;
	DWORD dwOffset = 1;
	WORD i;
	
#if MTRPNMAP!=PNUNMAP
	if ((iPn=SearchPnMap(MTRPNMAP,bPn)) < 0)
		return false;
#else
	iPn = bPn;
#endif


    if (iPn > PN_VALID_NUM)
        return false;

	dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//对应的4K空间的首地址

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	if (ExFlashRd(dwAddr, g_ExFlashBuf, EXSECT_SIZE) < 0)
	{
		if (ExFlashRd(dwAddr, g_ExFlashBuf, EXSECT_SIZE) < 0)
		{
			SignalSemaphore(g_semExFlashBuf);
			return false;
		}
	}

	if (!CheckSect(g_ExFlashBuf, EXSECT_SIZE))
	{
		SignalSemaphore(g_semExFlashBuf);
		return false;
	}
			
	//判断文件头标志位
	if (ReadItemEx(BN24, PN0, 0x410a, &bFlag) < 0)
	{
		SignalSemaphore(g_semExFlashBuf);
		return false;
	}
	
	if (bFlag != g_ExFlashBuf[0])//版本不一致
	{
		SignalSemaphore(g_semExFlashBuf);
		return false;
	}    
	
	//拆分返回
	for (i=0; i<wNum; i++)
	{
		memcpy(pTmp->pbData, pPtrTmp+dwOffset, pTmp->wLen); //pPtrTmp指向g_ExFlashBuf
		dwOffset += pTmp->wLen;
		pTmp++;
	}	
    
    SignalSemaphore(g_semExFlashBuf);  //用完就应该还

	return true;
}

//描述：保存测量点bPn的bFileNum中间数据项
//参数：@wPn需要读取数据的测量点号，F10中配置的测量点号
//	    @*pPnTmp 测量点中间数据缓存
//		@wNum PnTmp的个数
//返回：如果正确保存了数据则返回true
//      否则返回false
bool WritePnTmp(BYTE bPn, const TPnTmp* pPnTmp, WORD wNum)
{
	BYTE bFlag;
	BYTE *pPtrTmp = g_ExFlashBuf;
	int iPn = -1;
	DWORD dwAddr = 0;
	const TPnTmp *pTmp = pPnTmp;
	DWORD dwOffset = 1, i;
	
#if MTRPNMAP!=PNUNMAP
	if ((iPn=SearchPnMap(MTRPNMAP,bPn)) < 0)
		return false;
#else
	iPn = bPn;
#endif

    if (iPn > PN_VALID_NUM)
        return false;
	
	dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//对应的4K空间的首地址

	//判断文件头标志位
	if (ReadItemEx(BN24, PN0, 0x410a, &bFlag) < 0)
	{
		DTRACE(DB_TASKDB, ("WritePnTmp: Read 0x410a fail!\r\n"));
		return false;
	}

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	g_ExFlashBuf[0] = bFlag;	//写版本号
	//一块块改写数据
	for (i=0; i<wNum; i++)
	{
		memcpy(pPtrTmp+dwOffset, pTmp->pbData, pTmp->wLen);
		dwOffset += pTmp->wLen;
		pTmp++;
	}
	
	MakeSect(g_ExFlashBuf, EXSECT_SIZE);//添加文件标志和校验
	
	//改写完将扇区重新写入
	if (ExFlashWrSect (dwAddr, g_ExFlashBuf, EXSECT_SIZE) < 0)
	{
		if (ExFlashWrSect (dwAddr, g_ExFlashBuf, EXSECT_SIZE) < 0)
		{
			SignalSemaphore(g_semExFlashBuf);
			DTRACE(DB_TASKDB, ("WritePnTmp: ExFlashWrSect fail!\r\n"));
			return false;
		}
	}
	SignalSemaphore(g_semExFlashBuf);
		
	return true;
}
#if 0
DWORD GetIdAlrLen(BYTE bIdx, BYTE bPnNum)
{
	DWORD dwLen = 0;
	dwLen = bPnNum * GetFmtARDLen(bIdx) * 10 ;//内容
	dwLen += bPnNum ;//笔数
	dwLen += bPnNum ;//校验
	return dwLen;
}

bool GetAlrAddr(DWORD dwId, BYTE bPn, DWORD* dwAddr, WORD* wAlrLen)
{
#define ALR_ADDR 0//张强提供地址位
	BYTE i;
	DWORD dwOffset = 0;
	bool fGetAddr = false;
	dwAddr = ALR_ADDR;
	for (i = 0;GetAlrID(i)!=0;i++)
	{
		if (GetAlrID(i) == dwId)
		{
			if(GetAlrbPnNum(i)== PN_NUM)
			{
				dwOffset += GetIdAlrLen(i, bPn) ;//内容
			}

			*wAlrLen = GetFmtARDLen(i);
			fGetAddr = true;
			break;
		}
		else
		{
			//测量点×该事件的长度×10（笔数）+每个Pn的1个字节的真实笔数+每个Pn的1个字节的校验位
			dwOffset += GetIdAlrLen(i, GetAlrbPnNum(i));
		}
	}

	if(!fGetAddr)
	{
		return fGetAddr;
	}

	*dwAddr += dwOffset/(EXSECT_SIZE-2)*EXSECT_SIZE + dwOffset%(EXSECT_SIZE-2);
	return true; 
}
#endif

//描述：保存bAlrType类型的事件pbBuf
//参数：@bAlrType事件类型，普通事件or重要事件
//	    @*pbBuf保存数据缓冲区
//		@nLen 保存数据长度
//返回：如果正确保存了数据则返回保存数据的长度
//      否则返回相应的错误码（都是小于零的负数）
int WriteAlrRec(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
	int iRet = 0;
	DWORD dwOffset = 0;
	DWORD dwAddress = 0;
	WORD wMaxNum = ERR_MAX_ALR_NUM;
	WORD wTotalNum = 0;
	WORD wALrPtr = 0;
	BYTE bBuf[EXC_DATA_OFFSET] = {0};

	dwAddress = dwAddr;
	//获取总共存储的笔数
	memset(bBuf, 0x00, EXC_DATA_OFFSET);
	if (ExFlashRdData(dwAddress, NULL, bBuf, EXC_DATA_OFFSET) < 0)
	{
		return -1;
	}

	if (FADDR_ALRREC==dwAddress)
		wMaxNum = EXC_MAX_ALR_NUM;//告警最大笔数
	else if (FADDR_RPTFAILREC==dwAddress)
		wMaxNum = ERR_MAX_ALR_NUM;//告警上报失败后存入的最大笔数
	else if (FADDR_EVTREC==dwAddress)
		wMaxNum = ERR_MAX_ALR_NUM;//事件最大笔数

	if(IsAllAByte(bBuf, INVALID_DATA, 2))
	{
		wTotalNum = 0;
	}
	else
	{
		wTotalNum = ByteToWord(bBuf);
		if(wTotalNum > wMaxNum)
			wTotalNum = 0;
	}
	if(IsAllAByte(bBuf+2, INVALID_DATA, 2))
	{
		wALrPtr = 0;
	}
	else
	{
		wALrPtr = ByteToWord(bBuf+2);
		if(wALrPtr > wMaxNum)
			wALrPtr = 0;
	}
	
	dwOffset =dwAddress%EXSECT_SIZE + (DWORD)wALrPtr*ALRSECT_SIZE + EXC_DATA_OFFSET;
	dwAddress = dwAddress/EXSECT_SIZE*EXSECT_SIZE;
	dwAddress += dwOffset/(EXSECT_SIZE-2)*EXSECT_SIZE + dwOffset%(EXSECT_SIZE-2);
	
	dwLen += 1;
	memset(pbBuf+dwLen, 0, ALRSECT_SIZE-dwLen);
	pbBuf[0] = 0;
	MakeAlrRec(pbBuf, ALRSECT_SIZE-1);
    
	TraceBuf(DB_DB, "WriteAlrRec pbBuf -> ", pbBuf, dwLen);
	if ((iRet=ExFlashWrData(dwAddress, NULL, pbBuf, ALRSECT_SIZE)) < 0)
	{
		return iRet;
	}

	memset(bBuf, 0x00, EXC_DATA_OFFSET);
	dwAddress = dwAddr;
	if(wTotalNum < wMaxNum)
		wTotalNum ++;
	wALrPtr = (wALrPtr+1)%wMaxNum;

	WordToByte(wTotalNum, bBuf);
	WordToByte(wALrPtr, bBuf+2);

	if ((iRet=ExFlashWrData(dwAddress, NULL, bBuf, EXC_DATA_OFFSET)) < 0)
	{
		return iRet;
	}

	return dwLen;
}

//描述：读取bAlrType类型的事件pbBuf
//参数：@bAlrType事件类型，普通事件or重要事件
//		@bAlrPtr 需要读取的记录指针0~255
//	    @*pbBuf保存数据缓冲区
//返回：如果正确读取了数据则返回数据的长度
//      否则返回相应的错误码（都是小于零的负数）
#if 0
int ReadAlrRec(DWORD dwId, BYTE bPn, BYTE* pbBuf, WORD* wRdNum, WORD wTotal, WORD* wAlrLen)
{

	//int iRet;
	DWORD dwAddr, dwOffset;
	WORD wTotalNum = 0;
	DWORD dwTepId = 0;
	WORD wPtr = 0;

	if(dwId != 0xE20001FF)
		dwAddr = FADDR_ALRREC;
	else
		dwAddr = FADDR_ALRREC;
	//获取总共存储的笔数
	if (ExFlashRdData(dwAddr, NULL, pbBuf, 4) < 0)
	{	
		return -1;
	}
	
	wTotalNum = ByteToWord(pbBuf);
	wPtr = ByteToWord(pbBuf+2);

	if(wTotalNum==0 || wTotalNum<*wRdNum || wTotalNum>wTotal || wPtr>wTotal)
	{	
		return -1;
	}

	
	while(*wRdNum < wTotalNum)
	{	
		if(wTotalNum >= wTotal)//防止读取无效数据
			wPtr = (wPtr+(*wRdNum)) % wTotal;
		else
			wPtr = *wRdNum;
		
		dwOffset = 4 + ALRSECT_SIZE*wPtr +dwAddr;
		(*wRdNum)++;
		if (ExFlashRdData(dwOffset, NULL, pbBuf, ALRSECT_SIZE) < 0)
		{	
			return -1;
		}

		dwTepId	= ((DWORD)(pbBuf[6] << 24) | (DWORD)(pbBuf[5] << 16) | (DWORD)(pbBuf[4] << 8) | (DWORD)pbBuf[3]);
		if(dwId!=0xE20001FF && dwId!=0xE200FFFF)
		{
			//判断PN
			if(ByteToWord(pbBuf+1) != bPn)
				continue;

			//判断ID	
			if(dwId != dwTepId)
				continue;
		}
		else
		{
			if(dwTepId == 0)
				continue;
			*wAlrLen = GetFmtARDLen(dwTepId);
		}

		//判断校验
		if (!CheckAlrRec(pbBuf, ALRSECT_SIZE))
		{		
			continue;
		}
		break;//都没问题，出货
	}

	if(*wRdNum >= wTotalNum)
		wRdNum = wTotal;

	return ALRSECT_SIZE;
}
#endif

int ReadOneAlr(DWORD dwAddr, BYTE* pbBuf, WORD wRdNum)
{
	DWORD dwOffset = 0;
	DWORD dwAddress = 0;

	dwAddress = dwAddr;
	dwOffset =dwAddress%EXSECT_SIZE + (DWORD)wRdNum*ALRSECT_SIZE + EXC_DATA_OFFSET;
	dwAddress = dwAddress/EXSECT_SIZE*EXSECT_SIZE;
	dwAddress += dwOffset/(EXSECT_SIZE-2)*EXSECT_SIZE + dwOffset%(EXSECT_SIZE-2);
	if (ExFlashRdData(dwAddress, NULL, pbBuf, ALRSECT_SIZE) < 0)
	{	
		return TDB_ERR_FLS_RD_FAIL;
	}
	//判断校验
	if (!CheckAlrRec(pbBuf, ALRSECT_SIZE))
	{		
		return TDB_ERR_DATA_CHKSUM;
	}
	return ALRSECT_SIZE;
}
//描述：保存电能表拉合闸的配置参数；备注：pbBuf为TYkCtrl的指针，
//		在读写的时候已单个结构体大小字节TYkCtrl读写
//参数：@bPn测量点
//		@*pbBuf为保存的数据
//返回：如果正确保存了数据，则返回保存数据的长度
//		否则返回相应的错误码（都是小于零的负数）
int ExFlashReadPnCtrlPara(WORD wPn, TYkCtrl *tYkCtrl)
{
	DWORD dwAddr, dwOffset;
	int iRet;
	BYTE bBuf[sizeof(TYkCtrl)+1] = {0};

	dwAddr = FADDR_EXTYKPARA;
	dwOffset = (DWORD )wPn * sizeof(TYkCtrl);
	dwAddr += dwOffset/(EXSECT_SIZE-2)*EXSECT_SIZE + dwOffset%(EXSECT_SIZE-2);

	if ((iRet=ExFlashRdData(dwAddr, NULL, bBuf, sizeof(TYkCtrl)+1)) < 0)
	{
		return iRet;
	}
	if (!CheckAlrRec(bBuf, sizeof(TYkCtrl) + 1))
		return TDB_ERR_DATA_CHKSUM;
	memcpy(tYkCtrl, bBuf + 1, sizeof(TYkCtrl));

	return iRet;
}

//描述：保存电能表拉合闸的配置参数；备注：pbBuf为TYkCtrl的指针，
//		在读写的时候已单个结构体大小字节TYkCtrl读写
//参数：@bPn测量点
//		@*pbBuf为保存的数据
//		@nLen为保存的长度
//返回：如果正确保存了数据，则返回保存数据的长度
//		否则返回相应的错误码（都是小于零的负数）
int ExFlashWritePnCtrlPara(WORD wPn, TYkCtrl *tYkCtrl, WORD nLen)
{
	DWORD dwAddr, dwOffset;
	BYTE bBuf[sizeof(TYkCtrl) + 1];
	int iRet;

	if (nLen > sizeof(TYkCtrl))
		return -1;

	dwAddr = FADDR_EXTYKPARA;
	dwOffset = (DWORD )wPn * sizeof(TYkCtrl);
	dwAddr += dwOffset/(EXSECT_SIZE-2)*EXSECT_SIZE + dwOffset%(EXSECT_SIZE-2);

	memset(bBuf, 0, sizeof(bBuf));
	memcpy(&bBuf[1], tYkCtrl, sizeof(TYkCtrl));

	MakeAlrRec(bBuf, sizeof(TYkCtrl));   //to do :待确认记录长度
	iRet = ExFlashWrData(dwAddr, NULL, bBuf, sizeof(TYkCtrl)+1);

	return iRet;
}

//描述：读取bFn的文件分配表到pbFat，只负责读出来，不判断有效性
//参数：@bFn 需要读取的文件分配表号
//	    @*pbFat 读出来文件分配表存放缓存
//返回：如果正确读取则返回TDB_ERR_OK,否则返回相应错误码
//注意：每个文件分配表都有校验，为了使一个扇区能够存放整数倍个文件分配表，现将文件分配表设计成
//		1、文件分配表的第0位最低字节对应于FLASH的起始动态空间，而不是原来的FLASH首地址空间
//		2、目前静态空间一共占用了152个扇区共19个字节的文件分配表区，也就是说文件分配表的最后19个字节是无效的
//		   现使用最后的3个字节作文件分配表及扇区校验用三个字节FAT_SIZE-3，FAT_SIZE-2，FAT_SIZE-1
//		   分别对应于文件分配表校验位，扇区文件标记位(0x55)，以及扇区校验位(如果该文件分配表不是在扇区最后一个位置，则此位为0)
int ReadFat(BYTE bFn, BYTE* pbFat)
{
	DWORD dwAddr = FADDR_FAT + (DWORD )FAT_SIZE*(bFn-1);
	BYTE i;
	bool fRdOK = false;

	if (bFn > FN_MAX)
		return TDB_ERR_FN;

	for (i=0; i<2; i++)
	{
		if (ExFlashRd(dwAddr, pbFat, FAT_SIZE) < 0)
			continue;
	
		if (CheckFat(pbFat))
			return TDB_ERR_OK;
		else
			fRdOK = true;
	}

	if (fRdOK)
	{
		//DTRACE(DB_TASKDB, ("ReadFat: Read FN%d file table chksum fail!\r\n", bFn));  //todo:不支持的FN，没有分配空间
		return TDB_ERR_MACT_CHKSUM;		//读上来了，但是校验不对
	}
	else
	{
		DTRACE(DB_TASKDB, ("ReadFat: Read FN%d file table flash fail!\r\n", bFn));
		return TDB_ERR_FLS_RD_FAIL;		//读都读不上来
	}
}

//描述：将bFn的文件分配表pbFat写入到Flash相应位置
//参数：@bFn 需要写入的文件分配表号
//	    @*pbFat 文件分配表缓存
//返回：如果正确返回true，否则返回false
//注意：每个文件分配表都有校验，为了使一个扇区能够存放整数倍个文件分配表，现将文件分配表设计成
//		1、文件分配表的第0位最低字节对应于FLASH的起始动态空间，而不是原来的FLASH首地址空间
//		2、目前静态空间一共占用了152个扇区共19个字节的文件分配表区，也就是说文件分配表的最后19个字节是无效的
//		   现使用最后的3个字节作文件分配表及扇区校验用三个字节FAT_SIZE-3，FAT_SIZE-2，FAT_SIZE-1
//		   分别对应于文件分配表校验位，扇区文件标记位(0x55)，以及扇区校验位(如果该文件分配表不是在扇区最后一个位置，则此位为0)
bool WriteFat(BYTE bFn, BYTE* pbFat)
{
	DWORD dwOffset, dwSectAddr;	//MALLOC_TAB_ADDR不是一个扇区的首地址
	BYTE i;
		
	if (bFn > FN_MAX)
		return false;
	
	dwSectAddr = FADDR_FAT + (bFn-1)/FAT_PER_SECT*EXSECT_SIZE;
	dwOffset = FAT_SIZE * ((bFn-1)%FAT_PER_SECT);
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	for (i=0; i<2; i++)
	{
		//先把分配表所在扇区数据读上来
		if (ExFlashRd(dwSectAddr, g_ExFlashBuf, EXSECT_SIZE) > 0)
		{
			if (CheckSect(g_ExFlashBuf, EXSECT_SIZE))
				break;
		}
	}

	if (i >= 2)
	{
		MakeDefaultSect(g_ExFlashBuf, EXSECT_SIZE);	//校验出错了，扇区清除
		//return false;
	}

	//完了就改数据
	memcpy(&g_ExFlashBuf[dwOffset], pbFat, FAT_SIZE);

	MakeSect(g_ExFlashBuf, EXSECT_SIZE);

	//改写完将数据写回Flash
	for (i=0; i<2; i++)
	{
		if (ExFlashWrSect(dwSectAddr, g_ExFlashBuf, EXSECT_SIZE) > 0)
        {
            SignalSemaphore(g_semExFlashBuf);
			return true;
        }
	}
    
    SignalSemaphore(g_semExFlashBuf);

	return false;
}

//描述：检查文件分配表是否有效
//参数：@*pbFat 文件分配表
//返回：如果分配表无效返回true，有效返回false
bool FatIsInValid(BYTE *pbFat)
{
	WORD i;
	for (i=0; i<FAT_FLG_SIZE; i++)
	{
		if (pbFat[i] != 0x00)
			return false;
	}

	return true;
}

//描述：清除bFn的文件分配表信息
//参数：@bFn 需要清除的文件分配表号
//参数：@pbBuf 缓存区由外部传进来
//参数：@wBufSize缓存区的大小
//返回：如果成功清除则返回true,否则返回false
bool ClrFat(BYTE bFn, BYTE* pbFat)
{          
	memset(pbFat, 0x00, FAT_SIZE);

	MakeFat(pbFat);
	
	if (!WriteFat(bFn, pbFat))
		return false;
	
	return true;
}

//描述：清除所有文件分配表信息
//参数：NONE
//返回：如果成功清除则返回true,否则返回false
bool ResetAllFat()
{
	BYTE bFn;
	//BYTE bFat[FAT_SIZE];
	const TCommTaskCtrl* pTaskCtrl;
	
	TdbWaitSemaphore();
	for (bFn=1; bFn<=FN_MAX; bFn++)
	{
		pTaskCtrl = ComTaskFnToCtrl(bFn);
		if (pTaskCtrl == NULL)
			continue;

		if (ReadFat(bFn, g_TdbCtrl.bFat) != TDB_ERR_OK)
			continue;

		if (!CheckFat(g_TdbCtrl.bFat))
			continue;
		
		if (FatIsInValid(g_TdbCtrl.bFat))	//释放过的bFn文件分配表是可以通过CheckFat的，但内容全为0
			continue;
		
		ClrFat(bFn, g_TdbCtrl.bFat);
	}
	
	TdbSignalSemaphore();
	return true;
}


//描述：找到文件分配表pbFat首个动态分配空间的扇区偏移
//参数：@pbFat 文件分配表
//		@nSect 需要查找的第wSect个有效扇区
//返回：返回第1个动态分配扇区的偏移号,第1个扇区的偏移为0
WORD SchSectInFat(BYTE *pbFat, WORD wSect)
{
	WORD wIdx = 0;	//扇区索引数
	WORD wByte;
	BYTE bBit;

	for (wByte=0; wByte<FAT_FLG_SIZE; wByte++)
	{
		if (pbFat[wByte] == 0)
			continue;

		for (bBit=0; bBit<8; bBit++)
		{
			if ((wByte<<3)+bBit >= DYN_SECT_NUM)	//已经遍历完了动态分配区，该退出了
				return 0xffff;

			if (pbFat[wByte] & (0x01<<bBit))
			{
				wIdx++;		//扇区索引数加1
				if (wIdx == (wSect+1))
					return (wByte<<3)+bBit;	//找到就返回扇区偏移
				//else
				//	wIdx++;		//扇区索引数加1
			}
		}
	}

	return 0xffff;
}


//描述：对直抄1类测量点数据进行校验
WORD CheckPnData(const TPnTmp* pPnTmp, WORD wNum)
{
	WORD i;
	BYTE bSum = 0;

	for (i=0; i<wNum; i++)		//直抄要导入的测量点数据项目个数
	{
		bSum += CheckSum(pPnTmp[i].pbData, pPnTmp[i].wLen);
	}

	return 0x5500 + bSum;
}

//描述：取得直抄1类数据导入测量点数据的大小
WORD GetPnDataSize(const TPnTmp* pPnTmp, WORD wNum)
{
	WORD i;
	WORD wSize = 0;
	for (i=0; i<wNum; i++)		//直抄要导入的测量点数据项目个数
	{
		wSize += pPnTmp[i].wLen;
	}

	return wSize;
}

//描述：直抄1类数据导入测量点数据
//参数：@wPn需要读取数据的测量点号，F10中配置的测量点号
//	    @*pPnTmp 测量点中间数据缓存
//		@wNum PnTmp的个数
//返回：如果正确读取到所需数据则返回true
//      否则返回false
bool LoadPnData(WORD wPn, const TPnTmp* pPnTmp, WORD wNum)
{
	int iPn = -1;
	DWORD dwAddr = 0;
	DWORD dwOffset = 1;
	WORD i;
	WORD wPnDataSize = GetPnDataSize(pPnTmp, wNum) + 1;

#if MTRPNMAP!=PNUNMAP
	if ((iPn=SearchPnMap(MTRPNMAP, wPn)) < 0)
		return false;
#else
	iPn = wPn;
#endif


	if (iPn > PN_VALID_NUM)
		return false;

	dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//对应的4K空间的首地址

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	if (ExFlashRd(dwAddr, g_ExFlashBuf, wPnDataSize) < 0)
	{
		if (ExFlashRd(dwAddr, g_ExFlashBuf, wPnDataSize) < 0)
		{
			SignalSemaphore(g_semExFlashBuf);
			return false;
		}
	}

	//拆分返回
	for (i=0; i<wNum; i++)
	{
		memcpy(pPnTmp[i].pbData, &g_ExFlashBuf[dwOffset], pPnTmp[i].wLen);
		dwOffset += pPnTmp[i].wLen;
	}
	SignalSemaphore(g_semExFlashBuf);

	return true;
}

//本函数替代ExFlashEraseChip，目的是节约时间，
bool ClrAllData(void)
{
    //格FAT分配表
    /*FADDR_FAT;
    FADDR_PNTMP;
    FADDR_VITALR;
    FADDR_COMALR;
    FADDR_EXTPARA;*/
    //数据内容没有擦除
    
    //格式化范围
    DWORD dwStart = FADDR_FAT;
    
    /*for (dwStart=FADDR_FAT; dwStart<FADDR_DYN; )
    {
        ExFlashEraseSect(i);
        dwStart += EXSECT_SIZE;
    }*/
    //搞这么复杂目的是为了用最快的速度擦除
    while(dwStart < FADDR_EXTPARA)
    {
        if ((dwStart&0xffff) == 0) //起始地址符合64K块擦除
            break;        
        else if (((dwStart&0x7fff) == 0) && ((dwStart+0x8000)<=FADDR_EXTPARA))//起始地址符合32K块擦除
        {
            ExFlashEraseBlock(dwStart, 0);
            dwStart += 0x8000;
        }
        else if ((dwStart&0xfff) == 0)
        {
            ExFlashEraseSect(dwStart);
            dwStart += EXSECT_SIZE;
        }
        else  //起始地址不是扇区首地址
            return false;
    }
    
    //符合64K块擦
    while((dwStart+0x10000) <= FADDR_EXTPARA)  //FADDR_DYN
    {
        ExFlashEraseBlock(dwStart, 1);
        dwStart += 0x10000;
    }
    
    //剩余够32K，用32K擦，不够用4K擦
    while(dwStart < FADDR_EXTPARA)  //FADDR_DYN
    {        
        if (((dwStart&0x7fff) == 0) && ((dwStart+0x8000)<=FADDR_EXTPARA))//起始地址符合32K块擦除  //FADDR_DYN
        {
            ExFlashEraseBlock(dwStart, 0);
            dwStart += 0x8000;
        }
        else
        {
            ExFlashEraseSect(dwStart);
            dwStart += EXSECT_SIZE;
        }
    }   
    
    return true;
}
