/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrExcTask.h
 * 摘    要：实现告警的判断，记录，查询
 *
 * 版    本: 1.0 1
 * 作    者：陈曦
 * 完成日期：2008-06-
 *
 * 取代版本：1.0 0
 * 原 作 者:
 * 完成日期：
 * 备    注:
 
************************************************************************************************************/

#ifndef EXCTASK_H
#define EXCTASK_H

//#include "FaStruct.h"
//#include "TaskDB.h"
#include "MtrExc.h"
#include "LibDbStruct.h"


//各告警事件定义
//========事件名称========事件ID=================测量点类型===========
#define ERC_INIT_VER        1					//数据初始化和版本变更记录
#define ERC_PARACHG			3                   //变长事件
#define ERC_YXCHG			4					//状态变位记录
#define ERC_YK             	5					//遥控跳闸记录
#define ERC_PWRCTL          6					//功控控跳闸记录
#define ERC_ENGCTL          7					//电控控跳闸记录
#define ERC_MTRPARA         8                   //电表参数变更
#define ERC_IEXC        	9					//电流回路异常
#define ERC_VEXC           	10					//电压回路异常
#define ERC_DISORDER    	11					//相序异常
#define ERC_MTRTIME         12					//电能表时间超差
#define ERC_MTRERR        	13					//电表故障信息
#define ERC_PWRONOFF        14					//终端停/上电事件
#define ERC_HARMONIC        15					//谐波越限
#define ERC_DC        		16					//直流模拟量事件#
#define ERC_UNBALANCE    	17					//电压/电流不平衡度越限记录
#define ERC_BUYPARA         19					//购电参数设置记录
#define ERC_AUTH           	20					//消息认证错误
#define ERC_TERMERR			21					//终端故障记录
#define ERC_ENGDIFF         22					//差动组事件#
#define ERC_ENGALARM        23					//电控告警记录
#define ERC_VOLTEX          24					//电压越限记录
#define ERC_IEX          	25					//电流越限记录
#define ERC_SEX          	26					//视在功率越限记录
#define ERC_ENGDEC       	27					//电表事件…
#define ERC_ENGOVER       	28					//点能量超差
#define ERC_MTRFLEW         29					//电表事件…
#define ERC_MTRSTOP         30					//电表事件…
#ifdef PRO_698
#define ERC_MTRRDFAIL		31					//抄表失败
#define ERC_FLUXOVER		32					//终端与主站通信流量超门限事件记录
#define ERC_MTRSTATUSCHG	33					//电表状态字变位事件
#endif
#define ERC_CTEXC			34					//CT异常

#define ERC_MTRTAILLID		37					//电能表开表盖事件记录
#define ERC_MTRBUTTONLID	38					//电能表开端钮盒事件记录
#define ERC_MTRREFAIL		39					//补抄失败事件记录
#define ERC_FIELD_ERR		40					//磁场异常事件
#define ERC_SETTIME			41					//对时事件记录
//以下为电能表扩展事件
#define ERC_VMISS			40				//失压
//#define ERC_VLACK			41				//欠压
#define ERC_VDISORD			42				//电压逆相序
#define ERC_IDISORD			43				//电流逆相序
#define ERC_VEXCEED			44				//过压
#define ERC_VBREAK			45				//断相

#define ERC_IMISS			46				//失流
#define ERC_IEXCEED			47				//过流
#define ERC_VUNBALANCE		48				//电压不平衡
#define ERC_IUNBALANCE		49				//电流不平衡
#define ERC_FLOW_REVERSE	50				//潮流反向
#define ERC_OVER_LOAD		51				//过载
#define ERC_TEST_CONNECT	60				//通信测试请求



// #define CONNTYPE_1P      3
// #define CONNTYPE_3P3W    1   //电表接线方式-三相三线
// #define CONNTYPE_3P4W    2	 //三相四线
// #define EXC_REC_NUM     5   //读取某类型告警事件的最大记录条数
// #define EXC_REC_LENTH   50  //单条告警事件记录的最大长度；(除差动组越限配置的总加组的测量点个数>3（一般不会超过）此定义长度足够用)


bool CreatExcTaskTab(char* pszName);
BYTE CopyAlrData(BYTE *bSaveBuf, BYTE *pbBuf, BYTE bLen);
extern bool SaveAlrData(DWORD dwAlrID, WORD wPn, BYTE* pbBuf, DWORD dwLen);
extern bool HandleAlr(DWORD dwAlrID, WORD wPn, TBankItem* pBankItem, WORD wIdNum, DWORD dwCurMin, TTime tmNow, BYTE* bPreData, WORD wDataLen);
//extern BYTE ReadClass3Item(BYTE bAlrID, BYTE bRecNum, BYTE* pbBuf, BYTE& bLen);
extern BYTE ReadAlrItem(BYTE bAlrType, BYTE bAlrID, BYTE bRecNum, BYTE* pbBuf, BYTE* pbLen);
BYTE SortArray(DWORD dwData[], BYTE bDataNum, BYTE bRecNum, BYTE* bIndex);



//
//*******************************基类事件***************************************
//

//#define EXC_TASK_NUM	51		//34	//告警事件总类型数目
//BYTE m_bPnProp[PN_NUM+1];
//bool IsAlrAttriChg(BYTE bERC, BYTE& bAlrType);
//bool IsPnPropChg(BYTE bPn, BYTE& rbPnProp);

//extern BYTE m_bAlrType[EXC_TASK_NUM+1];

extern bool InitExcTask();	//初始化告警任务
extern bool ExcTaskInit();	
extern bool DoExcTask(BYTE bPn);	//执行测量点Pn的告警任务

//终端告警
//终端告警任务在终端发生事件的时候执行,不在读测量点数据的时候进行

#endif 

