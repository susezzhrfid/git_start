/*********************************************************************************************************
* Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�FileMgr.h
* ժ    Ҫ�����ļ���Ҫ���ڶ��ļ�����ӿڽ��з�װ
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2010-12-28
*********************************************************************************************************/
#ifndef FILEMGR_H
#define FILEMGR_H
#include "SysCfg.h"
#include "TypeDef.h"
#include "FaStruct.h"

//�ļ��Ŷ���:[0~0x80)Ƭ��FLASH
#define FILE_PNMAP		0		//�����㶯̬ӳ����ձ��ļ�
#define FILE_KEEP_PARA	1		//�ն˱��ֲ���
#define FILE_BN1_PARA	2		//BANK1��չ����
#define FILE_BN10_PARA	3		//BANK10��չ����
#define FILE_BN24_PARA	4		//BANK24��չ����
#define FILE_TERM_PARA	5		//�ն˲���
#define FILE_PN_PARA	6		//���������
#define FILE_BN11_DATA	7		//BANK11�м�����
#define FILE_BN25_PARA	8		//У׼����

//�ļ��Ŷ���:[0x80~0xff]Ƭ��FLASH
#define FILE_PWROFF_TMP		0x80	//�������
#define FILE_EXT_TERM_PARA	0x81	//�ն���չ����
#define FILE_BATTASK0       0x82    //�����֤����
#define FILE_BATTASK1       0x83    //��ʱ����
#define FILE_BATTASK_STATUS 0x84    //���������
#define FILE_SCHMTR_STATUS 0x85    //�ѱ�״̬��Ϣ�洢

#define FILE_NULL		0xffff	//���ļ�

typedef struct{
	int 	nSect;		//��ʼ��������
	int 	nPage;		//��ʼ��ҳ�ţ�-1��ʾʹ������������ҳÿ��256���ֽ�
	WORD 	wSectNum;	//ռ���������������nPage>=0,���ʾռ��ҳ���������nPage<0,���ʾռ����������
}TFileCfg;	//�ļ����ýṹ

#ifdef SYS_WIN
#define EXFLASH_ADDR	0
#define EXFLASH_SIZE    0x2000000   //32M
#endif

#define INSECT_SIZE		1024	//2048	//Ƭ��FLASH������С
#define EXSECT_SIZE		4096	//Ƭ��FLASH������С

#define PAGE_SIZE	256		//��������С
#define ALRSECT_SIZE	270		//�澯�洢��Ԫ 1��CS��+2��Pn��+ 4��ID�� + 1���澯״̬��+ 6��ʱ�䣩 +256

const TFileCfg* File2Cfg(WORD wFile);
bool IsPageFile(WORD wFile);	//�Ƿ��ǰ�ҳ������ļ�
int GetFileOffset(WORD wFile, DWORD dwSect);
int ReadFileSect(WORD wFile, DWORD dwSect, BYTE* pbData, int* piFileOff);	//��ȡ�ļ����ڵĵ�dwSect������
bool readfile(WORD wFile, DWORD dwOffset, BYTE* pbData, int iLen);
bool writefile(WORD wFile, DWORD dwSect, BYTE* pbData);
BYTE* GetFileDirAddr(WORD wFile, DWORD dwOffset);	//ȡ���ļ���ֱ�Ӷ���ַ

DWORD GetSectSize(WORD wFile);	//ȡ���ļ������������С
DWORD GetSectUsedSize(WORD wFile);	//ȡ���ļ�ʵ��ʹ�õ�������С
WORD GetFileSectNum(WORD wFile);	//ȡ���ļ��������������
bool CheckFile(WORD wFile, BYTE* pbFile, int iFileOff);		//�����ļ�ȫ������ĳ�������ĺϷ���
void MakeFile(WORD wFile, BYTE* pbFile);	//���ļ����������ļ���־��У��

bool ReadPwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen);
bool WritePwrOffTmp(TPowerOffTmp *pPwrOffTmp, WORD nLen);



#endif //FILEMGR_H
