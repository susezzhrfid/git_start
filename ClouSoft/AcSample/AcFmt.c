/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcFmt.cpp
 * 摘    要：本文件主要实现交采数据项的值到格式的转换接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
#include "Sample.h"
#include "AcSample.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "AcFmt.h"
#include "DbConst.h"
#include "DbFmt.h"
#include "LibDbAPI.h"

#if 0

#define ONE_PN	false	//仅入库到测量点0
#define DUO_PN	true	//支持两个测量点,入库到测量点0和配置的另外一个测量点


void AcValIToFmt(int val, BYTE* pbBuf, WORD wLen);
void AcValToFmtCpy(int val, BYTE* pbBuf, WORD wLen);

TAcValToDbCtrl g_AcValToDbCtrl[] = 
{
//		PN	 BANK	ID	 内部计算的索引		子ID个数, 长度,格式转换函数
	{DUO_PN, BN0, 0xb61f, AC_VAL_UA, 			3, 		2, 	ValToFmt7,},
	{DUO_PN, BN0, 0xb62f, AC_VAL_IA, 			3, 		3, 	ValToFmt25,},
	{DUO_PN, BN0, 0xb63f, AC_VAL_P,  			4, 		3, 	ValToFmt9,},
	{DUO_PN, BN0, 0xb64f, AC_VAL_Q,  			4, 		3, 	ValToFmt9,},
	{DUO_PN, BN0, 0xb65f, AC_VAL_COS,  			4, 		2, 	ValToFmt5},
	{DUO_PN, BN0, 0xb66f, AC_VAL_ANG_UA,  		6, 		2, 	ValToFmt5},	 //角度C1F49
	//{DUO_PN, BN0, 0xb67f, AC_VAL_S,  			4, 		3, 	ValToFmt9},	 //视在功率
	//{DUO_PN, BN0, 0xb6a0, AC_VAL_I0,  			1, 		3, 	ValToFmt25},//零序电流
	
	//{ONE_PN, BN2, 0x1120, AC_VAL_PHASESTATUS,  	1, 		1, 	AcValToFmtCpy},	//相序状态
};



#define AC_VAL2DB_NUM sizeof(g_AcValToDbCtrl)/sizeof(TAcValToDbCtrl)

TDataItem g_diHarPercent;
TDataItem g_diHarVal;
TDataItem g_diToTalHarPercent;
TDataItem g_diToTalHarVal;

void AcValIToFmt(int val, BYTE* pbBuf, WORD wLen)
{
	ValToFmt6((val+5)/10, pbBuf, wLen);
}

void AcValToFmtCpy(int val, BYTE* pbBuf, WORD wLen)
{
	memcpy(pbBuf, &val, wLen);
}

//描述:初始化交采数据入库的控制结构
//参数:@wPn 交采配置的测量点,不管是否配置了交采的测量点,都会写到测量点0
bool InitAcValToDb(WORD wPn)
{
	for (WORD i=0; i<AC_VAL2DB_NUM; i++)
	{
		//不管是否配置了交采的测量点,都会写到测量点0
		g_AcValToDbCtrl[i].diPn0 = GetItemEx(g_AcValToDbCtrl[i].wBn, 
									   	  	 PN0,
									   	  	 g_AcValToDbCtrl[i].wID);
		
		if (g_AcValToDbCtrl[i].fDuoPn  && wPn!=PN0)
		{ //该数据项支持写入到两个测量点,否则只写入到PN0 && 测量点配置了
			g_AcValToDbCtrl[i].diPn = GetItemEx(g_AcValToDbCtrl[i].wBn, 
									  	  	 	wPn,
									   	  	 	g_AcValToDbCtrl[i].wID);
		}
		else
		{
			memset(&g_AcValToDbCtrl[i].diPn, 0, sizeof(g_AcValToDbCtrl[i].diPn));
		}
	}
	
	//g_diHarPercent = GetItemEx(BN0, wPn, 0xB7FF); //当前A、B、C三相电压、电流,2~N次谐波含有率								   	  	 	
//	g_diToTalHarPercent = GetItemEx(BN2, PN0, 0x200f);

	return true;	
}

//描述:交采数据入库到数据库
//参数:@piVal 交采程序算好的采集量的值
void AcValToDb(int* piVal)
{
	BYTE* p;
	BYTE bBuf[64];
	
	for (WORD i=0; i<AC_VAL2DB_NUM; i++)
	{
		p = bBuf;
		for (WORD j=0; j<g_AcValToDbCtrl[i].wSubNum; j++)
		{
			g_AcValToDbCtrl[i].pfnAcValToFmt(piVal[g_AcValToDbCtrl[i].wIdx+j], 
											 p, g_AcValToDbCtrl[i].wLen);
			p += g_AcValToDbCtrl[i].wLen;
		}
		
		WriteItemDI(&g_AcValToDbCtrl[i].diPn0, bBuf);
		if (g_AcValToDbCtrl[i].diPn.pbAddr != NULL)
			WriteItemDI(&g_AcValToDbCtrl[i].diPn, bBuf);
	}	
}

//描述:谐波值的入库
//参数:@pwHarPercent 谐波含有率
//	   @pwHarVal 谐波有效值
void AcHarmonicToDb(WORD* pwHarPercent, WORD* pwHarVal)
{
	WORD i, j;
//	WORD* pwHarPercent0 = pwHarPercent;
//	WORD* pwHarVal0 = pwHarVal;
	BYTE bToTalBuf[16];
	BYTE bBuf[HARMONIC_NUM*SCN_NUM*2+1+64]; //64为预留
	
	//谐波含有率
	BYTE* p = bBuf;
	*p++ = HARMONIC_NUM;
	BYTE* p2 = bToTalBuf;
	
	for (i=0; i<6; i++)	//Ua,Ub,Uc,Ia,Ib,Ic
	{
		for (j=0; j<HARMONIC_NUM; j++)	//总,2~N
		{
			//BANK2的总谐波含有率
			if (j == 0)
			{	
				ValToFmt5(*pwHarPercent/10, p2, 2);
				p2 += 2;
			}
			
			//国标中的F58
		    if (i>=3 && j==0)	//电流没有总的谐波含有率
		    {
		        pwHarPercent++;
		    	continue;
		    }
		    
			ValToFmt5(*pwHarPercent++/10, p, 2);
			p += 2;
		}
	}
	
	if (g_diHarPercent.pbAddr != NULL)
		WriteItemDI(&g_diHarPercent, bBuf);
	
	if (g_diToTalHarPercent.pbAddr != NULL)
		WriteItemDI(&g_diToTalHarPercent, bToTalBuf);
	
/*	
	//谐波有效值
	p = bBuf;
	*p++ = HARMONIC_NUM;
	p2 = bToTalBuf;
	
	for (i=0; i<SCN_NUM; i++)
	{
		for (j=0; j<HARMONIC_NUM; j++) //2~N
		{
			if (j == 0) //BANK2的总谐波有效值
			{	
				if (i < 3)
					ValToFmt7(*pwHarVal++, p2, 2);
				else
					AcValIToFmt(*pwHarVal++, p2, 2);
					
				p2 += 2;
			}
			else //国标F57的2~N有效值
			{			
				if (i < 3)
					ValToFmt7(*pwHarVal++, p, 2);
				else
					AcValIToFmt(*pwHarVal++, p, 2);
					
				p += 2;
			}
		}
	}
	
	if (g_diHarVal.pbAddr != NULL)
		WriteItem(g_diHarVal, bBuf);
		
	if (g_diToTalHarVal.pbAddr != NULL)
		WriteItem(g_diToTalHarVal, bToTalBuf);*/
}

WORD AcEpToFmt(int64 val, BYTE* pbBuf)
{
	val = ABS(val);
	Uint64ToBCD((uint64 )val, pbBuf, 5);
	return 5;
}


WORD AcFmtToEp(BYTE* pbBuf, int64* piVal)
{
	*piVal = (int64 )BcdToUint64(pbBuf, 5);
	return 5;
}

WORD AcEqToFmt(int64 val, BYTE* pbBuf)
{
	val = ABS(val);
	Uint64ToBCD((uint64 )val, pbBuf, 4);
	return 4;
}

WORD AcFmtToEq(BYTE* pbBuf, int64* piVal)
{
	*piVal = (int64 )BcdToUint64(pbBuf, 4);
	return 4;
}

WORD AcDemandToFmt(DWORD dwDemand, BYTE* pbBuf)
{
	DWORDToBCD(dwDemand, pbBuf, 3);
	return 3;
}

WORD AcFmtToDemand(BYTE* pbBuf, DWORD* pdwDemand)
{
	*pdwDemand = BcdToDWORD(pbBuf, 3);
	return 3;
}
#endif