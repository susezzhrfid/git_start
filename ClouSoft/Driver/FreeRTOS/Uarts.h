/*********************************************************************************************************
 * Copyright (c) 2010,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Uart.h
 * 摘    要：串口类
 *
 * 当前版本：0.0.1
 * 作    者：李焱
 * 完成日期：2012-05-25
 *
 * 取代版本：
 * 原 作 者：杨进
 * 完成日期：
 * 备    注：
************************************************************************************************************/
#ifndef UARTS_H
#define UARTS_H
//#include "Typedef.h"
#include "Sysarch.h"

#define MAX_PORT_NUM        5         //最大可以定义5，但是只使用了3个所以定义3可以节省RAM

//校验位
//#define NOPARITY     0
//#define ODDPARITY    1
//#define EVENPARITY   2

//停止位
//#define ONESTOPBIT     0
//#define	ONE5STOPBITS   1
//#define TWOSTOPBITS    2

#define UART_RECV_BUFSIZE   256//(64*2+1)//180//256    //TCP/IP 还需要加上报头 ,升级报文有150字节,GPRS升级最长168B,   NOTE:最小为UART_PDC_BUFSIZE*2+1
#define UART_SEND_BUFSIZE   64

//接收缓冲区定义
typedef struct
{
	BYTE bRx[UART_RECV_BUFSIZE];
	WORD wHead;
	WORD wTail;
} TUartRx;

typedef struct
{
	BYTE bTx[UART_SEND_BUFSIZE];
	WORD wHead;
	WORD wTail;
} TUartTx;

//描述：初始化串口(信号量初始化等)
extern bool UartInit(WORD wPort);
//描述：打开串口
//参数：@wPort 串口号
//      @dwBaudRate 波特率
//      @bByteSize  数据位
//      @bStopBits  停止位
//      @bParity  校验位
//返回：0-没有错误
extern bool UartOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

//描述：关闭串口
//参数：无
//返回：0-没有错误
extern bool UartClose(WORD wPort);


//描述：通过串口读数据
//参数：@wPort 串口号
//      @pbBuf 读缓冲区
//      @dwBufSize  缓冲区大小
//      @dwTimeouts 接收超时时间（超过这么长时间仍没收到数据则认为超时）
//返回：读到的数据长度
extern WORD UartRead(WORD wPort, BYTE* pbBuf, WORD wMaxBufLen, DWORD dwTimeoutMs);

//描述：通过串口发送数据
//参数：@wPort 串口号
//      @pbData 待发送的数据缓冲区
//      @dwDataLen  待发送的数据长度
//      @dwTimeouts 发送超时时间(超过这个时间仍没发出则认为发送失败)
//返回：成功发送出去的数据长度
extern WORD UartWrite(WORD wPort, BYTE* pbBuf, WORD wTxLen, DWORD dwTimeoutMs);

extern void IsrUartRxTimeout(WORD wPort);

extern void ISRUsart0();
extern void ISRUsart1();
extern void ISRUsart2();
extern void ISRUsart3();
//extern void NVIC_EnableIRQ(IRQn_Type IRQn);
#endif
