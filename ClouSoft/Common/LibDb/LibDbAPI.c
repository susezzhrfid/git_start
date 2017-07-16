/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbAPI.cpp
 * 摘    要：本文件主要实现数据库的公共接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#include "TypeDef.h"
#include "FaCfg.h"
#include "LibDbCfg.h"
#include "DataManager.h"
#include "DbHook.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "FileMgr.h"
#include "SysDebug.h"
#include <assert.h>

//TSem   g_semDataRW;
//TSem   g_semDbSave;
bool g_fLockDB = false;
//TTime g_tmAccessDenied;
TDbData g_DbData;	//数据库临时数据

#ifdef EN_DB_SECTHOLDER //支持扇区保持
TSectHolder g_SectHolder;	//扇区数据保持结构
#endif //EN_DB_SECTHOLDER

TSem g_semDbBuf;
TSem g_semBankRW[DB_STORAGE_TYPE_NUM];
WORD g_wDynPn[DYN_PN_NUM];	//用来存储当前缓存的动态测量点
BYTE g_bDBBuf[DBBUF_SIZE];

const TItemDesc* BinarySearchItem(const TItemDesc* pItemDesc, WORD num, WORD wID)
{
    int little, big, mid;
	if (wID<pItemDesc[0].wID  || wID>pItemDesc[num-1].wID)
		return NULL;

    little = 0;
    big = num;
    while (little <= big)
    {                               
        mid = (little + big) >> 1;       //二分

        if (pItemDesc[mid].wID == wID) 
		{
			return pItemDesc + mid;
		}
		else if (wID > pItemDesc[mid].wID)
        {
          little = mid + 1;
        } 
        else  //if (wID < pItemDesc[mid].wID)
        {
          big = mid - 1;
        }

        mid = (little + big) >> 1;
	}

	return NULL;
}


int BinarySearchIndex(const TItemDesc* pItemDesc, WORD num, WORD wID)
{
    int little, big, mid;
	if (wID<pItemDesc[0].wID  || wID>pItemDesc[num-1].wID)
		return -1;

    little = 0;
    big = num;
    while (little <= big)
    {                               
        mid = (little + big) >> 1;       //二分

        if (pItemDesc[mid].wID == wID) 
		{
			return mid;
		}
		else if (wID > pItemDesc[mid].wID)
        {
			little = mid + 1;
        } 
        else  //if (wID < pItemDesc[mid].wID)
        {
			big = mid - 1;
        }

        mid = (little + big) >> 1;
	}

	return -1;
}



//描述：根据存储类型(RAM、INFLASH、EXFLASH等)的不同，取得不同的信号量
void LockBank(BYTE bStorage)
{
	if (bStorage >= DB_STORAGE_TYPE_NUM)
		return;

	WaitSemaphore(g_semBankRW[bStorage], SYS_TO_INFINITE);
}


void UnLockBank(BYTE bStorage)
{
	if (bStorage >= DB_STORAGE_TYPE_NUM)
		return;

	SignalSemaphore(g_semBankRW[bStorage]);
}

void SetDynPn(BYTE bBank, WORD wPn)
{
	if (bBank < DYN_PN_NUM)
	{
		g_wDynPn[bBank] = wPn;	//用来存储当前缓存的动态测量点
	}
}

//描述：取得动态存储测量点所在的BANK
BYTE GetBankOfDynPn(WORD wPn)
{
	BYTE i;
	for (i=0; i<DYN_PN_NUM; i++)
	{
		if (wPn == g_wDynPn[i])
			return i;
	}

	return 0xff;
}

#ifdef EN_DB_QRYTM
//描述：更新数据项时标
void UpdItemTime(const TBankCtrl* pBankCtrl, const TItemDesc* pItemDesc, WORD wPn, DWORD dwTime)
{
	DWORD i;
	DWORD dwTmIdx;
	BYTE bDynBn = 0;
	if (pBankCtrl->fUpdTime && (pItemDesc->wProp&DI_NTS)==0)
	{ //整个BANK支持时标			&&  本数据项不支持时标

		WORD wID, wOff;
		if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//动态存储，内存中只有一个测量点
		{
			bDynBn = GetBankOfDynPn(wPn);
			if (bDynBn >= DYN_PN_NUM)
				return;
if (wPn==97 && pItemDesc->wID==0x901f)
{
 	wPn = 97; 
}
			wPn = 0;
			wOff = ((pBankCtrl->dwIndexNum + 7) >> 3) * bDynBn * 2;
		}

		wID = pItemDesc->wID;
		dwTmIdx = pItemDesc->dwBlkIndex + wPn*pItemDesc->bBlkIndexNum + pItemDesc->bInnerIndex;
		//按照测量点展开后一个数据项的索引:dwBlkIndex+测量点*bBlkIndexNum+bInnerIndex

		if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)	//块ID,需要更新子项的时间
		{									//块ID自己含有的子数据项的个数
			i = dwTmIdx - (pItemDesc->bBlkIdIndexNum - 1);
			for (; i<=dwTmIdx; i++)
			{
				pBankCtrl->pbTimeFlg[wOff+(i>>3)] |= (1<<(i&7));	//只更新当前间隔时间标志
			}
		}
		else
		{
			pBankCtrl->pbTimeFlg[wOff+(dwTmIdx>>3)] |= 1<<(dwTmIdx&7);	//只更新当前间隔时间标志
		}
	}
}

bool IsItemTimeValid(const TBankCtrl* pBankCtrl, const TItemDesc* pItemDesc, WORD wPn, DWORD dwStartTime, DWORD dwEndTime)
{
	DWORD dwTmIdx, dwIdxSize;
	WORD wFlgPos0, wFlgPos1;
	BYTE bFlgMsk, bDynBn = 0;
	if (pBankCtrl->fUpdTime && (pItemDesc->wProp&DI_NTS)==0)
	{ //整个BANK支持时标			&&  本数据项不支持时标
		WORD wPnNum = pBankCtrl->wPnNum;
		if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//动态存储，内存中只有一个测量点
		{
			bDynBn = GetBankOfDynPn(wPn);
			if (bDynBn >= DYN_PN_NUM)
				return false;

			if (wPn==97 && pItemDesc->wID==0x901f)
			{
				wPn = 97;
			}

			wPnNum = 1;
			wPn = 0;
		}
		
		dwTmIdx = pItemDesc->dwBlkIndex + wPn*pItemDesc->bBlkIndexNum + 
					  pItemDesc->bInnerIndex;	//按照测量点展开后一个数据项的索引:dwBlkIndex+测量点*bBlkIndexNum+bInnerIndex

		dwIdxSize = (pBankCtrl->dwIndexNum*wPnNum + 7) >> 3;    //数据项索引的个数,即dwItemNum按照测量点个数展开后的个数

		bFlgMsk = 1<<(dwTmIdx&7);
		wFlgPos0 = dwIdxSize * (bDynBn<<1) + (dwTmIdx>>3);
		wFlgPos1 = dwIdxSize * ((bDynBn<<1) + 1) + (dwTmIdx>>3);

		if (dwStartTime == 0)
			dwEndTime = 0;

		if ((dwStartTime!=0 && pBankCtrl->pdwIntervTime[bDynBn<<1]<dwStartTime)  //当前间隔时间不符合要求
			|| (dwEndTime!=0 && pBankCtrl->pdwIntervTime[bDynBn<<1]>=dwEndTime))
		{
			if ((dwStartTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]<dwStartTime)  //上一间隔时间不符合要求
				|| (dwEndTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]>=dwEndTime))
			{
				return false;
			}
			else	//上一间隔时间符合要求
			{
				if (pBankCtrl->pbTimeFlg[wFlgPos1] & bFlgMsk)	//时间标志有效
					return true;
				else
					return false;
			}
		}
		else 	//当前间隔时间符合要求
		{
			if (pBankCtrl->pbTimeFlg[wFlgPos0] & bFlgMsk)	//时间标志有效
			{	
				return true;
			}
			else if ((dwStartTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]<dwStartTime)  //上一间隔时间不符合要求
					  || (dwEndTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]>=dwEndTime))
			{
				return false;
			}
			else	//上一间隔时间符合要求
			{
				if (pBankCtrl->pbTimeFlg[wFlgPos1] & bFlgMsk)	//时间标志有效
					return true;
				else
					return false;
			}
		}
	}

	return true;
}
#endif //EN_DB_QRYTM

void DbSetFileValid(const TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->fSect)
		g_DbData.dwFileValidFlg0 |= 1<<pBankCtrl->bBN;	//BANK0文件有效标志位
	else
		g_DbData.dwFileValidFlg1 |= 1<<pBankCtrl->bBN;	//扩展BANK文件有效标志位
}

void DbClrFileValid(const TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->fSect)
		g_DbData.dwFileValidFlg0 &= ~(1<<pBankCtrl->bBN);	//BANK0文件有效标志位
	else
		g_DbData.dwFileValidFlg1 &= ~(1<<pBankCtrl->bBN);	//扩展BANK文件有效标志位
}

bool DbIsFileValid(const TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->fSect)
		return (g_DbData.dwFileValidFlg0 & (1<<pBankCtrl->bBN)) != 0;	//BANK0文件有效标志位
	else
		return (g_DbData.dwFileValidFlg1 & (1<<pBankCtrl->bBN)) != 0;	//扩展BANK文件有效标志位
}

BYTE* GetItemAddr(DWORD dwOffset, const TBankCtrl* pBankCtrl)
{
#ifndef SYS_WIN
	if (pBankCtrl->bStorage == DB_STORAGE_IN)   //内部FLASH固定存储
	{
		return GetFileDirAddr(pBankCtrl->wFile, dwOffset);	//取得文件的直接读地址 //(BYTE* )(pFileCfg->dwAddr + dwSectSize*wSect + dwSectOff);
	}
	else 
#endif
	if (pBankCtrl->bStorage==DB_STORAGE_RAM || pBankCtrl->bStorage==DB_STORAGE_FRAM)
	{
		return pBankCtrl->pbBankData + dwOffset;
	}

	return NULL;
}

#ifdef EN_DB_SECTHOLDER //支持扇区保持
void DbWrHoldSect()
{
	if (g_SectHolder.fValid==false || g_SectHolder.fEnable==false)
		return;

	MakeFile(g_SectHolder.wFile, g_bDBBuf);	//给文件扇区加上文件标志和校验
	if (!writefile(g_SectHolder.wFile, g_SectHolder.wSect, g_bDBBuf))
	{
		if (!writefile(g_SectHolder.wFile, g_SectHolder.wSect, g_bDBBuf))
		{
			DbClrFileValid(g_SectHolder.pBankCtrl);
			return;	//设备有问题
		}
	}
//DTRACE(DB_ABB, ("DbWrHoldSect: end  at Click=%d, cost Click=%d.\n", GetClick(), GetClick()-dwLastClick));    
}

//描述：允许扇区数据保持
void DbEnSectHolder()
{
	WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
	if (g_SectHolder.fEnable == false)
	{
		memset(&g_SectHolder, 0, sizeof(g_SectHolder));	//扇区数据保持结构
		g_SectHolder.fEnable = true;
	}
}

void DbDisableSectHolder()
{
	DbWrHoldSect();
	memset(&g_SectHolder, 0, sizeof(g_SectHolder));	//扇区数据保持结构
	SignalSemaphore(g_semDbBuf);
}
#endif //EN_DB_SECTHOLDER



int DbWriteSect(const TBankCtrl* pBankCtrl, WORD wSect, DWORD dwSectOff, BYTE* pbBuf, WORD wLen)
{
	int iFileOff;

#ifdef EN_DB_SECTHOLDER //支持扇区保持
	if (g_SectHolder.fEnable && g_SectHolder.fValid &&	//允许扇区保持功能
		g_SectHolder.wFile==pBankCtrl->wFile && g_SectHolder.wSect==wSect)	//需要的扇区刚好被保持住了
	{
		iFileOff = GetFileOffset(pBankCtrl->wFile, wSect);
		if (iFileOff < 0)
			goto DbWriteSect_err;

		memcpy(g_bDBBuf+iFileOff+dwSectOff, pbBuf, wLen);
	}
	else if (g_SectHolder.fEnable)	//允许扇区数据保持功能,只是扇区发生了改变
	{
		if (g_SectHolder.fValid)	//扇区发生变化
			DbWrHoldSect();

		memset(g_bDBBuf, 0, DBBUF_SIZE);
		if (ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff) < 0)	//读取文件所在的第dwSect个扇区
			goto DbWriteSect_err;

		memcpy(g_bDBBuf+iFileOff+dwSectOff, pbBuf, wLen);

		g_SectHolder.fValid = true;
		g_SectHolder.wFile = pBankCtrl->wFile;
		g_SectHolder.wSect = wSect;
		g_SectHolder.pBankCtrl = pBankCtrl;
	}
	else //没允许扇区数据保持功能
#endif //EN_DB_SECTHOLDER
	{
		WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);

		memset(g_bDBBuf, 0, DBBUF_SIZE);
		if (ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff) < 0)	//读取文件所在的第dwSect个扇区
		{
			SignalSemaphore(g_semDbBuf);
			return -ERR_DEV;	//设备有问题
		}

		memcpy(g_bDBBuf+iFileOff+dwSectOff, pbBuf, wLen);

		MakeFile(pBankCtrl->wFile, g_bDBBuf);	//给文件扇区加上文件标志和校验
		if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
		{
			if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
			{
				DbClrFileValid(pBankCtrl);

				SignalSemaphore(g_semDbBuf);
				return -ERR_DEV;	//设备有问题
			}
		}

		SignalSemaphore(g_semDbBuf);
	}

	return wLen;

DbWriteSect_err:
	g_SectHolder.fValid = false;	//扇区数据保持无效
	return -ERR_DEV;	//设备有问题
}

int DbReadData(DWORD dwOffset, BYTE* pbBuf, WORD wLen, const TBankCtrl* pBankCtrl)
{
	DWORD dwSectSize, dwSectOff, dwUsedSize;
	WORD wSect, wCpyLen;
	if (pBankCtrl->bStorage==DB_STORAGE_IN || pBankCtrl->bStorage==DB_STORAGE_EX)   //内部FLASH固定存储 || 外部FLASH固定存储
	{
		if (DbIsFileValid(pBankCtrl))
		{
			dwSectSize = GetSectSize(pBankCtrl->wFile);	//取得文件保存的扇区大小
			dwUsedSize = GetSectUsedSize(pBankCtrl->wFile);//取得文件实际使用的扇区大小
            if (dwUsedSize == 0)
                return -ERR_DEV;
			wSect = dwOffset / dwUsedSize;
			dwSectOff = dwOffset % dwUsedSize;
			if (dwSectOff+wLen > dwUsedSize)	//跨扇区
			{
				wCpyLen = dwUsedSize - dwSectOff;
				if (!readfile(pBankCtrl->wFile, dwSectSize*wSect+dwSectOff, pbBuf, wCpyLen))
					return -ERR_DEV;	//设备有问题
				if (!readfile(pBankCtrl->wFile, dwSectSize*(wSect+1), pbBuf+wCpyLen, wLen-wCpyLen))
					return -ERR_DEV;	//设备有问题
			}
			else
			{
				if (!readfile(pBankCtrl->wFile, dwSectSize*wSect+dwSectOff, pbBuf, wLen))
					return -ERR_DEV;	//设备有问题
			}

			return wLen;
		}
		else
		{
			return -ERR_DEV;	//设备有问题
		}
	}
	else if (pBankCtrl->bStorage==DB_STORAGE_RAM || 
			 pBankCtrl->bStorage==DB_STORAGE_DYN ||
			 pBankCtrl->bStorage==DB_STORAGE_FRAM)
	{
		memcpy(pbBuf, pBankCtrl->pbBankData+dwOffset, wLen);
		return wLen;
	}

	return -ERR_DEV;	//设备有问题
}

int DbWriteData(DWORD dwOffset, BYTE* pbBuf, WORD wLen, const TBankCtrl* pBankCtrl, TItemAcess* pItemAcess, const TItemDesc* pItemDesc)
{
	DWORD dwUsedSize, dwSectOff;
	WORD wSect, wCpyLen;

	if (pItemDesc != NULL)
	{
		if (pItemDesc->wProp & DI_NSP) //No Space 不占用空间，使用其长度信息，且占用时间标志
			return wLen;
	}

	if (pBankCtrl->bStorage==DB_STORAGE_IN || pBankCtrl->bStorage==DB_STORAGE_EX)   //内部FLASH固定存储 || 外部FLASH固定存储
	{
		if (!DbIsFileValid(pBankCtrl))
		{
			if (DbLoadBankDefault(pBankCtrl))
				return -ERR_DEV;	//设备有问题
		}

		dwUsedSize = GetSectUsedSize(pBankCtrl->wFile); //取得文件实际使用的扇区大小
        if (dwUsedSize == 0)
            return -ERR_DEV;
		wSect = dwOffset / dwUsedSize;
		dwSectOff = dwOffset % dwUsedSize;
		if (dwSectOff+wLen > dwUsedSize)	//单个数据项跨扇区，分两次拷贝，只有扇区文件才有这种情况
		{
			//第一个扇区
			wCpyLen = dwUsedSize - dwSectOff;
			if (DbWriteSect(pBankCtrl, wSect, dwSectOff, pbBuf, wCpyLen) < 0)
				return -ERR_DEV;	//设备有问题

   			//第二个扇区
            if (DbWriteSect(pBankCtrl, wSect+1, 0, pbBuf+wCpyLen, wLen-wCpyLen) < 0) //跨扇区的肯定不是页文件，不用考虑dwFileOff
                return -ERR_DEV;	//设备有问题

			return wLen;
		}
		else	//单个数据项在同一扇区，一次拷贝就可以
		{
			return DbWriteSect(pBankCtrl, wSect, dwSectOff, pbBuf, wLen);
		}
	}
	else if (pBankCtrl->bStorage==DB_STORAGE_RAM || 
			 pBankCtrl->bStorage==DB_STORAGE_DYN ||
			 pBankCtrl->bStorage==DB_STORAGE_FRAM)
	{
		if (pItemAcess == NULL)
			memcpy(pBankCtrl->pbBankData+dwOffset, pbBuf, wLen);
		else if (pItemAcess->bType==DI_ACESS_UPD) //只有RAM数据，才支持更新数据项状态
			memset(pBankCtrl->pbBankData+dwOffset, GetInvalidData((BYTE )pItemAcess->dwVal), wLen);
		else
			memcpy(pBankCtrl->pbBankData+dwOffset, pbBuf, wLen);

		return wLen;
	}

	return -ERR_DEV;	//设备有问题
}

//返回:如果正确则返回数据的长度，如果错误则低8位返回错误代码，次低8位返回数据的长度
int WriteItem(WORD wImg, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwTime, const TBankCtrl* pBankCtrl)
{
	int iRet;
	int nIndex;
	DWORD dwOffset;
	const TItemDesc* pItemDesc;
#ifdef EN_DB_PNMAP
	int iPn;
	BYTE bPnMapSch;
#endif //EN_DB_PNMAP
	BYTE bDynBn = 0;

	if (pBankCtrl->pItemDesc == NULL)
		return -ERR_ITEM;

	nIndex = BinarySearchIndex(pBankCtrl->pItemDesc, pBankCtrl->dwItemNum, wID);
	if (nIndex < 0)
	    return -ERR_ITEM;

	if (pBankCtrl->pbBankData==NULL &&
		(pBankCtrl->bStorage==DB_STORAGE_RAM || pBankCtrl->bStorage==DB_STORAGE_DYN || pBankCtrl->bStorage==DB_STORAGE_FRAM)) //本BANK的只作为数据项描述用,真正的数据访问要靠相应的读写函数
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum == 0)	// && pBankCtrl->wImgNum==0
		return -ERR_ITEM; //本BANK的只作为数据项描述用,只支持DI_ACESS_INFO的ReadItem()访问

	pItemDesc = &pBankCtrl->pItemDesc[nIndex];

	if ((pItemDesc->wProp & DI_CMB) != 0) //组合ID不能进行后续的访问
		return -ERR_ITEM;

#ifdef EN_DB_PNMAP
	//测量点到实际存储号(映射号)的转换
	bPnMapSch = 0;
	if (pBankCtrl->bPnMapSch != 0)
		bPnMapSch = pBankCtrl->bPnMapSch;
	else if (pItemDesc->bPnMapSch != 0)
		bPnMapSch = pItemDesc->bPnMapSch;

	if (bPnMapSch != 0)
	{
		iPn = SearchPnMap(bPnMapSch, wPn);
		if (iPn < 0)
		{
			DTRACE(DB_DB, ("WriteItem: pnmap not found, wPn=%d, sch=%d, wID=%04x\n", 
						   wPn, bPnMapSch, wID));

			return -ERR_ITEM; //return -(ERR_PNUNMAP + (int )pItemDesc->wLen*0x100);
		}

		wPn = (WORD )iPn;
	}
#endif //EN_DB_PNMAP

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //本数据项不支持那么多个测量点
		return -ERR_ITEM;
	
	if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//动态存储，内存中只有一个测量点
	{
		bDynBn = GetBankOfDynPn(wPn);
		if (bDynBn >= DYN_PN_NUM)
			return -ERR_ITEM;

		//wPn = 0;
		dwOffset = pBankCtrl->dwBankSize*bDynBn + pItemDesc->dwBlockOffset + pItemDesc->wOffset;
				//一个数据项的偏移:dwBlockOffset+测量点*wBlockLen+wOffset
	}
	else
	{
		dwOffset = pItemDesc->dwBlockOffset + wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
				//一个数据项的偏移:dwBlockOffset+测量点*wBlockLen+wOffset
	}

	if (pItemAcess->bType == DI_ACESS_BUF)
	{
		LockBank(pBankCtrl->bStorage);
		iRet = DbWriteData(dwOffset, pItemAcess->pbBuf, pItemDesc->wLen, pBankCtrl, pItemAcess, pItemDesc);
#ifdef EN_DB_QRYTM
		UpdItemTime(pBankCtrl, pItemDesc, wPn, dwTime);	//更新数据项时标
#endif //EN_DB_QRYTM
		UnLockBank(pBankCtrl->bStorage);
	}
#ifdef EN_DB_QRYTM
	else if (pItemAcess->bType == DI_ACESS_UPD) //更新数据项状态
	{
		LockBank(pBankCtrl->bStorage);
		iRet = DbWriteData(dwOffset, pItemAcess->pbBuf, pItemDesc->wLen, pBankCtrl, pItemAcess, pItemDesc);
		UpdItemTime(pBankCtrl, pItemDesc, wPn, dwTime);	//更新数据项时标
		UnLockBank(pBankCtrl->bStorage);
	}
#endif //EN_DB_QRYTM
	else //if (pItemAcess->bType==DI_ACESS_INT32 || pItemAcess->bType==DI_ACESS_INT64)	//求值
	{
		return -ERR_ITEM;
	}

	//TODO：编程日志

	if (iRet>0 && pItemDesc->bWrOp!=INFO_NONE) // && pbPassword!=NULL
	{
		SetDelayInfo(pItemDesc->bWrOp);
	}	
	
	return iRet; //pItemDesc->wLen;
}


int ReadItem(WORD wImg, WORD wPn, WORD wID, TItemAcess* pItemAcess, 
			 DWORD dwStartTime, DWORD dwEndTime, 
			 const TBankCtrl* pBankCtrl)
{
	int iRet, nIndex;
	DWORD dwOffset;
	const TItemDesc* pItemDesc;
#ifdef EN_DB_PNMAP
	int iPn;
	BYTE bPnMapSch;
#endif //EN_DB_PNMAP
	BYTE bDynBn = 0;

	if (pBankCtrl->pItemDesc == NULL)
		return -ERR_ITEM;
		
    nIndex = BinarySearchIndex(pBankCtrl->pItemDesc, pBankCtrl->dwItemNum, wID);
	if (nIndex < 0)
	    return -ERR_ITEM;

	pItemDesc = &pBankCtrl->pItemDesc[nIndex];

	if ((pItemDesc->wProp & DI_CMB) != 0) //组合ID不能进行后续的访问
		return -ERR_ITEM;

	if (pItemAcess->bType == DI_ACESS_INFO)	//取数据项长度
	{	//如果wPnNum和wImgNum同时配置为0,表示本BANK的只作为数据项描述用
		if (pBankCtrl->wPnNum > 1)
			pItemAcess->pItemInfo->wPnNum = pBankCtrl->wPnNum;
		else
			pItemAcess->pItemInfo->wPnNum = pItemDesc->wPnNum;

		pItemAcess->pItemInfo->wLen = pItemDesc->wLen;
		return pItemDesc->wLen;
	}

	if (pBankCtrl->pbBankData==NULL &&
		(pBankCtrl->bStorage==DB_STORAGE_RAM || pBankCtrl->bStorage==DB_STORAGE_DYN || pBankCtrl->bStorage==DB_STORAGE_FRAM)) //本BANK的只作为数据项描述用,真正的数据访问要靠相应的读写函数
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum == 0)	 //&& pBankCtrl->wImgNum==0
		return -ERR_ITEM; //本BANK的只作为数据项描述用,只支持DI_ACESS_INFO的ReadItem()访问

#ifdef EN_DB_PNMAP
	//测量点到实际存储号(映射号)的转换
	bPnMapSch = 0;
	if (pBankCtrl->bPnMapSch != 0)
		bPnMapSch = pBankCtrl->bPnMapSch;
	else if (pItemDesc->bPnMapSch != 0)
		bPnMapSch = pItemDesc->bPnMapSch;

	if (bPnMapSch!=0 && pItemAcess->bType!=DI_ACESS_RDUNMAP)	//按照非映射的方式读
	{
		iPn = SearchPnMap(bPnMapSch, wPn);
		if (iPn < 0)
		{
			DTRACE(DB_DB, ("ReadItem: pnmap not found, wPn=%d, sch=%d, wID=%04x\n", 
						   wPn, bPnMapSch, wID));
			
			return -ERR_ITEM; //-(ERR_PNUNMAP + (int )pItemDesc->wLen*0x100);
		}

		wPn = (WORD )iPn;
	}
#endif //EN_DB_PNMAP

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //本数据项不支持那么多个测量点
		return -ERR_ITEM;
	
/*	wBlkIndexNum = 1;
	if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)
	{							//块ID自己含有的子数据项的个数
		wBlkIndexNum = pItemDesc->bBlkIdIndexNum - 1;
	}*/
	//TODO:检查文件是否有效，比如片内FLASH是否写好，片外FLASH已经调入RAM
#ifdef EN_DB_QRYTM
	if (!IsItemTimeValid(pBankCtrl, pItemDesc, wPn, dwStartTime, dwEndTime))
	{
		if (pItemAcess->bType==DI_ACESS_BUF || pItemAcess->bType==DI_ACESS_RDUNMAP)
		{
			memset(pItemAcess->pbBuf, GetInvalidData(ERR_APP_OK), pItemDesc->wLen);
		}
		else if (pItemAcess->bType == DI_ACESS_INT32) //按照整形32位读,把全部数据置为INVALID_VAL
		{
		}
		else if (pItemAcess->bType == DI_ACESS_INT64) //按照整形64位读,把全部数据置为INVALID_VAL64
		{
		}

		return -(ERR_TIME + (int )pItemDesc->wLen*0x100);
	}
#endif //EN_DB_QRYTM

	if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//动态存储，内存中只有一个测量点
	{
		bDynBn = GetBankOfDynPn(wPn);
		if (bDynBn >= DYN_PN_NUM)
			return -ERR_ITEM;
	
		//wPn = 0;
		dwOffset = pBankCtrl->dwBankSize*bDynBn + pItemDesc->dwBlockOffset + pItemDesc->wOffset;
				//一个数据项的偏移:dwBlockOffset+测量点*wBlockLen+wOffset
	}
	else
	{
		dwOffset = pItemDesc->dwBlockOffset + wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
					//一个数据项的偏移:dwBlockOffset+测量点*wBlockLen+wOffset
	}

	if (pItemAcess->bType==DI_ACESS_BUF || pItemAcess->bType==DI_ACESS_RDUNMAP)
	{
		LockBank(pBankCtrl->bStorage);
		iRet = DbReadData(dwOffset, pItemAcess->pbBuf, pItemDesc->wLen, pBankCtrl);
		UnLockBank(pBankCtrl->bStorage);
		return iRet; 
	}
	else if (pItemAcess->bType==DI_ACESS_INT32 || pItemAcess->bType==DI_ACESS_INT64)	//求值
	{
	}
#ifdef EN_DB_QRYTM
	else if (pItemAcess->bType == DI_ACESS_QRY) //查询数据项是否更新,什么都不用干
	{
		bool fInvalid = false;

		LockBank(pBankCtrl->bStorage);
		if (DbReadData(dwOffset, pItemAcess->pbBuf, pItemDesc->wLen, pBankCtrl))
			fInvalid = IsInvalidData(pItemAcess->pbBuf, pItemDesc->wLen);
		else
			fInvalid = true;
		UnLockBank(pBankCtrl->bStorage);

		if (fInvalid)
			return -(ERR_INVALID + (int )pItemDesc->wLen*0x100);
	}
#endif //EN_DB_QRYTM
	else if (pItemAcess->bType == DI_ACESS_GI)
	{
		TDataItem* pDI = (TDataItem* )pItemAcess->pbBuf;
		pDI->pbAddr = GetItemAddr(dwOffset, pBankCtrl);
		pDI->dwOffset = dwOffset;
		pDI->wLen = pItemDesc->wLen;
		pDI->pBankCtrl = pBankCtrl;
	}

	return pItemDesc->wLen;
}


void ReadItemDI(const TDataItem* pDI, BYTE* pbBuf)
{
	if (pDI->pBankCtrl==NULL || pDI->wLen==0)
		return;

    LockBank(pDI->pBankCtrl->bStorage);
	DbReadData(pDI->dwOffset, pbBuf, pDI->wLen, pDI->pBankCtrl);
	UnLockBank(pDI->pBankCtrl->bStorage);
}


void WriteItemDI(const TDataItem* pDI, BYTE* pbBuf)
{
	if (IsDbLocked())
		return;

	if (pDI->pBankCtrl==NULL || pDI->wLen==0)
		return;

    LockBank(pDI->pBankCtrl->bStorage);
	DbWriteData(pDI->dwOffset, pbBuf, pDI->wLen, pDI->pBankCtrl, NULL, NULL);
	UnLockBank(pDI->pBankCtrl->bStorage);
}

#ifdef EN_DB_QRYTM
//描述：通常是用来在抄到电表不支持的数据项或抄表失败的时候,更新数据项时间,
//		以加快应用查询数据的及时性
//参数：@wErr 错误代码,ERR_UNSUP电表不支持的数据项,ERR_FAIL抄表失败
//备注：目前只支持到更新数据项时间,不支持更新错误代码
bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime)
{
	TItemAcess ItemAcess;
	if (wBank != BN0)
		return true;
	
	ItemAcess.bType = DI_ACESS_UPD;	//更新数据项状态
	ItemAcess.dwVal = wErr;

	return DbWriteItemEx(wBank, wPn, wID, &ItemAcess, dwTime)>0;
}

//描述：查询数据项是否在dwStartTime后被更新过
//参数：@dwStartTime 抄表间隔的起始时间,从2000年1月1日0点0分0秒算起的秒
//					 注意与SubmitMeterReq()的dwStartTime不尽相同
//		@dwEndTime   小于相应的结束时间
//      @pBank0Item 指向数据项数组的指针
//      @wNum 数组元素的个数
// 		@pbBuf 给系统库借用的缓冲
//		@pwValidNum 用来返回合法数据项的个数
//返回：已经被确认的数据项个数,包括已经抄到,确认不支持的或确认抄不到的数据项
//备注：数据项被更新过不一定代表数据内容合法,电表不支持或抄表失败的数据项可以
//		用UpdItemErr()更新时间,以加快应用查询数据的及时性
int QueryItemTimeMbi(DWORD dwStartTime, DWORD dwEndTime, TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum)
{
	TItemAcess ItemAcess;
	int iRet;
	int iConfirmNum;
	WORD i, wValidNum;

	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_QRY;
	ItemAcess.pbBuf = pbBuf;	//给系统库借用的缓冲

	iConfirmNum = 0;
	wValidNum = 0;
	for (i=0; i<wNum; i++)
	{
		const WORD* pwSubID = CmbToSubID(pBankItem->wBn, pBankItem->wID);
		if (pwSubID == NULL)
		{
			iRet = DbReadItemEx(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, &ItemAcess, dwStartTime, dwEndTime);
		}
		else
		{
			int iLen;	//用来计算组合数据项的长度
			WORD wID;
			bool fInvalid = false;
			iRet = 0;

			while ((wID=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
			{
				iLen = DbReadItemEx(pBankItem->wBn, pBankItem->wPn, wID, &ItemAcess, dwStartTime, dwEndTime);
				
				if (iLen > 0)
				{
					iRet += iLen;
				}
				else if (iLen < 0)
				{
					iLen = -iLen;	//-(ERR_TIME + (int )pItemDesc->wLen*0x100);
					if ((iLen&0xff) != ERR_INVALID)  //时间被更新了,但数据项内容无效,比如电表不支持的数据项或抄表失败等
					{
						//错误代码如果返回ERR_ITEM/ERR_TIME等错误,该组合ID的剩余ID就不用再查询了,
						//因为该数据项已经彻底不符合要求了
						fInvalid = false;	//其它错误,不能归为ERR_INVALID
						iRet = -iLen;
						break;	
					}
					
					iRet += (iLen>>8);
					fInvalid = true;
				}
				else //iLen==0
				{
					fInvalid = false;	//其它错误,不能归为ERR_INVALID
					iRet = 0;
					break;
				}
			}

			if (fInvalid && wID==0)	//全部子数据项查完后,确定只剩下ERR_INVALID错误
				iRet = -(ERR_INVALID + iLen*0x100);
		}

		if (iRet > 0)
		{
			iConfirmNum++;
			wValidNum++;
		}
		else
		{
			iRet = (-iRet) & 0xff;	  //取错误代码
			if (iRet == ERR_INVALID)  //时间被更新了,但数据项内容无效,比如电表不支持的数据项或抄表失败等
				iConfirmNum++;
			else if (iRet == ERR_ITEM)
			{
				*pwValidNum = 0;
				return -ERR_ITEM;
			}
		}

		pBankItem++;
	}

	*pwValidNum = wValidNum;
	return iConfirmNum;
}

//描述:按照不同数据项不同时间的查询,看数据项是否在pdwStartTime后被更新过
//参数:@pdwStartTime 数据项时间的数组
// 		@pbBuf 给系统库借用的缓冲
int QueryItemTimeMt(DWORD* pdwStartTime, DWORD* pdwEndTime, TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum)
{
	int iRet = 0;
	WORD i, wValidRet=0, wValidNum=0;
	for (i=0; i<wNum; i++,pBankItem++)
	{
		int iConfirmNum = QueryItemTimeMbi(*pdwStartTime++, *pdwEndTime++, pBankItem, 1, pbBuf, &wValidNum);
		if (iConfirmNum == -ERR_ITEM)
		{
			*pwValidNum = 0;
			return -ERR_ITEM;
		}

		iRet += iConfirmNum;
		wValidRet += wValidNum;
	}
	
	*pwValidNum = wValidRet;
	return iRet;
}

int QueryItemTimeMid(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, WORD* pwID, WORD wNum, BYTE* pbBuf, WORD* pwValidNum)
{
	TBankItem BankItem[10]; //少量多次  50
	int iRet = 0;
	int iConfirmNum;
	WORD i, wValidRet=0, wValidNum=0;
	while (wNum > 0)
	{
		WORD n = wNum>=10 ? 10 : wNum;

		for (i=0; i<n; i++)
		{
			BankItem[i].wBn = wBn;  	//测量点号
			BankItem[i].wPn = wPn;  	//测量点号
			BankItem[i].wID = *pwID++;  //BN0或者645ID
		}

		iConfirmNum = QueryItemTimeMbi(dwStartTime, dwEndTime, BankItem, n, pbBuf, &wValidNum); //查询数据是否更新
		if (iConfirmNum == -ERR_ITEM)
		{
			*pwValidNum = 0;
			return -ERR_ITEM;
		}

		iRet += iConfirmNum;
		wValidRet += wValidNum;

		wNum -= n;
	}

	*pwValidNum = wValidRet;

	return iRet;
}

int ReadItemGetTime(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime)
{
	int iRet;
	TItemAcess ItemAcess;
	//memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_BUF;
	ItemAcess.pbBuf = pbBuf;
	ItemAcess.pdwTime = pdwTime;
	
	iRet = DbReadItemEx(wBank, wPn, wID, &ItemAcess, INVALID_TIME, INVALID_TIME);
	if (iRet > 0)
	{
		iRet = PostReadItemExHook(wBank, wPn, wID, pbBuf, iRet);
	}
	
	return iRet;
}
#endif //EN_DB_QRYTM

int WriteItemTm(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime)
{
	int iRet;
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_BUF;
	ItemAcess.pbBuf = pbBuf;
	
//    DTRACE(DB_CRITICAL, ("WriteItemTm Start at click = %d.\r\n", GetClick()));
	iRet = DbWriteItemEx(wBank, wPn, wID, &ItemAcess, dwTime);
//    DTRACE(DB_CRITICAL, ("WriteItemTm end at click = %d.\r\n", GetClick()));
    
	if (iRet > 0)
	{
//        DTRACE(DB_CRITICAL, ("PostWriteItemTm Start at click = %d.\r\n", GetClick()));
		iRet = PostWriteItemExHook(wBank, wPn, wID, pbBuf, iRet); //调用挂钩进行应用特殊处理
//       DTRACE(DB_CRITICAL, ("PostWriteItemTm end at click = %d.\r\n", GetClick()));
	}
		 
	return iRet;
}

int ReadItemTm(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iRet;
	TItemAcess ItemAcess;
#ifdef EN_DB_CMB
	const WORD* pwSubID = CmbToSubID(wBank, wID);

	if (pwSubID == NULL)	//单ID,不是组合ID
	{
#endif //EN_DB_CMB

		ItemAcess.bType = DI_ACESS_BUF;
		ItemAcess.pbBuf = pbBuf;
		ItemAcess.pdwTime = NULL;
		iRet = DbReadItemEx(wBank, wPn, wID, &ItemAcess, dwStartTime, dwEndTime);
		if (iRet > 0)
			iRet = PostReadItemExHook(wBank, wPn, wID, ItemAcess.pbBuf, iRet);

#ifdef EN_DB_CMB
	}
	else //组合ID
	{
		int iLen; //用来计算组合数据项的长度
		WORD id;
		BYTE bErr = 0;
		iRet = 0;

		while ((id=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
		{
			ItemAcess.bType = DI_ACESS_BUF;
			ItemAcess.pbBuf = pbBuf + iRet;
			ItemAcess.pdwTime = NULL;
			iLen = DbReadItemEx(wBank, wPn, id, &ItemAcess, dwStartTime, dwEndTime);
			if (iLen > 0)
			{	
				iLen = PostReadItemExHook(wBank, wPn, id, ItemAcess.pbBuf, iLen);
			}
			
			if (iLen > 0)
			{
				iRet += iLen;
			}
			else if (iLen < 0)
			{
				if (iLen == -ERR_ITEM)	//不支持的数据项
					return -ERR_ITEM;
				
				//支持的数据项,但发生了错误
				iLen = -iLen;	//-(ERR_TIME + (int )pItemDesc->wLen*0x100);
				bErr = iLen & 0xff; //目前错误只能保留一个
				iRet += (iLen>>8);
			}
			else //iLen==0
			{
				return -ERR_ITEM;
			}
		}

		if (bErr != 0)
			iRet = -(bErr + iRet*0x100);

		PostReadCmbIdHook(wBank, wPn, wID, pbBuf, dwStartTime, iRet); 
				//即使在部分错误的情况下,都要调用这个函数来调整相应的子ID
	}	
#endif //EN_DB_CMB

	return iRet;
}

#ifdef EN_DB_PNMAP
//按照非映射的方式读
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{	
	int iRet;
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_RDUNMAP;		//按照非映射的方式读
	ItemAcess.pbBuf = pbBuf;
	ItemAcess.pdwTime = NULL;
	
	iRet = DbReadItemEx(wBank, wPn, wID, &ItemAcess, dwStartTime, dwEndTime);
	if (iRet > 0)
	{
		iRet = PostReadItemExHook(wBank, wPn, wID, pbBuf, iRet);
	}

	return iRet;
}
#endif //EN_DB_PNMAP

#ifdef EN_DB_MULTI
//描述:多个ID的按缓冲区读
int ReadItemMid(WORD wBank, WORD wPn, const WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	WORD i;
	for (i=0; i<wNum; i++)
	{
		int iRet = ReadItemTm(wBank, wPn, *pwID++, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//不能确定数据项长度
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//取数据项长度
										//数据内容已经被填成了无效数据
		}

		iLen += iRet;
		pbBuf += iRet;
	}

	return iLen;
}

//描述:多个ID的按缓冲区读
int ReadItemMbi(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	WORD i;
	for (i=0; i<wNum; i++)
	{
		int iRet = ReadItemTm(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//不能确定数据项长度
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//取数据项长度
										//数据内容已经被填成了无效数据
		}

		iLen += iRet;
		pbBuf += iRet;
		pBankItem++;
	}

	return iLen;
}

//描述:多个ID按不同的时标的缓冲区读
int ReadItemMbiMt(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD* pdwTime)
{
	int iLen = 0;
	WORD i;
	for (i=0; i<wNum; i++)
	{
		int iRet = ReadItemTm(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, 
							  pbBuf, *pdwTime++, INVALID_TIME);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//不能确定数据项长度
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//取数据项长度
										//数据内容已经被填成了无效数据
		}

		iLen += iRet;
		pbBuf += iRet;
		pBankItem++;
	}

	return iLen;
}
#endif //EN_DB_MULTI

#ifdef EN_DB_ITEMINFO
int GetItemInfo(WORD wBn, WORD wID, TItemInfo* pItemInfo)
{
	int iItemLen;
	TItemAcess ItemAcess;

	ItemAcess.bType = DI_ACESS_INFO;	//取数据项信息(长度和段)
	ItemAcess.pItemInfo = pItemInfo;	
	pItemInfo->wSect = 0;
	iItemLen = DbReadItemEx(wBn, PN0, wID, &ItemAcess, INVALID_TIME, INVALID_TIME);
	if (iItemLen > 0)
	{
		pItemInfo->wLen = iItemLen;
		return iItemLen;
	}
	else 
	{
		return -ERR_ITEM;
	}
}

int GetItemLen(WORD wBn, WORD wID)
{
	int iRet = 0;
   	TItemInfo ItemInfo;
	const WORD* pwSubID = CmbToSubID(wBn, wID);

	if (pwSubID == NULL)	//单ID,不是组合ID
	{	
		return GetItemInfo(wBn, wID, &ItemInfo);
	}
	else //组合ID
	{
		int iLen; //用来计算组合数据项的长度
		WORD id;
		iRet = 0;

		while ((id=*pwSubID++) != 0) //把组合ID转换成依次对子ID的读
		{
			iLen = GetItemInfo(wBn, id, &ItemInfo);

			if (iLen > 0)
				iRet += iLen;
			else if (iLen <= 0)
				return iLen;
		}
	}	

	return iRet;
}

int GetItemsLenId(WORD* pwItemID, WORD wLen)
{
	int nTotalLen = 0;
	int nLastLen = 0;
	int i;

	for (i=0; i<wLen; i++)
	{
		WORD wID = *pwItemID++;
		if (wID==0x8ffe && nLastLen!=0) //如果是对应量
		{
			nTotalLen += nLastLen;
		}
		else
		{	
			int iRet = GetItemLen(BN0, wID);
			if (iRet > 0)
			{	
				nTotalLen += iRet;
				nLastLen = iRet;
			}
			else
			{
				return -i; //-ERR_ITEM;
			}
		}
	}

	return nTotalLen;
}


int GetItemsLenBi(TBankItem* pBankItem, WORD wNum)
{
	int nTotalLen = 0;
	int nLastLen = 0;
	int i;

	for (i=0; i<wNum; i++,pBankItem++)
	{
		if (pBankItem->wBn==BN0 && pBankItem->wID==0x8ffe && nLastLen!=0) //如果是对应量
		{
			nTotalLen += nLastLen;
		}
		else
		{	
			int iRet = GetItemLen(pBankItem->wBn, pBankItem->wID);
			if (iRet > 0)
			{	
				nTotalLen += iRet;
				nLastLen = iRet;
			}
			else
			{
				return -i; //-ERR_ITEM;
			}
		}
	}

	return nTotalLen;
}

int GetItemPnNum(WORD wBn, WORD wID)
{
	TItemInfo ItemInfo;

	if (GetItemInfo(wBn, wID, &ItemInfo) > 0)
		return ItemInfo.wPnNum;
	else
		return -ERR_ITEM;
}
#endif //EN_DB_ITEMINFO

//描述:取得数据项的读地址,主要针对那些内容比较长的数据项,比如测量点屏蔽位等,
//	   免去数据内容拷贝的时间消耗,直接访问只读的内存地址,这样也不会破坏系统库的内容
//返回:如果正确则返回数据项的地址,否则返回NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID)
{
	TDataItem di = GetItemEx(wBn, wPn, wID);
	return di.pbAddr;
}


//描述:取得数据项的读地址,对于片内FLASH、RAM的情况，直接返回pDI->pbAddr
//		对于片外FLASH或者windows模拟FLASH的情况，先执行一次读，把接收缓冲区的地址返回
//返回:如果正确则返回数据项的地址,否则返回NULL
const BYTE* GetItemRdAddrID(WORD wBn, WORD wPn, WORD wID, BYTE* pbBuf)
{
	TDataItem di = GetItemEx(wBn, wPn, wID);

#ifndef SYS_WIN		//windows下不能直接按地址读取内容，必须把先把数据读到内存中才能访问
	return di.pbAddr;
#else
	if (pbBuf == NULL)
		return NULL;

	ReadItemDI(&di, pbBuf);
	return pbBuf;
#endif
}

//描述：根据数据项存储类型的不同，对于片内FLASH、RAM的情况，直接返回pDI->pbAddr
//		对于片外FLASH或者windows模拟FLASH的情况，先执行一次读，把接收缓冲区的地址返回
BYTE* GetItemRdAddrDI(TDataItem* pDI, BYTE* pbBuf)
{
#ifndef SYS_WIN		//windows下不能直接按地址读取内容，必须把先把数据读到内存中才能访问
	if (pDI->pbAddr != NULL)
		return pDI->pbAddr;
#endif

	ReadItemDI(pDI, pbBuf);
	return pbBuf;
}


void TrigerSave()
{
	//g_DataManager.TrigerSaveAll();
}

void TrigerSavePara()
{
	//g_DataManager.TrigerSavePara();
}


int WriteItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf)
{
	return WriteItemTm(wBank, wPn, wID, pbBuf, INVALID_TIME);
}

//不指定时间读，单纯读数据,最基本的读函数
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf) 
{
	return ReadItemTm(wBank, wPn, wID, pbBuf, INVALID_TIME, INVALID_TIME); //指定时间读
}

TDataItem GetItemEx(WORD wBank, WORD wPoint, WORD wID)
{
	return DbGetItemEx(wBank, wPoint, wID);
}


void LockDB()
{
	g_fLockDB = true;
}

void UnLockDB()
{
	g_fLockDB = false;
}

bool IsDbLocked()
{
	return g_fLockDB;
}

//描述:初始化系统库的代码库
bool InitDbLib(TDbCtrl* pDbCtrl)
{
	WORD i;

	g_semDbBuf = NewSemaphore(1, 1);
//	g_semDataRW = NewSemaphore(1, 1);  //liyan
//	g_semDbSave = NewSemaphore(1, 1);  //liyan
	
	for (i=0; i<DB_STORAGE_TYPE_NUM; i++)
	{
		g_semBankRW[i] = NewSemaphore(1, 1);
	}
	
	//memset(g_wDynPn, 0xff, sizeof(g_wDynPn));	//用来存储当前缓存的动态测量点
	//g_wDynPn = 0xff;

#ifdef EN_DB_SECTHOLDER //支持扇区保持
	memset(&g_SectHolder, 0, sizeof(g_SectHolder));	//扇区数据保持结构
#endif //EN_DB_SECTHOLDER

	return DbInit(pDbCtrl);
}