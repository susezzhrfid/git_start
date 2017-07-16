/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbHook.c
 * 摘    要：本文件主要用来定义系统库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *********************************************************************************************************/
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "DataManager.h"
#include "ComAPI.h"
#include "SysDebug.h"
#include "MtrAPI.h"
#include "M590.h"
#include "AutoReader.h"
#include "FaAPI.h"
//广州有部分电表取不到瞬时功率，需要通过脉冲或者15分钟的电能来计算
//这个标志要来识别是否需要取替代ID作为电表功率(bit0-7代表测量点1-8)
//	BYTE g_bRedirectPwrID = 0; 

#define CMB_TO_SUB_ID_MAX	16
static const WORD g_wCmbToSubID[][CMB_TO_SUB_ID_MAX] =		//国标BANK0内部ID到645ID的映射
{
//	{0x126f, 0xb66f, 0}, //相位角

	//日月冻结需量取当前
	//正有
	{0x20cf, 0x8908, 0xa010, 0xb010, 0xa011, 0xb011, 0xa012, 0xb012, 0xa013, 0xb013, 0xa014, 0xb014, 0},
	//反有
	{0x20df, 0x8908, 0xa020, 0xb020, 0xa021, 0xb021, 0xa022, 0xb022, 0xa023, 0xb023, 0xa024, 0xb024, 0},
	//正无
	{0x20ef, 0x8908, 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 0},
	//反无
	{0x210f, 0x8908, 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0},


	//日月冻结取当前数据
	{0x166f, 0x8908, 0x9010, 0x9011, 0x9012, 0x9013, 0x9014, 0}, 	//当前正向有功
	{0x167f, 0x8908, 0x911f, 0}, 	//当前正向无功	 
	{0x168f, 0x8908, 0x902f, 0}, 	//当前反向有功
	{0x169f, 0x8908, 0x912f, 0}, 	//当前反向无功
	{0x16af, 0x8908, 0x913f, 0},	//当前一象限无功
	{0x16bf, 0x8908, 0x915f, 0},	//当前二象限无功
	{0x16cf, 0x8908, 0x916f, 0},	//当前三象限无功
	{0x16df, 0x8908, 0x914f, 0},	//当前四象限无功

	//日冻结取上1日数据
	{0x3761, 0x8908, 0x9a10, 0x9a11, 0x9a12, 0x9a13, 0x9a14, 0},	//上1日正向有功
	{0x3762, 0x8908, 0x9b1f, 0},	//上1日正向无功	 
	{0x3763, 0x8908, 0x9a2f, 0},	//上1日反向有功
	{0x3764, 0x8908, 0x9b2f, 0},	//上1日反向无功
	{0x3765, 0x8908, 0x9b3f, 0},	//上1日一象限无功
	{0x3766, 0x8908, 0x9b5f, 0},	//上1日二象限无功
	{0x3767, 0x8908, 0x9b6f, 0},	//上1日三象限无功
	{0x3768, 0x8908, 0x9b4f, 0}, 	//上1日四象限无功


	//月冻结取上1日数据
	{0x3777, 0x8908, 0x9a10, 0x9a11, 0x9a12, 0x9a13, 0x9a14, 0},	//上1日正向有功
	{0x3778, 0x8908, 0x9b1f, 0},	//上1日正向无功  
	{0x3779, 0x8908, 0x9a2f, 0},	//上1日反向有功
	{0x3780, 0x8908, 0x9b2f, 0},	//上1日反向无功
	{0x3781, 0x8908, 0x9b3f, 0},	//上1日一象限无功
	{0x3782, 0x8908, 0x9b5f, 0},	//上1日二象限无功
	{0x3783, 0x8908, 0x9b6f, 0},	//上1日三象限无功
	{0x3784, 0x8908, 0x9b4f, 0},	//上1日四象限无功

	//日冻结需量取上1日数据
	{0x3785, 0x8908, 0x9c00, 0x9c80, 0x9c01, 0x9c81, 0x9c02, 0x9c82, 0x9c03, 0x9c83, 0x9c04, 0x9c84, 0},	//上1日正向有功最大需量及发生时间（总、费率1~M）
	{0x3786, 0x8908, 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 0},	//上1日正向无功最大需量及发生时间（总、费率1~M）
	{0x3787, 0x8908, 0x9c20, 0x9ca0, 0x9c21, 0x9ca1, 0x9c22, 0x9ca2, 0x9c23, 0x9ca3, 0x9c24, 0x9ca4, 0},	//上1日反向有功最大需量及发生时间（总、费率1~M）
	{0x3788, 0x8908, 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0},	//上1日反向无功最大需量及发生时间（总、费率1~M）


	//月冻结需量取结算日数据
	{0x3789, 0x8908, 0xa410, 0xb410, 0xa411, 0xb411, 0xa412, 0xb412, 0xa413, 0xb413, 0xa414, 0xb414, 0},	//结算日正向有功最大需量及发生时间（总、费率1~M）
	{0x3790, 0x8908, 0xa510, 0xb510, 0xa511, 0xb511, 0xa512, 0xb512, 0xa513, 0xb513, 0xa514, 0xb514, 0},	//结算日正向无功最大需量及发生时间（总、费率1~M）
	{0x3791, 0x8908, 0xa420, 0xb420, 0xa421, 0xb421, 0xa422, 0xb422, 0xa423, 0xb423, 0xa424, 0xb424, 0},	//结算日反向有功最大需量及发生时间（总、费率1~M）
	{0x3792, 0x8908, 0xa520, 0xb520, 0xa521, 0xb521, 0xa522, 0xb522, 0xa523, 0xb523, 0xa524, 0xb524, 0},	//结算日反向无功最大需量及发生时间（总、费率1~M）
	//月冻结需量取上1日数据
	{0x3793, 0x8908, 0x9c00, 0x9c80, 0x9c01, 0x9c81, 0x9c02, 0x9c82, 0x9c03, 0x9c83, 0x9c04, 0x9c84, 0},	//上1日正向有功最大需量及发生时间（总、费率1~M）
	{0x3794, 0x8908, 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 0},	//上1日正向无功最大需量及发生时间（总、费率1~M）
	{0x3795, 0x8908, 0x9c20, 0x9ca0, 0x9c21, 0x9ca1, 0x9c22, 0x9ca2, 0x9c23, 0x9ca3, 0x9c24, 0x9ca4, 0},	//上1日反向有功最大需量及发生时间（总、费率1~M）
	{0x3796, 0x8908, 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0},	//上1日反向无功最大需量及发生时间（总、费率1~M）


	//月冻结取结算日数据
	{0x3877, 0x8908, 0x941f, 0},	//结算日正向有功
	{0x3878, 0x8908, 0x951f, 0},	//结算日正向无功 
	{0x3879, 0x8908, 0x942f, 0},	//结算日反向有功
	{0x3880, 0x8908, 0x952f, 0},	//结算日反向无功
	{0x3881, 0x8908, 0x953f, 0},	//结算日一象限无功
	{0x3882, 0x8908, 0x955f, 0},	//结算日二象限无功
	{0x3883, 0x8908, 0x956f, 0},	//结算日三象限无功
	{0x3884, 0x8908, 0x954f, 0},	//结算日四象限无功





	{0x8e70, 0xb6a0, 0}, //零序电流
};

//需量时间ID检测
bool IsDemandTimeId(WORD wID)
{
	if (wID>=0xe910 && wID<=0xe930)//当前A/B/C相正向有功最大需量发生时间
		return true;
	if (wID>=0xe911 && wID<=0xe931)//当前A/B/C相反向有功最大需量发生时间
		return true;
	if (wID>=0xe912 && wID<=0xe932)//当前A/B/C相组合无功1最大需量发生时间
		return true;
	if (wID>=0xe913 && wID<=0xe933)//当前A/B/C相组合无功2最大需量发生时间
		return true;
	if (wID>=0xe914 && wID<=0xe934)//当前A/B/C相第一象限无功最大需量发生时间
		return true;
	if (wID>=0xe915 && wID<=0xe935)//当前A/B/C相第二象限无功最大需量发生时间
		return true;
	if (wID>=0xe916 && wID<=0xe936)//当前A/B/C相第三象限无功最大需量发生时间
		return true;
	if (wID>=0xe917 && wID<=0xe937)//当前A/B/C相第四象限无功最大需量发生时间
		return true;
	if (wID>=0xe918 && wID<=0xe938)//当前A/B/C相正向视在最大需量发生时间
		return true;
	if (wID>=0xe919 && wID<=0xe939)//当前A/B/C相反向视在最大需量发生时间
		return true;

	if (wID>=0xec10 && wID<=0xec30)//月冻结A/B/C相正向有功最大需量发生时间
		return true;
	if (wID>=0xec11 && wID<=0xec31)//月冻结A/B/C相反向有功最大需量发生时间
		return true;
	if (wID>=0xec12 && wID<=0xec32)//月冻结A/B/C相组合无功1最大需量发生时间
		return true;
	if (wID>=0xec13 && wID<=0xec33)//月冻结A/B/C相组合无功2最大需量发生时间
		return true;
	if (wID>=0xec14 && wID<=0xec34)//月冻结A/B/C相第一象限无功最大需量发生时间
		return true;
	if (wID>=0xec15 && wID<=0xec35)//月冻结A/B/C相第二象限无功最大需量发生时间
		return true;
	if (wID>=0xec16 && wID<=0xec36)//月冻结前A/B/C相第三象限无功最大需量发生时间
		return true;
	if (wID>=0xec17 && wID<=0xec37)//月冻结A/B/C相第四象限无功最大需量发生时间
		return true;
	if (wID>=0xec18 && wID<=0xec38)//月冻结A/B/C相正向视在最大需量发生时间
		return true;
	if (wID>=0xec19 && wID<=0xec39)//月冻结A/B/C相反向视在最大需量发生时间
		return true;

	return false;
}

//为需量时间添加年
bool AddDemandTimeYear(BYTE* pbBuf)
{
	TTime tmTimeNow;
	GetCurTime(&tmTimeNow);

	if(pbBuf==NULL)
		return false;

	if(0==BcdToDWORD(pbBuf,4))
	{
		memset(pbBuf,0,5);
		return false;
	}

	if(BcdToByte(pbBuf[3])<tmTimeNow.nMonth)
		pbBuf[4] = ByteToBcd(tmTimeNow.nYear%100-1);
	else
		pbBuf[4] = ByteToBcd(tmTimeNow.nYear%100);

	return true;
}

//描述:非直流模拟量测量点是否有效
bool IsPnValid(WORD wPn)
{
	int iRet;
	BYTE bBuf[4];

	if (wPn == PN0)
		return true;

	iRet = ReadItemEx(BN0, wPn, 0x8900, bBuf);
	if (iRet <= 0)
		return false;

	return (bBuf[0] != 0x00);
}


//描述:取组合ID到子ID的映射数组
const WORD* CmbToSubID(WORD wBn, WORD wID)
{
	WORD i, wNum;
	if (wBn != BN0)
		return NULL;

	//集抄BANK和BANK0用同样的组合ID规则
	wNum = sizeof(g_wCmbToSubID) / (sizeof(WORD)*CMB_TO_SUB_ID_MAX);
	for (i=0; i<wNum; i++)
	{
		if (wID == g_wCmbToSubID[i][0])
		{
			return &g_wCmbToSubID[i][1];
		}
	}
	
	return NULL;
}


//描述:取组合ID到子ID的个数
WORD CmbToSubIdNum(WORD wBn, WORD wID)
{
	const WORD* pwSubID = CmbToSubID(wBn, wID);
	WORD wNum;

	if (pwSubID == NULL)
		return 1;
	
	wNum = 0;	
	while (*pwSubID++ != 0)	//把组合ID转换成依次对子ID的读
		wNum++;
	
	return wNum;
}


bool IsSP_TDMeter(BYTE bMain, BYTE bSub)
{
	if (bMain == MAINTYPE_CLASSD)
	{
		if (bSub==4 || bSub==5)
			return true;
		else
			return false;
	}
	else if (bMain == MAINTYPE_CLASSE)
	{
		if (bSub==14 || bSub==15)
			return true;
		else
			return false;
	}
	else
		return false;
}

//#ifndef SYS_WIN
//__no_init char g_szBootloader[32] @ 0x20001000;
//#endif

extern void ResetMtrUnsupIdFlg(WORD wPN);
//描述:写数据项的挂钩,在数据项被写后自动调用,进行一些非标准的操作
//参数:@nRet 在调用WriteItemEx()时的返回值,应该为正数
int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet)
{
	BYTE bBuf[10];
	DWORD dwSeconds=0, dwTimes = 0;

#ifdef SYS_WIN
	SYSTEMTIME tm;
	DWORD dwErr = 0;
#endif

	if (wBank==BN0 && wID==0x8030) //终端时间
	{
		memset(bBuf, 0,sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0xc850, bBuf);
		dwTimes = BcdToDWORD(bBuf, 3);
		dwTimes++;
		DWORDToBCD(dwTimes, bBuf, 3);
		WriteItemEx(BN0, PN0, 0xc850, bBuf);

	    //if (!IsPowerOff())
	    {//停电的时候不允许更改终端时间
		    TTime time;
		    time.nSecond = BcdToByte(pbBuf[0]);
		    time.nMinute = BcdToByte(pbBuf[1]);
		    time.nHour = BcdToByte(pbBuf[2]);
		    time.nDay = BcdToByte(pbBuf[3]);
		    time.nMonth = BcdToByte(pbBuf[4]);
		    time.nYear = 2000 + BcdToByte(pbBuf[5]);

		    dwSeconds = TimeToSeconds(&time);
		    dwSeconds += 2;
		    SecondsToTime(dwSeconds, &time);

		    time.nWeek = DayOfWeek(&time);

#ifdef SYS_WIN			
			memset(&tm,0,sizeof(tm));

			tm.wYear = time.nYear;
			tm.wMonth = time.nMonth;
			tm.wDay = time.nDay;
			tm.wHour = time.nHour;
			tm.wMinute = time.nMinute;
			tm.wSecond = time.nSecond;
			tm.wDayOfWeek = DayOfWeek(&time) - 1;
			
			if (!SetLocalTime(&tm))	
			{
				nRet = -(ERR_INVALID + nRet*0x100); 
			}
#else
    		if (!SetSysTime(&time))
	    		nRet = -(ERR_INVALID + nRet*0x100); 
#endif
	    	/*if (nRet >= 0)//如果对时过了冻结时间需要进行补数据冻结
			{
	    		//FrzAcData();

				DTRACE(DB_FAPROTO, ("PostWriteItemExHook: set system time ok!\r\n"));
			}*/


	   }
	  /* else	 
	   {
		   DTRACE(DB_FAPROTO, ("PostWriteItemExHook: due to terminal power off!\r\n"));
	        nRet = -(ERR_PERM + nRet*0x100);
	   }*/
	}
	else if (wBank==BN0 && wID==0x8032) //终端复位
	{
		if(*pbBuf == 0x00)
		{
			SetInfo(INFO_RST_PARA);
		}
		else if (*pbBuf == 0x01)
		{
			//SetInfo(INFO_RST_DATA);
			{
				DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_DATA at Click=%d......\n", GetClick()));
				StopMtrRd(0);
				FaResetData();		//清除测量点中间数据
				UnLockDB();
				ResetPowerOffTmp();
				SavePoweroffTmp(false);
				SetInfo(INFO_TASK_PARA);    //g_fUpdFatOk = UpdFat();		//更新Flash动态分配，一定要放在InitDB之后
                Sleep(2000);                
				//ResetCPU();
			}
		}
		else if (*pbBuf == 0x02)
		{
			SetInfo(INFO_APP_RST);
		}
	}
	else if (wBank==BN0 && wPn==PN0 &&wID==0x8f04) //清需量
	{
		if (*pbBuf == 0x00)
			SetInfo(INFO_CLR_DEMAND);
	}
	else if(wBank==BN0 && wID==0x8040) //如果是配变则为 结算日， 负控则为功控投入控制
	{
		/*BYTE bBuf[4];
		ReadItemEx(BN3, PN0, 0x3200, bBuf);
		if (bBuf[0] != 0)
		{
			nRet = WriteItemEx(BN0, PN0, 0x806e, bBuf);
		}*/
	}
	else if (wBank==BN0 && (wID&0xfff0)==0x8900)//(wID==0x890f || wID==0x8902))
	{
		if(wID==0x890f || wID==0x8902)
		{
#ifdef EN_CCT
			SetInfo(INFO_PLC_PARA);
			g_fCctInitOk = false;
#endif
		}
		DoMtrRdStat();
	}
	else if (wBank==BN0 && wPn==PN0 && wID==0x80a1)
	{
		if (*pbBuf == 0x00)
		{
			bBuf[0] = 0x01;
			WriteItemEx(BN2, PN0, 0x2042, bBuf);
		}
		else
		{
			bBuf[0] = 0x00;
			WriteItemEx(BN2, PN0, 0x2042, bBuf);
		}
	}

	return nRet;
}

//描述:读数据项的挂钩,在数据项被读后自动调用,进行一些非标准的操作
//参数:@nRet 在调用ReadItemEx()时的返回值,应该为正数
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet)
{	
	if (wBank==BN0 && wID==0x8030) //终端时间
	{
		TTime tmNow;
		GetCurTime(&tmNow);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nSecond);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nMinute);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nHour);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nDay);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nMonth);
		*pbBuf++ = ByteToBcd((BYTE)((tmNow.nYear - 2000)&0xff));
		return 6;
	}
	else if (wBank==BN2 && wID==0x1056)
	{
#ifndef SYS_WIN
		DWORD dwIP = GetLocalAddr("ppp0");
		*pbBuf++ = (dwIP>>24)&0xff;
		*pbBuf++ = (dwIP>>16)&0xff;
		*pbBuf++ = (dwIP>>8)&0xff;
		*pbBuf++ = (dwIP>>0)&0xff;
#endif
		return 4;
	}
    else if (wBank==BN0 && wID==0x8040) 
    {
       /* BYTE bBuf[4];
        ReadItemEx(BN3, PN0, 0x3200, bBuf);
        if (bBuf[0] != 0)
        {
            nRet = ReadItemEx(BN0, PN0, 0x806e, pbBuf);
        }*/
    }
    /*else if (wBank==BN0 && (wID&0xfff0)==0xb630)
    {
    	if (wPn>0 && wPn<PN_NUM)
    	{
	    	if ((g_bRedirectPwrID & (1<<(wPn-1))) != 0)
	    	{
	    		if (wID == 0xb630)	    
	    		{		
	    			nRet = ReadItemEx(BN11, wPn, 0x0221, pbBuf);
	    		}
	    		else if (wID == 0xb63f)
	    		{
	    			memset(pbBuf, 0xff, 12);
	    			nRet = ReadItemEx(BN11, wPn, 0x0221, pbBuf);
	    			return 12;
	    		}
	    		else
	    		{	
	    			memset(pbBuf, 0xff, 3);
	    			return 3;
	    		}
	    	}
    	}
    }
    else if (wBank==BN0 && wID==0x8e3f)
    {
    	if (wPn>0 && wPn<PN_NUM)
    	{    		
	    	if ((g_bRedirectPwrID & (1<<(wPn-1))) != 0)
	    	{
	    		if (wID == 0x8e30)
	    		{
	    			nRet = ReadItemEx(BN11, wPn, 0x0222, pbBuf);
	    		}
	    		else if (wID == 0x8e3f)
	    		{
		    		memset(pbBuf, 0xff, 16);
		    		ReadItemEx(BN11, wPn, 0x0222, pbBuf);
		    		return 16;
	    		}
	    		else
	    		{
	    			memset(pbBuf, 0xff, 4);
	    			return 4;	    			
	    		}
	    	}
	    }
    }*/
	//20140225-2
	/*else if (wBank==BN0 && (wID==0xb612 || wID==0xb61f) && wPn==GetAcPn() && GetConnectType(wPn)==CONNECT_3P3W)  //交采三相三线终端B相电压给零
	{
		if (wID == 0xB612)
		{
			memset(pbBuf, 0, 2);
		}
		else
		{
			memset(pbBuf+2, 0, 2);
		}
	}
	else if (wBank==BN0 && (wID==0xb622 || wID==0xb62f) && wPn==GetAcPn() && GetConnectType(wPn)==CONNECT_3P3W)
	{
		if (wID == 0xB622)
		{
			memset(pbBuf, 0, 3);
		}
		else
		{
			memset(pbBuf+3, 0, 3);
		}
	}*/
	else if(wBank==BN0 && IsNorSuportId(wID))
	{
		nRet = GetItemLen(wBank,wID);
		memset(pbBuf, 0xff, nRet);
	}
	else if(wBank==BN0 && IsDemandTimeId(wID))
	{
// 		TraceBuf(DB_CRITICAL, "************* PostReadItemExHook: time buf is   ", pbBuf, 5);
		AddDemandTimeYear(pbBuf);
		nRet = GetItemLen(wBank,wID);
	}
    else if (wBank==BN0 && wID==0x8875)
    {
        nRet = ReadItemEx(BN2, PN0, 0x1044, pbBuf);
    }
	else if (wBank==BN0 && wID==0x8863)
	{
		DWORDToBCD(PN_NUM, pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8864)
	{
		DWORDToBCD(GetMtrNum(), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8865)
	{
		DWORDToBCD(GetMtrClassNum(0x8906, 0, 0), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8866)
	{
		DWORDToBCD(GetMtrClassNum(0x8904, 0, 2), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8867)
	{
		DWORDToBCD(GetMtrClassNum(0x8906, 0, 1), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8874)
	{
		GetVitalPn(pbBuf);
		nRet = 40;
	}
	
	return nRet;
}


//描述:读组合数据项的挂钩,在数据项被读后自动调用,进行一些非标准的操作
//参数:@nRet 在调用ReadItemEx()时的返回值,应该为正数
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet)
{	
	const WORD* pwSubID = CmbToSubID(wBank, wID);	
	if (pwSubID == NULL)
		return nRet;

	//目前都把终端抄表时间直接替换为读函数带下来的时间
	if (*pwSubID==0x8910)
	{
		TTime time;
		if (dwTime == 0)
		{
			GetCurTime(&time);
		}
		else
		{
			SecondsToTime(dwTime, &time);
		}
		TimeToFmt15(&time, pbBuf);
	}
	else if (*pwSubID==0x8912)
	{
		TTime time;
		if (dwTime == 0)
		{
			GetCurTime(&time);
		}
		else
		{
			SecondsToTime(dwTime, &time);
		}
		TimeToFmt1(&time, pbBuf);
	}

	return nRet;
}

bool PswCheck(BYTE bPerm, BYTE* pbPassword)
{
	return true;
}


//描述:获取本系统的无效数据的定义
BYTE GetDbInvalidData()
{
	return INVALID_DATA;
}

//描述:获取本系统的无效数据的定义
BYTE GetInvalidData(BYTE bErr)
{
	if (bErr == ERR_FAIL)	//尝试失败,比如抄表失败等
		return 0xef;	//江苏版本要区分不支持数据项和抄表失败数据项
	else
		return INVALID_DATA;
}

//描述:是否是无效数据，无效数据可能存在多种定义
bool IsInvalidData(BYTE* p, WORD wLen)
{
	if (IsAllAByte(p, INVALID_DATA, wLen))
		return true;

	//if (IsAllAByte(p, 0xef, wLen))
	//	return true;

	return false;
}


//描述：获取本系统曲线的冻结间隔
BYTE GetCurveInterv()
{
	return TIME_UNIT_HOUR;
}
