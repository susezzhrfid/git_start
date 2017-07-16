/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DataManager.c
 * 摘    要：本文件主要实现系统数据库的数据项读写存储管理
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
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
//系统库私有成员定义
static TDbCtrl* 	m_pDbCtrl;		//外界对数据库进行参数配置的数据库控制结构
static WORD			m_wSectNum;		//BANK0中的SECT数目
static const TBankCtrl* 	m_pBank0Ctrl;
static WORD 		m_wBankNum;		//支持的BANK数目
static const TBankCtrl* 	m_pBankCtrl;
static WORD 		m_wPnMapNum;  	//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
static TPnMapCtrl*	m_pPnMapCtrl; 	//整个数据库不支持测量点动态映射则设为NULL
static TSem			m_semPnMap;
static DWORD		m_dwSaveClick;
static bool			m_fTrigerSaveAll;
static bool			m_fTrigerSavePara;
static bool			m_fTrigerSaveBank;
static DWORD		m_dwPnMapFileFlg;	//每1位置1表示一个文件发生了修改

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//部分函数定义
bool InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//系统库函数
//描述:将某个Bank下section的数据恢复成默认值
//参数:@wBank Bank号
//	   @wSect Section号
bool DbClearBankData(WORD wBank, WORD wSect)
{
	const TBankCtrl	*pBankCtrl;

	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	return DbLoadBankDefaultFLASH(pBankCtrl, 0);
}

//描述:装入1个文件的默认数据,
//		1.当描述表中存在多个测量点的描述,其实装入的是整个BANK的多个测量点的默认值,
//		  即装入的是该描述标所描述的多个测量点的默认数据;
//		2.当描述表中不存在多个测量点的描述时,则装入的是该描述标所描述的1个测量点的默认数据
//		  (可能是TBankCtrl中的多个测量点中的一个)
//参数:@pBankCtrl BANK控制结构
//	   @wFile 文件号,分别对应测量点号或者镜像号 
//	   @dwOffset 一个文件里的偏移,主要针对那些版本发生了改变的BANK,
//				后面又扩展了新的数据项,在装入文件内容的时候已经装载了
//				前面原有的部分,后面给新加的部分装入默认置
//				对于那些版本没有发生改变的BANK,把本参数置为0
bool DbLoadBankDefaultRAM(const TBankCtrl* pBankCtrl, DWORD dwOffset)
{
	const TItemDesc* pItemDesc;
	DWORD num;
	BYTE* pbDst;
	BYTE* pbDst0;
	const BYTE* pbSrc;
	DWORD i;
	WORD j;

	if (pBankCtrl->pbDefault == NULL)	//本BANK没有默认值,直接取0
	{
		memset(pBankCtrl->pbBankData+dwOffset, 0, pBankCtrl->dwFileSize-dwOffset);
		goto ret_ok;
	}

	if (!pBankCtrl->fMutiPnInDesc)  //描述表中不存在多个测量点的描述
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
		if ((pItemDesc[i].wProp&DI_SELF) && (pItemDesc[i].wProp&DI_NSP)==0)		 //本数据项能自成独立数据项
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



//描述:装入1个BANK的默认数据,
//参数:@pBankCtrl BANK控制结构
//	   @iFile 文件号,分别对应测量点号或者镜像号,小于0时表示装入整BANK默认数据 
//	   @dwOffset 一个文件里的偏移,主要针对那些版本发生了改变的BANK,
//				后面又扩展了新的数据项,在装入文件内容的时候已经装载了
//				前面原有的部分,后面给新加的部分装入默认置
//				对于那些版本没有发生改变的BANK,把本参数置为0
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
	DWORD dwOff = 0;	//拷贝过程中，相对整个文件开头的偏移，用来标记默认值是否该取

	WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);

	wSectNum = GetFileSectNum(pBankCtrl->wFile);	//取得文件保存的扇区数量
	dwUsedSize = GetSectUsedSize(pBankCtrl->wFile);//取得文件实际使用的扇区大小
										
	if (pBankCtrl->pbDefault == NULL)	//本BANK没有默认值,直接取0
	{
		for (wSect=0; wSect<wSectNum; wSect++)
		{
			memset(g_bDBBuf, 0, DBBUF_SIZE);
			iRet = ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff);	//读取文件所在的第dwSect个扇区
			if (iRet < 0)
				goto ret_err_rd;

			memset(g_bDBBuf+iFileOff, 0, dwUsedSize); //这里有可能会溢出
			MakeFile(pBankCtrl->wFile, g_bDBBuf);	//给文件扇区加上文件标志和校验
			if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
				goto ret_err_wr;
		}

        SignalSemaphore(g_semDbBuf);
		return true;
	}

	wSect = 0;	//文件的第几个扇区，对于按页分配的，应置为0
	memset(g_bDBBuf, 0, DBBUF_SIZE);
	iRet = ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff);	//读取文件所在的第dwSect个扇区
	if (iRet < 0)
		goto ret_err_rd;

	if (!CheckFile(pBankCtrl->wFile, g_bDBBuf, iFileOff))	//文件校验错误
	{
		memset(g_bDBBuf+iFileOff, 0, dwUsedSize);
		dwOffset = 0;	//如果文件校验错误，则默认值全部重新取
	}

	pbDst = g_bDBBuf + iFileOff;
	pbDst0 = pbDst;

	for (i=0; i<num; i++)	//每个数据项的拷贝
    {
		if ((pItemDesc[i].wProp&DI_SELF) && (pItemDesc[i].wProp&DI_NSP)==0)		 //本数据项能自成独立数据项
		{
			for (j=0; j<pItemDesc[i].wPnNum; j++)	//每个测量点的该数据项的拷贝
			{
				wCpyLen = pItemDesc[i].wLen;
				if (pbDst-pbDst0+wCpyLen > dwUsedSize)	//一个数据项剩余字节的拷贝
					wCpyLen = dwUsedSize - (pbDst-pbDst0);
				
				if (dwOff >= dwOffset)
					memcpy(pbDst, pbSrc, wCpyLen);
					
				pbDst += wCpyLen;
				dwOff += wCpyLen;	//拷贝过程中，相对整个文件开头的偏移，用来标记默认值是否该取

				if (pbDst-pbDst0 >= dwUsedSize)	//扇区文件要跨扇区了，或者页文件结束了
				{
					MakeFile(pBankCtrl->wFile, g_bDBBuf);	//给文件扇区加上文件标志和校验
					if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
					{
						if (!writefile(pBankCtrl->wFile, wSect, g_bDBBuf))
							goto ret_err_wr;
					}

					if (IsPageFile(pBankCtrl->wFile))	//按页分配的文件,应该在这里就完了
					{
						if (j+1!=pItemDesc[i].wPnNum || i+1!=num)	//数据项还没完
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
					iRet = ReadFileSect(pBankCtrl->wFile, wSect, g_bDBBuf, &iFileOff);	//读取文件所在的第dwSect个扇区
					if (iRet < 0)
						goto ret_err_rd;

					if (!CheckFile(pBankCtrl->wFile, g_bDBBuf, iFileOff))	//文件校验错误
					{
						memset(g_bDBBuf+iFileOff, 0, dwUsedSize);
						dwOffset = 0;
					}

					pbDst = g_bDBBuf + iFileOff;
					pbDst0 = pbDst;
				}

				if (wCpyLen < pItemDesc[i].wLen)	//一个数据项剩余字节的拷贝
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

	MakeFile(pBankCtrl->wFile, g_bDBBuf);	//给文件扇区加上文件标志和校验
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
	if (pBankCtrl->pItemDesc == NULL)  //无效的数据库的组描述
		return true;

#ifdef EN_DB_SECTHOLDER //支持扇区保持
	if (g_SectHolder.fEnable == false)	//在禁止扇区保持的情况下，必须自己申请/释放信号量
#endif //EN_DB_SECTHOLDER

    if (pBankCtrl->bStorage==DB_STORAGE_IN || pBankCtrl->bStorage==DB_STORAGE_EX)   //内部FLASH固定存储 || 外部FLASH固定存储
    {
        //检查文件是否完整        
        DWORD dwSectSize = GetSectSize(pBankCtrl->wFile);	//取得文件保存的扇区大小
        WORD wSectNum = GetFileSectNum(pBankCtrl->wFile);	//取得文件保存的扇区数量
        
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

        if (wSect < wSectNum)	//文件不完整，写入默认值
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
        else	//文件完整
        {
            DbSetFileValid(pBankCtrl);
            goto DbLoadBankDefault_ok;
        }
    }
    else if (pBankCtrl->bStorage == DB_STORAGE_RAM)
    {
        DbLoadBankDefaultRAM(pBankCtrl, 0);
    }
    else if (pBankCtrl->bStorage == DB_STORAGE_FRAM)	//FRAM,FLASH跟RAM同时保存，目前靠保存在掉电变量中
    {
        //因为这些数据都是上电的时候初始化的，没有装载成功就初始化为0，所以在这里不用做任何事情
    }

DbLoadBankDefault_ok:
//#ifdef EN_DB_SECTHOLDER //支持扇区保持
	//if (g_SectHolder.fEnable == false)	//在禁止扇区保持的情况下，必须自己申请/释放信号量
//#endif //EN_DB_SECTHOLDER
		
	return true;

DbLoadBankDefault_err:
//#ifdef EN_DB_SECTHOLDER //支持扇区保持
	//if (g_SectHolder.fEnable == false) //在禁止扇区保持的情况下，必须自己申请/释放信号量
//#endif //EN_DB_SECTHOLDER
		
	return false;
}


//描述:初始化一个Bank数据库
bool DbInitBank(const TBankCtrl* pBankCtrl)
{
#ifdef AUTO_COMPUTE
	pBankCtrl->dwMemUsage = 0; 	//内存使用量,单位字节,包括数据和时标存储空间
	//pBankCtrl->dwSaveClick = 0; //本BANK数据保存的时标

	if (pBankCtrl->pItemDesc == NULL)  //无效的数据库的组描述
		return true;	

	if (!InitItemDesc(pBankCtrl)) //初始化数据项描述表
		return false;

	if (pBankCtrl->pbBankData)
		delete []  pBankCtrl->pbBankData;

	pBankCtrl->pbBankData = NULL;
	if (pBankCtrl->wPnNum==0)
	{	//本BANK的只作为数据项描述用,真正的数据访问要靠相应的读写函数
		DTRACE(DB_DB, ("DbInitBank: <%s> ---just for item desc--- init ok, dwItemNum=%ld, dwBankSize=%ld, wPnNum=%d\n", 
			pBankCtrl->pszBankName,
			pBankCtrl->dwItemNum,
			pBankCtrl->dwBankSize,
			pBankCtrl->wPnNum));

		return true;
	}	

	//如果该BANK使用测量点动态映射,则pBankCtrl->wPnNum调整为映射方案中配置的实际支持的测量点数
	if (pBankCtrl->bPnMapSch>0 && pBankCtrl->bPnMapSch<=m_wPnMapNum) //方案号应该在1~m_wPnMapNum间
	{	
		pBankCtrl->wPnNum = m_pPnMapCtrl[pBankCtrl->bPnMapSch-1].wRealNum;
	}

	pBankCtrl->dwTotalSize = pBankCtrl->dwBankSize; //* pBankCtrl->wPnNum;
	//pBankCtrl->dwMemUsage += pBankCtrl->dwTotalSize; //内存使用量,单位字节,包括数据和时标存储空间

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

//描述:清除指定BANK/SECT,测量点数量配置为wPnNum的,指定测量点的数据
//备注:测量点数据的长度不能超过256个字节
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

	if (pBankCtrl->pItemDesc == NULL) //空的BANK
		return true;

	if (pBankCtrl->wPnNum > 1)	//本函数不支持按整个BANK配置测量点数量的BANK
		return false;
	
	memset(bBuf, 0, sizeof(bBuf));	//目前假定测量点数据都不会太长

	for (i=0; i<pBankCtrl->dwItemNum; i++)
	{
		if (pBankCtrl->pItemDesc[i].wPnNum == wPnNum)	//测量点数量配置为wPnNum
		{
			WriteItemTm(wBank, wPn, pBankCtrl->pItemDesc[i].wID, bBuf, (DWORD )0);	//清数据清时间
		}
	}

	return true;
}


//描述:数据库的初始化
//参数:@pDbCtrl 外界对数据库进行参数配置的数据库控制结构
//返回:如果成功则返回true,否则返回false
bool DbInit(TDbCtrl* pDbCtrl)
{
	DWORD dwTime;
	WORD i;
	m_pDbCtrl = pDbCtrl; //外界对数据库进行参数配置的数据库控制结构
	memset(&g_DbData, 0, sizeof(g_DbData));

	//为了访问方便,参数m_pDbCtrl中的部分变量拷贝出来直接使用
	m_wSectNum = m_pDbCtrl->wSectNum;		//BANK0中的SECT数目
	m_pBank0Ctrl = m_pDbCtrl->pBank0Ctrl;
	m_wBankNum = m_pDbCtrl->wBankNum;		//支持的BANK数目
	m_pBankCtrl = m_pDbCtrl->pBankCtrl;
	//m_iSectImg = m_pDbCtrl->iSectImg;		//485抄表数据镜像段,如果没有则配成-1
	//m_wImgNum = m_pDbCtrl->wImgNum;			//485抄表数据镜像个数
	m_wPnMapNum = m_pDbCtrl->wPnMapNum;  	//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
	m_pPnMapCtrl = m_pDbCtrl->pPnMapCtrl; 	//整个数据库不支持测量点动态映射则设为NULL
	//m_pDbUpgCtrl = m_pDbCtrl->pDbUpgCtrl;

	if (m_wSectNum>SECT_MAX || m_wBankNum>BANK_MAX || m_wPnMapNum>PNMAP_MAX)
	{
		DTRACE(DB_DB, ("DbInit: the following var over max, wSectNum=%d(%d), wBankNum=%d(%d), m_wPnMapNum=%d(%d)\r\n",
					   m_wSectNum, SECT_MAX, 
					   m_wBankNum, BANK_MAX,
					   m_wPnMapNum, PNMAP_MAX));
		return false;
	}
	
	if (m_pDbCtrl->wSaveInterv == 0) //保存间隔,单位分钟
		m_pDbCtrl->wSaveInterv = 15;	

	m_semPnMap = NewSemaphore(1, 1);
	
	m_fTrigerSaveBank = false;
	//memset(m_bSectSaveFlg, 0, sizeof(m_bSectSaveFlg));
	//memset(m_bBankSaveFlg, 0, sizeof(m_bBankSaveFlg));
	m_dwPnMapFileFlg = 0;

	//m_fDbUpg = InitUpgrade(m_pDbUpgCtrl);

	//m_dwMemUsage = 0;	  //内存使用量,单位字节,包括数据和时标存储空间
	
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
	DbTimeAdjBackward(dwTime); //把需要保存时间的数据项的时间全部校验一遍

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


//描述:当时间往前调整到dwTime(秒),数据库相应作出的调整
void DbTimeAdjBackward(DWORD dwTime)
{
}

//描述:取数据项的地址,但要注意不要直接用该地址访问数据,而是要使用数据库提供的函数ReadItem()和WriteItem()
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

//备注:$关于什么时候需要镜像
//		1.主要是针对抄表的测量点,如果同时存在不同时间间隔的抄表任务,比如15分钟
//		  和60分钟的,如果不使用镜像,则导致60分钟任务当中,和15分钟共有的数据项
//		  不断地被优先级高的15分钟任务刷新,导致60分钟数据的一致性不太好;
//		  如果使用了镜像,比如两个任务都同时要采901f,则对于60分钟的任务来说,
//		  它采集901f的优先级高了,保证了901f的采集都在每个小时的前15分钟内采集
//		2.对于交采/脉冲测量点,普通任务应该按照测量点分开,这样交采和脉冲测量点
//		  的采集不至于收到抄表测量点的影响,可以做得很准时,所以就没有必要使用镜像
//		3.对于同时存在抄表测量点和交采/脉冲测量点的计算任务,比如总加组的计算
//		  目前的并不要求参与计算的量都是在同一时刻采集到的,反而要求电表的量
//		  可以在一个间隔内保持不变,交采/脉冲使用最新的量不断地刷新总加功率,
//		  所以抄表测量点使用镜像,而交采/脉冲测量点不使用镜像,对于目前的使用
//		  情况来说是适合的
int DbReadItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess* pItemAcess, DWORD dwStartTime, DWORD dwEndTime)
{
	int iRet;
	WORD wSect;

	if (wBank == BANK0)
	{
		for (wSect=0; wSect<m_wSectNum; wSect++) //终端参数和
		{
			iRet = ReadItem(IMG0, wPn, wID, pItemAcess, dwStartTime, dwEndTime, &m_pBank0Ctrl[wSect]);
			if (iRet != -ERR_ITEM)
			{
				if (pItemAcess->bType == DI_ACESS_INFO)	//取数据项信息(长度和段)
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


//描述:取数据项的地址,但要注意不要直接用该地址访问数据,
//     而是要使用数据库提供的函数ReadItem()和WriteItem()
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
		//再到当前库中找
		for (wSect=0; wSect<m_wSectNum; wSect++) //终端参数和
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
//描述:设置系统库的当前间隔时间
//参数:@bSect BANK0的段号
//	   @dwCurIntervTime 当前间隔时间
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
			   dwIndexSize);	//本BANK数据的更新时间，只有在fUpdTime==true时使用
		memset(m_pBank0Ctrl[bSect].pbTimeFlg+dwIndexSize*(bDynBn<<1), 0, dwIndexSize);
		m_pBank0Ctrl[bSect].pdwIntervTime[(bDynBn<<1) + 1] = m_pBank0Ctrl[bSect].pdwIntervTime[bDynBn<<1]; //本BANK数据的当前间隔，只有在fUpdTime==true时使用
		m_pBank0Ctrl[bSect].pdwIntervTime[bDynBn<<1] = dwCurIntervTime;
	}
	else
	{
		memcpy(m_pBank0Ctrl[bSect].pbTimeFlg+dwIndexSize, 
			   m_pBank0Ctrl[bSect].pbTimeFlg, 
			   dwIndexSize);	//本BANK数据的更新时间，只有在fUpdTime==true时使用
		memset(m_pBank0Ctrl[bSect].pbTimeFlg, 0, dwIndexSize);
		m_pBank0Ctrl[bSect].pdwIntervTime[1] = m_pBank0Ctrl[bSect].pdwIntervTime[0]; //本BANK数据的当前间隔，只有在fUpdTime==true时使用
		m_pBank0Ctrl[bSect].pdwIntervTime[0] = dwCurIntervTime;
	}

	UnLockBank(m_pBank0Ctrl[bSect].bStorage);
}


//描述:取系统库的当前间隔时间
//参数:@bSect BANK0的段号
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
		m_pBank0Ctrl[bSect].pdwIntervTime[(bDynBn<<1) + 1] = 0; //本BANK数据的当前间隔，只有在fUpdTime==true时使用
	}
	else
	{
		memset(m_pBank0Ctrl[bSect].pbBankData, 0, m_pBank0Ctrl[bSect].dwTotalSize);
		memset(m_pBank0Ctrl[bSect].pbTimeFlg, 0, dwIndexSize<<1);
		m_pBank0Ctrl[bSect].pdwIntervTime[0] = dwCurIntervTime;
		m_pBank0Ctrl[bSect].pdwIntervTime[1] = 0; //本BANK数据的当前间隔，只有在fUpdTime==true时使用
	}

	UnLockBank(m_pBank0Ctrl[bSect].bStorage);
}

#endif

#ifdef EN_DB_PNMAP
//描述:初始化系统的TPnMapCtrl结构,并从文件系统把测量点号到存储号的映射表恢复到pwPnToMemMap
bool InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum)
{
	WORD i, j;
	WORD wMapNum;	//已经映射的个数
	WORD wMN;		//存储号
	bool fFileValid;
	//char szPathName[128];
	//char szHeader[160];
	for (i=0; i<wNum; i++)
	{
		//初始化TPnMapCtrl结构
		pPnMapCtrl[i].dwFileSize = ((pPnMapCtrl[i].wRealNum<<1) + 2)*sizeof(WORD);	//映射保存的文件大小,
		//前面两个WORD用来保存控制信息,其中第一个WORD是已经映射的个数,第二个保留
		pPnMapCtrl[i].wAllocSize = (pPnMapCtrl[i].wRealNum+7)>>3;	//存储空间分配表的大小

		//pPnMapCtrl[i].pwPnToMemMap = new WORD[pPnMapCtrl[i].dwFileSize]; //测量点号到存储号的映射表(需要保存到文件系统)
		//pPnMapCtrl[i].pbAllocTab = new BYTE[pPnMapCtrl[i].wAllocSize];	 //存储空间分配表(不保存到文件系统,动态更新)
		if (pPnMapCtrl[i].pwPnToMemMap==NULL || pPnMapCtrl[i].pbAllocTab==NULL)
		{
			DTRACE(DB_DB, ("InitPnMap: critical error : sys out of memory.\r\n"));
			return false;
		}

		memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
		memset(pPnMapCtrl[i].pbAllocTab, 0, pPnMapCtrl[i].wAllocSize);

		//从文件系统把测量点号到存储号的映射表恢复到pwPnToMemMap
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

			if (pPnMapCtrl[i].fGenerateMapWhenNoneExist)	//当没有映射表的时候，自动生成一一对应的映射表，主要是应对版本升级
			{
				wMapNum = pPnMapCtrl[i].pwPnToMemMap[0] = pPnMapCtrl[i].wRealNum; //已经映射的个数
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)+1] = j; //存储号 wMN<pPnMapCtrl[i].wRealNum
					pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)] = j;		//测量点号<pPnMapCtrl[i].wMaxPn
					pPnMapCtrl[i].pbAllocTab[wMN>>3] |= 1<<(wMN&7);	//表示该存储空间已经分配
				}
			}
		}
		else
		{
			//sprintf(szHeader, "PNMAP%d :", i);
			//TraceBuf(DB_DB, szHeader, (BYTE* )pPnMapCtrl[i].pwPnToMemMap, pPnMapCtrl[i].dwFileSize);

			//根据测量点号到存储号的映射表,初始化存储空间分配表
			wMapNum = pPnMapCtrl[i].pwPnToMemMap[0]; //已经映射的个数
			if (wMapNum > pPnMapCtrl[i].wRealNum)	//映射文件信息有误
			{
				DTRACE(DB_DB, ("InitPnMap: err! wMapNum(%d)>wRealNum(%d) in \r\n", 
								wMapNum, pPnMapCtrl[i].wRealNum));
				memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
			}
			else
			{
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)+1]; //存储号
					if (pPnMapCtrl[i].pwPnToMemMap[2+(j<<1)]<pPnMapCtrl[i].wMaxPn 	//测量点号
						&& wMN<pPnMapCtrl[i].wRealNum) 	//存储号
					{
						pPnMapCtrl[i].pbAllocTab[wMN>>3] |= 1<<(wMN&7);	//表示该存储空间已经分配
					}
					else	//映射文件信息有误
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

//描述:查找测量点对应的映射号
//返回:如果找到则返回对应的映射号,否则返回-1
int SearchPnMap(BYTE bSch, WORD wPn)
{
	int little, big, mid;
	WORD wMapNum;
	TPnMapCtrl* pPnMapCtrl;

	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return -1;

	pPnMapCtrl = &m_pPnMapCtrl[bSch-1];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return -1;

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);	
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}
	
	little = 0;
	big = (int )wMapNum-1;
	while (little <= big)
	{                               
		mid = (little + big) >> 1;  //二分

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

//描述:查找映射号对应的测量点
//返回:如果找到则返回对应的测量点,否则返回-1
int MapToPn(BYTE bSch, WORD wMn)
{
	int iPn = -1;
	WORD i, wMapNum;
	TPnMapCtrl* pPnMapCtrl;

	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return -1;

	pPnMapCtrl = &m_pPnMapCtrl[bSch-1];

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);	
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
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

//描述:申请动态映射测量点
//参数:@bSch 映射方案
// 	   @wPn 需要映射的测量点号
//返回:如果正确返回映射号,否则返回-1
int NewPnMap(BYTE bSch, WORD wPn)
{
	WORD i, j;
	WORD wMapNum;
	WORD wMN;
	TPnMapCtrl* pPnMapCtrl;
	int iMN = SearchPnMap(bSch, wPn);
	if (iMN >= 0)	//已经存在该测量点的映射了
		return iMN;

	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return -1;

	bSch--;	//转换成g_PnMapCtrl数组中对应的索引
	pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
	{
		DTRACE(DB_DB, ("DbNewPnMap: err, wPn(%d)>=wMaxPn(%d)\n", 
					   wPn, pPnMapCtrl->wMaxPn));
		return -1;
	}

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);	//SearchPnMap()也会申请释放m_semPnMap,不能在它前面等信号量
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
	if (wMapNum >= pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		DTRACE(DB_DB, ("DbNewPnMap: there is no room for pn=%d\n", wPn));
		return -1;
	}

	//分配存储号(或者叫映射号)
	for (i=0; i<pPnMapCtrl->wAllocSize; i++)
	{
		if (pPnMapCtrl->pbAllocTab[i] != 0xff)	//有没占用的空间
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

	if (j >= 8)	//应该不会出现这样的错误,以防万一
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	wMN = (i<<3) + j;
	pPnMapCtrl->pbAllocTab[i] |= 1<<j;	//标志该存储空间已经被分配

	//按测量点号从小到大,确定新测量点在映射表中的位置
	for (i=0; i<wMapNum; i++)
	{
		if (wPn < pPnMapCtrl->pwPnToMemMap[2+(i<<1)])
			break;
	}

	//挪出一个放新映射测量点的空间
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

//描述:删除映射测量点
//参数:@bSch 映射方案
// 	   @wPn 已经映射的测量点号
//返回:如果正确返回true,否则返回false
bool DeletePnMap(BYTE bSch, WORD wPn)
{
	WORD i;
	WORD wMapNum, wMN;
	TPnMapCtrl* pPnMapCtrl;
	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return false;

	bSch--;	//转换成g_PnMapCtrl数组中对应的索引
	pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return false;

	WaitSemaphore(m_semPnMap, SYS_TO_INFINITE);
	wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{	
		SignalSemaphore(m_semPnMap);
		return false;
	}
	
	//按测量点号从小到大,确定新测量点在映射表中的位置
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

	//删除对映射资源的占用
	wMN = pPnMapCtrl->pwPnToMemMap[2+(i<<1)+1];
	pPnMapCtrl->pbAllocTab[wMN>>3] &= ~(1<<(wMN&7));	//标志该存储空间已经被分配

	//后面的映射整体往前移动,占用被删除测量点的空间
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


//如果该BANK使用测量点动态映射,则pBankCtrl->wPnNum调整为映射方案中配置的实际支持的测量点数
int GetPnMapRealNum(BYTE bSch)
{
	if (bSch>0 && bSch<=m_wPnMapNum) //方案号应该在1~m_wPnMapNum间
		return m_pPnMapCtrl[bSch-1].wRealNum;
	else
		return -1;
}

//描述:保存测量点动态映射表
int SavePnMap()
{
	int iRet = 0;
	int iFileOff;
	DWORD dwFlg;
	WORD i;

	WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);

	if (m_dwPnMapFileFlg != 0) //有测量点映射文件发生了修改
	{
		dwFlg = 1;
		for (i=0; i<m_wPnMapNum; i++, dwFlg<<1)
		{
			if (m_dwPnMapFileFlg & dwFlg)
			{
				m_dwPnMapFileFlg &= ~dwFlg; //在保存前先清标志,以便在保存的过程中可以反应新的变化
				//从文件系统把测量点号到存储号的映射表恢复到pwPnToMemMap
				memset(g_bDBBuf, 0, DBBUF_SIZE);
				iRet = ReadFileSect(m_pPnMapCtrl[i].wFile, 0, g_bDBBuf, &iFileOff);	//读取文件所在的第dwSect个扇区
				if (iRet < 0)
				{
					SignalSemaphore(g_semDbBuf);
					return -ERR_DEV;	//设备有问题
				}

				memcpy(g_bDBBuf+iFileOff, (BYTE* )m_pPnMapCtrl[i].pwPnToMemMap, m_pPnMapCtrl[i].dwFileSize);
				MakeFile(m_pPnMapCtrl[i].wFile, g_bDBBuf);	//给文件扇区加上文件标志和校验
				if (!writefile(m_pPnMapCtrl[i].wFile, 0, g_bDBBuf))
				{
					if (!writefile(m_pPnMapCtrl[i].wFile, 0, g_bDBBuf))
					{
                        SignalSemaphore(g_semDbBuf);
						DTRACE(DB_DB, ("SavePnMap: fail to save pnmap file\n"));						
						return -ERR_DEV;	//设备有问题
					}
				}
			}
		}
	}

	SignalSemaphore(g_semDbBuf);
	return 0;
}
#endif //EN_DB_PNMAP
