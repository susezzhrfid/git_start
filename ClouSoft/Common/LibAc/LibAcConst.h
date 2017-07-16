/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LibAcConst.h
 * 摘    要：本文件主要实现对交流采样常量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 * 备    注: 
 *********************************************************************************************************/
#ifndef LIBACCONST_H
#define LIBACCONST_H

#define RATE_NUM           4

#define FREQ_UNIT    256      //256              提高运算精度,先乘上，最后再除掉
#define FREQ_SHIFT   8        
#define FFT_NUM	     32

//#define SCN_NUM           1    //最大支持的采样的通道数,改变该宏定义时注意同时改变数据库里校准参数0x2060的长度!!!!!
#define SCN_NUM      1   //新南网集中器没有交采数据,标零导致数组数据异常  Changed By Wing 2014/07/29


#define NUM_PER_CYC       FFT_NUM  	//64 每个周期采集的样点数
#define CYC_NUM		      1    //每个通道缓存多少个周期的样点
#define SBUF_SIZE         (NUM_PER_CYC*CYC_NUM)  //每个通道缓存的样点数

#define FREQ_CYC_NUM      256 //200     //频率计算的周期数

#define NUM_PER_CYC_45HZ  (NUM_PER_CYC*50/45)  //每个周期采集的样点数
#define NUM_PER_CYC_55HZ  (NUM_PER_CYC*50/55 + 1)  //每个周期采集的样点数

//FREQ_CYC_NUM个周波最大与最小的采集点数，这是由于调整了采样周期导致的。
#define NUM_PER_MAX_PN    (FREQ_CYC_NUM*NUM_PER_CYC*55/45 + 1)
#define NUM_PER_MIN_PN    (FREQ_CYC_NUM*NUM_PER_CYC*45/55)


//#define E_CONST        6000
						//  (瓦 *秒) / 脉冲常数

#define  E_PER_PULSE  0x1C9C3800 //(10L*1000L*3600L*1000L*10*8/6000)    //每个脉冲等于多少个 瓦/10 * 毫秒/8
  					 //  (瓦 *秒 *毫秒*瓦/10*毫秒/8) / 脉冲常数
												
//#define E_PER_PULSE    0x1C9C3800  //(1000*3600*1000*10*8/E_CONST)*10    //每个脉冲等于多少个 瓦/10 * 毫秒/8
  						  //  (瓦 *秒 *毫秒*瓦/10*毫秒/8) / 脉冲常数

//#define E_PER_PULSE       0xABA9500 //0x112A880 //(1000*3600*1000*10*8/16000)    //每个脉冲等于多少个 瓦/10 * 毫秒/8
  						 //  (瓦 *秒 *毫秒*瓦/10*毫秒/8) / 脉冲常数



#define QUAD_POS_P        0x00
#define QUAD_NEG_P        0x01 
#define QUAD_POS_Q        0x00
#define QUAD_NEG_Q        0x02

//终端内计算的电能的序号
#define EP_POS_A	0
#define EP_POS_B	1
#define EP_POS_C	2
#define EP_POS_ABC	3

#define EP_NEG_A	4
#define EP_NEG_B	5
#define EP_NEG_C	6
#define EP_NEG_ABC	7

#define EQ_POS_A	8
#define EQ_POS_B	9
#define EQ_POS_C	10
#define EQ_POS_ABC	11

#define EQ_NEG_A	12
#define EQ_NEG_B	13
#define EQ_NEG_C	14
#define EQ_NEG_ABC	15

#define EQ_Q1		16
#define EQ_Q2		17
#define EQ_Q3		18
#define EQ_Q4		19	


//交采内部数据的索引
#define AC_VAL_UA	0
#define AC_VAL_UB	1
#define AC_VAL_UC	2
#define AC_VAL_IA	3
#define AC_VAL_IB	4
#define AC_VAL_IC	5

#define AC_VAL_P	6
#define AC_VAL_PA	7
#define AC_VAL_PB	8
#define AC_VAL_PC	9
#define AC_VAL_Q	10
#define AC_VAL_QA	11
#define AC_VAL_QB	12
#define AC_VAL_QC	13

#define AC_VAL_COS	14
#define AC_VAL_COSA	15
#define AC_VAL_COSB	16
#define AC_VAL_COSC	17

#define AC_VAL_ANG_UA	18
#define AC_VAL_ANG_UB	19
#define AC_VAL_ANG_UC	20
#define AC_VAL_ANG_IA	21
#define AC_VAL_ANG_IB	22
#define AC_VAL_ANG_IC	23

#define AC_VAL_PRO_IA	24		//A向保护电流
#define AC_VAL_PRO_IB	25		//B向保护电流
#define AC_VAL_PRO_IC	26		//C向保护电流
#define AC_VAL_PRO_I0	27		//零序保护电流
#define AC_VAL_I0		28		//零序电流
#define AC_VAL_U0		29		//零序电压
#define AC_VAL_F		30		//频率

#define AC_VAL_PHASESTATUS	25	//相序状态
#define AC_VAL_MTRSTATUS	27	//电表状态字

#define AC_VAL_S			28
#define AC_VAL_SA			29	
#define AC_VAL_SB			30
#define AC_VAL_SC			31
							 
#define AC_VAL_NUM			(AC_VAL_F+1)


//无功电量累加标志配置时用到的常量定义
#define QADD	3
#define QSUB	2
#define Q1ADD	QADD
#define Q1SUB	QSUB
#define Q2ADD	(QADD<<2)
#define Q2SUB	(QSUB<<2)
#define Q3ADD	(QADD<<4)
#define Q3SUB	(QSUB<<4)
#define Q4ADD	(QADD<<6)
#define Q4SUB	(QSUB<<6)


//相序状态标志
#define DISORDER_U      0x01
#define DISORDER_I      0x02

#define	ID_TO_RATENUM(id)	( (id&0x000f)==0x000f ? RATE_NUM+1 : 1 )

/////////////////////////////////////////////////////////////////////////////
//配置
#define ACLOG_ENABLE	1 

#define AD_CHK_DELAY      (NUM_PER_CYC*50*2)
						
#define CONNECT_1P    	1	//单相表
#define CONNECT_3P3W    3
#define CONNECT_3P4W    4

//一下量的最大允许个数
#define HARM_NUM_MAX	15	//谐波最大允许次数
#define ENERGY_NUM_MAX	50	//电量最大允许个数
#define DEMAND_NUM_MAX 	50	//需量最大允许个数

#endif //LIBACCONST_H
