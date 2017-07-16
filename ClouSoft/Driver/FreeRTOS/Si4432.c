#include "Si4432.h"

#include "spi.h"
#include "Sysarch.h"
#include "syscfg.h"
#include "comm.h"
#include "Drivers.h"
#include "dmad.h"
#include "sysdebug.h"
#include "DrvHook.h"
#include "board.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define HARDWARE_SPI       1

//#define WL_PARA_TEST       0

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------------------------------------
// SPI
// checked - tvd
// ----------------------------------------------------------------------------------------------------------
/** SPI MISO pin definition. */
#if HARDWARE_SPI
#define PIN_SPI0_MISO    {PIO_PA25A_SPI0_MISO, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_PULLUP} //}
/** SPI MOSI pin definition. */
#define PIN_SPI0_MOSI    {PIO_PA26A_SPI0_MOSI, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI SPCK pin definition. */
#define PIN_SPI0_SPCK    {PIO_PA27A_SPI0_SPCK, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI chip select pin definition. */
#define PIN_SPI0_NPCS0  {PIO_PA28A_SPI0_NPCS0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#define PIN_SPI0_MISO    {PIO_PA25A_SPI0_MISO, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP} //}
/** SPI MOSI pin definition. */
#define PIN_SPI0_MOSI    {PIO_PA26A_SPI0_MOSI, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
/** SPI SPCK pin definition. */
#define PIN_SPI0_SPCK    {PIO_PA27A_SPI0_SPCK, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
/** SPI chip select pin definition. */
#define PIN_SPI0_NPCS0  {PIO_PA28A_SPI0_NPCS0, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
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

/*----------------------------------------------------------------------------
 *        Tyoes
 *----------------------------------------------------------------------------*/

#define PIN_WL_GDO0_INT         {PIO_PA20, PIOA, ID_PIOA, PIO_INPUT, PIO_IT_FALL_EDGE} //边沿中断

static const Pin PinWlGdo0Int = PIN_WL_GDO0_INT;

TCAvoid g_tCAvoid = {0, -120};

BYTE g_bRadioMode;

TWlPara g_tWlPara;

//TRxCnt g_tRxCnt = {-120, 0, 0};
TTxState g_tTxState;
TRxState g_tRxState;

TWlRx g_tWlRx[WL_NUM] = { 0 };
//TWlTx g_tWlTx[WL_NUM] = { 0 };

//--Si--//默认值 SI为17db
const BYTE g_bPower[10] = {0x1e, 0x18, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
//Si(db):17，  1,  1,  2,  5,  8, 11, 14, 17, 20

static TSem g_semWlUser;
static TSem g_semWlSyncRx;
static TSem g_semWlSyncTx;
static TSem g_semWlRx;
static bool g_fWlOpen;

static TSem g_semWlTxRx;
static TSem g_semSPI;

DWORD g_dwWlRxInterval;
DWORD g_dwWlRxIntervalConst;

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/
/** PIOs for all SPI modes */
static const Pin pSpiPins[] = {PINS_SPI0, PIN_SPI0_NPCS0};

/** SPI Clock setting (Hz) */
static uint32_t dwSpiClock = 200000;

/** 64 bytes data buffer for SPI transfer and receive */
//static uint8_t pucSpi0Buffer[64];

/** reception done*/
//volatile bool bRecvDone0 = false;

/** SPI clock configuration */
static const uint32_t dwClockConfigurations[] = { 200000, 500000, 1000000, 5000000};

/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/
TThreadRet WirelessTxRxTask(void *pvParameters);
void CfgWlIntPin(void);

/**
 *  \brief Callback function for DMA receiving.

static void _DmaRxCallback0( void )
{
    bRecvDone0 = true;
} */

/**
 * \brief Initialize SPI0 as master
 */
static void _Spi0MasterInitialize( void )
{
    //printf( "-I- Configure SPI0 as master\n\r" ) ;
    /* Master mode */
    SPI_Configure( SPI_MASTER_BASE, ID_SPI0, SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_PCS( 0 ) | SPI_MR_DLYBCS(200) ) ;
    //SPI_ConfigureNPCS( SPI_MASTER_BASE, 0, SPI_CSR_NCPHA | SPI_DLYBCT( 0, BOARD_MCK ) | SPI_DLYBS(100000, BOARD_MCK) | SPI_SCBR( dwSpiClock, BOARD_MCK) ) ;//SPI_CSR_NCPHA    //SPI_CSR_CSNAAT    
    SPI_ConfigureNPCS( SPI_MASTER_BASE, 0, SPI_CSR_CSAAT | SPI_CSR_NCPHA | SPI_DLYBCT( 0, BOARD_MCK ) | SPI_DLYBS(100000, BOARD_MCK) | SPI_SCBR( dwSpiClock, BOARD_MCK) ) ;  //片选不上升直到有其它片选
    /* Enables a SPI peripheral. */
    SPI_Enable( SPI_MASTER_BASE ) ;
}

/**
 * \brief Sets the specified SPI clock configuration.
 * \param configuration  Index of the configuration to set.
 */
static void _SetClockConfiguration( uint8_t configuration )
{
    dwSpiClock = dwClockConfigurations[configuration];
//  printf("Setting SPI0 clock #%d ... \n\r", (int32_t)dwClockConfigurations[configuration]);

    _Spi0MasterInitialize();
}
#if 0
/**
 * \brief Perform SPI master transfer using PDC.
 * \param pBuf Pointer to buffer to transfer.
 * \param size dwSize of the buffer.
 */
static void _SpiMasterTransfer( void * pBuf, uint32_t dwSize )
{
    sDmaTransferDescriptor td;

    td.dwSrcAddr = (uint32_t)&SPI0->SPI_RDR;//接收  SPI接收的数据放入pBuf
    td.dwDstAddr = (uint32_t) pBuf;
    td.dwCtrlA   = dwSize | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE; //size is set to 8-bit width
    td.dwCtrlB   = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR
                    | DMAC_CTRLB_FC_PER2MEM_DMA_FC
                    | DMAC_CTRLB_SRC_INCR_FIXED
                    | DMAC_CTRLB_DST_INCR_INCREMENTING;
    td.dwDscAddr = 0;
    DMAD_PrepareSingleTransfer(&dmad, spi0DmaRxChannel, &td);
    DMAD_StartTransfer(&dmad, spi0DmaRxChannel);

    td.dwSrcAddr = (uint32_t) pBuf;//发送 pBuf的数据发送出去，注意pBuf不会与接收冲突。因为这是半双工通信串行设备。
    td.dwDstAddr = (uint32_t)&SPI0->SPI_TDR;
    td.dwCtrlA   = dwSize | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    td.dwCtrlB   = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR
                    | DMAC_CTRLB_FC_MEM2PER_DMA_FC
                    | DMAC_CTRLB_SRC_INCR_INCREMENTING
                    | DMAC_CTRLB_DST_INCR_FIXED;
    td.dwDscAddr = 0;
    DMAD_PrepareSingleTransfer(&dmad, spi0DmaTxChannel, &td);
    DMAD_StartTransfer(&dmad, spi0DmaTxChannel);
}

/**
 * \brief DMA driver configuration
 */
static void _ConfigureDma( void )
{
    uint32_t dwCfg;
    uint8_t ucController;
    /* Driver initialize */
    DMAD_Initialize( &dmad, 0 );

    /* IRQ configure */
    NVIC_EnableIRQ(DMAC_IRQn);

    /* Allocate DMA channels for SPI0 */
    spi0DmaTxChannel = DMAD_AllocateChannel( &dmad, DMA_TRANSFER_MEMORY, ID_SPI0);
    spi0DmaRxChannel = DMAD_AllocateChannel( &dmad, ID_SPI0, DMA_TRANSFER_MEMORY);
    if (   spi0DmaTxChannel == DMA_ALLOC_FAILED
        || spi0DmaRxChannel == DMA_ALLOC_FAILED )
    {
        //printf("DMA channel allocat error\n\r");
        assert(0);
    }
    /* Set RX callback */
    DMAD_SetCallback(&dmad, spi0DmaRxChannel, (DmadTransferCallback)_DmaRxCallback0, 0);
    /* Configure DMA RX channel */
    ucController = (spi0DmaRxChannel >> DMAC_CHANNEL_NUM);
    dwCfg = 0 | DMAC_CFG_SRC_PER( DMAIF_GetChannelNumber( ucController, ID_SPI0, DMA_TRANSFER_RX ))
              | DMAC_CFG_SRC_H2SEL
              | DMAC_CFG_SOD
              | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &dmad, spi0DmaRxChannel, dwCfg );
    /* Configure DMA TX channel */
    ucController = (spi0DmaTxChannel >> DMAC_CHANNEL_NUM);
    dwCfg = 0 | DMAC_CFG_DST_PER( DMAIF_GetChannelNumber( ucController, ID_SPI0, DMA_TRANSFER_TX ))
              | DMAC_CFG_DST_H2SEL
              | DMAC_CFG_SOD
              | DMAC_CFG_FIFOCFG_ALAP_CFG;
    DMAD_PrepareChannel( &dmad, spi0DmaTxChannel, dwCfg );
}

/**
 * ISR for DMA interrupt
 */
void DMAC_IrqHandler(void)
{
    DMAD_Handler(&dmad);
}
#endif

void SpiInit(void)
{
    /* Configure PIO Pins for SPI */
    PIO_PinConfigure( pSpiPins, PIO_LISTSIZE( pSpiPins ) ) ; 
    
    /* Configure DMA with IRQ */
    //_ConfigureDma();
    
    _SetClockConfiguration( 1 ) ; //5M SPI时钟   //500K
    
    g_semSPI = NewSemaphore(1, 1);   
    
    return;    
}

#if (!HARDWARE_SPI)
#define DELAY  {__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();}

BYTE SPIRecvByte(void)          //先收高位              高位在前,低位在后
{
    BYTE i;
    BYTE bData;    
    for (i=0; i<8; i++)
    {
        PIO_PinClear(&pSpiPins[2]);
        DELAY;
        PIO_PinSet(&pSpiPins[2]);     
        DELAY;
        bData <<= 1; 
        if (PIO_PinGet(&pSpiPins[0]))//来一个时钟上升沿 
            bData |= 0x01;
        DELAY;
    }   
    PIO_PinClear(&pSpiPins[2]);
    DELAY;
    return bData;
}

BYTE SPISendByte(BYTE bData)
{
    BYTE i;
    for (i=0; i<8; i++)
    {                
        PIO_PinClear(&pSpiPins[2]);
        DELAY;
        if (bData & 0x80)
            PIO_PinSet(&pSpiPins[1]);
        else
            PIO_PinClear(&pSpiPins[1]);
        DELAY;
        bData <<= 1;       
        PIO_PinSet(&pSpiPins[2]);//上升沿  
        DELAY;
    } 
    PIO_PinClear(&pSpiPins[2]);
    DELAY;
    PIO_PinSet(&pSpiPins[1]);
    DELAY;
    return 1;
}
#endif

void SpiWriteRegister(BYTE bAdd, BYTE bData)
{     
    WaitSemaphore(g_semSPI, 0);
#if HARDWARE_SPI   
    SPI_ChipSelect(SPI0, 1);
    SPI_Write( SPI0, 0, bAdd | 0x80) ;
    SPI_Read( SPI0 ) ;
    SPI_Write( SPI0, 0, bData) ;
    SPI_Read( SPI0 ) ;
    SPI_ReleaseCS( SPI0 );
    
/*    pucSpi0Buffer[0] = bAdd | 0x80;
    pucSpi0Buffer[1] = bData;    
    _SpiMasterTransfer(pucSpi0Buffer, 2); 
    while ( !bRecvDone0 ) ;
    bRecvDone0 = false ;        */
#else
    PIO_PinClear(&pSpiPins[3]);
    DELAY;
    SPISendByte(bAdd | 0x80);   // 最高位为1，写数据
    SPISendByte(bData);
    DELAY;
    PIO_PinSet(&pSpiPins[3]);
    DELAY;
#endif
    SignalSemaphore(g_semSPI);
}

void SpiWriteBurstReg(BYTE bAdd, BYTE *pbBuf, BYTE bLen)
{    
    WaitSemaphore(g_semSPI, 0);
#if HARDWARE_SPI    
    BYTE i;    
    SPI_ChipSelect(SPI0, 1);
    SPI_Write( SPI0, 0, bAdd | 0x80 ) ;
    SPI_Read( SPI0 ) ;                           //可以防止SPI接收溢出
    for (i=0; i<bLen; i++)
    {
        SPI_Write( SPI0, 0, *pbBuf++ ) ;
        SPI_Read( SPI0 ) ;
    }
    SPI_ReleaseCS( SPI0 );       //关片选 
#else
    BYTE i = 0;
    PIO_PinClear(&pSpiPins[3]);
    DELAY;
    SPISendByte(bAdd | 0x80);   // 最高位为1，写数据    
    for (i=0; i<bLen; i++)
    {
        SPISendByte(*pbBuf++);        
    }    
    PIO_PinSet(&pSpiPins[3]);
    DELAY;
#endif
    SignalSemaphore(g_semSPI);
}

BYTE SpiReadRegister(BYTE bAdd)
{    
    WaitSemaphore(g_semSPI, 0);
#if HARDWARE_SPI   
    BYTE bData;
    SPI_ChipSelect(SPI0, 1);
    SPI_Write( SPI0, 0, bAdd ) ;
    bData = SPI_Read( SPI0 ) ;
    SPI_Write( SPI0, 0, 0x00 ) ;
    bData = SPI_Read( SPI0 ) ;
    SPI_ReleaseCS( SPI0 );       //关片选    
    SignalSemaphore(g_semSPI);
    return bData;
#else
    BYTE bData;
    PIO_PinClear(&pSpiPins[3]);
    DELAY;
    SPISendByte(bAdd);   //读数据
    bData = SPIRecvByte();
    DELAY;
    PIO_PinSet(&pSpiPins[3]);
    DELAY;
    SignalSemaphore(g_semSPI);
    return bData; 
#endif    
}

void SpiReadBurstRxFIFO(BYTE bLen)
{        
    BYTE i;
    BYTE bData;
    if (!bLen)  
        return;
     
    unsigned int dwBufLen = g_tWlRx[0].iTail + WL_RECV_BUFSIZE - g_tWlRx[0].iHead - 1;
	while (dwBufLen > WL_RECV_BUFSIZE) 
		dwBufLen -= WL_RECV_BUFSIZE;
    
    if (bLen > dwBufLen)
	{//缓冲区满了，这个数据就丢掉了
		SignalSemaphore(g_semWlRx);
	}
    
    WaitSemaphore(g_semSPI, 0);
      
    SPI_ChipSelect(SPI0, 1);
    SPI_Write( SPI0, 0, 0x7f ) ;
    SPI_Read( SPI0 ) ;                           //可以防止SPI接收溢出
    for (i=0; i<bLen; i++)
    {
        SPI_Write( SPI0, 0, 0x00 ) ;  //0x00是无意义的，任意数据都可以。选00因为00可以将硬件数据传输灯点亮。表示有数据通信,写一个字节是为了给读提供时钟
        bData = SPI_Read( SPI0 ) ;
        g_tWlRx[0].bRx[(g_tWlRx[0].iHead+i)%WL_RECV_BUFSIZE] = bData;        
    }
    g_tWlRx[0].iHead = ((g_tWlRx[0].iHead + bLen)%WL_RECV_BUFSIZE);
    SPI_ReleaseCS( SPI0 );       //关片选  
    g_dwWlRxInterval = g_dwWlRxIntervalConst;
    
    SignalSemaphore(g_semSPI);
    return;     
}

bool CheckWlMode(void)
{
    BYTE bDevType;
    BYTE i;    
    
    for (i=0; i<3; i++)
    {
        bDevType = SpiReadRegister(0);  //SI_4432_DTC     //TI SI模块可以自识别
        if (bDevType == 0x08)   //SI4432
        {        
            DTRACE(1, ("Si4432 : Wireless mode on work!\r\n"));
            return true;
        }
        else
        {
            Sleep(10);
        }
    }
    DTRACE(1, ("Si4432 : Wireless mode is fault!\r\n"));
    return false;
}

void RecvInit(void)
{
    //reset the RX FIFO
    SpiWriteRegister(0x08, 0x02);		
    SpiWriteRegister(0x08, 0x00);
        
    SpiWriteRegister(0x05, 0x93);//Enable RX FIFO Almost Full,Enable Valid Packet Received,Enable CRC Error 												//write 0x03 to the Interrupt Enable 1 register
    SpiWriteRegister(0x06, 0x00); 												//write 0x00 to the Interrupt Enable 2 register
			
    //read interrupt status registers to release all pending interrupts
	SpiReadRegister(0x03);											//read the Interrupt Status1 register
	SpiReadRegister(0x04);											//read the Interrupt Status2 register
                 
    g_bRadioMode = WL_STATE_RX;    
    g_tRxState.wBytesLeft = 0; /////////    
    //g_tRxCnt.dwDelay = GetTick();      //由IDLE 进入 RX  要延时809US	        
    //enable receiver chain again
    SpiWriteRegister(0x07, 0x05);
}

void SetTransRate(BYTE bBaud)
{    
    const BYTE bBaudPara[10][3] = 
    {
        {0x02, 0x75, 0x2c}, //300
        {0x04, 0xea, 0x2c}, //600
        {0x09, 0xd5, 0x2c}, //1200
        {0x13, 0xa9, 0x2c}, //2400
        {0x27, 0x52, 0x2c}, //4800
        {0x4e, 0xa5, 0x2c}, //9600
        {0x9d, 0x49, 0x2c}, //19200
        {0x09, 0xd5, 0x0c}, //38400
        {0x0e, 0xbf, 0x0c}, //57600
        {0x1d, 0x7e, 0x0c}, //115200
    };
    if (bBaud > 9)
        bBaud = 6;
    SpiWriteRegister(SI_4432_DR1R, bBaudPara[bBaud][0]);		//write 0x4E to the TXDataRate 1 register
	SpiWriteRegister(SI_4432_DR0R, bBaudPara[bBaud][1]);	    //write 0xA5 to the TXDataRate 0 register
	SpiWriteRegister(SI_4432_MMC1R, bBaudPara[bBaud][2]);	    ////////////
}

void SI4432SWReset(void)
{
    SpiWriteRegister(0x07, 0x80);  //SW reset 
    
    Sleep(20);

    SpiReadRegister(0x03); //read interrupt status registers to clear the interrupt flags and release NIRQ pin
    SpiReadRegister(0x04);
    
    //set the center frequency to 480 MHz
	SpiWriteRegister(SI_4432_FBSR, 0x60);		//write 0x75 to the Frequency Band Select register             
	SpiWriteRegister(SI_4432_NCF1R, 0x00);	    //write 0xBB to the Nominal Carrier Frequency1 register
	SpiWriteRegister(SI_4432_NCF2R, 0x00);  	//write 0x80 to the Nominal Carrier Frequency0 register
    
    //set the desired TX data rate (19.2kbps)
	//SpiWriteRegister(SI_4432_DR1R, 0x9D);		//write 0x4E to the TXDataRate 1 register
	//SpiWriteRegister(SI_4432_DR0R, 0x49);	    //write 0xA5 to the TXDataRate 0 register
	//SpiWriteRegister(SI_4432_MMC1R, 0x2C);	    ////////////
    //SpiWriteRegister(0x58, 0x80);
    SetTransRate(g_tWlPara.bBaud);
        
    //SpiWriteRegister(SI_4432_TXPR, 0x1F);	    //set the TX power to MAX       //write 0x1F to the TX Power register 
    RfSetPower(g_tWlPara.bPower);
    
	//SpiWriteRegister(SI_4432_FDR, 0x30);		//set the Tx deviation register (+-30kHz)   应该设置为带宽的一半
    SpiWriteRegister(SI_4432_FDR, 0x13);	    //set the Tx deviation register (+-12kHz)    da f = 625HZ
         
    SpiWriteRegister(0x1C, 0x24);		       //0x24->24K	//write 0x1E to the IF Filter Bandwidth register		
    SpiWriteRegister(0x20, 0x34);			   //write 0xD0 to the Clock Recovery Oversampling Ratio register	    
	SpiWriteRegister(0x21, 0x02);			   //write 0x00 to the Clock Recovery Offset 2 register		
	SpiWriteRegister(0x22, 0x75);		       //write 0x9D to the Clock Recovery Offset 1 register		
	SpiWriteRegister(0x23, 0x25);			   //write 0x49 to the Clock Recovery Offset 0 register	

    SpiWriteRegister(0x24, 0x02);			   //write 0x00 to the Clock Recovery Timing Loop Gain 1 register		
	SpiWriteRegister(0x25, 0x5F);			   //write 0x24 to the Clock Recovery Timing Loop Gain 0 register		
	   
    SpiWriteRegister(0x1D, 0x40);			   //write 0x40 to the AFC Loop Gearshift Override register		
	SpiWriteRegister(0x1E, 0x0A);			   //write 0x0A to the AFC Timing Control register		
	SpiWriteRegister(0x2A, 0x14);
    SpiWriteRegister(0x1F, 0x03);
    SpiWriteRegister(0x69, 0x60);

    SpiWriteRegister(0x34, 0x0A);			   //write 0x0A to the Preamble Length register
     
	//set preamble detection threshold to 20bits
	SpiWriteRegister(0x35, 0x2A); 		       //write 0x2A to the Preamble Detection Control  register

	//Disable header bytes; set variable packet length (the length of the payload is defined by the
	//received packet length field of the packet); set the synch word to two bytes long
	SpiWriteRegister(0x33, 0x02);		       //write 0x02 to the Header Control2 register    
	
	//Set the sync word pattern to 0x2DD4
	SpiWriteRegister(0x36, 0x2D);	 	      //write 0x2D to the Sync Word 3 register
	SpiWriteRegister(0x37, 0xD4);		      //write 0xD4 to the Sync Word 2 register
    
	//enable the WL_STATE_TX & WL_STATE_RX packet handler and CRC-16 (IBM) check
	SpiWriteRegister(0x30, 0x8D);			 //write 0x8D to the Data Access Control register
	//Disable the receive header filters
  	
    SpiWriteRegister(0x32, 0x00 );			//write 0x00 to the Header Control1 register            
	//enable FIFO mode and GFSK modulation
	SpiWriteRegister(0x71, 0x63);		
    
    /*set the GPIO's according the testcard type*/           
    SpiWriteRegister(0x0B, 0x14);
  	SpiWriteRegister(0x0C, 0x12);		   //write 0x12 to the GPIO1 Configuration(set TX state)
	SpiWriteRegister(0x0D, 0x15);		   //write 0x15 to the GPIO2 Configuration(set the RX state) 
 
    SpiWriteRegister(0x79, g_tWlPara.bWlCh);             //Frequency Hopping Channel Select
    SpiWriteRegister(0x7a, 50);            //Frequency Hopping Step Size  50*10=500Khz
    
    SpiWriteRegister(0x7d, 33);            //发送产生中断的阈值，默认4
    SpiWriteRegister(0x7e, 32);            //接收产生中断的阈值，默认55
	/*set the non-default Si443x registers*/
	//set  Crystal Oscillator Load Capacitance register
	SpiWriteRegister(0x09, 0x7F);		  //write 0xD7 to the Crystal Oscillator Load Capacitance register
	
    SpiWriteRegister(0x08, 0x01);			//	reset TX 							//write 0x02 to the Operating Function Control 2 register
    SpiWriteRegister(0x08, 0x00);
    
    RecvInit();
}

void SI4432Init(void)//void SI4432Init(void)
{
    g_semWlUser = NewSemaphore(1, 1);
    g_semWlSyncRx = NewSemaphore(1, 1);
    g_semWlSyncTx = NewSemaphore(1, 1);
    g_semWlRx = NewSemaphore(0, 1);
    g_semWlTxRx = NewSemaphore(0, 1);
    
    g_fWlOpen = false;
    
    g_dwWlRxIntervalConst = 0;
    g_dwWlRxInterval = 0;
    
    WlModeOn();  //开机
        
    /*SpiReadRegister(0x03); //read interrupt status registers to clear the interrupt flags and release NIRQ pin
    SpiReadRegister(0x04); */
        
    CfgWlIntPin();
        
    NewThread("WTX", WirelessTxRxTask, NULL, 200, THREAD_PRIORITY_NORMAL); //128
}

void SI4432ReInit(void)
{
    WlModeOn();  //开机
    Sleep(30); //16.8ms 
    
    CheckWlMode();
    
    /*SpiReadRegister(0x03); //read interrupt status registers to clear the interrupt flags and release NIRQ pin
    SpiReadRegister(0x04); */
    
    SI4432SWReset(); 
}

bool SI4432Open(DWORD dwBaudRate)//设置空中波特率
{
    WaitSemaphore(g_semWlUser, 0);

    if (dwBaudRate == 0)
        dwBaudRate = 19200;
    g_dwWlRxIntervalConst = (1000 * 8 * (32+3))/dwBaudRate; //无线收到32个字节才会中断，留3字节余量。留的字节应该超过1ms的传输时间
    if (g_dwWlRxIntervalConst < 4) g_dwWlRxIntervalConst = 4;
        
    SI4432ReInit();
    
    g_fWlOpen = true;
    return true;
}

bool SI4432Close(void)
{
    SignalSemaphore(g_semWlUser);
    g_fWlOpen = false;
    return true;
}

//在系统1ms定时器中断中调用
void IsrWlRxTimeout(void)
{
  	if (g_dwWlRxInterval)
  	{     	
    	g_dwWlRxInterval--;
    	if (g_dwWlRxInterval==0)			//表示一个连续的帧结束
			SignalSemaphoreFromISR(g_semWlRx);
  	}	
}

WORD SI4432Read(BYTE *pbBuf, WORD wMaxBufLen, DWORD dwTimeoutMs)
{
    BYTE *p = (BYTE *)pbBuf;
    DWORD dwDataLen;
    
    if (!g_fWlOpen)
		return 0;    
    
    WaitSemaphore(g_semWlSyncRx, 0);
    if (dwTimeoutMs)
		WaitSemaphore(g_semWlRx, dwTimeoutMs);
        
    if (pbBuf == NULL)  //特殊处理，清除数据
    {
        g_tWlRx[0].iTail = g_tWlRx[0].iHead;
        SignalSemaphore(g_semWlSyncRx);
    	return 0;
    }

	dwDataLen = g_tWlRx[0].iHead + WL_RECV_BUFSIZE - g_tWlRx[0].iTail;
	while (dwDataLen >= WL_RECV_BUFSIZE)
		dwDataLen -= WL_RECV_BUFSIZE;
	if (dwDataLen > 0)
	{
		if (dwDataLen > wMaxBufLen)
			dwDataLen = wMaxBufLen;
		for (int i=0; i<dwDataLen; i++)
			*p++ = g_tWlRx[0].bRx[(g_tWlRx[0].iTail + i)%WL_RECV_BUFSIZE];
		g_tWlRx[0].iTail = ((g_tWlRx[0].iTail + dwDataLen)%WL_RECV_BUFSIZE);
	}
    
    SignalSemaphore(g_semWlSyncRx);
    WlReadHook(dwDataLen);   //点灯之类工作
    return dwDataLen;
}

//一次最多发送255字节
BYTE SI4432Write(BYTE *pbTxBuf, BYTE bLen, DWORD dwTimeouts)  //发送可以死等，也可以用状态机（必须有专门发送缓存区）。
{        
    if (!g_fWlOpen)
		return 0;
    
    WaitSemaphore(g_semWlSyncTx, 0);
    //reset the TX FIFO		
    SpiWriteRegister(0x08, 0x01);		//write 0x02 to the Operating Function Control 2 register
    SpiWriteRegister(0x08, 0x00);	
    //disable the receiver chain  (but keep the XTAL running to have shorter TX on time)    
    SpiWriteRegister(0x07, 0x01);	            //write 0x01 to the Operating Function Control 1 register	
	
    //The Tx deviation register has to set according to the deviation before every tansmission
    //SpiWriteRegister(SI_4432_FDR, 0x08);  
    g_tTxState.pbBufIndex = pbTxBuf;
    g_bRadioMode = WL_STATE_TX;    
    //----------------------------------------------------------------------------------------
    SpiWriteRegister(0x3E, bLen);				//write 8 to the Transmit Packet Length register	 //发送长度字节 
    g_tTxState.wBytesLeft = bLen;
    if (g_tTxState.wBytesLeft <= FIFO_SIZE)
    {
        SpiWriteBurstReg(0x7f, pbTxBuf, g_tTxState.wBytesLeft);            
        g_tTxState.wBytesLeft = 0;
        g_tTxState.pbBufIndex = pbTxBuf + g_tTxState.wBytesLeft;
        SpiWriteRegister(0x05, 0x84);//write 0x04 to the Interrupt Enable 1 register	//打开允许发送完成中断        
    }
    else
    {
        SpiWriteBurstReg(0x7f, pbTxBuf, FIFO_SIZE); // Fill up the TX FIFO
        g_tTxState.wBytesLeft -= FIFO_SIZE;
        g_tTxState.pbBufIndex = pbTxBuf + FIFO_SIZE;
        SpiWriteRegister(0x05, 0xa0);//write 0x04 to the Interrupt Enable 1 register	//打开允许发送FIFO阈值中断
    }
    //Disable all other interrupts and enable the packet sent interrupt only.
    //This will be used for indicating the successfull packet transmission for the MCU
    SpiWriteRegister(0x06, 0x00);													//write 0x00 to the Interrupt Enable 2 register	
	//Read interrupt status regsiters. It clear all pending interrupts and the nIRQ pin goes back to high.
	SpiReadRegister(0x03);											//read the Interrupt Status1 register
	SpiReadRegister(0x04);											//read the Interrupt Status2 register
    //enable transmitter
	//The radio forms the packet and send it automatically.	
    SpiWriteRegister(0x07, 0x09);

    return bLen;  //这里只是将第一帧数据送给了无线，并启动了无线的发送。并没等将所有数据发完.剩下的数据由无线收发线程处理
}

//接收采用状态机控制
//BYTE g_bRadioMode;
bool CheckWlModeStat(void)
{       
    BYTE bDevStat = SpiReadRegister(0x02);   //Device Status
    if ((bDevStat&0xc0) != 0x00)    //有溢出
    {
        RecvInit();
        SignalSemaphore(g_semWlSyncTx);
    }                  //发送完之后进入Idle State
    if (((g_bRadioMode==WL_STATE_TX) && ((bDevStat&0x03)==0x01))     //RX State    无线模块状态不正确
        || ((g_bRadioMode==WL_STATE_RX) && ((bDevStat&0x03)==0x02))) //TX State    无线模块状态不正确
    {
        RecvInit();
        SignalSemaphore(g_semWlSyncTx);
    }

    //诊断无线模块是否有复位的情况
    bDevStat = SpiReadRegister(0x6e);   
    if (bDevStat == 0x0a) //模块复位
    {
        SI4432SWReset();   //软复位下        
        SignalSemaphore(g_semWlSyncTx);
    }
    return true;
}

void SI4432SetFrq(BYTE bCh)
{
    SpiWriteRegister(0x79, bCh);
}

//返回的是对应的DB值
int GetRSSI(void)
{
    BYTE bRSSI;
    int iRSSI;
    bRSSI = SpiReadRegister(0x26);
    iRSSI = ((80*bRSSI)-20780)/159;    //SI没有这个公式只有图表，此公式由图表简单总结而成，只能简单反映RSSI   //分段函数行不通
    return iRSSI;
}

void RfSetPower(BYTE bVal)
{
    const BYTE bPower[8] = {0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
                                //Si(db):1,  2,  5,  8, 11, 14, 17, 20    
    if (bVal > 7)
        bVal = 7;  //100mw
    SpiWriteRegister(0x6d, bPower[bVal]);
}

/**
 *  \brief Handler for Wireless falling edge interrupt.
 *
 *  Set Wireless event flag (WirelessEvt).
 */
static void WlMode_Handler( const Pin *pin )
{    
    /* For debounce, we disable the left-button isr */
    PIO_PinDisableIt(&PinWlGdo0Int) ;
    
    if ( pin->mask == PinWlGdo0Int.mask && pin->id == PinWlGdo0Int.id )//对应引脚的中断
    {
        SignalSemaphoreFromISR(g_semWlTxRx);//发送一个信号量
    }
    else
    {
    }

    /* Enable this isr */
    PIO_PinEnableIt(&PinWlGdo0Int) ;
}

void CfgWlIntPin(void)
{    
    /* Configure pios as inputs. */
    PIO_PinConfigure(&PinWlGdo0Int, 1);
    /* Adjust pio debounce filter parameters, uses 10 Hz filter. */
    //PIO_PinSetDebounceFilter(&PinWlGdo0Int, 10);
    /* Initialize pios interrupt handlers, see PIO definition in board.h. */
    PIO_PinConfigureIt(&PinWlGdo0Int, WlMode_Handler); 
    /* Enable PIO controller IRQs. */
    NVIC_EnableIRQ(PIOA_IRQn);
    /* Enable PIO line interrupts. */
    PIO_PinEnableIt(&PinWlGdo0Int);
}

//专门开一个线程是因为这里面要等信号量，在中断里是不能等信号量的。
TThreadRet WirelessTxRxTask(void *pvParameters)
{    
    BYTE bDataLen1;  
    WORD wRet;
    while(1)
    {        
        wRet = WaitSemaphore(g_semWlTxRx, 60000);
        if (wRet == SYS_ERR_TIMEOUT)
        {
            CheckWlModeStat();        //每60s检测一次无线的状态
            continue;
        }
        
        if (g_bRadioMode == WL_STATE_TX)
        {
            bDataLen1 = SpiReadRegister(0x03);				//read the Interrupt Status1 register  clear flag
	        if ((bDataLen1 & 0x80) == 0x80)//FIFO Underflow/Overflow Error
            {
                SpiWriteRegister(0x08, 0x01);	//reset the TX FIFO												//write 0x02 to the Operating Function Control 2 register
                SpiWriteRegister(0x08, 0x00);
                RecvInit();
                SignalSemaphore(g_semWlSyncTx);
            }
            if (g_tTxState.wBytesLeft == 0)//TX finish
            {                
                RecvInit();
                SignalSemaphore(g_semWlSyncTx);
            }
            else  //(add TX data)
            {            
                if (g_tTxState.wBytesLeft <= AVAILABLE_BYTES_IN_TX_FIFO)
                {
                    SpiWriteBurstReg(0x7f, g_tTxState.pbBufIndex, g_tTxState.wBytesLeft);            
                    g_tTxState.wBytesLeft = 0;
                    g_tTxState.pbBufIndex += g_tTxState.wBytesLeft;
                    SpiWriteRegister(0x05, 0x84);//write 0x04 to the Interrupt Enable 1 register	//打开允许发送完成中断    
                }
                else
                {                    
                    SpiWriteBurstReg(0x7f, g_tTxState.pbBufIndex, AVAILABLE_BYTES_IN_TX_FIFO); // Fill up the TX FIFO
                    g_tTxState.wBytesLeft -= AVAILABLE_BYTES_IN_TX_FIFO;
                    g_tTxState.pbBufIndex += AVAILABLE_BYTES_IN_TX_FIFO;
                    SpiWriteRegister(0x05, 0xa0);//write 0x04 to the Interrupt Enable 1 register	//打开允许发送FIFO阈值中断
                }
            }
        }
        else //if (g_bRadioMode == WL_STATE_RX) //(RX)
        {                        
            bDataLen1 = SpiReadRegister(0x03);   //ItStatus1
            if (!g_tRxState.wBytesLeft)  //第一次中断，读取报文长度
                g_tRxState.wBytesLeft = SpiReadRegister(0x4B);	 //read the Received Packet Length register
            
            if (bDataLen1&0x02)//接收完一个报文  //Enable Valid Packet Received
            {   
                //g_tRxCnt.iRSSI = GetRSSISI();
                //g_tRxCnt.iRSSI = g_tCAvoid.iRSSI;
                SpiReadBurstRxFIFO(g_tRxState.wBytesLeft);
                g_tRxState.wBytesLeft = 0; 
                
                //reset the RX FIFO
                SpiWriteRegister(0x08, 0x02);			//write 0x02 to the Operating Function Control 2 register
                SpiWriteRegister(0x08, 0x00);
                
                //enable receiver chain again
    		    SpiWriteRegister(0x07, 0x05);
            }
            else if (bDataLen1&0x10) //(Get RX data)   //Enable RX FIFO Almost Full
            {
                SpiReadBurstRxFIFO(BYTES_IN_RX_FIFO);
                g_tRxState.wBytesLeft -= BYTES_IN_RX_FIFO;
            }
            else    //CRC EEROR
            {
                RecvInit();
                SignalSemaphore(g_semWlSyncTx);
            }
        }
    }
}
