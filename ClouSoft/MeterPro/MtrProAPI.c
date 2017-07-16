/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterPro.h
 * 摘    要：本文件主要包含抄表协议的基本API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/

#include "MtrProAPI.h"
#include "FaConst.h"


struct TMtrPro g_MtrPro[DYN_PN_NUM];

//描述:通过测量点号取出对应测量点的参数和保存变量
//参数:	@wPn:测量点号
//		@pMtrPara:用于返回该电表参数的指针
bool LoadMtrPara(WORD wPn, TMtrPara* pMtrPara)
{
	if (pMtrPara == NULL)
		return false;

	///////////////////////
	return true;
}

//描述:通过测量点号写入对应测量点保存变量
//参数:	@wPn:测量点号
//		@pSaveInf:用于传入该电表保存变量的指针
//		@pbUnsupIdFlg:用于传入该电表保存的ID是否支持标志的缓存的指针
bool SaveMtrInfo(WORD wPn, TMtrSaveInf* pSaveInf)
{
	if (pSaveInf == NULL)
		return false;

	///////////////////////
	return true;
}

//描述:创建各电表协议的类变量
//参数:	@wPn:测量点号
//		@pMtrPara:用于输入电表参数的指针
//		@pSaveInf:用于输入电表保存变量的指针
//返回值:成功则返回具体电表协议结构的指针,失败则返回空指针
struct TMtrPro*  CreateMtrPro(WORD wPn, TMtrPara* pMtrPara, TMtrSaveInf* pSaveInf, bool fInit, BYTE bThrId)
{
	//判断协议参数的合理性
	//if (pMtrPara==NULL || pSaveInf==NULL || pbUnsupIdFlg==NULL)
		//return NULL;

	//if ( !LoadMtrInfo(wPn, pMtrPara, pSaveInf) )
//		return NULL;

	g_MtrPro[bThrId].pMtrPara = pMtrPara;	

//	if ( fInit ) //非首次初始化直接返回
//		ResetMtrUnsupIdFlg(pSaveInf->pbUnsupIdFlg, wPn);
	
	switch (pMtrPara->bProId)
	{
	case PROTOCOLNO_DLT645:			
		g_MtrPro[bThrId].pvMtrPro = &pSaveInf->tMtrPriv.t645Priv;			
		if ( Mtr645Init(&g_MtrPro[bThrId], fInit, bThrId) )
			return &g_MtrPro[bThrId];
		break;
	case PROTOCOLNO_DLT645_V07:
		g_MtrPro[bThrId].pvMtrPro = &pSaveInf->tMtrPriv.tV07Priv;	
		if ( Mtr645V07Init(&g_MtrPro[bThrId], fInit, bThrId) )
			return &g_MtrPro[bThrId];	
		break;		
	default:
		break;
	}

	return NULL;
}


//描述:抄读各电表协议数据接口
//参数:	@pMtrPro: 用于输入具体电表协议结构的指针
//		@wPn:测量点号
//		@wID:抄读ID
//		@pbBuf:用于返回抄读数据的指针

int AskMtrItem(struct TMtrPro* pMtrPro, WORD wPn, WORD wID, BYTE* pbBuf)
{
	const WORD wRateBlkId[] ={0xc31f, 0xc32f, 0xc33f}; //目前只抄3个费率ID	
	const WORD wRateBlkLen[] = {5, 42, 42};
	BYTE bBuf[50], bNum, i;	
	BYTE bPrintPro = pMtrPro->pMtrPara->bProId;
	char szProName[20];
	int iRv, iRet=0;
	BYTE* p = pbBuf;


	pMtrPro->pfnGetProPrintType(&bPrintPro, szProName);	
	DTRACE(bPrintPro, ("MtrPro=%s::Point=%d,read id=0x%x.\r\n", szProName,pMtrPro->pMtrPara->wPn, wID)); 

	//如果时段费率,拆分各块读取
	if (wID == 0xc60f)
	{
		bNum = sizeof(wRateBlkId)/sizeof(WORD);
		memset(pbBuf, m_bInvdData, (bNum-1)*42+5);
        //memset(pbBuf, 0, (bNum-1)*42+5);

		for (i=0; i<bNum; i++)
		{	
			//iRv = pMtrPro->pfnAskItem(pMtrPro, wRateBlkId[i], bBuf);
            if (i == 2)//台体测试只抄0xc33f
				iRv = pMtrPro->pfnAskItem(pMtrPro, wRateBlkId[i], bBuf);
			else
				iRv = -1;

			if (iRv > 0)
			{
				memcpy(p, bBuf, iRv);
			    p += iRv;
			}
			else if (i < 2)
			{//不支持的费率ID数据设置为0
				iRv = wRateBlkLen[i];
				memset(bBuf, 0, iRv);
				memcpy(p, bBuf, iRv);
				p += iRv;
			}
			//else 为兼容台体模拟表07协议的c31f和c32f不支持但费率时段支持,故通信异常也不退出继续抄
		}	
		iRet = (bNum-1)*42+5;
	}
	else
		iRet = pMtrPro->pfnAskItem(pMtrPro, wID, pbBuf);

	return  iRet;
}

