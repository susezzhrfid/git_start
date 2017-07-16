/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件主要实现对交流采样参数的装载和保存
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
#include "AcSample.h"
#include "Sample.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "LibDbAPI.h"

//extern	TSHJPara g_tSHJPara;	//Short-Circuit Judgement parameters

//描述：加载相间短路故障参数
void LoadSHJPara(WORD wPn, TSHJPara *pSHJPara)
{
	if (pSHJPara == NULL)
		return;
/*
	BYTE bBuf[128];

故障电流定值
无流定值
无流时长
第一故障脉冲最小时长
第一故障脉冲最大时长
第二故障脉冲最大时长
零序电流突变量定值
零序电流越限告警阀值
零序电压越限告警阀值
零序电流越限发生/恢复时间间隔
零序电压越限发生/恢复时间间隔
断相电压阀值
断相电流阀值
断相遥信持续时间
*/
	/*ReadItemEx(BN0, PN0, 0x895f, bBuf);
    memset(bBuf, 0, sizeof(bBuf));
    
	pSHJPara->dwIh = BcdToDWORD(bBuf, 4);//故障电流定值
	pSHJPara->dwIno = BcdToDWORD(bBuf+4, 4);//无流定值
	pSHJPara->wT2 = BcdToDWORD(bBuf+8, 2);//重合闸时间整定值
	pSHJPara->wT1Min = BcdToDWORD(bBuf+10, 2);//过流时间整点值(最小值)
	pSHJPara->wT1Max = BcdToDWORD(bBuf+12, 2);//过流时间整点值(最大值)
	pSHJPara->wT3 = BcdToDWORD(bBuf+14, 2);//后加速时间整定值
	pSHJPara->dwI0 = BcdToDWORD(bBuf+16, 4);//零序电流突变量整定值
    pSHJPara->dwZeroI = BcdToDWORD(bBuf+20, 4);//零序电流整定值
    if (pSHJPara->dwZeroI == 0)
    	pSHJPara->dwZeroI = 1000;

    pSHJPara->dwZeroU = BcdToDWORD(bBuf+24, 4);//零序电压整定值
    pSHJPara->wInOverTime = BcdToDWORD(bBuf+28, 2);//零序电流过流报警延迟时间
    pSHJPara->wUnOverTime = BcdToDWORD(bBuf+30, 2);//零序电压过流报警延迟时间
    pSHJPara->dwUOff = BcdToDWORD(bBuf+32, 4);//断相电压阀值
    if (pSHJPara->dwUOff == 0)
        pSHJPara->dwUOff = 20*100;

    pSHJPara->dwIOff = BcdToDWORD(bBuf+36, 4);//断相电流阀值
    if (pSHJPara->dwIOff == 0)
        pSHJPara->dwIOff = 5*100;
    pSHJPara->wOffTime = BcdToDWORD(bBuf+40, 2);//断相持续时间
	if (pSHJPara->wOffTime == 0)
		pSHJPara->wOffTime = 1;

	if (pSHJPara->wT1Min <= 20) 
	{
		pSHJPara->wT1Min = 250;
	}
	if (pSHJPara->dwIh < 50) 
	{
		pSHJPara->dwIh = 5*1000;
	}*/
}

//交采参数的初始化
bool AcLoadPara(WORD wPn, TAcPara* pAcPara)
{

	//WORD i;
	BYTE bBuf[64];
	//int iRet;

	memset(pAcPara, 0, sizeof(TAcPara));
	memset(bBuf, 0, sizeof(bBuf));
	pAcPara->wPoint = wPn;

	//工作模式
	/*ReadItemEx(BN0, PN0, 0x8903, bBuf);  //工作模式:0:深圳模式;1：广州模式;2:扬州模式；其它：深圳模式
	pAcPara->wWorkMode = bBuf[0];*/
    //pAcPara->wWorkMode = 0;
/*
交采基本参数
额定电压
额定电流
电压合格上限
电压合格下限
三相电压不平衡限值
三相电压不平衡限值
频率上限
频率下限
总畸变电压上限
总畸变电流上限
接线方式                                                    
相角方向1表示角度按照顺时针方向表示,Ua,Ub,Uc分别为0,120,240 */

	/*ReadItemEx(BN0, PN0, 0x894f, bBuf);
    memset(bBuf, 0, sizeof(bBuf));
    
	pAcPara->dwUn = BcdToDWORD(&bBuf[0], 4);
	if (pAcPara->dwUn == 0)
		pAcPara->dwUn = 22000;

	pAcPara->dwIn = BcdToDWORD(&bBuf[4], 4);
	if (pAcPara->dwIn == 0)
		pAcPara->dwIn = 5000;

	pAcPara->bConnectType = bBuf[40];	//接线方式
    if ((pAcPara->bConnectType != CONNECT_3P3W) || (pAcPara->bConnectType != CONNECT_1P)) 
        pAcPara->bConnectType = CONNECT_3P4W;

	pAcPara->bAngleClockwise = bBuf[41]; //1表示角度按照顺时针方向表示,Ua,Ub,Uc分别为0,120,240

	if (pAcPara->wWorkMode == 0) 
	{
		pAcPara->fCalcuHarmonic = true;
		pAcPara->wHarmNum = 15;
	} 
	else 
	{
		pAcPara->fCalcuHarmonic = false;
		pAcPara->wHarmNum = 0;
	}
*/
    pAcPara->fCalcuHarmonic = false;
    pAcPara->wHarmNum = 0;
	//pAcPara->wWorkMode = 6;   //TODO:根据库里的配置初始化，这里只是为了测试方便
	pAcPara->bConnectType = CONNECT_3P4W; 
	//LoadSHJPara(wPn, &g_tSHJPara);

	return true;
}

