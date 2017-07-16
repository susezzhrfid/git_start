/*********************************************************************************************************
 * Copyright (c) 2012,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：YX.c
 * 摘    要：本文件主要实现YX检测
 * 当前版本：1.0
 * 作    者：李焱
 * 完成日期：2012年3月 
*********************************************************************************************************/
#include "FaAPI.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "YX.h"
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include "drivers.h"
#include "DrvConst.h"
#include "DbAPI.h"
#include "SysDebug.h"
#include "ExcTask.h"

TYxPara *g_pYxPara;

#define  YX_BUF_SIZE       16

BYTE g_bOldPolar[YX_MASK_SIZE];

/////////////////////////////////////////////////////////////////////////
void YXLoadPara(TYxPara* pYxPara)
{
	BYTE bBuf[YX_BUF_SIZE];
    int iLen;
    BYTE i;
    
    //memset(pYxPara, 0, sizeof(TYxPara)); //bLastYxInput的值不能清0
    memcpy(g_bOldPolar, pYxPara->bYxPolar, sizeof(g_bOldPolar));  //原来的属性保存
    memset(pYxPara->bYxFlag, 0, sizeof(pYxPara->bYxFlag));
    memset(pYxPara->bYxPolar, 0, sizeof(pYxPara->bYxFlag));           
    
    /*if (ReadItemEx(BN0, PN0, 0x2020, bBuf) > 0)
       pYxPara.wDelayMs = bBuf[0]+(bBuf[1]<<8);
    if (pYxPara.wDelayMs < 5)
        pYxPara.wDelayMs = 100;*/

    iLen = ReadItemEx(BN0, PN0, 0x820f, bBuf);  //0常开 1常闭
    for (i=0; i<MAX_P0_YX; i++)
    {        
        if (iLen > 0)
        {
            //pYxPara->bYxFlag[i>>3] |= (bBuf[0]&(0x01<<(i&7)));  //接入标志
            pYxPara->bYxFlag[i>>3] |= (0x1&(0x01<<(i&7)));  //接入标志
            pYxPara->bYxPolar[i>>3] |= (bBuf[i+1]&(0x01<<(i&7))); //属性标志
            //pYxPara->bYxFlag[i>>3] |= ((bBuf[1]&YX_COS_MASK)>>YX_COS_BIT)<<(i&7);
            //pYxPara->bYxFlag[i>>3] |= ((bBuf[1]&YX_SOE_MASK)>>YX_SOE_BIT)<<(i&7);
            //pYxPara->bYxFlag[i>>3] |= ((bBuf[1]&YX_VIP_MASK)>>YX_VIP_BIT)<<(i&7);
	    	DTRACE(DB_CRITICAL, ("YXLoadPara : pYxPara->wYxFlag=%x, pYxPara->wYxPolar =%x.\n", pYxPara->bYxFlag[i>>3], pYxPara->bYxPolar[i>>3]));
        }
        else
        {
            pYxPara->bYxFlag[i>>3] |= 1<<(i&7);    //默认有效
            pYxPara->bYxPolar[i>>3] |= 1<<(i&7);   //默认a型触点
        }
    }
}
			
bool YXInit(TYxPara* pYxPara)
{	 
    BYTE i;
    BYTE bBuf[YX_BUF_SIZE];   //MAX_P0_YX
    BYTE bBitMask;
    g_pYxPara = pYxPara;    
    
    memset(pYxPara, 0, sizeof(TYxPara));
    
	YXLoadPara(pYxPara);
    
    memcpy(g_bOldPolar, pYxPara->bYxPolar, sizeof(g_bOldPolar));
	
	GetYxInput(pYxPara->bLastYxInput);
    for (i=0; i<YX_MASK_SIZE; i++)
    {
        pYxPara->bYxVal[i] = (pYxPara->bLastYxInput[i] ^ pYxPara->bYxPolar[i]) & pYxPara->bYxFlag[i];
        WriteItemEx(BN2, PN0, 0x1100, pYxPara->bYxVal);
    }    	
	//WriteItemEx(BN0, PN0, 0xA04f, pYxPara->bYxVal);
    
    memset(bBuf, 0, sizeof(bBuf));
    for (i=0; i<MAX_P0_YX; i++)
    {
        bBitMask = 1<<(i&7);
        if (g_pYxPara->bYxVal[i>>3]&bBitMask)   //？
            bBuf[i+1] = 1;
        else
            bBuf[i+1] = 0;
    }
    WriteItemEx(BN0, PN0, 0x821f, bBuf);

	return true;
}

//100MS调用一次,则去抖的时间为100MS
void YXRun(void)
{ 
	BYTE b;
    BYTE bYxInput[YX_MASK_SIZE] = {0};
    bool fYxChang = false;
    //BYTE bYxChg;
    BYTE bBuf[MAX_P0_YX];
    BYTE bPreBuf[YX_BUF_SIZE];
    BYTE bStateWord[2];
//    TTime tmNow;
    BYTE bBitMask;
    static DWORD dwTick = 0;
    BYTE bVal;
    WORD  wYXBit[] = {0x821f};
    WORD wDataLen = GetItemsLenId(wYXBit,sizeof(wYXBit)/sizeof(wYXBit[0]));
    TBankItem tDataID[1] = { 0 };
    BYTE bDataIdNum = 0;
    TTime now;
    GetCurTime(&now);
        
    if (GetTick()-dwTick < 100) //遥信100ms调一次
        return;
    dwTick = GetTick();
    
   	GetYxInput(bYxInput); //硬件的YX输入   	
   	
   	if (GetInfo(INFO_YX_PARA))	//脉冲配置参数或遥信C4F12变更
   		YXLoadPara(g_pYxPara);
    
	if (IsPowerOff())     //停电检测的遥信是不正确的。
		return;
    
    for (b=0; b<MAX_P0_YX; b++)
    {
        bBitMask = 1<<(b&7);
        if ((g_pYxPara->bYxFlag[b>>3] & bBitMask)  == 0) //无效
     		continue;
        
        if ((g_pYxPara->bYxPolar[b>>3]&bBitMask) != (g_bOldPolar[b>>3]&bBitMask))//如果某位的属性变化状态应该变，但是变位不应该变，变位只由硬件决定
        {
            bVal = (bYxInput[b>>3]^g_pYxPara->bYxPolar[b>>3]) & bBitMask;
            g_pYxPara->bYxVal[b>>3] = (g_pYxPara->bYxVal[b>>3] & (~bBitMask)) | bVal;
                                
            //ReadItemEx(BN0, PN0, 0x8211+b, bBuf);
            //bBuf[0] &= ~bBitMask;               //状态
            //bBuf[0] |= g_pYxPara->bYxVal[b>>3]&bBitMask;
            //bBuf[1] |= bBitMask;                  //变位
            if (g_pYxPara->bYxVal[b>>3]&bBitMask)   //？
                bBuf[b] = YX_STATE_OFF;
            else
                bBuf[b] = YX_STATE_ON;

            WriteItemEx(BN0, PN0, 0x8211+b, bBuf);
            fYxChang = true;
            
            //g_bOldPolar[b>>3] &= !(0xff&bBitMask);
            g_bOldPolar[b>>3] &= ~bBitMask;
            g_bOldPolar[b>>3] |= g_pYxPara->bYxPolar[b>>3]&bBitMask;
        }   
        
	 	if ((bYxInput[b>>3]^g_pYxPara->bLastYxInput[b>>3]) & bBitMask)//硬件有变化
	   	{
	    	g_pYxPara->bYXWobble[b>>3] |= bBitMask; //建立抖动标志  
	   	}
     	else //无变化
       	{            
        	if (g_pYxPara->bYXWobble[b>>3] & bBitMask) //有抖动
          	{
             	g_pYxPara->bYXWobble[b>>3] &= ~bBitMask;
             	
             	bVal = (bYxInput[b>>3]^g_pYxPara->bYxPolar[b>>3]) & bBitMask;
             	g_pYxPara->bYxVal[b>>3] = (g_pYxPara->bYxVal[b>>3] & (~bBitMask)) | bVal;
                
                if (bVal)
                {
                    DTRACE(DB_CRITICAL, ("DoYx : Yx-%d alarm!!\r\n", b));
                    ////SaveYxEvt(0x2021+b, PN0, 1);//发
                    //bYxChg = 0;
                    //WriteItemEx(BN0, PN0, 0xA020+b, &bYxChg);
                }
                else
                {
                    DTRACE(DB_CRITICAL, ("DoYx : Yx-%d recover!!\r\n", b));
                   // //SaveYxEvt(0x2021+b, PN0, 0);
                    //bYxChg = 1;
                    //WriteItemEx(BN0, PN0, 0xA020+b, &bYxChg);
                }
                
                if (g_pYxPara->bYxVal[b>>3]&bBitMask)   //？
                    bBuf[b] = YX_STATE_OFF;
                else
                    bBuf[b] = YX_STATE_ON;
                
                memset(bPreBuf, 0, sizeof(bPreBuf));
                ReadItemEx(BN0, PN0, 0x821f, bPreBuf);
                WriteItemEx(BN0, PN0, 0x8211+b, bBuf);  //0合  1分
                
                ReadItemEx(BN0,PN0,0x8860,bStateWord);
                if (bBuf[b] == YX_STATE_ON)
                    bStateWord[0] |= (0x08<<b);
                else
                    bStateWord[0] &= ~(0x08<<b);
                WriteItemEx(BN0,PN0,0x8860,bStateWord);
                                    
                //TO DO：SAVE YX ALRAM
                bDataIdNum = sizeof(wYXBit)/sizeof(wYXBit[0]);
                tDataID[0].wBn = BN0;
                tDataID[0].wID = wYXBit[0];
                tDataID[0].wPn = PN0;
                HandleAlr(0xE2000039, PN0, tDataID, bDataIdNum, 0, now, bPreBuf, wDataLen);
                
                /*ReadItemEx(BN0, PN0, 0x108f, bBuf);  
                bBuf[0] &= ~bBitMask;//状态
                bBuf[0] |= g_pYxPara->bYxVal[b>>3]&bBitMask;
                bBuf[1] |= bBitMask;                  //变位
                WriteItemEx(BN0, PN0, 0x108f, bBuf);

				bBuf[0] = bBitMask;
				bBuf[1] = g_pYxPara->bYxVal[b>>3];
				GetCurTime(&tmNow);
				SaveAlrData(ERC_YXCHG, tmNow, bBuf, 0, 0);*/
             	fYxChang = true;
            }
        }
    }
        
    memcpy(g_pYxPara->bLastYxInput, bYxInput, YX_MASK_SIZE);
    
    if (fYxChang)
    {
    	WriteItemEx(BN2, PN0, 0x1100, g_pYxPara->bYxVal);
    }
}
