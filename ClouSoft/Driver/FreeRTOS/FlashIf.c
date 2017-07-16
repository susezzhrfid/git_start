/*********************************************************************************************************
* Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�Flash.c
* ժ    Ҫ�����ļ���Ҫ���ڶ�FLASH�ӿڽ��з�װ
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2010-12-28
*********************************************************************************************************/
#include <stdio.h>
//#include <errno.h>
#include "TypeDef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "FlashIf.h"
//#include "SysAPI.h"
#include "SysDebug.h"
#include "SysFs.h"
#include "task.h"
//#include "flash.h"
#include "FileMgr.h"
#include "SpiBus.h"
#include "AD.h"
#include "flash_efc.h"

#ifdef SYS_WIN
#pragma warning(disable:4996)
#endif

//static BYTE g_InFlashBuf[INSECT_SIZE];
//static BYTE g_ExFlash[EXSECT_SIZE];

//static TSem g_semInFlash;
static TSem g_semExFlash;   //���ڱ�����д�ⲿFLASH�������򲻱����
//static BYTE g_bWrChkBuf[128];

static bool fInFlashWrEn = true;  //�ڲ�FLASH д����

void EnInFlash(void)
{
    fInFlashWrEn = true;
}

void DisInFlash(void)
{
    fInFlashWrEn = false;
}

bool IsEnInFlash(void)
{
    return fInFlashWrEn;
}

//��������ӡ������Ϣ
//������NONE
//���أ�NONE
int ErrInfo(int iErr)
{
	switch (iErr)
	{
	case FLASH_ERR_UNKNOW:
		DTRACE(DB_FS, ("Flash Operation: There has an unknow error!\r\n"));
		break;
	case FLASH_ERR_PARAMETER:
		DTRACE(DB_FS, ("Flash Operation: The parameter is wrong!\r\n"));
		break;
	case FLASH_ERR_RD_FAIL:
		DTRACE(DB_FS, ("Flash Operation: Read flash fail!\r\n"));
		break;
	case FLASH_ERR_ERASE:
		DTRACE(DB_FS, ("Flash Operation: Erase section fail!\r\n"));
		break;
	default:
		break;
	}
	
	return iErr;
}

//�������ڲ�Flash��ʼ��
//������NONE
//���أ�NONE
void InFlashInit(void)
{
	//����Ƶ��
	//  �� uSec ��ֵ��Ϊ 20��ָ�������������� 20 MHz ��Ƶ���¡� 
    DWORD dwRc;
    dwRc = flash_init(FLASH_ACCESS_MODE_128, 6);
	if (dwRc != FLASH_RC_OK) {
		DTRACE(DB_CRITICAL, ("-F- Initialization error\n\r"));
		return;
	}
}

//�������ڲ���ַת��
//������@dwLogicAddr �����߼�ƫ��
//���أ�������Flash�еľ��Ե�ַ
BYTE* ParaAddrConv(DWORD dwLogicAddr)
{
    return ((BYTE *)(dwLogicAddr+INDATA_ADDR));
}

//�������ڲ�Flash��
//������@dwAddr ���ڲ�Flash�ĵ�ַ
//		@pbBuf ���ڲ�Flash�Ļ�����
//		@dwLen �����ݵĳ���
//���أ���ȷ�������ض����ݵĳ��ȣ����򷵻���Ӧ������
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
//	return FlashRd(USER_PARA_PATH"InFlash.dat", dwAddr, pbBuf, dwLen);
	
	dwAddr += INDATA_ADDR;		//Ӧ�õ��õ�ʱ���ַ��0��ʼ�㣬��д��ʱ����Ҫ����ƫ��
	
	//����ַ�Լ����Ȳ���
	if (dwAddr+dwLen > INFLASH_END)	//��ַ���ߴӵ�ǰ��ַ��ʼ��dwLen�����ȳ������ڲ�Flash�Ľ���
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	memcpy(pbBuf, (BYTE *)dwAddr, dwLen);
	
	return dwLen;
}

//����:�ڲ�FLASH�޶�������д
//������@dwAddr д�ڲ�Flash�ĵ�ַ
//		@pbBuf д���ڲ�Flash������
//		@dwSectSize д���ڲ�Flash�Ĵ�С
//���أ���ȷд�뷵��д�볤�ȣ����򷵻���Ӧ������
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
#if 0
	WORD i;
	DWORD dwGetData;
#endif

	dwAddr += INDATA_ADDR;		//Ӧ�õ��õ�ʱ���ַ��0��ʼ�㣬��д��ʱ����Ҫ����ƫ��
	
	//���ȼ�����
	if (dwAddr&0x3ff != 0)			//��ַ����������������	1K
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if (dwAddr+dwSectSize >= INFLASH_END)	//��ַ�������ڲ�FLASH�ķ�Χ
		return ErrInfo(FLASH_ERR_PARAMETER);

	if (dwAddr < INDATA_ADDR) //�����Ƿ�ֹdwAddr��ͷ��ֻ����д�ڲ���������
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if (dwSectSize != INSECT_SIZE)	//д�����ݴ�С������������С
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if ((dwAddr%sizeof(unsigned long)!=0) || (dwSectSize%sizeof(unsigned long)!=0))	//���ֽ��ж�
		return ErrInfo(FLASH_ERR_PARAMETER);
    
	//  ����һ�� Flash �顣 
	//if (FlashSectErase(dwAddr) < 0)
/*	if (FLASHD_Erase(dwAddr) != 0)      //EEFC,���Զ�����
	{
		return ErrInfo(FLASH_ERR_ERASE);
	}*/
	
#if 0 //testing by Ryan
	for (i=0; i<INSECT_SIZE; i+=4)
	{
		dwGetData = HWREG(dwAddr+i);
		if (dwGetData != 0xffffffff)
			break;
	}
		
	if (i != INSECT_SIZE)
		dwGetData = HWREG(dwAddr+i+4);
#endif
           
    ADStop();

    EnterCritical();    
    
    if (IsEnInFlash())    //�ڲ�FLASH����д
    {
     
        flash_unlock(INDATA_ADDR, INFLASH_END-1, 0, 0);	
        
        BYTE bCrossWrite = 0;
        void     *pvBufferRemain = 0;
        DWORD dwSizeRemain = 0;
        /* Check if the write operation will cross over flash0 and flash1 */
        if ( ((dwAddr + dwSectSize) <= (IFLASH0_ADDR + IFLASH_SIZE/2)) ||
             (dwAddr >= IFLASH1_ADDR) )
        {
            bCrossWrite = 0;
        }
        else
        {
            /* Save information for write operation on flash1 */
            bCrossWrite = 1;
            pvBufferRemain = (void *)((BYTE *)pbBuf + (IFLASH1_ADDR - dwAddr));
            dwSizeRemain = dwSectSize - (IFLASH1_ADDR - dwAddr);
            /* Adjust size for write operation on flash0 */
            dwSectSize = (IFLASH1_ADDR - dwAddr);
        }
        
    fd_write:    
        //  ���һЩ���ݵ����²����� Flash ���С� 
        if (flash_write(dwAddr, pbBuf, dwSectSize, 1) != FLASH_RC_OK)//дFLASH���ܱ���ϣ�дһҳҪ4.2ms,д1KҪ15.2ms,�����Ӱ�콻��
        {
            flash_lock(INDATA_ADDR, INFLASH_END-1, 0, 0);
            ExitCritical();
            
            g_bJumpCyc = 1;
    
            ADStart();
            return ErrInfo(FLASH_ERR_WRT_FAIL);
        }
        
        /* Check if there is a writing operation on flash1 */
        if (bCrossWrite == 1)
        {
            dwAddr = IFLASH1_ADDR;
            pbBuf = pvBufferRemain;
            dwSectSize = dwSizeRemain;
            bCrossWrite = 0;
            
            dwSizeRemain = 0;
            while (dwSizeRemain++ <= 0x1fffff);  //��BANKʱ������ʱ�£�����д����
            
            goto fd_write;
        }
        
        flash_lock(INDATA_ADDR, INFLASH_END-1, 0, 0);
        
    }
	
	ExitCritical();
    g_bJumpCyc = 1;

    ADStart();
	
	return INSECT_SIZE;
}

//������Ƭ��Flash��ʽ������������ȫ�����
//������NONE
//���أ���ȷ����true,���󷵻�false
/*
bool InFlashFormat()
{
	DWORD dwAddr = INDATA_ADDR;
	
    ADStop();
    EnterCritical();   
    flash_unlock(INDATA_ADDR, INFLASH_END-1, 0, 0);	
	while (dwAddr < INFLASH_END)
	{
		if (flash_erase_page(dwAddr, IFLASH_ERASE_PAGES_4) != 0)   //flash_erase_page(dwAddr, IFLASH_ERASE_PAGES_4)
		{
			if (flash_erase_page(dwAddr, IFLASH_ERASE_PAGES_4) != 0)
			{
                flash_lock(INDATA_ADDR, INFLASH_END-1, 0, 0);	
                ExitCritical();
                g_bJumpCyc = 1;

                ADStart();                
				return false;
			}
		}
		dwAddr += INSECT_SIZE;
	}
    flash_lock(INDATA_ADDR, INFLASH_END-1, 0, 0);	
    ExitCritical();
    g_bJumpCyc = 1;

    ADStart();  
	
	return true;
}

//���������ϵͳ�������ڲ�Flash��ȫ������
//������NONE
//���أ���ȷ�������1�����򷵻���Ӧ������

int ClrAllPara()   //��InFlashFormat()һ��
{
	DWORD dwAddr;
	   
    EnterCritical();      //�������ٽ磬�����ź���������
    flash_unlock(INFLASH_LOCK_ADDR, INFLASH_END-1, 0, 0);	
    ADStop();
	for (dwAddr=INDATA_ADDR; dwAddr<INFLASH_END; dwAddr+=INSECT_SIZE)
	{        
		if (flash_erase_page(dwAddr, IFLASH_ERASE_PAGES_4) != 0)
		{
            flash_lock(INFLASH_LOCK_ADDR, INFLASH_END-1, 0, 0);	
            ExitCritical();
            g_bJumpCyc = 1;

            ADStart();            
			return ErrInfo(FLASH_ERR_ERASE);
		}        
	}
	flash_lock(INFLASH_LOCK_ADDR, INFLASH_END-1, 0, 0);	
    ExitCritical();
    g_bJumpCyc = 1;

    ADStart();    
	return 1;
}*/


//�������ѳ����ļ�д��Flash
//������@pbBuf-�������ݻ�����
//      @dwOffset-��������ƫ����
//      @dwLen-�������ݳ���
//���أ�true-д��ɹ���false-д��ʧ��
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen)
{    
    BYTE b;
    
    if (((dwAddr+dwLen)>=INAPP_ADDR) && (dwAddr<IN_PARACFG_ADDR)) //��ַ��Χ����ȷ, Ӧ�ó�����������д
        return false;
    
    b = dwLen%4;
   
    if (pbBuf!=NULL && dwLen!=0)
    {                
        if (b != 0)    //���ܱ�4����
            dwLen += (4-b);    //Note:todo:����ֻ�򵥴������£���Ҫ�ر�ע��
                        
	    if (dwAddr+dwLen > INFLASH_END)	//��ַ�������ڲ�FLASH�ķ�Χ
    		return false;
    		
    	if ((dwAddr%sizeof(unsigned long)!=0) || (dwLen%sizeof(unsigned long)!=0))	//���ֽ��ж�
    		return false;
    	
        ADStop();

        EnterCritical();    
        
        if (IsEnInFlash())    //�ڲ�FLASH����д
        {         
            //flash_unlock(INPROG_ADDR, INFLASH_END-1, 0, 0);	
            flash_unlock(INPROG_ADDR, IFLASH1_ADDR-1, 0, 0);	//���ܿ�BANK
            flash_unlock(IFLASH1_ADDR, INFLASH_END-1, 0, 0);	
            
            BYTE bCrossWrite = 0;
            void     *pvBufferRemain = 0;
            DWORD dwSizeRemain = 0;
            /* Check if the write operation will cross over flash0 and flash1 */
            if ( ((dwAddr + dwLen) <= (IFLASH0_ADDR + IFLASH_SIZE/2)) ||
                 (dwAddr >= IFLASH1_ADDR) )
            {
                bCrossWrite = 0;
            }
            else
            {
                /* Save information for write operation on flash1 */
                bCrossWrite = 1;
                pvBufferRemain = (void *)((BYTE *)pbBuf + (IFLASH1_ADDR - dwAddr));
                dwSizeRemain = dwLen - (IFLASH1_ADDR - dwAddr);
                /* Adjust size for write operation on flash0 */
                dwLen = (IFLASH1_ADDR - dwAddr);
            }
        
    fd_write:    
            //  ���һЩ���ݵ����²����� Flash ���С� 
            if (flash_write(dwAddr, pbBuf, dwLen, 1) != FLASH_RC_OK)//дFLASH���ܱ���ϣ�дһҳҪ4.2ms,д1KҪ15.2ms,�����Ӱ�콻��
            {
                //flash_lock(INPROG_ADDR, INFLASH_END-1, 0, 0);
                flash_lock(INPROG_ADDR, IFLASH1_ADDR-1, 0, 0);
                flash_lock(IFLASH1_ADDR, INFLASH_END-1, 0, 0);
                ExitCritical();
                
                g_bJumpCyc = 1;
        
                ADStart();
                return ErrInfo(FLASH_ERR_WRT_FAIL);
            }
            
            /* Check if there is a writing operation on flash1 */
            if (bCrossWrite == 1)
            {
                dwAddr = IFLASH1_ADDR;
                pbBuf = pvBufferRemain;
                dwLen = dwSizeRemain;
                bCrossWrite = 0;
                
                dwSizeRemain = 0;
                while (dwSizeRemain++ <= 0x1fffff);  //��BANKʱ������ʱ�£�����д����
                
                goto fd_write;
            }
        
            //flash_lock(INPROG_ADDR, INFLASH_END-1, 0, 0);
            flash_lock(INPROG_ADDR, IFLASH1_ADDR-1, 0, 0);
            flash_lock(IFLASH1_ADDR, INFLASH_END-1, 0, 0);
        }
    	ExitCritical();
        g_bJumpCyc = 1;
    
        ADStart();   
        return true;
    }
    
    return false;
}

//������BootLoader����,�����ݳ��������������򣬸���BOOTLOADER
//���أ���ȷд�뷵��д�볤�ȣ����򷵻���Ӧ������
bool BootLoaderUpd(void)
{
    BYTE bBuf[32];
    DWORD dwFileLen;
    WORD wFileCrc;
    DWORD dwMyLen;
    WORD wMyCrc = 0;
    DWORD dwAddr;
    DWORD dwLen;
    WORD i, wSect;
    DWORD dwOffset;
    
    if (!GetBakPara(NULL, &dwFileLen, &wFileCrc))
        return false;
    if ((dwFileLen > BL_LEN) || (dwFileLen == 0))
        return false;
    
    dwLen = dwFileLen;
    
    //����ļ���У��
    if (dwLen > (EXSECT_SIZE-EX_USER_APP_ADDR)) 
        dwMyLen = EXSECT_SIZE-EX_USER_APP_ADDR;
    else
        dwMyLen = dwLen;
        
    dwAddr = EX_USER_APP_ADDR;
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    ExFlashRd(dwAddr, g_ExFlashBuf, dwMyLen);
    wMyCrc = get_crc_16(wMyCrc, g_ExFlashBuf, dwMyLen);
    
    wSect = (dwLen-dwMyLen)/EXSECT_SIZE+1;//Ϊ�����Ч�ʣ�һ�ζ�4Kһ������
    
    for (i=0; i<wSect; i++)
    {        
        dwAddr += dwMyLen;
        dwLen -= dwMyLen;
        
        if (dwLen > EXSECT_SIZE)
            dwMyLen = EXSECT_SIZE;
        else
            dwMyLen = dwLen;
        
        if (dwMyLen == 0)
            break;
                           
        ExFlashRd(dwAddr, g_ExFlashBuf, dwMyLen);
        wMyCrc = get_crc_16(wMyCrc, g_ExFlashBuf, dwMyLen);     
        ClearWDG();
    }
    if (wFileCrc != wMyCrc)
    {
        SignalSemaphore(g_semExFlashBuf);
        return false;    
    }
    
    dwLen = dwFileLen;
    
    //����BOOTLOADER
    if (dwLen > (EXSECT_SIZE-EX_USER_APP_ADDR)) 
        dwMyLen = EXSECT_SIZE-EX_USER_APP_ADDR;
    else
        dwMyLen = dwLen;
        
    dwAddr = EX_USER_APP_ADDR;
    dwOffset = BL_ADDR;   //BOOTLOADER��ʼ��ַ
    
    ExFlashRd(dwAddr, g_ExFlashBuf, dwMyLen);
    
    //InUartWrite(g_ExFlashBuf, dwMyLen);//////for test
    
    Program(g_ExFlashBuf, dwOffset, dwMyLen);
        
    wSect = (dwLen-dwMyLen)/EXSECT_SIZE+1;//Ϊ�����Ч�ʣ�һ�ζ�4Kһ������
    
    for (i=0; i<wSect; i++)
    {        
        dwAddr += dwMyLen;
        dwOffset += dwMyLen;
        dwLen -= dwMyLen;
        
        if (dwLen > EXSECT_SIZE)
            dwMyLen = EXSECT_SIZE;
        else
            dwMyLen = dwLen;
        
        if (dwMyLen == 0)
            break;
                           
        ExFlashRd(dwAddr, g_ExFlashBuf, dwMyLen);
        
        //InUartWrite(g_ExFlashBuf, dwMyLen);//////for test
        ClearWDG();
        
        Program(g_ExFlashBuf, dwOffset, dwMyLen);  
    }
    
    memcpy(g_ExFlashBuf, &dwFileLen, 4);
    memcpy(g_ExFlashBuf+4, &wFileCrc, 4);
    Program(g_ExFlashBuf, BL_USE_LEN, 8);
    
    SignalSemaphore(g_semExFlashBuf);
   
    //�����ⲿ����Ϊ��Ч
    bBuf[EX_UPDATA_FLAG_ADDR] = 0;
    bBuf[EX_UPDATA_FLAG_ADDR+1] = 0;    
    ExFlashWrDataNoChk(EX_UPDATA_FLAG_ADDR, NULL, bBuf, 2);
    
    //todo:liyan��������֤���ǲ�����д��һ��
	return true;
}

//��ȡBOOTLOADER�İ汾��
BYTE GetBootVer(char *pcBuf, BYTE bBufSize)
{
    if (bBufSize < 32)
        return 0;
    memcpy(pcBuf, (void *)USER_BOOT_VER_ADDR, 32);
    return 32;
}

//��鲢������������BOOTLOADER����
void UpdatBL(void)
{
    //BYTE bBuf[32];
    char sName[24];
    
    if (!GetBakPara(sName, NULL, NULL))
        return;
    
    if (strstr(sName, "bootloader") != NULL)//Ƭ�����bootloader�����ܸ��µ�Ƭ�ڡ�
    {
        //SetInfo(INFO_UPDATE_BOOTLOADER); 
        DTRACE(DB_CRITICAL, ("UpdatBL : Bootloader updating......\n"));
        ClearWDG();
	    BootLoaderUpd();
        ClearWDG();
        DTRACE(DB_CRITICAL, ("UpdatBL : Bootloader update is over!\r\n"));
        SetInfo(INFO_APP_RST);  //���︴λ����ΪU������ܻ��в��������ļ���Ҫ����
    }
}

//------------------------------------------------------------------------------
//�ⲿFLASH������ֹ
//��������CPU��⵽�����Ӧ�ý�ֹ�ⲿFLASH��д����
void DisExFlash(void)
{
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
}


//����:�ⲿFlash��ʼ������
//������NONE
//���أ�NONE
void ExFlashInit(void)
{    
    g_semExFlash = NewSemaphore(1, 1);
}

//оƬû��
bool ExFlashIsNone(BYTE bChip)
{
    switch(bChip)
    {
    case EXFLASH_FST_CHIP:
        if (EXFLASH_1_SIZE == 0)
            return true;
        break;
    case EXFLASH_SND_CHIP:
        if (EXFLASH_2_SIZE == 0)
            return true;
        break;
    default:
        break;
    }
    return false;
}

//����:����ⲿFlash�Ƿ�busy
//������@bChip FlashƬѡ��Ϣ
//���أ������ѡƬFlash����busy״̬������true�����򷵻�false
bool ExFlashIsBusy(BYTE bChip)
{
	BYTE bReg;

	SPIEnable(bChip);
	SSI1SendByte(bChip, CMD_EXFSH_REG_L);
	bReg = SSI1GetByte(bChip);
	SPIDisable(bChip);

	return (bReg&STATUS_EXFSH_BUSY != 0x00);
}

//����:�ⲿFlash����
//������@bChip FlashƬѡ��Ϣ
//		@dwWaitMS ��ʱ�ȴ�ʱ��
//���أ������ȵ�Flashʹ��Ȩ����true�����򷵻�false
bool ExFlashPret(BYTE bChip, DWORD dwWaitMS)
{
	DWORD dwStart = GetTick();
    bool fInfinite = false;
    
    if (dwWaitMS == 0)
        fInfinite = true;
    
    if (ExFlashIsNone(bChip))
        return false;
      
	while (ExFlashIsBusy(bChip))
	{
		if (!fInfinite && (GetTick()-dwStart > dwWaitMS))
			return false;
        Sleep(5);
	}
	
	return true;
}

//����:�����ⲿFlashд����
//������@bChip FlashƬѡ��Ϣ
//���أ��ɹ����÷���true,���򷵻�false
bool ExFlashWrtEnable(BYTE bChip)
{
	DWORD dwStart = GetTick();
	if (!ExFlashPret(bChip, EXFLASH_WAIT_MS))		//�ȴ�Flash����
		return false;

	SPIEnable(bChip);					//ʹ��SSI�Լ�FLASH
	SSI1SendByte(bChip, CMD_EXFSH_WRT_EN);		//��д��������
	SPIDisable(bChip);
	
	return true;
}

//����:�����ⲿFlashд��ֹ
//������@bChip FlashƬѡ��Ϣ
//���أ�NONE
bool ExFlashWrtDisable(BYTE bChip)
{
	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))		//�ȴ�Flash����
		return false;
	
	SPIEnable(bChip);					//ʹ��SSI�Լ�FLASH
	SSI1SendByte(bChip, CMD_EXFSH_WRT_DIS);		//��д��������
	SPIDisable(bChip);
	
	return true;
}

//����:�����ⲿFlash��ָ������
//������@dwSectAddr ������������ʼ��ַ
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashEraseSect(DWORD dwSectAddr)
{
	BYTE bChip = (dwSectAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
	DWORD dwActAddr;

	//����ַ�Ƿ�����������ʼ��ַ
	if (dwSectAddr&0xfff != 0)
		return ErrInfo(FLASH_ERR_PARAMETER);

#if EXFLASH_ADDR                //��EXFLASH_ADDR����Ϊ0ʱ����������ȥ������
    if (dwSectAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	if (dwSectAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	//�����ַΪʵ�������ַ
	if (bChip == EXFLASH_FST_CHIP)
		dwActAddr = dwSectAddr - EXFLASH_FST_ADDR;
	else
		dwActAddr = dwSectAddr - EXFLASH_SND_ADDR;
    
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);

	ExFlashWrtEnable(bChip);		//����д����

	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }
	
	SPIEnable(bChip);					//ʹ��SSI�Լ�FLASH
	SSI1SendByte(bChip, CMD_EXFSH_SCT_ERS);	//����������
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));//���͵�ַ
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>8));
	SSI1SendByte(bChip, (BYTE)dwActAddr);
	SPIDisable(bChip);
	
	//	WEL���Զ�reset
    SignalSemaphore(g_semExFlash);
	return FLASH_ERR_OK;
}

//����:�����ⲿFlash��ָ����
//������@dwBlockAddr ���������ʼ��ַ
//      @bBlockType  0-32K�Ŀ飬1-64K�Ŀ�
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashEraseBlock(DWORD dwBlockAddr, BYTE bBlockType)
{
	BYTE bChip = (dwBlockAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
	DWORD dwActAddr;
    BYTE bCmd;

	//����ַ�Ƿ���������ʼ��ַ
    if (bBlockType == 0)
    {
    	if (dwBlockAddr&0x7fff != 0)
    		return ErrInfo(FLASH_ERR_PARAMETER);
        bCmd = CMD_EXFSH_BLK_32_ERS;
    }
    else
    {
        if (dwBlockAddr&0xffff != 0)
    		return ErrInfo(FLASH_ERR_PARAMETER);
        bCmd = CMD_EXFSH_BLK_64_ERS;
    }

#if EXFLASH_ADDR                //��EXFLASH_ADDR����Ϊ0ʱ����������ȥ������
    if (dwBlockAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	if (dwBlockAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	//�����ַΪʵ�������ַ
	if (bChip == EXFLASH_FST_CHIP)
		dwActAddr = dwBlockAddr - EXFLASH_FST_ADDR;
	else
		dwActAddr = dwBlockAddr - EXFLASH_SND_ADDR;
    
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);

	ExFlashWrtEnable(bChip);		//����д����

	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }
	
	SPIEnable(bChip);					//ʹ��SSI�Լ�FLASH
	SSI1SendByte(bChip, bCmd);	//����������
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));//���͵�ַ
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>8));
	SSI1SendByte(bChip, (BYTE)dwActAddr);
	SPIDisable(bChip);
    
    ExFlashPret(bChip, 1600); //��Ϊÿ������֮ǰ�����
	
	//	WEL���Զ�reset
    SignalSemaphore(g_semExFlash);
	return FLASH_ERR_OK;
}

//����:�ⲿFLASH�޶�������д
int ExFlashWrSectOnce(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	BYTE bChip = (dwAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
	WORD wPageCnt;
	DWORD dwActAddr;	//ʵ�ʲ�����ַ
	
//	BYTE bRdChkBuf[EXSECT_SIZE];
#if EXFLASH_ADDR                //��EXFLASH_ADDR����Ϊ0ʱ����������ȥ������
    if (dwAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	// ����ַ����
	if (dwAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if (dwAddr&0xfff != 0)	//��ַ����������������Ҳ����
		return ErrInfo(FLASH_ERR_PARAMETER);

	//��鳤�Ȳ���----����д���������ڿ�Ƭд�����⣬��Ϊ����������Ϊ��������
	if (dwSectSize != EXSECT_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	wPageCnt = dwSectSize/EXFLASH_PAGE_SIZE;

	// ��ַת��
	if (bChip == EXFLASH_FST_CHIP)
		dwActAddr = dwAddr-EXFLASH_FST_ADDR;
	else
		dwActAddr = dwAddr-EXFLASH_SND_ADDR;

	ExFlashEraseSect(dwAddr);	//������������--д��������Ҫ�ȶ��ٸģ������������Ӧ�����

    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
    
	//�ȴ���������
	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }

	//��������������д���ݣ���ҳд
	while (wPageCnt > 0)
	{
		//��������Ҫ����д����Ҫ���ڲ������棬��Ϊ����������ɻḴλд����д�������Ҳ�Ḵλд����
		ExFlashWrtEnable(bChip);

		if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))	//�ȴ�Flash����
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }

		SPIEnable(bChip);				//����SSI
		SSI1SendByte(bChip, CMD_EXFSH_WRT);	//��д����
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//����24λ��ַ
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);
		SSI1SendData(bChip, pbBuf, EXFLASH_PAGE_SIZE);	//Ȼ����256���ֽڵ�����
		SPIDisable(bChip);

		//һҳд�꣬�޸�ָ�룺��ַ�����ݶ���Ҫ�޸�
		pbBuf += EXFLASH_PAGE_SIZE;
		dwActAddr += EXFLASH_PAGE_SIZE;

		wPageCnt--;
	}

    SignalSemaphore(g_semExFlash);
	return dwSectSize;
}

//����:�ⲿFLASH�޶�������д
int ExFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	BYTE bWrChkBuf[64];  //����ջ�᲻�����
	DWORD dwChkLen = 0;
	
	ExFlashWrSectOnce(dwAddr, pbBuf, dwSectSize);		//д
	
	//��������֤һ��
	while (dwChkLen < dwSectSize)
	{
		ExFlashRd(dwAddr+dwChkLen, bWrChkBuf, sizeof(bWrChkBuf));
	
		if (memcmp(bWrChkBuf, pbBuf+dwChkLen, sizeof(bWrChkBuf)) != 0)		//��������д��ȥ�Ĳ�һ����ʱ����дһ��
		{
			return ExFlashWrSectOnce(dwAddr, pbBuf, dwSectSize);		//д
		}
		
		dwChkLen += sizeof(bWrChkBuf);
	}
	
	return dwSectSize;
}

//����:�ⲿFLASH���޶������Ķ�
int ExFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
	BYTE bChip = (dwAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
//	bool fChipArs;	//���������Ƿ���Ҫ������оƬ�ж�ȡ
	DWORD dwActAddr;	//ʵ�ʲ�����ַ
	DWORD dwRdLen;
	DWORD dwRet;
	
#if EXFLASH_ADDR                //��EXFLASH_ADDR����Ϊ0ʱ����������ȥ������
    if (dwAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	// ����ַ����
	if (dwAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	// ��鳤�Ȳ���
	if (dwAddr+dwLen > EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);
	
#if 0	//testing by Ryan---get Flash's manufacturer ID & Device ID
	BYTE bGetData[4];
	
	SPIEnable(EXFLASH_FST_CHIP);
	SSI1SendByte(bChip, CMD_EXFSH_GET_ID);				//��������
	SSI1SendByte(bChip, 0);	//��24λ��ַ
	SSI1SendByte(bChip, 0);
	SSI1SendByte(bChip, 0);
	SSI1GetData(bChip, bGetData, 2);
	SPIDisable(EXFLASH_FST_CHIP);

	SPIEnable(EXFLASH_SND_CHIP);
	SSI1SendByte(bChip, CMD_EXFSH_GET_ID2);				//��������
	SSI1GetData(bChip, bGetData, 3);
	SPIDisable(EXFLASH_SND_CHIP);
#endif

    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
    
	dwRdLen = 0;
	if ((dwAddr<EXFLASH_SND_ADDR) && (dwAddr+dwLen > EXFLASH_SND_ADDR))
	{
		//fChipArs = true;	//��Ҫ����ƬFlash
		dwRdLen = EXFLASH_1_SIZE-(dwAddr-EXFLASH_FST_ADDR);	//��һƬFlash�������ݳ���
		dwActAddr = dwAddr-EXFLASH_FST_ADDR;	//���Ȳ����ĵ�ַһ���ǵ�һƬ�ĵ�ַ

		// �ȶ���һƬ
		if(!ExFlashPret(EXFLASH_FST_CHIP, EXFLASH_WAIT_MS))	//�ȴ�Flash����
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }
		
		SPIEnable(EXFLASH_FST_CHIP);	//��Ҫ����ƬFLASH�����ܵ��⣬bChipһ���ǵ���EXFLASH_FST_CHIP
		SSI1SendByte(bChip, CMD_EXFSH_RD);				//��������
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//��24λ��ַ
		SSI1SendByte(bChip, (BYTE)(dwActAddr>> 8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);

		//���������ʼ��������
		dwRet = SSI1GetData(bChip, pbBuf, dwRdLen);
		SPIDisable(EXFLASH_FST_CHIP);

		//��һƬ OVER�����ڶ�Ƭ
		//�ڶ�Ƭ�Ǵ���ʼ��ַ��ʼ���ģ�����dwActAddr=0�����ĳ���ΪdwLen-dwRdLen;
		dwActAddr = 0;
		dwRdLen = dwLen-dwRdLen;

		if(!ExFlashPret(EXFLASH_SND_CHIP, EXFLASH_WAIT_MS))	//�ȴ�Flash����
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }

		SPIEnable(EXFLASH_SND_CHIP);	//��Ҫ����ƬFLASH�����ܵ��⣬bChipһ���ǵ���EXFLASH_FST_CHIP
		SSI1SendByte(bChip, CMD_EXFSH_RD);				//��������
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//��24λ��ַ
		SSI1SendByte(bChip, (BYTE)(dwActAddr>> 8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);

		//���������ʼ��������
		dwRet = SSI1GetData(bChip, pbBuf+dwRet, dwRdLen);	//���Ż�������һ���Ѿ������ĵط�
		SPIDisable(EXFLASH_SND_CHIP);
	}
	else	//ֻ��һ��оƬ�������ַת��Ϊʵ�ʵ�ַ�����Ⱦ�������Ҫ���ĳ���
	{
		//fChipArs = false;	//��Ҫ����ƬFlash
		dwRdLen = dwLen;	//��һƬFlash�������ݳ���
		if (dwAddr < EXFLASH_SND_ADDR)
			dwActAddr = dwAddr-EXFLASH_FST_ADDR;
		else
			dwActAddr = dwAddr-EXFLASH_SND_ADDR;

		if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))	//�ȴ�Flash����
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }

		SPIEnable(bChip);	//��Ҫ����ƬFLASH�����ܵ��⣬bChipһ���ǵ���EXFLASH_FST_CHIP
		SSI1SendByte(bChip, CMD_EXFSH_RD);				//��������
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//��24λ��ַ
		SSI1SendByte(bChip, (BYTE)(dwActAddr>> 8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);

		//���������ʼ��������
		dwRet = SSI1GetData(bChip, pbBuf, dwRdLen);
		SPIDisable(bChip);
	}

    SignalSemaphore(g_semExFlash);
	return dwLen;
}

//����:�����ⲿFlash��ָ������
//������@dwSectAddr ������������ʼ��ַ
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashErase(BYTE bChip)
{
    //WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);  //����ֻ�����������û�в���
	ExFlashWrtEnable(bChip);		//����д����
	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        //SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }

	SPIEnable(bChip);		//ʹ��SSI�Լ�FLASH
	SSI1SendByte(bChip, CMD_EXFSH_CHIP_ERS);	//����оƬ��������
	SPIDisable(bChip);  	//	WEL���Զ�reset
    //SignalSemaphore(g_semExFlash);
    return FLASH_ERR_OK;
}

//����:�����ⲿFlash��ָ������
//������@dwSectAddr ������������ʼ��ַ
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashEraseChip()
{
    bool fRet1 = true;
    bool fRet2 = true;
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
    
    ExFlashErase(EXFLASH_FST_CHIP);
    ExFlashErase(EXFLASH_SND_CHIP);	

	if(!ExFlashPret(EXFLASH_FST_CHIP, 60*1000))	//�ȴ���ʽ�����
    {
        fRet1 = false;
    }

	if(!ExFlashPret(EXFLASH_SND_CHIP, 60*1000))	//�ȴ���ʽ�����
    {
        fRet2 = false;  
    }
    
    SignalSemaphore(g_semExFlash);
    if ((!fRet1) || (!fRet2))
        return ErrInfo(FLASH_ERR_ERASE_CHIP);

	return FLASH_ERR_OK;
}
/*
//dwOffset��Ҫд����ʼ��ַ����֤�Ƕζ����   dwLen��Ҫд�ĳ���  ֻ֧��512�ֽڳ���д
int FlashWriteMain(DWORD dwOffset, BYTE* pBuf, DWORD dwLen)
{
//    BYTE  bTempBuf[MAIN_SEG_LEN]; 
    DWORD i;
	DWORD dwWritten = (dwOffset-FLASH_BASE_MAIN);
//	DWORD dwWritten1 = 0;
	DWORD dwLenWritten = 0;
//	dwOffset -= FLASH_BASE_MAIN;
//    DWORD dwTemp = 0;
//	dwTemp = FLASH_BASE_MAIN+(dwOffset/512)*512;//Ҫд�ĵ�ַ���ڶε��׵�ַ  ���Ӧ���Ѿ��Ƕ���ʼ��
 	WORD bSeg = dwWritten/INSECT_SIZE;//255��BANK

    if (FlashEraseSeg(bSeg, false) < 0)//Ҫд�������Ѿ���֤��BANK�������  ����
        return dwLenWritten;

	if(dwLen == INSECT_SIZE)
    for (i=0; i<INSECT_SIZE; i++)
    {
         if (FlashWriteByte(dwOffset++, &pBuf[i], false) < 0)
              return dwLenWritten;
		dwLenWritten++;			  
    }
    return dwLenWritten;
	
	while(dwLenWritten < dwLen)
	{
        if (FlashReadMain(dwTemp, bTempBuf, MAIN_SEG_LEN)<0)
            return dwLenWritten;

        if (FlashEraseSeg(bSeg++, false) < 0)
            return dwLenWritten;
		
		if(dwLenWritten == 0)
		{
		    memcpy(&bTempBuf[dwOffset%512], pBuf, 512-dwOffset%512);
			dwWritten1 = 512-dwOffset%512;		
		}
		else
		{
			if(dwLen >= 512+dwWritten1)//ʣ��ûд�ĳ��ȳ���BANK�Ĵ�С  ��������Ӧ����BANK�����
			{
	    	    memcpy(bTempBuf, &pBuf[dwLenWritten], 512);
				dwWritten1 = 512;					
			}
			else
			{
	        	memcpy(bTempBuf, &pBuf[dwLenWritten], dwLen-dwLenWritten);
				dwWritten1 = dwLen-dwLenWritten;
			}
		}
			
		dwLenWritten += dwWritten1;
        for (i=0; i<512; i++)
        {
            if (FlashWriteByte(dwTemp++, &bTempBuf[i], false) < 0)
                return dwLenWritten;;
        }			
	}

	return dwLen;  
}*/

bool GetBakPara(char *psName, DWORD *pdwLen, WORD *pwCrc)
{
    BYTE bBuf[32];
    DWORD dwDataLen; //�ļ�����
    WORD wFileCrc;
    
    //��ȡ�ļ���Ϣ,���ﲻ���ļ�������֤��Ҫ��д���ʱ����֤
    if (ExFlashRd(EX_UPDATA_FLAG_ADDR, bBuf, EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR) 
        != EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR)
    {
        if (ExFlashRd(EX_UPDATA_FLAG_ADDR, bBuf, EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR) //������
        != EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR)
            return false;
    }
    
    if ((bBuf[EX_UPDATA_FLAG_ADDR] != EX_UPDATA_FLAG_1) || 
        (bBuf[EX_UPDATA_FLAG_ADDR+1] != EX_UPDATA_FLAG_2))  //�ⲿ������±�־��ȷ
        return false;
    
    memcpy(&dwDataLen, &bBuf[EX_USER_APP_LEN_ADDR], 4);
    if ((dwDataLen == 0) || (dwDataLen > 400*1024))
        return false;
        
    if (pdwLen != NULL)
        *pdwLen = dwDataLen;
    
    memcpy(&wFileCrc, &bBuf[EX_USER_APP_CRC_ADDR], 2); 
    if (pwCrc != NULL)
        *pwCrc = wFileCrc;
    
    if (psName != NULL)
    {
        memcpy((BYTE *)psName, &bBuf[EX_USER_APP_NAME_ADDR], 16);
        psName[16] = 0;
    }
    return true;
}
