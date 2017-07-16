/*********************************************************************************************************
 * Copyright (c) 2012,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�YX.h
 * ժ    Ҫ�����ļ���Ҫʵ��YX���
 * ��ǰ�汾��1.0
 * ��    �ߣ�����
 * ������ڣ�2012��3�� 
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

#define YX_STATE_ON         0   //��
#define YX_STATE_OFF        1   //��

typedef struct {
	//DWORD dwYxFlag;	//��Ч��־λ,ĳλ��1��ʾ��λ��Ч                 
	//DWORD dwYxPolar;
    //����
    BYTE bYxFlag[YX_MASK_SIZE];  //��Ч��־λ,ĳλ��1��ʾ��λ��Ч        ÿλ��Ӧһ·Ӳ��ң��
    BYTE bYxPolar[YX_MASK_SIZE]; //����                
    //BYTE bYxCos[YX_MASK_SIZE];   //�Ƿ�����COS
    //BYTE bYxSoe[YX_MASK_SIZE];   //�Ƿ�����SOE
    //BYTE bYxVIP[YX_MASK_SIZE];   //�Ƿ���Ҫң��
    //WORD wDelayMs;               //ȥ��ʱ��
    BYTE bYXWobble[YX_MASK_SIZE];//������־
    
    //����
    BYTE bLastYxInput[YX_MASK_SIZE];
    BYTE bYxVal[YX_MASK_SIZE];
}TYxPara;	//ң�Ų���

bool YXInit(TYxPara* pYxPara);
void YXRun(void);
	
#endif //YX_H