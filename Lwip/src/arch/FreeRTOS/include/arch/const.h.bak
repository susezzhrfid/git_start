#ifndef const_h
#define const_h
#define FALSE                   0
#define TRUE                    1
#define  VISUALDSP 

//#define	NO_HZK			1	   //打开即编译不带汉字库的程序
//#define  DEBUG

#define  EXTERN_REF
#define  VOLTAGE_TRANSFER_250V

#define FULL_SPEED				1

#ifdef FULL_SPEED
#define  CLKIN                  16384000L
#define  CCLK                   (CLKIN*20L)
#define  SCLK                   (CCLK/5L)
#define TICKPERMS     			2
#endif

#ifdef SPEED_60_PERCENT
#define  CLKIN                  16384000L
#define  CCLK                   (CLKIN*12L)
#define  SCLK                   (CCLK/3L)
#define  TICKPERMS     			1.2
#endif

#ifdef SPEED_40_PERCENT
#define  CLKIN                  16384000L
#define  CCLK                   (CLKIN*8L)
#define  SCLK                   (CCLK/2L)
#define  TICKPERMS     			0.8
#endif

//#define  USEBTC
//#define  USE_CACHE
//#define  USE_DATACACHE 

//#define  USEBF533EMULATOR

//#define  DEBUG_TANSFER_MEASUREDATA
//#define  DEBUG_TANSFER_ENERGYDATA

//#define    DEBUG_TEST_RATE_STAT
//#define    DEBUF_GET_RATE      //一般和DEBUG_TEST_RATE_STAT一起用


//#define  DEBUG_TRANSFER_DEMAND
#define  DEBUG_MEASURE_STAT

#define   FAR_IRD    //远红外


#endif

/*
   NO_USE_SCRATCH
   
*/

//#pragma disable_warning cc0001
