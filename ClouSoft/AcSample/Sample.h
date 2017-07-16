/*********************************************************************************************************
 * Copyright (c) 2005,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ��Խ��������Ĺ����������������������ж���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2005��11��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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
   
//У׼��
//#pragma pack(1)
typedef struct
{
    WORD wAD;           //ADֵ,������Чֵ
    WORD wVal;          //ʵ��ֵ
}TCalibPt;

//ϵͳУ׼����
typedef struct
{    
    BYTE  bCalibPtNum[SCN_NUM+DC_CHANNEL_NUM];   //У׼����
    TCalibPt  tCalibTab[SCN_NUM+DC_CHANNEL_NUM][MAX_CALIB_PT_NUM]; //У��������
}TCalibCfg;  

extern TCalibCfg g_tCalibCfg;

long ConvertToLineVal(BYTE bType, long lAdVal);
void LoadCalibCfg(void);
int OnCalibrate(BYTE* pbBuf);

bool IsAcCalib(void);

#endif //SAMPLE_H
