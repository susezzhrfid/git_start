#include "Rtc8025t.h"
#include "board.h"
#include "SysDebug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Gpio.h"
#include "pmc.h"
#include "twi.h"
#include "sysclk.h"

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/

/** TWI Bus Clock 400kHz */
#define TWI_CLK               100000   //100K,10K

/** Slave address  */
#define RTC8025_ADDR    0x32//0x64

#define RTC8025_MEM_ADDR_LENGTH  1

#define BOARD_BASE_TWI_RTC8025    TWI1

/** Data to be sent */
#define  TEST_DATA_LENGTH  (sizeof(test_data_tx)/sizeof(uint8_t))

#define RX8025T_SECOND				0x0
#define RX8025T_MINUTES				0x1
#define RX8025T_HOURS				0x2
#define RX8025T_WEEKDAYS			0x3
#define RX8025T_DAYS				0x4
#define RX8025T_MONTHS				0x5
#define RX8025T_YEARS				0x6
#define RX8025T_RAM					0x7
#define RX8025T_ALARMW_MIN  		0x8
#define RX8025T_ALARMW_HOUR		    0x9
#define RX8025T_ALARMW_WEEK         0xA
#define RX8025T_TIMERCNT0		    0xB
#define RX8025T_TIMERCNT1    		0xC
#define RX8025T_EXTENTION   		0xD
#define RX8025T_FLAG				0xE
#define RX8025T_CONTROL 			0xF
// ----------------------------------------------------------------------------------------------------------
// TWI
// checked - tvd
// ----------------------------------------------------------------------------------------------------------

/** TWI0 data pin */
//#define PIN_TWI_TWD0   {PIO_PA17A_TWD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** TWI0 clock pin */
//#define PIN_TWI_TWCK0  {PIO_PA18A_TWCK0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/** TWI1 data pin */
#define PIN_TWI_TWD1   {PIO_PB12A_TWD1,  PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/** TWI1 clock pin */
#define PIN_TWI_TWCK1  {PIO_PB13A_TWCK1, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}

/** TWI1 pins */
#define PINS_TWI1      PIN_TWI_TWD1, PIN_TWI_TWCK1

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

/** Pio pins to configure. */
static const Pin pins_twi_temp[] = {PINS_TWI1};

static twi_options_t opt;
static twi_packet_t packet_tx, packet_rx;

//static Async tAsync;   //异步传输需要初始化tAsync

void TwiInit(void)
{
    /* Configure TWI pins */
    Gpio_Config(pins_twi_temp, PIO_LISTSIZE(pins_twi_temp));

    /* Configure TWI */
    pmc_enable_periph_clk(ID_TWI1);
    
    /* Configure the options of TWI driver */
	opt.master_clk = sysclk_get_cpu_hz();
	opt.speed      = TWI_CLK;

	/* Configure the data packet to be transmitted */
	packet_tx.chip        = RTC8025_ADDR;
	packet_tx.addr_length = RTC8025_MEM_ADDR_LENGTH;

	/* Configure the data packet to be received */
	packet_rx.chip        = packet_tx.chip;
	packet_rx.addr_length = packet_tx.addr_length;    
    
    twi_master_init(BOARD_BASE_TWI_RTC8025, &opt);
}

unsigned char GetSecond(void);

void Rtc8025Init(void)
{
    BYTE bData;
    BYTE bRet;
    bData = 0x08;    
    packet_tx.addr[0] = RX8025T_EXTENTION;
	packet_tx.buffer      = (uint8_t *) &bData;
	packet_tx.length      = 1;
    bRet = twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx);
    if (bRet)
        DTRACE(1, ("Write rtc8025 reg failed\r\n"));
    
    bData = 0x00;
    packet_tx.addr[0]     = RX8025T_FLAG;
    bRet = twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx);
    if (bRet)
        DTRACE(1, ("Write rtc8025 reg failed\r\n"));

    bData = 0x60;
    packet_tx.addr[0]     = RX8025T_CONTROL;
    bRet = twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx);
    if (bRet)
        DTRACE(1, ("Write rtc8025 reg failed\r\n"));    
}
/*
void TWI0_IrqHandler(void)//异步传输才会开中断
{
    TWID_Handler(&twid);
}*/

unsigned char GetHour(void)
{
	unsigned char dat=0, hours;
    packet_rx.addr[0]     = RX8025T_HOURS;
    packet_rx.buffer      = (uint8_t *) &dat;
    packet_rx.length      = 1;
    if (twi_master_read(BOARD_BASE_TWI_RTC8025, &packet_rx) != TWI_SUCCESS)
	  return 0xff;
	hours=(dat>>4)*10+(dat & 0x0f);
	return hours;
}

bool SetHour(unsigned char hours)
{
	unsigned char dat=0;
	if(hours>=24)
		return false;
		
	dat=hours%10;
	dat|=(hours/10)<<4;
    packet_tx.addr[0]     = RX8025T_HOURS;
	packet_tx.buffer      = (uint8_t *) &dat;
	packet_tx.length      = 1;
    if (twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx) != TWI_SUCCESS)
	  return false;
	return true;
}

unsigned char GetMinute(void)
{
	unsigned char dat=0,minutes;
    packet_rx.addr[0]     = RX8025T_MINUTES;
    packet_rx.buffer      = (uint8_t *) &dat;
    packet_rx.length      = 1;
    if (twi_master_read(BOARD_BASE_TWI_RTC8025, &packet_rx) != TWI_SUCCESS)
	  return 0xff;
	minutes=(dat>>4)*10+(dat & 0x0f);
	return minutes;
}

bool SetMinute(unsigned char minutes)
{
	unsigned char dat=0;
	if(minutes>=60)
		return false;

	dat=minutes%10;
	dat|=(minutes/10)<<4;
    packet_tx.addr[0]     = RX8025T_MINUTES;
	packet_tx.buffer      = (uint8_t *) &dat;
	packet_tx.length      = 1;
    if(twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx) != TWI_SUCCESS)
		return false;
	return true;	
}

unsigned char GetSecond(void)
{
	unsigned char dat=0,seconds;
    packet_rx.addr[0]     = RX8025T_SECOND;
    packet_rx.buffer      = (uint8_t *) &dat;
    packet_rx.length      = 1;
    if(twi_master_read(BOARD_BASE_TWI_RTC8025, &packet_rx) != TWI_SUCCESS)
	  return 0xff;
	seconds=(dat>>4)*10+(dat & 0x0f);
	return seconds;
}

bool SetSecond(unsigned char seconds)
{
	unsigned char dat;
	if(seconds>=60)
		return false;
		
	dat=seconds%10;
	dat|=(seconds/10)<<4;
    packet_tx.addr[0]     = RX8025T_SECOND;
	packet_tx.buffer      = (uint8_t *) &dat;
	packet_tx.length      = 1;
    if(twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx) != TWI_SUCCESS)
	  return false;
	return true;	
}

//设置星期几，从1～7，表示星期天到星期六
bool SetWeek(unsigned char days)
{
	if(days>7 || days<1)
		return false;
	days--;
    packet_tx.addr[0]     = RX8025T_WEEKDAYS;
	packet_tx.buffer      = (uint8_t *) &days;
	packet_tx.length      = 1;
    if(twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx) != TWI_SUCCESS)
	  return false;
	return true;
}

//返回星期几（1～7）
unsigned char GetWeek(void)
{
	unsigned char days=0;
    packet_rx.addr[0]     = RX8025T_WEEKDAYS;
    packet_rx.buffer      = (uint8_t *) &days;
    packet_rx.length      = 1;
    if(twi_master_read(BOARD_BASE_TWI_RTC8025, &packet_rx) != TWI_SUCCESS)
	  return 0xff;
	days++;
	return days;
}

unsigned char GetDay8025(void)
{
	unsigned char dat=0,dates;
    packet_rx.addr[0]     = RX8025T_DAYS;
    packet_rx.buffer      = (uint8_t *) &dat;
    packet_rx.length      = 1;
    if(twi_master_read(BOARD_BASE_TWI_RTC8025, &packet_rx) != TWI_SUCCESS)
	  return 0xff;
	dates=(dat>>4)*10+(dat & 0x0f);
	return dates;
}

bool SetDay(unsigned char dates)
{
	unsigned char dat=0;
	if(dates>=32 || dates<1)
		return false;
		
	dat=dates%10;
	dat|=(dates/10)<<4;
    packet_tx.addr[0]     = RX8025T_DAYS;
	packet_tx.buffer      = (uint8_t *) &dat;
	packet_tx.length      = 1;
    if(twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx) != TWI_SUCCESS)
	  return false;
	return true;
} 

unsigned char GetYear(void)
{
	unsigned char dat=0,years;
    packet_rx.addr[0]     = RX8025T_YEARS;
    packet_rx.buffer      = (uint8_t *) &dat;
    packet_rx.length      = 1;
    if(twi_master_read(BOARD_BASE_TWI_RTC8025, &packet_rx) != TWI_SUCCESS)
	  return 0xff;
	years=(dat>>4)*10+(dat & 0x0f);
	return years;
}

bool SetYear(int years)
{
	unsigned char dat=0;
	if(years>99 || years<0)
		return false;
		
	dat=years%10;
	dat|=(years/10)<<4;
    packet_tx.addr[0]     = RX8025T_YEARS;
	packet_tx.buffer      = (uint8_t *) &dat;
	packet_tx.length      = 1;
    if(twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx) != TWI_SUCCESS)
	  return false;
	return true;
} 

unsigned char GetMonth8025(void)
{
	unsigned char dat=0,months;
    packet_rx.addr[0]     = RX8025T_MONTHS;
    packet_rx.buffer      = (uint8_t *) &dat;
    packet_rx.length      = 1;
    if(twi_master_read(BOARD_BASE_TWI_RTC8025, &packet_rx) != TWI_SUCCESS)
	  return 0xff;
	months=(dat>>4)*10+(dat & 0x0f);
	return months;
}

bool SetMonth(unsigned char months)
{
	unsigned char dat=0;
	if(months>12 || months<1)
		return false;
		
	dat=months%10;
	dat|=(months/10)<<4;
    packet_tx.addr[0]     = RX8025T_MONTHS;
	packet_tx.buffer      = (uint8_t *) &dat;
	packet_tx.length      = 1;
    if(twi_master_write(BOARD_BASE_TWI_RTC8025, &packet_tx) != TWI_SUCCESS)
	  return false;
	return true;
} 

bool Set8025Time(const TTime *time)
{
   	WORD  nYear = time->nYear;

    nYear -= 2000;
	if(time->nMonth>12 || time->nMonth<1)
		return false;
  	if(nYear>99)
		return false;
	if(time->nDay>=32 || time->nDay<1)
		return false;
	if(time->nMinute>60)
		return false;
	if(time->nSecond>60)
		return false;

	if(!SetHour(time->nHour)) return false;	
	if(!SetMinute(time->nMinute)) return false;
	if(!SetSecond(time->nSecond)) return false;
	if(!SetMonth(time->nMonth)) return false;
	if(!SetYear(nYear)) return false;
	if(!SetDay(time->nDay)) return false;
	return true;
}

bool Get8025Time(TTime *time)
{
	time->nHour = GetHour();
	if(0xff == time->nHour) 
	  return false;
	time->nSecond = GetSecond();
	if(0xff == time->nSecond) 
	  return false;
	time->nWeek = GetWeek();
	if(0xff == time->nWeek) 
	  return false;
	time->nDay = GetDay8025();
	if(0xff == time->nDay) 
	  return false;
	time->nMinute = GetMinute();
	if(0xff == time->nMinute) 
	  return false;
	time->nMonth = GetMonth8025();
	if(0xff == time->nMonth) 
	  return false;
	time->nYear = GetYear();
	if(0xff == time->nYear) 
	  return false;

    time->nYear += 2000;
	return true;
}
