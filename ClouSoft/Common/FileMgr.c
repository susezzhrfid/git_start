/*********************************************************************************************************
* Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�FileMgr.c
* ժ    Ҫ�����ļ���Ҫ���ڶ��ļ�����ӿڽ��з�װ
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2010-12-28
*********************************************************************************************************/
#include "TypeDef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "FileMgr.h"
#include "Trace.h"
#include "sysapi.h"
#include "DbConst.h"
#include "FlashIf.h"

#include "FlashMgr.h"
#include "SysDebug.h"

//�ļ����������˵����
//1�������Ĵ�С���ָ�ʵ�ʴ洢�豸����һ�£��ֱ�ΪINSECT_SIZE(Ƭ��FLASH������С)��EXSECT_SIZE(Ƭ��FLASH������С)
//2��ҳ�Ĵ�С������Ƭ��FLASH����Ƭ��FLASH��ͳһΪ256���ֽ�
//3�����ڴ���ļ��������������䣬����С��������Сһ����ļ����鰴��ҳ�����䡣
//4��ÿ���ļ����ǰ�����������ҳ�����������з���
//5��Ϊ�˼򻯱�̣����ڰ���ҳ���з�����ļ����ļ����ܳ��������Ľ�β

//�ļ���д��˵����
//1������һ���������ļ����ṩ�����ļ��ź��ļ��ڲ�ƫ�ƵĶ�
//2������д�ļ�ǰ�Ķ����ṩ���������ŵ�������������Ӧ�ó�������޸���Ӧ���֣��������ֱ��ֲ��䣬
//	 Ȼ����������д��FLASH


static const TFileCfg g_InFileCfg[] = {	//Ƭ��FLASH�ļ����ýṹ
//��ʼ��������	��ʼ��ҳ��		ռ��ҳ����������
	{0,				-1,					1},		//0--�����㶯̬ӳ����ձ��ļ�(193*2+2)*2
	{1,				0,					2},		//1--�ն˱��ֲ�����304
	{1,				2,					1},		//2--BANK1��չ������40	
	{2,				-1,					1},		//3--BANK10��չ������462
	{3,				-1,					1},		//4--BANK24��չ������523
	{4,				-1,					1},		//5--sect1�ն˲��� 540
	{5,				-1,					13},	//6--sect4��������� 13115=13k,ԣ��3k //ע��˴�����Ϊ16�������ϵ��У�鲻��
	{21,			-1,					3},		//7--BN11����   1386	
	{24,			0,					2},		//8--BANK25У׼������328
};


TFileCfg g_ExFileCfg[] = {	//Ƭ��FLASH�ļ����ýṹ
//��ʼ��������	��ʼ��ҳ��		ռ����������
	{0,				-1,					1},		//���籣������ 135
	{1,				-1,					57},	//sect5�ն���չ����229597, 57������  
	{58,            -1,                 2},     //�洢�����֤����
	{60,            -1,                 2},     //�洢��ʱ����
	{62,            -1,                 1},     //�洢���������
};

const TFileCfg* File2Cfg(WORD wFile)
{
	if (wFile < 0x80)
	{
		if (wFile >= sizeof(g_InFileCfg)/sizeof(TFileCfg))
			return NULL;

		return &g_InFileCfg[wFile];
	}
	else
	{
		wFile -= 0x80;
		if (wFile >= sizeof(g_ExFileCfg)/sizeof(TFileCfg))
			return NULL;

		return &g_ExFileCfg[wFile];
	}
}

//�������Ƿ��ǰ�ҳ������ļ�
bool IsPageFile(WORD wFile)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	if (pFileCfg->nPage >= 0)	//��ҳ����
		return true;
	else							//����������
		return false;
}

//������ȡ���ļ���ʼƫ��
int GetFileOffset(WORD wFile, DWORD dwSect)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return -ERR_DEV;	//�豸������

	if (pFileCfg->nPage >= 0)	//��ҳ����
	{	
		dwSect = pFileCfg->nSect;	//��������������
		return PAGE_SIZE*pFileCfg->nPage;	//�����ļ���ʼƫ��
	}
	else						//����������
	{	
		dwSect += pFileCfg->nSect;	//��ʼ������ĵ�dwSect������
		return 0;	//�����ļ���ʼƫ��
	}
}

//��������ȡ�ļ����ڵĵ�dwSect������
//������@dwSect �ļ��ĵڼ������������ڰ�ҳ����ģ���Ϊ0
//		@piFileOff	���������ļ���ʼƫ��
//���أ������ȷ����0�������򷵻ظ���
int ReadFileSect(WORD wFile, DWORD dwSect, BYTE* pbData, int* piFileOff)
{
	int iRet;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return -ERR_DEV;	//�豸������

	if (pFileCfg->nPage >= 0)	//��ҳ����
	{	
		dwSect = pFileCfg->nSect;	//��������������
		*piFileOff = PAGE_SIZE*pFileCfg->nPage;	//�����ļ���ʼƫ��
	}
	else						//����������
	{	
		dwSect += pFileCfg->nSect;	//��ʼ������ĵ�dwSect������
		*piFileOff = 0;	//�����ļ���ʼƫ��
	}

	if (wFile < 0x80)
	{
		iRet = InFlashRd(INSECT_SIZE*dwSect, pbData, INSECT_SIZE);
		if (iRet == INSECT_SIZE)
			return 0;
		else if (iRet == -ERR_NOTEXIST)	//�ļ�������
		{
			memset(pbData, 0, INSECT_SIZE);
			return 0;
		}
		else 
			return -ERR_DEV;	//�豸������
	}
	else
	{
		iRet = ExFlashRd(EXSECT_SIZE*dwSect+EXFLASH_PARA_OFFSET, pbData, EXSECT_SIZE);	//�ⲿFLASH���޶������Ķ�
		if (iRet == EXSECT_SIZE)
			return 0;
		else if (iRet == -ERR_NOTEXIST)	//�ļ�������
		{
			memset(pbData, 0, EXSECT_SIZE);
			return 0;
		}
		else 
			return -ERR_DEV;	//�豸������
	}	
}

//������������ļ���
//������@wFile �ļ���
// 		@dwOffset �ļ��ڲ�ƫ�ƣ��������ļ���־��У���
//		@pbData ���ջ���
// 		@iLen ��ȡ����, -1��ʾ��Ĭ�ϳ��ȶ������ڰ�ҳ����Ķ�ȡ�����ļ������ڰ���������Ķ�һ������
//��ע��$��Ϊ��ĳ��IDʱ�ǲ���ҪУ��ģ�����������ʱ������Ҫ������ֻ�����ָ����ַ�����ݶ�������������У�飬
//		У�齻��Ӧ�ô���
//		$�ļ���ȡ��3�������
//		1���ϵ�ϵͳ��ɨ��ʱ��������������dwOffset��dwLen����������
//		2����������ʱ����ĳ��ID������������ض����ñ�����Ҳ����
//		3����������ʱ��дĳ��ID���������������ȶ�������dwOffset��dwLen����������
bool readfile(WORD wFile, DWORD dwOffset, BYTE* pbData, int iLen)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	if (pFileCfg->nPage >= 0)	//�԰�ҳ����ģ�������ʼƫ��
		dwOffset += PAGE_SIZE*pFileCfg->nPage;

	if (iLen < 0)	//-1��ʾ��Ĭ�ϳ��ȶ�
	{
		if (pFileCfg->nPage >= 0)	//���ڰ�ҳ����Ķ�ȡ�����ļ�
			iLen = PAGE_SIZE*pFileCfg->wSectNum;
		else						//���ڰ���������Ķ�һ������
			iLen = GetSectSize(wFile);
	}

	if (wFile < 0x80)
    {
		return InFlashRd(INSECT_SIZE*pFileCfg->nSect+dwOffset, pbData, iLen) == iLen;	//�ڲ�FLASH���޶������Ķ�
    }
	else
    {
        dwOffset += EXFLASH_PARA_OFFSET;  //�ⲿ����ƫ��
		return ExFlashRd(EXSECT_SIZE*pFileCfg->nSect+dwOffset, pbData, iLen) == iLen;	//�ⲿFLASH���޶������Ķ�
    }
}

//������д�ļ�
//������@wFile �ļ���
//		@pbData Ҫд�����ݻ���
// 		@dwSect �ļ��ĵڼ������������ڰ�ҳ����ģ���Ϊ0
// 		@dwLen д����
//��ע��$�ļ���Ч��־��У����Ӧ����ã�������ֻ���������д��ָ����ַ
//		$�ļ�д��������������д��dwOffset��dwLen���������룬����֮ǰ�����ϲ㻺���
bool writefile(WORD wFile, DWORD dwSect, BYTE* pbData)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	if (pFileCfg->nPage >= 0)	//���ڰ�ҳ����ģ��ļ��ĵڼ���������Ϊ0
		dwSect = 0;

	if (wFile < 0x80)
    {
		return InFlashWrSect(INSECT_SIZE*pFileCfg->nSect+INSECT_SIZE*dwSect, pbData, INSECT_SIZE) == INSECT_SIZE;	//�ڲ�FLASH�޶�������д
    }
	else
    {        
		return ExFlashWrSect(EXSECT_SIZE*pFileCfg->nSect+EXSECT_SIZE*dwSect+EXFLASH_PARA_OFFSET, pbData, EXSECT_SIZE) == EXSECT_SIZE;	//�ⲿFLASH�޶�������д
    }
}

//����Ƭ���ļ�
bool EraseInFile(WORD wFile)
{            
	DWORD dwUsedSize;
	WORD wSect, wSectNum;
    int iFileOff;
    int iRet;
    
    const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;
    
    if (wFile > 0x80)  //Ƭ��Ŀ����ò�������
        return true;
        
    wSectNum = GetFileSectNum(wFile);	//ȡ���ļ��������������
	dwUsedSize = GetSectUsedSize(wFile);//ȡ���ļ�ʵ��ʹ�õ�������С
    
    WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
    
    for (wSect=0; wSect<wSectNum; wSect++)
	{
		memset(g_bDBBuf, 0, DBBUF_SIZE);
		iRet = ReadFileSect(wFile, wSect, g_bDBBuf, &iFileOff);	//��ȡ�ļ����ڵĵ�dwSect������
		if (iRet < 0)
			goto ret_err;

		memset(g_bDBBuf+iFileOff, 0, dwUsedSize+2); //���ļ�������У��Ҳ���
		//MakeFile(pBankCtrl->wFile, g_bDBBuf);	//���ļ����������ļ���־��У��
		if (!writefile(wFile, wSect, g_bDBBuf))
			goto ret_err;
	}    

    SignalSemaphore(g_semDbBuf);
    return true;
    
ret_err:	
    SignalSemaphore(g_semDbBuf);
    DTRACE(DB_CRITICAL, ("fail to EraseFile.\r\n"));
    return false;
}

//��������Ƭ���ļ�,������У׼ϵ��
bool EraseAllInFile(void)
{
    bool fRet = true;
	BYTE i;
	for (i=0; i<sizeof(g_InFileCfg)/sizeof(TFileCfg); i++)
    {
        if (i == FILE_BN25_PARA)  //���ɲ���
        {
            continue;
        }
        
        fRet &= EraseInFile(i);
    }
    return fRet;
}

bool EraseAllInSpace(void)
{
#ifndef SYS_WIN
	DWORD dwAddr = INPROG_ADDR;

	//WaitSemaphore(g_semDbBuf, SYS_TO_INFINITE);
	memset(g_bDBBuf, 0, DBBUF_SIZE);
	while (dwAddr < INFLASH_END)
	{
		InFlashWrSect(dwAddr, g_bDBBuf, INSECT_SIZE);
		dwAddr += INSECT_SIZE;
	}
#endif //SYS_WIN
	//SignalSemaphore(g_semDbBuf);
	return true;
}

//������ȡ���ļ���ֱ�Ӷ���ַ
//������@wFile �ļ���
// 		@dwOffset �ļ��ڲ������ݵ�ƫ��
BYTE* GetFileDirAddr(WORD wFile, DWORD dwOffset)
{
	DWORD dwSect, dwSectOff, dwUsedSize;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return NULL;

	dwUsedSize = GetSectUsedSize(wFile);//ȡ���ļ�ʵ��ʹ�õ�������С
	dwSect = dwOffset / dwUsedSize;
	dwSectOff = dwOffset % dwUsedSize;

	if (pFileCfg->nPage >= 0)	//�԰�ҳ����ģ�������ʼƫ��
		dwSectOff += PAGE_SIZE*pFileCfg->nPage;

	if (wFile < 0x80)
#ifndef SYS_WIN
	  	return ParaAddrConv(INSECT_SIZE*pFileCfg->nSect + INSECT_SIZE*dwSect + dwSectOff);
#else
		return (BYTE* )(INSECT_SIZE*pFileCfg->nSect + INSECT_SIZE*dwSect + dwSectOff);	//�ڲ�FLASH���޶������Ķ�
#endif
	else
		return NULL;	//�ⲿFLASH���޶������Ķ�
}

//������ȡ���ļ������������С
DWORD GetSectSize(WORD wFile)
{
	if (wFile < 0x80)
		return INSECT_SIZE;		//Ƭ��FLASH������С
	else
		return EXSECT_SIZE;		//Ƭ��FLASH������С
}


//������ȡ���ļ�ʵ��ʹ�õ�������С
DWORD GetSectUsedSize(WORD wFile)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return 0;

	if (pFileCfg->nPage >= 0)	//�԰�ҳ����ģ�������ʼƫ��
		return PAGE_SIZE*pFileCfg->wSectNum - 2;  //��ȥ��У��
	else
		return GetSectSize(wFile) - 2;
}

//������ȡ���ļ��������������
WORD GetFileSectNum(WORD wFile)
{
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return 0;

	if (pFileCfg->nPage >= 0)	//��ҳ����
		return 1;
	else							//����������
		return pFileCfg->wSectNum;
}

//�����������ļ�ȫ������ĳ�������ĺϷ���
//������@iFileOff �ļ���pbFile�е�ƫ��
//��ע���ٶ��ļ����ǵ����������ģ����ǰ�������������������
bool CheckFile(WORD wFile, BYTE* pbFile, int iFileOff)
{
	DWORD dwEnd, dwUsedSize;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return false;

	dwUsedSize = GetSectUsedSize(wFile);

	if (pFileCfg->nPage >= 0)	//��ҳ����
		dwEnd = iFileOff + PAGE_SIZE*pFileCfg->wSectNum;
	else							//����������
		dwEnd = GetSectSize(wFile);

	if (pbFile[dwEnd-2] != 0x55)
		return false;

	if (pbFile[dwEnd-1] != CheckSum(pbFile+iFileOff, (WORD)dwUsedSize))
		return false;

	return true;
}

//���������ļ����������ļ���־��У��
//��ע���ٶ��ļ����ǰ��������������е�
void MakeFile(WORD wFile, BYTE* pbFile)
{
	int iFileOff;
	DWORD dwEnd, dwUsedSize;
	const TFileCfg* pFileCfg = File2Cfg(wFile);
	if (pFileCfg == NULL)
		return;

	dwUsedSize = GetSectUsedSize(wFile);
	if (pFileCfg->nPage >= 0)	//��ҳ����
	{
		iFileOff = PAGE_SIZE*pFileCfg->nPage;
		dwEnd = PAGE_SIZE*pFileCfg->nPage + PAGE_SIZE*pFileCfg->wSectNum;
	}
	else							//����������
	{	
		iFileOff = 0;
		dwEnd = GetSectSize(wFile);
	}

	pbFile[dwEnd-2] = 0x55;
	pbFile[dwEnd-1] = CheckSum(pbFile+iFileOff, (WORD)dwUsedSize);
}

//��ע���漰�������ⲿFLASH�ģ�ͳһ����g_semExFlashBuf�ź�������g_ExFlashBuf
bool ReadPwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen)
{
	BYTE i;
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	for (i=0; i<2; i++)
	{
		if (readfile(FILE_PWROFF_TMP, 0, g_ExFlashBuf, -1))
		{
			if (CheckFile(FILE_PWROFF_TMP, g_ExFlashBuf, 0))
				break;
		}
	}

	if (i >= 2)
	{
		memset(pPwrOffTmp, 0, nLen);
		SignalSemaphore(g_semExFlashBuf);
		//InitPoweroffTmp();
		
		return false;
	}

	memcpy(pPwrOffTmp, g_ExFlashBuf, nLen);

	SignalSemaphore(g_semExFlashBuf);
	return true;
}

//��ע���漰�������ⲿFLASH�ģ�ͳһ����g_semExFlashBuf�ź���
bool WritePwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen)
{
	BYTE i;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	memset(g_ExFlashBuf, 0, DBBUF_SIZE);
	memcpy(g_ExFlashBuf, pPwrOffTmp, nLen);

	MakeFile(FILE_PWROFF_TMP, g_ExFlashBuf);

	for (i=0; i<2; i++)
	{
		if (writefile(FILE_PWROFF_TMP, 0, g_ExFlashBuf))
		{
			break;
		}
	}

	SignalSemaphore(g_semExFlashBuf);
	if (i < 2)
		return true;
	else
		return false;
}
