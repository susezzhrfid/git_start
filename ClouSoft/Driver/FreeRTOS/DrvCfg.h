/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DrvCfg.h
 * 摘    要：本文件用来定义不同硬件环境下的各种配置
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
  *********************************************************************************************************/
#ifndef DRVCFG_H
#define DRVCFG_H

#define COMM_NUM       5

#if 0
#define  COMM_METER    1    //抄表口II        
#define  COMM_METER2   2    //抄表口III 
#define  COMM_METER3   0    //抄表口IV 
#define  COMM_GPRS     3    //GPRS          
#define  COMM_TEST     0    //485维护口I        
#define  COMM_LOCAL	   4    //红外维护口     

#define COMM_FARIR      COMM_LOCAL
#define COMM_DEBUG      COMM_TEST
#elif 1

#define  COMM_METER    1 //0    //抄表口II        
#define  COMM_METER2   0 //1    //抄表口III 
#define  COMM_METER3   4 //2    //抄表口IV 
#define  COMM_GPRS     3    //GPRS          
#define  COMM_TEST     1 //0    //485维护口I        
#define  COMM_LOCAL	   2 //4    //红外维护口     

#define COMM_FARIR      COMM_LOCAL
#define COMM_DEBUG      COMM_TEST
#define COMM_CCT_PLC    COMM_METER3
#endif



#endif //DRVCFG_H