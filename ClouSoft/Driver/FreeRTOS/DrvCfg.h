/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DrvCfg.h
 * ժ    Ҫ�����ļ��������岻ͬӲ�������µĸ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
  *********************************************************************************************************/
#ifndef DRVCFG_H
#define DRVCFG_H

#define COMM_NUM       5

#if 0
#define  COMM_METER    1    //�����II        
#define  COMM_METER2   2    //�����III 
#define  COMM_METER3   0    //�����IV 
#define  COMM_GPRS     3    //GPRS          
#define  COMM_TEST     0    //485ά����I        
#define  COMM_LOCAL	   4    //����ά����     

#define COMM_FARIR      COMM_LOCAL
#define COMM_DEBUG      COMM_TEST
#elif 1

#define  COMM_METER    1 //0    //�����II        
#define  COMM_METER2   0 //1    //�����III 
#define  COMM_METER3   4 //2    //�����IV 
#define  COMM_GPRS     3    //GPRS          
#define  COMM_TEST     1 //0    //485ά����I        
#define  COMM_LOCAL	   2 //4    //����ά����     

#define COMM_FARIR      COMM_LOCAL
#define COMM_DEBUG      COMM_TEST
#define COMM_CCT_PLC    COMM_METER3
#endif



#endif //DRVCFG_H