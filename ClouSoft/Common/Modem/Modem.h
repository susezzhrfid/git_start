/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Modem.h
 * 摘    要：本文件实现了通信MODEM的基类定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：可作为GPRS,CDMA和电话MODEM的基类
 *********************************************************************************************************/
#ifndef MODEM_H
#define MODEM_H
#include "DrvConst.h"
#include "Comm.h"
//#include "drivers.h"
#include "ProIfCfg.h"

#define GSN_LEN    		 15
#define PHONE_NUM_LEN    32
#define ACTIVE_SMS_LEN   32

#define MODEM_STEP_INIT	 0	//初始化
#define MODEM_STEP_RST	 1	//复位模块
#define MODEM_STEP_SIM	 2	//检测SIM卡
#define MODEM_STEP_REG	 3	//注册网咯

#define MODEM_NO_ERROR	 0	//没有错误
#define MODEM_RST_FAIL	 1	//模块复位失败
#define MODEM_SIM_FAIL	 2  //检测SIM失败
#define	MODEM_REG_FAIL	 3	//注册网咯失败
#define MODEM_CSQ_FAIL   4  //更新场强失败
#define MODEM_OTHER_FAIL 5  //其它错误


typedef struct {
	BYTE bManuftr[4];	//厂商代号	ASCII	4
	BYTE bModel[8];		//模块型号	ASCII	8
	BYTE bSoftVer[4];	//软件版本号	ASCII	4
	BYTE bSoftDate[3];	//软件发布日期：日月年	见附录A.20	3
	BYTE bHardVer[4];	//硬件版本号	ASCII	4
	BYTE bHardDate[3];	//硬件发布日期：日月年	见附录A.20	3
	BYTE bCCID[20];					//ＳＩＭ卡ICCID	ASCII	20
}TModemInfo;	//模块信息

struct TModem{
	//参数
	BYTE  bComm;	//串口号
	struct TProIf* pProIf;	//指向GPRS接口的指针
	void* pvModem;		//MODEM子类数据,指向具体实例
	BYTE bModuleVer;
    TModemInfo tModemInfo;

	//虚函数，需要实例化为具体接口的对应函数
	int (* pfnResetModem)(struct TModem* pModem);			//复位模块
	int (* pfnUpdateSignStrength)(struct TModem* pModem);	//更新场强
	int (* pfnInitAPN)(struct TModem* pModem);				//初始化APN

	bool (* pfnOpenPpp)(struct TModem* pModem);				//PPP拨号上网
	bool (* pfnClosePpp)(struct TModem* pModem);			//断开PPP连接
	bool (* pfnConnect)(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort); //作为客户端连接服务器
	bool (* pfnCloseCliSock)(struct TModem* pModem);	//关闭客户端socket
	int (* pfnCliSend)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//作为客户端发送数据           //int支持错误返回
	int (* pfnCliReceive)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//作为客户端接收数据
	bool (* pfnChkCliStatus)(struct TModem* pModem);	//检查客户端的socket连接是否依然有效：要主动发命令查询

	bool (* pfnSpecHandle)(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize); //对模块主动发过来的的数据进行特殊处理,如TCP/UDP报文、收到来自监听端口的新连接

#ifdef EN_PROIF_SVR		//支持服务器
	bool (* pfnListen)(struct TModem* pModem, bool fUdp, WORD wLocalPort);  //作为服务器监听端口
	bool (* pfnCloseSvrSock)(struct TModem* pModem);	//关闭服务器已经连接的socket
	bool (* pfnCloseListen)(struct TModem* pModem);	//关闭监听

	int (* pfnSvrSend)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//作为服务器发送数据
	int (* pfnSvrReceive)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//作为服务器接收数据

	bool (* pfnIsSvrAcceptOne)(struct TModem* pModem);		//作为服务器是否接收到一个客户端的连接
	bool (* pfnChkSvrStatus)(struct TModem* pModem);	//检查服务器的socket连接是否依然有效：要主动发命令查询
#endif //EN_PROIF_SVR		//支持服务器

	//数据
	BYTE bStep;		//当前的操作步骤
	//char* pszCSQ;
    
    //int pd;    
};  //MODEM基类

void ModemInit(struct TModem* pModem);
int ATCommand(struct TModem* pModem, char* pszCmd, char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds);
int WaitModemAnswer(struct TModem* pModem, char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds);
bool ATCmdTest(struct TModem* pModem, WORD wTimes);
int UpdateSignStrength(struct TModem* pModem);
//bool IsSignValid(WORD wSignStrength);
//void SignLedCtrl(BYTE bSignStrength);

int ATCmdGetInfo(struct TModem* pModem, char* pszCmd, char* pszAnsOK,
			  char* pszAnsErr, char* psRxHead, char *psBuf, WORD wBufSize,
              WORD nWaitSeconds);
bool GetModemVer(struct TModem* pModem);
bool GetModemCCID(struct TModem* pModem);

#endif  //MODEM_H



