/*********************************************************************************************************
* Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�Flash.h
* ժ    Ҫ�����ļ���Ҫ���ڶ�FLASH�ӿڽ��з�װ
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2010-12-28
*********************************************************************************************************/

#define IN_PARACFG_ADDR 0xF2900     //Ƭ��Flash���������ļ���ַ

BYTE* ParaAddrConv(DWORD dwLogical);
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize); //�ڲ�FLASH�޶�������д
int ExFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize);	//�ⲿFLASH�޶�������д
int ExFlashEraseBlock(DWORD dwBlockAddr, BYTE bBlockType);
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//�ڲ�FLASH���޶������Ķ�
int ExFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//�ⲿFLASH���޶������Ķ�
bool ExFlashEraseSect(DWORD dwAddr);
bool InFlashEraseSect(DWORD dwAddr);
void ExFlashEraseChip();
bool InFlashFormat();
int GetFileLen(char* pszPathName);
BYTE GetBootVer(char *pcBuf, BYTE bBufSize);
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen);

