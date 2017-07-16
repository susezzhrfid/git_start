/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Drivers.c
 * ժ    Ҫ���������API
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#include "SysDebug.h"
#include "Drivers.h"
#include "Sysarch.h"
#include "uarts.h"
#include "DrvCfg.h"
#include "FaCfg.h"
#include "FlashIf.h"
#include "SpiBus.h"
#include "rtc.h"
#include "AD.h"
#include "Rtc8025t.h"
#include "Gpio.h"
#include "Infrared.h"
#include "EsamCmd.h"
#include "pmc.h"
#include "trng.h"
//#include "Lcd.h"


//���㾲��ʵ��,IO�ڳ�ʼ������
//start
#define PIN_HANG_PC1             {PIO_PC1,   PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC2             {PIO_PC2,   PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC4             {PIO_PC4,   PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC5             {PIO_PC5,   PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC6             {PIO_PC6,   PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC8             {PIO_PC8,   PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC9             {PIO_PC9,   PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC10            {PIO_PC10,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC11            {PIO_PC11,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC12            {PIO_PC12,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC13            {PIO_PC13,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC20            {PIO_PC20,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC21            {PIO_PC21,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC22            {PIO_PC22,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC23            {PIO_PC23,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC24            {PIO_PC24,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC25            {PIO_PC25,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC26            {PIO_PC26,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC27            {PIO_PC27,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PC28            {PIO_PC28,  PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}//����

#define PIN_HANG_PD0	         {PIO_PD0,   PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PD6	         {PIO_PD6,   PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PD8             {PIO_PD8,   PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PD9             {PIO_PD9,   PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PD10            {PIO_PD10,  PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}//����


#define PIN_HANG_PA2             {PIO_PA2,   PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA4             {PIO_PA4,   PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA5             {PIO_PA5,   PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA7             {PIO_PA7,   PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA16            {PIO_PA16,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA17            {PIO_PA17,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA18            {PIO_PA18,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA19            {PIO_PA19,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA21            {PIO_PA21,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA22            {PIO_PA22,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA23            {PIO_PA23,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PA24            {PIO_PA24,  PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}//����




#define PIN_HANG_PB10            {PIO_PB10,  PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PB11            {PIO_PB11,  PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PB14            {PIO_PB14,  PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PB15            {PIO_PB15,  PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PB22            {PIO_PB22,  PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PB24            {PIO_PB24,  PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//����
#define PIN_HANG_PB26            {PIO_PB16,  PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//����
//end



//#define PIN_LED_RUN            {PIO_PC17, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
//#define PIN_LED_ALARM          {PIO_PB27, PIOB, ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_RUN            {PIO_PC1,  PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}//���ߵ�
#define PIN_LED_ALARM          {PIO_PC14, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}

#define PIN_LED_ONLINE    	   {PIO_PC17, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}//���ߵ�

#define PIN_LED_SIGNAL1G       {PIO_PC29, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}//�ź�ǿ�ȵ� G�̵�
#define PIN_LED_SIGNAL2R       {PIO_PC30, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}           //R���
#define PIN_LED_LOCAL_TX       {PIO_PC18, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_LOCAL_RX       {PIO_PC19, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_REMOTE_TX      {PIO_PC16, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT} //Զ��
#define PIN_LED_REMOTE_RX      {PIO_PC15, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}

#define PIN_GPRS_IGT           {PIO_PD3,  PIOD, ID_PIOD, PIO_OUTPUT_1, PIO_DEFAULT} //ON/OFF
#define PIN_GPRS_POWER         {PIO_PD2,  PIOD, ID_PIOD, PIO_OUTPUT_1, PIO_DEFAULT} //��Դ��������
#define PIN_GPRS_HOT_CTRL      {PIO_PC28, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}//GPRSģ����ȿ���

//#define PIN_PHY_RESET          {PIO_PA5,  PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT} 

#define PIN_HALL_DETECT        {PIO_PD9,  PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT} //�ų����

#if 1
#define PIN_GPRS_RESET         {PIO_PD7,  PIOD, ID_PIOD, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_STATE1             {PIO_PA1,  PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT} //ģ������ʶ��
#define PIN_STATE2             {PIO_PA0,  PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT} 
#define PIN_STATE3             {PIO_PD6,  PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT} 
#elif 0
#define PIN_STATE3             {PIO_PD7,  PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}
#define PIN_GPRS_RESET         {PIO_PD6,  PIOD, ID_PIOD, PIO_OUTPUT_1, PIO_DEFAULT} 
#define PIN_STATE2             {PIO_PA1,  PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT} //ģ������ʶ��
#define PIN_STATE1             {PIO_PA0,  PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT} 

#endif

//ң��
#define PIN_YX_1               {PIO_PA15, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}

#define PIN_WDG                {PIO_PB16, PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT} //Ӳ�����Ź�

//��ѹ�ͼ��
#define PIN_POWER_LOW          {PIO_PA20,  PIOA, ID_PIOA, PIO_INPUT,    PIO_DEFAULT}  //��ѹ����3.3Vʱ������͵�ƽ
#define PIN_POWER_OFF          {PIO_PC7,   PIOC, ID_PIOC, PIO_INPUT,    PIO_DEFAULT}  //��������������͵�ƽ
#define PIN_POWER_EN           {PIO_PC3,   PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}  //3V3��Դʹ�ܽţ�����ߵ�ƽʹ��

//�ز���λ����
#define PIN_PLC_RESET		   {PIO_PB19,  PIOB, ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT} //�ز���λ
#define PIN_PLC_POWER          {PIO_PB18,  PIOB, ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT} //�ز���Դ����

#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

static const Pin g_LEDs[] = 
{
    PIN_LED_RUN, 
    PIN_LED_ONLINE, 
    PIN_LED_ALARM,
    PIN_LED_SIGNAL1G,
    PIN_LED_SIGNAL2R,
    PIN_LED_LOCAL_TX,
    PIN_LED_LOCAL_RX,
    PIN_LED_REMOTE_TX,
    PIN_LED_REMOTE_RX,
};

static const Pin g_GpioYXs[] = 
{
    PIN_YX_1,    
};

//static const Pin PinPhyReset = PIN_PHY_RESET;

//������ƶ���
static const Pin PinPlcReset = PIN_PLC_RESET;
static const Pin PinPlcPWR = PIN_PLC_POWER;

static const Pin PinGprsPWR = PIN_GPRS_POWER;
static const Pin PinGprsIGT = PIN_GPRS_IGT;
static const Pin PinGprsReset = PIN_GPRS_RESET;

static const Pin PinWdg = PIN_WDG;
static const Pin PinMagnetic = PIN_HALL_DETECT;

static const Pin g_States[] =
{
    PIN_STATE1,
    PIN_STATE2,
    PIN_STATE3,
};

static const Pin g_PinPowerMgrs[] = 
{
    PIN_POWER_LOW,
    PIN_POWER_OFF,
    PIN_POWER_EN,
};

//���㾲��ʵ�� wing 2014/08/12
static const Pin g_HangPin[] =
{
	PIN_HANG_PC1 , 
	PIN_HANG_PC2 , 
	PIN_HANG_PC4 , 
	PIN_HANG_PC5 , 
	PIN_HANG_PC6 , 
	PIN_HANG_PC8 , 
	PIN_HANG_PC9 , 
	PIN_HANG_PC10, 
	PIN_HANG_PC11, 
	PIN_HANG_PC12, 
	PIN_HANG_PC13, 
	PIN_HANG_PC20, 
	PIN_HANG_PC21, 
	PIN_HANG_PC22, 
	PIN_HANG_PC23, 
	PIN_HANG_PC24, 
	PIN_HANG_PC25, 
	PIN_HANG_PC26, 
	PIN_HANG_PC27, 
	PIN_HANG_PC28, 
	PIN_HANG_PD0,  
	PIN_HANG_PD6,  
	PIN_HANG_PD8 , 
	PIN_HANG_PD9 , 
	PIN_HANG_PD10, 
	PIN_HANG_PA2 , 
	PIN_HANG_PA4 , 
	PIN_HANG_PA5 , 
	PIN_HANG_PA7 , 
	PIN_HANG_PA16, 
	PIN_HANG_PA17, 
	PIN_HANG_PA18, 
	PIN_HANG_PA19, 
	PIN_HANG_PA21, 
	PIN_HANG_PA22, 
	PIN_HANG_PA23, 
	PIN_HANG_PA24, 
	PIN_HANG_PB10, 
	PIN_HANG_PB11, 
	PIN_HANG_PB14, 
	PIN_HANG_PB15, 
	PIN_HANG_PB22, 
	PIN_HANG_PB24, 
	PIN_HANG_PB26, 
};

void GpioHangInit(void)
{
	WORD wID = 0;
	Gpio_Config(g_HangPin, ARRAY_SIZE(g_HangPin));
	for(wID = 0; wID < ARRAY_SIZE(g_HangPin); wID++)
	{
		Gpio_Clear(&g_HangPin[wID]);   
	}
}


void GpioPowerMgr(void)
{
    Gpio_Config(g_PinPowerMgrs, ARRAY_SIZE(g_PinPowerMgrs));
}

void GpioWdgInit(void)
{
    Gpio_Config(&PinWdg, pdTRUE);    
}

void ClearHardWdg(void)
{
    if (Gpio_Get(&PinWdg))
        Gpio_Clear(&PinWdg);
    else
        Gpio_Set(&PinWdg);
}

//fOn: true����  false�ص�
//bID: Ӧ�ò�����
void SetLed(bool fOn, BYTE bID)
{
	if (fOn)
        Gpio_Clear(&g_LEDs[bID]);  //�͵�ƽ����
    else
        Gpio_Set(&g_LEDs[bID]);    //�ߵ�ƽϨ��
}

//��ȡLED��״̬
//���� true-����״̬��false-Ϣ��״̬
bool GetLed(BYTE bID)
{
    bool fOn;
    fOn = Gpio_Get(&g_LEDs[bID]);
    return (!fOn);   //�͵�ƽ����
}

void ToggleLed(BYTE bID)
{
    if (Gpio_Get(&g_LEDs[bID]))
        Gpio_Clear(&g_LEDs[bID]);
    else
        Gpio_Set(&g_LEDs[bID]);
}

void RunLedToggle()
{
    ToggleLed(LED_RUN);
}

void AlertLedToggle()
{
}

void GpioGprsInit(void)
{
    Gpio_Config(&PinGprsPWR, pdTRUE);
    Gpio_Clear(&PinGprsPWR);  
    Gpio_Config(&PinGprsIGT, pdTRUE);
    Gpio_Clear(&PinGprsIGT);   
    
    Gpio_Config(&PinGprsReset, pdTRUE);
//    Gpio_Clear(&PinGprsIGT);   
#ifdef DEBUG
    //Sleep(2000);             //todo
#endif
}

void GpioLedInit(void)
{    
    Gpio_Config(g_LEDs, ARRAY_SIZE(g_LEDs));
}

void GpioYxInit(void)
{
    Gpio_Config(g_GpioYXs, ARRAY_SIZE(g_GpioYXs));
}

//���ز�ģ����и�λ,����ʹ������300ms��λ����
void PlcReset(void)
{ 
    //Gpio_Set(&PinPhyReset);
    //Sleep(30);
    Gpio_Clear(&PinPlcReset);
    Sleep(300);  
    Gpio_Set(&PinPlcReset);
}


//�ز���Դʹ��,�͵�ƽ�ز���Դ�Ͽ�
void PlcPwrOff(void)
{     
    Gpio_Clear(&PinPlcPWR);
	Sleep(300);  
}

//�ز���Դ���ƣ��ߵ�ƽ�ز���Դ��ͨ
void PlcPwrOn(void)
{
	Gpio_Set(&PinPlcPWR);
    Sleep(300);  
}


//Ӧ����������PHYͨ�ŵ������ٸ�λPHY����ΪPHYҪ����������ȡ��Ϣ���Ӷ�����PHY�ĳ�ʼģʽ
void PhyReset(void)
{ 
	return ;
    //Gpio_Set(&PinPhyReset);
    //Sleep(30);

    //Gpio_Clear(&PinPhyReset);
    //Sleep(5);  //8720 PHY��λֻ��Ҫ100US,���ﲻ��̫�������˻�Ӱ��TCPIP�߳�
    //Gpio_Set(&PinPhyReset);
}

void PhyClose(void)
{
	return ;
    //Gpio_Clear(&PinPhyReset);
}

void PlcInit(void)
{
	PlcPwrOn();
}

void GPIOInit()
{   
    GpioWdgInit();
    
    GpioLedInit();
    
    GpioGprsInit();
       
    GpioYxInit();      
    
    BatOnCtrl();

	Gpio_Config(&PinPlcReset, 1);
	
	Gpio_Config(&PinPlcPWR, 1);
	
    //Gpio_Config(&PinPhyReset, 1);  
    
    Gpio_Config(&PinMagnetic, 1);    
    
    Gpio_Config(g_States, ARRAY_SIZE(g_States));
    
    GpioPowerMgr();
	
    GpioHangInit(); //���㾲��ʵ�� wing 2014/08/12
    //InitLcd();
}

void RtcInit(void)
{
#ifndef RTC8025T
    rtc_set_hour_mode( RTC, 0 ) ;//24Сʱģʽ  
#else
    TwiInit();
    Rtc8025Init();    
    
#if 0    //test 8025T
    static TTime tTime;
    RtcGetTime(&tTime);
    
    tTime.nYear = 2013;
    tTime.nMonth = 2;
    tTime.nDay = 23;
    tTime.nHour = 9;
    tTime.nMinute = 58;
    tTime.nSecond = 0;
    RtcSetTime(&tTime);
    RtcGetTime(&tTime);    
#endif    
    
#endif
}

void RtcSetTime(const TTime* pTime)
{
#ifndef RTC8025T
    rtc_set_time( RTC, pTime->nHour, pTime->nMinute, pTime->nSecond ) ;
    rtc_set_date( RTC, pTime->nYear, pTime->nMonth, pTime->nDay, pTime->nWeek ) ;
#else
    Set8025Time(pTime);
#endif
}

void RtcGetTime(TTime* pTime)
{
#ifndef RTC8025T
    DWORD dw1;
    DWORD dw2;
    DWORD dw3;
    DWORD dw4;
    //rtc_get_time( RTC, &pTime->nHour, &pTime->nMinute, &pTime->nSecond ) ;
    rtc_get_time( RTC, &dw1, &dw2, &dw3 ) ;
    pTime->nHour = dw1;
    pTime->nMinute = dw2;
    pTime->nSecond = dw3;
    //rtc_get_date( RTC, &pTime->nYear, &pTime->nMonth, &pTime->nDay, &pTime->nWeek ) ;
    rtc_get_date( RTC, &dw1, &dw2, &dw3, &dw4 ) ;
    pTime->nYear = dw1;
    pTime->nMonth = dw2;
    pTime->nDay = dw3;
    pTime->nWeek = dw4;
#else
    Get8025Time(pTime);
#endif
}

//true -��ȷ��оƬ
bool CheckChipID(void)
{
    DWORD dwChipID;
    dwChipID = CHIPID->CHIPID_CIDR;
    if (dwChipID == 0x285E0A60)  //��ȷ����Ƭ
    {
        DTRACE(1, ("CPU is correct!\n"));
        return true;
    }
    else if (dwChipID == 0x285E0A30)//������Ƭ
        DTRACE(1, ("CPU is engineering samples!\n"));
    else
        DTRACE(1, ("CPU is error!\n"));
    return false;
}

int DrvInit(void)
{      
    for (BYTE i=0; i<COMM_NUM; i++)
        UartInit(i); 
    IfrInit();
    
    SpiInit();
    
    SysDebugInit();  
    
    //CheckChipID();
    
    GPIOInit();	        
    
    esam_init();
    
    ADInit();
    
    //DTRACE(DB_CRITICAL, ("DrvInit: "DRV_VER" OK.\r\n"));	 
        		
	RtcInit();   
    
    InitRandom();
         
	InFlashInit();     
    
	ExFlashInit();

	PlcInit();
    
    //ui_init();   //usb

#if 0           //test InFlash
    FlashInit();
    BYTE bBuf[300];
    memset(bBuf, 0, sizeof(bBuf));
    //InFlashRd(0, bBuf, sizeof(bBuf));
    ExFlashRdData(EXFLASH_SIZE/2, NULL, bBuf, sizeof(bBuf));
    memset(bBuf, 0x55, sizeof(bBuf));
    //InFlashWrSect(0, bBuf, INSECT_SIZE);
    ExFlashWrData(EXFLASH_SIZE/2, NULL, bBuf, sizeof(bBuf));
    memset(bBuf, 0, sizeof(bBuf));
    //InFlashRd(0, bBuf, sizeof(bBuf));
    ExFlashRdData(EXFLASH_SIZE/2, NULL, bBuf, sizeof(bBuf));
    
    memset(bBuf, 0xAA, sizeof(bBuf));
    InFlashWrSect(0, bBuf, INSECT_SIZE);
    ExFlashWrData(0, NULL, bBuf, sizeof(bBuf));
    memset(bBuf, 0, sizeof(bBuf));
    InFlashRd(0, bBuf, sizeof(bBuf));
    ExFlashRdData(0, NULL, bBuf, sizeof(bBuf));
    while(1);
#endif
    
    return 0;
}

void SetRTS()
{
 	//GPIOPinWrite(GPIO_PORTA_BASE, GPIOA_RTS, GPIOA_RTS);
}

void ResetRTS()
{
	//GPIOPinWrite(GPIO_PORTA_BASE, GPIOA_RTS, 0);
}

void ClearWDG()
{
	//WDT_Restart(WDT);

	ClearHardWdg();
}

void ResetCPU(void)
{    
    NVIC_SystemReset();
}
/*
void ModemReset(void)
{
    Gpio_Clear(&PinGprsIGT);
    Sleep(50);
    Gpio_Clear(&PinGprsReset); 
    Sleep(300);
    Gpio_Set(&PinGprsReset);
}*/

//�ӶϿ���Դ������
bool ModemPowerOn()
{ //��ģ��
    //MCU��TX�������̬��͵�ƽ--Ӳ����һ����������2.8V������4VGPRS��Դת������,���Ա�֤�͵�ƽ�����
    Gpio_Set(&PinGprsIGT);//IGT����
    Gpio_Set(&PinGprsPWR); //�ϵ�
    Sleep(120);    //376.3   >=100
    //MCU��TX���� Ӳ������
    Gpio_Clear(&PinGprsIGT);//IGT��
    Sleep(500);
    Gpio_Clear(&PinGprsReset); 
    Sleep(300);
    Gpio_Set(&PinGprsReset);
    Sleep(200);
    //Sleep(1000);   //376.3
    Gpio_Set(&PinGprsIGT);
    DTRACE(DB_FAPROTO, ("ModemPowerOn : power on!\r\n"));
    
    //Sleep(1000);
    //ModemReset();    
    Sleep(2000);   //�ʵ���ʱ������ʹģ���ʼ�����졣
    /*//��ģ��
    Gpio_Clear(&PinGprsIGT);
    Gpio_Set(&PinGprsPWR);   
    DTRACE(DB_FAPROTO, ("ModemPowerOn : power on!\r\n"));
    Sleep(500);   //����500MS
    
    Gpio_Set(&PinGprsIGT);   */

	return true;
}

bool ModemPowerOff()
{
    /*if (!Gpio_Get(&PinGprsIGT)) //����ǵ͵�ƽ,��Ҫ������������
    {
        Gpio_Set(&PinGprsIGT); 
        Sleep(10);
    }*/
    Gpio_Clear(&PinGprsIGT);
    Sleep(1000); //1S
    Gpio_Set(&PinGprsIGT);   
    Sleep(5000);
    Gpio_Clear(&PinGprsPWR);  
    //MCU��TX�������̬��͵�ƽ
    DTRACE(DB_FAPROTO, ("ModemPowerOn : power off!\r\n"));
    Sleep(5000);//�ص�3S���Կ����Ƿ�Ϩ�𣬴Ӷ��ж��ǲ����ж��˵�Դ��ͬʱҲ�ŵ�ģ���ϵ��ݵĵ�
    
	return true;
}

bool ResetGC864()
{/*
    PIO_Set(&PinGprsReset);
    Sleep(500);
    PIO_Clear(&PinGprsReset);
    Sleep(3000);
    PIO_Set(&PinGprsReset);
    Sleep(2000);*/
    return true;
}

bool ResetME3000()
{/*
	DWORD dwClick = GetClick();
	GPIOPinWrite(GPIO_PORTD_BASE, GPIOD_GPRS_EN, 0);	//����
	while (GetClick()-dwClick < 4)
		Sleep(100);
	GPIOPinWrite(GPIO_PORTD_BASE, GPIOD_GPRS_EN, GPIOD_GPRS_EN);	//����
	while (GetClick()-dwClick < 2)    
		Sleep(100);
  */     
    
	return true;
}

//�ȹ�һ�λ����ٿ�һ�λ������ǲ��ϵ���Դ
bool ResetM590(void)
{
    ModemPowerOff();
    ModemPowerOn();    
    return true;
}

bool ResetGL868(void)
{/*
    Gpio_Clear(&PinGprsReset);
    Sleep(300);
    Gpio_Set(&PinGprsReset);
    Sleep(1500);
    Gpio_Clear(&PinGprsReset);
    Sleep(1000);*/
    return true;
}

//������
bool IsPwrOff()
{/*
    DWORD pins;

    pins = GPIOPinRead(GPIO_PORTE_BASE, GPIOE_PWR_DECT);
    return ((pins&GPIOE_PWR_DECT) == false);*/
    
    return false;
}

//bCh ͨ��
void GetYxInput(BYTE *pbYxVal)
{
    if (pbYxVal == NULL)
        return; 
    
    BYTE i; 
    for (i=0; i<ARRAY_SIZE(g_GpioYXs); i++)
    {        
        pbYxVal[i>>3] |= Gpio_Get(&g_GpioYXs[i])<<(i&7);
    }    
}

//
void BatOnCtrl()
{
 	//GPIOPinWrite(GPIO_PORTF_BASE, GPIOF_BAT_CTRL, GPIOF_BAT_CTRL);
}

void BatOffCtrl()
{
 	//GPIOPinWrite(GPIO_PORTF_BASE, GPIOF_BAT_CTRL, 0);
}

bool GetMagnetic(void)
{
    return Gpio_Get(&PinMagnetic);
}

BYTE GetModeState(void)
{
    BYTE bState = 0;
    bState = Gpio_Get(&g_States[0]);
    bState |= Gpio_Get(&g_States[1])<<1;
    bState |= Gpio_Get(&g_States[2])<<2;
    return bState;
}

//��ʼ�����������������ÿ������һ����һ�����������Ϊ�˿ں�
void InitRandom(void)
{
    /* Configure PMC */
	
    pmc_enable_periph_clk(ID_TRNG);
	
    /* Enable TRNG */
	
    trng_enable(TRNG);
    NVIC_DisableIRQ(TRNG_IRQn);
}

bool GetRandom(DWORD *pdwRandom)
{
    uint32_t status;
	
    status = trng_get_interrupt_status(TRNG);
	
    if ((status & TRNG_ISR_DATRDY) == TRNG_ISR_DATRDY) 
    {		
        if (pdwRandom != NULL)
            *pdwRandom = trng_read_output_data(TRNG);
        //DTRACE(0, ("-- Random Value: %x --\n\r", *pdwRandom));
	
        return true;
    }
    return false;
}

//�������磬��Ҫ�����紦��
bool AcPowerOff(void)
{    
    if (Gpio_Get(&g_PinPowerMgrs[1]) == 0)
        return true;
    return false;
}

//�������粢�ң��������ݵ�ѹ����3.3����Ҫ���뱣��
bool PowerLow(void)
{
    if (AcPowerOff())
    {
        if (Gpio_Get(&g_PinPowerMgrs[0]) == 0) //��ѹ����3.3
            return true;
    }
    return false;
}

void PowerOffProtect(void)
{
    if (PowerLow())
    {
        DTRACE(DB_FAPROTO, ("PowerOffProtect: Power is too low!\r\n"));
        
        //��ֹFLASH����
        DisInFlash();
        
        DisExFlash();   //Ƭ��FLASH
        
        Gpio_Clear(&PinGprsPWR);  //GPRS�ĵ�ԴҲҪ�ر�
        Gpio_Clear(&g_PinPowerMgrs[2]);  //������Դ
        Sleep(100);
        ResetCPU();
        while(1);
    }
}
