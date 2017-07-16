 /*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ExcTask.cpp
 * 摘    要：告警事件的基类；告警事件存储的接口函数；告警事件对显示的接口函数
 *
 * 版    本: 1.0 1
 * 作    者：陈曦
 * 完成日期：2008-04-25
 *
 * 取代版本：1.0 0
 * 原 作 者:
 * 完成日期：
 * 备    注: 告警事件分为两类：###不需要提交抄表ID,比如交采，直流模拟量，谐波，差动组的事件，在CUnMtrExcTask类实现；
 *						       ###需要提交抄表ID的事件，在类CMtrExcTask中实现；
 *
 * 在类CMtrExcTask中，分为两类：###越限事件，通过比较当前周期的电表参数与设定的限值，从而确定是异常事件；
 *							    ###另外一类则需通过比较上一周期和此周期的电表参数，在类CUnExdExcTask中实现。
************************************************************************************************************/
#include "ExcTask.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "sysarch.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "FlashMgr.h"
#include "SysDebug.h"

#ifdef PRO_698
static const BYTE bAlrLen[] = {9, 1, 0, 2, 5, 9, 11, 2, 22, 22, //1~10
								18, 1, 2, 5, 43, 4, 21, 0, 26, 17,//11~20
								1, 0, 11, 8, 11, 8, 11, 12, 12, 7,//21~30
								16, 8, 29, 2, 0, 0, 64, 64, 16, 10,//31~40
								14, 0, 0, 0, 0, 0, 0, 0, 0, 0,//41~50
								0, 0, 0, 0, 0, 0, 0, 0, 0, 4};//51~60
#else
static const BYTE bAlrLen[] = {9, 0, 0, 2, 5, 9, 11, 2, 19, 19, //1~10
                   						18, 1, 2, 5, 43, 4, 18, 0, 26, 17,//11~20
								1, 0, 0, 8, 8, 8, 11, 12, 12, 7};//21~30
#endif

#define ALR_LEN_NUM	(sizeof(bAlrLen)/sizeof(BYTE))

/*
//描述：返回此周期事件的属性及投入是否变动；
//返回：若异常任务从无效变成有效,则返回true;
//		若异常任务无变动或者重要事件和一般事件过渡，则返回false;
bool IsAlrAttriChg(BYTE bERC, BYTE& bAlrType)
{
	bAlrType = GetErcType(bERC);
	if (m_bAlrType[bERC] != bAlrType)
	{
		if (m_bAlrType[bERC]>0 && bAlrType>0) //在重要事件和一般事件过渡，认为无变化；
		{
			m_bAlrType[bERC] = bAlrType;
			return false;
		}
		
		m_bAlrType[bERC] = bAlrType;	//从无效变成有效或从有效变成无效
		return true;	
	}
	else	//有效性没发生变化
	{
		return false;
	}
}
*/
/*
//描述：返回此周期测量点的属性及属性是否有变动
bool IsPnPropChg(BYTE bPn, BYTE& rbPnProp)
{
	//返回:0表示无效；1表示交采；2表示电表，3表示脉冲，4表示模拟量
	rbPnProp = GetPnProp(bPn);

	//DTRACE(DB_METER_EXC, ("IsPnPropChg::bPn=%d oldbPnProp =%d newbPnProp=%d.\r\n", bPn, m_bPnProp[bPn], rbPnProp));

	if (m_bPnProp[bPn] != rbPnProp)		
		return true;	
	else	
		return false;	
}
*/

//初始化告警任务
//在终端启动调用此函数
bool InitExcTask()
{
	return true;
}

/*
//执行测量点Pn的所有告警任务
bool DoExcTask(BYTE bPn)
{
	for (BYTE bTask=0; bTask<EXC_TASK_NUM; bTask++)
	{
		if ((bTask>=1 && bTask <=4) || bTask==14 || bTask==21 || bTask==31 || bTask==32)	//终端告警任务
			DoTermnTxc(bPn, bTask);
		else if ((bTask>=8 && bTask<=13) || bTask==17 || bTask==33)
			DoMtrExc(bPn, bTask);
	}
}
*/


const BYTE g_bChgAlrID[]={8,9,10,11,12,13,15,17,18,24,25,26,27,28,29,30,33,34,37,38,0};
//描述：拷贝告警数据到告警记录的数据段,
// 		pbBuf中的数据是按照05规约填写的,所以对于05规约来说不用调整
// 		对于698协议,要将05规约的电表事件转换成698规约的电表事件,测量点字节数变长一个字节
//参数：@bSaveBuf - 输出数据;
//      @pbBuf -输入数据；
//返回：无。
//备注：测量点字节数变长一个字节
BYTE CopyAlrData(BYTE *bSaveBuf, BYTE *pbBuf, BYTE bLen)
{
	//告警数据转换为698格式
#ifdef PRO_698
	BYTE bNum = 0;
	bool bFind = false;
	while (g_bChgAlrID[bNum])
	{
		if (bSaveBuf[0] == g_bChgAlrID[bNum])
		{
			bFind = true;
			break;
		}

		if(bSaveBuf[0] < g_bChgAlrID[bNum])
			break;
		bNum++;
	}

	if(bFind) 
	{
		bSaveBuf[1] = bSaveBuf[1]+1;	//长度增加一个字节
		bSaveBuf[7]= (pbBuf[0]&0x7F);	//测量点号
		pbBuf[0]= (pbBuf[0]&0x80);		//起/止标志
		memcpy(&bSaveBuf[8], pbBuf, bLen);
		return bLen+8;
	}
	else
	{
		memcpy(&bSaveBuf[7], pbBuf, bLen); 
	}
#else
	memcpy(&bSaveBuf[7], pbBuf, bLen); //05规约直接拷贝
#endif
	return bLen+7;
}

//描述：告警事件保存。
//参数：@bAlrID - 事件ID;
//      @time - 告警事件发生的时间；
//      @pbBuf - 告警事件内容；
//      @bLen - 定长事件默认为0；变长事件为事件内容长度（告警时间后的数据长度,目前只有Erc3、Erc22为变长事件）
//		@bErcType - 同GetErcType（）的功能，主要是在参数初始化事件用到,其他事件目前都不用
bool SaveAlrData(DWORD dwAlrID, WORD wPn, BYTE* pbBuf, DWORD dwLen)
{
	if (IsDbLocked())
		return false;

	if ((dwLen+6)>ALRSECT_SIZE)
		return false;

	WordToByte(wPn, pbBuf+1);
	DWordToByte(dwAlrID, pbBuf+3);
	dwLen += 6;

	//向任务数据库存储异常事件数据；
	if(dwAlrID&0x00010000)
    {
		WriteAlrRec(FADDR_EVTREC, pbBuf, dwLen);//存事件
    }
	else
    {
		WriteAlrRec(FADDR_ALRREC, pbBuf, dwLen);//存告警
       	SetLedCtrlMode(LED_ALARM, LED_MODE_TOGGLE);   //告警灯
    }

	return true;
}

//存储告警数据的接口函数
//                   告警ID        测量点   发生时要抄读的数据项   数据项个数    数据时间    告警发生时间  告警发生前数据内容  发生前内容长度
bool HandleAlr(DWORD dwAlrID, WORD wPn, TBankItem* pBankItem, WORD wIdNum, DWORD dwCurMin, TTime tmNow, BYTE* bPreData, WORD wDataLen)
{
	BYTE bRecBuf[ALRSECT_SIZE];
	BYTE *pbRec = bRecBuf+7;	//1个字节的CRC + 2个字节的Pn + 4个字节的ID
	WORD wTotalLen = 0;//此参数在新南网协议中，没有意义了
	int nRead = 0;
	WORD i;

	memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));

	//告警状态
	*pbRec++ = 1;
	//告警发生时间
	pbRec += TimeTo6Bcd(&tmNow,pbRec);

	wTotalLen += 7;//长度加上上面的7个字节

	if(bPreData!=NULL && wDataLen>0)
	{
		memcpy(pbRec, bPreData, wDataLen);
		wTotalLen += wDataLen;
		pbRec += wDataLen;
	}
	for (i=0; i<3; i++)
	{
		if (dwAlrID == 0xE200003D)
		{
			memcpy(pbRec, g_bRdMtrAlrStatus, sizeof(g_bRdMtrAlrStatus));
			memset(pbRec+sizeof(g_bRdMtrAlrStatus), 0, 256-sizeof(g_bRdMtrAlrStatus));
			wTotalLen += 256;
			break;
		}
		nRead = ReadItemMbi(pBankItem, wIdNum, pbRec, dwCurMin*60, INVALID_TIME);
		if (nRead >= 0)
			break;
	}
	if(nRead<0)
		nRead = 0;
	wTotalLen += nRead;

	return SaveAlrData(dwAlrID, wPn, bRecBuf, wTotalLen);
}

/*
//描述：从异常事件任务表中读取某个告警事件的最近n条记录。
//参数：@bAlrType - 告警事件类型
//      @bAlrID - 告警事件ID。
//      @bRecNum - 取告警事件的条数。
//返回：如果读到，则返回告警事件内容至pbBuf中,且返回读取的记录的条数；反之，则返回0。
//备注：告警事件单条记录格式：告警事件类型（1-重要；2-一般）+ 告警事件ID + 告警事件长度（时间+内容之和）+ 告警时间 + 告警内容；
BYTE ReadAlrItem(BYTE bAlrType, BYTE bAlrID, BYTE bRecNum, BYTE* pbBuf, BYTE* pbLen)
{
	//BYTE bType = GetErcType(bAlrID);
	//if (bAlrType != bType)
	//	return 0;
	BYTE bAlrItem = 0;//读到的告警事件记录条数（该条数<=允许上报条数）
	BYTE bLen = 0;
	WORD i=0;
	BYTE bBuf[37];

	if (bAlrID == 22) //差动组告警事件每条记录长度不固定，且用的比较少，故不在此函数中判断（待以后增补该事件读取函数，或将差动组告警记录格式改为记录长度固定。）
		return 0;

	memset(bBuf, 0, sizeof(bBuf));

	*pbLen = 0;
	//while (ReadAlrRec(bAlrType, j++, bBuf, MAX_ALR_LEN)>0)
	while (ReadAlrRec(bAlrType, i++, bBuf, 37)>0)
	{

		if (bBuf[0]==bAlrID)
		{
			bLen = bBuf[1]+2;
			memcpy(pbBuf++, &bAlrType, 1);
			memcpy(pbBuf, bBuf, bLen);
			pbBuf += bLen;
			
			if (++bAlrItem > bRecNum)
				break;
			
			*pbLen += bLen;
			memset(bBuf, 0, sizeof(bBuf));
			
		}
		else if (bBuf[0] == 0)
			break;

		if (i>255)
			break;

	}
	return bAlrItem;
}*/

