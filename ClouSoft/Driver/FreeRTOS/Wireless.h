#ifndef WIRELESS_H
#define WIRELESS_H

#include "Typedef.h"



//extern TTxCnt g_tTxCnt;
//extern TRxCnt g_tRxCnt;

extern TTxState g_tTxState;
extern TRxState g_tRxState;

extern BYTE g_bRadioMode;

extern const BYTE g_bPower[][2];
extern const BYTE g_bFreqGrp[][2];

void RfInit(void);
BYTE RfSendPacket(BYTE *pbTxBuf, BYTE bLen);
void RfRecvPacket(void);
void RfSetFreq(BYTE bChannel);
int GetRSSI(void);

#endif