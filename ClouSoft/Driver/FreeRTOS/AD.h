#ifndef _AD_H
#define _AD_H

#include "Typedef.h"
#include "SAM3XA.h"
#include "Sysarch.h"
#include "LibAcConst.h"
#include "DCSample.h"

/*
 * We use two ADC channel for this example:
 *    ADC_CHANNEL_1  (potentiometer)
 *    ADC_CHANNEL_15 (temperature sensor)
 */
/** Total number of ADC channels in use */
#define NUM_CHANNELS    (SCN_NUM + DC_CHANNEL_NUM)//(10)
/** ADC convention done mask */
#define ADC_DONE_MASK   ( (1<<NUM_CHANNELS) - 1 )

/** Size of the receive buffer and transmit buffer. */
#define BUFFER_SIZE     ( NUM_CHANNELS * CYC_NUM * NUM_PER_CYC )  //在计算时，只算了第一个周期,其余丢掉。这样做可以减少负荷。

/** Reference voltage for ADC, in mv */
#define VOLT_REF        (3300)//(3300)

/** The maximal digital value */
#define MAX_DIGITAL     (4095)

extern WORD g_wAdBuf[BUFFER_SIZE];

typedef struct 
{         
    bool fCurAdBuf1;                             //当前正在采样的缓存1
    //bool fAdBufBusy; 
    WORD *pwData;
    
    BYTE bPnCnt;     //for Without PDC
}TAdCtrl;

extern volatile TAdCtrl g_tAdCtrl;
extern TSem g_semAcDone;

void ADInit(void);
void ADStart(void);
void ADStop(void);

void SetAdFreq(void);

extern volatile DWORD g_dwFreqPtr;
extern volatile DWORD g_dwAdFreq;
extern volatile BYTE g_bFreqStep;
extern volatile BYTE g_bFreqSync;
extern volatile BYTE g_bJumpCyc;

#endif
