/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TaskConst.h
 * 摘    要：本文件主要用来定义任务常量
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2009年9月
 *********************************************************************************************************/
#ifndef TASKCONST_H
#define TASKCONST_H


#define SURVEY_INTERV 		15

#define COMTASK_REC_HEAD_LEN	7

//普通任务的测量点类型配置
#define TASK_PN_TYPE_P0		0x01
#define TASK_PN_TYPE_AC		0x02
#define TASK_PN_TYPE_MTR	0x04
#define TASK_PN_TYPE_PULSE	0x08
#define TASK_PN_TYPE_DC		0x10
#define TASK_PN_TYPE_GRP	0x20

//普通任务的冻结类型配置
#define TASK_FRZ_PROP_MTR	0x01	//电表自身冻结
#define TASK_FRZ_PROP_TEM	0x02	//终端自身冻结

#define TASK_PN_TYPE_MSR	(TASK_PN_TYPE_AC|TASK_PN_TYPE_MTR|TASK_PN_TYPE_PULSE) //测量点

#define REC_TIME_LEN   5

#define CCT_COMTASK_NUM_MAX		64		//集抄最大支持任务数
#define CCT_COMTASK_NUM			32		//集抄任务个数

//集抄任务ID定义
#define CCT_TASK_VIP		0		//重点户冻结
#define CCT_TASK_DAYFRZ		4		//集抄标准日冻结任务号

#define PNCHAR_MAX			16		//最大支持的测量点特征字个数
#define PNCHAR_ALL			0		//所有测量点特征字,放在测量点特征字个数字节,用来表示这种特殊情况
#define PNCHAR_VIP			0xff	//重点户测量点特征字,放在测量点特征字个数字节,用来表示这种特殊情况
			
//补抄宏定义
#define PN_TIMEOUT	-2
#define PN_FAIL		-1
#define PN_BUSY		0
#define PN_SUCCESS	1
#endif //TASKCONST_H

