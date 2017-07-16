/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TaskDB.h
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
************************************************************************************************************/
#pragma once
#ifndef _TASKDB_H
#define _TASKDB_H

#include "TypeDef.h"
#include "SysArch.h"
#include "Comstruct.h"
#include "TaskStruct.h"
#include "FlashMgr.h"

//�����Ĵ�������
#define TDB_ERR_OK			0	//�޴���
#define TDB_ERR_UNKNOW		-1	//δ֪����
#define TDB_ERR_EXIST		-2	//Fn�Ѿ�������ռ���
#define TDB_ERR_NOTEXIST	-3	//Fn��û�з���ռ�
#define TDB_ERR_FULL		-4	//Flash�ռ䲻��
#define TDB_ERR_LOCK		-5	//���������
#define TDB_ERR_DATALEN		-6	//���ݳ��ȴ���
#define TDB_ERR_PNFAIL		-7	//Pn��Ч��û��ӳ��
#define TDB_ERR_FLS_RD_FAIL	-8	//Flash��ʧ��
#define TDB_ERR_FLS_WR_FAIL	-9	//Flashдʧ��
#define TDB_ERR_SCH_FAIL	-10	//������¼ʧ��
#define TDB_ERR_SECT_CHKSUM	-11	//����У��ʧ��
#define TDB_ERR_WRONGOBJ	-12	//����������󣬹رմ�Fn�ռ䣬���¼����ʹ��
#define TDB_ERR_RD_PARA		-13	//��ϵͳ���������
#define TDB_ERR_FN          -14 //���µ�FN���ڷ�Χ��
#define TDB_ERR_MACT_CHKSUM -15 //�ļ������У��ʧ��
#define TDB_ERR_FN_INVALID	-16	//Fn��Ч����Fnû����TCommTaskCtrl g_taskCtrl[]������
#define TDB_ERR_DATA_CHKSUM	-17	//����У�����

#define INVALID_FN		0x00

typedef struct
{
	TSem	semTdb;	     	//���������ź���
	bool	fIsLocked;		//������Ƿ�����
	BYTE	bFn;			//����ʹ���������ļ��ţ���ʱ��ֵ���ر�ʱ���
	BYTE	bFat[FAT_SIZE]; 		//��һ��FN�ı�ʱ��Ҫ�Ѹ�FN���ļ�������ȶ�����
	BYTE	bGlobalFat[FAT_SIZE]; //ȫ�ֵ�FAT
	DWORD	dwPerDataLen;	//ÿ����ͨ���м����񳤶�
	BYTE	bSmplIntervU;	//�������ڵ�λ
	BYTE	bSmplIntervV;	//��������
}TTdbCtrl;


extern TTdbCtrl	g_TdbCtrl;


/******************************************************************************
 *	�˲���Ϊ������ṩ��Ӧ�ó���Ľӿں���
*******************************************************************************/
//������������ʼ��
//������NONE
//���أ��ɹ�����true
bool TdbInit();

//������ΪbFn����Flash�ռ�
//������@bFn ���ඳ���Fn�����õ�ʱ����뱣֤Fn��Ч������FN��TCommTaskCtrl g_taskCtrl[]��������
//���أ������ȷ���뵽�ռ䣬��������ռ���׵�ַ��һ������0��������
//      �������ʧ�ܣ��᷵��С��0�Ĵ�����
bool TdbMalloc(BYTE bFn, BYTE* pbFat, BYTE bOrder);

//�������ͷ�bFn�����Flash�ռ�
//������@bFn ���ඳ���Fn
//���أ������ȷ�ͷſռ䣬����1�����򷵻���Ӧ�Ĵ�����
bool TdbFree(BYTE bFn, BYTE* pbFat);

//�������ú��������ж�ϵͳ�Ƿ���ΪbFn����ռ䣬��Ԥ����Ӧ���ļ������
//������@bFn ���ඳ���Fn
//���أ�����Ѿ�����ռ䣬����1�����򷵻���Ӧ�Ĵ�����
int TdbOpenTable (BYTE bFn);

//���������Ԥ�����ļ������
//������@bFn ���ඳ���Fn
//���أ������ȷ����ˣ�����1�����򷵻���Ӧ�Ĵ�����
int TdbCloseTable(BYTE bFn);

//��������������ȡһ�ʼ�¼������Ҳֻ֧��һ�ʱʵĶ�
//������@bFn ��ȡ�Ķ��ඳ���Fn
//      @bPn ��ȡPn�����������
//      @tTime ��ȡ��¼��ʱ��
//      @pbBuf ������ؼ�¼��Buffer
//      @bRate ��ȡ�����ܶȣ����������ڽ��ɵ�ÿ�������ߣ�����ʱ����Ϊ0
//ע�⣺����ⲻ���pbBuf�ĳ��ȣ�Ӧ�ó�����Ҫ�Լ���֤
int TdbReadRec (BYTE bFn, WORD wPn, TTime tTime, BYTE *pbBuf);


//����������������һ�ʼ�¼���·������������Ӽ�¼ֻ֧��һ�ʱʼ�
//������@bFn ���ඳ���Fn
//      @pbData��Ҫ��ӵ������ļ�¼
//      @nLen��¼����
//���أ������ȷ����ˣ�����1�����򷵻���Ӧ�Ĵ�����
int TdbAppendRec(BYTE bFn, BYTE *pbData, WORD nLen);
/******************************************************************************
 *	�ṩ��Ӧ�ó���Ľӿں������嵽��Ϊֹ������Ϊ������ڲ�ʹ��
*******************************************************************************/

//������ȡ������ͷ�������������
//������@*pbData �ܵ��ﴫ���������ݰ�
//		@*pTaskCtrl ���ƽṹ
//���أ�����pbData���Ӧ��pTaskCtrl��ʽ������������
BYTE GetMonth(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl);

//������ȡ������ͷ�������������
//������@*pbData �ܵ��ﴫ���������ݰ�
//		@*pTaskCtrl ���ƽṹ
//���أ�����pbData���Ӧ��pTaskCtrl��ʽ������������
BYTE GetDay(BYTE *pbData, const TCommTaskCtrl *pTaskCtrl);

//�����������������
//������NONE
//���أ�NONE
void TdbLock();

//����������������
//������NONE
//���أ�NONE
void TdbUnLock();

//������������Ƿ�����
//������NONE
//���أ���������true
bool TdbIsLock();

void TdbWaitSemaphore();
void TdbSignalSemaphore();

//--------------------------------------------------------------------------------------------------------
// ����Calc*********�ĸ�����������û�м��pTaskCtrl�ĺϷ��ԣ���Ҫ���������б�֤pTaskCtrl�ǺϷ���
//���������㵥��Pn�����bFn������Ҫ���ٿռ䣬��λ��Byte����TCommTaskCtrlΪ����
//������@pTaskCtrl bFn��������ƽṹ
//���أ���Ҫʹ�õĿռ�
DWORD CalcPerPnDayRecSize(const TCommTaskCtrl *pTaskCtrl);

//���������㵥��Pn�����µ�bFn������Ҫ���ٿռ䣬��λ��Byte����TCommTaskCtrlΪ����
//������@pTaskCtrl bFn��������ƽṹ
//���أ���Ҫʹ�õĿռ�
DWORD CalcPerPnMonRecSize(const TCommTaskCtrl *pTaskCtrl);

//���������㵥��Pn��bFn������Ҫ���ٿռ䣬��λ��Byte����TCommTaskCtrlΪ����
//������@pTaskCtrl bFn��������ƽṹ
//���أ���Ҫʹ�õĿռ��ܺ�
DWORD CalcSpacesPerPn(const TCommTaskCtrl *pTaskCtrl);

//����������bFn��Ҫ���������Ŀռ䣬��λ��section
//������@bFn ���ඳ���Fn
//���أ���Ҫ������
WORD CalcFnNeedSects(BYTE bFn);

//����������bFn��Ҫ���ٿռ䣬��λ��BYTE
//������@bFn ���ඳ���Fn
//���أ���Ҫ������
DWORD CalcFnNeedSpaces(BYTE bFn);

//���������bTime��tTime�Ƿ�ƥ��
//������@bTime BYTE��ʽ��ʱ��
//		@tTime TTime��ʽ��ʱ��
//		@bIntervU ��������
//���أ����ƥ�䷵��true�����򷵻�false
bool IsSchTimeMatch(BYTE *bTime, TTime tTime, BYTE bIntervU);

//��������[wByteOffset].bBitOffset��ʼ����pbFat�ļ�������б�־nCount��������
//������@pbFat �ļ������
//		@wByteOffset �ļ�������ֽ��ϵ�ƫ��
//		@bBitOffset �ļ�������ֽ���λ��ƫ��
//		@nCount ��־�ĸ���
//���أ�NONE
bool SectMarkUsed(BYTE* pbFat, WORD wByte, BYTE bBit);

#endif	//_TASKDB_H
