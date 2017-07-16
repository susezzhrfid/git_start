/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：GprsIf.h
 * 摘    要：本文件实现了GPRS通信接口类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef GPRSIF_H
#define GPRSIF_H
#include "ProIf.h"
#include "Modem.h"
#include "SocketIf.h"

//通道的通信模式
#define CN_MODE_SOCKET      0	//基于TCP/IP的通信模式
#define CN_MODE_SMS      	1	//短信
#define CN_MODE_EMBED     	2	//模块嵌入式协议栈
#define CN_MODE_COMM     	3	//串口通信模式
#define CN_MODE_CMD     	4	//命令模式

#define CN_TYPE_GPRS        0   //GPRS
#define CN_TYPE_ET          1   //以太网

//在线模式
#define ONLINE_M_PERSIST    1	//永久在线模式
#define ONLINE_M_ACTIVE     2	//激活模式/非连续在线模式
#define ONLINE_M_PERIOD		3	//时段在线模式
#define ONLINE_M_SMS		4   //短信方式
#define ONLINE_M_JIT		5	//JUST IN TIME 按需要即时上线,如单独的上报端口
#define ONLINE_M_MIX		6	//服务器和客户端的混合模式

#define ONLINE_MODE			0
#define NONONLINE_MODE		1

typedef struct{
	//参数
	BYTE bOnlineMode;		//在线模式
	BYTE bCliBeatMin;		//客户端心跳间隔,单位分钟
	BYTE bSvrBeatMin;		//服务器心跳间隔,单位分钟
	BYTE bRstToDormanNum;	//复位到休眠的次数:任何情况的失败会导致模块复位，当模块复位次数到达这个次数后，进入休眠状态
	BYTE bConnectNum;		//连接失败连续尝试的次数
	BYTE bLoginNum; 		//登录失败连续尝试的次数
	WORD wDormanInterv;		//休眠时间间隔, 单位秒, ,0表示禁止休眠模式
	WORD wActiveDropSec; 	 //非连续在线模式的自动掉线时间,单位秒
	bool fEnableAutoSendActive; //允许主动上报激活
	struct TModem* pModem;		//GPRS模块
    
    TSocketIf *pSocketIf;       //终端协议栈，以太网的接口
    	
	DWORD dwFluxOverClick;	//流量超标的起始时标
   
	//数据
	bool fSvrTrans;		//当前处于服务器传输状态：主要用于混合模式下，用来区分是服务器还是客户端在传输，
						//要在调用发送接收函数前设定
	BYTE bCliState;		//客户机状态机：连接->登陆->通信->主动断开->空闲
	BYTE bSvrState;		//服务器状态机：监听->接收连接->服务器通信->
	BYTE bCnMode;		//通道模式,短信方式下设置为CN_MODE_SMS,
						//GPRS方式下设置为CN_MODE_SOCKET或CN_MODE_EMBED,
						//非连续在线方式下特指GPRS的通道模式,设置为CN_MODE_SOCKET或CN_MODE_EMBED,
    BYTE bCnType;       //通道类型以太网，GPRS
	BYTE bSignStrength;	//信号强度
	char cLastErr;		//最后的错误
    bool fCnSw;         //通道切换
    int ipd;            //pppoe
    DWORD dwNoRxRstAppInterv;  //无接收复位终端间隔,单位秒,0表示不复位      只有GPRS，以太网口才做了这个复位
}TGprsIf;	//GPRS通信接口子类结构


////////////////////////////////////////////////////////////////////////////////////////////
//GprsIf公共函数定义
bool GprsIfInit(struct TProIf* pProIf);
bool GprsIfRxFrm(struct TProIf* pProIf, BYTE* pbRxBuf, int iLen, bool fSvr); //外部传入接收数据并处理帧
extern BYTE GetSign(struct TProIf* pProIf);
extern BYTE GetSignStrength();
extern void UpdateSvrRxClick();
extern bool IsOnlineState(struct TProIf* pProIf);

void CheckNetStat(void);

#endif  //GPRSIF_H
