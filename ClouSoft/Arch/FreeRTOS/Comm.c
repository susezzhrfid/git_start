#include "Comm.h"
#include "DrvCfg.h"
#include "Uarts.h"
#include "FaCfg.h"

bool g_fCommIsOpen[COMM_NUM] = { false };
DWORD g_dwBaudRate[COMM_NUM];
BYTE g_bByteSize[COMM_NUM];
BYTE g_bStopBits[COMM_NUM];
BYTE g_bParity[COMM_NUM];

//描述：打开串口
//参数：@wPort 串口号
//      @dwBaudRate 波特率
//      @bByteSize  数据位
//      @bStopBits 停止位
//      @bParity 校验位
//返回：true-成功；false-失败
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

//描述：关串口
//参数：@wPort 串口号
//返回：true-成功；false-失败
bool CommClose(WORD wPort)
{
    bool fRet;
    if (wPort >= COMM_NUM)
        return false;
        
    fRet = UartClose(wPort);
    if (fRet) //表示关成功了
        g_fCommIsOpen[wPort] = false;
    
    return fRet;
}

//描述：设置串口参数
//参数：@wPort 串口号
//      @dwBaudRate 波特率
//      @bByteSize  数据位
//      @bStopBits 停止位
//      @bParity 校验位
//返回：true-成功；false-失败
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

//描述：设置串口波特率
//参数：@wPort 串口号
//      @dwBaudRate 波特率
//返回：true-成功；false-失败
bool CommSetBaudRate(WORD wPort, DWORD dwBaudRate)
{
    if (wPort >= COMM_NUM)
        return false;    	
        
    if (CommIsOpen(wPort))
        CommClose(wPort);
    
    return CommOpen(wPort, dwBaudRate, g_bByteSize[wPort], g_bStopBits[wPort], g_bParity[wPort]);
}

//描述：获取串口参数
//参数：@wPort 串口号
//      @pCommPara 串口参数
//返回：true-成功；false-失败
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

//描述：判断串口是否已经被打开
//参数：@wPort 串口号
//返回：true-串口打开；false-串口关闭
bool CommIsOpen(WORD wPort)
{
    if (wPort >= COMM_NUM)
        return false;
    
    return g_fCommIsOpen[wPort];
}

//描述：通过串口读数据
//参数：@wPort 串口号
//      @pbBuf 接收数据缓冲区
//      @dwLength  缓冲区长度
//      @dwTimeouts 接收超时时间（超过该时间则认为无数据接收，则接收本次接收）
//返回：读到的数据长度
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

//描述：通过串口发送数据
//参数：@wPort 串口号
//      @pbData 待发送的数据缓冲区
//      @dwDataLen  待发送数据长度
//      @dwTimeouts 发送超时时间（超过该时间仍未发送成功则认为发送失败）
//返回：读到的数据长度
int CommWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts)
{
  	WORD wLen = 0;
    if (wPort >= COMM_NUM)
        return 0;    
    
    if (g_dwBaudRate[wPort] < 300)
        return 0;
    
    if (dwTimeouts == 0)
        dwTimeouts = (dwDataLen*((11+2)*1000/g_dwBaudRate[wPort]+1))<<1;  //每字节后2BIT停顿，加1小数向前进位,多给一倍的时间
    
    wLen = UartWrite(wPort, pbData, dwDataLen, dwTimeouts);
	
	if (wPort != COMM_GPRS)
		DoLedBurst(LED_LOCAL_TX);
	
	return wLen;
}
