/*********************************************************************************************************
 * Copyright (c) 2005,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件对交流采样的公共变量、常量、函数进行定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2005年11月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef SAMPLE_H
#define SAMPLE_H
//#include "syscfg.h"
//#include "bios.h"
//#include "sysarch.h"
#include "AcConst.h"
#include "AcStruct.h"
#include "DCSample.h"
#include "AcSample.h"
#include "AD.h"

extern bool g_fDcSample;
//extern BYTE g_bDcSampleFlag;

bool InitAcSample();
bool InitDcSample();
bool InitRatePeriod(WORD wPn);
WORD GetRate(WORD wPn);
BYTE GetConnectType(WORD wPn);

WORD CaluPhaseStatus();
void CalcuDcValue();

//TThreadRet AcThread(void* pvPara);
   
//校准点
//#pragma pack(1)
typedef struct
{
    WORD wAD;           //AD值,或交流有效值
    WORD wVal;          //实际值
}TCalibPt;

//系统校准配置
typedef struct
{    
    BYTE  bCalibPtNum[SCN_NUM+DC_CHANNEL_NUM];   //校准点数
    TCalibPt  tCalibTab[SCN_NUM+DC_CHANNEL_NUM][MAX_CALIB_PT_NUM]; //校正样本点
}TCalibCfg;  

extern TCalibCfg g_tCalibCfg;

long ConvertToLineVal(BYTE bType, long lAdVal);
void LoadCalibCfg(void);
int OnCalibrate(BYTE* pbBuf);

bool IsAcCalib(void);

#endif //SAMPLE_H
