/*********************************************************************************************************
 * Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Comm.h
 * ժ    Ҫ��������,ʵ�ִ������ã���д
 *
 * ��ǰ�汾��
 * ��    �ߣ�
 * ������ڣ�
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
 * ��    ע��1���ð汾ʹ����DMA��ͨ��Ϊ�첽�ķ�ʽ�������÷�������APIʱ���������أ�ʵ�����ʱ������ֻ�Ƿŵ���
 *              DMA���ͻ��������У���û���ں�����������֮�����ݶ������ꡣ�û�����Ҫע����һ�㡣��
************************************************************************************************************/
#include <string.h>
#include "board.h"
#include "FreeRTOS.h"
#include "Sysarch.h"
#include "task.h"
#include "queue.h"
#include "Uarts.h"
#include "pmc.h"
#include "DrvHook.h"
#include "SysCfg.h"
#include "Gpio.h"
#include "pmc.h"

#define UART_PDC_BUFSIZE	64
//#define min(x, y)	(x>y ? y:x)

#define UART_NULL        0   //��ʹ��
#define UART_232         1   //232ģʽ
#define UART_HARD_485    2   //Ӳ��485ģʽ,  NOTE:����4(UART 4)�������ó�Ӳ��485ģʽ
#define UART_SOFT_485    3   //���485ģʽ����Ҫ�Լ�����485��������
#define UART_IRDA        4   //����ģʽ

/** USART0 pin RX */
#define PIN_USART0_RXD    {PIO_PA10, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART0 pin TX */
#define PIN_USART0_TXD    {PIO_PA11, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART0 pin RTS */
#define PIN_USART0_RTS    {PIO_PB25, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}

/** USART1 pin RX */
#define PIN_USART1_RXD    {PIO_PA12, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART1 pin TX */
#define PIN_USART1_TXD    {PIO_PA13, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART1 pin RTS */
#define PIN_USART1_RTS    {PIO_PA14, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
//��Ϊ����ģʽʱ
#define PIN_USART1_TXD_IRDA {PIO_PA13, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}

/** USART2 pin RX */
#define PIN_USART2_RXD    {PIO_PB21, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/** USART2 pin TX */
#define PIN_USART2_TXD    {PIO_PB20, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT} 

#define PIN_USART2_RTS    {PIO_PB22, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}

/** USART3 pin RX */
#define PIN_USART3_RXD    {PIO_PD5, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}  
/** USART3 pin TX */
#define PIN_USART3_TXD    {PIO_PD4, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
/** USART3 pin RTS */
#define PIN_USART3_RTS    {PIO_PF5, PIOF, ID_PIOF, PIO_PERIPH_A, PIO_DEFAULT}

//UART PINS
#define PIN_UART_RXD      {PIO_PA8, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}  
#define PIN_UART_TXD      {PIO_PA9, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

#ifdef SAM3X_EK_BOARD           //�������ϵĴ���ʹ������
#define PIN_USART0_EN     {PIO_PE14, PIOE, ID_PIOE, PIO_OUTPUT_0, PIO_DEFAULT}
static const Pin UsartEnPins = PIN_USART0_EN;
#endif

/** Pins to configure for the application. */
static const Pin Usart0Pins[] = { PIN_USART0_RXD, PIN_USART0_TXD, PIN_USART0_RTS };
static const Pin Usart1Pins[] = { PIN_USART1_RXD, PIN_USART1_TXD, PIN_USART1_RTS };
static const Pin Usart2Pins[] = { PIN_USART2_RXD, PIN_USART2_TXD, PIN_USART2_RTS };
static const Pin Usart3Pins[] = { PIN_USART3_RXD, PIN_USART3_TXD, PIN_USART3_RTS };
static const Pin UartPins[] = { PIN_UART_RXD, PIN_UART_TXD };

static const Pin *UsartsPins[] = {Usart0Pins, Usart1Pins, Usart2Pins, Usart3Pins, UartPins};

#ifdef HARD_CL818K5
//#define PIN_UART_CTRL       {PIO_PC21, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT} //UART ����
//static const Pin UCtrl = PIN_UART_CTRL;
//static const Pin TxIrda = PIN_USART1_TXD_IRDA;
//����ģʽ��������                //  ����1     ����/ά��/����      ����2        GPRS       ���� 
static const BYTE UsartsMode[] = {UART_HARD_485, UART_HARD_485, UART_HARD_485, UART_232, UART_232};
//���ô���485�����������
//static const Pin *Uart485Ctrl[] = {NULL, NULL, NULL, NULL, &UCtrl};
static const Pin *Uart485Ctrl[] = {NULL, NULL, NULL, NULL, NULL};
#endif

#ifdef HARD_CL7614B1
//����ģʽ��������
static const BYTE UsartsMode[] = {UART_232, UART_232, UART_HARD_485, UART_NULL, UART_NULL};
//���ô���485��������
static const Pin *Uart485Ctrl[] = {NULL, NULL, NULL, NULL, NULL};
#endif

#ifdef HARD_CL45055
//����ģʽ��������
static const BYTE UsartsMode[] = {UART_232, UART_HARD_485, UART_232, UART_232, UART_232};
//���ô���485��������
static const Pin *Uart485Ctrl[] = {NULL, NULL, NULL, NULL, NULL};
#endif

BYTE  g_bUartRxDMABuffer[MAX_PORT_NUM*2][UART_PDC_BUFSIZE];
BYTE  g_bUartTxDMABuffer[MAX_PORT_NUM][UART_PDC_BUFSIZE];      //����ʡ��

TUartRx g_tUartRx[MAX_PORT_NUM] = { 0 };
TUartTx g_tUartTx[MAX_PORT_NUM] = { 0 };

DWORD g_dwRxInterval[MAX_PORT_NUM];
DWORD g_dwRxIntervalConst[MAX_PORT_NUM];

static TSem g_semUartUser[MAX_PORT_NUM];
static TSem g_semUartSyncRx[MAX_PORT_NUM];
static TSem g_semUartSyncTx[MAX_PORT_NUM];
static TSem g_semUartRx[MAX_PORT_NUM] = {NULL};
static bool g_fPortOpen[MAX_PORT_NUM];

typedef struct
{
	BYTE *pbRx;
    BYTE bRxIdx;
	WORD wOffset;
	WORD wPDCSize;
} TUartDMARxMap;

TUartDMARxMap g_tUartDMARxMap[MAX_PORT_NUM];

void CtrlSend(BYTE bPort)
{
    Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
    if ((bPort >= MAX_PORT_NUM) || (Uart485Ctrl[bPort]==NULL))
		return;
    
    if (UsartsMode[bPort] == UART_SOFT_485)
    {
        if (Uart485Ctrl[bPort] != NULL)
            Gpio_Set(Uart485Ctrl[bPort]);  
        return;
    }
    else if (UsartsMode[bPort] == UART_IRDA)
    {
        Gpio_Config(&UsartsPins[bPort][1], 1);
        pUsart[bPort]->US_CR = US_CR_RXDIS; /* Disable Receiver. */
        pUsart[bPort]->US_CR = US_CR_TXEN;  /* Enable transmitter. */
    }       
}

void CtrlRecv(BYTE bPort)
{
    //Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
    if ((bPort >= MAX_PORT_NUM) || (Uart485Ctrl[bPort]==NULL))
		return;
    
    if (UsartsMode[bPort] == UART_SOFT_485)
    {
        if (Uart485Ctrl[bPort] != NULL)
            Gpio_Clear(Uart485Ctrl[bPort]); 
        return;
    }
    else if (UsartsMode[bPort] == UART_IRDA)
    {
        /*pUsart[bPort]->US_CR = US_CR_TXDIS;
        Gpio_Config(&TxIrda, 1);
        Gpio_Clear(&TxIrda); 
        pUsart[bPort]->US_CR = US_CR_RXEN;*/
    }    
}

static void RxFromDMA(WORD wPort)
{
	unsigned short head;
	unsigned short tail;
	unsigned short count;	
	unsigned char rxdata = 0;
	Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
    
    if (wPort >= MAX_PORT_NUM)
		return;
	
	if (g_tUartDMARxMap[wPort].wPDCSize == 0)//���ڻ�û��ʼ����DMA��ûӳ��
		return;
	
	do
	{
		// Reset the UART timeout early so that we don't miss one
		pUsart[wPort]->US_CR = US_CR_STTTO;
        
		head = pUsart[wPort]->US_RPR - (unsigned int)g_tUartDMARxMap[wPort].pbRx;
		tail = g_tUartDMARxMap[wPort].wOffset;
		// If the PDC has switched buffers, RPR won't contain
		// any address within the current buffer. Since head
		// is unsigned, we just need a one-way comparison to
		// find out.
		// In this case, we just need to consume the entire
		// buffer and resubmit it for DMA. This will clear the
		// ENDRX bit as well, so that we can safely re-enable
		// all interrupts below.
		head = min(head, g_tUartDMARxMap[wPort].wPDCSize);
		if (head != tail)
		{
			// head will only wrap around when we recycle
			// the DMA buffer, and when that happens, we
			// explicitly set tail to 0. So head will
			// always be greater than tail.
			count = head - tail;
			//copy data to user buffer
			unsigned int dwBufLen = g_tUartRx[wPort].wTail + UART_RECV_BUFSIZE - g_tUartRx[wPort].wHead - 1;//����������ʣһ���ֽ�
			while (dwBufLen > UART_RECV_BUFSIZE) 
				dwBufLen -= UART_RECV_BUFSIZE;
            
            if (dwBufLen < (UART_RECV_BUFSIZE>>1)) //�����Ҫ���ˣ���ǰ֪ͨӦ�ý�����ȡ��,����ᶪ��
                SignalSemaphoreFromISR(g_semUartRx[wPort]);
            
			if (count >= dwBufLen)
			{//���������ˣ�֪ͨ����
    			SignalSemaphoreFromISR(g_semUartRx[wPort]);                
				count = dwBufLen;
			} 
			//if (count == 0)  //DMA�����
				//break;
            
			for (int i=0; i<count; i++)
				g_tUartRx[wPort].bRx[(g_tUartRx[wPort].wHead+i)%UART_RECV_BUFSIZE] = *(g_tUartDMARxMap[wPort].pbRx+((tail+i)%g_tUartDMARxMap[wPort].wPDCSize));
			g_tUartRx[wPort].wHead = ((g_tUartRx[wPort].wHead + count)%UART_RECV_BUFSIZE);
			g_tUartDMARxMap[wPort].wOffset = head;//g_tUartDMARxMap[wPort].wOffset += count; ������붪���������жϻ�һֱ����
			rxdata = 1;            
		}
		
		// If the current buffer is full, we need to check if
		// the next one contains any additional data.
		if ((head>=g_tUartDMARxMap[wPort].wPDCSize) || ((head==tail)&&(pUsart[wPort]->US_CSR&US_CSR_RXBUFF)))//����жϲ���ʱ��ͷ��׷����β��ҲӦ���л�PDC����һֱ�ж�
		{
			g_tUartDMARxMap[wPort].wOffset = 0;			
			pUsart[wPort]->US_RNPR = (unsigned int)&g_bUartRxDMABuffer[wPort*2+g_tUartDMARxMap[wPort].bRxIdx][0]; //���US_CSR_RXBUFFΪ1����US_RNPR��ֵʱ�����Զ����ϸ���RPR
			pUsart[wPort]->US_RNCR = g_tUartDMARxMap[wPort].wPDCSize;			
			g_tUartDMARxMap[wPort].bRxIdx = (g_tUartDMARxMap[wPort].bRxIdx == 0 ? 1:0);
			g_tUartDMARxMap[wPort].pbRx = &g_bUartRxDMABuffer[wPort*2+g_tUartDMARxMap[wPort].bRxIdx][0];
		}
	} while (head >= g_tUartDMARxMap[wPort].wPDCSize);
	pUsart[wPort]->US_IER = US_IER_ENDRX | US_IER_TIMEOUT | US_IER_RXBUFF;	
	if (rxdata > 0)
	{
		if (wPort < MAX_PORT_NUM)
			g_dwRxInterval[wPort] = g_dwRxIntervalConst[wPort];
	}
}

static void TxFromDMA(WORD wPort)
{
	Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
	DWORD dwDataLen;
    
    if (wPort >= MAX_PORT_NUM)
		return;

	//PDC transmitting
	if (pUsart[wPort]->US_TCR != 0)
	{
		pUsart[wPort]->US_IER = US_IER_ENDTX | US_IER_TXBUFE;//��Ϊ������ʱ���ж�������	
		return;
	}
	
	// disable PDC transmit
	pUsart[wPort]->US_PTCR = US_PTCR_TXTDIS;
	// nothing left to transmit?	
	if (g_tUartTx[wPort].wTail == g_tUartTx[wPort].wHead) //�������˿������ε������Ϳ�ʼ��ʱ����
        return;	
	
	dwDataLen = g_tUartTx[wPort].wHead + UART_SEND_BUFSIZE - g_tUartTx[wPort].wTail;
	while (dwDataLen >= UART_SEND_BUFSIZE)
		dwDataLen -= UART_SEND_BUFSIZE;

	if (dwDataLen > UART_PDC_BUFSIZE)
		dwDataLen = UART_PDC_BUFSIZE;
	for (int i=0; i<dwDataLen; i++)
		g_bUartTxDMABuffer[wPort][i] = g_tUartTx[wPort].bTx[(g_tUartTx[wPort].wTail+i)%UART_SEND_BUFSIZE];
	g_tUartTx[wPort].wTail = ((g_tUartTx[wPort].wTail+dwDataLen)%UART_SEND_BUFSIZE);
	
	pUsart[wPort]->US_PTCR = US_PTCR_TXTDIS;
	pUsart[wPort]->US_TPR = (unsigned int)&g_bUartTxDMABuffer[wPort][0];
	pUsart[wPort]->US_TCR = dwDataLen;
		
	// re-enable PDC transmit and interrupts
	pUsart[wPort]->US_PTCR = US_PTCR_TXTEN;
	pUsart[wPort]->US_IER = US_IER_ENDTX | US_IER_TXBUFE;
}

static void HandleRecv(WORD wPort, unsigned int pending)
{		
	Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
	
	if (pending & (US_IMR_ENDRX | US_IMR_TIMEOUT | US_IMR_RXBUFF))     
	{
		pUsart[wPort]->US_IDR = US_IDR_ENDRX | US_IDR_TIMEOUT | US_IDR_RXBUFF;

		RxFromDMA(wPort);
	}
	
	if (pending & ( US_IMR_RXBRK | US_IMR_OVRE | US_IMR_FRAME | US_IMR_PARE))//  |US_IMR_RXBUFF ��������
	{
		// clear error
		pUsart[wPort]->US_CR = US_CR_RSTSTA;
	}	
}

static void HandleStatus(WORD wPort, unsigned int pending, unsigned int csr)
{
	//deal flow control, we do not need flow control, nothing to do
}

static void HandleTrans(WORD wPort, unsigned int pending)
{
	Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
		
	// PDC transmit
	if (pending  & (US_IMR_ENDTX | US_IMR_TXBUFE))
	{
		pUsart[wPort]->US_IDR = US_IDR_ENDTX | US_IDR_TXBUFE;
		TxFromDMA(wPort);
	}
}

void USART0_Handler()
{
    __disable_interrupt();
    unsigned int csr, pending, passcnt = 0;
	
	do
	{
		csr = USART0->US_CSR;
		pending = csr & USART0->US_IMR;
		if (!pending) break;
		HandleRecv(0, pending);
		HandleStatus(0, pending, csr);
		HandleTrans(0, pending);
	}while (++passcnt < 1);//32//256    64*32=2K
    __enable_interrupt();
}

void USART1_Handler()
{
    __disable_interrupt();
    unsigned int csr, pending, passcnt = 0;
	
	do
	{
		csr = USART1->US_CSR;
		pending = csr & USART1->US_IMR;
		if (!pending) break;
		HandleRecv(1, pending);
		HandleStatus(1, pending, csr);
		HandleTrans(1, pending);
	}while (++passcnt < 1);
    __enable_interrupt();
}

void USART2_Handler()
{
    __disable_interrupt();
    unsigned int csr, pending, passcnt = 0;
	
	do
	{
		csr = USART2->US_CSR;
		pending = csr & USART2->US_IMR;
		if (!pending) break;
		HandleRecv(2, pending);
		HandleStatus(2, pending, csr);
		HandleTrans(2, pending);
	}while (++passcnt < 1);
    __enable_interrupt();
}

void USART3_Handler()
{
    __disable_interrupt();
    unsigned int csr, pending, passcnt = 0;
	
	do
	{
		csr = USART3->US_CSR;
		pending = csr & USART3->US_IMR;
		if (!pending) break;
		HandleRecv(3, pending);
		HandleStatus(3, pending, csr);
		HandleTrans(3, pending);
	}while (++passcnt < 1);
    __enable_interrupt();
}

void UART_Handler()
{
    __disable_interrupt();
    Usart* pUsart = (Usart*) UART;
    unsigned int csr, pending, passcnt = 0;

	do
	{
		csr = pUsart->US_CSR;
		pending = csr & pUsart->US_IMR;
		if (!pending) break;
		HandleRecv(4, pending);
		HandleStatus(4, pending, csr);
		HandleTrans(4, pending);
	}while (++passcnt < 1);
    __enable_interrupt();
}

void IsrUartRxTimeout(WORD wPort)
{
    if (wPort >= MAX_PORT_NUM)
		return;
    
    if (UsartsMode[wPort] == UART_NULL) //����Ϊ����ʹ��
        return;
    
  	if (g_dwRxInterval[wPort])
  	{     	
    	g_dwRxInterval[wPort]--;
    	if (g_dwRxInterval[wPort]==0)			//��ʾһ��������֡����
			SignalSemaphoreFromISR(g_semUartRx[wPort]);
  	}
}

bool UartInit(WORD wPort)
{
    if (wPort >= MAX_PORT_NUM)
        return false;
    
    g_semUartUser[wPort] = NewSemaphore(1, 1);
    g_semUartSyncRx[wPort] = NewSemaphore(1, 1);
    g_semUartSyncTx[wPort] = NewSemaphore(1, 1);
    g_semUartRx[wPort] = NewSemaphore(0, 1);
        
    if (UsartsMode[wPort] == UART_NULL) //����Ϊ����ʹ��
    {        
    }
    else if ((UsartsMode[wPort] == UART_HARD_485) && (wPort != 4)) //Ӳ��485
    {
        Gpio_Config(UsartsPins[wPort], 3);
    }
    else if (UsartsMode[wPort] == UART_IRDA)//����
    {        
    }
    else //232�����485
    {
        Gpio_Config(UsartsPins[wPort], 2);
        
        if ((Uart485Ctrl[wPort]!=NULL) && (UsartsMode[wPort]==UART_SOFT_485)) //���485
            Gpio_Config(Uart485Ctrl[wPort], pdTRUE);        
    }
    
#ifdef SAM3X_EK_BOARD
    if (wPort == 0)
        Gpio_Config(&UsartEnPins, pdTRUE); 
#endif
    
	g_fPortOpen[wPort] = false;
	g_dwRxIntervalConst[wPort] = 0;
	g_dwRxInterval[wPort] = 0; 
    
	return true;
}

//�������򿪴���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits  ֹͣλ
//      @bParity  У��λ
//bool CUart::Open(DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
bool UartOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
{
    // Reset and disable receiver & transmitter
	const unsigned int dwParityTab[] = { US_MR_PAR_NO, US_MR_PAR_ODD, US_MR_PAR_EVEN };
	const unsigned int dwStopBitsTab[] = { US_MR_NBSTOP_1_BIT, US_MR_NBSTOP_1_5_BIT, US_MR_NBSTOP_2_BIT };
	const unsigned int dwByteSizeTab[] = { US_MR_CHRL_8_BIT, US_MR_CHRL_8_BIT, US_MR_CHRL_8_BIT, US_MR_CHRL_8_BIT, 
						   			US_MR_CHRL_8_BIT, US_MR_CHRL_5_BIT , US_MR_CHRL_6_BIT, US_MR_CHRL_7_BIT,
						   			US_MR_CHRL_8_BIT};
   
    //unsigned int dwIrqID[] = {17, 18, 19, 20};
	if (wPort >= MAX_PORT_NUM)
		return false;

	if (dwBaudRate == 0)
		return false;

	if (bParity > 2)
		return false;

	if (bStopBits > 2)
		return false;

	if (bByteSize > 8)
		return false;
    
    if (UsartsMode[wPort] == UART_NULL) //����Ϊ����ʹ��
        return false;
	
	Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
	const unsigned int dwDevID[] = { ID_USART0, ID_USART1, ID_USART2, ID_USART3, ID_UART };

	WaitSemaphore(g_semUartUser[wPort], SYS_TO_INFINITE);
    
    // enable peripheral clock
    if (pmc_is_periph_clk_enabled(dwDevID[wPort]))
    {
    }
    else
    {
        pmc_enable_periph_clk( dwDevID[wPort] ) ;
    }
	   
    //disable PDC receive
    pUsart[wPort]->US_PTCR = US_PTCR_RXTDIS;
    pUsart[wPort]->US_IDR = US_IDR_ENDRX | US_IDR_TIMEOUT | US_IDR_RXBUFF;
    
    // disable PDC transmit
    pUsart[wPort]->US_PTCR = US_PTCR_TXTDIS;
    pUsart[wPort]->US_IDR = US_IDR_ENDTX | US_IDR_TXBUFE;
    
    // Disable all interrupts, port and break condition.
    pUsart[wPort]->US_CR = US_CR_RSTSTA;
    pUsart[wPort]->US_IDR = 0xffffffff;
    
    // Configure mode
    if (wPort == 4)//����4û��Ӳ��485����,�����������485����
    {
        pUsart[wPort]->US_MR = US_MR_USCLKS_MCK | US_MR_CHMODE_NORMAL | dwParityTab[bParity] | dwStopBitsTab[bStopBits] | dwByteSizeTab[bByteSize];
    }
    else
    {
        if (UsartsMode[wPort] == UART_HARD_485) //485ģʽ    ���485ģʽ    ����RTS��485�����շ�״̬���п���
        {
            pUsart[wPort]->US_MR = US_MR_USCLKS_MCK | US_MR_CHMODE_NORMAL | US_MR_USART_MODE_RS485 | dwParityTab[bParity] | dwStopBitsTab[bStopBits] | dwByteSizeTab[bByteSize];
        }
        else if (UsartsMode[wPort] == UART_IRDA)//����ģʽ
        {
            pUsart[wPort]->US_IF = 4;
            pUsart[wPort]->US_MR = US_MR_USCLKS_MCK | US_MR_CHMODE_NORMAL | US_MR_USART_MODE_IRDA | dwParityTab[bParity] | dwStopBitsTab[bStopBits] | dwByteSizeTab[bByteSize];
        }
        else
        {
            pUsart[wPort]->US_MR = US_MR_USCLKS_MCK | US_MR_CHMODE_NORMAL | dwParityTab[bParity] | dwStopBitsTab[bStopBits] | dwByteSizeTab[bByteSize];
        }
    }
    
    DWORD dwBaud = ((BOARD_MCK*10) / dwBaudRate) / 16;
    dwBaud = (dwBaud + 5)/10;       //��������
    pUsart[wPort]->US_BRGR = dwBaud;     

    // Disable the interrupt first
//    AT91C_BASE_AIC->AIC_IDCR = 1 << dwDevID[wPort];   //zqq to do �رմ����ж�

    // Configure mode and handler
//    AT91C_BASE_AIC->AIC_SMR[(AT91C_ID_US0+wPort)] = 0;    //zqq to do
//    AT91C_BASE_AIC->AIC_SVR[(AT91C_ID_US0+wPort)] = (unsigned int)g_isrUart[wPort];   //zqq to do ע���жϺ���
    
    // Clear interrupt
//    AT91C_BASE_AIC->AIC_ICCR = 1 << dwDevID[wPort];       //SAM3X8E do not need
    
    //Enables interrupts coming from the given (unique) source
//    AT91C_BASE_AIC->AIC_IECR = 1 << dwDevID[wPort];       //SAM3X8E do not need

    g_tUartDMARxMap[wPort].pbRx = &g_bUartRxDMABuffer[wPort*2][0];
    g_tUartDMARxMap[wPort].bRxIdx = 0;
    g_tUartDMARxMap[wPort].wOffset = 0;
    if (wPort == 4) //д4��ͨ��   //UARTӲ����֧�ֽ��ճ�ʱ�ж�,�����Զ����һ֡�����Ӷ������ж�
        g_tUartDMARxMap[wPort].wPDCSize = 1;
    else
        g_tUartDMARxMap[wPort].wPDCSize = UART_PDC_BUFSIZE;
    
    // setup first PDC bank
    pUsart[wPort]->US_RPR = (unsigned int)&g_bUartRxDMABuffer[0+wPort*2][0];
    pUsart[wPort]->US_RCR = g_tUartDMARxMap[wPort].wPDCSize;
    // setup second PDC bank
    pUsart[wPort]->US_RNPR = (unsigned int)&g_bUartRxDMABuffer[1+wPort*2][0];
    pUsart[wPort]->US_RNCR = g_tUartDMARxMap[wPort].wPDCSize;
  
    // Finally, Reset Status Bits and Receiver
    pUsart[wPort]->US_CR = US_CR_RSTSTA | US_CR_RSTRX;
    // enable xmit & rcvr
    pUsart[wPort]->US_CR = US_CR_TXEN | US_CR_RXEN;

    // set UART timeout
    pUsart[wPort]->US_RTOR = US_RTOR_TO(3*10);//PDC_RX_TIMEOUT;
    pUsart[wPort]->US_TTGR = (2);       //ֹͣλ���һ��ͣ��
    pUsart[wPort]->US_CR = US_CR_STTTO;
    pUsart[wPort]->US_IER = US_IER_ENDRX | US_IER_TIMEOUT | US_IER_RXBUFF;
    pUsart[wPort]->US_IER = US_IER_OVRE | US_IER_FRAME | US_IER_PARE | US_IER_RXBRK;
  
    // enable PDC controller
    pUsart[wPort]->US_PTCR = US_PTCR_RXTEN;
    
    g_dwRxIntervalConst[wPort] = (1000 * 3 * 11)/dwBaudRate;//2*(1000 * 5 * 11)/dwBaudRate; //3B,11BIT,1000MS    
    if (g_dwRxIntervalConst[wPort] < 2) g_dwRxIntervalConst[wPort] = 2;      //10  
    
    CtrlRecv(wPort);
    
    NVIC_SetPriority(dwDevID[wPort], 9);
	
    /* Configure the RXBUFF interrupt */
    NVIC_EnableIRQ( dwDevID[wPort] );    

	g_fPortOpen[wPort] = true;
	return true;
}

//�������رմ���
//��������
//���أ�0-û�д���
bool UartClose(WORD wPort)
{
	Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
	
	if (wPort >= MAX_PORT_NUM)
		return false;
    
    if (UsartsMode[wPort] == UART_NULL) //����Ϊ����ʹ��
        return false;
    
	SignalSemaphore(g_semUartUser[wPort]);
   	      
    //disable PDC receive
    pUsart[wPort]->US_PTCR = US_PTCR_RXTDIS;
    pUsart[wPort]->US_IDR = US_IDR_ENDRX | US_IDR_TIMEOUT | US_IDR_RXBUFF;
    
    // disable PDC transmit
    pUsart[wPort]->US_PTCR = US_PTCR_TXTDIS;
    pUsart[wPort]->US_IDR = US_IDR_ENDTX | US_IDR_TXBUFE;
    
    // Disable all interrupts, port and break condition.
    pUsart[wPort]->US_CR = US_CR_RSTSTA;
    pUsart[wPort]->US_IDR = 0xffffffff;
    
	g_fPortOpen[wPort] = false;
	return true;
}

//ģ�⴮�ڷ���Ϊ������ʽ(ͬ��)
//�Դ����������ڼ�Debug��Ϊ�������ķ�ʽ(�첽)
//DWORD CUart::Write(BYTE *pbBuf, WORD wTxLen, DWORD dwTimeoutMs)
//������ͨ�����ڷ�������
//������@wPort ���ں�
//      @pbData �����͵����ݻ�����
//      @dwDataLen  �����͵����ݳ���
//      @dwTimeouts ���ͳ�ʱʱ��(�������ʱ����û��������Ϊ����ʧ��)
//���أ��ɹ����ͳ�ȥ�����ݳ���
WORD UartWrite(WORD wPort, BYTE* pbBuf, WORD wTxLen, DWORD dwTimeoutMs)
{
	WORD wSentLen;
	DWORD dwBufLen;
	int	iTxHead;

	DWORD dwTick = GetTick();
	Usart* pUsart[] = { USART0, USART1, USART2, USART3, (Usart*) UART };
    WORD wTxLenTmp = wTxLen;	
    BYTE *p = pbBuf;
    bool fFinal = false;
                
	if (wPort >= MAX_PORT_NUM)
		return 0;
    
    if (UsartsMode[wPort] == UART_NULL) //����Ϊ����ʹ��
        return 0;

	if (!g_fPortOpen[wPort])
		return 0;
    
    UartWriteHook(wPort, wTxLen);     //���֮��
    wTxLen = 0;
	WaitSemaphore(g_semUartSyncTx[wPort], SYS_TO_INFINITE);
	CtrlSend(wPort);
	while (wTxLenTmp > 0)
    {               
        if  (!pUsart[wPort]->US_TCR)   //PDC�������е����ݷ���
        {       
        	dwBufLen = g_tUartTx[wPort].wTail + UART_SEND_BUFSIZE - g_tUartTx[wPort].wHead - 1;
        	while (dwBufLen > UART_SEND_BUFSIZE)
        		dwBufLen -= UART_SEND_BUFSIZE;
        	if (dwBufLen == 0)
        	{//����������
        		while (GetTick()-dwTick <  dwTimeoutMs)
        		{
                    Sleep(0);
        			if  (!pUsart[wPort]->US_TCR)//transmitter is already runing, wait.
        				break;
        		}
        		dwBufLen = g_tUartTx[wPort].wTail + UART_SEND_BUFSIZE - g_tUartTx[wPort].wHead - 1;
        		while (dwBufLen > UART_SEND_BUFSIZE)
        			dwBufLen -= UART_SEND_BUFSIZE;
        		//transmitter is already runing.	
        		if (pUsart[wPort]->US_TCR && dwBufLen==0)
        		{//���˷��ͳ�ʱʱ���ˣ��������������ģ������ɣ����Ʒ�����ȥ��
                    pUsart[wPort]->US_CR = US_CR_RSTTX; //��λ�·���������λ����Ҫ��������                    
                    pUsart[wPort]->US_CR = US_CR_TXEN;// enable xmit
                    goto OVER;
        		}		
        	}
        
        	dwBufLen = g_tUartTx[wPort].wTail + UART_SEND_BUFSIZE - g_tUartTx[wPort].wHead - 1;
        	while (dwBufLen > UART_SEND_BUFSIZE)
        		dwBufLen -= UART_SEND_BUFSIZE;
        	wSentLen = wTxLenTmp > dwBufLen ? dwBufLen : wTxLenTmp;    //����������ʣһ���ֽ�
        	iTxHead = g_tUartTx[wPort].wHead;
        	for (int i=0; i<wSentLen; i++)
        	{
        		g_tUartTx[wPort].bTx[iTxHead++%UART_SEND_BUFSIZE] = *p++;
        	}
            
            wTxLenTmp -= wSentLen;
            
            if (wTxLenTmp == 0)  //�����һ���ˡ�
                fFinal = true;
        
        	//����ط�Ҫ���ж�
            //pUsart[wPort]->US_IDR = US_IDR_ENDTX | US_IDR_TXBUFE;   //zqq ���жϺ���������
        	g_tUartTx[wPort].wHead = (iTxHead%UART_SEND_BUFSIZE);
           	//pUsart[wPort]->US_IER = US_IER_ENDTX | US_IER_TXBUFE;  //zqq ���жϺ���������
        /*
        	if (pUsart[wPort]->US_PTSR & AT91C_PDC_TXTEN)//The transmitter is already running.  Yes, we really need this.
        	{
        		SignalSemaphore(g_semUartSyncTx[wPort]);
        		return wSentLen;
        	}
        */
        	if  (!pUsart[wPort]->US_TCR)
        	{//todo:Ӧ�Ƚ�TPR,TCR,TNPR,TNCR����
        		pUsart[wPort]->US_IER = US_IER_ENDTX | US_IER_TXBUFE;
        		// re-enable PDC transmit
        		pUsart[wPort]->US_PTCR = US_PTCR_TXTEN;	//�����������ж�,�ڴ����ж�������DMA����
        	}
            wTxLen += wSentLen;
        }     
        
        if (UsartsMode[wPort] == UART_SOFT_485)//���485ģʽ����ȷ�����ܳ�ȥ
        {
            if (fFinal)  //���һ���ˣ�ֻ�����һ�βż�鴮����λ�Ĵ���������Ӱ��Ч��
            {
                while  (!((pUsart[wPort]->US_CSR & US_CSR_TXEMPTY) && (!pUsart[wPort]->US_TCR)))   //������λ�Ĵ������ݷ���
                {
                    Sleep(0);  //ǿ��һ�ε��ȣ���������  
                    if (GetTick()-dwTick > dwTimeoutMs)//��ֹ�����п��ܳ�����ѭ��
                        break;
                }
            }
        }
        
        if (GetTick()-dwTick > dwTimeoutMs)//��ֹ�����п��ܳ�����ѭ��
        {             
            pUsart[wPort]->US_TCR = 0;//���͸�λ�ˣ�PDCҲ������0
            pUsart[wPort]->US_TNCR = 0;
            
            pUsart[wPort]->US_CR = US_CR_RSTTX; //��λ�·���������λ����Ҫ��������                    
            pUsart[wPort]->US_CR = US_CR_TXEN;  // enable xmit            
        	break;//��ʱ��PDC�е����ݻ�û�з��꣬�ǲ��ǳ������ˣ�
        }
        
        if (wTxLenTmp > 0)
            Sleep(0);
    }    
    
OVER:    
    CtrlRecv(wPort);  
	SignalSemaphore(g_semUartSyncTx[wPort]);
    
	return wTxLen;
}

//������ͨ�����ڶ�����
//������@wPort ���ں�
//      @pbBuf ��������
//      @dwBufSize  ��������С
//      @dwTimeouts ���ճ�ʱʱ�䣨������ô��ʱ����û�յ���������Ϊ��ʱ��
//���أ����������ݳ���
//DWORD CUart::Read(BYTE *pbBuf, WORD wMaxBufLen, DWORD dwTimeoutMs)
WORD UartRead(WORD wPort, BYTE* pbBuf, WORD wMaxBufLen, DWORD dwTimeoutMs)
{
	BYTE *p = (BYTE *)pbBuf;
	DWORD dwDataLen = 0;
	
	if (wPort >= MAX_PORT_NUM)
		return 0;
    
    if (UsartsMode[wPort] == UART_NULL) //����Ϊ����ʹ��
        return 0;
    
	if (!g_fPortOpen[wPort])
		return 0;    
    
	WaitSemaphore(g_semUartSyncRx[wPort], SYS_TO_INFINITE);
    
    if (pbBuf == NULL)  //���⴦���崮��
    {
        g_tUartRx[wPort].wTail = g_tUartRx[wPort].wHead;
        SignalSemaphore(g_semUartSyncRx[wPort]);
    	return 0;
    }
    
	if (dwTimeoutMs)
		WaitSemaphore(g_semUartRx[wPort], dwTimeoutMs);    
    
	dwDataLen = g_tUartRx[wPort].wHead + UART_RECV_BUFSIZE - g_tUartRx[wPort].wTail;
	while (dwDataLen >= UART_RECV_BUFSIZE)
		dwDataLen -= UART_RECV_BUFSIZE;
	if (dwDataLen > 0)
	{
		if (dwDataLen > wMaxBufLen)
			dwDataLen = wMaxBufLen;
		for (int i=0; i<dwDataLen; i++)
			*p++ = g_tUartRx[wPort].bRx[(g_tUartRx[wPort].wTail + i)%UART_RECV_BUFSIZE];
		g_tUartRx[wPort].wTail = ((g_tUartRx[wPort].wTail + dwDataLen)%UART_RECV_BUFSIZE);
	}
	SignalSemaphore(g_semUartSyncRx[wPort]);
    UartReadHook(wPort, dwDataLen);   //���֮�๤��
	return dwDataLen;
}
