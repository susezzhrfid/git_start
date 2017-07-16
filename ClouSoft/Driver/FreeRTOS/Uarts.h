/*********************************************************************************************************
 * Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Uart.h
 * ժ    Ҫ��������
 *
 * ��ǰ�汾��0.0.1
 * ��    �ߣ�����
 * ������ڣ�2012-05-25
 *
 * ȡ���汾��
 * ԭ �� �ߣ����
 * ������ڣ�
 * ��    ע��
************************************************************************************************************/
#ifndef UARTS_H
#define UARTS_H
//#include "Typedef.h"
#include "Sysarch.h"

#define MAX_PORT_NUM        5         //�����Զ���5������ֻʹ����3�����Զ���3���Խ�ʡRAM

//У��λ
//#define NOPARITY     0
//#define ODDPARITY    1
//#define EVENPARITY   2

//ֹͣλ
//#define ONESTOPBIT     0
//#define	ONE5STOPBITS   1
//#define TWOSTOPBITS    2

#define UART_RECV_BUFSIZE   256//(64*2+1)//180//256    //TCP/IP ����Ҫ���ϱ�ͷ ,����������150�ֽ�,GPRS�����168B,   NOTE:��СΪUART_PDC_BUFSIZE*2+1
#define UART_SEND_BUFSIZE   64

//���ջ���������
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

//��������ʼ������(�ź�����ʼ����)
extern bool UartInit(WORD wPort);
//�������򿪴���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits  ֹͣλ
//      @bParity  У��λ
//���أ�0-û�д���
extern bool UartOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

//�������رմ���
//��������
//���أ�0-û�д���
extern bool UartClose(WORD wPort);


//������ͨ�����ڶ�����
//������@wPort ���ں�
//      @pbBuf ��������
//      @dwBufSize  ��������С
//      @dwTimeouts ���ճ�ʱʱ�䣨������ô��ʱ����û�յ���������Ϊ��ʱ��
//���أ����������ݳ���
extern WORD UartRead(WORD wPort, BYTE* pbBuf, WORD wMaxBufLen, DWORD dwTimeoutMs);

//������ͨ�����ڷ�������
//������@wPort ���ں�
//      @pbData �����͵����ݻ�����
//      @dwDataLen  �����͵����ݳ���
//      @dwTimeouts ���ͳ�ʱʱ��(�������ʱ����û��������Ϊ����ʧ��)
//���أ��ɹ����ͳ�ȥ�����ݳ���
extern WORD UartWrite(WORD wPort, BYTE* pbBuf, WORD wTxLen, DWORD dwTimeoutMs);

extern void IsrUartRxTimeout(WORD wPort);

extern void ISRUsart0();
extern void ISRUsart1();
extern void ISRUsart2();
extern void ISRUsart3();
//extern void NVIC_EnableIRQ(IRQn_Type IRQn);
#endif
