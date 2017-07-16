/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrStruct.h
 * 摘    要：本文件主要实现抄表类结构定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年1月
 *********************************************************************************************************/
#ifndef MTRSTRUCT_H
#define MTRSTRUCT_H
#include "TypeDef.h"
#include "Comm.h"
#include "MtrConst.h"
#include "TypeDef.h"
#include "ComStruct.h"


//TCctRdCtrl控制标志位定义
#define PH1			(1<<0)		//允许单相表抄读
#define PH3			(1<<1)		//允许3相表抄读
#define M97			(1<<2)		//允许97表抄读
#define M07			(1<<3)		//允许07表抄读

#define DMCUR		(1<<4)		//DAY&MONTH FROZEN CURRENT 在日月冻结配置成抄电表当前数据时抄读
#define DMMTR		(1<<5)		//DAY&MONTH FROZEN METER FRZ 在日月冻结配置成抄电表冻结时抄读
#define DFCUR		(1<<6)		//DAYFLG FROZEN CURRENT 在抄表日冻结配置成抄电表当前数据时抄读
#define DFMTR		(1<<7)		//DAYFLG FROZEN METER 在抄表日冻结配置成抄电表冻结时抄读
#define PFCUR		(1<<8)		//PROFILE FROZEN CURRENT 在曲线配置成抄电表当前数据时抄读
#define PFMTR		(1<<9)		//PROFILE FROZEN METER 在曲线配置成抄电表冻结时抄读
#define DEMDMCUR	(1<<10)		//DEMAND DAY&MONTH FROZEN CURRENT 需量在日月冻结配置成抄电表当前数据时抄读
#define DEMDMMTR	(1<<11)		//DEMAND DAY&MONTH FROZEN METER FRZ 需量在日月冻结配置成抄电表冻结时抄读
#define DEMDFCUR	(1<<12)		//DEMAND DAYFLG FROZEN CURRENT 需量在抄表日月冻结配置成抄电表当前数据时抄读
#define DEMDFMTR	(1<<13)		//DEMAND DAYFLG FROZEN METER FRZ 需量在抄表日月冻结配置成抄电表冻结时抄读
#define MSETT		(1<<14)		//MONTH FROZEN SETT 在月冻结配置成抄结算日时抄读
#define DEMMSETT	(1<<15)		//MONTH FROZEN SETT 在需量月冻结配置成抄结算日时抄读

#define PH13		(PH1|PH3)	//允许单相/3相表抄读
#define M0797		(M97|M07)	//允许07/97表抄读

#define MTR_FN_MAX	8

#define ALLPN		0xff
#define ERCPN		161 //电表事件从8开始213+8>FN_MAX && 213+41<255

typedef struct{
	WORD  wID;					//内部ID
	BYTE  bIntervU;				//抄表间隔单位
	BYTE  bIntervV;				//抄表间隔值
	WORD  wInterID;				//抄表开始时间ID(该项为0，则取tmStartTime为开始时间)
	TTime tmStartTime;			//开始时间
	//BYTE  bC1Fn[MTR_FN_MAX];	//本wID相关的1类FN,第一个字节表示FN的个数,后面才是一个个FN
	//							//在这里配置了相应的FN，如果F38配置成允许，则本wID需要抄
	BYTE  bFn[MTR_FN_MAX];		//本wID相关的1类FN,第一个字节表示FN的个数,后面才是一个个FN
								//在这里配置了相应的FN，如果F39配置成允许，则本wID需要抄
								//dwFnFlg[n]==ALLPN 所有测量点
								//dwFnFlg[n]==PNCHAR_VIP 重点户测量点
	DWORD dwFnFlg[MTR_FN_MAX];	//控制标志位,dwC2Flg[]的每个元素与bC2Fn[]一一对应
								//分别控制每个FN在什么情况下进行抄读
}TMtrRdCtrl;	//电表抄读控制结构

//电表参数
typedef struct
{
	WORD	wPn;				//测量点号		
	BYTE	bProId;				//协议号
	//BYTE	bSubProId;			//子协议号
	BYTE	bRateNum;			//费率数
	BYTE	bAddr[6];			//表地址	
	BYTE	bPassWd[6];			//密码
	BYTE	bRateTab[4];		//费率顺序

	TCommPara CommPara;			//串口通信参数	
}TMtrPara; 

#define MTR_COMSET_DEFAULT	0x4b	//抄口缺省参数 1200,E,8,1

typedef struct{
	BYTE* pbData;		
	WORD  wLen;
}TPnTmp;	//测量点导入导出的临时数据

#endif	//MTRSTRUCT_H