/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbAPI.cpp
 * 摘    要：本文件主要实现协议相关的数据库标准接口之外的扩展接口
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 *********************************************************************************************************/
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "MtrAPI.h"
#include "DataManager.h"
#include "SysDebug.h"
#include "LibDbAPI.h"
#include "ExcTask.h"

//软件版本 
// const BYTE g_bSoftVer[SOFT_VER_LEN] = 
// {
// 	'C', 'L', 'O', 'U',	 //厂商代号 4
// 	'C', 'L', '8', '1', '8', 'K', '5', '-', //设备型号 8
// 	'0', '0', '1', 'n', //终端软件版本 4 x.xx A 主版本.副版本 测试版本
// 	0x05, 0x09, 0x14,   //终端软件发布日期
// 	'9', '2', '7', '0', '3', '2', '6', '4', 0, 0, 0, //终端配置容量信息 11 CPU版本(3), RAM大小(2), FLASH大小(2) 
// 	'1', '3', '7', '6', //终端通讯规约版本号 4
// 	'v', '0', '0', '1', //终端硬件版本 4
// 	0x01, 0x12, 0x12,   //终端硬件发布日期 3
// };
const BYTE g_bSoftVer[SOFT_VER_LEN] = 
{
	'C', 'L', '8', '1', '8', 'K', '5', '-', //设备型号 8  
	'0', '0', '2', 'i', //终端软件版本 4 x.xx A 主版本.副版本 测试版本  
	0x3d,0x5a,	 //厂商代号 2 
	0x04,//终端类型 01表示厂站，02表示专变，03表示公变，04表示集中器 1
	0x20, 0x10, 0x14,   //终端软件发布日期 3

	0x00,0x01, //终端通讯规约版本号 2

	0x00,0x12, //终端硬件版本 2
	0x15, 0x09, 0x14,   //终端硬件发布日期 3
	'9', '2', '7', '0', '3', '2', '6', '4', 0, 0, 0, //终端配置容量信息 11 CPU版本(3), RAM大小(2), FLASH大小(2) 
};
//内部软件版本
// const BYTE g_bInnerSoftVer[INN_SOFT_VER_LEN] = 
// {
// 	 '3', '0', '1', '2',     //版本4个字节 x.xx A 主版本.副版本 测试版本，正式归档的版本测试版本号为0
// 	 0x13, 0x01, 0x14,        //日期3个字节 BCD码，终端软件发布日期。
// };

const WORD g_FmtARDLen[23] = 
{
	0,
	7,
	91,
	159,
	27,
	31,
	35,
	55,
	35,
	37,
	10,
	58,
	9,
	20,
	34,
	25,
	25,
	25,
	10,
	23,
	12,
	263,//告警状态(1)+时间(6)+测量点号(在这里算3个)
	9,
};

//告警配置
const TAlrTaskCtrl g_AlrTaskCtrl[ALR_NUM] = 
{
	// 	{0xE2000001,	ARD2},//计量装置门开闭
	// 	{0xE2000002,	ARD2},//电流反极性(规约已删除此项)
	// 	{0xE2000003,	ARD2},//电压逆相序
	// 	{0xE2000004,	ARD2},//电流不平衡
	//	{0xE2000005,	ARD2},//电压不平衡
	//	{0xE2000006,	ARD2},//零序电流偏大
	//	{0xE2000007,	ARD2},//A相CT二次侧短路
	//	{0xE2000008,	ARD2},//B相CT二次侧短路
	//	{0xE2000009,	ARD2},//C相CT二次侧短路
	// 	{0xE200000A,	ARD2},//A相CT二次侧开路
	//	{0xE200000B,	ARD2},//B相CT二次侧开路
	//	{0xE200000C,	ARD2},//C相CT二次侧开路
	{0xE200000D,	ARD2},//A相潮流反向
	{0xE200000E,	ARD2},//B相潮流反向
	{0xE200000F,	ARD2},//C相潮流反向

	//{0xE2000010,	ARD2},//A相电流过流
	//{0xE2000011,	ARD2},//B相电流过流
	//{0xE2000012,	ARD2},//C相电流过流
	{0xE2000013,	ARD2},//A相电流失流
	{0xE2000014,	ARD2},//B相电流失流
	{0xE2000015,	ARD2},//C相电流失流
	{0xE2000016,	ARD2},//A相电压失压
	{0xE2000017,	ARD2},//B相电压失压
	{0xE2000018,	ARD2},//C相电压失压
	{0xE2000019,	ARD2},//全失压
	// 	{0xE200001A,	ARD2},//A相电压过压
	// 	{0xE200001B,	ARD2},//B相电压过压
	// 	{0xE200001C,	ARD2},//C相电压过压
	// 	{0xE200001D,	ARD2},//A相电压断相
	// 	{0xE200001E,	ARD2},//B相电压断相
	// 	{0xE200001F,	ARD2},//C相电压断相

	// 	{0xE2000020,	ARD14},//A相电压畸变
	// 	{0xE2000021,	ARD14},//B相电压畸变
	// 	{0xE2000022,	ARD14},//C相电压畸变
	// 	{0xE2000023,	ARD14},//A相电流畸变
	// 	{0xE2000024,	ARD14},//B相电流畸变
	// 	{0xE2000025,	ARD14},//C相电流畸变
	// 	{0xE2000026,	ARD2},//无功过补偿
	// 	{0xE2000027,	ARD2},//无功欠补偿
	// 	{0xE2000028,	ARD2},//功率超定值
	// 	{0xE2000029,	ARD2},//负荷过载
	// 	{0xE200002A,	ARD2},//超合同容量用电
	{0xE200002B,	ARD4},//剩余电费不足
	{0xE200002C,	ARD3},//示度下降
	// 	{0xE200002D,	ARD3},//电能表飞走
	{0xE200002E,	ARD2},//电能表停走
	// 	{0xE200002F,	ARD1},//电能表通讯失败

	// 	{0xE2000030,	ARD3},//差动告警
	// 	{0xE2000031,	ARD7},//最大需量手动复零
	{0xE2000032,	ARD1},//时钟电池电压过低
	{0xE2000033,	ARD2},//终端掉电
	{0xE2000034,	ARD2},//终端上电
	{0xE2000035,	ARD2},//电能表编程时间更改
	{0xE2000036,	ARD1},//电能表时段或费率更改
	// 	{0xE2000037,	ARD6},//电能表脉冲常数更改
	// 	{0xE2000038,	ARD5},//电能表的互感器倍率更改
 	{0xE2000039,	ARD9},//遥信变位
	{0xE200003A,	ARD10},//月通信流量越限
	{0xE200003B,	ARD12},//继电器变位
	{0xE200003C,	ARD12},//电能表拉合闸失败
	{0xE200003D,	ARD21},//抄表失败
	{0xE200003E,	ARD13},//电能表时钟异常
	// 	{0xE200003F,	ARD1},//电能表校时失败
	// 
	// 	{0xE2000040,	ARD15},//电能表A、B、C相失压总次数
	// 	{0xE2000041,	ARD16},//电能表A、B、C相失流总次数
	// 	{0xE2000042,	ARD17},//电能表A、B、C相潮流反向总次数
	// 	{0xE2000043,	ARD18},//电能表编程总次数
	// 	{0xE2000044,	ARD14},//A相电压偏差越限
	// 	{0xE2000045,	ARD14},//B相电压偏差越限
	// 	{0xE2000046,	ARD14},//C相电压偏差越限
	// 	{0xE2000047,	ARD14},//频率偏差越限
	// 	{0xE2000048,	ARD14},//A相闪变越限
	// 	{0xE2000049,	ARD14},//B相闪变越限
	// 	{0xE200004A,	ARD14},//C相闪变越限
	// 	{0xE200004B,	ARD14},//电压不平衡越限(规约已删除此项)
	// 	{0xE200004C,	ARD22},//发现未知电表
	// 	{0xE200004D,	ARD11},//表端钮盒开启告警
	// 	{0xE200004E,	ARD11},//表盖开启告警
	//事件记录任务
	// 	{0xE2010001,	ARD1},//电能表编程记录
	// 	{0xE201000F,	ARD1},//电量清零记录
	// 	{0xE2010010,	ARD1},//校时记录

	/**/{0x00000000,	ARD},/**///结束标志
};
WORD GetEventLen(DWORD dwId)
{
	if(dwId==0xE201000A)
		return 12;
	else if(dwId==0xE201000E)
		return 15;
	else if(dwId==0xE2010014)
		return 38;
	else
		return 0;

	return 0;
}

bool IsEventId(DWORD dwId)
{
	//暂只支持这三个ID的事件（停上电、控制、设参）
	if(dwId==0xE201000A || dwId==0xE201000E || dwId==0xE2010014)
		return true;

	return false;
}

WORD GetFmtARDLen(DWORD dwId)
{
	BYTE i;
	for(i=0; GetAlrID(i)!=0;i++)
	{
		if (GetAlrID(i) == dwId)
		{
			return g_FmtARDLen[g_AlrTaskCtrl[i].bFmt];
		}
	}
	
	return 0;
}

DWORD GetAlrID(BYTE i)
{
	return g_AlrTaskCtrl[i].dwAlrId;
}


//描述:获取非直流模拟量测量点性质
//返回:0表示无效；1表示交采；2表示电表，3表示脉冲//，4表示模拟量 
BYTE GetPnProp(WORD wPn)
{
	BYTE bProp;
	if (wPn >= POINT_NUM)// PN_NUM)
		return INVALID_POINT;
	
#if MTRPNMAP!=PNUNMAP		
	if (SearchPnMap(MTRPNMAP, wPn) < 0) //如果找到则返回对应的映射号,否则返回-1
		return INVALID_POINT;
#endif //MTRPNMAP!=PNUNMAP,
	
	if (ReadItemEx(BN0, wPn, 0x8900, &bProp) <= 0)	//测量点有效标志
		return INVALID_POINT;

	if (bProp != 1)
		return INVALID_POINT;

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return INVALID_POINT;

	return bProp;
}


//描述:取PN对应的端口号
//返回:如果正确则返回端口号,否则返回0
BYTE GetPnPort(WORD wPn)
{
	BYTE bBuf[10];
	BYTE bPort;
	if (ReadItemEx(BN0, wPn, 0x890a, bBuf) >= 0)
	{	
		bPort = bBuf[0]&0x3f;

		return bPort;
	}

	return 0;
}

//描述:取得测量点的电表协议类型
BYTE GetPnMtrPro(WORD wPn)
{
	BYTE bBuf[4];
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, wPn, 0x8903, bBuf);
	return MeterProtoLocalToInner(bBuf[0]);	
}

BYTE GetCctPnMtrPro(WORD wPn)
{
	BYTE bBuf[4];
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, wPn, 0x8903, bBuf);
	
	if (PROTOCOLNO_DLT645_V07 == MeterProtoLocalToInner(bBuf[0]))
		return CCT_MTRPRO_07;
	else
		return CCT_MTRPRO_97;
}


//描述:此测量点是否支持此Fn
bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass)
{
/*	const BYTE* pbCfg;
	const BYTE* p;	//指到相应组的用户小类号
	WORD i, wID;
	WORD sub;
	BYTE bMain;
	BYTE bSub;
	BYTE n, bMark, pos;
//#ifdef SYS_WIN
	BYTE bBuf[C2_CFG_LEN+10];
//#endif

	if (!GetUserType(wPn, &bMain, &bSub)) //获取用户用户大类号和小类号
		return false;

	wID = (bClass==1 ? 0x026f : 0x027f);

#ifdef SYS_WIN
	pbCfg = GetItemRdAddrID(BN0, bMain, wID, bBuf);
#else
	//pbCfg = GetItemRdAddrID(BN0, bMain, wID, NULL);
	pbCfg = bBuf;
	ReadItemEx(BN0, bMain, wID, bBuf);
#endif
	if (pbCfg==NULL)
		return false;

	if (bMain>=USR_MAIN_CLASS_NUM || bSub>=USR_SUB_CLASS_NUM || bFn==0)
		return false;

	if (pbCfg[0]==0xff && bMain==0)	//参数没有配置,用户大类号为0,则全部支持
	{
		for (i=0; i<sizeof(g_bMultiFn); i++)
		{
			if (bFn == g_bMultiFn[i])
	        	return true;
		}

		return false;
	}
	else if (pbCfg[0] >= USR_MAIN_CLASS_NUM)
		return false;

	//m = pbCfg[1];
	bMark = 1 << ((bFn-1)&0x07);
	pos = (bFn-1)>>3;
	p = &pbCfg[2+(WORD )bSub*33];	//指到相应组的用户小类号
	if ((bSub!=0) && (*p==bSub))	//用户小类号相等,如果小类号为0有配置,则按照这里的进行判断
	{
		n = p[1];	 //组数n
        if (pos >= n)
			return false;

		if ((bMark & p[2+pos]) != 0)
			return true;
		else
			return false;
	}
	else if (bSub == 0)	//小类号为0,默认为用户大类定义的所有数据配置项
	{
		BYTE bBuf[32];
		memset(bBuf, 0, sizeof(bBuf));
		for (sub=1; sub<USR_SUB_CLASS_NUM; sub++)
		{
			p = &pbCfg[2+(WORD )sub*33]; //指到相应组的用户小类号

			if (*p == sub)	//用户小类号相等
			{
				p++;	//跳过小类号
				n = *p++;	 //组数n
				if (n > 31)
					continue;

				for (i=0; i<n; i++)
				{
					bBuf[i] |= *p++;
				}
			}
		}

		if ((bMark & bBuf[pos]) != 0)
			return true;
		else
			return false;

	}*/

	return false;
}


//描述:获取用户用户大类号和小类号
/*bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub)
{
	BYTE bType;
	BYTE bBuf[32];
	if (!IsMtrPn(wPn))
		return false;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;

	bType = bBuf[MTR_USR_TYPE_OFFSET];
	*pbMain = (bType>>4) & 0x0f; 
	*pbSub = bType&0x0f;

	return true;
}*/

//描述:获取测量点类型和重点户属性
bool GetUserTypeAndVip(WORD wPn, BYTE* bVip, BYTE* bType)
{
	BYTE bBuf[2];
	if (!IsMtrPn(wPn))
		return false;

	if ((ReadItemEx(BN0, wPn, 0x8904, bBuf) <= 0) || (ReadItemEx(BN0, wPn, 0x8906, bBuf+1) <= 0))
		return false;
	*bVip = bBuf[1] & 0x0f; 
	*bType = bBuf[0]&0x0f;

	return true;
}


bool IsMtrPn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_METER;
}


//描述:获取事件的属性
//返回:0表示此事件无效,1表示重要事件,2表示一般事件
BYTE GetErcType(BYTE bErc)
{
	return IsAlrEnable(0xE2000001+bErc);
}


//终端软件版本变更保存
void SaveSoftVerChg()
{
	/*TTime tm;
	//有版本变更事件，等待任初始化完再写版本变更事件	
	if (g_SoftVerChg.time != 0)
	{
		SecondsToTime(g_SoftVerChg.time, &tm);
// 		SaveAlrData(ERC_INIT_VER, tm, g_SoftVerChg.bVerInfo, 0, 0);	
 	}

	//写入参数初始化事件，不和版本变更写一条事件，因为这个是掉电前的发生时间不同
	//有参数初始化事件，等待任初始化完再写	
	if (g_PowerOffTmp.bParaEvtType>0 && g_PowerOffTmp.ParaInit.time!=0)
	{
		SecondsToTime(g_PowerOffTmp.ParaInit.time, &tm);
// 		SaveAlrData(ERC_INIT_VER, tm, g_PowerOffTmp.ParaInit.bVerInfo, 0, g_PowerOffTmp.bParaEvtType);
		g_PowerOffTmp.bParaEvtType = 0;		
 	}*/
}

extern const BYTE g_bSoftVer[SOFT_VER_LEN];

//终端软件版本变更检测
void TermSoftVerChg()
{
	TTime tm;
	BYTE  bSoftVer[SOFT_VER_LEN];
    memcpy(bSoftVer, "SoftwareVer:", 12);
    bSoftVer[12] = 0;
    DTRACE(DB_DP, ("%s", bSoftVer));
    memcpy(bSoftVer, g_bSoftVer, 12);
    bSoftVer[12] = 0;
    DTRACE(DB_DP, ("%s\r\n", bSoftVer));

	WriteItemEx(BN0, PN0, 0x8817, (BYTE*)g_bSoftVer+18); //终端软件版本号
	WriteItemEx(BN0, PN0, 0x8818, (BYTE*)g_bSoftVer+12); //终端软件版本号
	WriteItemEx(BN0, PN0, 0x8819, (BYTE*)g_bSoftVer+20); //终端硬件版本号

	memcpy(bSoftVer, "CL818K5-SG-Std", 14);
	memset(bSoftVer+14, 0x00, 2);
	memcpy(bSoftVer+16, g_bSoftVer+8, 4);
	memcpy(bSoftVer+20, g_bSoftVer+15, 3);
	WriteItemEx(BN2, PN0, 0x2107, (BYTE*)bSoftVer);


/*	memset((BYTE*)&g_SoftVerChg, 0, sizeof(g_SoftVerChg)); //全局的缓存区
 
 	ReadItemEx(BN0, PN0, 0x100f, bSoftVer);
 	if (memcmp(bSoftVer+12, g_bSoftVer+12, SOFT_VER_LEN-12) != 0)//只要比较版本号？？
 	{
		GetCurTime(&tm);
		g_SoftVerChg.time = TimeToSeconds(&tm);
		g_SoftVerChg.bVerInfo[0] = 0x02;
		memcpy(&g_SoftVerChg.bVerInfo[1], &bSoftVer[12], 4);
		memcpy(&g_SoftVerChg.bVerInfo[5], &g_bSoftVer[12], 4);
		//SaveAlrData(ERC_INIT_VER, tm, bBuf);	//写版本变更事件			
		WriteItemEx(BN0, PN0, 0x100f, (BYTE *)g_bSoftVer);
		if (g_bSoftVer[12]=='V' && g_bSoftVer[13]=='0' && g_bSoftVer[14]=='.' && g_bSoftVer[15]=='5') //升级程序自动默认
		{
			BYTE bDayMonFrzSta = 1;
			WriteItemEx(BN24, PN0, 0x4111, &bDayMonFrzSta);
			WriteItemEx(BN24, PN0, 0x4115, &bDayMonFrzSta);
			WriteItemEx(BN24, PN0, 0x4116, &bDayMonFrzSta);
			WriteItemEx(BN0, PN0, 0x07bf, &bDayMonFrzSta); //允许主动上报
		}
 	}

	WriteItemEx(BN2, PN0, 0x2107, (BYTE *)g_bInnerSoftVer);*/
}

void PostDbInit()
{ 	
#ifdef PRO_698
	//SetDefFnCfg();		//设置默认大小类支持项配置
	//InitMtrSnToPn();	//本函数必须放到InitAcPn()前,因为涉及到装置序号到测量点号的映射
#endif

	TermSoftVerChg();
#if MTRPNMAP!=PNUNMAP
	//NewPnMap(MTRPNMAP, PN1);	//为内表固定映射一个测量PN1
#endif //MTRPNMAP!=PNUNMAP
    
	//if (!IsInMtrParaOk())
		//InitInMeterPn();
	//InitAcPn();
#if MTRPNMAP!=PNUNMAP
	UpdPnMap();
#endif
}

extern const TBankCtrl g_Bank0Ctrl[SECT_NUM];
extern const TBankCtrl g_BankCtrl[BANK_NUM];
//extern TPnMapCtrl g_PnMapCtrl[PNMAP_NUM];
TDbCtrl g_DbCtrl; //外界对数据库进行参数配置的数据库控制结构

//描述:系统数据库初始化
bool InitDB(void)
{
	memset(&g_DbCtrl, 0, sizeof(g_DbCtrl));

	//BANK0的控制字段
	g_DbCtrl.wSectNum = SECT_NUM;	//BANK0中的SECT数目
	g_DbCtrl.pBank0Ctrl = g_Bank0Ctrl;

	//BANK控制字段
	g_DbCtrl.wBankNum = BANK_NUM;	//支持的BANK数目
	g_DbCtrl.pBankCtrl = g_BankCtrl;
								 
	//测量点动态映射控制字段
	g_DbCtrl.wPnMapNum = PNMAP_NUM;		//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
	g_DbCtrl.pPnMapCtrl = NULL; //g_PnMapCtrl;	//整个数据库不支持测量点动态映射则设为NULL

	g_DbCtrl.wSaveInterv = 15;			//保存间隔,单位分钟

	//g_DbCtrl.pDbUpgCtrl = &g_DbUpgCtrl;  使用了测量点动态映射，文件大小保持跟原来一致，不用升级方案

	if (!InitDbLib(&g_DbCtrl)) //版本变更事件用到任务库
		return false;

	PostDbInit();

	//CheckHisProRptPara();
	
	return true;
}


//清测量点(电表,交采,脉冲)数据
void ClrPnData(WORD wPn)
{
}

#if MTRPNMAP!=PNUNMAP
void UpdPnMap()
{
	bool fPnMapFail = false;
	WORD wPn;
	BYTE bProp;

	//WaitSemaphore(g_semMtrPnMap, SYS_TO_INFINITE);
	//DTRACE(DB_DP, ("UpdPnMask()******1! \r\n"));

	for (wPn=1; wPn<PN_NUM; wPn++)
	{
		bProp = GetPnProp(wPn);

		//避免前后标志位的不一致,不管怎样,有效的测量点都重新申请一下测量点映射
		//无效的测量点都删除一下映射
		if (bProp == PN_PROP_METER)	//测量点有效
		{
			if (NewPnMap(MTRPNMAP, wPn) < 0)
				fPnMapFail = true;
		}
		else //测量点无效
		{
			DeletePnMap(MTRPNMAP, wPn);
		}
	}

	if (fPnMapFail)	//上一轮申请失败,有可能是后面的测量点还没释放,前面的测量点又在配置
	{				//再申请一轮就行了
		for (wPn=1; wPn<PN_NUM; wPn++)
		{
			bProp = GetPnProp(wPn);

			if (bProp == PN_PROP_METER)	//测量点有效
			{
				NewPnMap(MTRPNMAP, wPn);
			}
		}
	}

	//SignalSemaphore(g_semMtrPnMap);
}
#endif


//描述:是否V2007版645协议测量点
bool IsV07Mtr(WORD wPn)
{
	BYTE bType;
	BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return false;

	if (bProp != PN_PROP_METER)
		return false;

	if (ReadItemEx(BN0, wPn, 0x8903, &bType) <= 0)
		return false;	

	if (bType == PROTOCOLNO_DLT645_V07)
		return true;
	else
		return false;	
}

//描述:获取测量点冻结参数设置
void GetFrzStatus(WORD wPn, BYTE* pbCurveStatus, BYTE* pbDayFrzStatus, BYTE* pbDayFlgFrzStatus)
{
	ReadItemEx(BN10, wPn, 0xa190, pbCurveStatus);
	ReadItemEx(BN10, wPn, 0xa191, pbDayFrzStatus);
	ReadItemEx(BN10, wPn, 0xa1b2, pbDayFlgFrzStatus);
}

//描述：清除测量点曲线每天的96点设置，用于正常换日时
void ClearCurveFrzFlg(WORD wPn)
{
	BYTE bIsSaveFlg[19];//本数据块每天96点入库状态
	TTime tNow;		
	GetCurTime(&tNow);

	memset(bIsSaveFlg, 0, 12);
	bIsSaveFlg[12] = tNow.nYear-2000; 
	bIsSaveFlg[13] = tNow.nMonth;
	bIsSaveFlg[14] = tNow.nDay;

	WriteItemEx(BN0, wPn, 0xd881, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd889, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd901, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd905, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd945, bIsSaveFlg);	
	DTRACE(DB_DP, ("ClearCurveFrzFlg : day change has to clear wPn=%d\n", wPn));	
}

//描述：设置测量点费率数
void SetPnRateNum(WORD wPn, BYTE bRateNum)
{ 
	WriteItemEx(BN0, wPn, 0x8903, &bRateNum);
}

//描述:获取测量点的费率数
BYTE GetPnRateNum(WORD wPn)
{
	BYTE bRateNum = RATE_NUM;	
	WORD wBn = BN0;

	ReadItemEx(wBn, wPn, 0x8903, &bRateNum);	

	if (bRateNum==0 || bRateNum>RATE_NUM)
		bRateNum = RATE_NUM;

	return bRateNum;
}

//描述:是否根据电表返回电能数据的实际长度修改测量点的费率数
bool IsChgRateNumByMtr()
{
	BYTE bVal = 0;
	ReadItemEx(BN10, PN0, 0xa1a1, &bVal);
	return ((bVal==0)?false:true);
}

//描述：设置测量点为单相分时表single-phase time-division meter的标志
void SetPnSPHTDMtr(WORD wPn, BYTE bSPMR)
{ 	
	WriteItemEx(BN24, wPn, 0x4107, &bSPMR);
}

//描述：查询测量点是否为单相分时表的标志
bool IsPnSPHTDMtr(WORD wPn)
{ 	
	BYTE bVal = 0;
	ReadItemEx(BN24, wPn, 0x4107, &bVal);
	return ((bVal==0)?false:true);	
}

//描述:根据大小类号区分07单相表,以抄读曲线时转成抄读整点冻结数据
bool IsSinglePhaseV07Mtr(WORD wPn)
{
	return false;
}


//描述:根据协议号区分是只支持单抄的645协议还是支持块抄的97版645协议
//返回:1为支持块抄的97版645协议,2为只支持单抄的645协议.3为其他协议
BYTE IsSIDV97Mtr(WORD wPn)
{
	BYTE bBuf[F10_LEN_PER_PN+10];
	BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return 0;

	if (bProp != PN_PROP_METER)
		return 0;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return 0;	

	if (bBuf[MTR_PRO_OFFSET] == PROTOCOLNO_DLT645)
		return 1;
	else if (bBuf[MTR_PRO_OFFSET] == PROTOCOLNO_DLT645_SID)
		return 2;

	return 0;	
}

//描述:取得交采的测量点号
WORD GetAcPn()
{
	BYTE bProp = 0;
	WORD wPn;
	for (wPn=1; wPn<PN_NUM; wPn++)
	{		
		ReadItemEx(BN0, wPn, 0x8901, &bProp);
		if (bProp == PN_PROP_AC)
			return wPn;
	}

	return PN0;
}

//描述:取得测量点的接线方式
BYTE GetConnectType(WORD wPn)
{
	BYTE bBuf[16];

	ReadItemEx(BN0, wPn, 0x8910, bBuf);
	if (bBuf[0] == 3)		 //三项三线
		return CONNECT_3P3W; //终端接线方式 1	1:单相;3:三项三线;4:三相四线
	else if (bBuf[0] == 1) 	 //单相表
		return CONNECT_1P;
	else //三相四线
		return CONNECT_3P4W; 
}

/*
告警判断屏蔽字，其二进制值从
最低位0到最高位255分别对应
报警编号E2000001H~E20000FFH
的报警，0屏蔽，1不屏蔽。
*/
bool IsAlrEnable(DWORD dwAlrID)
{
	BYTE bAlrMask[32];
	WORD n = (WORD)(dwAlrID - 0xe2000001);

	if (dwAlrID<0xe200001 || dwAlrID>0xe200004e)
		return false;

	//ReadItem(0xE0000137, bAlrMask);
	ReadItemEx(BN0, PN0, 0x8034, bAlrMask);

	if (bAlrMask[n/8] & (1<<n%8))   //1不屏蔽
	{
// 		DTRACE(DB_TASK, ("%s : alr 0x%04x task enable.\r\n", __FUNCTION__, dwAlrID));
		return true;
	}
	else
	{
// 		DTRACE(DB_TASK, ("%s : alr 0x%04x task disable.\r\n", __FUNCTION__, dwAlrID));
		return false;
	}
}

bool IsCctPn(WORD wPn)
{

	BYTE bProp = GetPnProp(wPn); 
	if(bProp == PN_PROP_METER)
	{
		BYTE bPort = 0;
		if(ReadItemEx(BN0,wPn,0x890a,&bPort)>0)
		{
			if((bPort == PORT_CCT_PLC) || 
				(bPort == PORT_CCT_WIRELESS))
			{
				return true;
			}
		}
	}
	return false;
}
