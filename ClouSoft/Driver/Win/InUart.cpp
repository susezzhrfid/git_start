// Comm.cpp: implementation of the CInUart class.
//
//////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "InUart.h"
#include "stdio.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//����������ݿ顢XON��XOFF
#define MAXBLOCK 4096
#define XON      0x11
#define XOFF     0x13

CInUart g_InUart[10];

DWORD g_dwBaudRateTab[BR_NUM] = 
{
	CBR_1200, CBR_2400, CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400, 
	CBR_56000, CBR_57600, CBR_115200, CBR_128000, CBR_256000
};

char* g_szCommPortTab[] = 
{
"COM1",
"COM2", 
"COM3",
"COM4",
"COM5",
"COM6",
"COM7",
"COM8",
"COM9",
"COM10"
};

BYTE  g_bParityTab[PARITY_NUM] = 
{
	NOPARITY, ODDPARITY, EVENPARITY
};




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UINT CommWatchProc( LPVOID pParam);  //��������߳�

CInUart::CInUart()
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hTxWnd = m_hRxWnd = NULL;
	m_pfnRxCallback = NULL;
	m_pvCallbackArg = NULL;

	//��ʼ������
	m_fConnect = FALSE;  //�Ͽ����Ӳ˵�����Ч
	m_fWatching = FALSE;
	m_pThread = NULL;

	m_dwBaudRate = CBR_9600;
	m_bByteSize =  8;
	//m_bEcho = FALSE;
	//m_bNewLine = FALSE;
	m_bParity = NOPARITY;
	m_strPort = "COM1";
	m_bStopBits = 1;

	memset(&m_osRead, 0, sizeof(OVERLAPPED));
	memset(&m_osWrite, 0, sizeof(OVERLAPPED));
	memset(&m_osWait, 0, sizeof(OVERLAPPED));

	// Ϊ�ص��������¼������ֹ����ã���ʼ��Ϊ���źŵ�
	if ((m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		return;

	// Ϊ�ص�д�����¼������ֹ����ã���ʼ��Ϊ���źŵ�
	if ((m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		return;

	// ΪWaitCommEvent()�����¼������ֹ����ã���ʼ��Ϊ���źŵ�
	if ((m_osWait.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		return;

}


CInUart::~CInUart()
{
	Close();
	/*if (m_hComm == INVALID_HANDLE_VALUE)
	{
	CloseHandle(m_hComm);
	}*/

	CloseHandle(m_osRead.hEvent);
	CloseHandle(m_osWrite.hEvent);
	CloseHandle(m_osWait.hEvent);
}


BOOL CInUart::IsOpen()
{
	return m_hComm != INVALID_HANDLE_VALUE;
}


//���ô�������
BOOL CInUart::ConfigConnection()
{
	DCB dcb;

	if (!GetCommState(m_hComm, &dcb))
		return FALSE;

	dcb.fBinary = TRUE;
	dcb.BaudRate = m_dwBaudRate; //���ݴ�������
	dcb.ByteSize = m_bByteSize;  //ÿ�ֽ�λ��
	dcb.fParity = TRUE;

	dcb.Parity = m_bParity;      //У������ MARKPARITY;
	dcb.StopBits = m_bStopBits;  //ֹͣλ

	dcb.fOutxCtsFlow = FALSE;     //TRUE;     //Ӳ������������
	dcb.fRtsControl = FALSE;      //TRUE;

	// XON/XOFF����������
	dcb.fInX=dcb.fOutX = FALSE;          //TRUE;
	dcb.XonChar = FALSE;                 //XON;
	dcb.XoffChar = XOFF;
	dcb.XonLim = 50;
	dcb.XoffLim = 50;

	dcb.fRtsControl = RTS_CONTROL_ENABLE;

	return SetCommState(m_hComm, &dcb);
}

BOOL CInUart::SetBaudRate(DWORD dwBaudRate)
{  
	DWORD dwOldBaudRate = m_dwBaudRate;

	if ( !IsOpen() )
		Open();

	if (dwBaudRate == m_dwBaudRate)
		return true;

	m_dwBaudRate = dwBaudRate;

	if ( !ConfigConnection() ) //������ò��ɹ�����ԭ��������
	{
		m_dwBaudRate = dwOldBaudRate;
		return false;
	}

	return true;
}

BOOL CInUart::SetComm(DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
{
	DWORD dwOldBaudRate = m_dwBaudRate;
	DWORD dwOldByteSize = m_bByteSize;
	DWORD dwOldStopBits = m_bStopBits;
	DWORD dwOldParity = m_bParity;

	if ( !IsOpen() )
		Open();

	if (m_dwBaudRate==dwBaudRate && m_bByteSize==bByteSize 
		&& m_bStopBits==bStopBits && m_bParity==bParity)
		return true;

	m_dwBaudRate = dwBaudRate;
	m_bByteSize = bByteSize;
	m_bStopBits = bStopBits;
	m_bParity = bParity;

	if ( !ConfigConnection() ) //������ò��ɹ�����ԭ��������
	{
		m_dwBaudRate = dwOldBaudRate;
		m_bByteSize = (BYTE)dwOldByteSize;
		m_bStopBits = (BYTE)dwOldStopBits;
		m_bParity = (BYTE)dwOldParity;
		return false;
	}

	return true;
}

void CInUart::SetTimeouts(DWORD dwTimeouts)
{
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 2*(1000 * 5 * 11)/m_dwBaudRate;
	if (TimeOuts.ReadIntervalTimeout<5)  TimeOuts.ReadIntervalTimeout=5;  

	TimeOuts.ReadTotalTimeoutMultiplier = 0;  //  //��������������ܳ�ʱʱ���ϵ��,��λms
	TimeOuts.ReadTotalTimeoutConstant = dwTimeouts;  //��������������ܳ�ʱʱ���ϵ��,��λms
	//���ܳ�ʱʱ����㹫ʽ��
	//ReadTotalTimeout = (ReadTotalTimeoutMultiplier * bytes_to_read) + ReadTotalTimeoutConstant;

	SetCommTimeouts(m_hComm, &TimeOuts);
}


BOOL CInUart::Open()
{
	if (m_hComm != INVALID_HANDLE_VALUE)
		return FALSE;

	ResetEvent(m_osRead.hEvent);
	ResetEvent(m_osWrite.hEvent);
	ResetEvent(m_osWait.hEvent);

	m_hComm = CreateFile(m_strPort, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,   //FILE_FLAG_OVERLAPPED
		NULL); // �ص���ʽ

	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	SetupComm(m_hComm,MAXBLOCK,MAXBLOCK);
	//SetCommMask(m_hComm, EV_RXCHAR);


	// �Ѽ����ʱ��Ϊ��󣬰��ܳ�ʱ��Ϊ0������ReadFile�������ز���ɲ���
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 100;       //MAXDWORD�����ַ�����֮������ʱ����,��λms
	TimeOuts.ReadTotalTimeoutMultiplier = 2;  //  //��������������ܳ�ʱʱ���ϵ��,��λms
	TimeOuts.ReadTotalTimeoutConstant = 1000;  //��������������ܳ�ʱʱ���ϵ��,��λms
	//���ܳ�ʱʱ����㹫ʽ��
	//ReadTotalTimeout = (ReadTotalTimeoutMultiplier * bytes_to_read) + ReadTotalTimeoutConstant;

	// ����д��ʱ��ָ��WriteComm��Ա�����е�GetOverlappedResult�����ĵȴ�ʱ��
	TimeOuts.WriteTotalTimeoutMultiplier = 50;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	//д�ܳ�ʱʱ����㹫ʽ��
	//WriteTotalTimeout = (WriteTotalTimeoutMultiplier * bytes_to_write) + WriteTotalTimeoutConstant;

	SetCommTimeouts(m_hComm, &TimeOuts);

	if (!ConfigConnection())
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	return TRUE;
}

BOOL CInUart::Open(LPCTSTR szPort, DWORD dwBaudRate, BYTE bByteSize,
				   BYTE bStopBits, BYTE bParity)
{
	if (m_hComm != INVALID_HANDLE_VALUE)
		return FALSE;

	m_strPort = szPort;
	m_dwBaudRate = dwBaudRate;
	m_bByteSize = bByteSize;
	m_bStopBits = bStopBits;
	m_bParity = bParity;
	return Open();
}

BOOL CInUart::Open(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,
				   BYTE bStopBits, BYTE bParity)
{
	char szName[16];
	sprintf(szName, "COM%d", wPort);
	m_wPort = wPort;
	return Open(szName, dwBaudRate, bByteSize, bStopBits, bParity);
}

BOOL CInUart::Open(TCommPara &CommPara)
{
	if( CommPara.wPort > 10 || !CommPara.wPort )
		return false;

	return Open(CommPara.wPort, CommPara.dwBaudRate, CommPara.bByteSize, CommPara.bStopBits, CommPara.bParity);
}

bool CInUart::GetCommPara(TCommPara* pCommPara)
{
	if (IsOpen())
	{
		pCommPara->wPort = m_wPort; 
		pCommPara->dwBaudRate = m_dwBaudRate;
		pCommPara->bByteSize = m_bByteSize;
		pCommPara->bStopBits = m_bStopBits;
		pCommPara->bParity = m_bParity;
		return true;
	}
	else
	{
		return false;
	}
}

BOOL CInUart::Config(LPCTSTR szPort, DWORD dwBaudRate, BYTE bByteSize,
					 BYTE bStopBits, BYTE bParity)
{
	//if (m_hComm != INVALID_HANDLE_VALUE)
	//	 return FALSE;

	m_strPort = szPort;
	m_dwBaudRate = dwBaudRate;
	m_bByteSize = bByteSize;
	m_bStopBits = bStopBits;
	m_bParity = bParity;
	return TRUE;
}

BOOL CInUart::Config(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,
					 BYTE bStopBits, BYTE bParity)
{
	char szName[16];
	sprintf(szName, "COM%d", wPort);
	return Config( szName, dwBaudRate, bByteSize, bStopBits, bParity);
}



BOOL CInUart::Config(TCommPara &CommPara)
{
	if( CommPara.wPort > 10 || !CommPara.wPort )
		return false;

	return Config(CommPara.wPort, CommPara.dwBaudRate, CommPara.bByteSize, CommPara.bStopBits, CommPara.bParity);
}

BOOL CInUart::BeginWatch(void)
{
	m_pThread = AfxBeginThread(CommWatchProc,               //pfnThreadProc
		(LPVOID)this,                //pParam
		THREAD_PRIORITY_NORMAL,      //nPriority 
		0,                           //nStackSize  
		CREATE_SUSPENDED,            //dwCreateFlags 
		NULL);   //�����������߳�

	if (m_pThread == NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	else
	{
		m_pThread->ResumeThread(); // �ָ��߳�����
	}
	return TRUE;
}


// ��ָ���������ַ��Ӵ��п����
DWORD CInUart::Write(LPCVOID lpBuf, DWORD dwLength)
{
	BOOL fState;
	DWORD length = dwLength;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	DWORD dwLastErr;

	ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
	fState = WriteFile(m_hComm, lpBuf, dwLength, &length, &m_osWrite);

	//	FlushFileBuffers(m_hComm);
	if (!fState)
	{
		if ((dwLastErr=GetLastError()) == ERROR_IO_PENDING)
		{
			GetOverlappedResult(m_hComm, &m_osWrite, &length, TRUE);// �ȴ�
			//�ȴ�������첽������ɣ�����������ʱI/O�����Ѿ����
		}
		else
		{  
			length = 0;
		}
	}

	if (m_hRxWnd!=NULL && IsWindow(m_hRxWnd) && length!=0)  //����һ֡������������������
	{
		BYTE* pMsgData = new BYTE[length];
		memcpy(pMsgData, lpBuf, length);
		DWORD dwPara = 0x10000 + length;   //Direction | Length
		//::PostMessage(m_hRxWnd, WM_COMM_NOTIFY, (WPARAM)pMsgData, (LPARAM)dwPara);
	}

	return length;
}


//���������ô��ڵĻص�����������
//������@pfnRxCallback ���������յ�����ʱ���ص���·��Ľ��պ���
//      @pvCallbackArg �ص�ʱ������·��Ĳ���
//���أ���
void CInUart::SetCallback(void (*pfnRxCallback)(BYTE* pbBlock, WORD wLen, void* pvArg), 
						  void* pvCallbackArg)
{
	m_pfnRxCallback = pfnRxCallback;
	m_pvCallbackArg = pvCallbackArg;
}


//���������ý������ݵ���ʾ����
//������@hRxWnd �������ݵ���ʾ����
//���أ���
void CInUart::SetRxTxWnd(HWND hRxWnd, HWND hTxWnd)
{
	m_hRxWnd = hRxWnd;
	m_hTxWnd = hTxWnd;
}

// �Ӵ��п����뻺�����ж���ָ���������ַ�
DWORD CInUart::Read(LPVOID buf, DWORD dwLength)
{
	DWORD length = 0;
	COMSTAT ComStat;
	DWORD dwErrorFlags;	

	ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
	length = dwLength; //min(dwLength, ComStat.cbInQue);

	ReadFile(m_hComm, buf, dwLength, &length, &m_osRead);

	/*
	//�����յ������ݸ��µ����ڽ��ռ��ӽ��棬
	if (m_hRxWnd!=NULL && IsWindow(m_hRxWnd) &&length!=0)
	{ 
	PBYTE pbData = new BYTE[length];
	memcpy(pbData, buf, length);
	DWORD dwPara = 0x00000 + length;   //Direction | Length
	::PostMessage(m_hRxWnd, WM_COMM_NOTIFY, (WPARAM)pbData, (LPARAM)dwPara); 
	}*/

	return length;
}

DWORD CInUart::Read(LPVOID buf, DWORD dwLength, DWORD dwTimeout)
{
	SetTimeouts(dwTimeout);

	return Read(buf, dwLength);
}


BOOL CInUart::Close(void)
{
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
//��������߳�
//���������ݷŵ�ѭ������thePhy.m_PhyData.RcvBuf
UINT CommWatchProc(LPVOID pParam)
{  
	BYTE  bRcvBuf[MAXBLOCK];
	//OVERLAPPED os;
	DWORD dwMask, dwTrans;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	CInUart* pComm = (CInUart* )pParam;

	pComm->m_fWatching = TRUE;
	pComm->m_fConnect = TRUE;
	while (pComm->m_fConnect)
	{   
		ClearCommError(pComm->m_hComm, &dwErrorFlags, &ComStat);
		//�ڴ���ͨ���з����������նˡ���ż����ȣ�I/O����������ֹ
		//Ҫ��һ��ִ��I/O�������������ClearCommError(),���������ǣ�
		//1).�������������2).ȷ������ͨ��״̬
		if (ComStat.cbInQue)
		{
			DWORD dwRcvLen = pComm->Read(bRcvBuf, MAXBLOCK);
			if (dwRcvLen)
			{
				//����ͨ��Э��Ľ�����֡�ص�����
				if (pComm->m_pfnRxCallback != NULL)
				{
					pComm->m_pfnRxCallback(bRcvBuf, (WORD)dwRcvLen, pComm->m_pvCallbackArg);
				}
				if (pComm->m_hRxWnd != NULL)
				{ //�����յ������ݸ��µ����ڽ��ռ��ӽ��棬
					PBYTE pbData = new BYTE[dwRcvLen];
					memcpy(pbData, bRcvBuf, dwRcvLen);
					::PostMessage(pComm->m_hRxWnd, WM_DISPLAY,
						(WPARAM)pbData, (LPARAM)dwRcvLen); 
				}
			} 
			continue;
		}

		dwMask = 0;

		if (!WaitCommEvent(pComm->m_hComm, &dwMask, &pComm->m_osWait)) // �ص�����
		{                           //����Ҫ�رմ���ʱ������ͨ��ʹm_osWait.hEvent�õ�֪ͨ������
			if (GetLastError() == ERROR_IO_PENDING)
			{	
				GetOverlappedResult(pComm->m_hComm, &pComm->m_osWait, &dwTrans, TRUE); //���޵ȴ��ص��������
			}
			else
			{
				CloseHandle(pComm->m_osWait.hEvent);
				return (UINT)-1;
			}
		}
	}

	pComm->m_fWatching = FALSE;
	return 0;
}

extern "C"
{
	//��������ʼ������(�ź�����ʼ����)
	//������@wPort ���ں�
	//      @dwBaudRate ������
	//      @bByteSize  ����λ
	//      @bStopBits  ֹͣλ
	//      @bParity  У��λ
	//���أ�0-û�д���
	int InUartInit(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
	{
		return 0;
	}

	//�������򿪴���
	//������@wPort ���ں�
	//      @dwBaudRate ������
	//      @bByteSize  ����λ
	//      @bStopBits  ֹͣλ
	//      @bParity  У��λ
	//���أ�0-û�д���
	int InUartOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
	{
		if (wPort < 10)
		{
			if (g_InUart[wPort].Open(wPort, dwBaudRate, bByteSize, bStopBits, bParity))
				return 0;
		}
		return -1;
	}

	//�������رմ���
	//��������
	//���أ�0-û�д���
	int InUartClose(WORD wPort)
	{
		if (wPort < 10)
		{
			if (g_InUart[wPort].Close())
				return 0;
		}
		return -1;
	}

	//���������ô��ڣ������ʵȣ�
	//������@wPort ���ں�
	//      @dwBaudRate ������
	//      @bByteSize  ����λ
	//      @bStopBits  ֹͣλ
	//      @bParity  У��λ
	//���أ�0-û�д���
	int InUartSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
	{
		if (wPort < 10)
		{
			if (g_InUart[wPort].SetComm(dwBaudRate,bByteSize, bStopBits, bParity))
				return 0;
		}
		return -1;
	}

	//������ͨ�����ڶ�����
	//������@wPort ���ں�
	//      @pbBuf ��������
	//      @dwBufSize  ��������С
	//      @dwTimeouts ���ճ�ʱʱ�䣨������ô��ʱ����û�յ���������Ϊ��ʱ��
	//���أ����������ݳ���
	DWORD InUartRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts)
	{
		if (wPort < 10)
		{
			return g_InUart[wPort].Read(pbBuf, dwBufSize);
		}
		return 0;
	}

	//������ͨ�����ڷ�������
	//������@wPort ���ں�
	//      @pbData �����͵����ݻ�����
	//      @dwDataLen  �����͵����ݳ���
	//      @dwTimeouts ���ͳ�ʱʱ��(�������ʱ����û��������Ϊ����ʧ��)
	//���أ��ɹ����ͳ�ȥ�����ݳ���
	DWORD InUartWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts)
	{
		if (wPort < 10)
		{
			return g_InUart[wPort].Write(pbData, dwDataLen);
		}
		return 0;
	}
}
