/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：InUart.h
 * 摘    要：CPU自带的串口驱动
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
 ******************************************************************************/
#ifndef UART_H
#define UART_H
//#include "Typedef.h"
#include "Comm.h"

#define WM_DISPLAY                  WM_USER+2

#define BR_NUM      12
#define PARITY_NUM  3

typedef unsigned short u_short;

typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef unsigned long long  uint64; 
typedef unsigned long long  DDWORD;

typedef unsigned char BYTE;
typedef unsigned short WORD;


#define BR_1200      0
#define BR_2400      1  
#define BR_4800      2
#define BR_9600      3
#define BR_14400     4 
#define BR_19200     5  
#define BR_38400     6   
#define BR_56000     7 
#define BR_57600     8 
#define BR_115200    9 
#define BR_128000    10
#define BR_256000    11 

#define _PARITY_NO    0
#define _PARITY_ODD   1
#define _PARITY_EVEN  2


class CInUart  
{
private:
	CString  m_strPort;
	WORD	 m_wPort; 
	DWORD	 m_dwBaudRate;
	BYTE	 m_bByteSize;
	BYTE	 m_bStopBits;
	BYTE	 m_bParity;

	CWinThread* m_pThread;
	OVERLAPPED m_osRead, m_osWrite;  //用于重叠读/写
private:
	BOOL ConfigConnection();

public:
	HANDLE   m_hComm;
	BOOL     m_fConnect;
	BOOL     m_fWatching;
	OVERLAPPED m_osWait;             //用于WaitCommEvent()
	DWORD m_dwTimeouts;

	//与上层的接口
	HWND  m_hRxWnd;                 //接收数据的显示窗口
	HWND  m_hTxWnd;
	void  (*m_pfnRxCallback)(BYTE* pbBlock, WORD wLen, void* pvArg); 
	//当物理层接收到数据时，回调链路层的接收函数
	void* m_pvCallbackArg;          //回调时传递链路层的参数
public:
	BOOL Config(LPCTSTR szPort, DWORD dwBaudRate, BYTE bByteSize,
		BYTE bStopBits, BYTE bParity);
	BOOL Config(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,
		BYTE bStopBits, BYTE bParity);	
	BOOL Config(TCommPara &CommPara);

	BOOL Open(LPCTSTR szPort, DWORD dwBaudRate, BYTE bByteSize,
		BYTE bStopBits, BYTE bParity);

	BOOL Open(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,
		BYTE bStopBits, BYTE bParity);
	BOOL Open(TCommPara &CommPara);
	BOOL SetComm(DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);
	bool GetCommPara(TCommPara* pCommPara);

	BOOL  Open();
	BOOL  Close(void);
	BOOL  BeginWatch(void);
	DWORD Write(LPCVOID lpBuf, DWORD dwLength);
	DWORD Read(LPVOID buf, DWORD dwLength);
	DWORD Read(LPVOID buf, DWORD dwLength, DWORD dwTimeout);
	BOOL  IsOpen();
	BOOL SetBaudRate(DWORD dwBaudRate);
	void SetRxTxWnd(HWND hRxWnd, HWND hTxWnd);
	void SetCallback(void (*pfnRxCallback)(BYTE* pbBlock, WORD wLen, void* pvArg), 
		void* pvCallbackArg);
public:
	CInUart();
	virtual ~CInUart();

	bool SetDTR(){return true;};
	bool ClrDTR(){return true;};
	bool GetRI(void) { return false;};
	void SetTimeouts(DWORD dwTimeouts);
};

#endif
