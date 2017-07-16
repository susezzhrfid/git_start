/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterAPI.cpp
 * 摘    要：本文件主要实现抄表的公共接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#include "MtrAPI.h"
#include "DbGbAPI.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "MtrStruct.h"
#include "SysDebug.h"
#include "DrvCfg.h"
#include "MtrCtrl.h"
#include "ExcTask.h"
#include "GbPro.h"

DWORD GbValToBaudrate(BYTE val)
{
	static const DWORD dwBaudrate[] = {0, CBR_600, CBR_1200, CBR_2400, 
								  CBR_4800, CBR_4800, CBR_9600, CBR_19200};
	if (val <= 7)
		return dwBaudrate[val];
	else
		return CBR_1200;
}

BYTE GbBaudrateToVal(DWORD dwBaudRate)
{
	BYTE i;
	static const DWORD dwBaudrate[] = {0, CBR_600, CBR_1200, CBR_2400, 
								  CBR_4800, CBR_4800, CBR_9600, CBR_19200};
	for (i=0; i<sizeof(dwBaudrate)/sizeof(DWORD); i++)
	{
		if (dwBaudrate[i] == dwBaudRate)
			return i;
	}
	return 0;
}

BYTE GbValToParity(BYTE val)
{
	static const BYTE bParityTab[] = {NOPARITY, ODDPARITY, EVENPARITY}; 

	if (val < 3)
		return bParityTab[val];
	else	
		return NOPARITY;
}


BYTE GbValToStopBits(BYTE val)
{
	static const BYTE bStopBitsTab[] = {ONESTOPBIT, TWOSTOPBITS, ONE5STOPBITS};
	if (val>0 && val<=3)
		return bStopBitsTab[val-1];
	else
		return ONESTOPBIT;
}

BYTE GbValToByteSize(BYTE val)
{
	if (val>=5 && val<=8)
		return val;
	else
		return 8;
}

//描述:电表测量点串口参数的缺省定义
void GetDefaultCommPara(TMtrPara* pMtrPara)
{	
	TCommPara* pCommPara = &pMtrPara->CommPara;
	switch (pMtrPara->bProId)
	{	
#ifdef PROTOCOLNO_HT3A
		case PROTOCOLNO_HT3A://珠海恒通		
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_1200;//波特率
			pCommPara->bParity = 1;//奇校验
			pCommPara->bByteSize = 8;//数据位
			break;
#endif

#ifdef PROTOCOLNO_ABB2
		case PROTOCOLNO_ABB2://ABB圆表
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_1200;//波特率
			pCommPara->bParity = 0;//无校验
			pCommPara->bByteSize = 8;//数据位	
			break;	
#endif			
#ifdef PROTOCOLNO_ABB
		case PROTOCOLNO_ABB://ABB方表
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_1200;//波特率
			pCommPara->bParity = 0;//无校验
			pCommPara->bByteSize = 8;//数据位			
			break;	
#endif	
		
#ifdef PROTOCOLNO_EDMI
		case PROTOCOLNO_EDMI://红相表
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_9600;//波特率
			pCommPara->bParity = 0;//无校验
			pCommPara->bByteSize = 8;//数据位	
			break;
#endif	
			
#ifdef PROTOCOLNO_1107
		case PROTOCOLNO_1107://	A1700 1107规约
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_1200;//波特率
			pCommPara->bParity = 2;//偶校验
			pCommPara->bByteSize = 7;//数据位
			break;
#endif				

#ifdef PROTOCOLNO_LANDIS
		case PROTOCOLNO_LANDIS://兰吉尔1107规约
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_300;//波特率
			pCommPara->bParity = 2;//偶校验
			pCommPara->bByteSize = 7;//数据位
			break;
#endif	

#ifdef PROTOCOLNO_LANDIS_ZMC
		case PROTOCOLNO_LANDIS_ZMC://兰吉尔ZMC规约
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_9600;//波特率
			pCommPara->bParity = 0;//无校验
			pCommPara->bByteSize = 8;//数据位	
			break;
#endif

#ifdef PROTOCOLNO_DLMS
		case PROTOCOLNO_DLMS://爱拓利DLMS规约
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_9600;//波特率
			pCommPara->bParity = 0;//无校验
			pCommPara->bByteSize = 8;//数据位	
			break;
#endif				

#ifdef PROTOCOLNO_LANDIS_DLMS
		case PROTOCOLNO_LANDIS_DLMS://兰吉尔DLMS规约
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_300;//波特率
			pCommPara->bParity = 2;//偶校验
			pCommPara->bByteSize = 7;//数据位	
			break;
#endif

#ifdef PROTOCOLNO_HND
		case PROTOCOLNO_HND://浩宁达规约
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_1200;//波特率
			pCommPara->bParity = 0;//无校验
			pCommPara->bByteSize = 8;//数据位	
			break;
#endif	

#ifdef PROTOCOLNO_EMAIL
		case PROTOCOLNO_EMAIL://EMAIL表
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_1200;//波特率
			pCommPara->bParity = 0;//无校验
			pCommPara->bByteSize = 8;//数据位	
			break;	
#endif	

#ifdef PROTOCOLNO_DLT645_V07
		case PROTOCOLNO_DLT645_V07://2007版645表
			//密码用F10
			if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
				pCommPara->dwBaudRate = CBR_2400;//波特率
			pCommPara->bParity = 2;//偶校验
			pCommPara->bByteSize = 8;//数据位	
			break;	
#endif	

	default:
		if (pCommPara->dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
			pCommPara->dwBaudRate = CBR_1200;//波特率
		pCommPara->bParity = 2;//偶校验
		pCommPara->bByteSize = 8;//数据位	
		break;
	}
	pCommPara->bStopBits = GbValToStopBits(1);//停止位 =1

	pMtrPara->bRateTab[0] = 1;//费率顺序	//47
	pMtrPara->bRateTab[1] = 2;			//48
	pMtrPara->bRateTab[2] = 3;			//49
	pMtrPara->bRateTab[3] = 4;			//50	
}

void GetLogicPortNum(WORD* pwNum, WORD* pwMin, WORD* pwMax)
{
	*pwNum = LOGIC_PORT_NUM;
	*pwMin = LOGIC_PORT_MIN;
	*pwMax = LOGIC_PORT_MAX;
}


/*
//描述:抄表的逻辑端口到物理端口的映射
int MeterPortToPhy(BYTE bPortNo)
{
	BYTE bPortFun = PORT_FUN_RDMTR;
	if (bPortNo == LOGIC_PORT_MIN)
		return COMM_METER2;	    
	else if (bPortNo == (LOGIC_PORT_MIN+1))
	{
		ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
		//3口用于调试输出
		if (bPortFun != PORT_FUN_DEBUG)
			return COMM_METER;	
		else
			return -1;
	}
    else
    	return -1;
}*/



int MeterPortToPhy(BYTE bMeterPort)
{
	BYTE b = 0, b1 = 0, b3 = 0, bPortFun=0;
	ReadItemEx(BN0, PN0, 0x8710, (BYTE *)&b);			//抄表口
	ReadItemEx(BN0, PN0, 0x8720, (BYTE *)&b1);		//级联口
	//ReadItemEx(BN0, PN0, 0x8750, (BYTE *)&b2);			//测试口

//ReadItemEx(BANK7,POINT0, 0x7510, (BYTE *)&b3);//485顺序 //0-左级联  右抄表    1-左抄表  右级联

	if (bMeterPort == PORT_CCT_PLC || bMeterPort == PORT_CCT_WIRELESS)
		return COMM_METER3;
	
	b3=0;	//待扩展参数
	if (b3 == 0) 
	{
		if(bMeterPort == 1)
		{
			if(b1 == 0) // 测试口没有被占用
			{
				ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);	//功能设置，是否用于调试输出
				if (bPortFun != PORT_FUN_DEBUG)
					return COMM_METER;
			}
		}
		else
		{
			if (b == 0)//抄表口没有被占用
				return COMM_METER2;
		}
	}
	else
	{
		if(bMeterPort == 1)
		{
			if(b1 == 0)//抄表口没有被占用
				return COMM_METER2;
		}
		else
		{
			if (b == 0)//测试口没有被占用
				return COMM_METER;
		}
	}

	return -1;//没有就返回负数
}


BYTE MeterProtoLocalToInner(BYTE bProto)
{
	switch (bProto)
	{
	case 0x00:
	case 0x02:
		return PROTOCOLNO_DLT645;
	case 0x01:
	case 0x03:
		return PROTOCOLNO_DLT645_V07;
	/*case 0x04:	   
		return PROTOCOLNO_WS;
	case 0x05:
	case 0x06:
		return PROTOCOLNO_LANDIS;
	case 0x07:
		return PROTOCOLNO_EDMI;
	case 0x08:
		return PROTOCOLNO_ABB;
	case 0x09:
		return PROTOCOLNO_DLMS;
	case 0x0A:
		return PROTOCOLNO_EMAIL;
	case 0x0B:
		return PROTOCOLNO_HND;
	case 0x0C:
		return PROTOCOLNO_LANDIS_DLMS;*/
	default:
		return PROTOCOLNO_DLT645_V07;
	}
}


//描述:负荷控制国标2005协议的电表参数初始化
//参数:@pMtrInf	指向存放电表参数数组的电表参数结构指针
//	   @&bNum	返回的有效电表参数的数目
//返回:成功则返回true		
bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara)
{	
	int iPort=0;
	BYTE bProId, bProp = 0;
	BYTE bBuf[32];

	if (wPn == PN0)//测量点0不需要去抄表
		return false;

	if (ReadItemEx(BN0, wPn, 0x8900, bBuf) <= 0)
		return false;

	if (bBuf[0] == 0)
		return false;

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return false;

	if (bProp != PN_PROP_METER)
		return false;

	pMtrPara->wPn = wPn;
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;

	memcpy(pMtrPara->bAddr,bBuf,6);//端口地址
	if (ReadItemEx(BN0, wPn, 0x8903, bBuf) <= 0)
		return false;	

	bProId = bBuf[0];
	pMtrPara->bProId = MeterProtoLocalToInner(bProId);
	//ReadItemEx(BANK3, POINT0, 0x30d0+wPn, bBuf);
	//pMtrPara->bSubProId = bBuf[0];

	if (ReadItemEx(BN0, wPn, 0x890b, bBuf) <= 0)
		return false;

	pMtrPara->CommPara.dwBaudRate = ValToBaudrate(bBuf[0]);//波特率
	pMtrPara->CommPara.bParity = ValToParity(bBuf[1]);//校验位
	pMtrPara->CommPara.bByteSize = ValToByteSize(bBuf[2]);//数据位数
	pMtrPara->CommPara.bStopBits = ValToStopBits(bBuf[3]);//停止位

	if (ReadItemEx(BN0, wPn, 0x8930, bBuf) <= 0)
		return false;

	memcpy(pMtrPara->bPassWd, bBuf, 8);//密码	

	if (ReadItemEx(BN0, wPn, 0x890a, bBuf) <= 0) //端口号
		return false;
	//DTRACE(DB_METER, ("GetMeterPara :Point%d 0x8901=%d,8930=%d,8932=%d,8931=%d,8903=%d,8905=%d\n", wPn,bProp,pMtrInf->CommPara.bParity, pMtrInf->CommPara.bStopBits, pMtrInf->CommPara.bByteSize,pMtrInf->bProId, pMtrInf->CommPara.dwBaudRate));

	iPort = MeterPortToPhy(bBuf[0]); //描述:抄表的逻辑端口到物理端口的映射
	if (COMM_METER3 == iPort)
	{
		pMtrPara->CommPara.dwBaudRate = CBR_9600;//波特率
		pMtrPara->CommPara.bParity = EVENPARITY;//校验位
		pMtrPara->CommPara.bByteSize = 8;//数据位数
		pMtrPara->CommPara.bStopBits = 1;//停止位
	}

	if (iPort < 0)
	{
		DTRACE(DB_METER, ("GetMeterPara : fail to map port %d to physic\n", iPort));
		return false;
	}

	pMtrPara->CommPara.wPort = (WORD )iPort;
	/*pMtrInf->wMeterPort = bBuf[0]+1;
	//这里调整只对645起作用，要在入库的地方统一调整
	if (pMtrInf->bProId == PROTOCOLNO_DLT645_V07)//2007版645
	{
		pMtrInf->bExtValid = 0xff;
		pMtrInf->bWakeUpCtrl = 4;//唤醒字符
		pMtrInf->bEnergyPtPos = 2;//电能小数位
		pMtrInf->bNeedPtPos = 4;//需量小数位
		pMtrInf->bVolPtPos = 1;//电压小数位
		pMtrInf->bCurPtPos = 3;//电流小数位
		pMtrInf->bActPowerPtPos = 4;//有功功率小数位
		pMtrInf->bReActPowerPtPos = 4;//无功功率小数位
		pMtrInf->bPowerFactorPtPos = 3;//功率因素小数位
	}
	else//97版645
	{
		pMtrInf->bExtValid = 0xff;
		pMtrInf->bWakeUpCtrl = 0;//唤醒字符
		pMtrInf->bEnergyPtPos = 2;//电能小数位数   默认 2
		pMtrInf->bNeedPtPos = 4;////需量小数位数   默认 4
		pMtrInf->bVolPtPos = 0;//电压小数位数  默认 0
		pMtrInf->bCurPtPos = 2;//电流小数位数  默认 2
		pMtrInf->bActPowerPtPos = 4;//有功功率小数位数/  默认4
		pMtrInf->bReActPowerPtPos = 2;//无功功率小数位数 默认2
		pMtrInf->bPowerFactorPtPos = 3;//功率因数小数位数/  默认 3
	}

	ReadItemEx(BANK1, POINT0, 0x2050+wPn, pMtrInf->bRateTab);  //0x2050 4 电表->645费率对照表0，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	if (pMtrInf->bRateTab[0]==0 || pMtrInf->bRateTab[0]>4)
	{
		pMtrInf->bRateTab[0] = 1;
		pMtrInf->bRateTab[1] = 2;
		pMtrInf->bRateTab[2] = 3;
		pMtrInf->bRateTab[3] = 4;
	}	*/

	//if (pMtrPara->CommPara.dwBaudRate == 0) //国标规约,当波特率为0时,取用缺省参数
	{
		GetDefaultCommPara(pMtrPara);
	}

	return true;
}


//描述:取抄表间隔
/*BYTE GetMeterInterv(WORD wPn)
{
#ifdef PRO_698
	BYTE bBuf[110];
	BYTE bMeterInterv = 0;
	BYTE bPort = 2;
	BYTE bMode = 0;
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
	{
		bMeterInterv = 60;	//15
		return bMeterInterv;
	}
	bPort = bBuf[MTR_PORT_OFFSET]&0x1f;
	if (ReadItemEx(BN0, bPort-1, 0x021f, bBuf) > 0)	//F33
	{	
		bMeterInterv = bBuf[9]; //8 bBuf[20]; //如果没有设置,系统库默认值为0,下面会自动取默认值
	}
	else
	{
		bMeterInterv = 60; //15
	}

	if (bMeterInterv==0 || bMeterInterv>60)
		bMeterInterv = 60; //15
	else if (bMeterInterv > 30)
		bMeterInterv = 60;
	else if (bMeterInterv > 15)
		bMeterInterv = 30;
	else if (bMeterInterv > 10)
		bMeterInterv = 15;

	ReadItemEx(BN2, PN0, 0x2040, &bMode);
	if (bMode==0 && bMeterInterv<60)
		bMeterInterv = 60; //过测试有效时间强制抄表间隔不小于60分钟

	return bMeterInterv;
#else
	return GetMeterInterv();
//#endif
}

//描述:取抄表日
bool GetPnDate(WORD wPn, BYTE* pbBuf)
{
	BYTE bPort;
	BYTE bBuf[MTR_PARA_LEN];
    if (sizeof(bBuf) < GetItemLen(BN0, 0x8902))
    {
        DTRACE(DB_METER, ("bBuf is too small\r\n"));
        return false;
    }
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
	{
		bPort = PN2;
	}
	else
	{
		bPort = bBuf[MTR_PORT_OFFSET]&0x1f;	//4
	}

	return GetMtrDate(bPort, pbBuf);
}*/
/*
//描述:取抄表日
bool GetMtrDate(BYTE bPort, BYTE* pbBuf)
{
	static const BYTE bDefault[6] = {0x01, 0x00, 0x00, 0x00, 0x10, 0x00} ;
	BYTE bBuf[110];
    
    if (sizeof(bBuf) < GetItemLen(BN0, 0x021f))
    {
        DTRACE(DB_METER, ("bBuf is too small\r\n"));
        return false;
    }
	if (ReadItemEx(BN0, bPort-1, 0x021f, bBuf) > 0)	//F33
	{	
		if (IsAllAByte(&bBuf[3], 0, 6)) //没有配置 2
			memcpy(pbBuf, bDefault, 6);
		else
			memcpy(pbBuf, &bBuf[3], 6);
	}
	else
	{
		memcpy(pbBuf, bDefault, 6);
	}
	return true;
}*/

//描述:	获取电表地址,
//参数:	@wPn 测量点号
//		@pbAddr 用来返回地址
//返回:	如果成功则返回true,否则返回false
//备注:	
bool GetMeterAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bBuf[MTR_PARA_LEN];

	if (!IsMtrPn(wPn))
		return false;
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf)<=0)
		return false;

	memcpy(pbAddr, bBuf+MTR_ADDR_OFFSET, 6);
	return true;
}


BYTE GetRdMtrInterV(BYTE bPort)
{
	//BYTE b,b1,b3;	//20140517-2
	BYTE bMeterU;
	BYTE bBuf[8];
	//BYTE bMode;
	BYTE bRdMtrFeq;

	memset(bBuf,0,sizeof(bBuf));

	/*ReadItemEx(BN2, PN0, 0x2040, &bMode);
	if (bMode != 0)//测试模式
	{
		bRdMtrFeq = 1;
	}
	else*/
	{
		// 		ReadItemEx(BN1, PN0, 0x2060, bBuf);  //0x2060 1 电表抄表间隔,BCD,单位分钟
		/*ReadItemEx(BN0, PN0,0x8710, (BYTE *)&b);
		ReadItemEx(BN0, PN0, 0x8720, (BYTE *)&b1);
		//ReadItemEx(BN0, PN0,0x8750, (BYTE *)&b2);
		//ReadItemEx(BANK7,POINT0, 0x7510, (BYTE *)&b3);//485顺序 //0-左级联  右抄表    1-左抄表  右级联
		b3 = 0;
		if (0==b3)
		{
			if (0==b)
				ReadItemEx(BN0, PN0, 0x8820, bBuf);
			else if (0==b1)
				ReadItemEx(BN0, PN0, 0x8821, bBuf);
			//else if (0==b2)
			//	ReadItem(PN0, 0x8822, bBuf);
		} 
		else
		{
			if (0==b)
				ReadItemEx(BN0, PN0, 0x8822, bBuf);
			else if (0==b1)
				ReadItemEx(BN0, PN0, 0x8821, bBuf);
			//else if (0==b2)
			//	ReadItemEx(BN0, PN0, 0x8820, bBuf);
		}*/

		if ((PORT_CCT_PLC != bPort && PORT_CCT_WIRELESS != bPort))
		{
			if (bPort >= LOGIC_PORT_NUM)
				bPort = 0;

			ReadItemEx(BN0, PN0, 0x8820+bPort, bBuf);
		}
		else
		{
			if (PORT_CCT_PLC == bPort)
			{
				ReadItemEx(BN0, PN0, 0x8841, bBuf);
			}

			if (PORT_CCT_WIRELESS == bPort)
			{
				ReadItemEx(BN0, PN0, 0x8842, bBuf);
			}
		}
		
		bMeterU = bBuf[5];
		switch(bMeterU+2)//加2是为了与国网相对应
		{
		case TIME_UNIT_MINUTE:
			bRdMtrFeq = bBuf[6];
			if(bRdMtrFeq>60)
				bRdMtrFeq = 60;
			break;
		case TIME_UNIT_HOUR:
		case TIME_UNIT_DAY:
		case TIME_UNIT_MONTH:
			bRdMtrFeq = 60;
			break;
		}

		if(0==bRdMtrFeq)
		{
			ReadItemEx(BN1, PN0, 0x2060, bBuf);  //0x2060 1 电表抄表间隔,BCD,单位分钟
			bRdMtrFeq = BcdToByte(bBuf[0]);
		}
		if (bRdMtrFeq==0 ||bRdMtrFeq>60)
			bRdMtrFeq = 60;
	}

	return bRdMtrFeq;
}

//描述:取抄表间隔
BYTE GetMeterInterv(WORD wPn)
{
	return GetRdMtrInterV(GetPnPort(wPn));
}


//描述:取得所有测量点屏蔽位,
void GetAllPnMask(BYTE* pbPnMask)
{
	ReadItemEx(BN0, PN0, 0x056f, pbPnMask); //测量点屏蔽位
}

//描述：产生事件
//参数: 
//返回: 
//备注: 一旦有搜到新电表，就产生事件
void PushEvtParaChanged()
{
	TTime now;
	BYTE bErcData[6], *pbBuf;

	pbBuf = bErcData;
	*pbBuf++ = 0;	//主站MAC地址

	*pbBuf++ = 0;//DA
	*pbBuf++ = 0;//DAG
	*pbBuf++ = 0x02;//DT
	*pbBuf++ = 0x01;//DTG

	GetCurTime(&now);

	g_dwUpdateTime = GetCurSec();
// 	SaveAlrData(ERC_PARACHG, now, bErcData, (int)(pbBuf-bErcData), 0);
}

//描述:搜索测量点
WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn)
{
	WORD i, j;
	BYTE bBitMask;
	i = wStartPn >> 3;
	j = wStartPn & 7;
	for (; i<PN_MASK_SIZE; i++)
	{
		if (pbPnMask[i] != 0)
		{
			bBitMask = 1 << j;
			for (; j<8; j++,bBitMask<<=1)
			{
				if (pbPnMask[i] & bBitMask)
					return (i<<3)+j;
			}
		}

		j = 0;
	}

	return POINT_NUM;
}

//描述：搜索一个空白的测量点号
WORD SearchUnUsedPnFromMask(const BYTE* pbPnMask, WORD wStartPn)
{
	WORD i, j;
	i = wStartPn >> 3;
	j = wStartPn & 7;
	for (; i<PN_MASK_SIZE; i++)
	{
		BYTE bBitMask = 1 << j;
		for (; j<8; j++,bBitMask<<=1)
		{
			if (!(pbPnMask[i] & bBitMask))
				return (i<<3)+j;
		}

		j = 0;
	}

	return POINT_NUM;
}

//描述:根据测量点参数找出该端口抄表总数和抄表成功数
BYTE GetMtrNumByPort(WORD wPort, WORD* pbTotalMtrNum, WORD* pwMtrRdSuccNum)
{
	WORD i;
	BYTE bPos, bMask;
	BYTE bMtrNum = 0;
	BYTE bMtrRdSuccNum = 0;

	for (i=1; i<POINT_NUM; i++)
	{
		if ((GetPnProp(i)==PN_PROP_METER && GetPnPort(i)==wPort) ||
			(GetPnProp(i)==PN_PROP_CCT && GetPnPort(i)==wPort))
		{
			bMtrNum++;

			bPos = i>>3;
			bMask = 1 << (i & 7);
			if (g_bMtrRdStatus[bPos] & bMask)
				bMtrRdSuccNum++;
		}
	}	

	*pbTotalMtrNum += bMtrNum;
	*pwMtrRdSuccNum += bMtrRdSuccNum;
	return bMtrNum;
}

//描述:根据测量点参数找出已经找到(或配置)了多少块表
BYTE GetMtrNum()
{
	WORD wPn;
	BYTE bBuf[8];
	BYTE bMtrNum = 0;

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		if (GetPnProp(wPn) == PN_PROP_METER) 
			bMtrNum++;
	}	

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8920, bBuf);

		if(!IsAllAByte(bBuf, 0, sizeof(bBuf)))
			bMtrNum++;
	}	

	return bMtrNum;	
}

BYTE GetMtrClassNum(WORD wId, WORD wExpPn, BYTE bMtrClass)
{
	WORD wPn;
	BYTE bBuf[2];
	BYTE bMtrNum = 0;

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		if(wPn == wExpPn)
			continue;

		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8900, bBuf);
		ReadItemEx(BN0, wPn, wId, bBuf+1);

		if(bBuf[0]==1 && bBuf[1]==bMtrClass)
			bMtrNum++;
	}	

	return bMtrNum;	

}


BYTE GetVitalPn(BYTE* p)
{
	WORD wPn;
	BYTE bBuf[2];
	BYTE bLen = 0;
	BYTE* pTemp = p;

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8900, bBuf);
		ReadItemEx(BN0, wPn, 0x8906, bBuf+1);

		if(bBuf[0]==1 && bBuf[1]==1)
		{
			DWORDToBCD(wPn, pTemp, 2);
			pTemp += 2;
			bLen  += 2;
		}
	}	

	return bLen;	

}
/*
//描述:删除30天未成功抄表的pn
bool DelReadFailPn()
{
	WORD wPn, wSn;
	BYTE bBuf[8];
	BYTE bTemp[3] = {0};
	TTime tLastTime, tCurTime;
	GetCurTime(&tCurTime);

	for(wPn = 0; wPn<POINT_NUM; wPn++)
	{
		memset(bBuf , 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn ,0x8921, bBuf);
		if (memcmp(bBuf, bTemp, 3)==0 || !IsMtrPn(wPn))
			continue;

		tLastTime.nYear = bBuf[0]+2000;
		tLastTime.nMonth = bBuf[1];
		tLastTime.nDay = bBuf[2];

		memset(bBuf , 0x00, sizeof(bBuf));
		if (DaysPast(&tLastTime, &tCurTime) >= 30)
		{ 
			bBuf[0] = 1;
			bBuf[1] = 0;
			wSn = MtrPnToSn(wPn);
			WordToByte(wSn, &bBuf[2]);
			bBuf[4] = 0;
			bBuf[5] = 0;
			WriteItemEx(BN0, BN0, 0x00af, bBuf);
			//WriteItemEx(BN0, wPn ,0x8921, bTemp); //删除该测量点的配置时间
			DTRACE(DB_POINT, ("DelReadFailPn: Delete Pn:%d due to Read meter fail 30 days!!\r\n",wPn));
		}
	}

	return true;
}

//描述：找一个抄表失败时间最长的表并删除
WORD  SearchOnePnReadFail(WORD wMinPn, WORD wMaxPn)
{
	WORD i, wSn, wPn = wMinPn;
	BYTE bBuf[8];
	BYTE bTemp[3] = {0};
	DWORD dwReadDays = 0, dwtemp = 0;

	TTime tLastTime;
	GetCurTime(&tLastTime);

	//找出第一个抄表时间
	for(i= wMinPn; i<wMaxPn; i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, i ,0x8921, bBuf);
		if(memcmp(bBuf, bTemp, 3)==0 || !IsMtrPn(i))
			continue;

		tLastTime.nYear = bBuf[0]+2000;
		tLastTime.nMonth = bBuf[1];
		tLastTime.nDay = bBuf[2];

		dwReadDays = DaysFrom2000(&tLastTime);
		break;
	}

	for(i = wMinPn; i<wMaxPn; i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, i ,0x8921, bBuf);
		if(memcmp(bBuf, bTemp, 3)==0 || !IsMtrPn(i))
		{
			continue;
		}

		tLastTime.nYear = bBuf[0]+2000;
		tLastTime.nMonth = bBuf[1];
		tLastTime.nDay = bBuf[2];

		dwtemp = DaysFrom2000(&tLastTime);
		if(dwtemp < dwReadDays) //这个更早
		{
			wPn = i;
			dwReadDays = dwtemp;
		}
	}

	//删除该表
	bBuf[0] = 1;
	bBuf[1] = 0;
	wSn = MtrPnToSn(wPn);
	WordToByte(wSn, &bBuf[2]);
	bBuf[4] = 0;
	bBuf[5] = 0;
	WriteItemEx(BN0, BN0, 0x00af, bBuf);
	//WriteItemEx(BN0, wPn ,0x8921, bTemp); //删除该测量点的配置时间
	DTRACE(DB_POINT, ("SearchDelReadFailPn: Delete Pn:%d due to Read Meter fail %ld days!!\r\n",wPn, dwReadDays));

	return wPn;
}*/

//描述：存储搜到的电表信息
bool SaveSearchPnToPointSect(BYTE* pbMtrAddr, BYTE bPro, BYTE bPort)
{
	//BYTE bProp;
	WORD wPn;
	BYTE  bAddrTemp[6];
	BYTE bPnFlg[256], bBuf[8];

	if(pbMtrAddr == NULL)
		return false;

	memset(bPnFlg, 0, sizeof(bPnFlg));
	ReadItemEx(BN0, PN0, 0x0560, bPnFlg); //测量点屏蔽位
	//查找是否存在相同的电表
	for (wPn=1; wPn<POINT_NUM; wPn++)
	{	
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8920, bBuf);
		//电表地址一样，说明已经保存了
		if (memcmp(bBuf, pbMtrAddr, 6)==0 && !IsAllAByte(pbMtrAddr, INVALID_DATA, 6))
		{  //电表地址相等                                 &&  有效的电表地址
			return false;
		}
		if (GetMeterAddr(wPn, bAddrTemp))
		{
			if (memcmp(bAddrTemp, pbMtrAddr, 6) == 0)//是F10已经配置好的电表
			{
				return false;
			}
		}
	}

	//全新的电表，找一个空位保存起来
	wPn = 1;
	while (wPn < POINT_NUM)
	{
		memset(bBuf, 0, sizeof(bBuf));

		//这里搜出来的是F10里面没有配的空白测量点号
		wPn = SearchUnUsedPnFromMask(bPnFlg, wPn);

		if (wPn == POINT_NUM)
			break; //wPn = SearchOnePnReadFail(2, POINT_NUM); //一般交采测量点为1

		ReadItemEx(BN0, wPn, 0x8920, bBuf);
		if (IsAllAByte(bBuf, 0, sizeof(bBuf))) //电表位置为0，说明这个位置可以存放
		{ 
			memcpy(bBuf, pbMtrAddr, 6);
			bBuf[6] = bPro;
			bBuf[7] = bPort;
			WriteItemEx(BN0, wPn, 0x8920, bBuf);
			DTRACE(DB_POINT, ("SaveSearchPnToPointSect: Find a New Meter, New Pn No:%d, Meter addr :%02x%02x%02x%02x%02x%02x, bPro:%d, Port & Baud:%d .\r\n", wPn, bBuf[0], bBuf[1], bBuf[2], bBuf[3], bBuf[4], bBuf[5], bBuf[6], bBuf[7]));
			break;
		}

		wPn++;
	}
	return true;
}

/*bool SaveSearchPnToF10()
{
	BYTE  bPnFlg[256];
	BYTE  bBuf[30];
	BYTE  bPnBuf[8];
	BYTE  bAddrTemp[6];
	bool  bIsSave = false;
	bool  bIsValid = false;
	WORD wPos, wPn, wPnTmp;
	BYTE bMask, bSchMtr;

	ReadItemEx(BN0, PN0, 0x07ef, &bSchMtr);	//--自动维护状态：00－关闭，01－启用并更新F10，02－启用但不更新F10，缺省为关闭
	if (bSchMtr != 0x01)
		return false;

	//集中器在收到查询测量点状态命令（F150）n分钟（n=0~20，一般取10）内不做本地F10终端电能表/交流采样装置配置参数的自动更新。
	//时间判断应该放到这里，因为一旦写了F10，F150中的测量点有效标志自动同步生效，但是变更标志却是异步地更新到F150
	//所以这里的做法应该是F10跟F150同步更新
	if (GetCurSec() - g_dwUpdateTime <= 10*60)
		return false;
	memset(bPnFlg, 0x00, sizeof(bPnFlg));
	//ReadItemEx(BN0, PN0, 0x056f, bPnFlg);	//F150

	memset(bPnBuf, 0x00, sizeof(bPnBuf));
	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		bIsSave= false;
		ReadItemEx(BN0, wPn, 0x8920, bPnBuf);
		if (IsAllAByte(bPnBuf, 0x00, 8))
			continue;

		//查找是否存在相同的电表或者采集器
		for (wPnTmp=1; wPnTmp<POINT_NUM; wPnTmp++)
		{            
			if (GetMeterAddr(wPnTmp, bAddrTemp))
			{
				if (memcmp(bAddrTemp, bPnBuf, 6) == 0 )//F10存在相同表号
				{
					bIsSave = true;	//F10已经保存过
					memset(bPnBuf, 0x00, sizeof(bPnBuf));
					WriteItemEx(BN0, wPn, 0x8920, bPnBuf);
					break;
				}
			} //if (GetMeterAddr(wPnTmp, bAddrTemp))
		} //for (WORD wPnTmp=1; wPnTmp<POINT_NUM; wPnTmp++)

		if (bIsSave)
			continue;

		//在F10中不存在相同电表
		memset(bBuf, 0x00, sizeof(bBuf));
		WordToByte(1, bBuf);           //测量点个数
		WordToByte(wPn, &bBuf[2]);     //装置序号
		WordToByte(wPn, &bBuf[4]);     //测量点号
		bBuf[6] = bPnBuf[7];          //通信速率及端口号
		bBuf[7] = bPnBuf[6];          //通信协议类型
		memcpy(&bBuf[8], bPnBuf, 6);   //通信地址
		memset(&bBuf[14], 0, 6);       //通信密码
		bBuf[20] = 4;                  //费率个数
		bBuf[21] = 0;                  //整数及小数位个数
		bBuf[28] = 0x31;               //大小类号

		if (WriteItemEx(BN0, PN0, 0x00af, bBuf) > 0)
		{	
			bMask = 1<<(wPn&0x07);
			wPos = wPn>>3;

			if(wPos<256)
			{
				ReadItemEx(BN0, PN0, 0x0560, bPnFlg);	//F150
				bPnFlg[wPos] |= bMask; //有效标志
				WriteItemEx(BN0, PN0, 0x0560, bPnFlg);
				ReadItemEx(BN0, PN0, 0x0561, bPnFlg);	//F150
				bPnFlg[wPos] |= bMask; //变位标志
				WriteItemEx(BN0, PN0, 0x0561, bPnFlg);
			}
			DTRACE(DB_POINT, ("SaveSearchPnToF10: Add Meter To F10, Pn :%d, Meter addr :%02x%02x%02x%02x%02x%02x, bPro:%d, Port & Baud:%d .\r\n", wPn, bPnBuf[0], bPnBuf[1], bPnBuf[2], bPnBuf[3], bPnBuf[4], bPnBuf[5], bPnBuf[6], bPnBuf[7]));
		}

		memset(bPnBuf, 0x00, sizeof(bPnBuf));
		WriteItemEx(BN0, wPn, 0x8920, bPnBuf);
		bIsValid = true;
	}

	if (bIsValid)
	{
		//WriteItemEx(BN0, PN0, 0x056f, bPnFlg);
		DTRACE(DB_POINT, ("SaveSearchPnToF10: Update Pn's Flag OK!.###########\r\n"));
		PushEvtParaChanged();
	}

	return true;
}*/

static BYTE g_bMtrChgFlg[PN_MASK_SIZE];

void SetMtrChgFlg(WORD wPn)
{
	BYTE bPos = wPn>>3;
	BYTE bMask = 1<<(wPn & 7);
	
	g_bMtrChgFlg[bPos] |= bMask;
}

void ClrMtrChgFlg(WORD wPn)
{
	BYTE bPos = wPn>>3;
	BYTE bMask = 1<<(wPn & 7);
	
	g_bMtrChgFlg[bPos] &= ~bMask;
}

bool IsMtrParaChg(WORD wPn)
{
	BYTE bPos = wPn>>3;
	BYTE bMask = 1<<(wPn & 7);

	return (g_bMtrChgFlg[bPos]&bMask) != 0;
}

void NewMtrThread()
{
  	SaveSoftVerChg();
	NewThread("Mt0", MtrRdThread, (void * )0, 1024, THREAD_PRIORITY_LOWEST);		//堆栈小于640,VC上容易创建线程失败
	NewThread("Mt1", MtrRdThread, (void * )1, 1024, THREAD_PRIORITY_LOWEST);		//576
}

//描述：初始化抄表
bool InitMeter()
{
	memset(g_bMtrChgFlg, 0, sizeof(g_bMtrChgFlg));
	memset(g_bRdMtrAlr, 0, sizeof(g_bRdMtrAlr));
	memset(g_bRdMtrAlrStatus, 0, sizeof(g_bRdMtrAlrStatus));

	return true;
}
