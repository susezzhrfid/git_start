#ifndef COMCONST_H
#define COMCONST_H
//#include "FaCfg.h"

//������붨��
//#define ERR_OK       0x0
#define ERR_FORWARD  0x1
#define ERR_INVALID  0x2        //�������ݷǷ�
#define ERR_PERM     0x3
#define ERR_ITEM     0x4
#define ERR_TIME     0x5        //ʱ��ʧЧ
#define ERR_ADDR     0x11
#define ERR_SEND     0x12
#define ERR_SMS      0x13 

//#define ERR_TIMEOUT  0x20
#define ERR_SYS      0x30

#define ERR_IMG		 0x40

//ʱ�䵥λ�Ķ���
#define TIME_UNIT_MINUTE  2
#define TIME_UNIT_HOUR    3
#define TIME_UNIT_DAY     4
#define TIME_UNIT_MONTH   5	
#define TIME_UNIT_DAYFLG  10	//һ��������ձ�־λ,��Ҫ����ʵ��һ������Ķ���ն���
#define TIME_UNIT_MTR	  20	
#define TIME_UNIT_STA	21

//��������ʱ�䵥λ�Ķ���
#define TIME_UNIT_MINUTE_TASK  0
#define TIME_UNIT_HOUR_TASK    1
#define TIME_UNIT_DAY_TASK     2
#define TIME_UNIT_MONTH_TASK   3	

#endif  //COMCONST_H



 
