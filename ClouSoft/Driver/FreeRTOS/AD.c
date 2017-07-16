//����ʵ�ֽ�ֱ������
#include "AD.h"
#include "Sample.h"
#include "adc.h"
#include "board.h"
#include "tc.h"
#include <string.h>
#include "Drivers.h"
#include "SysDebug.h"
#include "LibAcConst.h"
#include "pmc.h"
#include "sysclk.h"

#define    AD_TEST            0
#define    MCK_TIMER_TEST     0

TSem g_semAcDone = NULL;

__no_init static WORD wADValue1[BUFFER_SIZE];//���ݴ��˳��Ϊ��1����1�㣬��2����1��...��1����2�㣬��2����2��
__no_init static WORD wADValue2[BUFFER_SIZE];
__no_init WORD g_wAdBuf[BUFFER_SIZE];

/** ADC test mode structure */
typedef struct _AdcTestMode
{
    uint8_t ucTriggerMode;
    uint8_t ucPdcEn;
    uint8_t ucSequenceEn;
    uint8_t ucGainEn;
    uint8_t ucOffsetEn;
    uint8_t ucPowerSaveEn;
} tAdcTestMode;

/** ADC trigger modes */
typedef enum _TriggerMode
{
    TRIGGER_MODE_SOFTWARE = 0,
    TRIGGER_MODE_ADTRG,
    TRIGGER_MODE_TIMER,
    TRIGGER_MODE_PWM,
    TRIGGER_MODE_FREERUN
} eTriggerMode;

/**Channel list for sequence*/
enum adc_channel_num_t ch_list[] = {
    //ADC_CHANNEL_0,   //������ȥ����AD0�������ݲɼ� Wing 2014/07/28
    ADC_CHANNEL_3,
	//ADC_TEMPERATURE_SENSOR,	
};

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

/** ADC test mode */
static const tAdcTestMode gAdcTestMode = 
{
    TRIGGER_MODE_TIMER,
    1, //ucPdcEn
    0, 
    0,
    0,
    0,
};

/** ADC sample data */
volatile TAdCtrl g_tAdCtrl;

//WORD g_wCurScn = 0;		//��ǰ��������ͨ��
//WORD g_wDCCurScn = 0;	//��ǰֱ������ͨ��
//DWORD g_dwAcWrPtr = 0;	//��ǰ����дָ��
//DWORD g_dwAcRdPtr = 0;	//��ǰ���ɶ�ָ��
volatile DWORD g_dwFreqPtr = 0;	//Ƶ�ʸ��ٵļ���ָ�� 
volatile DWORD g_dwAdFreq = NUM_PER_CYC*50000; //AD����Ƶ��(ͬһͨ�����������)     g_dwAdInterv�ĵ���

volatile BYTE g_bFreqStep = 0; 	//Ƶ�ʸ��ٵĵ�ǰ����:
						//0-���ɼ����������һ����Ƶ��;1-AD�жϲ������µ�Ƶ�ʽ��в���
volatile BYTE g_bFreqSync = 2;   //g_bFreqSync=0ʱ����Ƶ�ʸ��٣��л������������Ҫ�������ܲ��ٽ���Ƶ�ʸ��٣�
//��һ���ܲ�����ΪDMA�ڻ����˾ɵļ�������ɵõĵ㣬�ڶ����ܲ�ǰ���þɵ�Ƶ�ʲ��˼����㡣

volatile BYTE g_bJumpCyc = 0;//����Ƶ�ʸ��µ��������������дFLASHʱͣ���˶�ʱ�������ܲ���ĵ㲻��ȷ����Ҫ������

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

static void adc_pdc_rx_init(Adc * p_adc, WORD * p_s_buffer1, WORD * p_s_buffer2, DWORD ul_size)
{
	/* Check if the first PDC bank is free. */
	p_adc->ADC_RPR = (DWORD) p_s_buffer1;
	p_adc->ADC_RCR = ul_size;
	p_adc->ADC_PTCR = ADC_PTCR_RXTEN;
    /* Check if the second PDC bank is free. */		
    p_adc->ADC_RNPR = (DWORD) p_s_buffer2;
    p_adc->ADC_RNCR = ul_size;
}

static DWORD adc_read_buffer(Adc * p_adc, WORD * p_s_buffer, DWORD ul_size)
{
	/* Check if the first PDC bank is free. */
	if ((p_adc->ADC_RCR == 0) && (p_adc->ADC_RNCR == 0)) 
    {
		p_adc->ADC_RPR = (DWORD) p_s_buffer;
		p_adc->ADC_RCR = ul_size;
		p_adc->ADC_PTCR = ADC_PTCR_RXTEN;

		return 1;
	} 
    else 
    {	/* Check if the second PDC bank is free. */
		if (p_adc->ADC_RNCR == 0) 
        {
			p_adc->ADC_RNPR = (DWORD) p_s_buffer;
			p_adc->ADC_RNCR = ul_size;

			return 1;
		} 
        else 
        {
			return 0;
		}
	}
}

/** TC waveform configurations */
static const struct WaveformConfiguration waveformConfigurations = 
{
    //TC_CMR_TCCLKS_TIMER_CLOCK4, 178, 30,
    //{TC_CMR_TCCLKS_TIMER_CLOCK3, 375, 50},
    //{TC_CMR_TCCLKS_TIMER_CLOCK3, 800, 75},
    TC_CMR_TCCLKS_TIMER_CLOCK1, 50*NUM_PER_CYC/*1600*/, 50,  //ռ�ձ�Ϊ50��50hz��Ƶ��һ���ܲ���32����1600HZ     
    //{TC_CMR_TCCLKS_TIMER_CLOCK1, 4000, 55}
};

/**
 * \brief Configure clock, frequency and dutyclcle in wave mode.
 */
static void _TcWaveformConfigure(void)
{
    const uint32_t dwDivisors[5] = {2, 8, 32, 128, BOARD_MCK / 32768};
    uint32_t dwRA, dwRC;    
    DDWORD ddwAdFreq;
    /*  Set channel 1 as waveform mode*/
    REG_TC0_CMR0 = waveformConfigurations.dwClockSelection  /* Waveform Clock Selection */
                   | TC_CMR_WAVE                                        /* Waveform mode is enabled */
                   | TC_CMR_ACPA_SET                                    /* RA Compare Effect: set */
                   | TC_CMR_ACPC_CLEAR                                  /* RC Compare Effect: clear */
                   | TC_CMR_CPCTRG                                     /* UP mode with automatic trigger on RC Compare */
                   | TC_CMR_WAVSEL_UP_RC; 
    if (g_dwAdFreq == 0)
        dwRC = (BOARD_MCK / dwDivisors[waveformConfigurations.dwClockSelection]) / waveformConfigurations.dwFrequency;
    else
    {                
        ddwAdFreq = ((BOARD_MCK / dwDivisors[waveformConfigurations.dwClockSelection])*10000ULL) / g_dwAdFreq;//�п��ܳ�����������,��Բ�����Ƶ�ʸ������һ�����
        dwRC = (ddwAdFreq+5)/10;//��������
    }
    REG_TC0_RC0 = dwRC;
    dwRA = (100 - waveformConfigurations.wDutyCycle) * dwRC / 100;
    REG_TC0_RA0 = dwRA;
}

/**
 * \brief Configure TC0 channel 1 as waveform operating mode.
 */
static void _TcWaveformInitialize(void)
{
    volatile uint32_t dwDummy;
    /* Configure the PMC to enable the Timer Counter clock for TC0 channel 1. */
    pmc_enable_periph_clk(ID_TC0);
    /*  Disable TC clock */
    REG_TC0_CCR0 = TC_CCR_CLKDIS;
    /*  Disable interrupts */
    REG_TC0_IDR0 = 0xFFFFFFFF;
    /*  Clear status register */
    dwDummy = REG_TC0_SR0;
    /* Configure waveform frequency and duty cycle */
    _TcWaveformConfigure();
    /* Enable TC0 channel 1 */
    //REG_TC0_CCR0 =  TC_CCR_CLKEN | TC_CCR_SWTRG ;/* Start the Timer */

    /*printf ("Start waveform: Frequency = %d Hz,Duty Cycle = %2d%%\n\r",
            waveformConfigurations.dwFrequency,
            waveformConfigurations.wDutyCycle);*/  
    
    /* Set TIOA0 trigger */    
    adc_configure_trigger(ADC, ADC_TRIG_TIO_CH_0, 0);
    
#if MCK_TIMER_TEST  //test mck    
    REG_TC0_IER0 = 0x10;
    NVIC_SetPriority(TC0_IRQn, 10);
    NVIC_EnableIRQ( TC0_IRQn ) ;        
#endif
}

void SetAdFreq(void)
{
    volatile uint32_t dwDummy;
    static bool fAdjFreq = false;
    //if (g_wFreqStep==0 && (g_dwAcWrPtr&0x003f)==0) //0-���ɼ����������һ����Ƶ��
    if (g_bFreqStep == 0)
	{	      
        tc_stop( TC0, 0 ) ;             /* Stop the Timer */
        //_TcWaveformInitialize();
        /*  Clear status register */
        dwDummy = REG_TC0_SR0;
        _TcWaveformConfigure();
        tc_start(TC0, 0) ;
		g_bFreqStep = 1; //1-AD�жϲ������µ�Ƶ�ʽ��в���
		g_dwFreqPtr = 0; //Ƶ�ʸ��ٵļ���ָ��    
        g_bFreqSync = 1;   //��ǰһ������һ�����ݲ����������١�      
        fAdjFreq = true;
	}
    else
    {
        if (fAdjFreq) //������Ƶ��
        {
            fAdjFreq = false;
            g_bJumpCyc = 1;     //�ӵ�һ���ܲ�
        }
        if (g_bFreqSync != 0)
            g_bFreqSync--;
    }
}

/**
 * \brief (Re)Sart ADC sample.
 * Initialize ADC, set clock and timing, set ADC to given mode.
 */
void ADStart( void )
{    
    tc_start(TC0, 0) ;
}

void ADInit(void)
{    
//#if AD_TEST         //for test
    //uint32_t dwTickLast = 0, dwTick = 0;
//#endif    
    
    g_semAcDone = NewSemaphore(0, 1);   
    
    uint32_t dw ;
    
    /* Initialize ADC */
    pmc_enable_periph_clk(ID_ADC);
    adc_init(ADC, sysclk_get_main_hz(), 6000000, 8);    //6M 
    /*
     * Formula: ADCClock = MCK / ( (PRESCAL+1) * 2 )
     * For example, MCK = 84MHZ, PRESCAL = 6, then:
     *     ADCClock = 84 / ((6+1) * 2) = 6MHz;
     */
    /* Set ADC clock */    
    /* Formula:
     *     Startup  Time = startup value / ADCClock
     *     Transfer Time = (TRANSFER * 2 + 3) / ADCClock
     *     Tracking Time = (TRACKTIM + 1) / ADCClock
     *     Settling Time = settling value / ADCClock
     * For example, ADC clock = 6MHz (166.7 ns)
     *     Startup time = 512 / 6MHz = 85.3 us
     *     Transfer Time = (1 * 2 + 3) / 6MHz = 833.3 ns
     *     Tracking Time = (0 + 1) / 6MHz = 166.7 ns
     *     Settling Time = 3 / 6MHz = 500 ns
     */
    /* Set ADC timing */
    adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_3, 1);
    
    /* Enable channel number tag. */
	//adc_enable_tag(ADC);   //LCDR���ͨ����

    /* Enable/disable sequencer */
    if ( gAdcTestMode.ucSequenceEn )
    {
        /* Set user defined channel sequence. */
		adc_configure_sequence(ADC, ch_list, 1); //��������ҪҪ����  Wing 2014/07/28 2);

		/* Enable sequencer. */
		adc_start_sequencer(ADC);

        /* Enable channels */     
        for (dw=0; dw<NUM_CHANNELS; dw++)
        {
            adc_enable_channel( ADC, (enum adc_channel_num_t)dw ) ;
        }          
    }
    else
    {
        /* Disable sequencer */
        adc_stop_sequencer(ADC);

        /* Enable  channels */   //���������˳��ʱ������һ���ʹ���һ��
        /*for (dw=0; dw<NUM_CHANNELS; dw++)
        {
            adc_enable_channel( ADC, ADC_CHANNEL_0+dw ) ;
        } */
        //adc_enable_channel( ADC, ADC_CHANNEL_0 ) ;  //��������Ҫ���� Wing 2014/07/28 
        adc_enable_channel( ADC, ADC_CHANNEL_3 ) ;
        //adc_enable_channel(ADC, ADC_TEMPERATURE_SENSOR);   //Ӱ����PB27��Ϊͨ��IO��
    }
    /* Enable the temperature sensor (CH15). */
	adc_enable_ts(ADC);

    /* Set gain and offset (only single ended mode used here) */
    adc_disable_anch(ADC);	/* Disable analog change. */
    if ( gAdcTestMode.ucGainEn  )
    {
        adc_enable_anch(ADC);
        //adc_set_channel_input_gain(ADC, ADC_CHANNEL_0, ADC_GAINVALUE_2);/* gain = 2 *///��������Ҫ���� Wing 2014/07/28 
        adc_set_channel_input_gain(ADC, ADC_CHANNEL_3, ADC_GAINVALUE_2);
    }
    else
    {
        //adc_set_channel_input_gain(ADC, ADC_CHANNEL_0, ADC_GAINVALUE_0); /* gain = 1 *///��������Ҫ���� Wing 2014/07/28 
        adc_set_channel_input_gain(ADC, ADC_CHANNEL_3, ADC_GAINVALUE_0);
    }

    if ( gAdcTestMode.ucOffsetEn )
    {        
        adc_enable_anch(ADC);
		//adc_enable_channel_input_offset(ADC, ADC_CHANNEL_0);//��������Ҫ���� Wing 2014/07/28 
        adc_enable_channel_input_offset(ADC, ADC_CHANNEL_3);
    }
    else
    {        
        //adc_disable_channel_input_offset(ADC, ADC_CHANNEL_0);//��������Ҫ���� Wing 2014/07/28 
        adc_disable_channel_input_offset(ADC, ADC_CHANNEL_3);
    }

    /* Set power save */
    /* Note: SLEEP mode doesn't work in engineer samples. */
    if ( gAdcTestMode.ucPowerSaveEn )
    {
        adc_configure_power_save(ADC, 1, 0);
    }
    else
    {
        adc_configure_power_save(ADC, 0, 0);
    }
    /* Transfer with/without PDC */
    if ( gAdcTestMode.ucPdcEn )
    {
        //adc_read_buffer( ADC, wADValue1, BUFFER_SIZE) ;        //RPR
        //adc_read_buffer( ADC, wADValue2, BUFFER_SIZE) ;        //RNPR
        adc_pdc_rx_init(ADC, wADValue1, wADValue2, BUFFER_SIZE);
        
        /* Enable PDC channel interrupt */
        adc_enable_interrupt( ADC, ADC_IER_RXBUFF ) ;
    }
    else
    {
        /* Enable Data ready interrupt */
        //adc_enable_interrupt( ADC, ADC_IER_DRDY ) ;   //��LCDR���Զ����DRDY
        adc_enable_interrupt( ADC, 1<<(NUM_CHANNELS-1)) ;//ADC_IER_EOC0-ADC_IER_EOC10
    }
    
    adc_enable_interrupt( ADC, ADC_IER_GOVRE ) ; //���Ҳ�ж���
    adc_enable_interrupt( ADC, ADC_IER_ENDRX ) ; //RCR����0���ж�
    
    NVIC_SetPriority(ADC_IRQn, 7);
    
    /* Enable ADC interrupt */
    NVIC_EnableIRQ( ADC_IRQn ) ;
    
    memset( (void*)&g_tAdCtrl, 0, sizeof( g_tAdCtrl ) );
    adc_get_status(ADC);

    /* Configure trigger mode and start convention */
    switch ( gAdcTestMode.ucTriggerMode )
    {
        case TRIGGER_MODE_SOFTWARE:
            adc_configure_trigger(ADC, ADC_TRIG_SW, 0);	/* Disable hardware trigger. */
        break ;
        case TRIGGER_MODE_ADTRG:
            //Gpio_Config( &gPinADTRG, 1 ) ;
            adc_configure_trigger(ADC, ADC_TRIG_EXT, 0);
        break ;
        case TRIGGER_MODE_TIMER :
            _TcWaveformInitialize();
        break ;
        case TRIGGER_MODE_PWM :
            //_ConfigurePwmTrigger() ;
        break ;
        case TRIGGER_MODE_FREERUN :
            //ADC_SetFreeRunMode( ADC, 1 ) ;
        break ;
        default :
        break ;
    }
#if 0    
//#if AD_TEST         //for test
    ADStart();
    while (1)
    {
        /* ADC software trigger per 1s */
        dwTick = GetTick();
        if ( gAdcTestMode.ucTriggerMode == TRIGGER_MODE_SOFTWARE)
        {
            if ( (dwTick - dwTickLast) > 1000 )
            {
                dwTickLast = dwTick;
                ADC_StartConversion(ADC);        //ÿ�δ�����ֻת��һ��ͨ��������������ͨ��ת�����һ��
            }
        }        
    }    
#endif    
}

void ADStop(void)
{
    tc_stop( TC0, 0 ) ;             /* Stop the Timer */
}

void ADC_Handler(void)
{
    __disable_interrupt();
    //uint32_t i;
    uint32_t dwStatus;
    volatile uint32_t dwOver;
    uint8_t  ucGroupDone = 0;    
    
#if AD_TEST   //���Բ���������ʱ��Ƶ��
    static unsigned char fTestLed = 0;   
    if (fTestLed)
        SetLed(true, 1);   //LED_LINE_ERR
    else
        SetLed(false, 1);//LED_LINE_ERR
    fTestLed = !fTestLed;       
#endif        

    //ADStop();
    
    SetAdFreq();//Ƶ�ʸ������һ����Ƶ�ʣ���������Ƶ�ʲ���,�´εõ������ݲ�����Ƶ�ʵġ�
        
    dwStatus = adc_get_status(ADC);
    dwOver = ADC->ADC_OVER;
/*     
    if ( !gAdcTestMode.ucPdcEn )
    {
        if (dwOver & (~(0xFFFFFFFF<<NUM_CHANNELS)))//���       ע�⣬�ϵ��Ӱ����������Ǵ���Բ���
        {                    
            for (i = 0; i < NUM_CHANNELS; i++)
            {
                if (dwOver & (1<<(NUM_CHANNELS-1)))
                {                    
                    adc_get_channel_value(ADC, (enum adc_channel_num_t)i);
                }
            }    
            //DTRACE(1, ("AD GOVER!\r\n"));//��ӡ��Ҫ���ź������ж��ﲻ�ܵ��ź���       
            return;
        } 
    }*/    
     
    /* With PDC transfer */ //����ͨ������64���ж�һ��,ʵ�����ǻ����������ж�
    if ( gAdcTestMode.ucPdcEn )
    {
        if ( (dwStatus & ADC_ISR_ENDRX) == ADC_ISR_ENDRX )//1�����ջ�������
        {     
            if (g_tAdCtrl.fCurAdBuf1 == false)
            {
                adc_read_buffer( ADC, wADValue1, BUFFER_SIZE) ;  //���¸�PDC RNPRһ������ݵĵ�ַ                
                g_tAdCtrl.fCurAdBuf1 = true;
                g_tAdCtrl.pwData = wADValue1;
            }
            else
            {
                adc_read_buffer( ADC, wADValue2, BUFFER_SIZE) ;  //���¸�PDC RNPRһ������ݵĵ�ַ                
                g_tAdCtrl.fCurAdBuf1 = false;
                g_tAdCtrl.pwData = wADValue2;
            }                
            ucGroupDone = 1;         
        }
        else //if ((dwStatus & ADC_ISR_RXBUFF) == ADC_ISR_RXBUFF ) //�����2��������������      ���뱣֤������жϻ�����������
        {
            adc_read_buffer( ADC, wADValue1, BUFFER_SIZE) ;        //RPR
            adc_read_buffer( ADC, wADValue2, BUFFER_SIZE) ;        //RNPR
        
            memset( (void*)&g_tAdCtrl, 0, sizeof( g_tAdCtrl ) );
        }
    }

    // Without PDC transfer   //����ͨ�������ж�һ��
/*    if ( !gAdcTestMode.ucPdcEn )
    {
        for (i = 0; i < NUM_CHANNELS; i++)
        {
            if (dwStatus & (1<<(NUM_CHANNELS-1)))
            {
                if (g_tAdCtrl.fCurAdBuf1 == false) 
                {
                    wADValue2[g_tAdCtrl.bPnCnt+i] = adc_get_channel_value(ADC, (enum adc_channel_num_t)i);
                }
                else
                {
                    wADValue1[g_tAdCtrl.bPnCnt+i] = adc_get_channel_value(ADC, (enum adc_channel_num_t)i);
                }
            }
        }
        
        g_tAdCtrl.bPnCnt++;
        if (g_tAdCtrl.bPnCnt >= FFT_NUM)
        {
            g_tAdCtrl.bPnCnt = 0;
            ucGroupDone = 1;
            if (g_tAdCtrl.fCurAdBuf1 == false) 
                g_tAdCtrl.pwData = wADValue2;
            else
                g_tAdCtrl.pwData = wADValue1;
            g_tAdCtrl.fCurAdBuf1 = !g_tAdCtrl.fCurAdBuf1;
        }
    }*/
    
    if (ucGroupDone == 1)
    {
        SignalSemaphoreFromISR(g_semAcDone);//����һ���ź�������ʾ����ͨ��һ�β�����ɡ�        
    }  
    
    //ADStart();
    __enable_interrupt();
    
    return;
}

#if MCK_TIMER_TEST   //������Ƶ
void TC0_Handler(void)
{
    volatile uint32_t dwStatus ;
    dwStatus = REG_TC0_SR0 ;   //�������״̬�������жϱ�־���ܱ�������������жϲ�ͣ�ز�����
    
    static unsigned char fTest1Led = 0;   
    if (fTest1Led)
        SetLed(true, 2);   //LED_WIRELESS_NET
    else
        SetLed(false, 2);  //LED_WIRELESS_NET
    fTest1Led = !fTest1Led;
}
#endif
