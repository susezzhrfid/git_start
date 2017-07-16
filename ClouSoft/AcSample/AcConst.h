/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcConst.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԽ������������Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 * ��    ע: 
 *********************************************************************************************************/
#ifndef ACCONST_H
#define ACCONST_H
#include "LibAcConst.h"

////////////////////////////////////////////////////////////////////////////
//������������
#ifdef VER_METER
#define RATE_PERIOD_NUM    14
#else
#define RATE_PERIOD_NUM    8
#endif  //VER_METER

#define MAX_DAY_CHART_NUM  8
#define MAX_HOLIDAY_NUM    13
#define MAX_ZONE_NUM	   14	
//#define RATE_PERIOD_NUM    8

  						   



//�����ڲ����ݵ�����
#define PULSE_VAL_P		0
#define PULSE_VAL_Q		1
#define PULSE_VAL_COS	2

#define PULSE_VAL_NUM	3

#define MAX_PULSE_TYPE  	4		//����������
//��������
#define EP_POS    0		//�����й�
#define EQ_POS    1		//�����޹�
#define EP_NEG    2		//�����й�
#define EQ_NEG    3		//�����޹�

#define MAX_YMNUM  8


/////////////////////////////////////////////////////////////////////////////
//����
#define AC_ENERGY_NUM   	20//8  //���ɵĵ�����������
#define AC_DEMAND_NUM   	8	//���ɵ�������������	

#define PULSE_ENERGY_NUM   	1  //����ĵ�����������
#define PULSE_DEMAND_NUM   	1	//�����������������	
//һЩ�ɵĶ���
#define   VOLTAGE_N              220
#define   CURRENT_N              500// 5A  NN.NN A
#define   POWER_N                11000//  1,1kW  NN.NNNN   kW
#define   COS_N                  1000//   N.NNN   
#define   POWER_3N               33000//  1,1kW  NN.NNNN   kW


#define	EP_MAX 9999999999L
#define	EQ_MAX 99999999L

#endif //ACCONST_H
