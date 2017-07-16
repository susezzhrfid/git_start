/*********************************************************************************************************
 * Copyright (c) 2012,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：YX.h
 * 摘    要：本文件主要实现YX检测
 * 当前版本：1.0
 * 作    者：李焱
 * 完成日期：2012年3月 
*********************************************************************************************************/
#ifndef YX_H
#define YX_H

#include "Typedef.h"
#include "stdlib.h"
#include "DbConst.h"

#define YX_MASK_SIZE       (MAX_P0_YX/8+1)

#define YX_FLAG_MASK       0X01
#define YX_COS_MASK        0X02
#define YX_SOE_MASK        0X04
#define YX_VIP_MASK        0X08
#define YX_POLAR_MASK      0X10

#define YX_FLAG_BIT         0
#define YX_COS_BIT          1
#define YX_SOE_BIT          2
#define YX_VIP_BIT          3
#define YX_POLAR_BIT        4

#define YX_STATE_ON         0   //合
#define YX_STATE_OFF        1   //分

typedef struct {
	//DWORD dwYxFlag;	//有效标志位,某位置1表示该位有效                 
	//DWORD dwYxPolar;
    //参数
    BYTE bYxFlag[YX_MASK_SIZE];  //有效标志位,某位置1表示该位有效        每位对应一路硬件遥信
    BYTE bYxPolar[YX_MASK_SIZE]; //极性                
    //BYTE bYxCos[YX_MASK_SIZE];   //是否生成COS
    //BYTE bYxSoe[YX_MASK_SIZE];   //是否生成SOE
    //BYTE bYxVIP[YX_MASK_SIZE];   //是否重要遥信
    //WORD wDelayMs;               //去抖时间
    BYTE bYXWobble[YX_MASK_SIZE];//抖动标志
    
    //数据
    BYTE bLastYxInput[YX_MASK_SIZE];
    BYTE bYxVal[YX_MASK_SIZE];
}TYxPara;	//遥信参数

bool YXInit(TYxPara* pYxPara);
void YXRun(void);
	
#endif //YX_H