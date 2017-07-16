#ifndef DCSAMPLE_H
#define DCSAMPLE_H

#include "ComAPI.h"

#define DC_CHANNEL_NUM		1//3个直流采样通道   //Note:调整这三个值，就需要同时调整0x8961的长度

#define  MAX_CALIB_PT_NUM      5  //支持的最大校准点数  

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
