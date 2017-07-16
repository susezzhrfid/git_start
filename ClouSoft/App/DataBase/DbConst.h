/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbStruct.h
 * 摘    要：本文件主要实现数据库的常量定义
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef DBCONST_H
#define DBCONST_H
#include "FaCfg.h"
#include "LibDbConst.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//数据库标准常量定义
#define POINT_NUM     	  97			//集中器专用
#define CCT_POINT_NUM     32            //载波测量点
#define PN_MASK_SIZE  ((POINT_NUM+7)/8)			//测量点屏蔽位数组的大小
//#define PN_MASK_SIZE  ((PN_NUM+7)/8)	//测量点屏蔽位数组的大小
#define PN_NUM     	  	 97		//负控,配变,网络表,电能量采集器使用
#define REAL_PN_NUM	  	 96		//负控,配变,网络表,电能量采集器使用
#define REAL_PN_MASK_SIZE  ((REAL_PN_NUM+7)/8)		//真正需要的测量点屏蔽位数组的大小
#define PN_VALID_NUM	 96			//有效测量点的个数
#define METER_NUM		 96			//普通电表的数量,不包括重点户

#define BANK_NUM		 26

#define MAX_PN_NUM       1       //todo:这里是交采要用到的。
#define MAX_P0_YX	     1	

//集抄测量点数据分BANK存储使用到的BANK号定义
#define CCT_BN_SPM		21			//单相表数据BANK
#define CCT_BN_MFM		21			//多功能表数据BANK

//数据库BANK0的SECT定义
#define SECT_KEEP_PARA			0
#define SECT_TERM_PARA			1
#define SECT_TERM_DATA			2
#define SECT_PN_DATA			3
#define SECT_PN_PARA			4
#define SECT_EXT_TERM_PARA		5

#define SECT_NUM	 			6	//7

#define IMG_NUM     	 		0

#define INVALID_POINT     0

//版本信息的长度定义
#ifdef PRO_698
#define SOFT_VER_LEN	36	//软件版本的字节长度
#else
#define SOFT_VER_LEN	30	//软件版本的字节长度
#endif
#define INN_SOFT_VER_LEN	7	//内部软件版本的字节长度

//测量点动态映射
#define PNMAP_NUM		1		//测量点动态映射方案数量
								 
//测量点动态映射方案,方案号索引从1开始算,0表示不映射
#define MTRPNMAP		PNUNMAP		//1 PNUNMAP/PMMAP1用这个宏定义来切换负控支持的测量点需不需要动态映射
#define PNMAP_CCTMFM	PNUNMAP	//集抄多功能表映射方案,
#define PNMAP_VIP		PNUNMAP	//集抄重点户映射方案,
					 
/////////////////////////////////////////////////////////////////////////////////////////////
//国标版常量定义

//系统库文件定义
#define DB_FILE

//起始测量号的定义
#define MTR_START_PN	1	//电表起始测量号
#define GRP_START_PN	1	//总加组起始测量号
#define TURN_START_PN	1	//轮次起始测量号

//个数定义
#define GRP_NUM			8	//总加组个数
#define TURN_NUM		4	//轮次个数


//数据类的定义
#define GB_DATACLASS1	1	//瞬时数据
#define GB_DATACLASS2	2	//冻结数据
#define GB_DATACLASS3	3	//事件
#define GB_DATACLASS4	4	//参数
#define GB_DATACLASS5	5	//控制
#define GB_DATACLASS6	6	//传输文件
#define GB_MAXDATACLASS			7//	6+1
#define GB_DATACLASS8	8	//请求被级联终端主动上报
#define GB_DATACLASS9	9	//请求终端配置

//数据格式定义
#define FMT_UNK	0	//未知数据格式
#define FMT1	(1)
#define FMT2	(2)
#define FMT3	(3)
#define FMT4	(4)
#define FMT5	(5)
#define FMT6	(6)
#define FMT7	(7)
#define FMT8	(8)
#define FMT9	(9)
#define FMT10	(10)
#define FMT11	(11)
#define FMT12	(12)
#define FMT13	(13)
#define FMT14	(14)
#define FMT15	(15)
#define FMT16	(16)
#define FMT17	(17)
#define FMT18	(18)
#define FMT19	(19)
#define FMT20	(20)
#define FMT21	(21)
#define FMT22	(22)
#define FMT23	(23)
#define FMT24	(24)
#define FMT25	(25)
#define FMT26	(26)
#define FMT27	(27)
#define FMT28	(28)
#define FMT29	(29)
#define FMT30	(30)

#define FMT_NUM		31
#define FMTEX_NUM	1

//非附录的扩展格式，从80开始
#define FMTEX_START 80
#define FMT_BIN		80


#define FMT_BCD	(24*0x100)
#define   FMT_ROUND   (1)
#define   FMT_NROUND  (0)

//不同版本的E,U,I,P,Q,cos的格式定义可能不一样,请使用以下的定义
//这些定义都是针对645ID的,非645ID的格式以协议为准
#define EFMT		FMT14			//电能
#define REFMT		FMT11			//电能
#define UFMT		FMT7			//电压
#define PFMT		FMT9			//有功功率
#define QFMT		FMT9			//有功功率
#define COSFMT		FMT5			//功率因素			
#define DMFMT		FMT23			//需量
#define DMTFMT		FMT17			//需量时间
#define ANGFMT		FMT5			//角度
#define ANGFMT		FMT5
#define VBRKCOUNTSFMT		FMT8	//断相次数
#define VBRKACCUMTFMT		FMT10	//断相累计时间
#define VBRKTIMEFMT			FMT17	//断相起始/结束时刻
#define PROGTIMEFMT			FMT17	//编程时间
#define DMCLEANTIMEFMT		FMT17	//需量清零时间
#define PROGCOUNTSFMT		FMT8	//编程次数
#define DMCLEANCOUNTSFMT	FMT8	//需量清零次数
#define BATTWORKTFMT		FMT10	//电池工作时间

#define ELEN		4
#define RELEN		4
#define ULEN		2
#define PLEN		3
#define QLEN		3
#define COSLEN		2
#define DMLEN		3
#define DMTLEN		5
#define ANGLEN		2

#ifdef PRO_GB2005
	#define IFMT		FMT6			//电流
	#define ILEN		2
#else	//PRO_698
	#define IFMT		FMT25			//国电版不同
	#define ILEN		3
#endif

#define VBRKCOUNTSLEN		3	//断相次数
#define VBRKACCUMTLEN		3	//断相累计时间
#define VBRKTIMELEN			4	//断相起始/结束时刻
#define PROGTIMELEN			4	//编程时间
#define DMCLEANTIMELEN		4	//需量清零时间
#define PROGCOUNTSLEN		2	//编程次数
#define DMCLEANCOUNTSLEN	2	//需量清零次数
#define BATTWORKTLEN		3	//电池工作时间

#define EMPTY_DATA 	 0
#define INVALID_DATA 0xff

#define INVALID_VAL 	(-0x7ffffff0)
#define INVALID_VAL64 	(-0x7ffffffffffffff0LL)

#define INVALID_TIME 	0	//无效的时间


//数据类1-5的细分定义,因4类的最全，采用做通用 
#define CLASS_NULL				0	//备用                          
#define CLASS_P0				1	//PN无定义                      
#define CLASS_METER				2	//测量点                        
#define CLASS_SUMGROUP			3	//总加组                        
#define CLASS_MEASURE			4	//直流模拟量                    
#define CLASS_CONTROLTURN		5	//控制轮次                      
#define CLASS_TASK				8	//任务号                        
                           
#define GB_MAXOFF				1           //注意PN型的从1开始，0空余                                
#define GB_MAXCONTROLTURN		(4+GB_MAXOFF)	//功控轮次                      
#define GB_MAXMETER				PN_NUM	//电表数                        
#define GB_MAXPULSE				PN_NUM	//脉冲点数                      
#define GB_MAXMEASURE			PN_NUM//(4+GB_MAXOFF)	//直流测量点数                  
#define GB_MAXSTATE  			(4+GB_MAXOFF)	//状态量数                      
#define GB_MAXBRANCH			(8+GB_MAXOFF)	//分路数                        
#define GB_MAXTASK				(64+GB_MAXOFF)  //任务数                        
#define GB_MAXSUMGROUP			(8+GB_MAXOFF)   //总加组号	 
#define GB_MAXCOMCHNNOTE		(5+GB_MAXOFF)	//普通中文信息条数
#define GB_MAXIMPCHNNOTE		(5+GB_MAXOFF)	//重要中文信息条数

#define GB_MAXERCODE			31  //事件代码      
#define GB_MAXCOMMTHREAD		4	//通信线程个数 
                                                                    
//单独定义参数部分为极限可能的容量                                  
#define GBC4_MAXMETER			GB_MAXMETER				//电表数            
#define GBC4_MAXSUMGROUP		GB_MAXSUMGROUP	//总加组            
#define GBC4_MAXMEASURE			GB_MAXMEASURE//直流测量点数      
#define GBC4_MAXTASK			GB_MAXTASK		//任务数            
#define GBC4_MAXCONTROLTURN		GB_MAXCONTROLTURN//功控轮次      
#define GBC4_MAXPULSE			GB_MAXPULSE     //脉冲数    
#define GBC4_MTRPORTNUM			4				//32配置F33时的通信端口数

//单独定义参数部分几个变长参数的长度空间定义 
#define GBC4IDLEN_F15			(256)
#define GBC4IDLEN_F27			(256)//(512)
#define GBC4IDLEN_F41			(137)
#define GBC4IDLEN_F65_OLD		(256)//最多只能支持61个信息点
#define GBC4IDLEN_F66_OLD		GBC4IDLEN_F65_OLD
#define GBC4IDLEN_F65			(1029)//最多能支持255个信息点 4*255+9
#define GBC4IDLEN_F66			GBC4IDLEN_F65

#define COM_TASK_IDLEN			(512)	//每个普通任务参数最多19+1+2*1+1+4*255=1043，暂开放512
#define FWD_TASK_IDLEN			(281)	//每个中继任务参数最多26+255=281
#define MAX_COM_TASK			 254
#define MAX_FWD_TASK			 254

#define MAX_MTR_CHANNEL			 32		//有效抄表通道个数
#define MTR_CHANNEL_IDLEN		 7		//有效抄表通道参数长度
#define MTR_PARA_CFGLEN			 712	//抄表通道参数配置表长度

#ifdef PRO_698
#define USR_MAIN_CLASS_NUM		7				//16用户大类数
#define USR_SUB_CLASS_NUM		16				//16用户小类数
												 
//单独定义几个变长参数的长度空间定义 
//#define GBC1IDLEN_F16			(((PN_NUM+7)>>3)+1) 
#define GBC1IDLEN_F169			(130)	//(6*7+1)*3+1

//单独定义几个变长参数的长度空间定义 
#define GBC9IDLEN_F2			(((GBC4_MTRPORTNUM-1)*12)+17)	
#define GBC9IDLEN_F6			((USR_MAIN_CLASS_NUM*(31+1))+2)
#endif

#define ADDONS_NULL		0
#define ADDONS_TIME		1
#define ADDONS_CURVE	2
#define ADDONS_DAYFRZ	3
#define ADDONS_MONFRZ	4
#define ADDONS_COPYDAY	5
#define ADDONS_EC		6

//费率数
#define RATE_NUM		4
#define TOTAL_RATE_NUM  (RATE_NUM+1)	//总+分费率的个数

//谐波次数
#define HARMONIC_NUM		19

//端口设置
#define	PN_PORT_INVALID	0	//无效端口( 针对脉冲无效设置，设置为PN_PROP_INVALID 时，此次测量点属性修改操作无效)

//测量点属性

#define PN_PROP_METER	1	//电表
#define PN_PROP_DC		2	//直流模拟量
#define PN_PROP_PULSE	3	//脉冲
#define PN_PROP_CALCU	4	//计算值
#define	PN_PROP_AC		5	//交采
#define PN_PROP_CCT		6	//集抄测量点(包括各种链路,以后有需要可以扩展为PN_PROP_PLC,PN_PROP_CCT485等,不过为了避免扩展太多,还是推荐使用PN_PROP_CCT)
#define PN_PROP_EXTAC   7	//外接交采装置
#define PN_PROP_UNSUP	0xff	//暂时不支持的测量点类型

//相对于测量点属性的另外一种测量点类型定义,每种类型占一位
#define PN_TYPE_P0		0x01	//测量点0
#define PN_TYPE_AC		0x02	//交采
#define PN_TYPE_MTR		0x04	//电表
#define PN_TYPE_PULSE	0x08	//脉冲
#define PN_TYPE_DC		0x10	//直流模拟量
#define PN_TYPE_GRP		0x20	//总加组

#define PN_TYPE_MSR		(PN_TYPE_AC|PN_TYPE_MTR|PN_TYPE_PULSE) //测量点

//////////////////////////////////////////////////////////////////////////////////////
//GB2005和698相对于系统库数据项的不同定义

#define FMT22TOCUR_SCALE	100

#define F25_LEN				11
#define F25_CONN_OFFSET		10

#define F26_VOLUPPER_OFFSET	6
#define F26_VOLLOWER_OFFSET 11
#define F26_CURUPPER_OFFSET	16
#define F26_CURUP_OFFSET	22	
#define F26_ZCURUP_OFFSET	28	
#define F26_SUPER_OFFSET	34
#define F26_SUP_OFFSET		40
#define F26_VUNB_OFFSET		46
#define F26_IUNB_OFFSET		51

#define MTR_PARA_LEN		27		//8902
#define MTR_PORT_OFFSET		4		//8902中通信速率和端口偏移
#define MTR_BAUD_OFFSET		4		//8902中通信速率和端口偏移
#define MTR_PRO_OFFSET		5		//8902中规约类型偏移
#define MTR_ADDR_OFFSET		6		//8902中电表地址偏移
#define MTR_PSW_OFFSET		12		//8902中电表密码偏移
#define MTR_RATE_NUM_OFFSET 18		//8902中费率个数偏移
#define MTR_DOT_NUM_OFFSET 19		//8902中小数点个数偏移
#define MTR_USR_TYPE_OFFSET	26		//8902中大小类号偏移


#define MTRPRO_TO_IFMT_SCALE 1		//电表协议库的电流格式到主站通信协议格式的量程转换

#define NO_CUR				50		//无电流的固定阀值
#define STD_UN				2200	//标准额定电压
#define STD_IN				5000	//标准额定电流
									 
#define F10_LEN_PER_PN		27		//F10中每个测量点参数的长度
#define F10_MTRNUM_LEN		2		//F10中本次电能表/交流采样装置配置数量n的长度
#define F10_SN_LEN			2		//F10中电能表/交流采样装置序号的长度
#define F10_PN_LEN			2		//F10中所属测量点号的长度							

#define C1_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM) 	//(大类号+小类号组数+(用户小类号+信息类组数n+31)*USR_SUB_CLASS_NUM)
#define C2_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM)	//(大类号+小类号组数+(用户小类号+信息类组数n+31)*USR_SUB_CLASS_NUM)

#define RATE_SIZE	24		//费率参数的长度		//注意，该值待确定。。。//140809





#define ARDFMT_NUM	22
//告警格式
#define ARD			0
#define	ARD1		1
#define	ARD2		2
#define	ARD3		3
#define	ARD4		4
#define	ARD5		5
#define	ARD6		6
#define	ARD7		7
#define	ARD8		8
#define	ARD9		9
#define	ARD10		10
#define	ARD11		11
#define	ARD12		12
#define	ARD13		13
#define	ARD14		14
#define	ARD15		15
#define	ARD16		16
#define	ARD17		17
#define	ARD18		18
#define	ARD19		19
#define	ARD20		20
#define	ARD21		21
#define	ARD22		22
#define ALR_NUM		25

#endif //DBCONST_H

