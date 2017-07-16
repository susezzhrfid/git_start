/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LibCctConst.h
 * 摘    要：本文件主要实现集抄基本常量定义及配置常量定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年1月
 *********************************************************************************************************/
#ifndef LIBCCTCONST_H
#define LIBCCTCONST_H

#define PLC_MODULE_RT	1	//科陆自动路由算法(福星晓程)
#define PLC_MODULE_ES	2	//东软

//载波相位信息
#define PLC_PHASE_UNK		0	
#define PLC_PHASE_A			1
#define PLC_PHASE_B			2
#define PLC_PHASE_C			3

//帧格式中的字段偏移
#define FRM_HEAD			        0
#define FRM_LEN				        1
#define FRM_C			            3  
#define FRM_INF		        		4 	  //信息域R

#define FRM_ADDR					10 
#define FRM_AFN			        	10    //不带地址的AFN偏移
#define FRM_AFN_A			        22 	  //带地址的AFN偏移
#define FRM_TRAN_645ADDR		    28 	  //转发数据645帧的地址偏移


//工作模式定义
#define WM_RDMTR	0	//抄表
#define WM_STUDY	1	//学习
//AFN
#define AFN_CON		            	0x00	//确认∕否认
#define AFN_INIT			        0x01	//初始化
#define AFN_TRAN	    	    	0x02	//数据转发
#define AFN_QRYDATA			    	0x03	//查询数据
#define FRM_LNKTEST			    	0x04	//链路接口检测
#define AFN_CTRL			    	0x05	//控制命令
#define AFN_REP			    		0x06	//主动上报
#define AFN_QRYRT					0x10	//路由查询
#define AFN_SETRT		    		0x11	//路由设置
#define AFN_CTRLRT					0x12	//路由控制
#define AFN_RTFWD 		    		0x13	//路由数据转发
#define AFN_RTRD 		    		0x14	//路由数据抄读
#define AFN_TRSFILE   				0x15    //文件传输


#define AFN_FCNETOP					0x09	//网络操作
#define AFN_FCROUTE					0x08	//友讯达组网操作类
#define AFN_FCNETINFO				0x07	//友讯达网络信息类
#define AFN_RPT						0x0a	//友讯达主动上报

//FN定义
#define F1		1
#define F2		2
#define F3		3
#define F4		4
#define F5		5
#define F6		6
#define F7		7
#define F8		8
#define F9		9
#define F10		10
#define F11		11

//路由请求抄读内容的抄读标志
#define RD_FLG_FAIL		0 	//抄读失败
#define RD_FLG_SUCC 	1	//抄读成功
#define RD_FLG_TORD		2	//可以抄读


#define AFN_DEBUG					0xcc    //友讯达调试信息输出的AFN
//中继深度
#define PLC_FWD_DEPTH_0		0	//中继深度0
#define FWD_DEPTH_MAX		7	//最大支持的中继深度

//载波集中器的底层通信链路定义
#define AR_LNK_485  	  0x00		  //485抄表
#define AR_LNK_ES		  0x02		  //东软载波(模块路由)
#define AR_LNK_TC		  0x04		  //青岛鼎信(模块路由)
#define AR_LNK_STD_TC	  0x05		  //青岛鼎信698标准接口
#define AR_LNK_STD_XC	  0x06		  //福星晓程698标准接口
#define AR_LNK_STD_RC	  0x07		  //瑞斯康698标准接口
#define AR_LNK_STD_ES	  0x08		  //东软698标准接口
#define AR_LNK_RF		  0x09		  //无线方案
#define AR_LNK_ES_EX	  0x0a		  //东软载波(扩充协议)
#define AR_LNK_STD_FC	  0x0b		  //友讯达无线标准接口
#define AR_LNK_STD_MI	  0x0c		  //弥亚微
#define AR_LNK_STD_SR	  0x0d		  //桑锐
#define AR_LNK_STD_LM	  0x0e		  //力合微
#define AR_LNK_STD_GD     0x0f        //国电御辉  
#define AR_LNK_STD_XC_A   0x10        //福星晓程自组网 
#define AR_LNK_STD_LK	  0x11		  //河北电科院版本
#define AR_LNK_STD_LM_N	  0x12		  //力合微新,为兼容老的力合微模块,新加这个吧.
#define AR_LNK_STD_RZ     0x13        //北京新鸿基
#define AR_LNK_STD_ZC     0x14        //北京中宸鸿昌
#define AR_LNK_STD_NT     0x15        //中睿昊天
#define AR_LNK_STD_HR     0x16        //杭州宏睿
#define AR_LNK_STD_MX     0x17       //江苏麦希小无线
#define AR_LNK_STD_NT_HB  0x18       //河北中睿昊天

#define AR_LNK_FXR	0x81		//福星晓程(科陆自动路由算法)
#define AR_LNK_ESR	0x82		//东软载波(科陆自动路由算法)
#define AR_LNK_ATR	0x83		//安泰采集器方案(科陆自动路由算法)
#define AR_LNK_TCR	0x84		//青岛鼎信(科陆自动路由算法)

#define AR_LNK_NUM	AR_LNK_TCR	//自动抄表系统底层通信链路种类数

#define RT_INFO_LEN			28
#define RT_SUBST_LEN		12

#define	RD_DONE			0x01	//抄完
#define RD_UNDONE		0x00	//未抄完

#define	LenStateW		0x10	//状态字节长度

//目前集抄支持的电表协议,定义跟698.42保持一致
#define	CCT_MTRPRO_97	1	//97版645
#define	CCT_MTRPRO_07	2	//07版645
#define	CCT_MTRPRO_NJSL	3	//南京松林版645

//集抄广播控制字定义,保持跟698.42兼容,扩展0x10为广播校时
#define	BC_CTL_TRANS	0x00	//透明传输
#define	BC_CTL_97		0x01	//DLT/645-1997 
#define	BC_CTL_07		0x02 	//DLT/645-2007
#define	BC_CTL_PHASE	0x03    //相位识别功能; 04H FFH保留。
#define	BC_CTL_ADJTIME	0x10	//广播校时
#define	BC_CTL_FRZ		0x11	//广播冻结

//集抄抄读错误定义
#define	CCT_RD_ERR_OK		0		//无错误
#define	CCT_RD_ERR_ITEM		1		//数据项的内容错误,比如日冻结时标错误等
#define	CCT_RD_ERR_RX		2		//接收失败
#define	CCT_RD_ERR_ADDR		3		//接收地址没找到
#define	CCT_RD_ERR_REPLY 	4		//异常应答帧,错误信息字放在数据缓冲的第一个字节
#define	CCT_RD_ERR_UNSUP 	5		//不支持数据项

#define CCT_SPD_NUM         5       //376.2-2013支持的载波通信速率数量
#endif


