/*********************************************************************************************************
 * Copyright (c) 2005,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件对交流采样的公共变量、常量、函数进行定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2005年11月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/

#include "AcSample.h"
#include "Sample.h"
#include "AcFmt.h"
#include "DbAPI.h"
#include "sysdebug.h"
#include "FaConst.h"

bool g_fDcSample;
//BYTE g_bDcSampleFlag;

//描述：初始化交流采样
bool InitAcSample()
{
	//memset(g_sSampleBuf, 0, sizeof(g_sSampleBuf));  	//无功计算可能用到之前还没有的电压样点,所以先清除	
	AcSampleInit(PN0);
	//g_fStartAdCheck = false;    //启动AD通道错乱检测		
	return true;
}

//描述：初始化直流采样
bool InitDcSample()
{
    //BYTE bBuf[4];
	//ReadItemEx(BN0, POINT0, 0x8980, bBuf);
	//if (bBuf[0] != 0)
	//{
		//ReadItemEx(BN0, POINT0, 0x8981, &g_bDcSampleFlag);
		g_fDcSample = true;
		DCSmpleInit();

		return true;
	//}
	//else
	//{
		//g_fDcSample = false;
		//g_bDcSampleFlag = 0;
		//return false;
	//}
}

TCalibCfg g_tCalibCfg;

//描述:根据当前AD值获取校准样点
//返回:查找到完全相等的点时返回0,样点通过nAD1和nVal1返回
//     没有完全相等点时返回1,相邻的样点通过nAD1、nVal1和nAD2、nVal2返回
//     失败返回负数
//备注:校准时要求从小到大进行,因此系统的校准配置g_tCalibCfg.tCalibTab是按升序
//排列的
int GetCalibPt(BYTE bType, DWORD dwCurAD, DWORD* dwAD1, DWORD* dwVal1, DWORD* dwAD2, DWORD* dwVal2)
{
    BYTE  bLow = 0;
    if (g_tCalibCfg.bCalibPtNum[bType] < 1)  
        return -1;
    BYTE  bHi = g_tCalibCfg.bCalibPtNum[bType] - 1;
    
    if (g_tCalibCfg.bCalibPtNum[bType] == 1)//单点校准时，算上原点
    {
        *dwAD1 = 0;
        *dwVal1 = 0;
        
        *dwAD2 = g_tCalibCfg.tCalibTab[bType][bHi].wAD;
        *dwVal2 = g_tCalibCfg.tCalibTab[bType][bHi].wVal;
        return 1;
    }
    
    if (g_tCalibCfg.tCalibTab[bType][bLow].wAD > dwCurAD)  
    {
        *dwAD1 = g_tCalibCfg.tCalibTab[bType][bLow].wAD;
        *dwVal1 = g_tCalibCfg.tCalibTab[bType][bLow].wVal;
        
        *dwAD2 = g_tCalibCfg.tCalibTab[bType][bLow+1].wAD;
        *dwVal2 = g_tCalibCfg.tCalibTab[bType][bLow+1].wVal;
        return 1;
    }
    
    if (g_tCalibCfg.tCalibTab[bType][bHi].wAD < dwCurAD)
    {
        *dwAD1 = g_tCalibCfg.tCalibTab[bType][bHi-1].wAD;
        *dwVal1 = g_tCalibCfg.tCalibTab[bType][bHi-1].wVal;
        
        *dwAD2 = g_tCalibCfg.tCalibTab[bType][bHi].wAD;
        *dwVal2 = g_tCalibCfg.tCalibTab[bType][bHi].wVal;
        return 1;
    } 
    
    while(bLow < bHi)
    {
        if (dwCurAD == g_tCalibCfg.tCalibTab[bType][bLow].wAD)
        {//刚好相等
            *dwAD1 = g_tCalibCfg.tCalibTab[bType][bLow].wAD;
            *dwVal1 = g_tCalibCfg.tCalibTab[bType][bLow].wVal;
            return 0;
        }
        else if (dwCurAD == g_tCalibCfg.tCalibTab[bType][bLow+1].wAD)
        {//刚好相等            
            *dwAD1 = g_tCalibCfg.tCalibTab[bType][bLow+1].wAD;
            *dwVal1 = g_tCalibCfg.tCalibTab[bType][bLow+1].wVal;
            return 0;
        }
        else if (dwCurAD > g_tCalibCfg.tCalibTab[bType][bLow].wAD && dwCurAD < g_tCalibCfg.tCalibTab[bType][bLow+1].wAD)
        {
            *dwAD1 = g_tCalibCfg.tCalibTab[bType][bLow].wAD;
            *dwVal1 = g_tCalibCfg.tCalibTab[bType][bLow].wVal;
            *dwAD2 = g_tCalibCfg.tCalibTab[bType][bLow+1].wAD;
            *dwVal2 = g_tCalibCfg.tCalibTab[bType][bLow+1].wVal;
            return 1;
        }
        
        bLow++;
    }
    
    return -1;  //unkown error   
}

//描述:将AD值转换成实际的线路上电流值,用校准结果修正采样结果
//返回:异常情况返回负数
long ConvertToLineVal(BYTE bType, long lAdVal)
{
    DWORD dwAD1, dwAD2;
    DWORD dwVal1, dwVal2;
    long lVal = 0;
    int iRet = GetCalibPt(bType, (DWORD)lAdVal, &dwAD1, &dwVal1, &dwAD2, &dwVal2);
    
    if (iRet==0)    //AD值和校准点刚好相同
        return dwVal1;
    
    //AD值和实际电流值的关系应该是单调的,正常不会出现这种情况,这里以防万一
    if (dwAD2 == dwAD1)  
        return -1;
    
    if (iRet > 0)
    {
        //两点式直线方程 (Y-Y1)/(Y2-Y1) = (X-X1)/(X2-X1)        
        lVal = (lAdVal - (long)dwAD1)*(long)(dwVal2 - dwVal1) / (long)(dwAD2 - dwAD1) + (long)dwVal1;
        if (lVal < 0)
            lVal = 0;
        return lVal;
    }
    
    return -1;     
}

//------------以下校准方式适合交流/直流校准,但交流校准的是有效值--------------
//返回0-OK
int OnCalibrate(BYTE* pbBuf)
{
    BYTE* pbRx = pbBuf;    
    BYTE bStep = *pbRx++;    //校准步骤
    
    BYTE bBuf[16];
    BYTE bBuf2[4];    
    BYTE i;
	//WORD wUn;

    //for (i=0; i<4; i++)  //CL818K5采样数据放在第5通道
        //pbRx += 4;
    
    for (i=0; i<SCN_NUM+DC_CHANNEL_NUM; i++)  //需要校准的通道数,如果直流需要校准，也应加上
    {
        //从数据库读取当前的采样原始值
        if (i < SCN_NUM)  //交流
        {
            ReadItemEx(BN2, PN0, 0xba11, bBuf);  //未校准的有效值   
            memcpy(bBuf2, &bBuf[i<<1], 2);
            memcpy(&bBuf2[2], pbRx, 2);           //校点值
            pbRx += 4;
			/*if (bStep == 1)
			{
				wUn = 21000;
				memcpy(&bBuf2[2], (BYTE*)&wUn, 2);
			}
			else
			{
				wUn = 22000;
				memcpy(&bBuf2[2], (BYTE*)&wUn, 2);
			}*/
			WriteItemEx(BN25, PN0, 0x1003+(i<<12)+bStep-1, bBuf2);
        }
        else                  //直流
        {
            ReadItemEx(BN2, PN0, 0xba20, bBuf);  //bBuf前两字节是AD原始值或有效值 
            memcpy(bBuf2, &bBuf[(i-SCN_NUM)<<1], 2);
            memcpy(&bBuf2[2], pbRx, 2);
            pbRx += 4;
            WriteItemEx(BN25, PN0, 0x1003+(i<<12)+bStep-1, bBuf2); //todo:
        }
        
        WriteItemEx(BN25, PN0, 0x1002+(i<<12), &bStep);//保存新的校准样点数
    }
    
    LoadCalibCfg();  //发送一个消息
    
    return ERR_APP_OK;
}

//描述:初始化系统交采校准配置
void LoadCalibCfg(void)
{
    BYTE bBuf[6];
    BYTE bCalibPtNum = 0;
    
    BYTE i;
    BYTE j;
    memset(&g_tCalibCfg, 0, sizeof(g_tCalibCfg));
    for (i=0; i<SCN_NUM+DC_CHANNEL_NUM; i++)
    {         
        ReadItemEx(BN25, PN0, 0x1002+(i<<12), &bCalibPtNum);
        if (bCalibPtNum > MAX_CALIB_PT_NUM) //点数不正确
            continue;
        g_tCalibCfg.bCalibPtNum[i] = bCalibPtNum;
        
        for(j=0; j<bCalibPtNum; j++)
        {            
            ReadItemEx(BN25, PN0, 0x1003+(i<<12)+j, bBuf); 
            g_tCalibCfg.tCalibTab[i][j].wAD = ByteToDWORD(bBuf, 2);
            g_tCalibCfg.tCalibTab[i][j].wVal = ByteToDWORD(&bBuf[2], 2); 
        }
    }        
}

//检测是否交采校准了
bool IsAcCalib(void)
{
    if ((g_tCalibCfg.bCalibPtNum[1]>0) && 
        (g_tCalibCfg.bCalibPtNum[1]<MAX_CALIB_PT_NUM))
        return true;
    
    return false;
}
