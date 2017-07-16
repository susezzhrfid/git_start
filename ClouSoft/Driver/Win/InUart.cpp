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

//定义最大数据块、XON和XOFF
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

UINT CommWatchProc( LPVOID pParam);  //物理层监察线程

CInUart::CInUart()
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hTxWnd = m_hRxWnd = NULL;
	m_pfnRxCallback = NULL;
	m_pvCallbackArg = NULL;

	//初始化操作
	m_fConnect = FALSE;  //断开连接菜单项无效
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

	// 为重叠读创建事件对象，手工重置，初始化为无信号的
	if ((m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		return;

	// 为重叠写创建事件对象，手工重置，初始化为无信号的
	if ((m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		return;

	// 为WaitCommEvent()创建事件对象，手工重置，初始化为无信号的
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


//配置串口连接
BOOL CInUart::ConfigConnection()
{
	DCB dcb;

	if (!GetCommState(m_hComm, &dcb))
		return FALSE;

	dcb.fBinary = TRUE;
	dcb.BaudRate = m_dwBaudRate; //数据传输速率
	dcb.ByteSize = m_bByteSize;  //每字节位数
	dcb.fParity = TRUE;

	dcb.Parity = m_bParity;      //校验设置 MARKPARITY;
	dcb.StopBits = m_bStopBits;  //停止位

	dcb.fOutxCtsFlow = FALSE;     //TRUE;     //硬件流控制设置
	dcb.fRtsControl = FALSE;      //TRUE;

	// XON/XOFF流控制设置
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

	if ( !ConfigConnection() ) //如果设置不成功，则原参数不变
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

	if ( !ConfigConnection() ) //如果设置不成功，则原参数不变
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

	TimeOuts.ReadTotalTimeoutMultiplier = 0;  //  //用来计算读操作总超时时间的系数,单位ms
	TimeOuts.ReadTotalTimeoutConstant = dwTimeouts;  //用来计算读操作总超时时间的系数,单位ms
	//读总超时时间计算公式：
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
		NULL); // 重叠方式

	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	SetupComm(m_hComm,MAXBLOCK,MAXBLOCK);
	//SetCommMask(m_hComm, EV_RXCHAR);


	// 把间隔超时设为最大，把总超时设为0将导致ReadFile立即返回并完成操作
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 100;       //MAXDWORD两个字符到达之间的最大时间间隔,单位ms
	TimeOuts.ReadTotalTimeoutMultiplier = 2;  //  //用来计算读操作总超时时间的系数,单位ms
	TimeOuts.ReadTotalTimeoutConstant = 1000;  //用来计算读操作总超时时间的系数,单位ms
	//读总超时时间计算公式：
	//ReadTotalTimeout = (ReadTotalTimeoutMultiplier * bytes_to_read) + ReadTotalTimeoutConstant;

	// 设置写超时以指定WriteComm成员函数中的GetOverlappedResult函数的等待时间
	TimeOuts.WriteTotalTimeoutMultiplier = 50;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	//写总超时时间计算公式：
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
		NULL);   //创建并挂起线程

	if (m_pThread == NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	else
	{
		m_pThread->ResumeThread(); // 恢复线程运行
	}
	return TRUE;
}


// 将指定数量的字符从串行口输出
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
			GetOverlappedResult(m_hComm, &m_osWrite, &length, TRUE);// 等待
			//等待挂起的异步操作完成，当函数返回时I/O操作已经完成
		}
		else
		{  
			length = 0;
		}
	}

	if (m_hRxWnd!=NULL && IsWindow(m_hRxWnd) && length!=0)  //发送一帧，把它输出到输出窗口
	{
		BYTE* pMsgData = new BYTE[length];
		memcpy(pMsgData, lpBuf, length);
		DWORD dwPara = 0x10000 + length;   //Direction | Length
		//::PostMessage(m_hRxWnd, WM_COMM_NOTIFY, (WPARAM)pMsgData, (LPARAM)dwPara);
	}

	return length;
}


//描述：设置串口的回调函数及参数
//参数：@pfnRxCallback 当物理层接收到数据时，回调链路层的接收函数
//      @pvCallbackArg 回调时传递链路层的参数
//返回：无
void CInUart::SetCallback(void (*pfnRxCallback)(BYTE* pbBlock, WORD wLen, void* pvArg), 
						  void* pvCallbackArg)
{
	m_pfnRxCallback = pfnRxCallback;
	m_pvCallbackArg = pvCallbackArg;
}


//描述：设置接收数据的显示窗口
//参数：@hRxWnd 接收数据的显示窗口
//返回：无
void CInUart::SetRxTxWnd(HWND hRxWnd, HWND hTxWnd)
{
	m_hRxWnd = hRxWnd;
	m_hTxWnd = hTxWnd;
}

// 从串行口输入缓冲区中读入指定数量的字符
DWORD CInUart::Read(LPVOID buf, DWORD dwLength)
{
	DWORD length = 0;
	COMSTAT ComStat;
	DWORD dwErrorFlags;	

	ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
	length = dwLength; //min(dwLength, ComStat.cbInQue);

	ReadFile(m_hComm, buf, dwLength, &length, &m_osRead);

	/*
	//将接收到的数据更新到串口接收监视界面，
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
//物理层监察线程
//读到的数据放到循环队列thePhy.m_PhyData.RcvBuf
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
		//在串口通信中发生错误，如终端、奇偶错误等，I/O操作将会终止
		//要进一步执行I/O操作，必须调用ClearCommError(),它的作用是：
		//1).清楚错误条件，2).确定串口通信状态
		if (ComStat.cbInQue)
		{
			DWORD dwRcvLen = pComm->Read(bRcvBuf, MAXBLOCK);
			if (dwRcvLen)
			{
				//调用通信协议的接收组帧回调函数
				if (pComm->m_pfnRxCallback != NULL)
				{
					pComm->m_pfnRxCallback(bRcvBuf, (WORD)dwRcvLen, pComm->m_pvCallbackArg);
				}
				if (pComm->m_hRxWnd != NULL)
				{ //将接收到的数据更新到串口接收监视界面，
					PBYTE pbData = new BYTE[dwRcvLen];
					memcpy(pbData, bRcvBuf, dwRcvLen);
					::PostMessage(pComm->m_hRxWnd, WM_DISPLAY,
						(WPARAM)pbData, (LPARAM)dwRcvLen); 
				}
			} 
			continue;
		}

		dwMask = 0;

		if (!WaitCommEvent(pComm->m_hComm, &dwMask, &pComm->m_osWait)) // 重叠操作
		{                           //当需要关闭串口时，可以通过使m_osWait.hEvent得到通知而返回
			if (GetLastError() == ERROR_IO_PENDING)
			{	
				GetOverlappedResult(pComm->m_hComm, &pComm->m_osWait, &dwTrans, TRUE); //无限等待重叠操作结果
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
	//描述：初始化串口(信号量初始化等)
	//参数：@wPort 串口号
	//      @dwBaudRate 波特率
	//      @bByteSize  数据位
	//      @bStopBits  停止位
	//      @bParity  校验位
	//返回：0-没有错误
	int InUartInit(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
	{
		return 0;
	}

	//描述：打开串口
	//参数：@wPort 串口号
	//      @dwBaudRate 波特率
	//      @bByteSize  数据位
	//      @bStopBits  停止位
	//      @bParity  校验位
	//返回：0-没有错误
	int InUartOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
	{
		if (wPort < 10)
		{
			if (g_InUart[wPort].Open(wPort, dwBaudRate, bByteSize, bStopBits, bParity))
				return 0;
		}
		return -1;
	}

	//描述：关闭串口
	//参数：无
	//返回：0-没有错误
	int InUartClose(WORD wPort)
	{
		if (wPort < 10)
		{
			if (g_InUart[wPort].Close())
				return 0;
		}
		return -1;
	}

	//描述：设置串口（波特率等）
	//参数：@wPort 串口号
	//      @dwBaudRate 波特率
	//      @bByteSize  数据位
	//      @bStopBits  停止位
	//      @bParity  校验位
	//返回：0-没有错误
	int InUartSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
	{
		if (wPort < 10)
		{
			if (g_InUart[wPort].SetComm(dwBaudRate,bByteSize, bStopBits, bParity))
				return 0;
		}
		return -1;
	}

	//描述：通过串口读数据
	//参数：@wPort 串口号
	//      @pbBuf 读缓冲区
	//      @dwBufSize  缓冲区大小
	//      @dwTimeouts 接收超时时间（超过这么长时间仍没收到数据则认为超时）
	//返回：读到的数据长度
	DWORD InUartRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts)
	{
		if (wPort < 10)
		{
			return g_InUart[wPort].Read(pbBuf, dwBufSize);
		}
		return 0;
	}

	//描述：通过串口发送数据
	//参数：@wPort 串口号
	//      @pbData 待发送的数据缓冲区
	//      @dwDataLen  待发送的数据长度
	//      @dwTimeouts 发送超时时间(超过这个时间仍没发出则认为发送失败)
	//返回：成功发送出去的数据长度
	DWORD InUartWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts)
	{
		if (wPort < 10)
		{
			return g_InUart[wPort].Write(pbData, dwDataLen);
		}
		return 0;
	}
}
