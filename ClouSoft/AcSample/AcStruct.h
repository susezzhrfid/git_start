/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcStruct.h
 * 摘    要：本文件主要实现对交流采样数据结构定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 
 *********************************************************************************************************/
#ifndef ACSTRUCT_H
#define ACSTRUCT_H
#include "Typedef.h"
#include "DbStruct.h"
#include "LibAcStruct.h"

typedef struct{
	DWORD dwStartTime;
	WORD  wRate;
}TRatePeriod;   //电表的时段费率定义

typedef struct{
	WORD  nMonth;
	WORD  nDay;
	WORD  wDayChart;
}TZone;   //电表的时段费率定义

typedef struct{
	WORD 		wZoneNum;      //年时区数
	WORD		wDayChartNum;  //日时段表数
	WORD		wPeriodNum;    //日时段数
	WORD		wRateNum;      //费率数
	WORD		wHolidayNum;   //公共假日数
	WORD		wRestDayChart;  //周休日采用的日时段表号
	BYTE		bRestStatus;    //周休日状态字
	TZone		zZone[MAX_ZONE_NUM];         //年时区表
	TZone	    zHoliday[MAX_HOLIDAY_NUM];   //公共假日表
	TRatePeriod rpDayChart[MAX_DAY_CHART_NUM][RATE_PERIOD_NUM];  //日时段表
}TTOU;  //645协议规定的整套时段费率表从数据库转到内存变量

typedef void (* TPfnAcValToFmt)(int val, BYTE* pbBuf, WORD wLen);

typedef struct{
	bool fDuoPn;  	//该数据项是否支持双测量点号入库,
					//主要是针对国标交采的测量点号可以配置,固定入测量点0,另外一个根据配置来入
					//其它BANK的扩展的数据项,由于都是自己用的数据项,所以一般都不支持双测量点
	WORD wBn;  		//BANK号
	WORD wID;     	//数据项ID,
					//如果为块ID,则wInnerID配为第一个ID的索引,wSubNum为子ID的个数
	WORD wIdx;		//内部计算的索引
	WORD wSubNum;	//子ID的个数
	WORD wLen;		//单个数据项的长度
	TPfnAcValToFmt pfnAcValToFmt;	//格式转换函数
	
	//以下部分由程序自动初始化
	TDataItem diPn0;
	TDataItem diPn;
}TAcValToDbCtrl;	//交采数据入库控制

typedef struct{
	WORD wBn;  		//BANK号
	WORD wID;     	//数据项ID,
					//如果为块ID,则wInnerID配为第一个ID的索引,wSubNum为子ID的个数
	WORD wIdx;		//内部计算的索引
	WORD wSubNum;	//子ID的个数
	WORD wLen;		//单个数据项的长度
	TPfnAcValToFmt pfnAcValToFmt;	//格式转换函数
	
	//以下部分由程序自动初始化	
	TDataItem diPn[MAX_YMNUM];
}TPulseValToDbCtrl;	//脉冲数据入库控制



#endif //ACSTRUCT_H
