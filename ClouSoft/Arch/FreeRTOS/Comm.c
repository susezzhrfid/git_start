#include "Comm.h"
#include "DrvCfg.h"
#include "Uarts.h"
#include "FaCfg.h"

bool g_fCommIsOpen[COMM_NUM] = { false };
DWORD g_dwBaudRate[COMM_NUM];
BYTE g_bByteSize[COMM_NUM];
BYTE g_bStopBits[COMM_NUM];
BYTE g_bParity[COMM_NUM];

//�������򿪴���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits ֹͣλ
//      @bParity У��λ
//���أ�true-�ɹ���false-ʧ��
bool CommOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,	BYTE bStopBits, BYTE bParity)
{
    bool fRet;
    if (wPort >= COMM_NUM)
        return false;    
    
    fRet = UartOpen(wPort, dwBaudRate, bByteSize, bStopBits, bParity);
    if (fRet)
        g_fCommIsOpen[wPort] = true;
    
    if (g_fCommIsOpen[wPort])
    {
        g_dwBaudRate[wPort] = dwBaudRate;
        g_bByteSize[wPort] = bByteSize;
        g_bStopBits[wPort] = bStopBits;
        g_bParity[wPort] = bParity;
    }
    
    return fRet;
}

//�������ش���
//������@wPort ���ں�
//���أ�true-�ɹ���false-ʧ��
bool CommClose(WORD wPort)
{
    bool fRet;
    if (wPort >= COMM_NUM)
        return false;
        
    fRet = UartClose(wPort);
    if (fRet) //��ʾ�سɹ���
        g_fCommIsOpen[wPort] = false;
    
    return fRet;
}

//���������ô��ڲ���
//������@wPort ���ں�
//      @dwBaudRate ������
//      @bByteSize  ����λ
//      @bStopBits ֹͣλ
//      @bParity У��λ
//���أ�true-�ɹ���false-ʧ��
/*bool CommSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
{
    if (wPort >= COMM_NUM)
        return false;

    if (UartSetup(wPort, dwBaudRate, bByteSize, bStopBits, bParity) == 0)
    {
        g_dwBaudRate[wPort] = dwBaudRate;
        g_bByteSize[wPort] = bByteSize;
        g_bStopBits[wPort] = bStopBits;
        g_bParity[wPort] = bParity;    
        return true;
    }
    
    return false;
}*/

//���������ô��ڲ�����
//������@wPort ���ں�
//      @dwBaudRate ������
//���أ�true-�ɹ���false-ʧ��
bool CommSetBaudRate(WORD wPort, DWORD dwBaudRate)
{
    if (wPort >= COMM_NUM)
        return false;    	
        
    if (CommIsOpen(wPort))
        CommClose(wPort);
    
    return CommOpen(wPort, dwBaudRate, g_bByteSize[wPort], g_bStopBits[wPort], g_bParity[wPort]);
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
    if (wPort >= COMM_NUM)
        return false;
    
    return g_fCommIsOpen[wPort];
}

//������ͨ�����ڶ�����
//������@wPort ���ں�
//      @pbBuf �������ݻ�����
//      @dwLength  ����������
//      @dwTimeouts ���ճ�ʱʱ�䣨������ʱ������Ϊ�����ݽ��գ�����ձ��ν��գ�
//���أ����������ݳ���
int CommRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts)
{
  	WORD wLen = 0;
    if (wPort >= COMM_NUM)
        return 0;
  
    wLen = UartRead(wPort, pbBuf, dwBufSize, dwTimeouts);
	if (wLen>0 && wPort!=COMM_GPRS)
	{
		DoLedBurst(LED_LOCAL_RX);
	}
	return wLen;
}

//������ͨ�����ڷ�������
//������@wPort ���ں�
//      @pbData �����͵����ݻ�����
//      @dwDataLen  ���������ݳ���
//      @dwTimeouts ���ͳ�ʱʱ�䣨������ʱ����δ���ͳɹ�����Ϊ����ʧ�ܣ�
//���أ����������ݳ���
int CommWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts)
{
  	WORD wLen = 0;
    if (wPort >= COMM_NUM)
        return 0;    
    
    if (g_dwBaudRate[wPort] < 300)
        return 0;
    
    if (dwTimeouts == 0)
        dwTimeouts = (dwDataLen*((11+2)*1000/g_dwBaudRate[wPort]+1))<<1;  //ÿ�ֽں�2BITͣ�٣���1С����ǰ��λ,���һ����ʱ��
    
    wLen = UartWrite(wPort, pbData, dwDataLen, dwTimeouts);
	
	if (wPort != COMM_GPRS)
		DoLedBurst(LED_LOCAL_TX);
	
	return wLen;
}
