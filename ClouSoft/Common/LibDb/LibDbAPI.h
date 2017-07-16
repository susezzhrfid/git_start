/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbAPI.h
 * 摘    要：本文件主要实现数据库的公共接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef LIBDBAPI_H
#define LIBDBAPI_H

#include "TypeDef.h"
#include "sysarch.h"
#include "LibDbStruct.h"
#include "DataManager.h"
#include "ComStruct.h"

extern TSem   g_semDataRW;
extern TTime g_tmAccessDenied;
//extern CDataManager g_DataManager;
extern TDbData g_DbData;	//数据库临时数据

#ifdef EN_DB_SECTHOLDER //支持扇区保持
extern TSectHolder g_SectHolder;	//扇区数据保持结构
#endif //EN_DB_SECTHOLDER

extern TSem g_semDbBuf;

extern BYTE g_bDBBuf[];

const TItemDesc* BinarySearchItem(const TItemDesc* pItemDesc, WORD num, WORD wID);
int BinarySearchIndex(const TItemDesc* pItemDesc, WORD num, WORD wID);
bool InitItemDesc(TBankCtrl* pBankCtrl);
int ReadItem(WORD wImg, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwStartTime, DWORD dwEndTime, const TBankCtrl* pBankCtrl);
int WriteItem(WORD wImg, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwTime, const TBankCtrl* pBankCtrl);
void ReadItemDI(const TDataItem* pDI, BYTE* pbBuf);
void WriteItemDI(const TDataItem* pDI, BYTE* pbBuf);

int WriteItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf); //不指定时间写，最基本的写函数
int WriteItemTm(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime);

//单个ID按缓冲区的读
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf); //不指定时间读，单纯读数据,最基本的读函数
int ReadItemTm(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime); //指定时间读

int ReadItemGetTime(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime); //读数据同时取数据库中的时标

//多个ID的读接口,目前只支持按缓冲区的读
int ReadItemMid(WORD wBank, WORD wPn, const WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime); //按相同测量点相同时间读
int ReadItemMbi(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);	//按相同时间读
int ReadItemMbiMt(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD* pdwTime);	//按不同时间读

//按照非映射的方式读
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);

TDataItem GetItemEx(WORD wBank, WORD wPoint, WORD wID);

int GetItemInfo(WORD wBn, WORD wID, TItemInfo* pItemInfo);
int GetItemLen(WORD wBn, WORD wID);
int GetItemsLenId(WORD* pwItemID, WORD wLen);
int GetItemsLenBi(TBankItem* pBankItem, WORD wNum);
int GetItemPnNum(WORD wBn, WORD wID);

//描述:取得数据项的读地址,主要针对那些内容比较长的数据项,比如测量点屏蔽位等,
//	   免去数据内容拷贝的时间消耗,直接访问只读的内存地址,这样也不会破坏系统库的内容
//返回:如果正确则返回数据项的地址,否则返回NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID);

//描述：根据数据项存储类型的不同，对于片内FLASH、RAM的情况，直接返回pDI->pbAddr
//		对于片外FLASH或者windows模拟FLASH的情况，先执行一次读，把接收缓冲区的地址返回
const BYTE* GetItemRdAddrID(WORD wBn, WORD wPn, WORD wID, BYTE* pbBuf);

//描述：根据数据项存储类型的不同，对于片内FLASH、RAM的情况，直接返回pDI->pbAddr
//		对于片外FLASH或者windows模拟FLASH的情况，先执行一次读，把接收缓冲区的地址返回
BYTE* GetItemRdAddrDI(TDataItem* pDI, BYTE* pbBuf);

bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime);

//数据项时间的查询查询,看数据项是否在指定时间内更新了
int QueryItemTimeMbi(DWORD dwStartTime, DWORD dwEndTime, TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);
					//按照TBankItem的通用做法
int QueryItemTimeMid(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, WORD* pwID, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);
					//同一个BANK同一个测量点不同ID的查询
int QueryItemTimeMt(DWORD* pdwStartTime, DWORD* pdwEndTime, TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);
					//按照不同数据项不同时间的查询

bool WriteDbFile(char* pszPathName, BYTE* pbData, DWORD dwLen, TSem semBankRW);

void LockBank(BYTE bStorage);
void UnLockBank(BYTE bStorage);
void LockDB();
void UnLockDB();
bool IsDbLocked();
void SetDynPn(BYTE bBank, WORD wPn);

void TrigerSave();
void TrigerSavePara();

bool InitDbLib(TDbCtrl* pDbCtrl);

void DbSetFileValid(const TBankCtrl* pBankCtrl);
void DbClrFileValid(const TBankCtrl* pBankCtrl);
bool DbIsFileValid(const TBankCtrl* pBankCtrl);
void DbEnSectHolder();
void DbDisableSectHolder();

#endif //LIBDBAPI_H