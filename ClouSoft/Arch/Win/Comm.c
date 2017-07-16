/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Comm.c
 * ժ    Ҫ��ϵͳ����ͨ�ŷ�װʵ���ļ�
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
//#include "InUart.h"
#include "Comm.h"

bool g_fCommIsOpen[MAX_COMM_NUM] = { false };

DWORD g_dwBaudRate[MAX_COMM_NUM];
BYTE g_bByteSize[MAX_COMM_NUM];
BYTE g_bStopBits[MAX_COMM_NUM];
BYTE g_bParity[MAX_COMM_NUM];

extern int InUartInit(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

extern int InUartOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

extern int InUartClose(WORD wPort);

extern int InUartSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

extern DWORD InUartRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts);

extern DWORD InUartWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts);



//�������򿪴���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits ֹͣλ
//      @bParity У��λ
//���أ�true-�ɹ���false-ʧ��
bool CommOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,	BYTE bStopBits, BYTE bParity)
{
    if (wPort >= MAX_COMM_NUM)
        return false;
    
	g_fCommIsOpen[wPort] = (InUartOpen(wPort, dwBaudRate, bByteSize, bStopBits, bParity) == 0);
	if (g_fCommIsOpen[wPort])
	{
		g_dwBaudRate[wPort] = dwBaudRate;
		g_bByteSize[wPort] = bByteSize;
		g_bStopBits[wPort] = bStopBits;
		g_bParity[wPort] = bParity;
	}
    
    return g_fCommIsOpen[wPort];
}

//�������ش���
//������@wPort ���ں�
//���أ�true-�ɹ���false-ʧ��
bool CommClose(WORD wPort)
{
    int iRet;
    if (wPort >= MAX_COMM_NUM)
        return false;
        
    iRet = InUartClose(wPort);
    if (iRet == 0) //��ʾ�سɹ���
        g_fCommIsOpen[wPort] = false;

	return !g_fCommIsOpen[wPort];
}

//���������ô��ڲ���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits ֹͣλ
//      @bParity У��λ
//���أ�true-�ɹ���false-ʧ��
bool CommSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
{
    if (wPort >= MAX_COMM_NUM)
        return false;
    
	if (InUartSetup(wPort, dwBaudRate, bByteSize, bStopBits, bParity) == 0)
	{
		g_dwBaudRate[wPort] = dwBaudRate;
		g_bByteSize[wPort] = bByteSize;
		g_bStopBits[wPort] = bStopBits;
		g_bParity[wPort] = bParity;
		return true;
	}
	return false;
}

//���������ô��ڲ�����
//������@wPort ���ں�
//      @dwBaudRate ������
//���أ�true-�ɹ���false-ʧ��
bool CommSetBaudRate(WORD wPort, DWORD dwBaudRate)
{
	if (wPort >= MAX_COMM_NUM)
		return false;

	if (InUartSetup(wPort, dwBaudRate, g_bByteSize[wPort], g_bStopBits[wPort], g_bParity[wPort]) == 0)
	{
		g_dwBaudRate[wPort] = dwBaudRate;
		return true;
	}
	return false;
}

//��������ȡ���ڲ���
//������@wPort ���ں�
//      @pCommPara ���ڲ���
//���أ�true-�ɹ���false-ʧ��
bool CommGetPara(WORD wPort, TCommPara* pCommPara)
{
    if (CommIsOpen(wPort))
    {
	pCommPara->wPort = wPort; 
	pCommPara->dwBaudRate = g_dwBaudRate[wPort];
	pCommPara->bByteSize = g_bByteSize[wPort];
	pCommPara->bStopBits = g_bStopBits[wPort];
	pCommPara->bParity = g_bParity[wPort];
	return true;
    }

    return false;
}

//�������жϴ����Ƿ��Ѿ�����
//������@wPort ���ں�
//���أ�true-���ڴ򿪣�false-���ڹر�
bool CommIsOpen(WORD wPort)
{
    if (wPort >= MAX_COMM_NUM)
        return false;
    
    return g_fCommIsOpen[wPort];
}

//������ͨ�����ڶ�����
//������@wPort ���ں�
//      @pbBuf �������ݻ�����
//      @dwLength  ����������
//      @dwTimeouts ���ճ�ʱʱ�䣨������ʱ������Ϊ�����ݽ��գ�����ձ��ν��գ�
//���أ����������ݳ���
DWORD CommRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts)
{
    if (wPort >= MAX_COMM_NUM)
        return 0;
    
    return InUartRead(wPort, pbBuf, dwBufSize, dwTimeouts);
}

//������ͨ�����ڷ�������
//������@wPort ���ں�
//      @pbData �����͵����ݻ�����
//      @dwDataLen  ���������ݳ���
//      @dwTimeouts ���ͳ�ʱʱ�䣨������ʱ����δ���ͳɹ�����Ϊ����ʧ�ܣ�
//���أ����������ݳ���
DWORD CommWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts)
{
    if (wPort >= MAX_COMM_NUM)
        return 0;

	return InUartWrite(wPort, pbData, dwDataLen, dwTimeouts);
}

