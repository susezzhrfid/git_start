#ifndef COMM_H
#define COMM_H

#include "Typedef.h"

#define CBR_300     300
#define CBR_600     600
#define CBR_1200	1200
#define CBR_2400	2400
#define CBR_4800	4800
#define CBR_9600	9600
#define CBR_19200	19200
#define CBR_38400	38400
#define	CBR_57600	57600
#define	CBR_115200	115200

#define NOPARITY        0
#define ODDPARITY       1
#define EVENPARITY      2

#define ONESTOPBIT      0
#define	ONE5STOPBITS    1
#define TWOSTOPBITS     2

//#include "Comm.h"

typedef struct {	
	WORD wPort; 
	DWORD dwBaudRate; 
	BYTE bByteSize; 
	BYTE bStopBits; 
	BYTE bParity;
}TCommPara; //�������� 

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

//���������ô��ڲ��������ڴ򿪺�ſ�ʹ�ã�
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits ֹͣλ
//      @bParity У��λ
//���أ�true-�ɹ���false-ʧ��
//bool CommSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

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
int CommRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts);

//������ͨ�����ڷ�������
//������@wPort ���ں�
//      @pbData �����͵����ݻ�����
//      @dwDataLen  ���������ݳ���
//      @dwTimeouts ���ͳ�ʱʱ�䣨������ʱ����δ���ͳɹ�����Ϊ����ʧ�ܣ�
//���أ����������ݳ���
int CommWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts);


#endif
