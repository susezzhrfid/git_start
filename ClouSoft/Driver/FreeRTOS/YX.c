/*********************************************************************************************************
 * Copyright (c) 2012,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�YX.c
 * ժ    Ҫ�����ļ���Ҫʵ��YX���
 * ��ǰ�汾��1.0
 * ��    �ߣ�����
 * ������ڣ�2012��3�� 
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
    
    //memset(pYxPara, 0, sizeof(TYxPara)); //bLastYxInput��ֵ������0
    memcpy(g_bOldPolar, pYxPara->bYxPolar, sizeof(g_bOldPolar));  //ԭ�������Ա���
    memset(pYxPara->bYxFlag, 0, sizeof(pYxPara->bYxFlag));
    memset(pYxPara->bYxPolar, 0, sizeof(pYxPara->bYxFlag));           
    
    /*if (ReadItemEx(BN0, PN0, 0x2020, bBuf) > 0)
       pYxPara.wDelayMs = bBuf[0]+(bBuf[1]<<8);
    if (pYxPara.wDelayMs < 5)
        pYxPara.wDelayMs = 100;*/

    iLen = ReadItemEx(BN0, PN0, 0x820f, bBuf);  //0���� 1����
    for (i=0; i<MAX_P0_YX; i++)
    {        
        if (iLen > 0)
        {
            //pYxPara->bYxFlag[i>>3] |= (bBuf[0]&(0x01<<(i&7)));  //�����־
            pYxPara->bYxFlag[i>>3] |= (0x1&(0x01<<(i&7)));  //�����־
            pYxPara->bYxPolar[i>>3] |= (bBuf[i+1]&(0x01<<(i&7))); //���Ա�־
            //pYxPara->bYxFlag[i>>3] |= ((bBuf[1]&YX_COS_MASK)>>YX_COS_BIT)<<(i&7);
            //pYxPara->bYxFlag[i>>3] |= ((bBuf[1]&YX_SOE_MASK)>>YX_SOE_BIT)<<(i&7);
            //pYxPara->bYxFlag[i>>3] |= ((bBuf[1]&YX_VIP_MASK)>>YX_VIP_BIT)<<(i&7);
	    	DTRACE(DB_CRITICAL, ("YXLoadPara : pYxPara->wYxFlag=%x, pYxPara->wYxPolar =%x.\n", pYxPara->bYxFlag[i>>3], pYxPara->bYxPolar[i>>3]));
        }
        else
        {
            pYxPara->bYxFlag[i>>3] |= 1<<(i&7);    //Ĭ����Ч
            pYxPara->bYxPolar[i>>3] |= 1<<(i&7);   //Ĭ��a�ʹ���
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
        if (g_pYxPara->bYxVal[i>>3]&bBitMask)   //��
            bBuf[i+1] = 1;
        else
            bBuf[i+1] = 0;
    }
    WriteItemEx(BN0, PN0, 0x821f, bBuf);

	return true;
}

//100MS����һ��,��ȥ����ʱ��Ϊ100MS
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
        
    if (GetTick()-dwTick < 100) //ң��100ms��һ��
        return;
    dwTick = GetTick();
    
   	GetYxInput(bYxInput); //Ӳ����YX����   	
   	
   	if (GetInfo(INFO_YX_PARA))	//�������ò�����ң��C4F12���
   		YXLoadPara(g_pYxPara);
    
	if (IsPowerOff())     //ͣ�����ң���ǲ���ȷ�ġ�
		return;
    
    for (b=0; b<MAX_P0_YX; b++)
    {
        bBitMask = 1<<(b&7);
        if ((g_pYxPara->bYxFlag[b>>3] & bBitMask)  == 0) //��Ч
     		continue;
        
        if ((g_pYxPara->bYxPolar[b>>3]&bBitMask) != (g_bOldPolar[b>>3]&bBitMask))//���ĳλ�����Ա仯״̬Ӧ�ñ䣬���Ǳ�λ��Ӧ�ñ䣬��λֻ��Ӳ������
        {
            bVal = (bYxInput[b>>3]^g_pYxPara->bYxPolar[b>>3]) & bBitMask;
            g_pYxPara->bYxVal[b>>3] = (g_pYxPara->bYxVal[b>>3] & (~bBitMask)) | bVal;
                                
            //ReadItemEx(BN0, PN0, 0x8211+b, bBuf);
            //bBuf[0] &= ~bBitMask;               //״̬
            //bBuf[0] |= g_pYxPara->bYxVal[b>>3]&bBitMask;
            //bBuf[1] |= bBitMask;                  //��λ
            if (g_pYxPara->bYxVal[b>>3]&bBitMask)   //��
                bBuf[b] = YX_STATE_OFF;
            else
                bBuf[b] = YX_STATE_ON;

            WriteItemEx(BN0, PN0, 0x8211+b, bBuf);
            fYxChang = true;
            
            //g_bOldPolar[b>>3] &= !(0xff&bBitMask);
            g_bOldPolar[b>>3] &= ~bBitMask;
            g_bOldPolar[b>>3] |= g_pYxPara->bYxPolar[b>>3]&bBitMask;
        }   
        
	 	if ((bYxInput[b>>3]^g_pYxPara->bLastYxInput[b>>3]) & bBitMask)//Ӳ���б仯
	   	{
	    	g_pYxPara->bYXWobble[b>>3] |= bBitMask; //����������־  
	   	}
     	else //�ޱ仯
       	{            
        	if (g_pYxPara->bYXWobble[b>>3] & bBitMask) //�ж���
          	{
             	g_pYxPara->bYXWobble[b>>3] &= ~bBitMask;
             	
             	bVal = (bYxInput[b>>3]^g_pYxPara->bYxPolar[b>>3]) & bBitMask;
             	g_pYxPara->bYxVal[b>>3] = (g_pYxPara->bYxVal[b>>3] & (~bBitMask)) | bVal;
                
                if (bVal)
                {
                    DTRACE(DB_CRITICAL, ("DoYx : Yx-%d alarm!!\r\n", b));
                    ////SaveYxEvt(0x2021+b, PN0, 1);//��
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
                
                if (g_pYxPara->bYxVal[b>>3]&bBitMask)   //��
                    bBuf[b] = YX_STATE_OFF;
                else
                    bBuf[b] = YX_STATE_ON;
                
                memset(bPreBuf, 0, sizeof(bPreBuf));
                ReadItemEx(BN0, PN0, 0x821f, bPreBuf);
                WriteItemEx(BN0, PN0, 0x8211+b, bBuf);  //0��  1��
                
                ReadItemEx(BN0,PN0,0x8860,bStateWord);
                if (bBuf[b] == YX_STATE_ON)
                    bStateWord[0] |= (0x08<<b);
                else
                    bStateWord[0] &= ~(0x08<<b);
                WriteItemEx(BN0,PN0,0x8860,bStateWord);
                                    
                //TO DO��SAVE YX ALRAM
                bDataIdNum = sizeof(wYXBit)/sizeof(wYXBit[0]);
                tDataID[0].wBn = BN0;
                tDataID[0].wID = wYXBit[0];
                tDataID[0].wPn = PN0;
                HandleAlr(0xE2000039, PN0, tDataID, bDataIdNum, 0, now, bPreBuf, wDataLen);
                
                /*ReadItemEx(BN0, PN0, 0x108f, bBuf);  
                bBuf[0] &= ~bBitMask;//״̬
                bBuf[0] |= g_pYxPara->bYxVal[b>>3]&bBitMask;
                bBuf[1] |= bBitMask;                  //��λ
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
