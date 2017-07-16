/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TaskDB.c
 * ժ    Ҫ���������ݿ�ʵ�֣��������ݹ��������¼��������������
  *
 * ��ǰ�汾��1.0.0
 * ��    �ߣ�������
 * ������ڣ�2011-03-23
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
 * ��    ע��1�������ʹ�õ�ʱ����Ҫ��ʹ���ļ�һ�����ȳɹ���Ȼ�����ִ����ز���
 *           2���������Ӽ�¼����Ҫ�����ñ�����Ϣ��������Ϣ�ȡ�ȫ��������ⰲ�ź�
 *			 3����������ȡ��¼��ʱ�򣬲���֧�����������Լ���������������������ã���˹�������Ӧ�ó�����ɡ�
 *				Ӧ�ó����Լ���������վ����¼�������֣���һ������������ȡ
 * �汾��Ϣ:
 ---2011-03-14:---V1.01----������---	
 	1��
************************************************************************************************************/
//#include "stdafx.h"
#include <stdio.h>
#include "TaskDB.h"
#include "CommonTask.h"
#include "DbConst.h"
#include "FlashMgr.h"
#include "ComConst.h"
#include "SysDebug.h"
#include "ComAPI.h"
#include "DbAPI.h"


#define TDB_VERSION			"1.0.0"
#define TDB_RECBUF_LEN      256	//û�м�¼����256���ֽڳ��ȡ�


TTdbCtrl	g_TdbCtrl;

//������������ʼ��
//������NONE
//���أ��ɹ�����true
bool TdbInit()
{
	memset(&g_TdbCtrl, 0, sizeof(g_TdbCtrl));
	g_TdbCtrl.fIsLocked = false;
	g_TdbCtrl.bFn = INVALID_FN;
    
    g_TdbCtrl.semTdb = NewSemaphore(1, 1);

	if (!FlashInit())
		return false;

	DTRACE(DB_TASKDB, ("TaskDB::Init:Ver %s.\r\n", TDB_VERSION));

	return true;
}


//������������Ƿ�����
//������NONE
//���أ���������true
bool TdbIsLock()
{
	return g_TdbCtrl.fIsLocked;
}

//�����������������
//������NONE
//���أ�NONE
void TdbLock()
{
	g_TdbCtrl.fIsLocked = true;
}

//����������������
//������NONE
//���أ�NONE
void TdbUnLock()
{
	g_TdbCtrl.fIsLocked = false;
}


void TdbWaitSemaphore()
{
	WaitSemaphore(g_TdbCtrl.semTdb, SYS_TO_INFINITE);
}

void TdbSignalSemaphore()
{
	SignalSemaphore(g_TdbCtrl.semTdb);
}

//���������㵥����ͨ������Ҫ�Ĵ洢�ռ�
//������@bFn��ͨ�����
//���أ���Ҫʹ�õĿռ�
DWORD CalcSpacePerComTask(BYTE bFn)
{
	g_TdbCtrl.bSmplIntervU = GetComTaskSmplIntervU(bFn);
	g_TdbCtrl.bSmplIntervV = GetComTaskSmplInterV(bFn);
	g_TdbCtrl.dwPerDataLen = GetComTaskPerDataLen(bFn)+5;
	return g_TdbCtrl.dwPerDataLen*GetComTaskRecNum(bFn);
}

//����������һ����ͨ��������ڸ�������ʼ�洢ƫ��
//������@bFn��ͨ�����
//		@bBuf�ñʼ�¼ʱ��
//���أ����������洢�׵�ַƫ��
DWORD CalcComTaskOneRecOffSet(BYTE bFn, BYTE* bBuf)
{
	BYTE bWeek, bTmBuf[5];
	TTime tmRec;
	DWORD dwDay, dwOffSet = 0;
	memset(&tmRec, 0, sizeof(tmRec));
	memcpy(bTmBuf, bBuf, sizeof(bTmBuf));
	Swap(bTmBuf, sizeof(bTmBuf));
	Fmt15ToTime(bTmBuf, &tmRec);
	switch(g_TdbCtrl.bSmplIntervU)
	{
		case TIME_UNIT_MINUTE://��
		{
			bWeek = DayOfWeek(&tmRec);
			if (g_TdbCtrl.bSmplIntervV != 0)
				dwOffSet = ((bWeek-1)*1440 + tmRec.nHour*60 + tmRec.nMinute)/g_TdbCtrl.bSmplIntervV; //ÿ����һ��,�ɴ�һ������
			break;
		}
		case TIME_UNIT_HOUR://ʱ
		{
			dwDay = DaysFrom2000(&tmRec);
			dwOffSet = dwDay%15*24 + tmRec.nHour; //ÿСʱһ��,�ɴ�����
			break;
		}
		case TIME_UNIT_DAY://��
		{
			dwOffSet = tmRec.nDay - 1; //ÿ���һ��,�ɴ�һ����
			break;
		}
		case TIME_UNIT_MONTH://��
		{
			dwOffSet = tmRec.nMonth - 1; //ÿ�´�һ��,�ɴ�һ��
			break;
		}
	}

	dwOffSet *= g_TdbCtrl.dwPerDataLen; //5�ֽ��Ǽ�¼ʱ��
	return dwOffSet;
}

//���������㵥���м�������Ҫ�Ĵ洢�ռ�
//������@bFn�м������
//���أ���Ҫʹ�õĿռ�
DWORD CalcSpacePerFwdTask(BYTE bFn)
{
	g_TdbCtrl.bSmplIntervU = GetFwdTaskSmplIntervU(bFn);
	g_TdbCtrl.bSmplIntervV = GetFwdTaskSmplInterV(bFn);
	g_TdbCtrl.dwPerDataLen = GetFwdTaskPerDataLen(bFn)+5;
	return g_TdbCtrl.dwPerDataLen*GetFwdTaskRecNum(bFn);
}

//����������һ���м���������ڸ�������ʼ�洢ƫ��
//������@bFn�м������
//		@bBuf�ñʼ�¼ʱ��
//���أ����������洢�׵�ַƫ��
DWORD CalcFwdTaskOneRecOffSet(BYTE bFn, BYTE* bBuf)
{
	BYTE bWeek, bTmBuf[5];
	TTime tmRec;
	DWORD dwDay, dwOffSet = 0;
	memset(&tmRec, 0, sizeof(tmRec));
	memcpy(bTmBuf, bBuf, sizeof(bTmBuf));
	Swap(bTmBuf, sizeof(bTmBuf));
	Fmt15ToTime(bTmBuf, &tmRec);
	switch(g_TdbCtrl.bSmplIntervU)
	{
		case TIME_UNIT_MINUTE://��
		{
			bWeek = DayOfWeek(&tmRec);
			if (g_TdbCtrl.bSmplIntervV != 0)
				dwOffSet = ((bWeek-1)*1440 + tmRec.nHour*60 + tmRec.nMinute)/g_TdbCtrl.bSmplIntervV; //ÿ����һ��,�ɴ�һ������
			break;
		}
		case TIME_UNIT_HOUR://ʱ
		{
			dwDay = DaysFrom2000(&tmRec);
			dwOffSet = dwDay%15*24 + tmRec.nHour; //ÿСʱһ��,�ɴ�����
			break;
		}
		case TIME_UNIT_DAY://��
		{
			dwOffSet = tmRec.nDay - 1; //ÿ���һ��,�ɴ�һ����
			break;
		}
		case TIME_UNIT_MONTH://��
		{
			dwOffSet = tmRec.nMonth - 1; //ÿ�´�һ��,�ɴ�һ��
			break;
		}
	}

	dwOffSet *= g_TdbCtrl.dwPerDataLen; //5�ֽ��Ǽ�¼ʱ��
	return dwOffSet;
}


//����������bFn��Ҫ���ٿռ䣬��λ��BYTE
//������@bFn ���ඳ���Fn
//���أ���Ҫ������
DWORD CalcFnNeedSpaces(BYTE bFn)
{
	const TCommTaskCtrl *pTaskCtrl = ComTaskFnToCtrl(bFn);

	if (bFn < FN_COMSTAT)
	{
		if (pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_P0)
		{
			return CalcSpacesPerPn(pTaskCtrl); //�ն�����ֻ����һ��
		}
		else	
			return CalcSpacesPerPn(pTaskCtrl) * PN_VALID_NUM;
	}
	else if (bFn < FN_FWDSTAT)
	{
		return CalcSpacePerComTask(bFn-FN_COMSTAT);
	}
	else if (bFn < FN_MAX)
	{
		return CalcSpacePerFwdTask(bFn-FN_FWDSTAT);
	}
	else
		return 0;
}

//����������bFn��Ҫ���������Ŀռ䣬��λ��section
//������@bFn ���ඳ���Fn
//���أ���Ҫ������
WORD CalcFnNeedSects(BYTE bFn)
{
	DWORD dwNeedSpaces = CalcFnNeedSpaces(bFn);

	return (dwNeedSpaces%(EXSECT_SIZE-2)? dwNeedSpaces/(EXSECT_SIZE-2) + 1 : dwNeedSpaces/(EXSECT_SIZE-2));
}

//���������㵥��Pn��bFn������Ҫ���ٿռ䣬��λ��Byte����TCommTaskCtrlΪ����
//������@pTaskCtrl bFn��������ƽṹ
//���أ���Ҫʹ�õĿռ��ܺ�
DWORD CalcSpacesPerPn(const TCommTaskCtrl *pTaskCtrl)
{
	//��������ÿ��������
	return CalcPerPnMonRecSize(pTaskCtrl) * ComTaskGetMonthNum(pTaskCtrl);
}

//���������㵥��Pn�����µ�bFn������Ҫ���ٿռ䣬��λ��Byte����TCommTaskCtrlΪ����
//������@pTaskCtrl bFn��������ƽṹ
//���أ���Ҫʹ�õĿռ�
DWORD CalcPerPnMonRecSize(const TCommTaskCtrl *pTaskCtrl)
{
	DWORD dwRecSize;

	if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)//�¶����Լ������ն��ᣬһ��һ��
		dwRecSize = ComTaskGetDataLen(pTaskCtrl) + ComTaskGetRecOffset(pTaskCtrl) + 1;	//+1 �ӵ���У��
	else
		dwRecSize = CalcPerPnDayRecSize(pTaskCtrl) * ComTaskGetRecNumPerPnMon(pTaskCtrl);

	return dwRecSize;
}

//���������㵥��Pn�����bFn������Ҫ���ٿռ䣬��λ��Byte����TCommTaskCtrlΪ����
//������@pTaskCtrl bFn��������ƽṹ
//���أ���Ҫʹ�õĿռ�
DWORD CalcPerPnDayRecSize(const TCommTaskCtrl *pTaskCtrl)
{
	WORD wRecSize;

	if (pTaskCtrl == NULL)
		return 0;
	
	if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
		return 0;

	wRecSize = ComTaskGetDataLen(pTaskCtrl);
	
	if (pTaskCtrl->bIntervU < TIME_UNIT_DAY)
	{
		wRecSize *= 24;
		wRecSize += CURVE_PER_DAY*ComTaskGetRecOffset(pTaskCtrl);	//�����������ﲻ��1������ݣ�ֻ��1���������ݣ�6��Сʱ1�ʣ�
		wRecSize += CURVE_PER_DAY;	//+1 �ӵ���У��
	}
	else
	{
		wRecSize += ComTaskGetRecOffset(pTaskCtrl);	//�����������ﲻ��1������ݣ�ֻ��1���������ݣ�6��Сʱ1�ʣ�
		wRecSize += 1;	//+1 �ӵ���У��
	}

	return wRecSize;
}


//���������㵥��Pn����һ�ʼ�¼��bFn������Ҫ���ٿռ䣬��λ��Byte����TCommTaskCtrlΪ����
//������@pTaskCtrl bFn��������ƽṹ
//���أ���Ҫʹ�õĿռ�
DWORD CalcOneCurveRecSize(const TCommTaskCtrl *pTaskCtrl)
{
	WORD wRecSize;

	if (pTaskCtrl == NULL)
		return 0;

	if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
		return 0;

	wRecSize = ComTaskGetDataLen(pTaskCtrl);

	if (pTaskCtrl->bIntervU < TIME_UNIT_DAY)
		wRecSize *= (24/CURVE_PER_DAY);	//6

	wRecSize += ComTaskGetRecOffset(pTaskCtrl);	//�����������ﲻ��1������ݣ�ֻ��1���������ݣ�6��Сʱ1�ʣ�
	wRecSize += 1;	//+1 �ӵ���У��

	return wRecSize;
}


//��������[wByte].bBitOffset��ʼ����pbFat�ļ�������б�־nCount��������
//������@pbFat �ļ������
//		@wByte �ļ�������ֽ��ϵ�ƫ��
//		@bBit �ļ�������ֽ���λ��ƫ��
//		@nCount ��־�ĸ���
//���أ�NONE
bool SectMarkUsed(BYTE* pbFat, WORD wByte, BYTE bBit)
{
	if (wByte>=FAT_FLG_SIZE || bBit>7) //������
		return false;

	pbFat[wByte] |= (0x01<<bBit);

	return true;
}

//���������bTime��tTime�Ƿ�ƥ��
//������@bTime BYTE��ʽ��ʱ��
//		@tTime TTime��ʽ��ʱ��
//		@bIntervU ��������
//���أ����ƥ�䷵��true�����򷵻�false
bool IsSchTimeMatch(BYTE *bTime, TTime tTime, BYTE bIntervU)
{
	 if (bIntervU == TIME_UNIT_MONTH)//�¶�����Ҫ��-�¾Ϳ�����
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_HOUR)	//���߶�������bTime��ʽΪ��-ʱ-��-��-�� �����з�-ʱ��Ϊ0
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[2]))   )
			return true;
		else
			return false;
	}
	else if ((bIntervU==TIME_UNIT_DAY))	//������Ҫ�ж�ʱ��ĸ�ʽ��Ϊ��-��-��
	{
		//���߶���Ҫ��λ��Сʱ����Ҫ��-��-��
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[2]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_MINUTE)
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[0]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[2]))
			&& (tTime.nHour==BcdToByte(bTime[3]))
			&& ((tTime.nMinute)==BcdToByte(bTime[4]))  )
			return true;
		else
			return false;
	}

	return false;
}

//���������bTime��tTime�Ƿ�ƥ��
//������@bTime BYTE��ʽ��ʱ��
//		@tTime TTime��ʽ��ʱ��
//		@bIntervU ��������
//���أ����ƥ�䷵��true�����򷵻�false
bool IsSchRecTimeMatch(BYTE *bTime, TTime tTime, BYTE bIntervU)
{
	if (bIntervU == TIME_UNIT_MINUTE)
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[4]))
			&& (tTime.nMonth==BcdToByte(bTime[3]))
			&& (tTime.nDay==BcdToByte(bTime[2]))
			&& (tTime.nHour==BcdToByte(bTime[1]))
			&& ((tTime.nMinute)==BcdToByte(bTime[0]))  )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_MONTH)//�¶�����Ҫ��-�¾Ϳ�����
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[1]))
			&& (tTime.nMonth==BcdToByte(bTime[0]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_HOUR)	//���߶�������bTime��ʽΪ��-ʱ-��-��-�� �����з�-ʱ��Ϊ0
	{
		if (   (tTime.nYear%100==BcdToByte(bTime[4]))
			&& (tTime.nMonth==BcdToByte(bTime[3]))
			&& (tTime.nDay==BcdToByte(bTime[2]))   )
			return true;
		else
			return false;
	}
	else if (bIntervU == TIME_UNIT_DAY)	//������Ҫ�ж�ʱ��ĸ�ʽ��Ϊ��-��-��
	{
		//���߶���Ҫ��λ��Сʱ����Ҫ��-��-��
		if (   (tTime.nYear%100==BcdToByte(bTime[2]))
			&& (tTime.nMonth==BcdToByte(bTime[1]))
			&& (tTime.nDay==BcdToByte(bTime[0]))   )
			return true;
		else
			return false;
	}
	
	return false;
}

//-----------------------------------------------------------------------
//		ͨ���ܵ�ȡ�õļ�¼��ʽ����ȷ���£�
//		�ն��ᡢ�����ն���
//		ʱ��(������)+�������(BYTE)+����------��->��->��->PN->Data
//		�¶���
//		ʱ��(����)+�������(BYTE)+����--------��->��->PN->Data
//		����
//		ʱ��(FMT15)+�������(BYTE)+����-------��->ʱ->��->��->��->PN->Data
//-----------------------------------------------------------------------
//������ȡ������ͷ�������������
//������@*pbData �ܵ��ﴫ���������ݰ�
//		@*pTaskCtrl ���ƽṹ
//���أ�����pbData���Ӧ��pTaskCtrl��ʽ������������
BYTE GetMonth(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl)
{
// 	if (pTaskCtrl->bIntervU==TIME_UNIT_HOUR || pTaskCtrl->bIntervU==TIME_UNIT_MINUTE)
// 		return BcdToByte(*(pbData+3));
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAY)
// 		return BcdToByte(*(pbData+1));
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
// 		return BcdToByte(*pbData);
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAYFLG)
// 		return BcdToByte(*(pbData+1));
	if (pTaskCtrl->bIntervU)
	{
		return BcdToByte(*(pbData+1));
	}
	else
		return 0;
}

//������ȡ������ͷ�������������
//������@*pbData �ܵ��ﴫ���������ݰ�
//		@*pTaskCtrl ���ƽṹ
//���أ�����pbData���Ӧ��pTaskCtrl��ʽ������������
BYTE GetDay(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl)
{
// 	if (pTaskCtrl->bIntervU==TIME_UNIT_HOUR || pTaskCtrl->bIntervU==TIME_UNIT_MINUTE)
// 		return BcdToByte(*(pbData+2));
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAY)
// 		return BcdToByte(*pbData);
// 	else if (pTaskCtrl->bIntervU == TIME_UNIT_DAYFLG)
// 		return BcdToByte(*pbData);
	if (pTaskCtrl->bIntervU)
	{
		return BcdToByte(*(pbData+2));
	}
	else
		return 0;
}

//������ΪbFn����Flash�ռ�
//������@bFn ���ඳ���Fn
//���أ������ȷ���뵽�ռ䣬��������ռ���׵�ַ��һ������0��������
//      �������ʧ�ܣ��᷵��С��0�Ĵ�����
//ע�⣺����ռ���ļ������ṹΪ���¶��壺
//		�ļ�������Ǹ���Flash��ַ��ͷ��β��Ӧ�ֲ��ģ����Ǵ�Flash�Ķ�̬�ռ俪ʼ
//		Ҳ����˵FnMalcTab[0]��Ӧ��������Flash��̬�ռ俪ʼ�ĵ�һ��������FnMalcTab����һֱ��Flash�ռ���ĩ
//		��ΪFlash�ռ仹��һ�����Ǿ�̬�����˵ģ��ⲿ�ֿռ䲻���ļ���������ʾ�������ļ��������ĩ�ض���
//		һ���ֵ�λ������Ч�ģ���һ����Ч��������ΪSTATICAREA_USED_SECT/8����һ���ļ�������ڳ�ʼ����ʱ��
//		����Ϊ0��
//		�ļ����������һλ�������ļ�������У��λ��У��ֵ�����ļ������ǰFAT_SIZE-1�����ȵĺ�ȡ��
bool TdbMalloc(BYTE bFn, BYTE* pbFat, BYTE bOrder)
{
	int iByte;
	WORD wNeedSects;	//��Ҫ�����������
	WORD wAllocSects = 0;	//�Ѿ������������
	int iBit;
	
	if (TdbIsLock())	//������Ƿ��Ѿ�������ɾ���ռ��ʱ���TDB_ERR_LOCK
	{
		DTRACE(DB_TASKDB, ("TaskDB::TdbMalloc: TaskDB is locked\r\n"));
		return false; //������ʱ��ֻ��ɾ������������ 
	}

	memset(pbFat, 0, FAT_SIZE);

	wNeedSects = CalcFnNeedSects(bFn); //�����bFn��������������Ҫ�õ�g_TdbCtrl
	wAllocSects = 0;
    	
    if ((bOrder&0x01) == 0) //�������
    {
    	//��ȫ�ַ�������Ѽ��ռ�
    	for (iByte=bOrder%5; iByte<FAT_FLG_SIZE; iByte++)
    	{
    		if (g_TdbCtrl.bGlobalFat[iByte] == 0xff)
    			continue;
    
    		for (iBit=0; iBit<8; iBit++)
    		{
    			if ((iByte<<3)+iBit >= DYN_SECT_NUM)	//�Ѿ��������˶�̬�����������˳���
    				goto TdbMalloc_end;
    
    			if (wAllocSects == wNeedSects) //�������㹻�Ŀռ䣬��¼������
    				goto TdbMalloc_end;	//�п��ܻ��������һ�������Ÿպ÷����꣬���������������ж���Ҫ�ŵ�������
    
    			if ((g_TdbCtrl.bGlobalFat[iByte] & (1<<iBit)) == 0) //������δ��ʹ�ã�ռ����
    			{
    				SectMarkUsed(pbFat, iByte, iBit);
    				wAllocSects++;
    			}
    		}	//for (bBit=0; bBit<8; bBit++)
    	}	//for (wByte=0; wByte<FAT_FLG_SIZE; wByte++)
    }
    else//�������
    {    
        //��ȫ�ַ�������Ѽ��ռ�
        for (iByte=FAT_FLG_SIZE-1-bOrder%5; iByte>=0; iByte--)
        {
            if (g_TdbCtrl.bGlobalFat[iByte] == 0xff)
    			continue;
            for (iBit=7; iBit>=0; iBit--)
            {
                if ((iByte<<3)+iBit >= DYN_SECT_NUM)	//�����˶�̬������������
    				continue;
                if (wAllocSects == wNeedSects) //�������㹻�Ŀռ䣬��¼������
    				goto TdbMalloc_end;	//�п��ܻ��������һ�������Ÿպ÷����꣬���������������ж���Ҫ�ŵ�������
                if ((g_TdbCtrl.bGlobalFat[iByte] & (1<<iBit)) == 0) //������δ��ʹ�ã�ռ����
    			{
    				SectMarkUsed(pbFat, iByte, iBit);
    				wAllocSects++;
    			}
            }
        }
    }

TdbMalloc_end:
	if (wAllocSects == wNeedSects)
	{
		MakeFat(pbFat);

		// ��������������Ϣ��bFn�ļ������д��Flash
		if (!WriteFat(bFn, pbFat))
		{
			DTRACE(DB_TASKDB, ("TdbMalloc: Write FN%d FAT fail\r\n", bFn));
			return false;
		}

/*		if (!CleanFlash(bFn, pbFat))	//��ʽ�����뵽�Ŀռ�
		{
			DTRACE(DB_TASKDB, ("TdbMalloc: Clean FN%d Spaces Fail\r\n", bFn));
			return false;
		}
        ------- �ٶ�̫���������ʱ����ն�̬������д���ݵ�ʱ�������*/
		for (iByte=0; iByte<FAT_FLG_SIZE; iByte++)
			g_TdbCtrl.bGlobalFat[iByte] |= pbFat[iByte];//��¼ȫ�ֱ���
        
		DTRACE(DB_TASKDB, ("TdbMalloc: FN%d Malloc Sucess\r\n", bFn));
		return true;
	}

	DTRACE(DB_TASKDB, ("TdbMalloc: FN%d malloc fail, Because flash is full\r\n", bFn));
	return false;
}

//�������ͷ�bFn�����Flash�ռ�
//������@bFn ���ඳ���Fn
//���أ������ȷ�ͷſռ䣬����1�����򷵻���Ӧ�Ĵ�����
bool TdbFree(BYTE bFn, BYTE* pbFat)
{
	memset(pbFat, 0x00, FAT_SIZE);

	MakeFat(pbFat);

	if (!WriteFat(bFn, pbFat))
	{
		DTRACE(DB_TASKDB, ("TdbFree: Fn%d free FAT fail\r\n", bFn));
		return false;
	}
	
	DTRACE(DB_TASKDB, ("TdbFree: Fn%d free FAT ok\r\n", bFn));
	return true;
}

//�������ú��������ж�ϵͳ�Ƿ���ΪbFn����ռ䣬��Ԥ����Ӧ���ļ������
//       ������TdbCloseTable�ɶԳ��֣����۴򿪳ɹ����
//������@bFn ���ඳ���Fn
//���أ�����Ѿ�����ռ䣬����1�����򷵻���Ӧ�Ĵ�����
int TdbOpenTable (BYTE bFn)
{
	WORD wNeedSects;
	const TCommTaskCtrl* pTaskCtrl = ComTaskFnToCtrl(bFn);
    
    TdbWaitSemaphore();

	if (TdbIsLock())	//������Ƿ��Ѿ�������ɾ���ռ��ʱ���TDB_ERR_LOCK
	{
		DTRACE(DB_TASKDB, ("TdbOpenTable: Tdb is locked\r\n"));
		return TDB_ERR_LOCK; //������ʱ��ֻ��ɾ������������
	}
	
	//if (pTaskCtrl == NULL)
		//return TDB_ERR_NOTEXIST;

	if (ReadFat(bFn, g_TdbCtrl.bFat) < 0)
	{
		g_TdbCtrl.bFn = INVALID_FN;

		DTRACE(DB_TASKDB, ("TdbOpenTable: Read FN%d malloc table fail!\r\n", bFn));
		return TDB_ERR_NOTEXIST;
	}

	wNeedSects = CalcFnNeedSects(bFn);
	if (CalcuBitNum(g_TdbCtrl.bFat, FAT_FLG_SIZE) != wNeedSects)	
	{
		g_TdbCtrl.bFn = INVALID_FN;
		DTRACE(DB_TASKDB, ("TdbOpenTable: FN%d malloc bad!\r\n", bFn));
		return TDB_ERR_NOTEXIST;
	}
	
	g_TdbCtrl.bFn = bFn;
	return TDB_ERR_OK;
}

//���������Ԥ�����ļ������
//      ������TdbOpenTable�ɶԳ���
//������@bFn ���ඳ���Fn
//���أ������ȷ����ˣ�����1�����򷵻���Ӧ�Ĵ�����
int TdbCloseTable(BYTE bFn)
{
    g_TdbCtrl.bFn = INVALID_FN;   
    TdbSignalSemaphore();
	return bFn;
}

/*
ͨ���ܵ�ȡ�õļ�¼��ʽ����ȷ���£�
�ն���
ʱ��(������)+�������(BYTE)+����------��->��->��->PN->Data
�¶���
ʱ��(����)+�������(BYTE)+����--------��->��->PN->Data
����
ʱ��(FMT15)+�������(BYTE)+����-------��->��->��->PN->Data
*/
//����������������һ�ʼ�¼���·������������Ӽ�¼ֻ֧��һ�ʱʼ�
//������@bFn ���ඳ���Fn
//      @pbData��Ҫ��ӵ������ļ�¼
//      @nLen��¼����
//���أ������ȷ����ˣ�����1�����򷵻���Ӧ�Ĵ�����
int TdbAppendRec(BYTE bFn, BYTE *pbData, WORD nLen)
{
	BYTE bTimeLen = 3;	//��д��¼���µ���Чʱ�곤��
	BYTE bPnInRec;	//����FLASH�������¼��Ƚ�PN����
	//for test
	BYTE bBufTmp[132];//��¼����ʱ���棬128�����������ݼ��㣬���������Ϊ5���ֽڣ����Ͽ�ͷ��ʱ��Ͳ�����ţ�21*6+4+1=131��(6��������+3���ֽ�ʱ��+1�ֽڲ������+1���ֽڵ�У���ܹ�131
	WORD wSect, wFullRecSize;
	int iPn;
	DWORD dwAddr = 0, dwOffset;//dwAddr���ݵľ����ַ��dwOffset���ݾ����ַ��ȫ��ƫ��
	const TCommTaskCtrl	*pTaskCtrl = ComTaskFnToCtrl(bFn);

	if (TdbIsLock())
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: Tdb is locked\r\n"));
		return TDB_ERR_LOCK;
	}

	if (g_TdbCtrl.bFn != bFn)	//����⻹û��
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d tab not opened!\r\n", bFn));
		return TDB_ERR_UNKNOW;
	}

	if (pTaskCtrl == NULL)
	{
		if (bFn >= FN_COMSTAT)
		{
			if (bFn < FN_FWDSTAT)
			{
				if (nLen != g_TdbCtrl.dwPerDataLen)
				{
					DTRACE(DB_TASKDB, ("TdbAppendRec: ComTask FN%d Record lens is not matched with g_taskCtrl.\r\n", bFn));
					return TDB_ERR_DATALEN;
				}
				//����ñʼ�¼����������¼�׵�ַƫ��
				dwOffset = CalcComTaskOneRecOffSet(bFn-FN_COMSTAT, pbData);
			}
			else if (bFn < FN_MAX)
			{
				if (nLen > g_TdbCtrl.dwPerDataLen)
				{
					DTRACE(DB_TASKDB, ("TdbAppendRec: FwdTask FN%d Record lens is not matched with g_taskCtrl.\r\n", bFn));
					return TDB_ERR_DATALEN;
				}
				//����ñʼ�¼����������¼�׵�ַƫ��
				dwOffset = CalcFwdTaskOneRecOffSet(bFn-FN_COMSTAT, pbData);
			}
			else
				return TDB_ERR_UNKNOW;

			wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//�����ļ���������̬����ռ��ƫ�ƣ���λ��Ҫ����������
			if (wSect == 0xffff)
			{
				DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
				return TDB_ERR_UNKNOW;
			}
		
			//�ã����¾߱����϶��磬���������׵�ַ
			dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;
		
			//�Ͼ����ַ�ˣ���ʼ������������־��У����������
			dwOffset %= (EXSECT_SIZE-2); //ԭ��dwOffset���Ǵ������׿�ʼ��ģ�����ֻ��ֱ��ģ��Ϳ�����

			if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, pbData, nLen) < 0)
				return TDB_ERR_FLS_RD_FAIL;
			if (bFn < FN_FWDSTAT)
			{
				DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: ComTask FN%03d, %02x-%02x-%02x %02x:%02x append record succes!\r\n", bFn-FN_COMSTAT, pbData[4], pbData[3], pbData[2], pbData[1], pbData[0]));
			}
			else
			{
				DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FwdTask FN%03d, %02x-%02x-%02x %02x:%02x append record succes!\r\n", bFn-FN_FWDSTAT, pbData[4], pbData[3], pbData[2], pbData[1], pbData[0]));
			}
			return 1;
		}
		else
			return TDB_ERR_UNKNOW;
	}

	bTimeLen = ComTaskGetRecOffset(pTaskCtrl)-1; //��g_TdbCtrlȷ��ʱ�䳤��
	//����Pn����öμ�¼����Fn���ƫ����
	bPnInRec = *(pbData+4);
	//���ȼ��
	if (nLen != ComTaskGetRecSize(pTaskCtrl))
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d Record lens is not matched with g_taskCtrl.\r\n", bFn));
		return TDB_ERR_DATALEN;
	}

	//����Pn����öμ�¼����Fn���ƫ����
	//bPnInRec = *(pbData+bTimeLen);
#if MTRPNMAP!=PNUNMAP		
	iPn = SearchPnMap(MTRPNMAP, bPnInRec);
#else
	if (bPnInRec > 0)
		iPn = bPnInRec - 1;
	else
		iPn = 0;
#endif

	if (iPn<0 || iPn>PN_VALID_NUM)
	{
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%d record's PN%d can not Search in PnMap.\r\n", bFn, bPnInRec));
		return TDB_ERR_PNFAIL;
	}

	if (pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_P0)
		iPn = 0; //���ɻ��ն�����ֻ����һ��

	dwOffset = CalcSpacesPerPn(pTaskCtrl);	//����Pnռ�õĿռ�
	dwOffset *= iPn;

	//����ÿ��Pn����������ƫ�ƣ����ǹ̶��ģ������������ݣ������ݶ��ǵ�2λ
	//�˴μ������dwOffset�Ѿ�ָ������ŵ���������
	dwOffset += CalcPerPnMonRecSize(pTaskCtrl)*((GetMonth(pbData, pTaskCtrl)-1)%ComTaskGetMonthNum(pTaskCtrl));

	//��ϸ����Pn���ڵľ������ݵ��ƫ�ƣ���2�����
	//1������������Ϊͳһʱ��洢�ģ��������ߺ��ն���
	//   ��������һ������ݹ���һ��ʱ�꣬����Ҳ����Ϊ�������ݵ㴦��
	if (pTaskCtrl->bIntervU<TIME_UNIT_MONTH || pTaskCtrl->bIntervU== TIME_UNIT_DAYFLG)	//�¶������¾���Ϊ���ߺ��ն�������
		dwOffset += CalcPerPnDayRecSize(pTaskCtrl)*(GetDay(pbData, pTaskCtrl)-1);//��λ������

	wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//�����ļ���������̬����ռ��ƫ�ƣ���λ��Ҫ����������
	if (wSect == 0xffff)
	{
		DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
		return TDB_ERR_UNKNOW;
	}

	//�ã����¾߱����϶��磬���������׵�ַ
	dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;	
	
	//����Ҫ��ʼ���ˣ��������⴦���ȴ�������ߣ���Щ������Ҫ�Ƚ�ʱ���������ţ�ֱ��д
	if (pTaskCtrl->bIntervU == TIME_UNIT_DAY)
	{//���ڷ��������ݣ���ʱ�Ѿ��Ǿ����������ڵ�λ�ã�����ֱ�����������
		//�ȿ�������
        
        	//�Ͼ����ַ�ˣ���ʼ������������־��У����������
    	dwOffset %= (EXSECT_SIZE-2); //ԭ��dwOffset���Ǵ������׿�ʼ��ģ�����ֻ��ֱ��ģ��Ϳ�����

		wFullRecSize = nLen + 1;	//������һ�ʼ�¼�ĳ��ȣ�����У��ͣ�
        if (wFullRecSize >= sizeof(bBufTmp))
            return TDB_ERR_UNKNOW;
		memcpy(bBufTmp, pbData, 3);  //ʱ��
		memcpy(bBufTmp+3, pbData+4, nLen-3); //�������������������
		MakeData(bBufTmp, wFullRecSize); //���У��

		if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wFullRecSize) < 0)
			return TDB_ERR_FLS_WR_FAIL;
	}
	//������
	else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
	{
       	//�Ͼ����ַ�ˣ���ʼ������������־��У����������
    	dwOffset %= (EXSECT_SIZE-2); //ԭ��dwOffset���Ǵ������׿�ʼ��ģ�����ֻ��ֱ��ģ��Ϳ�����
		wFullRecSize = nLen + 1;	//������һ�ʼ�¼�ĳ��ȣ�����У��ͣ�
		if (wFullRecSize >= sizeof(bBufTmp))
			return TDB_ERR_UNKNOW;
		memcpy(bBufTmp, pbData, 2);  //ʱ��
		memcpy(bBufTmp+2, pbData+4, nLen-2); //�����������
		MakeData(bBufTmp, wFullRecSize); //���У��

		if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wFullRecSize) < 0)
			return TDB_ERR_FLS_WR_FAIL;
	}
	else//���ˣ��ֵ�����
	{//�����������ݣ���ʱ��ֻ�������ݿ���
		//��һ������һ������ݶ�������
		WORD wLenPerRec = CalcOneCurveRecSize(pTaskCtrl);	//����һ�ʼ�¼(6��Сʱ)�����ݳ��ȣ�������У�� xxxxx��+ 1
		WORD bTmpOffset = 0;//�������ݵ���������ݿ��ƫ��
		BYTE bIdx=0, bOffset=0;

        if (wLenPerRec >= sizeof(bBufTmp))
            return TDB_ERR_UNKNOW;

		if (BcdToByte(*(pbData+3)) >= 24)	//�ж�Сʱ�Ϸ���
			return TDB_ERR_UNKNOW;
		else
			bIdx = BcdToByte(*(pbData+3)) / (24/CURVE_PER_DAY);	//����6��Сʱһ�ʼ�¼,������㵱ǰСʱ��һ���еĵڼ���

		dwOffset += bIdx*wLenPerRec;	//��λ���ڼ��ʼ�¼

        //�Ͼ����ַ�ˣ���ʼ������������־��У����������
    	dwOffset %= (EXSECT_SIZE-2); //ԭ��dwOffset���Ǵ������׿�ʼ��ģ�����ֻ��ֱ��ģ��Ϳ�����        
		if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wLenPerRec) < 0)
			return TDB_ERR_FLS_RD_FAIL;

		if (!CheckData(bBufTmp, wLenPerRec))	//xxxxx��Ӧ������һ�ݼ�¼
        {
			//return TDB_ERR_DATA_CHKSUM;
            bBufTmp[5] = INVALID_DATA;     //����ֱ�ӷ��أ�������һ������
        }

		//���ˣ��������͸ñȽ�ʱ���ˣ�������Ҳ��Ҫ�Ƚ�,ʱ��ֻ�����-��-�գ�����һ����ƥ�䶼���ɹ�
		if ((memcmp(&bBufTmp[0], pbData, 3) != 0) || (bPnInRec != bBufTmp[3]))
		{
			//��һ����һ����ȫ������ɾ��
			memset(bBufTmp, INVALID_DATA, sizeof(bBufTmp));

			// дʱ���Լ��������
			memcpy(&bBufTmp[0], pbData, 3);	//3�����ȵ�ʱ��

			bBufTmp[3] = *(pbData+4);	//1��������
		}

		bOffset = BcdToByte(*(pbData+3)) % (24/CURVE_PER_DAY);
		//ָ��ǰ��¼ʱ��ľ���λ�ã���������
		bTmpOffset = ComTaskGetRecOffset(pTaskCtrl) + bOffset*ComTaskGetDataLen(pTaskCtrl);
        if ((bTmpOffset+nLen-4) >= sizeof(bBufTmp))
            return TDB_ERR_UNKNOW;
		memcpy(&bBufTmp[bTmpOffset], pbData+5, nLen-4);

		//���У��
		MakeData(bBufTmp, wLenPerRec);

		//д�ص�Flash��
		if (ExFlashWrData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wLenPerRec) < 0)
			return TDB_ERR_FLS_WR_FAIL;
	}

	if (pTaskCtrl->bIntervU <= TIME_UNIT_HOUR)
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%03d, PN%03d %02x-%02x-%02x %02x:%02x append record succes!\r\n", bFn, *(pbData+bTimeLen), pbData[4], pbData[3], pbData[2], pbData[1], pbData[0]));
	else if (pTaskCtrl->bIntervU == TIME_UNIT_MONTH)
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%03d, PN%03d %02x-%02x append record succes!\r\n", bFn, *(pbData+bTimeLen), pbData[1], pbData[0]));
	else
		DTRACE(DB_TASKDB, ("TaskDB::TdbAppendRec: FN%03d, PN%03d %02x-%02x-%02x append record succes!\r\n", bFn, *(pbData+bTimeLen), pbData[2], pbData[1], pbData[0]));
	return 1;
}


//��������������ȡһ�ʼ�¼������Ҳֻ֧��һ�ʱʵĶ�
//������@bFn ��ȡ�Ķ��ඳ���Fn
//      @wPn ��ȡPn������/��ͨ����������/�м����������ŵ�����
//      @tTime ��ȡ��¼��ʱ��
//      @pbBuf ������ؼ�¼��Buffer
//      @bRate ��ȡ�����ܶȣ����������ڽ��ɵ�ÿ�������ߣ�����ʱ����Ϊ0
//ע�⣺����ⲻ���pbBuf�ĳ��ȣ�Ӧ�ó�����Ҫ�Լ���֤
int TdbReadRec(BYTE bFn, WORD wPn, TTime tTime, BYTE *pbBuf)
{
	DWORD dwAddr, dwOffset; //dwOffset�����ַ��ƫ����, dwAddr�����ַ
	int iPn, iRet;//��ʵ�Ĳ������
	const TCommTaskCtrl* pTaskCtrl = ComTaskFnToCtrl(bFn);
	WORD wSect, nDataLen, nLen = 0;//nDataLen������¼�ĳ���, nLen����Flash�еļ�¼���ȣ�����ʱ���PN�Ż���У�飩
//	WORD wNeedSects;

	BYTE bSmplIntervU, bSmplInterV;
	BYTE bTimeLen, bBufTmp[132]; //�������ݻ�����
	BYTE bPnInRec;	//��¼��Ĳ������
	BYTE bTimeInFlash[] = {0x00, 0x00, 0x00, 0x00, 0x00};
	
	if (TdbIsLock())
	{
		iRet =  TDB_ERR_LOCK;
		goto TdbReadRec_err_ret;
	}

	if (g_TdbCtrl.bFn != bFn)	//����⻹û��
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: FN%d tab not opened!\r\n", bFn));
		iRet = TDB_ERR_NOTEXIST;
		goto TdbReadRec_err_ret;
	}

	if (pTaskCtrl == NULL)
	{
		if (bFn >= FN_COMSTAT)
		{
			TimeToFmt15(&tTime, bBufTmp);
			Swap(bBufTmp, 5);
			//����ñʼ�¼����������¼�׵�ַƫ��
			if (bFn < FN_FWDSTAT)
				dwOffset = CalcComTaskOneRecOffSet(bFn-FN_COMSTAT, bBufTmp);
			else if (bFn < FN_MAX)
				dwOffset = CalcFwdTaskOneRecOffSet(bFn-FN_FWDSTAT, bBufTmp);
			else
			{
				iRet = TDB_ERR_NOTEXIST;
				goto TdbReadRec_err_ret;
			}
			wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//�����ļ���������̬����ռ��ƫ�ƣ���λ��Ҫ����������
			if (wSect == 0xffff)
			{
				DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
				return TDB_ERR_UNKNOW;
			}

			//�ã����¾߱����϶��磬���������׵�ַ
			dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;

			//�Ͼ����ַ�ˣ���ʼ������������־��У����������
			dwOffset %= (EXSECT_SIZE-2); //ԭ��dwOffset���Ǵ������׿�ʼ��ģ�����ֻ��ֱ��ģ��Ϳ�����

			/*if (bFn < FN_FWDSTAT)
				nDataLen = GetComTaskPerDataLen(bFn-FN_COMSTAT); //������¼�ĳ���,�����ݲ���,Ҳ����Ҫ���صĳ���
			else //if (bFn < FN_MAX)
				nDataLen = GetFwdTaskPerDataLen(bFn-FN_FWDSTAT); //������¼�ĳ���,�����ݲ���,Ҳ����Ҫ���صĳ���*/

			nDataLen = g_TdbCtrl.dwPerDataLen - 5;
			nLen = nDataLen + 5;//��¼�ĳ���,   ��+5-----��¼ʱ��

			//�Ȱ����ݼ�¼ʱ�������
			if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bTimeInFlash, 5) < 0)
			{
				iRet = TDB_ERR_FLS_RD_FAIL;
				goto TdbReadRec_err_ret;
			}

			if (g_TdbCtrl.bSmplIntervU == TIME_UNIT_MINUTE)
			{
				TTime tmRec;
				DWORD dwRecMins, dwRdMins, dwPastMin;
				Swap(bTimeInFlash, 5);
				Fmt15ToTime(bTimeInFlash, &tmRec);
				dwRecMins = MinutesFrom2000(&tmRec);
				dwRdMins = MinutesFrom2000(&tTime);
				if (dwRecMins < dwRdMins)
					dwPastMin = dwRdMins - dwRecMins;
				else
					dwPastMin = dwRecMins - dwRdMins;
				if (dwPastMin > g_TdbCtrl.bSmplIntervV)
				{
					DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: time is not match!\r\n"));
					iRet = TDB_ERR_SCH_FAIL;
					goto TdbReadRec_err_ret;
				}
			}
			else if (!IsSchTimeMatch(bTimeInFlash, tTime, g_TdbCtrl.bSmplIntervU)) //ʱ��Ƚϲ���
			{
				DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: time is not match!\r\n"));
				iRet = TDB_ERR_SCH_FAIL;
				goto TdbReadRec_err_ret;
			}

			//����ñʼ�¼����������¼�׵�ַƫ�ƣ���ȥ��¼ʱ��
			if (bFn < FN_FWDSTAT)
				dwOffset = CalcComTaskOneRecOffSet(bFn-FN_COMSTAT, bBufTmp) + 5;
			else if (bFn < FN_MAX)
				dwOffset = CalcFwdTaskOneRecOffSet(bFn-FN_FWDSTAT, bBufTmp) + 5;
			else
			{
				iRet = TDB_ERR_NOTEXIST;
				goto TdbReadRec_err_ret;
			}
			wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//�����ļ���������̬����ռ��ƫ�ƣ���λ��Ҫ����������
			if (wSect == 0xffff)
			{
				DTRACE(DB_TASKDB, ("TdbAppendRec: FN%d's malloc table is invalid!\r\n",bFn));
				return TDB_ERR_UNKNOW;
			}

			//�ã����¾߱����϶��磬���������׵�ַ
			dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;

			//�Ͼ����ַ�ˣ���ʼ������������־��У����������
			dwOffset %= (EXSECT_SIZE-2); //ԭ��dwOffset���Ǵ������׿�ʼ��ģ�����ֻ��ֱ��ģ��Ϳ�����

			//�����ݶ�����---������¼ʱ��
			if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, pbBuf, nDataLen) < 0)
			{
				iRet = TDB_ERR_FLS_RD_FAIL;
				goto TdbReadRec_err_ret;
			}

			//DTRACE(DB_TASKDB, ("TdbReadRec: read record sucess!\r\n"));	//xxxxx:���ȥ��,�����ӡ̫��
			return nDataLen;
		}
		else
		{
			iRet = TDB_ERR_NOTEXIST;
			goto TdbReadRec_err_ret;
		}
	}

	nDataLen = ComTaskGetDataLen(pTaskCtrl); //������¼�ĳ���,�����ݲ���,Ҳ����Ҫ���صĳ���
	
	//wNeedSects = CalcFnNeedSects(bFn);

	//����Pn����öμ�¼����Fn���ƫ����
#if MTRPNMAP!=PNUNMAP		
	iPn = SearchPnMap(MTRPNMAP,bPn);
#else
	if (wPn > 0)
		iPn = wPn - 1;
	else
		iPn = 0;
#endif

	if (iPn<0 || iPn>PN_VALID_NUM)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: the require PN%d can not Search in PnMap.\r\n", wPn));
		iRet = TDB_ERR_PNFAIL;
		goto TdbReadRec_err_ret;
	}

	bTimeLen = ComTaskGetRecOffset(pTaskCtrl)-1;	//ʱ�䳤��
	nLen = nDataLen + bTimeLen + 1 + 1;//��¼�ĳ���,1----PN��    ��+1-----У��   ----nLen����ר���ڷ����߶���
	
	if (pTaskCtrl->bIntervU < TIME_UNIT_DAY)	//С���ն�����ж�Ϊ���߶��ᣬΪ���ݷǽ��հ�
	{
		nLen = nDataLen;	//�������ݵ�ʱ���Pn����ÿһ������ݹ���һ������˵��ʼ�¼ֻ�������ݣ�У�鲻����
	}

	if (pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_AC || pTaskCtrl->bPnChar[0]==TASK_PN_TYPE_P0)
		iPn = 0; //���ɻ��ն�����ֻ����һ��

	dwOffset = CalcSpacesPerPn(pTaskCtrl);	//����Pnռ�õĿռ�
	dwOffset *= iPn;

	//����ÿ��Pn����������ƫ�ƣ����ǹ̶��ģ������������ݣ������ݶ��ǵ�2λ
	//�˴μ������dwOffset�Ѿ�ָ������ŵ���������
	dwOffset += CalcPerPnMonRecSize(pTaskCtrl)*((tTime.nMonth-1)%ComTaskGetMonthNum(pTaskCtrl));

	//��ϸ����Pn���ڵľ������ݵ��ƫ�ƣ���2�����
	//1������������Ϊͳһʱ��洢�ģ��������ߺ��ն���
	//   ��������һ������ݹ���һ��ʱ�꣬����Ҳ����Ϊ�������ݵ㴦��
	if (pTaskCtrl->bIntervU<TIME_UNIT_MONTH)	//�¶������¾���Ϊ���ߺ��ն�������
		dwOffset += CalcPerPnDayRecSize(pTaskCtrl)*(tTime.nDay-1);//��λ������
	
	//OK��׼����������ˣ���һ����ʵ�ʵ�ַ�ĸɻ�ȶ��ļ������
	wSect = SchSectInFat(g_TdbCtrl.bFat, dwOffset/(EXSECT_SIZE-2));	//�����ļ���������̬����ռ��ƫ�ƣ���λ��Ҫ����������
	if (wSect == 0xffff)
	{
		DTRACE(DB_TASKDB, ("TdbReadRec: FN%d's malloc table is invalid!\r\n",bFn));
		iRet = TDB_ERR_UNKNOW;
		goto TdbReadRec_err_ret;
	}

	//�ã����¾߱����϶��磬���������׵�ַ
	dwAddr = FADDR_DYN + wSect*EXSECT_SIZE;
	//����Ҫ��ʼ�������ˣ��������ߺͷ��������֣���Ϊ������Ҫ���⴦�����÷����߿���
	if (pTaskCtrl->bIntervU >= TIME_UNIT_DAY)
	{//���������ݴ�ʱ�Ѿ���λ���������µ�ʱ���Ӧ��λ������

        //�����ն��ᡢ�¶���ͳ����ն��ᣬ��ʱ�Ѿ���Ҫ����ļ�¼λ���ˣ�΢������������־��У�鼴��
	    dwOffset %= (EXSECT_SIZE-2); //��Ȼ������ַ�Ѿ����ˣ�ƫ�ƾʹ������׿�ʼ����
        if (nLen >= sizeof(bBufTmp))
        {
            iRet = TDB_ERR_UNKNOW;
            goto TdbReadRec_err_ret;
        }
		//�Ȱ����ݶ�����---����У���
		if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, nLen) < 0)
		{
			iRet = TDB_ERR_FLS_RD_FAIL;
			goto TdbReadRec_err_ret;
		}

		if (!CheckData(bBufTmp, nLen))
		{
			iRet = TDB_ERR_DATA_CHKSUM;
			goto TdbReadRec_err_ret;
		}

		bPnInRec = bBufTmp[bTimeLen];
		memcpy(bTimeInFlash, bBufTmp, bTimeLen);
		if (!IsSchTimeMatch(bTimeInFlash, tTime, pTaskCtrl->bIntervU) || (bPnInRec!=wPn))
		{	//ʱ�겻һ�����߲����㲻һ�£�ֱ�ӻ���Ч
			DTRACE(DB_TASKDB, ("TdbReadRec: time not match!\r\n"));
			iRet = TDB_ERR_SCH_FAIL;
			goto TdbReadRec_err_ret;
		}
		else //���أ���Ҫ��ʱ���PN���ӵ�
			memcpy(pbBuf, &bBufTmp[nLen-nDataLen-1], nDataLen);	//-1������У��

		//DTRACE(DB_TASKDB, ("TdbReadRec: read record sucess!\r\n"));	//xxxxx:���ȥ��,�����ӡ̫��
		return nDataLen;
	}
	else //if (pTaskCtrl->bIntervU == TIME_UNIT_HOUR)//���ˣ���������
	{//�����������ݣ���ʱ��ֻ�������ݿ���
		//��һ������һ������ݶ�������
 		WORD wLenPerRec = CalcOneCurveRecSize(pTaskCtrl);	//һ������ݳ��ȣ�����У�� xxxxx��+ 1
 		WORD bTmpOffset = 0;//�������ݵ���������ݿ��ƫ��
		BYTE bIdx = 0, bOffset=0;
 		if (wLenPerRec >= sizeof(bBufTmp))
         {
             iRet = TDB_ERR_UNKNOW;
             goto TdbReadRec_err_ret;
        }

		if (tTime.nHour >= 24)	//�ж�Сʱ�Ϸ���
			return TDB_ERR_UNKNOW;
		else
			bIdx = tTime.nHour / (24/CURVE_PER_DAY);	//����6��Сʱһ�ʼ�¼,������㵱ǰСʱ��һ���еĵڼ���

		dwOffset += bIdx*wLenPerRec;	//��λ���ڼ��ʼ�¼

	    //�������ݻ�ͣ���տ������ϣ���Ϊ��Ҫ�ж�ʱ�꣬�ݲ���������¼��ƫ��
	    dwOffset %= (EXSECT_SIZE-2); //��Ȼ������ַ�Ѿ����ˣ�ƫ�ƾʹ������׿�ʼ����
		if (ExFlashRdData(dwAddr+dwOffset, g_TdbCtrl.bFat, bBufTmp, wLenPerRec) < 0)
		{
			iRet = TDB_ERR_FLS_RD_FAIL;
			goto TdbReadRec_err_ret;
		}

		if (!CheckData(bBufTmp, wLenPerRec))
		{
			iRet = TDB_ERR_DATA_CHKSUM;
			goto TdbReadRec_err_ret;
		}

		bPnInRec = bBufTmp[bTimeLen];
		memcpy(bTimeInFlash, bBufTmp, 3);	//���ߴ����3�����ȵ�ʱ��

		//���ˣ��������͸ñȽ�ʱ���ˣ�������Ҳ��Ҫ�Ƚ�,ʱ��ֻ�����-��-�գ�����һ����ƥ�䶼���ɹ�
		if (!IsSchTimeMatch(bTimeInFlash, tTime, pTaskCtrl->bIntervU) || (bPnInRec!=wPn)) 
		{//ʱ�겻һ����ֱ�ӻ���Ч
			DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: time is not match!\r\n"));
			iRet = TDB_ERR_SCH_FAIL;
			goto TdbReadRec_err_ret;
		}
		else//���أ���Ҫ��ʱ���PN���ӵ�
		{
			bOffset = tTime.nHour % (24/CURVE_PER_DAY);		//ָ��ǰ��¼ʱ��ľ���λ�ã���������
			bTmpOffset = ComTaskGetRecOffset(pTaskCtrl) + bOffset*nDataLen;//�ҵ�����ƫ��
			//bTmpOffset = ComTaskGetRecOffset(pTaskCtrl) + bOffset*ComTaskGetDataLen(pTaskCtrl);

			DTRACE(DB_TASKDB, ("TaskDB::TdbReadRec: read record sucess!\r\n"));
			memcpy(pbBuf, &bBufTmp[bTmpOffset], nDataLen);
			return nDataLen;
		}
	}

	//DTRACE(DB_TASKDB, ("TdbReadRec: can not search record with these condition!\r\n"));
	iRet = TDB_ERR_SCH_FAIL;

TdbReadRec_err_ret:
	memset(pbBuf, INVALID_DATA, nDataLen);
	return iRet;
}

