/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcConst.cpp
 * 摘    要：本文件主要实现对交流采样常量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 
 *********************************************************************************************************/
#ifndef ACCONST_H
#define ACCONST_H
#include "LibAcConst.h"

////////////////////////////////////////////////////////////////////////////
//基本常量定义
#ifdef VER_METER
#define RATE_PERIOD_NUM    14
#else
#define RATE_PERIOD_NUM    8
#endif  //VER_METER

#define MAX_DAY_CHART_NUM  8
#define MAX_HOLIDAY_NUM    13
#define MAX_ZONE_NUM	   14	
//#define RATE_PERIOD_NUM    8

  						   



//脉冲内部数据的索引
#define PULSE_VAL_P		0
#define PULSE_VAL_Q		1
#define PULSE_VAL_COS	2

#define PULSE_VAL_NUM	3

#define MAX_PULSE_TYPE  	4		//脉冲类型数
//脉冲属性
#define EP_POS    0		//正向有功
#define EQ_POS    1		//正向无功
#define EP_NEG    2		//反向有功
#define EQ_NEG    3		//反向无功

#define MAX_YMNUM  8


/////////////////////////////////////////////////////////////////////////////
//配置
#define AC_ENERGY_NUM   	20//8  //交采的电能类型数量
#define AC_DEMAND_NUM   	8	//交采的需量类型数量	

#define PULSE_ENERGY_NUM   	1  //脉冲的电能类型数量
#define PULSE_DEMAND_NUM   	1	//脉冲的需量类型数量	
//一些旧的定义
#define   VOLTAGE_N              220
#define   CURRENT_N              500// 5A  NN.NN A
#define   POWER_N                11000//  1,1kW  NN.NNNN   kW
#define   COS_N                  1000//   N.NNN   
#define   POWER_3N               33000//  1,1kW  NN.NNNN   kW


#define	EP_MAX 9999999999L
#define	EQ_MAX 99999999L

#endif //ACCONST_H
