/*******************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FaConst.h
 * 摘    要：本文件主要定义系统应用中使用到的一些常量
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 ******************************************************************************/
#ifndef FACONST_H
#define FACONST_H

#define  PWROFF_VER       0x01

#define ERR_APP_OK       0x0
#define ERR_FORWARD  0x1
#define ERR_INVALID  0x2    //设置内容非法
#define ERR_PERM     0x3
#define ERR_ITEM     0x4	//数据库不支持的数据项
#define ERR_TIME     0x5    //时间失效

#define ERR_UNSUP	 0x06	//电表不支持的数据项等
#define ERR_FAIL	 0x07	//尝试失败,比如抄表失败等

#define ERR_ADDR     0x11
#define ERR_SEND     0x12
#define ERR_SMS      0x13 
#define ERR_PNUNMAP	 0x14	//测量点未映射
#define ERR_DEV	 	 0x15	//设备有问题
#define ERR_NOTEXIST 0x16	//不存在

#define ERR_APP_TIMEOUT  0x20
#define ERR_SYS      0x30

#define ERR_CALIB_OVER   0x40  //校准完成
#define ERR_PARA         0x41  //参数不正确
#define ERR_OUT_CALIB    0x42  //不在校准状态

//应用通知:
//把在延时通知中,需要2分钟才提交的通知消息放到前面,
//其它1分钟可以提交较快的ID放到INFO_LONG_DELAY_END规定的号码后
#define INFO_NONE	   			0
#define INFO_APP_RST			1
#define INFO_SYS_RST   			2
#define INFO_WK_PARA			3	//GPRS工作线程参数更改
#define INFO_FAP_PARA			4	//主站通信协议参数更改
#define INFO_COMM_ADDR			5	//终端地址、区县码更改
#define INFO_COMM_RLD			6	//与主站通信需要重装参数
//短延时
#define INFO_230M_PARA			7	//电台参数更改
#define INFO_EXC_PARA			8	//异常任务参数变更
//无延时
#define INFO_ACTIVE	   			9	//短信或者振铃激活信息
#define INFO_METER_PARA			10   //电表参数更改
#define INFO_RST_PARA			11  //参数复位
#define INFO_RST_DATA			12	//数据复位
#define	INFO_CLR_DEMAND			13	//清需量
#define INFO_AC_PARA			14	//交采参数变更
#define INFO_TASK_PARA			15	//任务参数变更
#define INFO_TASK_ENABLE		16	//任务使能标志变更
#define INFO_YX_PARA			17	//遥信参数变化
#define INFO_STAT_PARA 	   		18	//数据统计参数变更
#define INFO_PULSE	   			19	//脉冲参数更改

#define INFO_CTRL				20	//控制信息电控
#define INFO_TEMPCTRL			21	//临时限电控
#define INFO_WTCURVE			22	//写功控曲线
#define INFO_POINTPARA_CTRL		23	//写测量点参数
#define INFO_REMOTEDOWN			24	//与主站通信需要重装参数

#define INFO_GPRS_OFFLINE		25	//要求GPRS下线

#define INFO_DC_SAMPLE			26


#define INFO_PLC_MTRSCHCMD		27		//Plc搜索抄表命令
#define INFO_PLC_STATUPDATA		28		//Plc统计更新
#define INFO_PLC_RDALLRTINFO 	29  //读取所有节点的中继路由信息
#define INFO_PLC_PARA	   		30   //载波参数更改
#define INFO_PLC_CLRRT	   		31	//清路由
#define INFO_PLC_STOPRD			32	 //停止抄表
#define INFO_PLC_RESUMERD		33	 //恢复抄表	
#define INFO_PLC_UPDATE_ROUTE   34  //载波路由器升级
#define INFO_PLC_WIRLESS_CHANGE 35  //无线信道变更
#define INFO_PLC_RADIO_PARA     36  //无线参数变更
#define INFO_PLC_TIME_SET		37  //集中器时间被更改
#define INFO_PLC_MOD_CHANGED    38  //更换载波模块

#define INFO_START_REREAD       39
#define INFO_PRO_REGINFO 		40  //请求下发注册表信

#define INFO_FRZ_TASK_PARA      41  //冻结任务参数变更    Jason add. 20130719
#define INFO_STAT_TERM			42	//终端统计数据
#define INFO_CLR_STAT_TERM		43	//终端统计数据

#define INFO_RST_ALLPARA		44   //清楚所有参数数据

#define INFO_AC_REPOWERON       45  //交采重新上电	//20140517-1

#define INFO_STOP_FEED_WDG		46	//停止喂看门狗
#define INFO_DISCONNECT			47		//DISCONNECT
#define INFO_RESTART_RDMTR		48	//指定端口重新抄表
#define INFO_START_485I_MTRSCH	49  //启动4851搜表命令
#define INFO_STOP_485I_MTRSCH	50  //停止4851搜表命令
#define INFO_START_485II_MTRSCH	51  //启动4852搜表命令
#define INFO_STOP_485II_MTRSCH	52  //停止4852搜表命令
#define INFO_START_485III_MTRSCH	53  //启动4853搜表命令
#define INFO_STOP_485III_MTRSCH	54  //停止4853搜表命令
#define INFO_UPDATE_BOOTLOADER  55  //Bootloader更新
#define INFO_MAC_RST            56  //以太网物理层复位
#define INFO_PORT_FUN            57  //端口功能改变消息
#define INFO_RST_TERM_STAT		58	//复位终端统计数据
#define INFO_FWDTASK_PARA       59  //中继任务参数
#define INFO_COMM_RST			60	//与主站通信需要复位
#define INFO_TASK_INIT			61  //任务参数初始化
#define INFO_END	   			62	//空消息,作为所有消息的结束
									//把本通知恒定作为最后一个

#define INFO_NUM	   	    		(INFO_END+1)
#define INFO_SHORT_DELAY_START	 	INFO_230M_PARA
#define INFO_NO_DELAY_START	 		INFO_ACTIVE

//消息长延时与短延时的定义,单位秒
#define INFO_SHORT_DELAY	6
#define INFO_LONG_DELAY		30


//电表协议内部定义
#define PROTOCOLNO_NULL			2
#define PROTOCOLNO_DLT645		0	//DL645
//#define PROTOCOLNO_AC			2	//交采装置
#define PROTOCOLNO_ABB			3	//ABB方表
#define PROTOCOLNO_FUJ			4	//福建扩展
#define PROTOCOLNO_EDMI			5	//红相
//#define PROTOCOLNO_ABB2			6	//ABB圆表	
#define PROTOCOLNO_WS			7	//威盛I型
#define PROTOCOLNO_HND			8	//浩宁达
#define PROTOCOLNO_LANDIS  		9	//兰吉尔1107
#define PROTOCOLNO_OSTAR 		10	//蜀达
//#define PROTOCOLNO_DLMS  		11	//爱拓利	
//#define PROTOCOLNO_HL645  		12	//华隆老表	
//#define PROTOCOLNO_AH645		13	//安徽645	
#define PROTOCOLNO_1107			14	//A1700
#define PROTOCOLNO_HT3A			15	//恒通
#define PROTOCOLNO_LANDIS_DLMS	16  //兰吉尔DLMS
//#define PROTOCOLNO_TJ645		17	//天津645
#define PROTOCOLNO_HB645		18	//湖北645
#define PROTOCOLNO_EMAIL		19	//EMAIL表	
#define PROTOCOLNO_MODBUS		20	//德国GMC A2000表(MODBUS协议)
//#define PROTOCOLNO_LANDIS_ZMC	22	//兰吉尔ZMC表协议
//#define PROTOCOLNO_BJT645		23  //北京97版645
#define PROTOCOLNO_DL645_Q		24  //97版645的无功表
#define PROTOCOLNO_DLT645_SID	28	//97版645的单ID抄读
//#define PROTOCOLNO_NMG645		29  //内蒙古表
#define PROTOCOLNO_DLT645_V07	1	//2007版645表
//#define PROTOCOLNO_TEMPERATURE	35	//多通道温度测试仪
//#define	PROTOCOLNO_DTM645		36  //湖北防窃电数据转换模块

//#ifdef VER_CL195N4
#define PROTOCOLNO_DLT645_11	11	//DL645
#define PROTOCOLNO_DLT645_12	12	//DL645
#define PROTOCOLNO_DLMS  		25	//爱拓利	
#define PROTOCOLNO_HL645  		26	//华隆老表	
//#endif

#define	CCT_MTRPRO_97	1	//97版645
#define	CCT_MTRPRO_07	2	//07版645

#define PROTOCOLNO_MAXNO		40	//最大的电表协议号，目前不超过40

#define FLG_FORMAT_DISK   		0x34aebd24
#define FLG_DEFAULT_CFG   		0x8a5bc4be
#define FLG_REMOTE_DOWN   		0xbe7245cd
#define FLG_HARD_RST   	  		0x4ab8ce90
#define FLG_DISABLE_METEREXC    0xce7821bd
#define FLG_ENERGY_CLR    		0xee6ad23f
#define FLG_APP_RST             1

#define TYPE_FRZ_TASK		    0		//冻结任务数据
#define TYPE_COMM_TASK			1		//普通任务数据
#define TYPE_FWD_TASK			2		//中继任务数据
#define TYPE_ALR_EVENT		    3		//告警事件

//485口功能
#define PORT_FUN_RDMTR		0		//抄表口
#define PORT_FUN_INMTR		1		//被抄口
#define PORT_FUN_LINK		2		//级联口
#define PORT_FUN_VARCPS		3		//接无功补偿装置
#define PORT_FUN_ACQ		4		//采集口
#define PORT_FUN_DEBUG		0xFF	//debug输出(只对3口有效)

		//注意:本数组的定义就不要再改了,否则引起混乱,
		// 	   我们就固定认为逻辑端口根据老的习惯,是从右->左的顺序开始编号1,2...
#define LOGIC_PORT_NUM	2		//(sizeof(g_iInSnToPhyPort)/sizeof(int))

#define LOGIC_PORT_MIN	0									//最小的逻辑端口定义
#define LOGIC_PORT_MAX	(LOGIC_PORT_MIN+LOGIC_PORT_NUM-1)	//最大的逻辑端口定义
#define INMTR_BAUD			CBR_2400

#define TRANSMIT_TIME_OUT   50      //文件传输中的转发超时时间 单位s

#define COM_FUNC_METER  0 //485串口功能定义
#define COM_FUNC_LINK   1
#define COM_FUNC_DL645  2
#define COM_FUNC_UPLOAD  3

#define COM_FUNC_ACQ		4		//采集口
#define COM_FUNC_JC485		5		//集抄485口.

#define SG_FAP_DATA_EX   6			//自定义命令报文文件名偏移 (2字节科陆识别码 + 1扩展控制码 + 3个字节自定义密码)

#endif

