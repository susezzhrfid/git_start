/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
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
extern TDbData g_DbData;	//���ݿ���ʱ����

#ifdef EN_DB_SECTHOLDER //֧����������
extern TSectHolder g_SectHolder;	//�������ݱ��ֽṹ
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

int WriteItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf); //��ָ��ʱ��д���������д����
int WriteItemTm(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime);

//����ID���������Ķ�
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf); //��ָ��ʱ���������������,������Ķ�����
int ReadItemTm(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime); //ָ��ʱ���

int ReadItemGetTime(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime); //������ͬʱȡ���ݿ��е�ʱ��

//���ID�Ķ��ӿ�,Ŀǰֻ֧�ְ��������Ķ�
int ReadItemMid(WORD wBank, WORD wPn, const WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime); //����ͬ��������ͬʱ���
int ReadItemMbi(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);	//����ͬʱ���
int ReadItemMbiMt(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD* pdwTime);	//����ͬʱ���

//���շ�ӳ��ķ�ʽ��
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);

TDataItem GetItemEx(WORD wBank, WORD wPoint, WORD wID);

int GetItemInfo(WORD wBn, WORD wID, TItemInfo* pItemInfo);
int GetItemLen(WORD wBn, WORD wID);
int GetItemsLenId(WORD* pwItemID, WORD wLen);
int GetItemsLenBi(TBankItem* pBankItem, WORD wNum);
int GetItemPnNum(WORD wBn, WORD wID);

//����:ȡ��������Ķ���ַ,��Ҫ�����Щ���ݱȽϳ���������,�������������λ��,
//	   ��ȥ�������ݿ�����ʱ������,ֱ�ӷ���ֻ�����ڴ��ַ,����Ҳ�����ƻ�ϵͳ�������
//����:�����ȷ�򷵻�������ĵ�ַ,���򷵻�NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID);

//����������������洢���͵Ĳ�ͬ������Ƭ��FLASH��RAM�������ֱ�ӷ���pDI->pbAddr
//		����Ƭ��FLASH����windowsģ��FLASH���������ִ��һ�ζ����ѽ��ջ������ĵ�ַ����
const BYTE* GetItemRdAddrID(WORD wBn, WORD wPn, WORD wID, BYTE* pbBuf);

//����������������洢���͵Ĳ�ͬ������Ƭ��FLASH��RAM�������ֱ�ӷ���pDI->pbAddr
//		����Ƭ��FLASH����windowsģ��FLASH���������ִ��һ�ζ����ѽ��ջ������ĵ�ַ����
BYTE* GetItemRdAddrDI(TDataItem* pDI, BYTE* pbBuf);

bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime);

//������ʱ��Ĳ�ѯ��ѯ,���������Ƿ���ָ��ʱ���ڸ�����
int QueryItemTimeMbi(DWORD dwStartTime, DWORD dwEndTime, TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);
					//����TBankItem��ͨ������
int QueryItemTimeMid(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, WORD* pwID, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);
					//ͬһ��BANKͬһ�������㲻ͬID�Ĳ�ѯ
int QueryItemTimeMt(DWORD* pdwStartTime, DWORD* pdwEndTime, TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);
					//���ղ�ͬ�����ͬʱ��Ĳ�ѯ

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