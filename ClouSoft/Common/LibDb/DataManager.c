/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DataManager.c
 * ժ    Ҫ�����ļ���Ҫʵ��ϵͳ���ݿ���������д�洢����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
*********************************************************************************************************/
#include "FaCfg.h"
#include "LibDbCfg.h"
#include "DataManager.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "DbHook.h"
#include "Trace.h"
//#include "sysapi.h"
#include "FileMgr.h"
#include "SysDebug.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//ϵͳ��˽�г�Ա����
static TDbCtrl* 	m_pDbCtrl;		//�������ݿ���в������õ����ݿ���ƽṹ
static WORD			m_wSectNum;		//BANK0�е�SECT��Ŀ
static const TBankCtrl* 	m_pBank0Ctrl;
static WORD 		m_wBankNum;		//֧�ֵ�BANK��Ŀ
static const TBankCtrl* 	m_pBankCtrl;
static WORD 		m_wPnMapNum;  	//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
static TPnMapCtrl*	m_pPnMapCtrl; 	//�������ݿⲻ֧�ֲ����㶯̬ӳ������ΪNULL
static TSem			m_semPnMap;
static DWORD		m_dwSaveClick;
static bool			m_fTrigerSaveAll;
static bool			m_fTrigerSavePara;
static bool			m_fTrigerSaveBank;
static DWORD		m_dwPnMapFileFlg;	//ÿ1λ��1��ʾһ���ļ��������޸�

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//���ֺ�������
bool InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//ϵͳ�⺯��
//����:��ĳ��Bank��section�����ݻָ���Ĭ��ֵ
//����:@wBank Bank��
//	   @wSect Section��
bool DbClearBankData(WORD wBank, WORD wSect)
{
	const TBankCtrl	*pBankCtrl;

	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	return DbLoadBankDefaultFLASH(pBankCtrl, 0);
}

//����:װ��1���ļ���Ĭ������,
//		1.���������д��ڶ�������������,��ʵװ���������BANK�Ķ���������Ĭ��ֵ,
//		  ��װ����Ǹ��������������Ķ���������Ĭ������;
//		2.���������в����ڶ�������������ʱ,��װ����Ǹ���������������1���������Ĭ������
//		  (������TBankCtrl�еĶ���������е�һ��)
//����:@pBankCtrl BANK���ƽṹ
//	   @wFile �ļ���,�ֱ��Ӧ������Ż��߾���� 
//	   @dwOffset һ���ļ����ƫ��,��Ҫ�����Щ�汾�����˸ı��BANK,
//				��������չ���µ�������,��װ���ļ����ݵ�ʱ���Ѿ�װ����
//				ǰ��ԭ�еĲ���,������¼ӵĲ���װ��Ĭ����
//				������Щ�汾û�з����ı��BANK,�ѱ�������Ϊ0
bool DbLoadBankDefaultRAM(const TBankCtrl* pBankCtrl, DWORD dwOffset)
{
	const TItemDesc* pItemDesc;
	DWORD num;
	BYTE* pbDst;
	BYTE* pbDst0;
	const BYTE* pbSrc;
	DWORD i;
	WORD j;

	if (pBankCtrl->pbDefault == NULL)	//��BANKû��Ĭ��ֵ,ֱ��ȡ0
	{
		memset(pBankCtrl->pbBankData+dwOffset, 0, pBankCtrl->dwFileSize-dwOffset);
		goto ret_ok;
	}

	if (!pBankCtrl->fMutiPnInDesc)  //�������в����ڶ�������������
	{
		memcpy(pBankCtrl->pbBankData+dwOffset, pBankCtrl->pbDefault+dwOffset,
			   pBankCtrl->dwFileSize-dwOffset);

		goto ret_ok;
	}

	pItemDesc = pBankCtrl->pItemDesc;
	num = pBankCtrl->dwItemNum;
	pbDst = pBankCtrl->pbBankData;
	pbDst0 = pbDst;
	pbSrc = pBankCtrl->pbDefault;
	for (i=0; i<num; i++)
    {
		if ((pItemDesc[i].wProp&DI_SELF) && (pItemDesc[i].wProp&DI_NSP)==0)		 //�����������Գɶ���������
		{
			for (j=0; j<pItemDesc[i].wPnNum; j++)
			{
				if ((DWORD )(pbDst-pbDst0) >= dwOffset)
					memcpy(pbDst, pbSrc, pItemDesc[i].wLen);
					
				pbDst += pItemDesc[i].wLen;
			}

			pbSrc += pItemDesc[i].wLen;
		}
	}

ret_ok:
	DTRACE(DB_CRITICAL, ("DbLoadBankDefaultRAM: load default for bank %s ok.\r\n", pBankCtrl->pszBankName));
	return true;
}



//����:װ��1��BANK��Ĭ������,
//����:@pBankCtrl BANK���ƽṹ
//	   @iFile �ļ���,�ֱ��Ӧ������Ż��߾����,С��0ʱ��ʾװ����BANKĬ������ 
//	   @dwOffset һ���ļ����ƫ��,��Ҫ�����Щ�汾�����˸ı��BANK,
//				��������չ���µ�������,��װ���ļ����ݵ�ʱ���Ѿ�װ����
//				ǰ��ԭ�еĲ���,������¼ӵĲ���װ��Ĭ����
//				������Щ�汾û�з����ı��BANK,�ѱ�������Ϊ0
bool DbLoadBankDefaultFLASH(const TBankCtrl* pBankCtrl, DWORD dwOffset)
{
	int iRet;
	WORD i, j;
	DWORD dwUsedSize;
	WORD wSect, wSectNum;
	const TItemDesc* pItemDesc = pBankCtrl->pItemDesc;
	DWORD num = pBankCtrl->dwItemNum;
	BYTE* pbDst = g_bDBBuf;
	BYTE* pbDst0 = pbDst;
	const BYTE* pbSrc = pBankCtrl->pbDefault;
	WORD wCpyLen; 
	int iFileOff;
	DWORD dwOff = 0;	//���������У���������ļ���ͷ��ƫ�ƣ��������Ĭ��ֵ�Ƿ��ȡ

	WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);

	wSectNum = GetFileSectNum(pBankCtrl->wFile);	//ȡ���ļ��������������
	dwUsedSize = GetSectUsedSize(pBankCtrl->wFile);//ȡ���ļ�ʵ��ʹ�õ�������С
										
	if (pBankCtrl->pbDefault == NULL)	//��BANKû��Ĭ��ֵ,ֱ��ȡ0
	{
		for (wSect=0; wSect<wSectNum; wSect++)
		{
			memset(g_bDBBuf, 0, DBBUF_SIZE);
			iRet = ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff);	//��ȡ�ļ����ڵĵ�dwSect������
			if (iRet < 0)
				goto ret_err_rd;

			memset(g_bDBBuf+iFileOff, 0, dwUsedSize); //�����п��ܻ����
			MakeFile(pBankCtrl->wFile, g_bDBBuf);	//���ļ����������ļ���־��У��
			if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
				goto ret_err_wr;
		}

        SignalSemaphore(g_semDbBuf);
		return true;
	}

	wSect = 0;	//�ļ��ĵڼ������������ڰ�ҳ����ģ�Ӧ��Ϊ0
	memset(g_bDBBuf, 0, DBBUF_SIZE);
	iRet = ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff);	//��ȡ�ļ����ڵĵ�dwSect������
	if (iRet < 0)
		goto ret_err_rd;

	if (!CheckFile(pBankCtrl->wFile, g_bDBBuf, iFileOff))	//�ļ�У�����
	{
		memset(g_bDBBuf+iFileOff, 0, dwUsedSize);
		dwOffset = 0;	//����ļ�У�������Ĭ��ֵȫ������ȡ
	}

	pbDst = g_bDBBuf + iFileOff;
	pbDst0 = pbDst;

	for (i=0; i<num; i++)	//ÿ��������Ŀ���
    {
		if ((pItemDesc[i].wProp&DI_SELF) && (pItemDesc[i].wProp&DI_NSP)==0)		 //�����������Գɶ���������
		{
			for (j=0; j<pItemDesc[i].wPnNum; j++)	//ÿ��������ĸ�������Ŀ���
			{
				wCpyLen = pItemDesc[i].wLen;
				if (pbDst-pbDst0+wCpyLen > dwUsedSize)	//һ��������ʣ���ֽڵĿ���
					wCpyLen = dwUsedSize - (pbDst-pbDst0);
				
				if (dwOff >= dwOffset)
					memcpy(pbDst, pbSrc, wCpyLen);
					
				pbDst += wCpyLen;
				dwOff += wCpyLen;	//���������У���������ļ���ͷ��ƫ�ƣ��������Ĭ��ֵ�Ƿ��ȡ

				if (pbDst-pbDst0 >= dwUsedSize)	//�����ļ�Ҫ�������ˣ�����ҳ�ļ�������
				{
					MakeFile(pBankCtrl->wFile, g_bDBBuf);	//���ļ����������ļ���־��У��
					if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
					{
						if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
							goto ret_err_wr;
					}

					if (IsPageFile(pBankCtrl->wFile))	//��ҳ������ļ�,Ӧ�������������
					{
						if (j+1!=pItemDesc[i].wPnNum || i+1!=num)	//�����û��
						{
							DTRACE(DB_CRITICAL, ("DbLoadBankDefaultFLASH: fail to load default for bank %s, due to def & desc mismatch.\r\n", pBankCtrl->pszBankName));
                            SignalSemaphore(g_semDbBuf);
							return false;
						}
						else
                        {
                            SignalSemaphore(g_semDbBuf);
							return true;
                        }
					}

					wSect++;
					memset(g_bDBBuf, 0, DBBUF_SIZE);
					iRet = ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff);	//��ȡ�ļ����ڵĵ�dwSect������
					if (iRet < 0)
						goto ret_err_rd;

					if (!CheckFile(pBankCtrl->wFile, g_bDBBuf, iFileOff))	//�ļ�У�����
					{
						memset(g_bDBBuf+iFileOff, 0, dwUsedSize);
						dwOffset = 0;
					}

					pbDst = g_bDBBuf + iFileOff;
					pbDst0 = pbDst;
				}

				if (wCpyLen < pItemDesc[i].wLen)	//һ��������ʣ���ֽڵĿ���
				{
					if (dwOff >= dwOffset)
						memcpy(pbDst, pbSrc+wCpyLen, pItemDesc[i].wLen-wCpyLen);

					pbDst += pItemDesc[i].wLen-wCpyLen;
					dwOff += pItemDesc[i].wLen-wCpyLen;
				}
			}

			pbSrc += pItemDesc[i].wLen;
		}
	}

	MakeFile(pBankCtrl->wFile, g_bDBBuf);	//���ļ����������ļ���־��У��
	if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
	{
		if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
			goto ret_err_wr;
	}

//ret_ok:
    SignalSemaphore(g_semDbBuf);
	DTRACE(DB_CRITICAL, ("DbLoadBankDefaultFLASH: load default for bank %s ok.\r\n", pBankCtrl->pszBankName));    
	return true;

ret_err_wr:
    SignalSemaphore(g_semDbBuf);
	DTRACE(DB_CRITICAL, ("DbLoadBankDefaultFLASH: fail to write default for bank %s.\r\n", pBankCtrl->pszBankName));    
	return false;

ret_err_rd:
    SignalSemaphore(g_semDbBuf);
	DTRACE(DB_CRITICAL, ("DbLoadBankDefaultFLASH: fail to read default for bank %s.\r\n", pBankCtrl->pszBankName));   
	return false;
}

bool DbLoadBankDefault(const TBankCtrl* pBankCtrl)
{
	WORD wSect;
	if (pBankCtrl->pItemDesc == NULL)  //��Ч�����ݿ��������
		return true;

#ifdef EN_DB_SECTHOLDER //֧����������
	if (g_SectHolder.fEnable == false)	//�ڽ�ֹ�������ֵ�����£������Լ�����/�ͷ��ź���
#endif //EN_DB_SECTHOLDER

    if (pBankCtrl->bStorage==DB_STORAGE_IN || pBankCtrl->bStorage==DB_STORAGE_EX)   //�ڲ�FLASH�̶��洢 || �ⲿFLASH�̶��洢
    {
        //����ļ��Ƿ�����        
        DWORD dwSectSize = GetSectSize(pBankCtrl->wFile);	//ȡ���ļ������������С
        WORD wSectNum = GetFileSectNum(pBankCtrl->wFile);	//ȡ���ļ��������������
        
        WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
        
        for (wSect=0; wSect<wSectNum; wSect++)
        {
            if (readfile(pBankCtrl->wFile, dwSectSize*wSect, g_bDBBuf, -1))
            {
                if (!CheckFile(pBankCtrl->wFile, g_bDBBuf, 0))
                    break;
            }
            else
                break;
        }
        
        SignalSemaphore(g_semDbBuf);

        if (wSect < wSectNum)	//�ļ���������д��Ĭ��ֵ
        {
            if (DbLoadBankDefaultFLASH(pBankCtrl, 0))
            {
                DbSetFileValid(pBankCtrl);
                goto DbLoadBankDefault_ok;
            }
            else
            {
                DbClrFileValid(pBankCtrl);
                goto DbLoadBankDefault_err;
            }
        }
        else	//�ļ�����
        {
            DbSetFileValid(pBankCtrl);
            goto DbLoadBankDefault_ok;
        }
    }
    else if (pBankCtrl->bStorage == DB_STORAGE_RAM)
    {
        DbLoadBankDefaultRAM(pBankCtrl, 0);
    }
    else if (pBankCtrl->bStorage == DB_STORAGE_FRAM)	//FRAM,FLASH��RAMͬʱ���棬Ŀǰ�������ڵ��������
    {
        //��Ϊ��Щ���ݶ����ϵ��ʱ���ʼ���ģ�û��װ�سɹ��ͳ�ʼ��Ϊ0�����������ﲻ�����κ�����
    }

DbLoadBankDefault_ok:
//#ifdef EN_DB_SECTHOLDER //֧����������
	//if (g_SectHolder.fEnable == false)	//�ڽ�ֹ�������ֵ�����£������Լ�����/�ͷ��ź���
//#endif //EN_DB_SECTHOLDER
		
	return true;

DbLoadBankDefault_err:
//#ifdef EN_DB_SECTHOLDER //֧����������
	//if (g_SectHolder.fEnable == false) //�ڽ�ֹ�������ֵ�����£������Լ�����/�ͷ��ź���
//#endif //EN_DB_SECTHOLDER
		
	return false;
}


//����:��ʼ��һ��Bank���ݿ�
bool DbInitBank(const TBankCtrl* pBankCtrl)
{
#ifdef AUTO_COMPUTE
	pBankCtrl->dwMemUsage = 0; 	//�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
	//pBankCtrl->dwSaveClick = 0; //��BANK���ݱ����ʱ��

	if (pBankCtrl->pItemDesc == NULL)  //��Ч�����ݿ��������
		return true;	

	if (!InitItemDesc(pBankCtrl)) //��ʼ��������������
		return false;

	if (pBankCtrl->pbBankData)
		delete []  pBankCtrl->pbBankData;

	pBankCtrl->pbBankData = NULL;
	if (pBankCtrl->wPnNum==0)
	{	//��BANK��ֻ��Ϊ������������,���������ݷ���Ҫ����Ӧ�Ķ�д����
		DTRACE(DB_DB, ("DbInitBank: <%s> ---just for item desc--- init ok, dwItemNum=%ld, dwBankSize=%ld, wPnNum=%d\n", 
			pBankCtrl->pszBankName,
			pBankCtrl->dwItemNum,
			pBankCtrl->dwBankSize,
			pBankCtrl->wPnNum));

		return true;
	}	

	//�����BANKʹ�ò����㶯̬ӳ��,��pBankCtrl->wPnNum����Ϊӳ�䷽�������õ�ʵ��֧�ֵĲ�������
	if (pBankCtrl->bPnMapSch>0 && pBankCtrl->bPnMapSch<=m_wPnMapNum) //������Ӧ����1~m_wPnMapNum��
	{	
		pBankCtrl->wPnNum = m_pPnMapCtrl[pBankCtrl->bPnMapSch-1].wRealNum;
	}

	pBankCtrl->dwTotalSize = pBankCtrl->dwBankSize; //* pBankCtrl->wPnNum;
	//pBankCtrl->dwMemUsage += pBankCtrl->dwTotalSize; //�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�

	return true;
#else	
	bool fRet = DbLoadBankDefault(pBankCtrl);
/*	DTRACE(DB_DB, ("DbInitBank: <%s> init ok, dwItemNum=%ld, dwIndexNum=%ld, dwDefaultSize=%ld, dwBankSize=%ld, wPnNum=%d, dwTotalSize=%ld, wFileNum=%d, dwFileSize=%ld, dwMemUsage=%ld\n", 
				   pBankCtrl->pszBankName,
				   pBankCtrl->dwItemNum,
				   pBankCtrl->dwIndexNum,
				   pBankCtrl->dwDefaultSize, pBankCtrl->dwBankSize,
				   pBankCtrl->wPnNum,
				   pBankCtrl->dwTotalSize, pBankCtrl->wFileNum, pBankCtrl->dwFileSize,
				   pBankCtrl->dwMemUsage));*/

	return fRet;
#endif
}

//����:���ָ��BANK/SECT,��������������ΪwPnNum��,ָ�������������
//��ע:���������ݵĳ��Ȳ��ܳ���256���ֽ�
bool DbClrPnData(WORD wBank, WORD wSect, WORD wPnNum, WORD wPn)
{
	DWORD i;
	const TBankCtrl* pBankCtrl;
	BYTE bBuf[256];
	if (wBank>=m_wBankNum || wSect>=m_wSectNum)
		return false;

	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	if (pBankCtrl->pItemDesc == NULL) //�յ�BANK
		return true;

	if (pBankCtrl->wPnNum > 1)	//��������֧�ְ�����BANK���ò�����������BANK
		return false;
	
	memset(bBuf, 0, sizeof(bBuf));	//Ŀǰ�ٶ����������ݶ�����̫��

	for (i=0; i<pBankCtrl->dwItemNum; i++)
	{
		if (pBankCtrl->pItemDesc[i].wPnNum == wPnNum)	//��������������ΪwPnNum
		{
			WriteItemTm(wBank, wPn, pBankCtrl->pItemDesc[i].wID, bBuf, (DWORD )0);	//��������ʱ��
		}
	}

	return true;
}


//����:���ݿ�ĳ�ʼ��
//����:@pDbCtrl �������ݿ���в������õ����ݿ���ƽṹ
//����:����ɹ��򷵻�true,���򷵻�false
bool DbInit(TDbCtrl* pDbCtrl)
{
	DWORD dwTime;
	WORD i;
	m_pDbCtrl = pDbCtrl; //�������ݿ���в������õ����ݿ���ƽṹ
	memset(&g_DbData, 0, sizeof(g_DbData));

	//Ϊ�˷��ʷ���,����m_pDbCtrl�еĲ��ֱ�����������ֱ��ʹ��
	m_wSectNum = m_pDbCtrl->wSectNum;		//BANK0�е�SECT��Ŀ
	m_pBank0Ctrl = m_pDbCtrl->pBank0Ctrl;
	m_wBankNum = m_pDbCtrl->wBankNum;		//֧�ֵ�BANK��Ŀ
	m_pBankCtrl = m_pDbCtrl->pBankCtrl;
	//m_iSectImg = m_pDbCtrl->iSectImg;		//485�������ݾ����,���û�������-1
	//m_wImgNum = m_pDbCtrl->wImgNum;			//485�������ݾ������
	m_wPnMapNum = m_pDbCtrl->wPnMapNum;  	//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
	m_pPnMapCtrl = m_pDbCtrl->pPnMapCtrl; 	//�������ݿⲻ֧�ֲ����㶯̬ӳ������ΪNULL
	//m_pDbUpgCtrl = m_pDbCtrl->pDbUpgCtrl;

	if (m_wSectNum>SECT_MAX || m_wBankNum>BANK_MAX || m_wPnMapNum>PNMAP_MAX)
	{
		DTRACE(DB_DB, ("DbInit: the following var over max, wSectNum=%d(%d), wBankNum=%d(%d), m_wPnMapNum=%d(%d)\r\n",
					   m_wSectNum, SECT_MAX, 
					   m_wBankNum, BANK_MAX,
					   m_wPnMapNum, PNMAP_MAX));
		return false;
	}
	
	if (m_pDbCtrl->wSaveInterv == 0) //������,��λ����
		m_pDbCtrl->wSaveInterv = 15;	

	m_semPnMap = NewSemaphore(1, 1);
	
	m_fTrigerSaveBank = false;
	//memset(m_bSectSaveFlg, 0, sizeof(m_bSectSaveFlg));
	//memset(m_bBankSaveFlg, 0, sizeof(m_bBankSaveFlg));
	m_dwPnMapFileFlg = 0;

	//m_fDbUpg = InitUpgrade(m_pDbUpgCtrl);

	//m_dwMemUsage = 0;	  //�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
	
	for (i=0; i<m_wSectNum; i++) 
	{
		if (DbInitBank(&m_pBank0Ctrl[i]) == false)
			return false;
		
		//m_dwMemUsage += m_pBank0Ctrl[i].dwMemUsage;
	}

	for (i=0; i<m_wBankNum; i++)
	{
		if (DbInitBank(&m_pBankCtrl[i]) == false)
			return false;
	
		//m_dwMemUsage += m_pBankCtrl[i].dwMemUsage;
	}

#ifdef EN_DB_PNMAP
	if (!InitPnMap(m_pPnMapCtrl, m_wPnMapNum))
		return false;
#endif //EN_DB_PNMAP

	dwTime = GetCurSec();
	DbTimeAdjBackward(dwTime); //����Ҫ����ʱ����������ʱ��ȫ��У��һ��

	m_dwSaveClick = 0;
	m_fTrigerSaveAll = false;
	m_fTrigerSavePara = false;

	//SetEmptyTime(&g_tmAccessDenied);
	DTRACE(DB_CRITICAL, ("DbInit: init ok\r\n"));

#ifdef AUTO_COMPUTE
	OutputCfg();
#endif
	return true;
}


//����:��ʱ����ǰ������dwTime(��),���ݿ���Ӧ�����ĵ���
void DbTimeAdjBackward(DWORD dwTime)
{
}

//����:ȡ������ĵ�ַ,��Ҫע�ⲻҪֱ���øõ�ַ��������,����Ҫʹ�����ݿ��ṩ�ĺ���ReadItem()��WriteItem()
TDataItem DbGetItem(WORD wPoint, WORD wID)
{
	TDataItem di;
	memset(&di, 0, sizeof(di));
	
	/*const TItemDesc* pItemDesc = BinarySearchItem(g_TermnParaDesc, sizeof(g_TermnParaDesc)/sizeof(const TItemDesc), wID);
	if (pItemDesc != NULL)
	{
		di.pbAddr = m_pbTermnPara + pItemDesc->wOffset;
		di.wLen = pItemDesc->wLen;
		di.pfModified = &m_fParaModified;
		return di;
	}*/

	return di;
}

//��ע:$����ʲôʱ����Ҫ����
//		1.��Ҫ����Գ���Ĳ�����,���ͬʱ���ڲ�ͬʱ�����ĳ�������,����15����
//		  ��60���ӵ�,�����ʹ�þ���,����60����������,��15���ӹ��е�������
//		  ���ϵر����ȼ��ߵ�15��������ˢ��,����60�������ݵ�һ���Բ�̫��;
//		  ���ʹ���˾���,������������ͬʱҪ��901f,�����60���ӵ�������˵,
//		  ���ɼ�901f�����ȼ�����,��֤��901f�Ĳɼ�����ÿ��Сʱ��ǰ15�����ڲɼ�
//		2.���ڽ���/���������,��ͨ����Ӧ�ð��ղ�����ֿ�,�������ɺ����������
//		  �Ĳɼ��������յ�����������Ӱ��,�������ú�׼ʱ,���Ծ�û�б�Ҫʹ�þ���
//		3.����ͬʱ���ڳ��������ͽ���/���������ļ�������,�����ܼ���ļ���
//		  Ŀǰ�Ĳ���Ҫ�����������������ͬһʱ�̲ɼ�����,����Ҫ�������
//		  ������һ������ڱ��ֲ���,����/����ʹ�����µ������ϵ�ˢ���ܼӹ���,
//		  ���Գ��������ʹ�þ���,������/��������㲻ʹ�þ���,����Ŀǰ��ʹ��
//		  �����˵���ʺϵ�
int DbReadItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwStartTime, DWORD dwEndTime)
{
	int iRet;
	WORD wSect;

	if (wBank == BANK0)
	{
		for (wSect=0; wSect<m_wSectNum; wSect++) //�ն˲�����
		{
			iRet = ReadItem(IMG0, wPn, wID, pItemAcess, dwStartTime, dwEndTime, &m_pBank0Ctrl[wSect]);
			if (iRet != -ERR_ITEM)
			{
				if (pItemAcess->bType == DI_ACESS_INFO)	//ȡ��������Ϣ(���ȺͶ�)
					pItemAcess->pItemInfo->wSect = wSect;

				return iRet;
			}
		}
	}
	else if (wBank<m_wBankNum && m_pBankCtrl[wBank].pItemDesc!=NULL)
	{
		return ReadItem(IMG0, wPn, wID, pItemAcess, dwStartTime, dwEndTime, &m_pBankCtrl[wBank]);
	}
	
	return -ERR_ITEM;
}


int DbWriteItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwTime)
{
	int iRet;
	WORD wSect;

	if (IsDbLocked())
		return -ERR_ITEM;
		
	if (wBank == BANK0)
	{
		/*if (wID==0x8902)
		{
			int kk;
			kk  = 0;

		}*/
		for (wSect=0; wSect<m_wSectNum; wSect++)
		{
			iRet = WriteItem(IMG0, wPn, wID, pItemAcess, dwTime, &m_pBank0Ctrl[wSect]);
			if (iRet != -ERR_ITEM)
				return iRet;
		}
	}
	else if (wBank<m_wBankNum && m_pBankCtrl[wBank].pItemDesc!=NULL)
	{
		return WriteItem(IMG0, wPn, wID, pItemAcess, dwTime, &m_pBankCtrl[wBank]);
	}

	return -ERR_ITEM;
}


//����:ȡ������ĵ�ַ,��Ҫע�ⲻҪֱ���øõ�ַ��������,
//     ����Ҫʹ�����ݿ��ṩ�ĺ���ReadItem()��WriteItem()
TDataItem DbGetItemEx(WORD wBank, WORD wPn, WORD wID)
{
	int iRet;
	WORD wSect;
	TDataItem di;
	TItemAcess ItemAcess;

	memset(&di, 0, sizeof(di));
	memset(&ItemAcess, 0, sizeof(ItemAcess));
	ItemAcess.bType = DI_ACESS_GI;
	ItemAcess.pbBuf = (BYTE* )&di;
	
	if (wBank == BANK0)
	{
		//�ٵ���ǰ������
		for (wSect=0; wSect<m_wSectNum; wSect++) //�ն˲�����
		{
			iRet = ReadItem(IMG0, wPn, wID, &ItemAcess, 
							INVALID_TIME, INVALID_TIME, 
							&m_pBank0Ctrl[wSect]);
								  
			if (iRet != -ERR_ITEM)
				break;
		}
	}
	else if (wBank < m_wBankNum) // && wBank!=BANK0
	{
		iRet = ReadItem(IMG0, wPn, wID, &ItemAcess, 
						INVALID_TIME, INVALID_TIME, 
						&m_pBankCtrl[wBank]);
	} 

	return di;
}

#ifdef EN_DB_QRYTM
//����:����ϵͳ��ĵ�ǰ���ʱ��
//����:@bSect BANK0�Ķκ�
//	   @dwCurIntervTime ��ǰ���ʱ��
void DbSetCurInterv(BYTE bSect, BYTE bDynBn, DWORD dwCurIntervTime)
{
	DWORD dwIndexSize;
	if (!m_pBank0Ctrl[bSect].fUpdTime)
		return;
	if (m_pBank0Ctrl[bSect].bStorage==DB_STORAGE_DYN && bDynBn>=DYN_PN_NUM)
		return;

	LockBank(m_pBank0Ctrl[bSect].bStorage);

	dwIndexSize = (m_pBank0Ctrl[bSect].dwIndexNum + 7) >> 3;
	
	if (m_pBank0Ctrl[bSect].bStorage == DB_STORAGE_DYN)
	{
		memcpy(m_pBank0Ctrl[bSect].pbTimeFlg+dwIndexSize*((bDynBn<<1) + 1), 
			   m_pBank0Ctrl[bSect].pbTimeFlg+dwIndexSize*(bDynBn<<1), 
			   dwIndexSize);	//��BANK���ݵĸ���ʱ�䣬ֻ����fUpdTime==trueʱʹ��
		memset(m_pBank0Ctrl[bSect].pbTimeFlg+dwIndexSize*(bDynBn<<1), 0, dwIndexSize);
		m_pBank0Ctrl[bSect].pdwIntervTime[(bDynBn<<1) + 1] = m_pBank0Ctrl[bSect].pdwIntervTime[bDynBn<<1]; //��BANK���ݵĵ�ǰ�����ֻ����fUpdTime==trueʱʹ��
		m_pBank0Ctrl[bSect].pdwIntervTime[bDynBn<<1] = dwCurIntervTime;
	}
	else
	{
		memcpy(m_pBank0Ctrl[bSect].pbTimeFlg+dwIndexSize, 
			   m_pBank0Ctrl[bSect].pbTimeFlg, 
			   dwIndexSize);	//��BANK���ݵĸ���ʱ�䣬ֻ����fUpdTime==trueʱʹ��
		memset(m_pBank0Ctrl[bSect].pbTimeFlg, 0, dwIndexSize);
		m_pBank0Ctrl[bSect].pdwIntervTime[1] = m_pBank0Ctrl[bSect].pdwIntervTime[0]; //��BANK���ݵĵ�ǰ�����ֻ����fUpdTime==trueʱʹ��
		m_pBank0Ctrl[bSect].pdwIntervTime[0] = dwCurIntervTime;
	}

	UnLockBank(m_pBank0Ctrl[bSect].bStorage);
}


//����:ȡϵͳ��ĵ�ǰ���ʱ��
//����:@bSect BANK0�Ķκ�
DWORD DbGetCurInterv(BYTE bSect, BYTE bDynBn)
{
	if (!m_pBank0Ctrl[bSect].fUpdTime)
		return 0;
	if (m_pBank0Ctrl[bSect].bStorage==DB_STORAGE_DYN && bDynBn>=DYN_PN_NUM)
		return 0;

	if (m_pBank0Ctrl[bSect].bStorage == DB_STORAGE_DYN)
		return m_pBank0Ctrl[bSect].pdwIntervTime[bDynBn<<1];
	else
		return m_pBank0Ctrl[bSect].pdwIntervTime[0];
}

void DbInitTimeData(BYTE bSect, BYTE bDynBn, DWORD dwCurIntervTime)
{
	DWORD dwIndexSize;
	if (!m_pBank0Ctrl[bSect].fUpdTime)
		return;
	if (m_pBank0Ctrl[bSect].bStorage==DB_STORAGE_DYN && bDynBn>=DYN_PN_NUM)
		return;

	LockBank(m_pBank0Ctrl[bSect].bStorage);

	dwIndexSize = (m_pBank0Ctrl[bSect].dwIndexNum + 7) >> 3;

	if (m_pBank0Ctrl[bSect].bStorage == DB_STORAGE_DYN)
	{
		memset(m_pBank0Ctrl[bSect].pbBankData+bDynBn*m_pBank0Ctrl[bSect].dwTotalSize, 0, m_pBank0Ctrl[bSect].dwTotalSize);
		memset(m_pBank0Ctrl[bSect].pbTimeFlg+dwIndexSize*(bDynBn<<1), 0, dwIndexSize<<1);
		m_pBank0Ctrl[bSect].pdwIntervTime[bDynBn<<1] = dwCurIntervTime;
		m_pBank0Ctrl[bSect].pdwIntervTime[(bDynBn<<1) + 1] = 0; //��BANK���ݵĵ�ǰ�����ֻ����fUpdTime==trueʱʹ��
	}
	else
	{
		memset(m_pBank0Ctrl[bSect].pbBankData, 0, m_pBank0Ctrl[bSect].dwTotalSize);
		memset(m_pBank0Ctrl[bSect].pbTimeFlg, 0, dwIndexSize<<1);
		m_pBank0Ctrl[bSect].pdwIntervTime[0] = dwCurIntervTime;
		m_pBank0Ctrl[bSect].pdwIntervTime[1] = 0; //��BANK���ݵĵ�ǰ�����ֻ����fUpdTime==trueʱʹ��
	}

	UnLockBank(m_pBank0Ctrl[bSect].bStorage);
}

#endif

#ifdef EN_DB_PNMAP
//����:��ʼ��ϵͳ��TPnMapCtrl�ṹ,�����ļ�ϵͳ�Ѳ�����ŵ��洢�ŵ�ӳ���ָ���pwPnToMemMap
bool InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum)
{
	WORD i, j;
	WORD wMapNum;	//�Ѿ�ӳ��ĸ���
	WORD wMN;		//�洢��
	bool fFileValid;
	//char szPathName[128];
	//char szHeader[160];
	for (i=0; i<wNum; i++)
	{
		//��ʼ��TPnMapCtrl�ṹ
		pPnMapCtrl[i].dwFileSize = ((pPnMapCtrl[i].wRealNum<<1) + 2)*sizeof(WORD);	//ӳ�䱣����ļ���С,
		//ǰ������WORD�������������Ϣ,���е�һ��WORD���Ѿ�ӳ��ĸ���,�ڶ�������
		pPnMapCtrl[i].wAllocSize = (pPnMapCtrl[i].wRealNum+7)>>3;	//�洢�ռ�����Ĵ�С

		//pPnMapCtrl[i].pwPnToMemMap = new WORD[pPnMapCtrl[i].dwFileSize]; //������ŵ��洢�ŵ�ӳ���(��Ҫ���浽�ļ�ϵͳ)
		//pPnMapCtrl[i].pbAllocTab = new BYTE[pPnMapCtrl[i].wAllocSize];	 //�洢�ռ�����(�����浽�ļ�ϵͳ,��̬����)
		if (pPnMapCtrl[i].pwPnToMemMap==NULL || pPnMapCtrl[i].pbAllocTab==NULL)
		{
			DTRACE(DB_DB, ("InitPnMap: critical error : sys out of memory.\r\n"));
			return false;
		}

		memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
		memset(pPnMapCtrl[i].pbAllocTab, 0, pPnMapCtrl[i].wAllocSize);

		//���ļ�ϵͳ�Ѳ�����ŵ��洢�ŵ�ӳ���ָ���pwPnToMemMap
		//sprintf(szPathName, "%sPNMAP%d.cfg", m_pDbCtrl->pszDbPath, i);
		fFileValid = false;
        WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
		if (readfile(pPnMapCtrl[i].wFile, 0, g_bDBBuf, -1))
		{
			if (CheckFile(pPnMapCtrl[i].wFile, g_bDBBuf, 0))
			{
				memcpy((BYTE* )pPnMapCtrl[i].pwPnToMemMap, g_bDBBuf, pPnMapCtrl[i].dwFileSize);
				fFileValid = true;
			}
		}
        SignalSemaphore(g_semDbBuf);
		if (fFileValid == false)
		{
			DTRACE(DB_DB, ("InitPnMap: fail to read PNMAP%d\r\n", i));
			memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);

			if (pPnMapCtrl[i].fGenerateMapWhenNoneExist)	//��û��ӳ����ʱ���Զ�����һһ��Ӧ��ӳ�����Ҫ��Ӧ�԰汾����
			{
				wMapNum = pPnMapCtrl[i].pwPnToMemMap[0] = pPnMapCtrl[i].wRealNum; //�Ѿ�ӳ��ĸ���
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)+1] = j; //�洢�� wMN<pPnMapCtrl[i].wRealNum
					pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)] = j;		//�������<pPnMapCtrl[i].wMaxPn
					pPnMapCtrl[i].pbAllocTab[wMN>>3] |= 1<<(wMN&7);	//��ʾ�ô洢�ռ��Ѿ�����
				}
			}
		}
		else
		{
			//sprintf(szHeader, "PNMAP%d :", i);
			//TraceBuf(DB_DB, szHeader, (BYTE* )pPnMapCtrl[i].pwPnToMemMap, pPnMapCtrl[i].dwFileSize);

			//���ݲ�����ŵ��洢�ŵ�ӳ���,��ʼ���洢�ռ�����
			wMapNum = pPnMapCtrl[i].pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
			if (wMapNum > pPnMapCtrl[i].wRealNum)	//ӳ���ļ���Ϣ����
			{
				DTRACE(DB_DB, ("InitPnMap: err! wMapNum(%d)>wRealNum(%d) in \r\n", 
								wMapNum, pPnMapCtrl[i].wRealNum));
				memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
			}
			else
			{
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)+1]; //�洢��
					if (pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)]<pPnMapCtrl[i].wMaxPn 	//�������
						&& wMN<pPnMapCtrl[i].wRealNum) 	//�洢��
					{
						pPnMapCtrl[i].pbAllocTab[wMN>>3] |= 1<<(wMN&7);	//��ʾ�ô洢�ռ��Ѿ�����
					}
					else	//ӳ���ļ���Ϣ����
					{
						DTRACE(DB_DB, ("InitPnMap: err! wMN=%d, Pn=%d, wMaxPn=%d, wRealNum=%d \r\n", 
										wMN, pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)], pPnMapCtrl[i].wMaxPn,
										pPnMapCtrl[i].wRealNum));

						memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
						memset(pPnMapCtrl[i].pbAllocTab, 0, pPnMapCtrl[i].wAllocSize);
						break;
					}
				}
			}
		}
	}

	return true;
}

//����:���Ҳ������Ӧ��ӳ���
//����:����ҵ��򷵻ض�Ӧ��ӳ���,���򷵻�-1
int SearchPnMap(BYTE bSch, WORD wPn)
{
	int little, big, mid;
	WORD wMapNum;
	TPnMapCtrl* pPnMapCtrl;

	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return -1;

	pPnMapCtrl = &m_pPnMapCtrl[bSch-1];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return -1;

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);	
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}
	
	little = 0;
	big = (int )wMapNum-1;
	while (little <= big)
	{                               
		mid = (little + big) >> 1;  //����

		if (wPn == pPnMapCtrl->pwPnToMemMap[2+(mid<<1)])
		{
			SignalSemaphore(m_semPnMap);
			return pPnMapCtrl->pwPnToMemMap[2+(mid<<1)+1];
		}
		else if (wPn > pPnMapCtrl->pwPnToMemMap[2+(mid<<1)])
		{
			little = mid + 1;
		} 
		else  //if (wPn < pPnMapCtrl.pwPnToMemMap[2+mid*2])
		{
			big = mid - 1;
		}

		mid = (little + big) >> 1;
	}

	SignalSemaphore(m_semPnMap);
	return -1;
}

//����:����ӳ��Ŷ�Ӧ�Ĳ�����
//����:����ҵ��򷵻ض�Ӧ�Ĳ�����,���򷵻�-1
int MapToPn(BYTE bSch, WORD wMn)
{
	int iPn = -1;
	WORD i, wMapNum;
	TPnMapCtrl* pPnMapCtrl;

	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return -1;

	pPnMapCtrl = &m_pPnMapCtrl[bSch-1];

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);	
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	iPn = -1;
	for (i=0; i<wMapNum; i++)
	{                               
		if (wMn == pPnMapCtrl->pwPnToMemMap[2+(i<<1)+1])
		{
			iPn = pPnMapCtrl->pwPnToMemMap[2+(i<<1)];
			break;
		}
	}

	SignalSemaphore(m_semPnMap);
	return iPn;
}

//����:���붯̬ӳ�������
//����:@bSch ӳ�䷽��
// 	   @wPn ��Ҫӳ��Ĳ������
//����:�����ȷ����ӳ���,���򷵻�-1
int NewPnMap(BYTE bSch, WORD wPn)
{
	WORD i, j;
	WORD wMapNum;
	WORD wMN;
	TPnMapCtrl* pPnMapCtrl;
	int iMN = SearchPnMap(bSch, wPn);
	if (iMN >= 0)	//�Ѿ����ڸò������ӳ����
		return iMN;

	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return -1;

	bSch--;	//ת����g_PnMapCtrl�����ж�Ӧ������
	pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
	{
		DTRACE(DB_DB, ("DbNewPnMap: err, wPn(%d)>=wMaxPn(%d)\n", 
					   wPn, pPnMapCtrl->wMaxPn));
		return -1;
	}

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);	//SearchPnMap()Ҳ�������ͷ�m_semPnMap,��������ǰ����ź���
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum >= pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		DTRACE(DB_DB, ("DbNewPnMap: there is no room for pn=%d\n", wPn));
		return -1;
	}

	//����洢��(���߽�ӳ���)
	for (i=0; i<pPnMapCtrl->wAllocSize; i++)
	{
		if (pPnMapCtrl->pbAllocTab[i] != 0xff)	//��ûռ�õĿռ�
			break;
	}

	if (i >= pPnMapCtrl->wAllocSize)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	for (j=0; j<8; j++)
	{
		if ((pPnMapCtrl->pbAllocTab[i]&(1<<j)) == 0)
			break;
	}

	if (j >= 8)	//Ӧ�ò�����������Ĵ���,�Է���һ
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	wMN = (i<<3) + j;
	pPnMapCtrl->pbAllocTab[i] |= 1<<j;	//��־�ô洢�ռ��Ѿ�������

	//��������Ŵ�С����,ȷ���²�������ӳ����е�λ��
	for (i=0; i<wMapNum; i++)
	{
		if (wPn < pPnMapCtrl->pwPnToMemMap[2+(i<<1)])
			break;
	}

	//Ų��һ������ӳ�������Ŀռ�
	for (j=wMapNum; j>i; j--)
	{
		pPnMapCtrl->pwPnToMemMap[2+(j<<1)] = pPnMapCtrl->pwPnToMemMap[2+((j-1)<<1)];
		pPnMapCtrl->pwPnToMemMap[2+(j<<1)+1] = pPnMapCtrl->pwPnToMemMap[2+((j-1)<<1)+1];
	}

	pPnMapCtrl->pwPnToMemMap[2+(i<<1)] = wPn;
	pPnMapCtrl->pwPnToMemMap[2+(i<<1)+1] = wMN;
	pPnMapCtrl->pwPnToMemMap[0] ++;

	m_dwPnMapFileFlg |= (DWORD )1<<bSch;
	SignalSemaphore(m_semPnMap);

	DTRACE(DB_DB, ("DbNewPnMap: new map=%d for pn=%d\n", wMN, wPn));
	return wMN;
}

//����:ɾ��ӳ�������
//����:@bSch ӳ�䷽��
// 	   @wPn �Ѿ�ӳ��Ĳ������
//����:�����ȷ����true,���򷵻�false
bool DeletePnMap(BYTE bSch, WORD wPn)
{
	WORD i;
	WORD wMapNum, wMN;
	TPnMapCtrl* pPnMapCtrl;
	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return false;

	bSch--;	//ת����g_PnMapCtrl�����ж�Ӧ������
	pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return false;

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{	
		SignalSemaphore(m_semPnMap);
		return false;
	}
	
	//��������Ŵ�С����,ȷ���²�������ӳ����е�λ��
	for (i=0; i<wMapNum; i++)
	{
		if (wPn == pPnMapCtrl->pwPnToMemMap[2+(i<<1)])
			break;
	}

	if (i >= wMapNum)
	{
		SignalSemaphore(m_semPnMap);
		return false;
	}

	//ɾ����ӳ����Դ��ռ��
	wMN = pPnMapCtrl->pwPnToMemMap[2+(i<<1)+1];
	pPnMapCtrl->pbAllocTab[wMN>>3] &= ~(1<<(wMN&7));	//��־�ô洢�ռ��Ѿ�������

	//�����ӳ��������ǰ�ƶ�,ռ�ñ�ɾ��������Ŀռ�
	for (; i<wMapNum-1; i++)
	{
		pPnMapCtrl->pwPnToMemMap[2+(i<<1)] = pPnMapCtrl->pwPnToMemMap[2+((i+1)<<1)];
		pPnMapCtrl->pwPnToMemMap[2+(i<<1)+1] = pPnMapCtrl->pwPnToMemMap[2+((i+1)<<1)+1];
	}

	pPnMapCtrl->pwPnToMemMap[0]--;
	m_dwPnMapFileFlg |= (DWORD )1<<bSch;
	SignalSemaphore(m_semPnMap);
	return true;
}


//�����BANKʹ�ò����㶯̬ӳ��,��pBankCtrl->wPnNum����Ϊӳ�䷽�������õ�ʵ��֧�ֵĲ�������
int GetPnMapRealNum(BYTE bSch)
{
	if (bSch>0 && bSch<=m_wPnMapNum) //������Ӧ����1~m_wPnMapNum��
		return m_pPnMapCtrl[bSch-1].wRealNum;
	else
		return -1;
}

//����:��������㶯̬ӳ���
int SavePnMap()
{
	int iRet = 0;
	int iFileOff;
	DWORD dwFlg;
	WORD i;

	WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);

	if (m_dwPnMapFileFlg != 0) //�в�����ӳ���ļ��������޸�
	{
		dwFlg = 1;
		for (i=0; i<m_wPnMapNum; i++, dwFlg<<1)
		{
			if (m_dwPnMapFileFlg & dwFlg)
			{
				m_dwPnMapFileFlg &= ~dwFlg; //�ڱ���ǰ�����־,�Ա��ڱ���Ĺ����п��Է�Ӧ�µı仯
				//���ļ�ϵͳ�Ѳ�����ŵ��洢�ŵ�ӳ���ָ���pwPnToMemMap
				memset(g_bDBBuf, 0, DBBUF_SIZE);
				iRet = ReadFileSect(m_pPnMapCtrl[i].wFile, 0, g_bDBBuf, &iFileOff);	//��ȡ�ļ����ڵĵ�dwSect������
				if (iRet < 0)
				{
					SignalSemaphore(g_semDbBuf);
					return -ERR_DEV;	//�豸������
				}

				memcpy(g_bDBBuf+iFileOff, (BYTE* )m_pPnMapCtrl[i].pwPnToMemMap, m_pPnMapCtrl[i].dwFileSize);
				MakeFile(m_pPnMapCtrl[i].wFile, g_bDBBuf);	//���ļ����������ļ���־��У��
				if (!writefile(m_pPnMapCtrl[i].wFile, 0, g_bDBBuf))
				{
					if (!writefile(m_pPnMapCtrl[i].wFile, 0, g_bDBBuf))
					{
                        SignalSemaphore(g_semDbBuf);
						DTRACE(DB_DB, ("SavePnMap: fail to save pnmap file\n"));						
						return -ERR_DEV;	//�豸������
					}
				}
			}
		}
	}

	SignalSemaphore(g_semDbBuf);
	return 0;
}
#endif //EN_DB_PNMAP
