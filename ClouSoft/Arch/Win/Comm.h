/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Comm.h
 * ժ    Ҫ��ϵͳ����ͨ�ŷ�װͷ�ļ�
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#ifndef COMM_H
#define COMM_H
#include "Typedef.h"
typedef struct {	
	WORD wPort; 
	DWORD dwBaudRate; 
	BYTE bByteSize; 
	BYTE bStopBits; 
	BYTE bParity;
}TCommPara; //�������� 

#define MAX_COMM_NUM    10


//�������򿪴���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits ֹͣλ
//      @bParity У��λ
//���أ�true-�ɹ���false-ʧ��
bool CommOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,	BYTE bStopBits, BYTE bParity);

//�������ش���
//������@wPort ���ں�
//���أ�true-�ɹ���false-ʧ��
bool CommClose(WORD wPort);

//���������ô��ڲ���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits ֹͣλ
//      @bParity У��λ
//���أ�true-�ɹ���false-ʧ��
bool CommSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

//���������ô��ڲ�����
//������@wPort ���ں�
//      @dwBaudRate ������
//���أ�true-�ɹ���false-ʧ��
bool CommSetBaudRate(WORD wPort, DWORD dwBaudRate);

//��������ȡ���ڲ���
//������@wPort ���ں�
//      @pCommPara ���ڲ���
//���أ�true-�ɹ���false-ʧ��
bool CommGetPara(WORD wPort, TCommPara* pCommPara);

//�������жϴ����Ƿ��Ѿ�����
//������@wPort ���ں�
//���أ�true-���ڴ򿪣�false-���ڹر�
bool CommIsOpen(WORD wPort);

//������ͨ�����ڶ�����
//������@wPort ���ں�
//      @pbBuf �������ݻ�����
//      @dwLength  ����������
//      @dwTimeouts ���ճ�ʱʱ�䣨������ʱ������Ϊ�����ݽ��գ�����ձ��ν��գ�
//���أ����������ݳ���
DWORD CommRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts);

//������ͨ�����ڷ�������
//������@wPort ���ں�
//      @pbData �����͵����ݻ�����
//      @dwDataLen  ���������ݳ���
//      @dwTimeouts ���ͳ�ʱʱ�䣨������ʱ����δ���ͳɹ�����Ϊ����ʧ�ܣ�
//���أ����������ݳ���
DWORD CommWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts);

#endif
