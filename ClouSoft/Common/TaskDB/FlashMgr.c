/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FlashMgr.c
 * ժ    Ҫ��Flash�ռ�������ݴ�Ź滮��������ݵĽṹ���壩
 *			 �ṩ��дFlash�����ֵĽӿڣ����������֣��̶��洢�ռ���м����ݲ��֡��̶��洢�ռ���¼���¼���֡���̬���䲿�֣�
 *
 * ��ǰ�汾��1.0.0
 * ��    �ߣ�������
 * ������ڣ�2011-03-23
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
 * ��    ע��1����Flash��д�Ľ�һ����װ��Ӧ�ó���Ҫ���м����ݱ�����Flash�е�ʱ����Ҫ�ڴ˶������ṹ����Ϣ
 *			 2��������Flash��Ϊ�������֣��̶��洢�ռ���м����ݲ��֡��̶��洢�ռ���¼���¼���֡���̬���䲿�֣�
 *				���ڸ����ִ洢�����ݽṹ�Լ���ʽ�����죬����в�ͬ�Ķ�д�ӿڣ�Ӧ����Ҫ�ϸ�����Ӧ�Ľӿڲ���
 * �汾��Ϣ:
---2011-03-14:---V1.01----������---	
	1��
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

TSem	g_semExFlashBuf;//���ڱ���g_ExFlashBuf
TSem    m_semTaskCfg;
//BYTE g_ExFlashBuf[EXSECT_SIZE];

#pragma section = "NFCRAM"
BYTE *g_ExFlashBuf;//�ⲿFlash��д������        ���������������NFC RAM��

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

//����������F39�и��£������£��޸���Ӧ��Flash��̬����
//������NONE
//���أ����Ĳ����ĸ���
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
    //��ӱ�������Ҫԭ����FLASH��ʽ��̫����̨����Թ���ȥ��ֻ��ÿ�δ��ҷ���ռ��˳��ﵽ��ʽ����Ŀ��
    //�����ַ��������ʽ�������Ͻ�����Ҳ������֮�١�
    //���������ݿⱾ������Զ����ģ���Ϊ��ʽ�����ϵ磬�������Ҫ�����ݿ�ȡһ����������׶˶���
    //��һ���ô�������������˳����һ�������������ھ����㷨���ӳ���FLASH��������
    if (ReadItemEx(BN24, PN0, 0x5025, &bOrder) < 0)//0-�������ռ䣬1-�������ռ�        //todo:liyan
		return false;

	TdbWaitSemaphore();

	//1��ɨ�������ļ��������OK��FN�������λ��ȫ�ַ�������򵱲�����
	memset(g_TdbCtrl.bGlobalFat, 0x00, sizeof(g_TdbCtrl.bGlobalFat));
	memset(bFnValidFlg, 0, sizeof(bFnValidFlg));
	memset(g_TdbCtrl.bFat, 0, FAT_SIZE);  //���ź����������Խ��ã�g_TdbCtrl.bFat

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
		if (wSectCnt != wNeedSects)	//֧�ֵ�FN���������������� || ��֧�ֵ�FN����������������0
		{
			TdbFree(bFn, g_TdbCtrl.bFat);
			continue;
		}

		if (!fFnSup)
			continue;

		for (i=0; i<FAT_FLG_SIZE; i++)
		{
			if ((g_TdbCtrl.bGlobalFat[i] & g_TdbCtrl.bFat[i]) != 0x00)	//���ظ���������
				break;
		}

		if (i < FAT_FLG_SIZE) //���ظ���������
		{
			TdbFree(bFn, g_TdbCtrl.bFat);
			continue;
		}

		//������ȫ��ȷ
		for (i=0; i<FAT_FLG_SIZE; i++)
		{
			g_TdbCtrl.bGlobalFat[i] |= g_TdbCtrl.bFat[i];
		}

		bPos = (bFn-1)>>3;
		bMask = 1<<((bFn-1)&7);
		bFnValidFlg[bPos] |= bMask;
	}

	//2��Ϊû����ɹ���FN����ռ�
	for (bFn=1; bFn<=FN_MAX; bFn++)
	{
		bPos = (bFn-1)>>3;
		bMask = 1<<((bFn-1)&7);

		if (IsFnSupByPn(bFn) && (bFnValidFlg[bPos] & bMask)==0) //û����ɹ���FN
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


//�������ϵ�ʱɨ��Flash����ȡȫ���ļ������g_TdbCtrl.bGlobalFat��Ҳ����ɨ���ļ������
//������NONE
//���أ������ȷ������true,���򷵻�false
//�ܽ᣺
bool FlashInit()
{
#ifdef SYS_WIN
	DWORD i;
	int iFileLen = GetFileLen(USER_PARA_PATH"InFlash.dat");
	if (iFileLen != INSECT_SIZE*INSECT_NUM)
	{
		for (i=0; i<INSECT_NUM; i++)
		{
			//if (InFlashRd(i*INSECT_SIZE, g_ExFlashBuf, INSECT_SIZE) != INSECT_SIZE)	//�ϵ��һ�ζ�ʧ�ܣ����ļ������ڣ�����
			{
				InFlashEraseSect(i*INSECT_SIZE);
			}
		}
	}
	/*for (i=0; i<INSECT_NUM; i++)
	{
		if (InFlashRd(i*INSECT_SIZE, g_ExFlashBuf, INSECT_SIZE) != INSECT_SIZE)	//�ϵ��һ�ζ�ʧ�ܣ����ļ������ڣ�����
		{
			InFlashEraseSect(i*INSECT_SIZE);
		}
	}*/

	//windows����Ҫ�Ƚ��ļ���ʼ��һ��
	iFileLen = GetFileLen(USER_PARA_PATH"ExFlash.dat");
	if (iFileLen != EXSECT_SIZE*FLASH_SECT_NUM)
	{
		for (i=0; i<FLASH_SECT_NUM; i++)
		{
			//if (ExFlashRd(i*EXSECT_SIZE, g_ExFlashBuf, EXSECT_SIZE) != EXSECT_SIZE)	//�ϵ��һ�ζ�ʧ�ܣ����ļ������ڣ�����
			{
				ExFlashEraseSect(i*EXSECT_SIZE);
			}
		}
	}
	/*for (i=0; i<FLASH_SECT_NUM; i++)
	{
		if (ExFlashRd(i*EXSECT_SIZE, g_ExFlashBuf, EXSECT_SIZE) != EXSECT_SIZE)	//�ϵ��һ�ζ�ʧ�ܣ����ļ������ڣ�����
		{
			ExFlashEraseSect(i*EXSECT_SIZE);
		}
	}*/
#endif

	g_ExFlashBuf = GetNfcRamAddr();
	g_semExFlashBuf = NewSemaphore(1, 1);	//�ź�����ʼ��
	m_semTaskCfg = NewSemaphore(1, 1);

	DTRACE(DB_TASKDB, ("Flash::Init OK! Ver %s\r\n", FM_VERSION));
	return true;
}

//��������g_ExFlashBuf����ΪĬ��ֵ
//������NONE
//���أ�NONE
static void MakeDefaultSect(BYTE *pbBuf, WORD wSize)
{
	memset(pbBuf, 0xff, wSize);
	MakeSect(pbBuf, wSize);
}

//����������ΪMakeDataInSect���񣬼��㳤��ΪwLen������ȫ��ΪbDefaultֵ�Ļ���ռ��У��ֵ
//������@wLen ���ݳ���---ָ����Ч���ݳ��ȣ�������У��
//		@bDefault Ĭ��ֵ
//���أ�У��ֵ��У��ֵΪwLen��bDefault�ܺ�ģ��0xff��ֵȡ��
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

//��������g_ExFlashBuf��wStart��ʼ��ÿwPeriod�������У�飬У��ֵ�����wPeriodλ��
//������@wStart ��ʼλ��
//		@wPeriod ÿһ�εĳ���
//���أ��������һ���������ں�ʣ��Ŀռ�
WORD MakeDataInSect(WORD wStart, WORD wPeriod, BYTE *pbBuf, WORD wSize)
{
	WORD wOffset = wStart;	//��ʼ�����ƫ��
	BYTE bChkSum = CalcChkSum(wPeriod-1, 0xee);

	while (wOffset <= wSize-2)
	{
		pbBuf[wOffset-1] = bChkSum;
		wOffset += wPeriod;
	}

	return wOffset-(wSize-2)-1;
}

//���������pbFat��ռ�õĶ�̬Flash�ռ估��Ϣ,ֻ�������Flash��������ļ������
//������@bFn ��Ҫ������ļ������������bFn
//		@pbFat ��Ҫ������ļ��������Ϣ
//���أ������ȷ������true,���򷵻�false
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
			if (dwSect >= DYN_SECT_NUM)	//�Ѿ��������˶�̬�����������˳���
				return true;

			if (pbFat[wByte] & (0x01<<bBit)) //��������ռ��
			{
				ExFlashEraseSect(FADDR_DYN+EXSECT_SIZE*dwSect);
			}
		}
	}
	
	return true;
}

//���������g_ExFlashBuf
//������NONE
//���أ������ȷ������true,���򷵻�false
static bool CheckSect(BYTE *pbBuf, WORD wSize)
{
	BYTE bCheckVal;
	bCheckVal = ~CheckSum(pbBuf, wSize-2);
	
	if (pbBuf[wSize-2]==0x55 && pbBuf[wSize-1]==bCheckVal)
		return true;
	
	return false;
}

//��������g_ExFlashBuf���ļ���־�Լ���У��
//������NONE
//���أ�NONE
static void MakeSect(BYTE *pbBuf, WORD wSize)
{
	pbBuf[wSize-2] = 0x55;
	pbBuf[wSize-1] = (BYTE)(~CheckSum(pbBuf, wSize-2));
	
	return;
}

//���������pbFat�Ƿ���ȷ
//������@pbFat ��Ҫ�����ļ������
//���أ������ȷ������true,���򷵻�false
bool CheckFat(BYTE* pbFat)
{
	BYTE bCheckVal;
	bCheckVal = ~CheckSum(pbFat, FAT_SIZE-3);

	if ((pbFat[FAT_SIZE-2]==0x55) && (pbFat[FAT_SIZE-3]==bCheckVal))
		return true;
	else
		return false;
}

//��������pbFat��У��
//������@pbFat ��Ҫ����У����ļ������
//���أ�NONE
void MakeFat(BYTE *pbFat)
{
	pbFat[FAT_SIZE-3] = (BYTE)(~CheckSum(pbFat, FAT_SIZE-3));	//FAT��У��
	pbFat[FAT_SIZE-2] = 0x55;
	pbFat[FAT_SIZE-1] = 0x00;	//������У��

	return;
}

//����������¼���¼pbRec�Ƿ���ȷ
//������@pbRec ��Ҫ����У��ļ�¼
//���أ������ȷ������true,���򷵻�false
bool CheckAlrRec(BYTE *pbRec, DWORD dwLen)
{
	return (pbRec[0] == ((BYTE)(~CheckSum(pbRec+1, dwLen-1))));
}

//���������¼���¼pbRec��У��
//������@pbRec��Ҫ����У����¼���¼
//���أ�NONE
void MakeAlrRec(BYTE *pbRec, DWORD dwLen)
{
	pbRec[0] = (BYTE)(~CheckSum(pbRec+1, dwLen));

	return;
}

//�����������ʷ��¼pbRec�Ƿ���ȷ
//������@pbRec ��Ҫ����У��ļ�¼
//		@nLen ��ʷ��¼�ĳ��ȣ�����У��
//���أ������ȷ������true,���򷵻�false
bool CheckData(BYTE *pbRec, WORD nLen)
{
	return (pbRec[nLen-1] == ((BYTE)(~CheckSum(pbRec, nLen-1))));
}

//����������ʷ��¼pbRec��У��
//������@pbRec��Ҫ����У�����ʷ��¼
//		@nLen ��ʷ��¼�ĳ��ȣ�����У��
//���أ�NONE
void MakeData(BYTE *pbRec, WORD nLen)
{
	pbRec[nLen-1] = (BYTE)(~CheckSum(pbRec, nLen-1));
}

//����������wSectOffsetƫ�Ƶ�������ַ
//������@wSectOffset ����ƫ����,��0��ʼ
//���أ������ȷ���㣬����Flash��ַ�����򷵻�-1
DWORD SectToPhyAddr(WORD wSect)
{
	return FADDR_DYN + wSect*EXSECT_SIZE;
}

//����������pbFnMacTab�����wByteOffset, bBitOffsetƫ�Ƶ������ĵ�ַ
//������@wByteOffset �ļ��������ֽ�ƫ��
//		@bBitOffset �ֽ�ƫ���µ�λƫ��
//���أ����ض�Ӧ��Flash��ַ
DWORD FatToPhyAddr(WORD wByte, BYTE bBit)
{
	return SectToPhyAddr((wByte<<3)+bBit);
}

//������Ѱ��pbFnMacTab����dwAddr������������һ����Ч������ַ
//������@*pbFnMacTab �ļ������
//	    @dwAddrSect ��ǰ������ַ�����ص�����һ����Ч������ַ
//���أ������ȷ���ң�������һ�����ĵ�ַ�����򷵻�0
WORD SchNextSect(BYTE *pbFat, WORD wSect)
{
	WORD wByte;
	BYTE bBit;

	//���pbFnMacTabΪ�գ���ʾ���ڹ̶��洢�ռ��ڣ�����Ҫ���ң�ֱ�ӷ���Flash��һ��������ַ
	if (pbFat == NULL)
		return wSect+1;

	//�����Ƕ�̬������
	if (wSect < DYN_SECT_START)
		return 0xffff;

	wSect++;	//����һ��������ʼ��
	wSect -= DYN_SECT_START;	//ת��Ϊ��̬����ĵ�һ��������0��ʼ��
	wByte = wSect>>3;
	bBit = wSect&7;

	for (; wByte<FAT_FLG_SIZE; wByte++)
	{
		for (; bBit<8; bBit++)
		{
			if ((wByte<<3)+bBit >= DYN_SECT_NUM)	//�Ѿ��������˶�̬�����������˳���
				return 0xffff;

			if ((pbFat[wByte]&(0x01<<bBit)) != 0x00)
			{
				return (DYN_SECT_START+(wByte<<3)+bBit);
			}
		}

		bBit = 0;
	}

	//�����ˣ�˵���Ѿ���dwAddrSect��ʼ��������pbFnMacTab�ˣ��Ҳ�����ֻ�÷���0
	return 0xffff;
}

//������дFlash
//������@dwAddr д��ĵ�ַ
//		@*pbMacTab ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashWrData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen, wLen0 = wLen;
	BYTE i, j;

	wSect = dwAddr / EXSECT_SIZE;
	dwSectOff = dwAddr % EXSECT_SIZE;
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    
	for (i=0; i<2; i++)	//����һ������
	{
		//�ȶ�����������
		for (j=0; j<2; j++)
		{
			if (ExFlashRd(EXSECT_SIZE*wSect, g_ExFlashBuf, EXSECT_SIZE) != EXSECT_SIZE)
				continue;
			
			if (CheckSect(g_ExFlashBuf, EXSECT_SIZE)) //����־��У��
				break;
		}
		
		if (j >= 2)
		{
			MakeDefaultSect(g_ExFlashBuf, EXSECT_SIZE);
			//return TDB_ERR_FLS_RD_FAIL;
		}

		if (dwSectOff+wLen > EXSECT_SIZE-2)	//������
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

//������дFlash����У��
//������@dwAddr д��ĵ�ַ
//		@*pbMacTab ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashWrDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen, wLen0 = wLen;
	BYTE i, j;

	wSect = dwAddr / EXSECT_SIZE;
	dwSectOff = dwAddr % EXSECT_SIZE;
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    
	for (i=0; i<2; i++)	//����һ������
	{
		//�ȶ�����������
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

		if (dwSectOff+wLen > EXSECT_SIZE)	//������
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

//��������Flash
//������@dwAddr д��ĵ�ַ
//		@*pbFat ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashRdData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen;
	
	dwSectOff = dwAddr % EXSECT_SIZE;
	if (dwSectOff+wLen > EXSECT_SIZE-2)	//������
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

//��������Flash����У��
//������@dwAddr д��ĵ�ַ
//		@*pbFat ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashRdDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen)
{
	DWORD dwSectOff;
	WORD wSect, wCpyLen;
	
	dwSectOff = dwAddr % EXSECT_SIZE;
	if (dwSectOff+wLen > EXSECT_SIZE)	//������
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



//��������TPnTmp���������ó�0������ʱʹ��
//������@*pPnTmp�������м����ݻ���
//		@wNum PnTmp�ĸ���
//���أ���
void ClrPnTmp(const TPnTmp* pPnTmp, WORD wNum)
{
	WORD i;

	for (i=0; i<wNum; i++)
	{
		memset(pPnTmp->pbData, 0x00, pPnTmp->wLen);
		pPnTmp++;
	}
}

//���������ⲿFlash��ȡ��Ҫ�����ĳ�������
//������@*pbBuf ���������ļ�������buf����ȫ�����pbBuf������ʹ��g_ExFlashBuf
//		@nLen ���ζ�ȡ�ĳ���
//		@dwOffset �����ļ�ͷdwStartOffsetƫ�Ƶ�λ�ÿ�ʼ��ȡ
//���أ������ȷ��ȡ�����������򷵻�true
//      ���򷵻�flash
//ע�⣺��������鳤�ȣ�Ӧ�ó�����Ҫ�Լ���֤���յĻ������㹻��
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

//�����������õĳ���д���ⲿFlash��
//������@*pbBuf д���ⲿFlash�ĳ����buffet
//		@nLen ����дFlash�ĳ���
//		@dwStartOffset �����ļ�ͷdwStartOffsetƫ�Ƶ�λ�ÿ�ʼдFlash
//���أ������ȷд�������򷵻�true
//      ���򷵻�false
//ע�⣺��������鳤�ȣ�Ӧ�ó�����Ҫ�Լ���֤���յĻ������㹻��
bool WriteUpdateProg(BYTE* pbBuf, DWORD nLen, DWORD dwOffset)
{
	DWORD dwAddr;

	dwAddr = FADDR_UPDFW + dwOffset;
	
	if (ExFlashWrDataNoChk(dwAddr, NULL, pbBuf, nLen) < 0)
		return false;

	return true;
}

//������д���������У�鼰���������־
//������@wCrc16 crcУ��ֵ
//���أ����У����ȷ����true�����򷵻�falsh
bool WriteUpdProgCrc(char *szPfile, DWORD dwUpdLen, WORD wCrc16)
{
	DWORD dwAddr = FADDR_UPDINFO;
	BYTE bBuf[UPDINFO_LEN];	//����������Ϣ������
	
	bBuf[0] = 0x55;
	bBuf[1] = 0xaa;
	memcpy(bBuf+2, (BYTE*)szPfile, 16);
	
	DWordToByte(dwUpdLen, &bBuf[18]);
	WordToByte(wCrc16, &bBuf[22]);
		
	if (ExFlashWrDataNoChk(dwAddr, NULL, bBuf, UPDINFO_LEN) < 0)
		return false;
	
	return true;
}

//���������������bPn��bFileNum�м�������
//������@wPn��Ҫ��ȡ���ݵĲ�����ţ�F10�����õĲ������
//���أ������ȷ����������򷵻�true
//      ���򷵻�false
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
		
	//�ж��ļ�ͷ��־λ
	if (ReadItemEx(BN24, PN0, 0x410a, &bFlag) < 0)
		return false;
	
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    
    dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//��Ӧ��4K�ռ���׵�ַ
	memset(g_ExFlashBuf, 0, EXSECT_SIZE);
    
	g_ExFlashBuf[0] = bFlag;	//д�汾��
	
	MakeSect(g_ExFlashBuf, EXSECT_SIZE);//����ļ���־��У��
	
	//��д�꽫��������д��
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


//��������ȡ������wPn��bFileNum�м�������
//������@wPn��Ҫ��ȡ���ݵĲ�����ţ�F10�����õĲ������
//	    @*pPnTmp �������м����ݻ���
//		@wNum PnTmp�ĸ���
//���أ������ȷ��ȡ�����������򷵻�true
//      ���򷵻�false
//ע�⣺��������鳤�ȣ�Ӧ�ó�����Ҫ�Լ���֤���յĻ������㹻��
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

	dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//��Ӧ��4K�ռ���׵�ַ

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
			
	//�ж��ļ�ͷ��־λ
	if (ReadItemEx(BN24, PN0, 0x410a, &bFlag) < 0)
	{
		SignalSemaphore(g_semExFlashBuf);
		return false;
	}
	
	if (bFlag != g_ExFlashBuf[0])//�汾��һ��
	{
		SignalSemaphore(g_semExFlashBuf);
		return false;
	}    
	
	//��ַ���
	for (i=0; i<wNum; i++)
	{
		memcpy(pTmp->pbData, pPtrTmp+dwOffset, pTmp->wLen); //pPtrTmpָ��g_ExFlashBuf
		dwOffset += pTmp->wLen;
		pTmp++;
	}	
    
    SignalSemaphore(g_semExFlashBuf);  //�����Ӧ�û�

	return true;
}

//���������������bPn��bFileNum�м�������
//������@wPn��Ҫ��ȡ���ݵĲ�����ţ�F10�����õĲ������
//	    @*pPnTmp �������м����ݻ���
//		@wNum PnTmp�ĸ���
//���أ������ȷ�����������򷵻�true
//      ���򷵻�false
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
	
	dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//��Ӧ��4K�ռ���׵�ַ

	//�ж��ļ�ͷ��־λ
	if (ReadItemEx(BN24, PN0, 0x410a, &bFlag) < 0)
	{
		DTRACE(DB_TASKDB, ("WritePnTmp: Read 0x410a fail!\r\n"));
		return false;
	}

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	g_ExFlashBuf[0] = bFlag;	//д�汾��
	//һ����д����
	for (i=0; i<wNum; i++)
	{
		memcpy(pPtrTmp+dwOffset, pTmp->pbData, pTmp->wLen);
		dwOffset += pTmp->wLen;
		pTmp++;
	}
	
	MakeSect(g_ExFlashBuf, EXSECT_SIZE);//����ļ���־��У��
	
	//��д�꽫��������д��
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
	dwLen = bPnNum * GetFmtARDLen(bIdx) * 10 ;//����
	dwLen += bPnNum ;//����
	dwLen += bPnNum ;//У��
	return dwLen;
}

bool GetAlrAddr(DWORD dwId, BYTE bPn, DWORD* dwAddr, WORD* wAlrLen)
{
#define ALR_ADDR 0//��ǿ�ṩ��ַλ
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
				dwOffset += GetIdAlrLen(i, bPn) ;//����
			}

			*wAlrLen = GetFmtARDLen(i);
			fGetAddr = true;
			break;
		}
		else
		{
			//����������¼��ĳ��ȡ�10��������+ÿ��Pn��1���ֽڵ���ʵ����+ÿ��Pn��1���ֽڵ�У��λ
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

//����������bAlrType���͵��¼�pbBuf
//������@bAlrType�¼����ͣ���ͨ�¼�or��Ҫ�¼�
//	    @*pbBuf�������ݻ�����
//		@nLen �������ݳ���
//���أ������ȷ�����������򷵻ر������ݵĳ���
//      ���򷵻���Ӧ�Ĵ����루����С����ĸ�����
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
	//��ȡ�ܹ��洢�ı���
	memset(bBuf, 0x00, EXC_DATA_OFFSET);
	if (ExFlashRdData(dwAddress, NULL, bBuf, EXC_DATA_OFFSET) < 0)
	{
		return -1;
	}

	if (FADDR_ALRREC==dwAddress)
		wMaxNum = EXC_MAX_ALR_NUM;//�澯������
	else if (FADDR_RPTFAILREC==dwAddress)
		wMaxNum = ERR_MAX_ALR_NUM;//�澯�ϱ�ʧ�ܺ�����������
	else if (FADDR_EVTREC==dwAddress)
		wMaxNum = ERR_MAX_ALR_NUM;//�¼�������

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

//��������ȡbAlrType���͵��¼�pbBuf
//������@bAlrType�¼����ͣ���ͨ�¼�or��Ҫ�¼�
//		@bAlrPtr ��Ҫ��ȡ�ļ�¼ָ��0~255
//	    @*pbBuf�������ݻ�����
//���أ������ȷ��ȡ�������򷵻����ݵĳ���
//      ���򷵻���Ӧ�Ĵ����루����С����ĸ�����
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
	//��ȡ�ܹ��洢�ı���
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
		if(wTotalNum >= wTotal)//��ֹ��ȡ��Ч����
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
			//�ж�PN
			if(ByteToWord(pbBuf+1) != bPn)
				continue;

			//�ж�ID	
			if(dwId != dwTepId)
				continue;
		}
		else
		{
			if(dwTepId == 0)
				continue;
			*wAlrLen = GetFmtARDLen(dwTepId);
		}

		//�ж�У��
		if (!CheckAlrRec(pbBuf, ALRSECT_SIZE))
		{		
			continue;
		}
		break;//��û���⣬����
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
	//�ж�У��
	if (!CheckAlrRec(pbBuf, ALRSECT_SIZE))
	{		
		return TDB_ERR_DATA_CHKSUM;
	}
	return ALRSECT_SIZE;
}
//������������ܱ�����բ�����ò�������ע��pbBufΪTYkCtrl��ָ�룬
//		�ڶ�д��ʱ���ѵ����ṹ���С�ֽ�TYkCtrl��д
//������@bPn������
//		@*pbBufΪ���������
//���أ������ȷ���������ݣ��򷵻ر������ݵĳ���
//		���򷵻���Ӧ�Ĵ����루����С����ĸ�����
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

//������������ܱ�����բ�����ò�������ע��pbBufΪTYkCtrl��ָ�룬
//		�ڶ�д��ʱ���ѵ����ṹ���С�ֽ�TYkCtrl��д
//������@bPn������
//		@*pbBufΪ���������
//		@nLenΪ����ĳ���
//���أ������ȷ���������ݣ��򷵻ر������ݵĳ���
//		���򷵻���Ӧ�Ĵ����루����С����ĸ�����
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

	MakeAlrRec(bBuf, sizeof(TYkCtrl));   //to do :��ȷ�ϼ�¼����
	iRet = ExFlashWrData(dwAddr, NULL, bBuf, sizeof(TYkCtrl)+1);

	return iRet;
}

//��������ȡbFn���ļ������pbFat��ֻ��������������ж���Ч��
//������@bFn ��Ҫ��ȡ���ļ�������
//	    @*pbFat �������ļ�������Ż���
//���أ������ȷ��ȡ�򷵻�TDB_ERR_OK,���򷵻���Ӧ������
//ע�⣺ÿ���ļ��������У�飬Ϊ��ʹһ�������ܹ�������������ļ�������ֽ��ļ��������Ƴ�
//		1���ļ������ĵ�0λ����ֽڶ�Ӧ��FLASH����ʼ��̬�ռ䣬������ԭ����FLASH�׵�ַ�ռ�
//		2��Ŀǰ��̬�ռ�һ��ռ����152��������19���ֽڵ��ļ����������Ҳ����˵�ļ����������19���ֽ�����Ч��
//		   ��ʹ������3���ֽ����ļ����������У���������ֽ�FAT_SIZE-3��FAT_SIZE-2��FAT_SIZE-1
//		   �ֱ��Ӧ���ļ������У��λ�������ļ����λ(0x55)���Լ�����У��λ(������ļ�����������������һ��λ�ã����λΪ0)
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
		//DTRACE(DB_TASKDB, ("ReadFat: Read FN%d file table chksum fail!\r\n", bFn));  //todo:��֧�ֵ�FN��û�з���ռ�
		return TDB_ERR_MACT_CHKSUM;		//�������ˣ�����У�鲻��
	}
	else
	{
		DTRACE(DB_TASKDB, ("ReadFat: Read FN%d file table flash fail!\r\n", bFn));
		return TDB_ERR_FLS_RD_FAIL;		//������������
	}
}

//��������bFn���ļ������pbFatд�뵽Flash��Ӧλ��
//������@bFn ��Ҫд����ļ�������
//	    @*pbFat �ļ��������
//���أ������ȷ����true�����򷵻�false
//ע�⣺ÿ���ļ��������У�飬Ϊ��ʹһ�������ܹ�������������ļ�������ֽ��ļ��������Ƴ�
//		1���ļ������ĵ�0λ����ֽڶ�Ӧ��FLASH����ʼ��̬�ռ䣬������ԭ����FLASH�׵�ַ�ռ�
//		2��Ŀǰ��̬�ռ�һ��ռ����152��������19���ֽڵ��ļ����������Ҳ����˵�ļ����������19���ֽ�����Ч��
//		   ��ʹ������3���ֽ����ļ����������У���������ֽ�FAT_SIZE-3��FAT_SIZE-2��FAT_SIZE-1
//		   �ֱ��Ӧ���ļ������У��λ�������ļ����λ(0x55)���Լ�����У��λ(������ļ�����������������һ��λ�ã����λΪ0)
bool WriteFat(BYTE bFn, BYTE* pbFat)
{
	DWORD dwOffset, dwSectAddr;	//MALLOC_TAB_ADDR����һ���������׵�ַ
	BYTE i;
		
	if (bFn > FN_MAX)
		return false;
	
	dwSectAddr = FADDR_FAT + (bFn-1)/FAT_PER_SECT*EXSECT_SIZE;
	dwOffset = FAT_SIZE * ((bFn-1)%FAT_PER_SECT);
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	for (i=0; i<2; i++)
	{
		//�Ȱѷ���������������ݶ�����
		if (ExFlashRd(dwSectAddr, g_ExFlashBuf, EXSECT_SIZE) > 0)
		{
			if (CheckSect(g_ExFlashBuf, EXSECT_SIZE))
				break;
		}
	}

	if (i >= 2)
	{
		MakeDefaultSect(g_ExFlashBuf, EXSECT_SIZE);	//У������ˣ��������
		//return false;
	}

	//���˾͸�����
	memcpy(&g_ExFlashBuf[dwOffset], pbFat, FAT_SIZE);

	MakeSect(g_ExFlashBuf, EXSECT_SIZE);

	//��д�꽫����д��Flash
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

//����������ļ�������Ƿ���Ч
//������@*pbFat �ļ������
//���أ�����������Ч����true����Ч����false
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

//���������bFn���ļ��������Ϣ
//������@bFn ��Ҫ������ļ�������
//������@pbBuf ���������ⲿ������
//������@wBufSize�������Ĵ�С
//���أ�����ɹ�����򷵻�true,���򷵻�false
bool ClrFat(BYTE bFn, BYTE* pbFat)
{          
	memset(pbFat, 0x00, FAT_SIZE);

	MakeFat(pbFat);
	
	if (!WriteFat(bFn, pbFat))
		return false;
	
	return true;
}

//��������������ļ��������Ϣ
//������NONE
//���أ�����ɹ�����򷵻�true,���򷵻�false
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
		
		if (FatIsInValid(g_TdbCtrl.bFat))	//�ͷŹ���bFn�ļ�������ǿ���ͨ��CheckFat�ģ�������ȫΪ0
			continue;
		
		ClrFat(bFn, g_TdbCtrl.bFat);
	}
	
	TdbSignalSemaphore();
	return true;
}


//�������ҵ��ļ������pbFat�׸���̬����ռ������ƫ��
//������@pbFat �ļ������
//		@nSect ��Ҫ���ҵĵ�wSect����Ч����
//���أ����ص�1����̬����������ƫ�ƺ�,��1��������ƫ��Ϊ0
WORD SchSectInFat(BYTE *pbFat, WORD wSect)
{
	WORD wIdx = 0;	//����������
	WORD wByte;
	BYTE bBit;

	for (wByte=0; wByte<FAT_FLG_SIZE; wByte++)
	{
		if (pbFat[wByte] == 0)
			continue;

		for (bBit=0; bBit<8; bBit++)
		{
			if ((wByte<<3)+bBit >= DYN_SECT_NUM)	//�Ѿ��������˶�̬�����������˳���
				return 0xffff;

			if (pbFat[wByte] & (0x01<<bBit))
			{
				wIdx++;		//������������1
				if (wIdx == (wSect+1))
					return (wByte<<3)+bBit;	//�ҵ��ͷ�������ƫ��
				//else
				//	wIdx++;		//������������1
			}
		}
	}

	return 0xffff;
}


//��������ֱ��1����������ݽ���У��
WORD CheckPnData(const TPnTmp* pPnTmp, WORD wNum)
{
	WORD i;
	BYTE bSum = 0;

	for (i=0; i<wNum; i++)		//ֱ��Ҫ����Ĳ�����������Ŀ����
	{
		bSum += CheckSum(pPnTmp[i].pbData, pPnTmp[i].wLen);
	}

	return 0x5500 + bSum;
}

//������ȡ��ֱ��1�����ݵ�����������ݵĴ�С
WORD GetPnDataSize(const TPnTmp* pPnTmp, WORD wNum)
{
	WORD i;
	WORD wSize = 0;
	for (i=0; i<wNum; i++)		//ֱ��Ҫ����Ĳ�����������Ŀ����
	{
		wSize += pPnTmp[i].wLen;
	}

	return wSize;
}

//������ֱ��1�����ݵ������������
//������@wPn��Ҫ��ȡ���ݵĲ�����ţ�F10�����õĲ������
//	    @*pPnTmp �������м����ݻ���
//		@wNum PnTmp�ĸ���
//���أ������ȷ��ȡ�����������򷵻�true
//      ���򷵻�false
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

	dwAddr = FADDR_PNTMP + iPn*EXSECT_SIZE;//��Ӧ��4K�ռ���׵�ַ

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
	if (ExFlashRd(dwAddr, g_ExFlashBuf, wPnDataSize) < 0)
	{
		if (ExFlashRd(dwAddr, g_ExFlashBuf, wPnDataSize) < 0)
		{
			SignalSemaphore(g_semExFlashBuf);
			return false;
		}
	}

	//��ַ���
	for (i=0; i<wNum; i++)
	{
		memcpy(pPnTmp[i].pbData, &g_ExFlashBuf[dwOffset], pPnTmp[i].wLen);
		dwOffset += pPnTmp[i].wLen;
	}
	SignalSemaphore(g_semExFlashBuf);

	return true;
}

//���������ExFlashEraseChip��Ŀ���ǽ�Լʱ�䣬
bool ClrAllData(void)
{
    //��FAT�����
    /*FADDR_FAT;
    FADDR_PNTMP;
    FADDR_VITALR;
    FADDR_COMALR;
    FADDR_EXTPARA;*/
    //��������û�в���
    
    //��ʽ����Χ
    DWORD dwStart = FADDR_FAT;
    
    /*for (dwStart=FADDR_FAT; dwStart<FADDR_DYN; )
    {
        ExFlashEraseSect(i);
        dwStart += EXSECT_SIZE;
    }*/
    //����ô����Ŀ����Ϊ���������ٶȲ���
    while(dwStart < FADDR_EXTPARA)
    {
        if ((dwStart&0xffff) == 0) //��ʼ��ַ����64K�����
            break;        
        else if (((dwStart&0x7fff) == 0) && ((dwStart+0x8000)<=FADDR_EXTPARA))//��ʼ��ַ����32K�����
        {
            ExFlashEraseBlock(dwStart, 0);
            dwStart += 0x8000;
        }
        else if ((dwStart&0xfff) == 0)
        {
            ExFlashEraseSect(dwStart);
            dwStart += EXSECT_SIZE;
        }
        else  //��ʼ��ַ���������׵�ַ
            return false;
    }
    
    //����64K���
    while((dwStart+0x10000) <= FADDR_EXTPARA)  //FADDR_DYN
    {
        ExFlashEraseBlock(dwStart, 1);
        dwStart += 0x10000;
    }
    
    //ʣ�๻32K����32K����������4K��
    while(dwStart < FADDR_EXTPARA)  //FADDR_DYN
    {        
        if (((dwStart&0x7fff) == 0) && ((dwStart+0x8000)<=FADDR_EXTPARA))//��ʼ��ַ����32K�����  //FADDR_DYN
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
