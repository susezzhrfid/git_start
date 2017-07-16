#ifndef DCSAMPLE_H
#define DCSAMPLE_H

#include "ComAPI.h"

#define DC_CHANNEL_NUM		1//3��ֱ������ͨ��   //Note:����������ֵ������Ҫͬʱ����0x8961�ĳ���

#define  MAX_CALIB_PT_NUM      5  //֧�ֵ����У׼����  

bool DCSmpleInit();
void DoDCSample();

//bool SetAdjustVector(BYTE* bpTr);
//void CalcAdjVector(BYTE* bpTr);
BYTE GetDCChannel(BYTE bCurScn);

void SaveSampleData();
bool GetADVector();
bool AdjustValue(BYTE bChannel);
//bool GetAdjustVector();
void SaveAdjustVector();

typedef struct {    
    int iDcValue[DC_CHANNEL_NUM];
    WORD wDCVector[DC_CHANNEL_NUM];
    
    //int iDcChannel;
}TDcSample;

#endif
