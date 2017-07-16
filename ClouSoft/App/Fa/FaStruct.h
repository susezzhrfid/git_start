#ifndef FASTRUCT_H
#define FASTRUCT_H

#include "ComStruct.h"
//#include "DbStruct.h"
#include "FaConst.h"
#include "ThreadMonitor.h"
#include "StatMgr.h"
#include "DbCfg.h"

typedef struct{
	DWORD time;			//发生时间
	BYTE bVerInfo[9];	//版本信息
}TSoftVerChg;		//缓存的版本变更事件
/*
typedef struct {
	WORD wYxFlag;	//有效标志位,某位置1表示该位有效
	WORD wYxPolar;
}TYxPara;	//遥信参数
*/
typedef struct{
	BYTE bVer;              //版本,变成BYTE字节，避免发生对齐问题 
	bool fTmpValid;         //掉电暂存变量有效标志
	bool fGPRSConected;     //GPRS已连接
	BYTE bRemoteDownIP[8];  //远程下载软件的服务器IP地址
    WORD wLocalPort;        //终端本地端口号，每次使用自增，4097-9200
	bool fAlrPowerOff;		//掉电前上报了停电告警
	WORD wRstNum;			//复位次数
	WORD wMonitorRstNum;	//线程监控复位次数
	char szMonitorRstThrd[THRD_NAME_LEN];	//线程监控复位最后一次复位的线程名称
	short iRemoteDownPort;
	TTime tPowerOn;        //上次来电时间
	TTime tPoweroff;        //上次停电时间
	BYTE  bParaEvtType;		//记录掉电前参数初始化事件的类型：0无效 1重要 2一般
	TSoftVerChg ParaInit;	//参数初始化事件
    BYTE bAcAvrVolt[30];  //交采平均电压曲线,每两个字节为一个点BCD码
    TTermStat tTermStat;	//终端统计：包括日月供电时间、复位次数、流量
	TVoltStat tDayVoltStat;	//日电压统计
    TVoltStat tMonVoltStat; //月电压统计
	BYTE bSect2Data[SECT2_DATA_SZ];	//系统库需要掉电时保存的变量，
									//这些变量平常写得比较多，不能每次都直接保存到FLASH
	BYTE bBatTaskInfo[5]; // 掉电保存的对时任务的信息    
}TPowerOffTmp;   //掉电暂存变量

#endif //FASTRUCT_H

