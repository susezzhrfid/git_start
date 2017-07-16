/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FlashMgr.h
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
************************************************************************************************************/
#pragma once
#ifndef _FLASHMGR_H
#define _FLASHMGR_H

#include "TypeDef.h"
#include "SysArch.h"
#include "MtrStruct.h"
#include "FileMgr.h"
#include "FlashIf.h"
#include "GbPro.h"

extern TSem	g_semExFlashBuf;//�ⲿFlash��д�ź���
extern BYTE	*g_ExFlashBuf;//�ⲿFlash��д������
extern TSem    m_semTaskCfg;
//extern BYTE g_ExFlashBuf[EXSECT_SIZE];

#define INSECT_NUM	50	//32
//#define EXFLASH_SIZE	(4*1024*1024)	//4M

#define FLASH_SECT_NUM (EXFLASH_SIZE/EXSECT_SIZE)

//�¼�
#define MAX_ALR_LEN	(72+1)	//37���¼����ȣ�1��ָ1���ֽڵ�У��

//��������������Ƭ��Flashһ����С�Ŀռ� 256K == 64������
#define UDP_STR_SECT_OFFSET	0

//�ļ��������ʼλ�õ�����ƫ��
#define MT_STR_SECT_OFFSET		0


// Flash�ռ��м����ݱ�����    ���ݶ���   260K
#define TD_STR_SECT_OFFSET	0

// Flash�ռ��¼���¼������   ���ݽṹ���� 19072Byte
// ��Ҫ�¼�
//��Ҫ�¼�������ʼλ�õ�����ƫ��
#define VA_STR_SECT_OFFSET		0	//��һ���м����ݿ�ռ��������������


//��ͨ�¼�����ʼλ�õ�����ƫ��
#define CA_STR_SECT_OFFSET		VITALALR_REC_SIZE%EXSECT_SIZE
// ��ͨ�¼�,��Ϊ�м����������������õģ�������Ҫ�¼�����ʼ��Ҳһ����������ͷ������������ƫ��

#define ALRREC_SIZE		(EXSECT_SIZE*33)	
#define EVTREC_SIZE		(EXSECT_SIZE*5)

#define EXTPARA_SIZE    (EXSECT_SIZE*64)    //�ⲿ�����ռ�133922Bytes, ռ��33������, �ֿ�64��������Ԥ��31�������ռ䡣

//�澯����
#define VIT_ALR		0x01
#define COM_ALR		0x02

#define EXC_DATA_OFFSET     4		//�澯����ƫ��(2 �ܱ��� + 2 ��ǰ����)
#define EXC_MAX_ALR_NUM     500		//�澯��¼������
#define EXC_MAX_EVENT_NUM   10		//�¼���¼������
#define ERR_MAX_ALR_NUM     10		//�����ϱ�ʧ�ܸ澯��¼������

//�ļ���������
#define FN_DAYSTAT			1						//��������ʼFN��
#define FN_MONSTAT			11						//��������ʼFN��
#define FN_CURSTAT			21						//����������ʼFN��
#define FN_COMSTAT			30						//��ͨ����������ʼFN��
#define FN_FWDSTAT			94						//�м�����������ʼFN��
#define FN_MAX				158						//����FN��
#define FAT_NUM				158						//�ļ������ĸ���

#define FAT_SIZE 			1024						//ÿ���ļ��ķ�����С
#define FAT_FLG_SIZE 		1021						//����������ڴ�С
#define FAT_PER_SECT 	(EXSECT_SIZE/FAT_SIZE)		//ÿ���������ļ���������

#define FAT_TOTAL_SIZE		((FN_MAX+62)*FAT_SIZE)//0x37000		//�ܵ��ļ�������С
								//�ļ�������ܳ���Ϊ0x36C00����ռ54��������0xC00���ֽڣ�Ϊ�����55������	//216*1024 216��Fn��ÿ��1024���ֽ�-----ͬ��
								//�ļ������Ҫ��������������Ϊ��һ�鲻ʵ������У�飬����ʵ���ļ������У��

#define UPDINFO_LEN			28  //!!!!!!! 2014/09/12 ��bootloader��Ӧ  

#define FADDR_UPDINFO		EXFLASH_ADDR
#define FADDR_UPDFW			(EXFLASH_ADDR+UPDINFO_LEN)	//24���ֽڵ��ļ���Ϣ�ͱ�־λ
#define FADDR_F1_INFO       (EXFLASH_ADDR+EXSECT_SIZE*127)//�ļ�������F1����ʾ����Ϣ�Լ�����Զ������Ϣ
#define FADDR_FAT			(EXFLASH_ADDR+EXSECT_SIZE*128)	//�򻯺�ļ��㹫ʽ
#define FADDR_PNTMP			(FADDR_FAT+FAT_TOTAL_SIZE)		//(FADDR_UPDFW+EXSECT_SIZE*64)	//�򻯺�ļ��㹫ʽ
#define FADDR_ALRREC		(FADDR_PNTMP+EXSECT_SIZE*PN_NUM*8)    //�澯��¼
#define FADDR_EVTREC		(FADDR_ALRREC+ALRREC_SIZE)      //�¼���¼
#define FADDR_EXTYKPARA		(FADDR_EVTREC+EVTREC_SIZE)      //ң������բ����
#define FADDR_RPTFAILREC	(FADDR_EXTYKPARA+EXSECT_SIZE)   //�����ϱ�ʧ�ܼ�¼
#define FADDR_EXTPARA		(FADDR_RPTFAILREC+EXSECT_SIZE)
#define FADDR_DYN			(FADDR_EXTPARA+EXTPARA_SIZE)

#define STA_SECT_NUM	(FADDR_DYN/EXSECT_SIZE)
#define DYN_SECT_NUM	((EXFLASH_ADDR+EXFLASH_SIZE-FADDR_DYN)/EXSECT_SIZE) //7741

#define DYN_SECT_START	(FADDR_DYN/EXSECT_SIZE)//451


#define EXFLASH_PARA_OFFSET	    FADDR_EXTPARA		//Ƭ��Flash��չ������ʼƫ�Ƶ�ַ

//----------------------------------------------------------------------------------
// ���º���ΪFlashMgrΪӦ���ṩ�Ľӿ�

bool UpdFat();

//�������ϵ�ʱɨ��Flash����ȡȫ���ļ������g_TdbCtrl.bGlobalFat��Ҳ����ɨ���ļ������
//������NONE
//���أ������ȷ������true,���򷵻�false
bool FlashInit();

//��������g_ExFlashBuf����ΪĬ��ֵ
//������NONE
//���أ�NONE
void MakeDefaultSect(BYTE *pbBuf, WORD wSize);

//���������ⲿFlash��ȡ��Ҫ�����ĳ�������
//������@*pbBuf ���������ļ�������buf����ȫ�����pbBuf������ʹ��g_ExFlashBuf
//		@nLen ���ζ�ȡ�ĳ���
//		@dwStartOffset �����ļ�ͷdwStartOffsetƫ�Ƶ�λ�ÿ�ʼ��ȡ
//���أ������ȷ��ȡ�����������򷵻�true
//      ���򷵻�flash
//ע�⣺��������鳤�ȣ�Ӧ�ó�����Ҫ�Լ���֤���յĻ������㹻��
bool ReadUpdateProg(BYTE* pbBuf, DWORD nLen, DWORD dwStartOffset);

//�����������õĳ���д���ⲿFlash��
//������@*pbBuf д���ⲿFlash�ĳ����buffet
//		@nLen ����дFlash�ĳ���
//		@dwStartOffset �����ļ�ͷdwStartOffsetƫ�Ƶ�λ�ÿ�ʼдFlash
//���أ������ȷд�������򷵻�true
//      ���򷵻�false
//ע�⣺��������鳤�ȣ�Ӧ�ó�����Ҫ�Լ���֤���յĻ������㹻��
bool WriteUpdateProg(BYTE* pbBuf, DWORD nLen, DWORD dwStartOffset);

//������д���������У�鼰���������־
//������@wCrc16 crcУ��ֵ
//���أ����У����ȷ����true�����򷵻�falsh
//bool WriteUpdProgCrc(DWORD dwUpdLen, WORD wCrc16);
bool WriteUpdProgCrc(char *szPfile, DWORD dwUpdLen, WORD wCrc16);

//���������������bPn���м�������
//������@wPn��Ҫ��ȡ���ݵĲ�����ţ�F10�����õĲ������
//���أ������ȷ����������򷵻�true
//      ���򷵻�false
bool ClrPnTmpData(BYTE bPn);

//��������ȡ������wPn��bFileNum�м�������
//������@bPn��Ҫ��ȡ���ݵĲ�����ţ�F10�����õĲ������
//	    @*pPnTmp �������м����ݻ���
//		@wNum PnTmp�ĸ���
//���أ������ȷ��ȡ�����������򷵻�true
//      ���򷵻�false
//ע�⣺��������鳤�ȣ�Ӧ�ó�����Ҫ�Լ���֤���յĻ������㹻��
bool ReadPnTmp(BYTE bPn, const TPnTmp* pPnTmp, WORD wNum);

//���������������bPn��bFileNum�м�������
//������@bPn��Ҫ��ȡ���ݵĲ�����ţ�F10�����õĲ������
//	    @*pPnTmp �������м����ݻ���
//		@wNum PnTmp�ĸ���
//���أ������ȷ�����������򷵻�true
//      ���򷵻�false
bool WritePnTmp(BYTE bPn, const TPnTmp* pPnTmp, WORD wNum);

//��������ȡbAlrType�����¼����¼�ָ��
//������@bAlrType�¼����ͣ���ͨ�¼�or��Ҫ�¼�
//���أ������ȷ��ȡ�������¼���¼���������򷵻���Ӧ�Ĵ����루�������Ǹ�����
int GetAlrRecPtr(BYTE bAlrType);

//����������bAlrType���͵��¼�pbBuf
//������@bAlrType�¼����ͣ���ͨ�¼�or��Ҫ�¼�
//���أ������ȷ��ȡ�������¼���¼���������򷵻���Ӧ�Ĵ����루�������Ǹ�����
int GetAlrRecNum(BYTE bAlrType);

//��������ȡbAlrType���͵��¼�pbBuf
//������@bAlrType�¼����ͣ���ͨ�¼�or��Ҫ�¼�
//		@bAlrPtr ��Ҫ��ȡ�ļ�¼ָ��0~255
//	    @*pbBuf�������ݻ�����
//���أ������ȷ��ȡ�������򷵻����ݵĳ���
//      ���򷵻���Ӧ�Ĵ����루����С����ĸ�����
//int ReadAlrRec(DWORD dwId, BYTE bPn, BYTE* pbBuf, WORD* wRdNum, WORD wTotal, WORD* wAlrLen);
int ReadOneAlr(DWORD dwAddr, BYTE* pbBuf, WORD wRdNum);

//����������bAlrType���͵��¼�pbBuf
//������@bAlrType�¼����ͣ���ͨ�¼�or��Ҫ�¼�
//	    @*pbBuf�������ݻ�����
//		@nLen �������ݳ���
//���أ������ȷ�����������򷵻ر������ݵĳ���
//      ���򷵻���Ӧ�Ĵ����루����С����ĸ�����
int WriteAlrRec(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);

//������������ܱ�����բ�����ò�������ע��pbBufΪTYkCtrl��ָ�룬
//		�ڶ�д��ʱ���ѵ����ṹ���С�ֽ�TYkCtrl��д
//������@bPn������
//		@*pbBufΪ���������
//���أ������ȷ���������ݣ��򷵻ر������ݵĳ���
//		���򷵻���Ӧ�Ĵ����루����С����ĸ�����
int ExFlashReadPnCtrlPara(WORD wPn, TYkCtrl *tYkCtrl);

//������������ܱ�����բ�����ò�������ע��pbBufΪTYkCtrl��ָ�룬
//		�ڶ�д��ʱ���ѵ����ṹ���С�ֽ�TYkCtrl��д
//������@bPn������
//		@*pbBufΪ���������
//		@nLenΪ����ĳ���
//���أ������ȷ���������ݣ��򷵻ر������ݵĳ���
//		���򷵻���Ӧ�Ĵ����루����С����ĸ�����
int ExFlashWritePnCtrlPara(WORD wPn, TYkCtrl *tYkCtrl, WORD nLen);

//���ⲿʹ�õĽӿڵ��˽���
//----------------------------------------------------------------------------------

//���������g_ExFlashBuf
//������NONE
//���أ������ȷ������true,���򷵻�false
bool CheckSect(BYTE *pbBuf, WORD wSize);

//��������g_ExFlashBuf���ļ���־�Լ���У��
//������NONE
//���أ�NONE
void MakeSect(BYTE *pbBuf, WORD wSize);

//���������pbMacTab�Ƿ���ȷ
//������@pbMacTab ��Ҫ�����ļ������
//���أ������ȷ������true,���򷵻�false
bool CheckFat(BYTE *pbMacTab);

//��������pbMacTab��У��
//������@pbMacTab ��Ҫ����У����ļ������
//���أ�NONE
void MakeFat(BYTE *pbMacTab);

//����������¼���¼pbRec�Ƿ���ȷ
//������@pbRec ��Ҫ����У��ļ�¼
//���أ������ȷ������true,���򷵻�false
bool CheckAlrRec(BYTE *pbRec, DWORD dwLen);

//���������¼���¼pbRec��У��
//������@pbRec��Ҫ����У����¼���¼
//���أ�NONE
void MakeAlrRec(BYTE *pbRec, DWORD dwLen);

//�����������ʷ��¼pbRec�Ƿ���ȷ
//������@pbRec ��Ҫ����У��ļ�¼
//		@nLen ��ʷ��¼�ĳ���
//���أ������ȷ������true,���򷵻�false
bool CheckData(BYTE *pbRec, WORD nLen);

//����������ʷ��¼pbRec��У��
//������@pbRec��Ҫ����У�����ʷ��¼
//		@nLen ��ʷ��¼�ĳ���
//���أ�NONE
void MakeData(BYTE *pbRec, WORD nLen);


/*****************************************
 *  ������У���Flash��д�ӿ�
******************************************/

//��������Flash
//������@dwAddr д��ĵ�ַ
//		@*pbFat ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashRdData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//��������Flash����У��
//������@dwAddr д��ĵ�ַ
//		@*pbFat ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashRdDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//������дFlash
//������@dwAddr д��ĵ�ַ
//		@*pbFat ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashWrData(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//������дFlash����У��
//������@dwAddr д��ĵ�ַ
//		@*pbMacTab ��Ҫд��Fn���ļ���������û�У�����NULL���ò���ΪNULL��ʱ������д����
//	    @*pbBuf ��Ҫд��Flash���������ݣ���pbBuf������ʹ��g_ExFlashBuf���ӿ�
//		@nLen д��Flash�����ݳ���
//���أ������ȷд�룬����true,���򷵻�false
//ע�⣺������Ϊ�������󣬺��������Ҫ������ͱ�־
int ExFlashWrDataNoChk(DWORD dwAddr, BYTE *pbFat, BYTE* pbBuf, WORD wLen);

//   Flash����ӿ����
/************************************************************/

//��������TPnTmp���������ó�0������ʱʹ��
//������@*pPnTmp�������м����ݻ���
//		@wNum PnTmp�ĸ���
//���أ���
void ClrPnTmp(const TPnTmp* pPnTmp, WORD wNum);

//��������ȡbFn���ļ������pbFat
//������@bFn ��Ҫ��ȡ���ļ�������
//	    @*pbFat �������ļ�������Ż���
//���أ������ȷ����true�����򷵻�false
int ReadFat(BYTE bFn, BYTE *pbFat);

//��������bFn���ļ������pbFatд�뵽Flash��Ӧλ��
//������@bFn ��Ҫд����ļ�������
//	    @*pbFat �ļ��������
//���أ������ȷ����true�����򷵻�false
bool WriteFat(BYTE bFn, BYTE *pbFat);

//����������ļ�������Ƿ���Ч
//������@*pbFat �ļ������
//���أ�����������Ч����true����Ч����false
bool FatIsInValid(BYTE *pbFat);

//���������bFn���ļ��������Ϣ
//������@bFn ��Ҫ������ļ�������
//���أ�����ɹ�����򷵻�true,���򷵻�false
bool ClrFat(BYTE bFn, BYTE* pbFat);

//��������������ļ��������Ϣ
//������NONE
//���أ�����ɹ�����򷵻�true,���򷵻�false
bool ResetAllFat();

//����������ΪMakeDataInSect���񣬼��㳤��ΪwLen������ȫ��ΪbDefaultֵ�Ļ���ռ��У��ֵ
//������@wLen ���ݳ���---ָ����Ч���ݳ��ȣ�������У��
//		@bDefault Ĭ��ֵ
//���أ�У��ֵ��У��ֵΪwLen��bDefault�ܺ�ģ��0xff��ֵȡ��
BYTE CalcChkSum(WORD wLen, BYTE bDefault);

//��������g_ExFlashBuf��wStart��ʼ��ÿwPeriod�������У�飬У��ֵ�����wPeriodλ��
//������@wStart ��ʼλ��
//		@wPeriod ÿһ�εĳ���
//���أ��������һ���������ں�ʣ��Ŀռ�
WORD MakeDataInSect(WORD wStart, WORD wPeriod, BYTE *pbBuf, WORD wSize);

//���������pbFat��ռ�õ�Flash�ռ估��Ϣ,ֻ�������Flash��������ļ������
//������@bFn  ��Ҫ������ļ������������bFn
//		@pbFat ��Ҫ������ļ��������Ϣ
//���أ������ȷ������true,���򷵻�false
bool CleanFlash(BYTE bFn, BYTE *pbFat);

//�������ҵ��ļ������pbFat�׸���̬����ռ������ƫ��
//������@pbFat �ļ������
//		@nSect ��Ҫ���ҵĵ�nSect����Ч����
//���أ����ص�1����̬����������ƫ�ƺ�,��1��������ƫ��Ϊ0
WORD SchSectInFat(BYTE *pbFat, WORD nSect);

//����������wSectOffsetƫ�Ƶ�������ַ
//������@wSectOffset ����ƫ����,��0��ʼ
//���أ������ȷ���㣬����Flash��ַ�����򷵻�-1
DWORD SectionToPhysicalAddr(WORD wSectOffset);

//��������ֱ��1����������ݽ���У��
WORD CheckPnData(const TPnTmp* pPnTmp, WORD wNum);

//������ȡ��ֱ��1�����ݵ�����������ݵĴ�С
WORD GetPnDataSize(const TPnTmp* pPnTmp, WORD wNum);

//������ֱ��1�����ݵ������������
bool LoadPnData(WORD wPn, const TPnTmp* pPnTmp, WORD wNum);

#endif	//_FLASHMGR_H