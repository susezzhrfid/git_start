/*********************************************************************************************************
* Copyright (c) 2013,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：ParaMgr.c
* 摘    要：本文件主要用于对参数配置文件进行应用
* 当前版本：1.0
* 作    者：李焱
* 完成日期：2013-7-28
*********************************************************************************************************/
#include "ParaMgr.h"
#include "FlashIf.h"
#include "FaCfg.h"
#include "SysDebug.h"
#include "LibDbAPI.h"
#include "DbConst.h"
#include "DbGbAPI.h"
#include "EsamCmd.h"
#include "compiler.h"


bool OnWriteSpecialPara(WORD wBn, WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen)
{
	WORD wClass, wFN, wPN, wNum;
	BYTE* p = pbBuf;
	if (wID == 0xffff)
	{
		memcpy(&wClass, p+sizeof(WORD)*2, sizeof(WORD));
		memcpy(&wFN, p+sizeof(WORD)*3, sizeof(WORD));
		memcpy(&wPN, p+sizeof(WORD)*4, sizeof(WORD));

//		GBWriteItemEx(wClass,wFN,wPN,p+sizeof(WORD)*5,&wNum);

/*		if (wClass == GB_DATACLASS1 && wFN == 23)
		{//剩余电量加载到扩展ID
			int64 iRe=0;
			iRe=Fmt3ToVal64( p+sizeof(WORD)*5, 4);
			WriteItemVal64(BN0, wPN, 0x0a05, &iRe);				
			//TrigerSaveBank(BN0, SECT_CTRL_DATA, 0);
		}*/
		return true;			
	}	
	return false;
}


//文件格式:文件长度(4个字节)+CRC校验(4个字节)+数据1+数据2+...+数据n
//数据格式:数据项长度(2个字节,包含数据项ID/数据项BANK/数据项Point/数据项内容长度)
//+数据项ID(2个字节)+数据项BANK(2个字节)+数据项Point(2个字节)+数据项内容
bool ParaMgrParse(void)
{
    DWORD dwLen;
    BYTE *p;
    
    WORD wID;
    WORD wBank;
    WORD wPoint;
    WORD wLen;
	BYTE* p1;    

    DWORD dwCrc;
    WORD wOffset = 0;
    DWORD dwAddr;
    
    memcpy(&dwLen, (void *)IN_PARACFG_ADDR, sizeof(DWORD));
    memcpy(&dwCrc, (void *)(IN_PARACFG_ADDR+4), sizeof(DWORD));
        
    //检验参数配置文件的正确性
    if (dwLen<=8 || dwLen>IN_PARACFG_SIZE)
        return false;
    
    dwAddr = IN_PARACFG_ADDR+8;
    wID = 0; //临时用作算CRC
    wOffset += sizeof(DWORD)+sizeof(DWORD);
    
    WaitSemaphore(g_semEsam, SYS_TO_INFINITE);   
    
    while(wOffset < dwLen)
    {
        wLen = min(dwLen-wOffset, sizeof(g_bEsamTxRxBuf));
        memcpy(g_bEsamTxRxBuf, (void *)dwAddr, wLen);
        wID = get_crc_16(wID, g_bEsamTxRxBuf, wLen);
        wOffset += wLen;
        dwAddr += wLen;
    }
    if (wID != dwCrc)
    {
        goto ERROROUT;        
    }
        
    //应用参数配置文件            
    dwLen -= 8;
    dwAddr = IN_PARACFG_ADDR+8;
                             
    while (dwLen > 0) 
    {                
        p = g_bEsamTxRxBuf;
        //ExFlashRd(dwAddr, p, sizeof(WORD)*4);
        memcpy(p, (void *)dwAddr, sizeof(WORD)*4);
        dwAddr += sizeof(WORD)*4;
                
  		p1 = p;
        memcpy(&wLen, p, sizeof(WORD));
        p += sizeof(WORD);
        memcpy(&wID, p, sizeof(WORD));
        p += sizeof(WORD);
        memcpy(&wBank, p, sizeof(WORD));
        p += sizeof(WORD);
        memcpy(&wPoint, p, sizeof(WORD));
        p += sizeof(WORD);
                                
        if ((wLen+sizeof(WORD)) > sizeof(g_bEsamTxRxBuf))
        {
            DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : Buf is to small\r\n"));
            goto ERROROUT;
        }
                                
        //ExFlashRd(dwAddr, p, wLen-sizeof(WORD)*3);
        memcpy(p, (void *)dwAddr, wLen-sizeof(WORD)*3);
        dwAddr += (wLen-sizeof(WORD)*3);
        
    	if (!OnWriteSpecialPara(wBank, wPoint, wID, p1, wLen))
        {
            if (WriteItemEx(wBank, wPoint, wID, p) != (wLen-sizeof(WORD)*3))
            {
                DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : Parse para error, BANK=%d, POINT=%d, ID=%d\r\n", wBank, wPoint, wID));
                goto ERROROUT;
            }
        }
        dwLen -= (wLen+sizeof(WORD));                
    }
            
    SignalSemaphore(g_semEsam);
            
    return true;

ERROROUT:
    SignalSemaphore(g_semEsam);
    return false;
}
