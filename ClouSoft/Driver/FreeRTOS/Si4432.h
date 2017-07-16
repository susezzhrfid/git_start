#ifndef SI4432_H
#define SI4432_H

#include "Typedef.h"

#define SI_4432_DTC              0x00            //Device Type Code
#define SI_4432_STATUS1          0x03            //Status1
#define SI_4432_STATUS2          0x04            //Status2
#define SI_4432_IE1R             0x05            //Interrupt Enable 1 register
#define SI_4432_IE2R             0x06            //Interrupt Enable 2 register
#define SI_4432_OFC1R            0x07            //Operating & Function Control1 register 
#define SI_4432_OFC2R            0x08            //Operating & Function Control2 register 
#define SI_4432_COLCR            0x09            //Crystal Oscillator Load Capacitance register

#define SI_4432_GPIO1CR          0x0c            //GPIO1 Configuration(set the TX state)
#define SI_4432_GPIO2CR          0x0d            //GPIO2 Configuration(set the RX state) 

#define SI_4432_IFFBR            0x1c            //IF Filter Bandwidth register	
#define SI_4432_AFCLGOR          0x1d            //AFC Loop Gearshift Override register
#define SI_4432_AFCTCR           0x1e            //AFC Timing Control register		
#define SI_4432_CRORR            0x20            //Clock Recovery Oversampling Ratio register
#define SI_4432_CRO2R            0x21            //Clock Recovery Offset 2 register
#define SI_4432_CRO1R            0x22            //Clock Recovery Offset 1 register
#define SI_4432_CRO0R            0x23            //Clock Recovery Offset 0 register
#define SI_4432_CRTLG1R          0x24            //Clock Recovery Timing Loop Gain 1 register
#define SI_4432_CRTLG0R          0x25            //Clock Recovery Timing Loop Gain 0 register

#define SI_4432_DACR             0x30            //Data Access Control registerData Access Control register

#define SI_4432_HC1R             0x32            //Header Control1 register
#define SI_4432_HC2R             0x33            //Header Control2 register 
#define SI_4432_PLR              0x34            //Preamble Length register
#define SI_4432_PDCR             0x35            //Preamble Detection Control  register
#define SI_4432_SW3R             0x36            //Sync Word 3 register
#define SI_4432_SW2R             0x37            //Sync Word 2 register

#define SI_4432_RPLR             0x4b            //Received Packet Length register

//#define SI_4432                  0x58

#define SI_4432_TXPR             0x6d            //TX Power register
#define SI_4432_DR1R             0x6e            //TXDataRate 1 register
#define SI_4432_DR0R             0x6f            //TXDataRate 0 register
#define SI_4432_MMC1R            0x70            //Modulation Mode Control 1 register
#define SI_4432_MMC2R            0x71            //Modulation Mode Control 2 register
#define SI_4432_FDR              0x72            //Frequency Deviation register 

#define SI_4432_FBSR             0x75            //Frequency Band Select register 
#define SI_4432_NCF1R            0x76            //Nominal Carrier Frequency1 register
#define SI_4432_NCF2R            0x77            //Nominal Carrier Frequency2 register

#define SI_4432_RXFIFO           0x7f            //FIFO Access register


#define WL_STATE_IDLE  0
#define WL_STATE_RX    1
#define WL_STATE_TX    2

#define FIFO_SIZE   64

#define AVAILABLE_BYTES_IN_TX_FIFO  32        //60
#define BYTES_IN_TX_FIFO            FIFO_SIZE - AVAILABLE_BYTES_IN_TX_FIFO
#define BYTES_IN_RX_FIFO            32

//只支持小于长度255的报文 
typedef struct
{
    WORD wBytesLeft;              // Used to keep track of how many bytes are left to be written to                                 // the TX FIFO
    //BYTE bIterations;             // For packets greater than 64 bytes, this variable is used to keep 
                                  // track of how many time the TX FIFO should be re-filled to its limit 
    //BYTE bWriteRemainingDataFlag; // When this flag is set, the TX FIFO should not be filled entirely
//    BYTE bPacketSentFlag;         // Flag set when GDO0 indicates that the packet is sent
    BYTE *pbBufIndex;             // Pointer to current position in the txBuffer 
    //WORD wPacketsSent;            // Number of packets transmitted
    //BYTE bPktFormat;              // Infinite or fixed packet mode
}TTxState;

typedef struct
{
    WORD wBytesLeft;              // Used to keep track of how many bytes are left to be read from the RX FIFO
    //BYTE bPacketReceivedFlag;     // Flag set when GDO0 indicates that a packet is received
    //BYTE bSyncOrEndOfPacket;      // Flag used to determine if the interrupt is caused by a rising or
                                  // a falling edge
    BYTE *pbBufIndex;             // Pointer to current position in the rxBuffer 
    //WORD wLengthByte;             // LengthByte (This example require variable packet length mode)
    //BYTE bCrcOK;                  // CRC_OK flag from status byte in RX FIFO
    //BYTE bRSSI;
    //WORD wPacketsReceived;        // Number of packets received
    //BYTE bPktFormat;              // Infinite or fixed packet mode
    //BYTE bStep;                   //接收到帧头后提取长度字节
}TRxState;

/*
typedef struct
{
    BYTE *pbHead;
    BYTE *pbCur;
    BYTE bLen;
    BYTE bState;          //1->Tx  0->Rx
}TTxCnt;*/

//typedef struct
//{
    //BYTE bFrmLen;
    //BYTE bRxLen;
    //int iRSSI;
//    BYTE bLQI;    
//    DWORD dwDelay;
    //BYTE bEnRdRssi;     //允许读RSSI
    
//}TRxCnt;
#define WL_NUM       1

#define WL_RECV_BUFSIZE   64
#define WL_SEND_BUFSIZE   64

typedef struct
{
	BYTE bRx[WL_RECV_BUFSIZE];
	int iHead;
	int iTail;
} TWlRx;

typedef struct
{
	BYTE bTx[WL_SEND_BUFSIZE];
	int iHead;
	int iTail;
} TWlTx;

typedef struct 
{
    BYTE bWlCh;
    BYTE bPower;
    BYTE bBaud;
}TWlPara;

extern TWlPara g_tWlPara;

void SpiInit(void);

void SI4432Init(void);
bool SI4432Open(DWORD dwBaudRate);
bool SI4432Close(void);
BYTE SI4432Write(BYTE *pbTxBuf, BYTE bLen, DWORD dwTimeouts);
WORD SI4432Read(BYTE *pbBuf, WORD wMaxBufLen, DWORD dwTimeoutMs);

void SI4432SetFrq(BYTE bCh);
int GetRSSI(void);

void RfSetPower(BYTE bVal);

typedef struct
{
    BYTE bCADelay;        //45ms
    int iRSSI;            //当前接收的场强
}TCAvoid;

#endif