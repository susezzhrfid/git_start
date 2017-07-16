/*********************************************************************************************************
* Copyright (c) 2010,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：Flash.c
* 摘    要：本文件主要用于对FLASH接口进行封装
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2010-12-28
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
static TSem g_semExFlash;   //用于保护读写外部FLASH的流程序不被打断
//static BYTE g_bWrChkBuf[128];

static bool fInFlashWrEn = true;  //内部FLASH 写允许。

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

//描述：打印错误信息
//参数：NONE
//返回：NONE
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

//描述：内部Flash初始化
//参数：NONE
//返回：NONE
void InFlashInit(void)
{
	//设置频率
	//  将 uSec 的值设为 20，指明处理器运行在 20 MHz 的频率下。 
    DWORD dwRc;
    dwRc = flash_init(FLASH_ACCESS_MODE_128, 6);
	if (dwRc != FLASH_RC_OK) {
		DTRACE(DB_CRITICAL, ("-F- Initialization error\n\r"));
		return;
	}
}

//描述：内部地址转换
//参数：@dwLogicAddr 参数逻辑偏移
//返回：参数在Flash中的绝对地址
BYTE* ParaAddrConv(DWORD dwLogicAddr)
{
    return ((BYTE *)(dwLogicAddr+INDATA_ADDR));
}

//描述：内部Flash读
//参数：@dwAddr 读内部Flash的地址
//		@pbBuf 读内部Flash的缓冲区
//		@dwLen 读数据的长度
//返回：正确读出返回读数据的长度，否则返回相应错误码
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
//	return FlashRd(USER_PARA_PATH"InFlash.dat", dwAddr, pbBuf, dwLen);
	
	dwAddr += INDATA_ADDR;		//应用调用的时候地址从0开始算，读写的时候需要加上偏移
	
	//检查地址以及长度参数
	if (dwAddr+dwLen > INFLASH_END)	//地址或者从当前地址开始算dwLen个长度超出了内部Flash的界限
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	memcpy(pbBuf, (BYTE *)dwAddr, dwLen);
	
	return dwLen;
}

//描述:内部FLASH限定扇区的写
//参数：@dwAddr 写内部Flash的地址
//		@pbBuf 写入内部Flash的内容
//		@dwSectSize 写入内部Flash的大小
//返回：正确写入返回写入长度，否则返回相应错误码
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
#if 0
	WORD i;
	DWORD dwGetData;
#endif

	dwAddr += INDATA_ADDR;		//应用调用的时候地址从0开始算，读写的时候需要加上偏移
	
	//首先检查参数
	if (dwAddr&0x3ff != 0)			//地址不是扇区的整数倍	1K
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if (dwAddr+dwSectSize >= INFLASH_END)	//地址超出了内部FLASH的范围
		return ErrInfo(FLASH_ERR_PARAMETER);

	if (dwAddr < INDATA_ADDR) //这里是仿止dwAddr掉头。只允许写内部参数部分
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if (dwSectSize != INSECT_SIZE)	//写的内容大小超出了扇区大小
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if ((dwAddr%sizeof(unsigned long)!=0) || (dwSectSize%sizeof(unsigned long)!=0))	//整字节判断
		return ErrInfo(FLASH_ERR_PARAMETER);
    
	//  擦除一个 Flash 块。 
	//if (FlashSectErase(dwAddr) < 0)
/*	if (FLASHD_Erase(dwAddr) != 0)      //EEFC,会自动擦除
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
    
    if (IsEnInFlash())    //内部FLASH允许写
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
        //  编程一些数据到最新擦除的 Flash 块中。 
        if (flash_write(dwAddr, pbBuf, dwSectSize, 1) != FLASH_RC_OK)//写FLASH不能被打断，写一页要4.2ms,写1K要15.2ms,这个会影响交采
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
            while (dwSizeRemain++ <= 0x1fffff);  //跨BANK时必需延时下，否则写不了
            
            goto fd_write;
        }
        
        flash_lock(INDATA_ADDR, INFLASH_END-1, 0, 0);
        
    }
	
	ExitCritical();
    g_bJumpCyc = 1;

    ADStart();
	
	return INSECT_SIZE;
}

//描述：片内Flash格式化，将参数段全部清除
//参数：NONE
//返回：正确返回true,错误返回false
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

//描述：清除系统库存放在内部Flash的全部内容
//参数：NONE
//返回：正确清除返回1，否则返回相应错误码

int ClrAllPara()   //与InFlashFormat()一样
{
	DWORD dwAddr;
	   
    EnterCritical();      //进入了临界，不用信号量保护了
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


//描述：把程序文件写入Flash
//参数：@pbBuf-程序数据缓冲区
//      @dwOffset-程序数据偏移量
//      @dwLen-程序数据长度
//返回：true-写入成功，false-写入失败
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen)
{    
    BYTE b;
    
    if (((dwAddr+dwLen)>=INAPP_ADDR) && (dwAddr<IN_PARACFG_ADDR)) //地址范围不正确, 应用程序区不允许写
        return false;
    
    b = dwLen%4;
   
    if (pbBuf!=NULL && dwLen!=0)
    {                
        if (b != 0)    //不能被4整除
            dwLen += (4-b);    //Note:todo:这里只简单处理了下，需要特别注意
                        
	    if (dwAddr+dwLen > INFLASH_END)	//地址超出了内部FLASH的范围
    		return false;
    		
    	if ((dwAddr%sizeof(unsigned long)!=0) || (dwLen%sizeof(unsigned long)!=0))	//整字节判断
    		return false;
    	
        ADStop();

        EnterCritical();    
        
        if (IsEnInFlash())    //内部FLASH允许写
        {         
            //flash_unlock(INPROG_ADDR, INFLASH_END-1, 0, 0);	
            flash_unlock(INPROG_ADDR, IFLASH1_ADDR-1, 0, 0);	//不能跨BANK
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
            //  编程一些数据到最新擦除的 Flash 块中。 
            if (flash_write(dwAddr, pbBuf, dwLen, 1) != FLASH_RC_OK)//写FLASH不能被打断，写一页要4.2ms,写1K要15.2ms,这个会影响交采
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
                while (dwSizeRemain++ <= 0x1fffff);  //跨BANK时必需延时下，否则写不了
                
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

//描述：BootLoader更新,将备份程序区的引导程序，更新BOOTLOADER
//返回：正确写入返回写入长度，否则返回相应错误码
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
    
    //检查文件的校验
    if (dwLen > (EXSECT_SIZE-EX_USER_APP_ADDR)) 
        dwMyLen = EXSECT_SIZE-EX_USER_APP_ADDR;
    else
        dwMyLen = dwLen;
        
    dwAddr = EX_USER_APP_ADDR;
    
    WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);
    ExFlashRd(dwAddr, g_ExFlashBuf, dwMyLen);
    wMyCrc = get_crc_16(wMyCrc, g_ExFlashBuf, dwMyLen);
    
    wSect = (dwLen-dwMyLen)/EXSECT_SIZE+1;//为了提高效率，一次读4K一个扇区
    
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
    
    //更新BOOTLOADER
    if (dwLen > (EXSECT_SIZE-EX_USER_APP_ADDR)) 
        dwMyLen = EXSECT_SIZE-EX_USER_APP_ADDR;
    else
        dwMyLen = dwLen;
        
    dwAddr = EX_USER_APP_ADDR;
    dwOffset = BL_ADDR;   //BOOTLOADER起始地址
    
    ExFlashRd(dwAddr, g_ExFlashBuf, dwMyLen);
    
    //InUartWrite(g_ExFlashBuf, dwMyLen);//////for test
    
    Program(g_ExFlashBuf, dwOffset, dwMyLen);
        
    wSect = (dwLen-dwMyLen)/EXSECT_SIZE+1;//为了提高效率，一次读4K一个扇区
    
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
   
    //设置外部程序为无效
    bBuf[EX_UPDATA_FLAG_ADDR] = 0;
    bBuf[EX_UPDATA_FLAG_ADDR+1] = 0;    
    ExFlashWrDataNoChk(EX_UPDATA_FLAG_ADDR, NULL, bBuf, 2);
    
    //todo:liyan读出来验证下是不是与写的一样
	return true;
}

//读取BOOTLOADER的版本号
BYTE GetBootVer(char *pcBuf, BYTE bBufSize)
{
    if (bBufSize < 32)
        return 0;
    memcpy(pcBuf, (void *)USER_BOOT_VER_ADDR, 32);
    return 32;
}

//检查并升级备份区的BOOTLOADER程序
void UpdatBL(void)
{
    //BYTE bBuf[32];
    char sName[24];
    
    if (!GetBakPara(sName, NULL, NULL))
        return;
    
    if (strstr(sName, "bootloader") != NULL)//片外的是bootloader，不能更新到片内。
    {
        //SetInfo(INFO_UPDATE_BOOTLOADER); 
        DTRACE(DB_CRITICAL, ("UpdatBL : Bootloader updating......\n"));
        ClearWDG();
	    BootLoaderUpd();
        ClearWDG();
        DTRACE(DB_CRITICAL, ("UpdatBL : Bootloader update is over!\r\n"));
        SetInfo(INFO_APP_RST);  //这里复位是因为U盘里可能还有参数配置文件需要升级
    }
}

//------------------------------------------------------------------------------
//外部FLASH操作禁止
//描述：当CPU检测到掉电后，应该禁止外部FLASH的写操作
void DisExFlash(void)
{
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
}


//描述:外部Flash初始化函数
//参数：NONE
//返回：NONE
void ExFlashInit(void)
{    
    g_semExFlash = NewSemaphore(1, 1);
}

//芯片没有
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

//描述:检查外部Flash是否busy
//参数：@bChip Flash片选信息
//返回：如果该选片Flash处于busy状态，返回true，否则返回false
bool ExFlashIsBusy(BYTE bChip)
{
	BYTE bReg;

	SPIEnable(bChip);
	SSI1SendByte(bChip, CMD_EXFSH_REG_L);
	bReg = SSI1GetByte(bChip);
	SPIDisable(bChip);

	return (bReg&STATUS_EXFSH_BUSY != 0x00);
}

//描述:外部Flash保护
//参数：@bChip Flash片选信息
//		@dwWaitMS 超时等待时间
//返回：正常等到Flash使用权返回true，否则返回false
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

//描述:设置外部Flash写允许
//参数：@bChip Flash片选信息
//返回：成功设置返回true,否则返回false
bool ExFlashWrtEnable(BYTE bChip)
{
	DWORD dwStart = GetTick();
	if (!ExFlashPret(bChip, EXFLASH_WAIT_MS))		//等待Flash空闲
		return false;

	SPIEnable(bChip);					//使能SSI以及FLASH
	SSI1SendByte(bChip, CMD_EXFSH_WRT_EN);		//发写允许命令
	SPIDisable(bChip);
	
	return true;
}

//描述:设置外部Flash写禁止
//参数：@bChip Flash片选信息
//返回：NONE
bool ExFlashWrtDisable(BYTE bChip)
{
	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))		//等待Flash空闲
		return false;
	
	SPIEnable(bChip);					//使能SSI以及FLASH
	SSI1SendByte(bChip, CMD_EXFSH_WRT_DIS);		//发写允许命令
	SPIDisable(bChip);
	
	return true;
}

//描述:擦除外部Flash的指定扇区
//参数：@dwSectAddr 擦除扇区的起始地址
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
int ExFlashEraseSect(DWORD dwSectAddr)
{
	BYTE bChip = (dwSectAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
	DWORD dwActAddr;

	//检查地址是否是整扇区起始地址
	if (dwSectAddr&0xfff != 0)
		return ErrInfo(FLASH_ERR_PARAMETER);

#if EXFLASH_ADDR                //当EXFLASH_ADDR定义为0时，这样可以去掉警告
    if (dwSectAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	if (dwSectAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	//处理地址为实际物理地址
	if (bChip == EXFLASH_FST_CHIP)
		dwActAddr = dwSectAddr - EXFLASH_FST_ADDR;
	else
		dwActAddr = dwSectAddr - EXFLASH_SND_ADDR;
    
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);

	ExFlashWrtEnable(bChip);		//设置写允许

	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }
	
	SPIEnable(bChip);					//使能SSI以及FLASH
	SSI1SendByte(bChip, CMD_EXFSH_SCT_ERS);	//发擦除命令
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));//发送地址
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>8));
	SSI1SendByte(bChip, (BYTE)dwActAddr);
	SPIDisable(bChip);
	
	//	WEL会自动reset
    SignalSemaphore(g_semExFlash);
	return FLASH_ERR_OK;
}

//描述:擦除外部Flash的指定块
//参数：@dwBlockAddr 擦除块的起始地址
//      @bBlockType  0-32K的块，1-64K的块
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
int ExFlashEraseBlock(DWORD dwBlockAddr, BYTE bBlockType)
{
	BYTE bChip = (dwBlockAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
	DWORD dwActAddr;
    BYTE bCmd;

	//检查地址是否是整块起始地址
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

#if EXFLASH_ADDR                //当EXFLASH_ADDR定义为0时，这样可以去掉警告
    if (dwBlockAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	if (dwBlockAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	//处理地址为实际物理地址
	if (bChip == EXFLASH_FST_CHIP)
		dwActAddr = dwBlockAddr - EXFLASH_FST_ADDR;
	else
		dwActAddr = dwBlockAddr - EXFLASH_SND_ADDR;
    
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);

	ExFlashWrtEnable(bChip);		//设置写允许

	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }
	
	SPIEnable(bChip);					//使能SSI以及FLASH
	SSI1SendByte(bChip, bCmd);	//发擦除命令
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));//发送地址
	SSI1SendByte(bChip, (BYTE)(dwActAddr>>8));
	SSI1SendByte(bChip, (BYTE)dwActAddr);
	SPIDisable(bChip);
    
    ExFlashPret(bChip, 1600); //因为每条命令之前还会等
	
	//	WEL会自动reset
    SignalSemaphore(g_semExFlash);
	return FLASH_ERR_OK;
}

//描述:外部FLASH限定扇区的写
int ExFlashWrSectOnce(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	BYTE bChip = (dwAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
	WORD wPageCnt;
	DWORD dwActAddr;	//实际操作地址
	
//	BYTE bRdChkBuf[EXSECT_SIZE];
#if EXFLASH_ADDR                //当EXFLASH_ADDR定义为0时，这样可以去掉警告
    if (dwAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	// 检查地址参数
	if (dwAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);
	
	if (dwAddr&0xfff != 0)	//地址不是扇区的整数倍也不行
		return ErrInfo(FLASH_ERR_PARAMETER);

	//检查长度参数----扇区写操作不存在跨片写的问题，因为是以整扇区为操作对象
	if (dwSectSize != EXSECT_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	wPageCnt = dwSectSize/EXFLASH_PAGE_SIZE;

	// 地址转换
	if (bChip == EXFLASH_FST_CHIP)
		dwActAddr = dwAddr-EXFLASH_FST_ADDR;
	else
		dwActAddr = dwAddr-EXFLASH_SND_ADDR;

	ExFlashEraseSect(dwAddr);	//擦除扇区内容--写扇区不需要先读再改，这个工作交给应用完成

    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
    
	//等待擦除结束
	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }

	//擦除结束，往里写数据，按页写
	while (wPageCnt > 0)
	{
		//擦除结束要设置写允许，要放在擦除后面，因为擦除动作完成会复位写允许，写操作完成也会复位写允许
		ExFlashWrtEnable(bChip);

		if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))	//等待Flash空闲
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }

		SPIEnable(bChip);				//启用SSI
		SSI1SendByte(bChip, CMD_EXFSH_WRT);	//发写命令
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//发送24位地址
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);
		SSI1SendData(bChip, pbBuf, EXFLASH_PAGE_SIZE);	//然后发送256个字节的数据
		SPIDisable(bChip);

		//一页写完，修改指针：地址和内容都需要修改
		pbBuf += EXFLASH_PAGE_SIZE;
		dwActAddr += EXFLASH_PAGE_SIZE;

		wPageCnt--;
	}

    SignalSemaphore(g_semExFlash);
	return dwSectSize;
}

//描述:外部FLASH限定扇区的写
int ExFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize)
{
	BYTE bWrChkBuf[64];  //测试栈会不会溢出
	DWORD dwChkLen = 0;
	
	ExFlashWrSectOnce(dwAddr, pbBuf, dwSectSize);		//写
	
	//读出来验证一下
	while (dwChkLen < dwSectSize)
	{
		ExFlashRd(dwAddr+dwChkLen, bWrChkBuf, sizeof(bWrChkBuf));
	
		if (memcmp(bWrChkBuf, pbBuf+dwChkLen, sizeof(bWrChkBuf)) != 0)		//读上来与写下去的不一样的时候，重写一次
		{
			return ExFlashWrSectOnce(dwAddr, pbBuf, dwSectSize);		//写
		}
		
		dwChkLen += sizeof(bWrChkBuf);
	}
	
	return dwSectSize;
}

//描述:外部FLASH不限定扇区的读
int ExFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen)
{
	BYTE bChip = (dwAddr<EXFLASH_SND_ADDR) ? EXFLASH_FST_CHIP : EXFLASH_SND_CHIP;
//	bool fChipArs;	//读的内容是否需要从两个芯片中读取
	DWORD dwActAddr;	//实际操作地址
	DWORD dwRdLen;
	DWORD dwRet;
	
#if EXFLASH_ADDR                //当EXFLASH_ADDR定义为0时，这样可以去掉警告
    if (dwAddr < EXFLASH_ADDR)
        return ErrInfo(FLASH_ERR_PARAMETER);
#endif
	// 检查地址参数
	if (dwAddr >= EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);

	// 检查长度参数
	if (dwAddr+dwLen > EXFLASH_ADDR+EXFLASH_SIZE)
		return ErrInfo(FLASH_ERR_PARAMETER);
	
#if 0	//testing by Ryan---get Flash's manufacturer ID & Device ID
	BYTE bGetData[4];
	
	SPIEnable(EXFLASH_FST_CHIP);
	SSI1SendByte(bChip, CMD_EXFSH_GET_ID);				//发读命令
	SSI1SendByte(bChip, 0);	//发24位地址
	SSI1SendByte(bChip, 0);
	SSI1SendByte(bChip, 0);
	SSI1GetData(bChip, bGetData, 2);
	SPIDisable(EXFLASH_FST_CHIP);

	SPIEnable(EXFLASH_SND_CHIP);
	SSI1SendByte(bChip, CMD_EXFSH_GET_ID2);				//发读命令
	SSI1GetData(bChip, bGetData, 3);
	SPIDisable(EXFLASH_SND_CHIP);
#endif

    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
    
	dwRdLen = 0;
	if ((dwAddr<EXFLASH_SND_ADDR) && (dwAddr+dwLen > EXFLASH_SND_ADDR))
	{
		//fChipArs = true;	//需要读两片Flash
		dwRdLen = EXFLASH_1_SIZE-(dwAddr-EXFLASH_FST_ADDR);	//第一片Flash读的数据长度
		dwActAddr = dwAddr-EXFLASH_FST_ADDR;	//首先操作的地址一定是第一片的地址

		// 先读第一片
		if(!ExFlashPret(EXFLASH_FST_CHIP, EXFLASH_WAIT_MS))	//等待Flash空闲
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }
		
		SPIEnable(EXFLASH_FST_CHIP);	//需要跨两片FLASH读，能到这，bChip一定是等于EXFLASH_FST_CHIP
		SSI1SendByte(bChip, CMD_EXFSH_RD);				//发读命令
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//发24位地址
		SSI1SendByte(bChip, (BYTE)(dwActAddr>> 8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);

		//发送完命令开始接收数据
		dwRet = SSI1GetData(bChip, pbBuf, dwRdLen);
		SPIDisable(EXFLASH_FST_CHIP);

		//第一片 OVER，读第二片
		//第二片是从起始地址开始读的，所以dwActAddr=0，读的长度为dwLen-dwRdLen;
		dwActAddr = 0;
		dwRdLen = dwLen-dwRdLen;

		if(!ExFlashPret(EXFLASH_SND_CHIP, EXFLASH_WAIT_MS))	//等待Flash空闲
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }

		SPIEnable(EXFLASH_SND_CHIP);	//需要跨两片FLASH读，能到这，bChip一定是等于EXFLASH_FST_CHIP
		SSI1SendByte(bChip, CMD_EXFSH_RD);				//发读命令
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//发24位地址
		SSI1SendByte(bChip, (BYTE)(dwActAddr>> 8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);

		//发送完命令开始接收数据
		dwRet = SSI1GetData(bChip, pbBuf+dwRet, dwRdLen);	//接着缓冲区上一次已经读到的地方
		SPIDisable(EXFLASH_SND_CHIP);
	}
	else	//只在一个芯片里读，地址转换为实际地址，长度就是所需要读的长度
	{
		//fChipArs = false;	//需要读两片Flash
		dwRdLen = dwLen;	//第一片Flash读的数据长度
		if (dwAddr < EXFLASH_SND_ADDR)
			dwActAddr = dwAddr-EXFLASH_FST_ADDR;
		else
			dwActAddr = dwAddr-EXFLASH_SND_ADDR;

		if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))	//等待Flash空闲
        {
            SignalSemaphore(g_semExFlash);
			return ErrInfo(FLASH_ERR_UNKNOW);
        }

		SPIEnable(bChip);	//需要跨两片FLASH读，能到这，bChip一定是等于EXFLASH_FST_CHIP
		SSI1SendByte(bChip, CMD_EXFSH_RD);				//发读命令
		SSI1SendByte(bChip, (BYTE)(dwActAddr>>16));	//发24位地址
		SSI1SendByte(bChip, (BYTE)(dwActAddr>> 8));
		SSI1SendByte(bChip, (BYTE)dwActAddr);

		//发送完命令开始接收数据
		dwRet = SSI1GetData(bChip, pbBuf, dwRdLen);
		SPIDisable(bChip);
	}

    SignalSemaphore(g_semExFlash);
	return dwLen;
}

//描述:擦除外部Flash的指定扇区
//参数：@dwSectAddr 擦除扇区的起始地址
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
int ExFlashErase(BYTE bChip)
{
    //WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);  //这里只是下了命令，并没有擦完
	ExFlashWrtEnable(bChip);		//设置写允许
	if(!ExFlashPret(bChip, EXFLASH_WAIT_MS))
    {
        //SignalSemaphore(g_semExFlash);
		return ErrInfo(FLASH_ERR_UNKNOW);
    }

	SPIEnable(bChip);		//使能SSI以及FLASH
	SSI1SendByte(bChip, CMD_EXFSH_CHIP_ERS);	//发送芯片擦除命令
	SPIDisable(bChip);  	//	WEL会自动reset
    //SignalSemaphore(g_semExFlash);
    return FLASH_ERR_OK;
}

//描述:擦除外部Flash的指定扇区
//参数：@dwSectAddr 擦除扇区的起始地址
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
int ExFlashEraseChip()
{
    bool fRet1 = true;
    bool fRet2 = true;
    WaitSemaphore(g_semExFlash, SYS_TO_INFINITE);
    
    ExFlashErase(EXFLASH_FST_CHIP);
    ExFlashErase(EXFLASH_SND_CHIP);	

	if(!ExFlashPret(EXFLASH_FST_CHIP, 60*1000))	//等待格式化完成
    {
        fRet1 = false;
    }

	if(!ExFlashPret(EXFLASH_SND_CHIP, 60*1000))	//等待格式化完成
    {
        fRet2 = false;  
    }
    
    SignalSemaphore(g_semExFlash);
    if ((!fRet1) || (!fRet2))
        return ErrInfo(FLASH_ERR_ERASE_CHIP);

	return FLASH_ERR_OK;
}
/*
//dwOffset是要写的起始地址，保证是段对齐的   dwLen是要写的长度  只支持512字节长度写
int FlashWriteMain(DWORD dwOffset, BYTE* pBuf, DWORD dwLen)
{
//    BYTE  bTempBuf[MAIN_SEG_LEN]; 
    DWORD i;
	DWORD dwWritten = (dwOffset-FLASH_BASE_MAIN);
//	DWORD dwWritten1 = 0;
	DWORD dwLenWritten = 0;
//	dwOffset -= FLASH_BASE_MAIN;
//    DWORD dwTemp = 0;
//	dwTemp = FLASH_BASE_MAIN+(dwOffset/512)*512;//要写的地址所在段的首地址  这个应该已经是段起始了
 	WORD bSeg = dwWritten/INSECT_SIZE;//255个BANK

    if (FlashEraseSeg(bSeg, false) < 0)//要写的数据已经保证是BANK对齐的了  并且
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
			if(dwLen >= 512+dwWritten1)//剩下没写的长度超过BANK的大小  程序到这里应该是BANK对齐的
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
    DWORD dwDataLen; //文件长度
    WORD wFileCrc;
    
    //读取文件信息,这里不对文件进行验证，要求写入的时候验证
    if (ExFlashRd(EX_UPDATA_FLAG_ADDR, bBuf, EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR) 
        != EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR)
    {
        if (ExFlashRd(EX_UPDATA_FLAG_ADDR, bBuf, EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR) //做两次
        != EX_USER_APP_ADDR-EX_UPDATA_FLAG_ADDR)
            return false;
    }
    
    if ((bBuf[EX_UPDATA_FLAG_ADDR] != EX_UPDATA_FLAG_1) || 
        (bBuf[EX_UPDATA_FLAG_ADDR+1] != EX_UPDATA_FLAG_2))  //外部程序更新标志正确
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
