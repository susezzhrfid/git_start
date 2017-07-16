/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Comm.h
 * 摘    要：系统串口通信封装头文件
 * 当前版本：1.0.0
 * 作    者：杨进
 * 完成日期：2011年1月
 * 备    注：
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
}TCommPara; //串口配置 

#define MAX_COMM_NUM    10


//描述：打开串口
//参数：@wPort 串口号
//      @dwBaudRate 波特率
//      @bByteSize  数据位
//      @bStopBits 停止位
//      @bParity 校验位
//返回：true-成功；false-失败
bool CommOpen(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,	BYTE bStopBits, BYTE bParity);

//描述：关串口
//参数：@wPort 串口号
//返回：true-成功；false-失败
bool CommClose(WORD wPort);

//描述：设置串口参数
//参数：@wPort 串口号
//      @dwBaudRate 波特率
//      @bByteSize  数据位
//      @bStopBits 停止位
//      @bParity 校验位
//返回：true-成功；false-失败
bool CommSetup(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);

//描述：设置串口波特率
//参数：@wPort 串口号
//      @dwBaudRate 波特率
//返回：true-成功；false-失败
bool CommSetBaudRate(WORD wPort, DWORD dwBaudRate);

//描述：获取串口参数
//参数：@wPort 串口号
//      @pCommPara 串口参数
//返回：true-成功；false-失败
bool CommGetPara(WORD wPort, TCommPara* pCommPara);

//描述：判断串口是否已经被打开
//参数：@wPort 串口号
//返回：true-串口打开；false-串口关闭
bool CommIsOpen(WORD wPort);

//描述：通过串口读数据
//参数：@wPort 串口号
//      @pbBuf 接收数据缓冲区
//      @dwLength  缓冲区长度
//      @dwTimeouts 接收超时时间（超过该时间则认为无数据接收，则接收本次接收）
//返回：读到的数据长度
DWORD CommRead(WORD wPort, BYTE* pbBuf, DWORD dwBufSize, DWORD dwTimeouts);

//描述：通过串口发送数据
//参数：@wPort 串口号
//      @pbData 待发送的数据缓冲区
//      @dwDataLen  待发送数据长度
//      @dwTimeouts 发送超时时间（超过该时间仍未发送成功则认为发送失败）
//返回：读到的数据长度
DWORD CommWrite(WORD wPort, BYTE* pbData, DWORD dwDataLen, DWORD dwTimeouts);

#endif
