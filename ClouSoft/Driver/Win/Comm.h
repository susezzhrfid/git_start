// Comm.h: interface for the CComm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMM_H__1ED6BFD0_CF4D_42C0_A67E_FB91F700E5AE__INCLUDED_)
#define AFX_COMM_H__1ED6BFD0_CF4D_42C0_A67E_FB91F700E5AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WM_DISPLAY                  WM_USER+2

#define BR_NUM      12
#define PARITY_NUM  3

extern char* g_szCommPortTab[];
extern DWORD g_dwBaudRateTab[BR_NUM];
extern BYTE g_bParityTab[PARITY_NUM];

//extern char* g_pszBaudRateStrTab[BR_NUM];
//extern char* g_pszParityStrTab[PARITY_NUM];

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

typedef struct {	
	WORD wPort; 
	DWORD dwBaudRate; 
	BYTE bByteSize; 
	BYTE bStopBits; 
	BYTE bParity;
}TCommPara; //串口配置 

class CComm  
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
	CComm();
	virtual ~CComm();

	bool SetDTR(){return true;};
	bool ClrDTR(){return true;};
	bool GetRI(void) { return false;};
	void SetTimeouts(DWORD dwTimeouts);
};

#endif // !defined(AFX_COMM_H__1ED6BFD0_CF4D_42C0_A67E_FB91F700E5AE__INCLUDED_)
