#include "Infrared.h"
#include "board.h"
#include "Gpio.h"
#include "pmc.h"
#include "tc.h"

#define IFR_38K_TEST    0

//#define IFR_38K  {PIO_PD8, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define IFR_38K  {PIO_PA3,  PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}//在线灯

static const Pin PIfr38k = IFR_38K;

/*----------------------------------------------------------------------------
 *        Types
 *----------------------------------------------------------------------------*/
/** Describes a possible Timer configuration as waveform mode */
struct WaveformConfiguration {
    /** Internal clock signals selection. */
    uint32_t dwClockSelection;
    /** Waveform frequency (in Hz). */
    uint32_t dwFrequency;
    /** Duty cycle in percent (positive)*/
    uint16_t wDutyCycle;
};

/** TC waveform configurations */
static const struct WaveformConfiguration waveformConfigurations = 
{
    //{TC_CMR_TCCLKS_TIMER_CLOCK4, 178, 30},
    //{TC_CMR_TCCLKS_TIMER_CLOCK3, 375, 50},
    //{TC_CMR_TCCLKS_TIMER_CLOCK3, 800, 75},
   // TC_CMR_TCCLKS_TIMER_CLOCK1, 38400, 50,  //占空比为50，红外38.4K调制信号  
    TC_CMR_TCCLKS_TIMER_CLOCK1, 38400, 50,  //占空比为50，红外38.4K调制信号   
    //{TC_CMR_TCCLKS_TIMER_CLOCK1, 4000, 55}
};

/**
 * \brief Configure clock, frequency and dutyclcle in wave mode.
 */
static void TcWaveformConfigure(void)
{
    const uint32_t dwDivisors[5] = {2, 8, 32, 128, BOARD_MCK / 32768};
    uint32_t dwRB, dwRC;    
    
    /*  Set channel 1 as waveform mode*/
    REG_TC0_CMR1 = waveformConfigurations.dwClockSelection  /* Waveform Clock Selection */
                   | TC_CMR_WAVE                                        /* Waveform mode is enabled */
                   | TC_CMR_BCPB_SET                                    /* RB Compare Effect: set */
                   | TC_CMR_BCPC_CLEAR                                  /* RC Compare Effect: clear */
                   | TC_CMR_CPCTRG                                     /* RC Compare Trigger Enable */
                   | TC_CMR_WAVSEL_UP_RC                               /*UP mode with automatic trigger on RC Compare*/
                   | TC_CMR_EEVT_XC0;     //EEVT不能是0，否则TIOB被配置成了触发输入
    
    dwRC = (BOARD_MCK / dwDivisors[waveformConfigurations.dwClockSelection]) / waveformConfigurations.dwFrequency;
    
    REG_TC0_RC1 = dwRC;
    dwRB = (100 - waveformConfigurations.wDutyCycle) * dwRC / 100;
    REG_TC0_RB1 = dwRB;
}

/**
 * \brief Configure TC2 channel 1 as waveform operating mode.
 */
static void TcWaveformInitialize(void)
{
    volatile uint32_t dwDummy;
    /* Configure the PMC to enable the Timer Counter clock for TC2 channel 1. */
    pmc_enable_periph_clk(ID_TC1);
    /*  Disable TC clock */
    REG_TC0_CCR1 = TC_CCR_CLKDIS;
    /*  Disable interrupts */
    REG_TC0_IDR1 = 0xFFFFFFFF;
    /*  Clear status register */
    dwDummy = REG_TC0_SR1;
    /* Configure waveform frequency and duty cycle */
    TcWaveformConfigure();
    
#if IFR_38K_TEST  //test mck    
    REG_TC0_IER1 = 0x10;                //RC比较中断
    NVIC_EnableIRQ( TC1_IRQn ) ;        
#endif
    
    /* Enable TC2 channel 1 */
    REG_TC0_CCR1 =  TC_CCR_CLKEN | TC_CCR_SWTRG ;/* Start the Timer */    
}

#if IFR_38K_TEST   //测试
void TC8_Handler(void)
{
    volatile uint32_t dwStatus ;
    dwStatus = REG_TC0_SR1 ;   //必须读出状态，否则中断标志不能被清除，将导致中断不停地产生。
    
    static unsigned char fTest1Led = 0;   
    if (fTest1Led)
		Gpio_Set(&PIfr38k);    //高电平熄灭
	//SetLed(true, 2);   //LED_WIRELESS_NET
    else
		Gpio_Clear(&PIfr38k);
    //SetLed(false, 2);  //LED_WIRELESS_NET
    fTest1Led = !fTest1Led;
}
#endif

bool IfrInit(void)  //38.4K信号可以只有发送时才打开。
{
    Gpio_Config(&PIfr38k, 1);
    
    TcWaveformInitialize();
        
    return true;
}
