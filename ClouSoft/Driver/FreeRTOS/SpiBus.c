#include "spi.h"
#include "Sysarch.h"
#include "syscfg.h"
#include "comm.h"
#include "Drivers.h"
//#include "dmad.h"
#include "sysdebug.h"
#include "Gpio.h"
#include "board.h"
#include "sysclk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include "SpiBus.h"

//#define ESAM_HARDWARE_SPI      

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------------------------------------
// SPI
// checked - tvd
// ----------------------------------------------------------------------------------------------------------
/** SPI MISO pin definition. */
#define PIN_SPI0_MISO    {PIO_PA25A_SPI0_MISO, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_PULLUP} //}
/** SPI MOSI pin definition. */
#define PIN_SPI0_MOSI    {PIO_PA26A_SPI0_MOSI, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI SPCK pin definition. */
#define PIN_SPI0_SPCK    {PIO_PA27A_SPI0_SPCK, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI chip select pin definition. */
#define PIN_SPI0_NPCS0  {PIO_PA28A_SPI0_NPCS0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

#define PIN_SPI0_NPCS3  {PIO_PB23B_SPI0_NPCS3, PIOB, ID_PIOB, PIO_PERIPH_B, PIO_DEFAULT}

//ESAMƬѡ�Ƕ����ģ���������ƣ�������Ӳ������
#define PIN_SPI_ESAM_CS {PIO_PB15, PIOB, ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#ifndef ESAM_HARDWARE_SPI   //��Ҫ��Ӳ����֧��
//ESAM
#define PIN_SPI_ESAM_MISO    {PIO_PA22, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP} //}
/** SPI MOSI pin definition. */
#define PIN_SPI_ESAM_MOSI    {PIO_PA23, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
/** SPI SPCK pin definition. */
#define PIN_SPI_ESAM_SPCK    {PIO_PA16, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#endif

/** List of SPI pin definitions (MISO, MOSI & SPCK). */
#define PINS_SPI0        PIN_SPI0_MISO, PIN_SPI0_MOSI, PIN_SPI0_SPCK

/** Global DMA driver for all transfer */
//static sDmad dmad;

/** DMA receive channel of SPI0 */
//static uint32_t spi0DmaRxChannel;
/** DMA transmit channel of SPI0 */
//static uint32_t spi0DmaTxChannel;

/** SPI base address for SPI master mode*/
#define SPI_MASTER_BASE      SPI0


/* Chip select. */
#define SPI_CHIP_SEL 0

#define SPI_PCS_CH1   0
#define SPI_PCS_CH2   3
#define SPI_PCS_ESAM  1      //ESAMѡ��SPIͨ��1������Ƭѡ�Ƕ�����

/* Clock polarity. */
#define SPI_CLK_POLARITY 0

/* Clock phase. */
#define SPI_CLK_PHASE 1

/* Delay before SPCK. */
#define SPI_DLYBS 0x15//0x40  //250

/* Delay between consecutive transfers. */
#define SPI_DLYBCT 0//0x10  //250

static TSem g_semSPI;

#define SPI_ESAM_DLYBS      0xff//50  //50US     time = dlybs/mck     100
#define SPI_ESAM_DLYBCT     40//15  //15us     time = 32*dlybct/mck  1


/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/
/** PIOs for all SPI modes */
#ifdef ESAM_HARDWARE_SPI
static const Pin pSpiPins[] = {PINS_SPI0, PIN_SPI0_NPCS0, PIN_SPI0_NPCS3, PIN_SPI_ESAM_CS};
#else    //��Ҫ��Ӳ����֧��
static const Pin pSpiPins[] = {PINS_SPI0, PIN_SPI0_NPCS0, PIN_SPI0_NPCS3};
static const Pin pSoftSpiPins[] = {PIN_SPI_ESAM_CS, PIN_SPI_ESAM_MISO, PIN_SPI_ESAM_MOSI, PIN_SPI_ESAM_SPCK};
#endif
/* SPI clock setting (Hz). */
static uint32_t gs_ul_spi_clock = 8000000;

/** 64 bytes data buffer for SPI transfer and receive */
//static uint8_t pucSpi0Buffer[64];

/** reception done*/
//volatile bool bRecvDone0 = false;

/* SPI clock configuration. */
static const uint32_t gs_ul_clock_configurations[] =
		{ 500000, 1000000, 2000000, 5000000, 8000000, 21000000, 42000000};

/**
 * \brief Initialize SPI0 as master
 */
static void spi_master_initialize( void )
{
    /* Configure an SPI peripheral. */
	spi_enable_clock(SPI_MASTER_BASE);
	spi_disable(SPI_MASTER_BASE);
	spi_reset(SPI_MASTER_BASE);
	spi_set_lastxfer(SPI_MASTER_BASE);
	spi_set_master_mode(SPI_MASTER_BASE);
	spi_disable_mode_fault_detect(SPI_MASTER_BASE);
	//spi_set_peripheral_chip_select_value(SPI_MASTER_BASE, SPI_CHIP_SEL);
    spi_set_delay_between_chip_select(SPI_MASTER_BASE, 200);   //  time = dlybcs/mck  
    
    //SPI FLASH
	spi_set_clock_polarity(SPI_MASTER_BASE, SPI_PCS_CH1, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI_MASTER_BASE, SPI_PCS_CH1, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI_MASTER_BASE, SPI_PCS_CH1, SPI_CSR_BITS_8_BIT);
	spi_set_baudrate_div(SPI_MASTER_BASE, SPI_PCS_CH1, (sysclk_get_cpu_hz() / gs_ul_spi_clock));
	spi_set_transfer_delay(SPI_MASTER_BASE, SPI_PCS_CH1, SPI_DLYBS, SPI_DLYBCT);
    spi_configure_cs_behavior(SPI_MASTER_BASE, SPI_PCS_CH1, SPI_CS_KEEP_LOW);
        
    //SPI Flash
    spi_set_clock_polarity(SPI_MASTER_BASE, SPI_PCS_CH2, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI_MASTER_BASE, SPI_PCS_CH2, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI_MASTER_BASE, SPI_PCS_CH2, SPI_CSR_BITS_8_BIT);
	spi_set_baudrate_div(SPI_MASTER_BASE, SPI_PCS_CH2, (sysclk_get_cpu_hz() / gs_ul_spi_clock));
	spi_set_transfer_delay(SPI_MASTER_BASE, SPI_PCS_CH2, SPI_DLYBS, SPI_DLYBCT);
    spi_configure_cs_behavior(SPI_MASTER_BASE, SPI_PCS_CH2, SPI_CS_KEEP_LOW);
    
    //SPI ESAM    MODE 3
#ifdef ESAM_HARDWARE_SPI
    spi_set_clock_polarity(SPI_MASTER_BASE, SPI_PCS_ESAM, 1);
	spi_set_clock_phase(SPI_MASTER_BASE, SPI_PCS_ESAM, 0);
	spi_set_bits_per_transfer(SPI_MASTER_BASE, SPI_PCS_ESAM, SPI_CSR_BITS_8_BIT);
	spi_set_baudrate_div(SPI_MASTER_BASE, SPI_PCS_ESAM, (sysclk_get_cpu_hz() / 5000000));   //�̶�5M
	spi_set_transfer_delay(SPI_MASTER_BASE, SPI_PCS_ESAM, SPI_ESAM_DLYBS, SPI_ESAM_DLYBCT);
    spi_configure_cs_behavior(SPI_MASTER_BASE, SPI_PCS_ESAM, SPI_CS_KEEP_LOW);
#endif //ESAM_HARDWARE_SPI
        
	spi_enable(SPI_MASTER_BASE);
}

/**
 * \brief Sets the specified SPI clock configuration.
 * \param configuration  Index of the configuration to set.
 */
static void spi_set_clock_configuration( uint8_t configuration )
{
    gs_ul_spi_clock = gs_ul_clock_configurations[configuration];
    spi_master_initialize();
}

void SpiInit(void)
{
    /* Configure PIO Pins for SPI */
    Gpio_Config( pSpiPins, PIO_LISTSIZE( pSpiPins ) ) ;     
#ifndef ESAM_HARDWARE_SPI   //��Ҫ��Ӳ����֧��
    Gpio_Config( pSoftSpiPins, PIO_LISTSIZE( pSoftSpiPins ) ) ;   
#endif
    
    /* Configure DMA with IRQ */
    //_ConfigureDma();
    
    spi_set_clock_configuration( 5 ) ; //21M SPIʱ�� �����ⲿFLASH,���75M
    
    g_semSPI = NewSemaphore(1, 1);   
    
    return;    
}

//#define DELAY  {__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();}
//84M
void Delay1us(WORD wTime)
{
    while(wTime--)
    {
        for (int j=0; j<5; j++)
        {
            __NOP();
        }
    }
}

#ifndef ESAM_HARDWARE_SPI
BYTE SPIRecvByte(void)          //���ո�λ              ��λ��ǰ,��λ�ں�
{
    BYTE i;
    BYTE bData;    
    EnterCritical();
    for (i=0; i<8; i++)
    {
        Gpio_Clear(&pSoftSpiPins[3]);        
        bData <<= 1;         
        if (Gpio_Get(&pSoftSpiPins[1]))//��һ��ʱ�������� 
            bData |= 0x01;        
        Gpio_Set(&pSoftSpiPins[3]);
    }
    ExitCritical();

    return bData;
}

BYTE SPISendByte(BYTE bData)
{
    BYTE i;
    EnterCritical();
    for (i=0; i<8; i++)
    {                
        Gpio_Clear(&pSoftSpiPins[3]);        
        if (bData & 0x80)
            Gpio_Set(&pSoftSpiPins[2]);
        else
            Gpio_Clear(&pSoftSpiPins[2]);
        bData <<= 1; 
        Gpio_Set(&pSoftSpiPins[3]);//������  
    }
    Delay1us(1);
    Gpio_Clear(&pSoftSpiPins[2]);
    ExitCritical();

    return 1;
}
#endif //ESAM_HARDWARE_SPI

//ʹ��SPI--�������ź�������
bool SPIEnable(BYTE bChip)
{
#ifndef ESAM_HARDWARE_SPI    
    if (bChip == ESAM_CHIP)
    {     
        //OsEnterCritical();  //���ص�����ESAM�׳���   EnterCritical()?
        Delay1us(12);
        EnterCritical();
        Gpio_Clear(&pSoftSpiPins[0]);    //ע�����������жϣ������CPU��BUG��ɵġ�
        ExitCritical();
        Delay1us(60);
        return true;
    }
#endif //ESAM_HARDWARE_SPI
    
	WaitSemaphore(g_semSPI, SYS_TO_INFINITE);	

	//FlashƬѡ���Ϊ�ͣ�ʹ��Ƭѡ---��Ҫ��ֻ֤��һƬ��Ƭѡ
	switch (bChip)
	{
	case EXFLASH_FST_CHIP:	//Ƭѡ��һ��        
        spi_set_peripheral_chip_select_value(SPI0, 0x0e);
		break;
	case EXFLASH_SND_CHIP:	//Ƭѡ�ڶ���
        spi_set_peripheral_chip_select_value(SPI0, 0x07);
		break;
#ifdef ESAM_HARDWARE_SPI
    case ESAM_CHIP:         //ESAMƬѡ        //�ɸ߱��ʱ��ʱ10US    
        //OsEnterCritical(); //todo:���ж�ô��
        Delay1us(10);
        EnterCritical();
        Gpio_Clear(&pSpiPins[5]); 
        ExitCritical();
        spi_set_peripheral_chip_select_value(SPI0, 0x0d);  //PS = 0ʱ���룬����
        Delay1us(100);
        break;    
#endif //ESAM_HARDWARE_SPI
	default:	//���ǵ�һƬҲ���ǵڶ�Ƭ��ʱ�򣬲�ƬѡFlah
		break;
	}
	
  	spi_enable(SPI0);
	return true;
}
    
//��ֹSPI--���ͷű������ź���
bool SPIDisable(BYTE bChip)
{
#ifndef ESAM_HARDWARE_SPI    
    if (bChip == ESAM_CHIP)
    {
        //OsExitCritical();
        EnterCritical();
        Gpio_Set(&pSoftSpiPins[0]); 
        ExitCritical();
        Delay1us(12);
        return true;
    }
#else
    if (bChip == ESAM_CHIP)
    {
        //OsExitCritical();
        ExitCritical();
        Gpio_Set(&pSpiPins[5]);  
        ExitCritical();
        Delay1us(12);
    }
#endif //ESAM_HARDWARE_SPI
    
	spi_disable(SPI0);
    spi_set_lastxfer(SPI0);
	SignalSemaphore(g_semSPI);

	return true;
}

//����һ���ֽ�
void SSI1SendByte(BYTE bChip, BYTE bData)
{
#ifndef ESAM_HARDWARE_SPI
    if (bChip == ESAM_CHIP)
    {
        SPISendByte(bData);
        return;
    }
#endif
        
    BYTE bCs = 0;
    WORD wData;
    switch(bChip)
    {
    case EXFLASH_FST_CHIP:
        bCs = SPI_PCS_CH1;
        break;
    case EXFLASH_SND_CHIP:
        bCs = SPI_PCS_CH2;
        break;
    case ESAM_CHIP:
        bCs = SPI_PCS_ESAM;
        break;
    default:
        break;
    }
    spi_write(SPI0, bData, bCs, 0);
    spi_read(SPI0, &wData, &bCs);
}

//����һ���ֽ�
BYTE SSI1GetByte(BYTE bChip)
{
#ifndef ESAM_HARDWARE_SPI
    if (bChip == ESAM_CHIP)
    {        
        return SPIRecvByte();
    }    
#endif //ESAM_HARDWARE_SPI
    
	WORD wReadData = 0;
    BYTE bCs = 0;
    switch(bChip)
    {
    case EXFLASH_FST_CHIP:
        bCs = SPI_PCS_CH1;
        break;
    case EXFLASH_SND_CHIP:
        bCs = SPI_PCS_CH2;
        break;
    case ESAM_CHIP:
        bCs = SPI_PCS_ESAM;
        break;
    default:
        break;
    }
    spi_write(SPI0, 0x00, bCs, 0);//����һ����Ч�ֽڣ��Բ�������ʱ��	
    spi_read(SPI0, &wReadData, &bCs);
	return (BYTE)wReadData;
}

//��������pulData������ͨ��SSI1����ȥ
//������@*pulData ׼������ȥ������
//		@nLen �������ݵĳ���
//���أ���ȷ���ͷ��ط��͵ĳ��ȣ����򷵻�0
//�ܽ᣺ 
DWORD SSI1SendData(BYTE bChip, BYTE *pbData, WORD nLen)
{
	WORD i = 0;
    
	while (i<nLen)
	{
		SSI1SendByte(bChip, *pbData++);
        if (bChip == ESAM_CHIP)  //ֻ��SPI ESAM����Ҫÿ���ֽڼ�����ʱ15US
            Delay1us(15);
		i++;
	}
	
	return nLen;
}

//��������SSI1�������ݻ���
//������@*pulData �������ݻ�����
//		@nLen �����ݵĳ���
//���أ���ȷ���շ������ݵĳ��ȣ����򷵻�0
//�ܽ᣺ 
DWORD SSI1GetData(BYTE bChip, BYTE *pbData, WORD nLen)
{
	WORD i = 0;
    
	while (i<nLen)
	{
		*pbData++ = SSI1GetByte(bChip);
        if (bChip == ESAM_CHIP)  //ֻ��SPI ESAM����Ҫÿ���ֽڼ�����ʱ15US
            Delay1us(15);
		i++;
	}
	
	return nLen;
}
