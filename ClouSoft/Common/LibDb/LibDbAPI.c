/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
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
TDbData g_DbData;	//���ݿ���ʱ����

#ifdef EN_DB_SECTHOLDER //֧����������
TSectHolder g_SectHolder;	//�������ݱ��ֽṹ
#endif //EN_DB_SECTHOLDER

TSem g_semDbBuf;
TSem g_semBankRW[DB_STORAGE_TYPE_NUM];
WORD g_wDynPn[DYN_PN_NUM];	//�����洢��ǰ����Ķ�̬������
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
        mid = (little + big) >> 1;       //����

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
        mid = (little + big) >> 1;       //����

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



//���������ݴ洢����(RAM��INFLASH��EXFLASH��)�Ĳ�ͬ��ȡ�ò�ͬ���ź���
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
		g_wDynPn[bBank] = wPn;	//�����洢��ǰ����Ķ�̬������
	}
}

//������ȡ�ö�̬�洢���������ڵ�BANK
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
//����������������ʱ��
void UpdItemTime(const TBankCtrl* pBankCtrl, const TItemDesc* pItemDesc, WORD wPn, DWORD dwTime)
{
	DWORD i;
	DWORD dwTmIdx;
	BYTE bDynBn = 0;
	if (pBankCtrl->fUpdTime && (pItemDesc->wProp&DI_NTS)==0)
	{ //����BANK֧��ʱ��			&&  �������֧��ʱ��

		WORD wID, wOff;
		if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//��̬�洢���ڴ���ֻ��һ��������
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
		//���ղ�����չ����һ�������������:dwBlkIndex+������*bBlkIndexNum+bInnerIndex

		if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)	//��ID,��Ҫ���������ʱ��
		{									//��ID�Լ����е���������ĸ���
			i = dwTmIdx - (pItemDesc->bBlkIdIndexNum - 1);
			for (; i<=dwTmIdx; i++)
			{
				pBankCtrl->pbTimeFlg[wOff+(i>>3)] |= (1<<(i&7));	//ֻ���µ�ǰ���ʱ���־
			}
		}
		else
		{
			pBankCtrl->pbTimeFlg[wOff+(dwTmIdx>>3)] |= 1<<(dwTmIdx&7);	//ֻ���µ�ǰ���ʱ���־
		}
	}
}

bool IsItemTimeValid(const TBankCtrl* pBankCtrl, const TItemDesc* pItemDesc, WORD wPn, DWORD dwStartTime, DWORD dwEndTime)
{
	DWORD dwTmIdx, dwIdxSize;
	WORD wFlgPos0, wFlgPos1;
	BYTE bFlgMsk, bDynBn = 0;
	if (pBankCtrl->fUpdTime && (pItemDesc->wProp&DI_NTS)==0)
	{ //����BANK֧��ʱ��			&&  �������֧��ʱ��
		WORD wPnNum = pBankCtrl->wPnNum;
		if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//��̬�洢���ڴ���ֻ��һ��������
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
					  pItemDesc->bInnerIndex;	//���ղ�����չ����һ�������������:dwBlkIndex+������*bBlkIndexNum+bInnerIndex

		dwIdxSize = (pBankCtrl->dwIndexNum*wPnNum + 7) >> 3;    //�����������ĸ���,��dwItemNum���ղ��������չ����ĸ���

		bFlgMsk = 1<<(dwTmIdx&7);
		wFlgPos0 = dwIdxSize * (bDynBn<<1) + (dwTmIdx>>3);
		wFlgPos1 = dwIdxSize * ((bDynBn<<1) + 1) + (dwTmIdx>>3);

		if (dwStartTime == 0)
			dwEndTime = 0;

		if ((dwStartTime!=0 && pBankCtrl->pdwIntervTime[bDynBn<<1]<dwStartTime)  //��ǰ���ʱ�䲻����Ҫ��
			|| (dwEndTime!=0 && pBankCtrl->pdwIntervTime[bDynBn<<1]>=dwEndTime))
		{
			if ((dwStartTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]<dwStartTime)  //��һ���ʱ�䲻����Ҫ��
				|| (dwEndTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]>=dwEndTime))
			{
				return false;
			}
			else	//��һ���ʱ�����Ҫ��
			{
				if (pBankCtrl->pbTimeFlg[wFlgPos1] & bFlgMsk)	//ʱ���־��Ч
					return true;
				else
					return false;
			}
		}
		else 	//��ǰ���ʱ�����Ҫ��
		{
			if (pBankCtrl->pbTimeFlg[wFlgPos0] & bFlgMsk)	//ʱ���־��Ч
			{	
				return true;
			}
			else if ((dwStartTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]<dwStartTime)  //��һ���ʱ�䲻����Ҫ��
					  || (dwEndTime!=0 && pBankCtrl->pdwIntervTime[(bDynBn<<1)+1]>=dwEndTime))
			{
				return false;
			}
			else	//��һ���ʱ�����Ҫ��
			{
				if (pBankCtrl->pbTimeFlg[wFlgPos1] & bFlgMsk)	//ʱ���־��Ч
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
		g_DbData.dwFileValidFlg0 |= 1<<pBankCtrl->bBN;	//BANK0�ļ���Ч��־λ
	else
		g_DbData.dwFileValidFlg1 |= 1<<pBankCtrl->bBN;	//��չBANK�ļ���Ч��־λ
}

void DbClrFileValid(const TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->fSect)
		g_DbData.dwFileValidFlg0 &= ~(1<<pBankCtrl->bBN);	//BANK0�ļ���Ч��־λ
	else
		g_DbData.dwFileValidFlg1 &= ~(1<<pBankCtrl->bBN);	//��չBANK�ļ���Ч��־λ
}

bool DbIsFileValid(const TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->fSect)
		return (g_DbData.dwFileValidFlg0 & (1<<pBankCtrl->bBN)) != 0;	//BANK0�ļ���Ч��־λ
	else
		return (g_DbData.dwFileValidFlg1 & (1<<pBankCtrl->bBN)) != 0;	//��չBANK�ļ���Ч��־λ
}

BYTE* GetItemAddr(DWORD dwOffset, const TBankCtrl* pBankCtrl)
{
#ifndef SYS_WIN
	if (pBankCtrl->bStorage == DB_STORAGE_IN)   //�ڲ�FLASH�̶��洢
	{
		return GetFileDirAddr(pBankCtrl->wFile, dwOffset);	//ȡ���ļ���ֱ�Ӷ���ַ //(BYTE* )(pFileCfg->dwAddr + dwSectSize*wSect + dwSectOff);
	}
	else 
#endif
	if (pBankCtrl->bStorage==DB_STORAGE_RAM || pBankCtrl->bStorage==DB_STORAGE_FRAM)
	{
		return pBankCtrl->pbBankData + dwOffset;
	}

	return NULL;
}

#ifdef EN_DB_SECTHOLDER //֧����������
void DbWrHoldSect()
{
	if (g_SectHolder.fValid==false || g_SectHolder.fEnable==false)
		return;

	MakeFile(g_SectHolder.wFile, g_bDBBuf);	//���ļ����������ļ���־��У��
	if (!writefile(g_SectHolder.wFile, g_SectHolder.wSect, g_bDBBuf))
	{
		if (!writefile(g_SectHolder.wFile, g_SectHolder.wSect, g_bDBBuf))
		{
			DbClrFileValid(g_SectHolder.pBankCtrl);
			return;	//�豸������
		}
	}
//DTRACE(DB_ABB, ("DbWrHoldSect: end  at Click=%d, cost Click=%d.\n", GetClick(), GetClick()-dwLastClick));    
}

//�����������������ݱ���
void DbEnSectHolder()
{
	WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
	if (g_SectHolder.fEnable == false)
	{
		memset(&g_SectHolder, 0, sizeof(g_SectHolder));	//�������ݱ��ֽṹ
		g_SectHolder.fEnable = true;
	}
}

void DbDisableSectHolder()
{
	DbWrHoldSect();
	memset(&g_SectHolder, 0, sizeof(g_SectHolder));	//�������ݱ��ֽṹ
	SignalSemaphore(g_semDbBuf);
}
#endif //EN_DB_SECTHOLDER



int DbWriteSect(const TBankCtrl* pBankCtrl, WORD wSect, DWORD dwSectOff, BYTE* pbBuf, WORD wLen)
{
	int iFileOff;

#ifdef EN_DB_SECTHOLDER //֧����������
	if (g_SectHolder.fEnable && g_SectHolder.fValid &&	//�����������ֹ���
		g_SectHolder.wFile==pBankCtrl->wFile && g_SectHolder.wSect==wSect)	//��Ҫ�������պñ�����ס��
	{
		iFileOff = GetFileOffset(pBankCtrl->wFile, wSect);
		if (iFileOff < 0)
			goto DbWriteSect_err;

		memcpy(g_bDBBuf+iFileOff+dwSectOff, pbBuf, wLen);
	}
	else if (g_SectHolder.fEnable)	//�����������ݱ��ֹ���,ֻ�����������˸ı�
	{
		if (g_SectHolder.fValid)	//���������仯
			DbWrHoldSect();

		memset(g_bDBBuf, 0, DBBUF_SIZE);
		if (ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff) < 0)	//��ȡ�ļ����ڵĵ�dwSect������
			goto DbWriteSect_err;

		memcpy(g_bDBBuf+iFileOff+dwSectOff, pbBuf, wLen);

		g_SectHolder.fValid = true;
		g_SectHolder.wFile = pBankCtrl->wFile;
		g_SectHolder.wSect = wSect;
		g_SectHolder.pBankCtrl = pBankCtrl;
	}
	else //û�����������ݱ��ֹ���
#endif //EN_DB_SECTHOLDER
	{
		WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);

		memset(g_bDBBuf, 0, DBBUF_SIZE);
		if (ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff) < 0)	//��ȡ�ļ����ڵĵ�dwSect������
		{
			SignalSemaphore(g_semDbBuf);
			return -ERR_DEV;	//�豸������
		}

		memcpy(g_bDBBuf+iFileOff+dwSectOff, pbBuf, wLen);

		MakeFile(pBankCtrl->wFile, g_bDBBuf);	//���ļ����������ļ���־��У��
		if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
		{
			if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
			{
				DbClrFileValid(pBankCtrl);

				SignalSemaphore(g_semDbBuf);
				return -ERR_DEV;	//�豸������
			}
		}

		SignalSemaphore(g_semDbBuf);
	}

	return wLen;

DbWriteSect_err:
	g_SectHolder.fValid = false;	//�������ݱ�����Ч
	return -ERR_DEV;	//�豸������
}

int DbReadData(DWORD dwOffset, BYTE* pbBuf, WORD wLen, const TBankCtrl* pBankCtrl)
{
	DWORD dwSectSize, dwSectOff, dwUsedSize;
	WORD wSect, wCpyLen;
	if (pBankCtrl->bStorage==DB_STORAGE_IN || pBankCtrl->bStorage==DB_STORAGE_EX)   //�ڲ�FLASH�̶��洢 || �ⲿFLASH�̶��洢
	{
		if (DbIsFileValid(pBankCtrl))
		{
			dwSectSize = GetSectSize(pBankCtrl->wFile);	//ȡ���ļ������������С
			dwUsedSize = GetSectUsedSize(pBankCtrl->wFile);//ȡ���ļ�ʵ��ʹ�õ�������С
            if (dwUsedSize == 0)
                return -ERR_DEV;
			wSect = dwOffset / dwUsedSize;
			dwSectOff = dwOffset % dwUsedSize;
			if (dwSectOff+wLen > dwUsedSize)	//������
			{
				wCpyLen = dwUsedSize - dwSectOff;
				if (!readfile(pBankCtrl->wFile, dwSectSize*wSect+dwSectOff, pbBuf, wCpyLen))
					return -ERR_DEV;	//�豸������
				if (!readfile(pBankCtrl->wFile, dwSectSize*(wSect+1), pbBuf+wCpyLen, wLen-wCpyLen))
					return -ERR_DEV;	//�豸������
			}
			else
			{
				if (!readfile(pBankCtrl->wFile, dwSectSize*wSect+dwSectOff, pbBuf, wLen))
					return -ERR_DEV;	//�豸������
			}

			return wLen;
		}
		else
		{
			return -ERR_DEV;	//�豸������
		}
	}
	else if (pBankCtrl->bStorage==DB_STORAGE_RAM || 
			 pBankCtrl->bStorage==DB_STORAGE_DYN ||
			 pBankCtrl->bStorage==DB_STORAGE_FRAM)
	{
		memcpy(pbBuf, pBankCtrl->pbBankData+dwOffset, wLen);
		return wLen;
	}

	return -ERR_DEV;	//�豸������
}

int DbWriteData(DWORD dwOffset, BYTE* pbBuf, WORD wLen, const TBankCtrl* pBankCtrl, TItemAcess* pItemAcess, const TItemDesc* pItemDesc)
{
	DWORD dwUsedSize, dwSectOff;
	WORD wSect, wCpyLen;

	if (pItemDesc != NULL)
	{
		if (pItemDesc->wProp & DI_NSP) //No Space ��ռ�ÿռ䣬ʹ���䳤����Ϣ����ռ��ʱ���־
			return wLen;
	}

	if (pBankCtrl->bStorage==DB_STORAGE_IN || pBankCtrl->bStorage==DB_STORAGE_EX)   //�ڲ�FLASH�̶��洢 || �ⲿFLASH�̶��洢
	{
		if (!DbIsFileValid(pBankCtrl))
		{
			if (DbLoadBankDefault(pBankCtrl))
				return -ERR_DEV;	//�豸������
		}

		dwUsedSize = GetSectUsedSize(pBankCtrl->wFile); //ȡ���ļ�ʵ��ʹ�õ�������С
        if (dwUsedSize == 0)
            return -ERR_DEV;
		wSect = dwOffset / dwUsedSize;
		dwSectOff = dwOffset % dwUsedSize;
		if (dwSectOff+wLen > dwUsedSize)	//����������������������ο�����ֻ�������ļ������������
		{
			//��һ������
			wCpyLen = dwUsedSize - dwSectOff;
			if (DbWriteSect(pBankCtrl, wSect, dwSectOff, pbBuf, wCpyLen) < 0)
				return -ERR_DEV;	//�豸������

   			//�ڶ�������
            if (DbWriteSect(pBankCtrl, wSect+1, 0, pbBuf+wCpyLen, wLen-wCpyLen) < 0) //�������Ŀ϶�����ҳ�ļ������ÿ���dwFileOff
                return -ERR_DEV;	//�豸������

			return wLen;
		}
		else	//������������ͬһ������һ�ο����Ϳ���
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
		else if (pItemAcess->bType==DI_ACESS_UPD) //ֻ��RAM���ݣ���֧�ָ���������״̬
			memset(pBankCtrl->pbBankData+dwOffset, GetInvalidData((BYTE )pItemAcess->dwVal), wLen);
		else
			memcpy(pBankCtrl->pbBankData+dwOffset, pbBuf, wLen);

		return wLen;
	}

	return -ERR_DEV;	//�豸������
}

//����:�����ȷ�򷵻����ݵĳ��ȣ�����������8λ���ش�����룬�ε�8λ�������ݵĳ���
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
		(pBankCtrl->bStorage==DB_STORAGE_RAM || pBankCtrl->bStorage==DB_STORAGE_DYN || pBankCtrl->bStorage==DB_STORAGE_FRAM)) //��BANK��ֻ��Ϊ������������,���������ݷ���Ҫ����Ӧ�Ķ�д����
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum == 0)	// && pBankCtrl->wImgNum==0
		return -ERR_ITEM; //��BANK��ֻ��Ϊ������������,ֻ֧��DI_ACESS_INFO��ReadItem()����

	pItemDesc = &pBankCtrl->pItemDesc[nIndex];

	if ((pItemDesc->wProp & DI_CMB) != 0) //���ID���ܽ��к����ķ���
		return -ERR_ITEM;

#ifdef EN_DB_PNMAP
	//�����㵽ʵ�ʴ洢��(ӳ���)��ת��
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

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //�������֧����ô���������
		return -ERR_ITEM;
	
	if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//��̬�洢���ڴ���ֻ��һ��������
	{
		bDynBn = GetBankOfDynPn(wPn);
		if (bDynBn >= DYN_PN_NUM)
			return -ERR_ITEM;

		//wPn = 0;
		dwOffset = pBankCtrl->dwBankSize*bDynBn + pItemDesc->dwBlockOffset + pItemDesc->wOffset;
				//һ���������ƫ��:dwBlockOffset+������*wBlockLen+wOffset
	}
	else
	{
		dwOffset = pItemDesc->dwBlockOffset + wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
				//һ���������ƫ��:dwBlockOffset+������*wBlockLen+wOffset
	}

	if (pItemAcess->bType == DI_ACESS_BUF)
	{
		LockBank(pBankCtrl->bStorage);
		iRet = DbWriteData(dwOffset, pItemAcess->pbBuf, pItemDesc->wLen, pBankCtrl, pItemAcess, pItemDesc);
#ifdef EN_DB_QRYTM
		UpdItemTime(pBankCtrl, pItemDesc, wPn, dwTime);	//����������ʱ��
#endif //EN_DB_QRYTM
		UnLockBank(pBankCtrl->bStorage);
	}
#ifdef EN_DB_QRYTM
	else if (pItemAcess->bType == DI_ACESS_UPD) //����������״̬
	{
		LockBank(pBankCtrl->bStorage);
		iRet = DbWriteData(dwOffset, pItemAcess->pbBuf, pItemDesc->wLen, pBankCtrl, pItemAcess, pItemDesc);
		UpdItemTime(pBankCtrl, pItemDesc, wPn, dwTime);	//����������ʱ��
		UnLockBank(pBankCtrl->bStorage);
	}
#endif //EN_DB_QRYTM
	else //if (pItemAcess->bType==DI_ACESS_INT32 || pItemAcess->bType==DI_ACESS_INT64)	//��ֵ
	{
		return -ERR_ITEM;
	}

	//TODO�������־

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

	if ((pItemDesc->wProp & DI_CMB) != 0) //���ID���ܽ��к����ķ���
		return -ERR_ITEM;

	if (pItemAcess->bType == DI_ACESS_INFO)	//ȡ�������
	{	//���wPnNum��wImgNumͬʱ����Ϊ0,��ʾ��BANK��ֻ��Ϊ������������
		if (pBankCtrl->wPnNum > 1)
			pItemAcess->pItemInfo->wPnNum = pBankCtrl->wPnNum;
		else
			pItemAcess->pItemInfo->wPnNum = pItemDesc->wPnNum;

		pItemAcess->pItemInfo->wLen = pItemDesc->wLen;
		return pItemDesc->wLen;
	}

	if (pBankCtrl->pbBankData==NULL &&
		(pBankCtrl->bStorage==DB_STORAGE_RAM || pBankCtrl->bStorage==DB_STORAGE_DYN || pBankCtrl->bStorage==DB_STORAGE_FRAM)) //��BANK��ֻ��Ϊ������������,���������ݷ���Ҫ����Ӧ�Ķ�д����
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum == 0)	 //&& pBankCtrl->wImgNum==0
		return -ERR_ITEM; //��BANK��ֻ��Ϊ������������,ֻ֧��DI_ACESS_INFO��ReadItem()����

#ifdef EN_DB_PNMAP
	//�����㵽ʵ�ʴ洢��(ӳ���)��ת��
	bPnMapSch = 0;
	if (pBankCtrl->bPnMapSch != 0)
		bPnMapSch = pBankCtrl->bPnMapSch;
	else if (pItemDesc->bPnMapSch != 0)
		bPnMapSch = pItemDesc->bPnMapSch;

	if (bPnMapSch!=0 && pItemAcess->bType!=DI_ACESS_RDUNMAP)	//���շ�ӳ��ķ�ʽ��
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

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //�������֧����ô���������
		return -ERR_ITEM;
	
/*	wBlkIndexNum = 1;
	if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)
	{							//��ID�Լ����е���������ĸ���
		wBlkIndexNum = pItemDesc->bBlkIdIndexNum - 1;
	}*/
	//TODO:����ļ��Ƿ���Ч������Ƭ��FLASH�Ƿ�д�ã�Ƭ��FLASH�Ѿ�����RAM
#ifdef EN_DB_QRYTM
	if (!IsItemTimeValid(pBankCtrl, pItemDesc, wPn, dwStartTime, dwEndTime))
	{
		if (pItemAcess->bType==DI_ACESS_BUF || pItemAcess->bType==DI_ACESS_RDUNMAP)
		{
			memset(pItemAcess->pbBuf, GetInvalidData(ERR_APP_OK), pItemDesc->wLen);
		}
		else if (pItemAcess->bType == DI_ACESS_INT32) //��������32λ��,��ȫ��������ΪINVALID_VAL
		{
		}
		else if (pItemAcess->bType == DI_ACESS_INT64) //��������64λ��,��ȫ��������ΪINVALID_VAL64
		{
		}

		return -(ERR_TIME + (int )pItemDesc->wLen*0x100);
	}
#endif //EN_DB_QRYTM

	if (pBankCtrl->bStorage == DB_STORAGE_DYN)	//��̬�洢���ڴ���ֻ��һ��������
	{
		bDynBn = GetBankOfDynPn(wPn);
		if (bDynBn >= DYN_PN_NUM)
			return -ERR_ITEM;
	
		//wPn = 0;
		dwOffset = pBankCtrl->dwBankSize*bDynBn + pItemDesc->dwBlockOffset + pItemDesc->wOffset;
				//һ���������ƫ��:dwBlockOffset+������*wBlockLen+wOffset
	}
	else
	{
		dwOffset = pItemDesc->dwBlockOffset + wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
					//һ���������ƫ��:dwBlockOffset+������*wBlockLen+wOffset
	}

	if (pItemAcess->bType==DI_ACESS_BUF || pItemAcess->bType==DI_ACESS_RDUNMAP)
	{
		LockBank(pBankCtrl->bStorage);
		iRet = DbReadData(dwOffset, pItemAcess->pbBuf, pItemDesc->wLen, pBankCtrl);
		UnLockBank(pBankCtrl->bStorage);
		return iRet; 
	}
	else if (pItemAcess->bType==DI_ACESS_INT32 || pItemAcess->bType==DI_ACESS_INT64)	//��ֵ
	{
	}
#ifdef EN_DB_QRYTM
	else if (pItemAcess->bType == DI_ACESS_QRY) //��ѯ�������Ƿ����,ʲô�����ø�
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
//������ͨ���������ڳ������֧�ֵ�������򳭱�ʧ�ܵ�ʱ��,����������ʱ��,
//		�Լӿ�Ӧ�ò�ѯ���ݵļ�ʱ��
//������@wErr �������,ERR_UNSUP���֧�ֵ�������,ERR_FAIL����ʧ��
//��ע��Ŀǰֻ֧�ֵ�����������ʱ��,��֧�ָ��´������
bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime)
{
	TItemAcess ItemAcess;
	if (wBank != BN0)
		return true;
	
	ItemAcess.bType = DI_ACESS_UPD;	//����������״̬
	ItemAcess.dwVal = wErr;

	return DbWriteItemEx(wBank, wPn, wID, &ItemAcess, dwTime)>0;
}

//��������ѯ�������Ƿ���dwStartTime�󱻸��¹�
//������@dwStartTime ����������ʼʱ��,��2000��1��1��0��0��0���������
//					 ע����SubmitMeterReq()��dwStartTime������ͬ
//		@dwEndTime   С����Ӧ�Ľ���ʱ��
//      @pBank0Item ָ�������������ָ��
//      @wNum ����Ԫ�صĸ���
// 		@pbBuf ��ϵͳ����õĻ���
//		@pwValidNum �������غϷ�������ĸ���
//���أ��Ѿ���ȷ�ϵ����������,�����Ѿ�����,ȷ�ϲ�֧�ֵĻ�ȷ�ϳ�������������
//��ע����������¹���һ�������������ݺϷ�,���֧�ֻ򳭱�ʧ�ܵ����������
//		��UpdItemErr()����ʱ��,�Լӿ�Ӧ�ò�ѯ���ݵļ�ʱ��
int QueryItemTimeMbi(DWORD dwStartTime, DWORD dwEndTime, TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum)
{
	TItemAcess ItemAcess;
	int iRet;
	int iConfirmNum;
	WORD i, wValidNum;

	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_QRY;
	ItemAcess.pbBuf = pbBuf;	//��ϵͳ����õĻ���

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
			int iLen;	//�����������������ĳ���
			WORD wID;
			bool fInvalid = false;
			iRet = 0;

			while ((wID=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
			{
				iLen = DbReadItemEx(pBankItem->wBn, pBankItem->wPn, wID, &ItemAcess, dwStartTime, dwEndTime);
				
				if (iLen > 0)
				{
					iRet += iLen;
				}
				else if (iLen < 0)
				{
					iLen = -iLen;	//-(ERR_TIME + (int )pItemDesc->wLen*0x100);
					if ((iLen&0xff) != ERR_INVALID)  //ʱ�䱻������,��������������Ч,������֧�ֵ�������򳭱�ʧ�ܵ�
					{
						//��������������ERR_ITEM/ERR_TIME�ȴ���,�����ID��ʣ��ID�Ͳ����ٲ�ѯ��,
						//��Ϊ���������Ѿ����ײ�����Ҫ����
						fInvalid = false;	//��������,���ܹ�ΪERR_INVALID
						iRet = -iLen;
						break;	
					}
					
					iRet += (iLen>>8);
					fInvalid = true;
				}
				else //iLen==0
				{
					fInvalid = false;	//��������,���ܹ�ΪERR_INVALID
					iRet = 0;
					break;
				}
			}

			if (fInvalid && wID==0)	//ȫ��������������,ȷ��ֻʣ��ERR_INVALID����
				iRet = -(ERR_INVALID + iLen*0x100);
		}

		if (iRet > 0)
		{
			iConfirmNum++;
			wValidNum++;
		}
		else
		{
			iRet = (-iRet) & 0xff;	  //ȡ�������
			if (iRet == ERR_INVALID)  //ʱ�䱻������,��������������Ч,������֧�ֵ�������򳭱�ʧ�ܵ�
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

//����:���ղ�ͬ�����ͬʱ��Ĳ�ѯ,���������Ƿ���pdwStartTime�󱻸��¹�
//����:@pdwStartTime ������ʱ�������
// 		@pbBuf ��ϵͳ����õĻ���
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
	TBankItem BankItem[10]; //�������  50
	int iRet = 0;
	int iConfirmNum;
	WORD i, wValidRet=0, wValidNum=0;
	while (wNum > 0)
	{
		WORD n = wNum>=10 ? 10 : wNum;

		for (i=0; i<n; i++)
		{
			BankItem[i].wBn = wBn;  	//�������
			BankItem[i].wPn = wPn;  	//�������
			BankItem[i].wID = *pwID++;  //BN0����645ID
		}

		iConfirmNum = QueryItemTimeMbi(dwStartTime, dwEndTime, BankItem, n, pbBuf, &wValidNum); //��ѯ�����Ƿ����
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
		iRet = PostWriteItemExHook(wBank, wPn, wID, pbBuf, iRet); //���ùҹ�����Ӧ�����⴦��
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

	if (pwSubID == NULL)	//��ID,�������ID
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
	else //���ID
	{
		int iLen; //�����������������ĳ���
		WORD id;
		BYTE bErr = 0;
		iRet = 0;

		while ((id=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
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
				if (iLen == -ERR_ITEM)	//��֧�ֵ�������
					return -ERR_ITEM;
				
				//֧�ֵ�������,�������˴���
				iLen = -iLen;	//-(ERR_TIME + (int )pItemDesc->wLen*0x100);
				bErr = iLen & 0xff; //Ŀǰ����ֻ�ܱ���һ��
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
				//��ʹ�ڲ��ִ���������,��Ҫ�������������������Ӧ����ID
	}	
#endif //EN_DB_CMB

	return iRet;
}

#ifdef EN_DB_PNMAP
//���շ�ӳ��ķ�ʽ��
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{	
	int iRet;
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_RDUNMAP;		//���շ�ӳ��ķ�ʽ��
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
//����:���ID�İ���������
int ReadItemMid(WORD wBank, WORD wPn, const WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	WORD i;
	for (i=0; i<wNum; i++)
	{
		int iRet = ReadItemTm(wBank, wPn, *pwID++, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//����ȷ���������
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//ȡ�������
										//���������Ѿ����������Ч����
		}

		iLen += iRet;
		pbBuf += iRet;
	}

	return iLen;
}

//����:���ID�İ���������
int ReadItemMbi(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	WORD i;
	for (i=0; i<wNum; i++)
	{
		int iRet = ReadItemTm(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//����ȷ���������
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//ȡ�������
										//���������Ѿ����������Ч����
		}

		iLen += iRet;
		pbBuf += iRet;
		pBankItem++;
	}

	return iLen;
}

//����:���ID����ͬ��ʱ��Ļ�������
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
			if (iRet == -ERR_ITEM)		//����ȷ���������
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//ȡ�������
										//���������Ѿ����������Ч����
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

	ItemAcess.bType = DI_ACESS_INFO;	//ȡ��������Ϣ(���ȺͶ�)
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

	if (pwSubID == NULL)	//��ID,�������ID
	{	
		return GetItemInfo(wBn, wID, &ItemInfo);
	}
	else //���ID
	{
		int iLen; //�����������������ĳ���
		WORD id;
		iRet = 0;

		while ((id=*pwSubID++) != 0) //�����IDת�������ζ���ID�Ķ�
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
		if (wID==0x8ffe && nLastLen!=0) //����Ƕ�Ӧ��
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
		if (pBankItem->wBn==BN0 && pBankItem->wID==0x8ffe && nLastLen!=0) //����Ƕ�Ӧ��
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

//����:ȡ��������Ķ���ַ,��Ҫ�����Щ���ݱȽϳ���������,�������������λ��,
//	   ��ȥ�������ݿ�����ʱ������,ֱ�ӷ���ֻ�����ڴ��ַ,����Ҳ�����ƻ�ϵͳ�������
//����:�����ȷ�򷵻�������ĵ�ַ,���򷵻�NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID)
{
	TDataItem di = GetItemEx(wBn, wPn, wID);
	return di.pbAddr;
}


//����:ȡ��������Ķ���ַ,����Ƭ��FLASH��RAM�������ֱ�ӷ���pDI->pbAddr
//		����Ƭ��FLASH����windowsģ��FLASH���������ִ��һ�ζ����ѽ��ջ������ĵ�ַ����
//����:�����ȷ�򷵻�������ĵ�ַ,���򷵻�NULL
const BYTE* GetItemRdAddrID(WORD wBn, WORD wPn, WORD wID, BYTE* pbBuf)
{
	TDataItem di = GetItemEx(wBn, wPn, wID);

#ifndef SYS_WIN		//windows�²���ֱ�Ӱ���ַ��ȡ���ݣ�������Ȱ����ݶ����ڴ��в��ܷ���
	return di.pbAddr;
#else
	if (pbBuf == NULL)
		return NULL;

	ReadItemDI(&di, pbBuf);
	return pbBuf;
#endif
}

//����������������洢���͵Ĳ�ͬ������Ƭ��FLASH��RAM�������ֱ�ӷ���pDI->pbAddr
//		����Ƭ��FLASH����windowsģ��FLASH���������ִ��һ�ζ����ѽ��ջ������ĵ�ַ����
BYTE* GetItemRdAddrDI(TDataItem* pDI, BYTE* pbBuf)
{
#ifndef SYS_WIN		//windows�²���ֱ�Ӱ���ַ��ȡ���ݣ�������Ȱ����ݶ����ڴ��в��ܷ���
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

//��ָ��ʱ���������������,������Ķ�����
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf) 
{
	return ReadItemTm(wBank, wPn, wID, pbBuf, INVALID_TIME, INVALID_TIME); //ָ��ʱ���
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

//����:��ʼ��ϵͳ��Ĵ����
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
	
	//memset(g_wDynPn, 0xff, sizeof(g_wDynPn));	//�����洢��ǰ����Ķ�̬������
	//g_wDynPn = 0xff;

#ifdef EN_DB_SECTHOLDER //֧����������
	memset(&g_SectHolder, 0, sizeof(g_SectHolder));	//�������ݱ��ֽṹ
#endif //EN_DB_SECTHOLDER

	return DbInit(pDbCtrl);
}