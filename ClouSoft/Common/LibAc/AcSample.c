/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件对73360采集到的样点进行计算,算出有效值,功率,功率因数,频率,电能,相角等
 * 当前版本：见本文VER_STR
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 ---------------------------------------------------------------------------------------------------------
* 版本信息:
 ---2009-4-16:---V1.04----岑坚宇---
*********************************************************************************************************/
#include "Sample.h"
#include "AcSample.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "AcFmt.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "AcHook.h"
#include "SysDebug.h"
#include "AD.h"
#include "DbAPI.h"

#define VER_STR	"Ver1.07"

#define ANGLE(x) ((float)x / (10*180.0) * 3.1415926)
#define PI	3.1415927

#define PRINTF_AD_DATA     0 //AD 原始值, 完整周波     ,打印来不及
#define PRINTF_AD_ONEPOINT 0 //AD 原始值，每周波只打一点
#define PRINTF_IEVALUE     0 //AD FFT后未校准的有效值
#define PRINTF_FREQ        0 //频率跟踪捕获的信号频率

#define FFT                0 //1-FFT，0-不使用FFT算法,使用均方根算法

#define CHANNLE_NONE	0//无效通道
#define UA	1//A相电压
#define UB	2//B相电压
#define UC	3//C相电压
#define IA	4//A相电流
#define IB	5//B相电流
#define IC	6//C相电流
#define U0	7//C相电流
#define I0	8//零序电流
#define IAP	9//A相保护电流
#define IBP	10//A相保护电流
#define ICP	11//A相保护电流

#define CHANNEL0	0
#define CHANNEL1	1
#define CHANNEL2	2
#define CHANNEL3	3
#define CHANNEL4	4
#define CHANNEL5	5
#define CHANNEL6	6
#define CHANNEL7	7
#define CHANNEL8	8
#define CHANNEL9	9
#define CHANNEL10	10
#define CHANNEL11	11
#define CHANNEL12	12
#define CHANNEL13	13
#define CHANNEL14	14
#define CHANNEL15	15
#define CHANNEL_NULL  16//无效，没有使用

#define MAX_CHANNEL_NUM		16

//事件表示的顺序
#define 	EVENT_FLAG_3_SHORT				0//三相短路
#define 	EVENT_FLAG_2_SHORT				1//2相短路
#define 	EVENT_FLAG_2_SHORT_GROUND		2//2相接地短路
#define 	EVENT_FLAG_1_SHORT_GROUND		3//单相接地短路
#define 	EVENT_FLAG_I0_OVER				4//零序电流过流
#define 	EVENT_FLAG_U0_OVER				5//零序电压过流
#define 	EVENT_FLAG_PHASE_OFF			6//断线故障

#define 	TWO_SHORT					0
#define 	TWO_SHORT_RECOVER			1
#define 	TWO_SHORT_GROUD				2
#define 	TWO_SHORT_GROUD_RECOVER		3
#define 	THREE_SHORT					4
#define 	THREE_SHORT_RECOVER			5
#define 	ONE_GROUD					6
#define 	ONE_GROUD_RECOVER			7

#define RAW_UA	0//A项电压
#define RAW_UB	1//B项电压
#define RAW_UC	2//C项电压
#define RAW_IA	3//A项电流
#define RAW_IB	4//B项电流
#define RAW_IC	5//C项电流
#define RAW_U0	6//零序电压
#define RAW_I0	7//零序电流

//static DWORD g_dwEventFlag;//事件标识
//#define 	IS_ENVET_OPERATION(x)	((g_dwEventFlag & (0x01<<x)) != 0)
//#define 	IS_ENVET_ID_OPERATION(x)	((g_dwEventFlag & (0x01<<(x-0xc021))) != 0)

//static bool g_fStatus[PN_ALL][PHASE_ALL];//相间短路
//static bool g_fStatus_groud[PN_ALL][PHASE_ALL];//相间接地短路

//static bool g_fStatusABC[PN_ALL];//三相短路
//static bool g_fStatus_one_phase_groud[PN_ALL][PHASE_ALL];//单相单路

//#pragma align   16
//内部成员变量,放在这里定义,避免出现对齐的问题
static complex_fract16 m_cplxFftIn[SCN_NUM][FFT_NUM];  //保留点裕量,增加可靠性 
static complex_fract32 m_cplxFftOut[SCN_NUM][FFT_NUM]; //保留点裕量,增加可靠性 +2零序电流和电压	
//static complex_fract32 m_cplxFftOutUnAdj[SCN_NUM][FFT_NUM]; //保留点裕量,增加可靠性 +2零序电流和电压	

//extern TDataItem g_diHarPercent;
//TSHJPara g_tSHJPara;

//static bool fOccur[PN_ALL][PHASE_ALL] = {false};
//static bool g_fInOccur[PN_ALL] = {false};
//static bool g_fInRec[PN_ALL] = {false};
//static bool g_fUnOccur[PN_ALL] = {false};
//static bool g_fUnRec[PN_ALL] = {false};
//static DWORD g_InOccClick[PN_ALL] = {0};
//static DWORD g_InRecClick[PN_ALL] = {0};
//static DWORD g_UnOccClick[PN_ALL] = {0};
//static DWORD g_UnRecClick[PN_ALL] = {0};

TAcSample g_AcSample;

//static DWORD g_dwTimeStart[PN_ALL][PHASE_ALL] = {0};//发生时间
//static DWORD g_dwTimeRetray[PN_ALL][PHASE_ALL] = {0};//恢复时间

//初始化内部成员,及一些基本参数,如接线方式、校正参数等
//浙江版和国标版的不同参数接口，可以包装在本函数中
bool AcSampleInit(WORD wPn)
{
	//WORD i;
//	BYTE bBuf[64];
	//g_dwEventFlag = 0;

	g_AcSample.wPn = wPn;
	AcLoadPara(g_AcSample.wPn, &g_AcSample.AcPara);
	//ReadItemEx(BN0, POINT0, 0x8970, (BYTE *)&g_dwEventFlag); //todo
	
	//ReadItemEx(BN0, POINT0, 0x8971, (BYTE *)&g_AcSample.iCurrentVal); //todo

	memset(m_cplxFftIn, 0, sizeof(m_cplxFftIn));
	memset(m_cplxFftOut, 0, sizeof(m_cplxFftOut));
	//memset(m_cplxFftOutUnAdj, 0, sizeof(m_cplxFftOutUnAdj));
	//memset(m_cplxFftTmp, 0, sizeof(m_cplxFftTmp));
	memset(g_AcSample.iValue, 0, sizeof(g_AcSample.iValue));
	//memset(g_AcSample.iP, 0, sizeof(g_AcSample.iP));
	//memset(g_AcSample.iQ, 0, sizeof(g_AcSample.iQ));
	//memset(g_AcSample.iCos, 0, sizeof(g_AcSample.iCos));
	//memset(g_AcSample.iAngle, 0, sizeof(g_AcSample.iAngle));

	//memset(g_fStatus, false, sizeof(g_fStatus));
	//memset(g_fStatus_groud, false, sizeof(g_fStatus_groud));

	//memset(g_fStatusABC, false, sizeof(g_fStatusABC));
	//memset(g_fStatus_one_phase_groud, false, sizeof(g_fStatus_one_phase_groud));

    //memset(g_AcSample.dwOffStartTime, 0, sizeof(g_AcSample.dwOffStartTime));
    //memset(g_AcSample.dwOffRecoverTime, 0, sizeof(g_AcSample.dwOffRecoverTime));
    //memset(g_AcSample.fPowerOff, 0, sizeof(g_AcSample.fPowerOff));
    //memset(g_AcSample.fOffRecover, 0, sizeof(g_AcSample.fOffRecover));    

	//g_AcSample.ptSOEPtr = GetItemEx(BN0, PN0, 0xc31f);
	
    //DTRACE(0, ("%s init in mode %d\r\n",__FUNCTION__, g_AcSample.AcPara.wWorkMode));
	//g_AcSample.tChannelCfg.bpChannelPtr = NULL;
	    		
	ResetFreqSync();     
    
	g_AcSample.wFreqCn = 1;   //防止刚上电ResetFreqSync()误选
    
    g_AcSample.wFreqRstCnt = 0;
	
	g_AcSample.diAdjPara = GetItemEx(BN25, PN0, 0x8960);	//校正参数

	//g_AcSample.diAcVect = GetItemEx(BN2, PN0, 0xba10);	//交采基波矢量

	//g_diHarPercent = GetItemEx(BN0, wPn, 0xB7FF); //当前A、B、C三相电压、电流,2~N次谐波含有率//todo

	//for (i=1; i<MAX_PN_NUM; i++) //YC数据块
	//{
		//g_AcSample.diYcBlk[i] = GetItemEx(BN0, i, 0xb6ff); //todo
	//}
    //g_AcSample.diYcBlk[0] = GetItemEx(BN0, PN0, 0xb6ff); //todo

	g_AcSample.fAdjParaSet = true;	//默认从FLASH中读出

    DTRACE(DB_CRITICAL, ("CAcSample::Init: "VER_STR" init ok\n"));

	return true;
}

void LoadPara()
{
	if (GetInfo(INFO_AC_PARA) == false)
		return;	

	WORD wPn = GetAcPn();	//取得交采的测量点号
	if (wPn != g_AcSample.wPn)		//测量点号发生了改变
	{
		g_AcSample.wPn = wPn;
		//InitAcValToDb(wPn);	//初始化交采数据入库的控制结构
	}	

	AcLoadPara(g_AcSample.wPn, &g_AcSample.AcPara);
}

//描述：初始化校正，在每一次重新校正之前调用此接口初始化
void InitAdj()
{
	g_AcSample.fAdjParaSet = false;
    DTRACE(DB_CRITICAL, ("Ac adjust init ok\r\n"));
}

bool TrigerAdj(BYTE* pbBuf)
{
	WriteItemDI(&g_AcSample.diAdjPara, pbBuf);

	g_AcSample.fAdjParaSet = true;
    DTRACE(DB_CRITICAL, ("Ac adjust ok!\r\n"));
	return true;
}

/*
bool IsPowerOff() 
{ 
    return g_AcSample.fPowerOff; 
}   //交流采样检测到掉电了
*/

void UpdateFreq(void)
{    
    DWORD dwFreq;
    int iAddf;
    dwFreq = g_AcSample.dwFreq;    

    //频率
	//默认:m_dwFreqPnts = FREQ_CYC_NUM * NUM_PER_CYC * FREQ_UNIT;		
    if (g_AcSample.dwFreqPnts == 0)
        return;
    
    g_AcSample.dwFreq = ((DDWORD)g_dwAdFreq*FREQ_CYC_NUM*FREQ_UNIT)/g_AcSample.dwFreqPnts;//单位是HZ/1000,即放大了一千倍
                                                      //还有1000没除是为了把频率的小数保留成50.000Hz
#if PRINTF_FREQ
    DTRACE(1, ("%d\r\n", g_AcSample.dwFreq));
#endif		    
    WriteItemEx(BN2, PN0, 0x1054, (BYTE *)&g_AcSample.dwFreq); 
	    
    iAddf = g_AcSample.dwFreq - dwFreq;    
    if (iAddf <= 3 && iAddf >= -3)//精度+-0.003HZ//频率偏差超过一定范围才更新采样间隔，因为更新一次就会有一个周期一部分是旧的采样间隔采得的数据，一部分是新的采样间隔采得的数据
    {
        return;   //
    }
    //中断里用到了这两个变量，更新的时候必须保护起来
    EnterCritical();
    g_dwAdFreq = g_AcSample.dwFreq * NUM_PER_CYC; //新的采样频率
    
	g_bFreqStep = 0; //频率跟踪的当前步骤:0-交采计算程序计算出一个新频率;1-AD中断采用最新的频率进行采样        
    ExitCritical();
}

//#define COMPOSE_SAMPLE(ps, p1, p2, frac) ( *(ps + p1) + ( (*(ps + p1) - *(ps + p2)) * frac >> FREQ_SHIFT) )
#define COMPOSE_SAMPLE(ps, p1, p2, frac) ( *(ps + p1) + (*(ps + p2) - *(ps + p1)) * frac / FREQ_UNIT )

//描述：从AD的采样缓冲区中，均匀抽取FFT_NUM个样点，用来做FFT变换
bool CopyToFftBuf()
{    
#if PRINTF_AD_DATA    
    DTRACE(1, ("\r\n"));//for test!
#endif
	for (WORD wCn=0; wCn<SCN_NUM; wCn++)
	{		
		for (WORD i=0; i<FFT_NUM; i++)
		{
			m_cplxFftIn[wCn][i].re = g_wAdBuf[wCn+1+i*NUM_CHANNELS];//g_sSampleBuf[wCn][wAcRdPtr++];//第一道是电池电压,第二道时交采，第三道是温度			            
#if PRINTF_AD_DATA
            if (wCn == 0)   //for test!
            {                
                DTRACE(1, ("%d\n", m_cplxFftIn[wCn][i].re));                    
            }
#endif
		}    
#if PRINTF_AD_ONEPOINT
        if (wCn == 0)   //for test!
        {                
            DTRACE(1, ("%d\n", g_wAdBuf[wCn+1+0*NUM_CHANNELS]));                    
        }
#endif        
	}
	return true;
}

void PhaseAdjust()
{
}

//#define PRECOMPOSE_SAMPLE(ps, p1, p2, frac) ( *(ps + p1) - ((*(ps + p1) - *(ps + p2)) * frac >> FREQ_SHIFT) )
//						//p2是p1前面的点

#define COMPOSE_SAMPLE2(ps, p1, p, p2, frac) (frac<0 ? *(ps + p) + ( (*(ps + p2) - *(ps + p)) * -frac >> FREQ_SHIFT )	: \
														*(ps + p) - ( (*(ps + p) - *(ps + p1)) * frac >> FREQ_SHIFT )	)
						//p是当前点，p1是前面一点，p2后面一点
						//frac为正，使电压波形滞后，波形整体往右移
						//frac为负，使电压波形超前，波形整体往左移
//关于角度校正的正负的例子：一台终端在角度校正前的误差如下
// 测试点		误差  
//  0.5L		-0.84
//  0.5C		0.85
//	0.8L		-0.275
//	0.8C		0.293

//从误差的情况可以看出终端的电流相对与电压滞后了一定的角度，
//那么电压相对于电流的角度校正值frac应为正，表示电压往后移，以缩小这个角度误差

//每周波计算一次,算出有效值,功率,功率因数,频率,电能
//返回：一个周波采样并计算完成返回true.正在采样返回false
//section("L1_code")
bool Calcu()
{	
	LoadPara();
	WORD i;
	//WORD wSamplePtr = SBUF_SIZE;
#if (FFT != 0)
	complex_fract16 cplxFftOut[FFT_NUM];
#endif

	//拷贝采样数据到缓存
    if (!CopyToFftBuf())
		return false;
    
#if (FFT==0)  //均方根算法
    BYTE j;
    long lRms;
    DWORD dwAdData[SCN_NUM]; 
    BYTE bVolt[4];
    static WORD wAdData[SCN_NUM] = {0};
    static BYTE bCnt = 0;   //求平均值的周波个数统计
    memset(dwAdData, 0, sizeof(dwAdData));
    for (i=0; i<SCN_NUM; i++)   
    {
        for (j=0; j<FFT_NUM; j++)//求均值
        {            
            dwAdData[i] += (DWORD)m_cplxFftIn[i][j].re;
        }
        dwAdData[i] = dwAdData[i]/FFT_NUM; 
        
        for (j=0; j<FFT_NUM; j++)//去掉直流信号
        {
            if (m_cplxFftIn[i][j].re >= dwAdData[i])//避免出现负数
                m_cplxFftIn[i][j].re -= dwAdData[i]; 
            else
                m_cplxFftIn[i][j].re = dwAdData[i]-m_cplxFftIn[i][j].re;
        }
    }        
    memset(dwAdData, 0, sizeof(dwAdData));                         
    for (i=0; i<SCN_NUM; i++) //交流32个点求均方和
    {
        for (j=0; j<FFT_NUM; j++)
        {                               
            dwAdData[i] += (DWORD)m_cplxFftIn[i][j].re*m_cplxFftIn[i][j].re;//求平方和                
        }
        dwAdData[i] = dwAdData[i]/FFT_NUM;
        dwAdData[i] = (DWORD)(sqrt((float)dwAdData[i])); //求根号
        wAdData[i] += (WORD)dwAdData[i];   //多个周波求和
    }  
    
    bCnt++;
    if (bCnt >= 25)
    {         
        for (i=0; i<SCN_NUM; i++)//各通道数据校准入库
        {        
            wAdData[i] = wAdData[i]/bCnt;
#if PRINTF_IEVALUE        
            if (i==0)
            {   
                DTRACE(1, ("%d\r\n", dwAdData[i])); //未校准的值
            }
#endif                
            lRms = (long)ConvertToLineVal(i, wAdData[i]);  //校正结果
            if (lRms >= 0)
            {       
                g_AcSample.iValue[i] = lRms;
                DWORDToBCD((DWORD)(lRms+5)/10, bVolt, sizeof(bVolt));  //只保留一位小数
                WriteItemEx(BN2, PN0, 0x1031+i, bVolt); //校准后的值入库
                //DTRACE(1, ("%d\r\n", lRms)); //校准的值            
            }
            else
            {
                //DTRACE(DB_TASK, ("#### Convert Err!\n"));
            }  
        }
         
        //原始值入库
        WriteItemEx(BN2, PN0, 0xba11, (BYTE *)wAdData);
        memset(wAdData, 0, sizeof(wAdData));
        
        bCnt = 0;  
    } 
#else

	//动态装载校正参数,方便组态校正
	ReadItemDI(&g_AcSample.diAdjPara, (BYTE* )&g_AcSample.iAdj);

	//FFT变换并计算各通道有效值
    for (i=0; i<SCN_NUM; i++) //32点240us可以计算完一个通道，64点550us
    {
        //SetLed(true, 2);        //test fft cost time
        //memset(&m_cplxFftOut[i], 0, sizeof(m_cplxFftOut[i]));
		//memset(&m_cplxFftOutUnAdj[i], 0, sizeof(m_cplxFftOutUnAdj[i]));
		
    	//cfft_fr16(m_cplxFftIn[i], NULL, cplxFftOut, w, 1, FFT_NUM, 0, 0);
        cfft_fr16(m_cplxFftIn[i], NULL, cplxFftOut, NULL, 1, FFT_NUM, 0, 0); 
        
		//NOTE:complex_fract16型,在所测的电流值比较大(如保护电流的情况)的情况下,
        //VectAjd函数中进行校正时会溢出,所以将m_cplxFftOut由complex_fract16型改为
        //了complex_fract32型,使用中间临时变量cplxFftOut参与cfft_fr16的计算,在VectAjd
        //中将cplxFftOut值拷贝给m_cplxFftOut参与接下来的计算,但是VectAjd里是跳过了直流
        //分量的,如果下面的的计算中使用到了直流分量,可能会有问题
        //VectAdj(cplxFftOut, m_cplxFftOut[i], m_cplxFftOutUnAdj[i], g_AcSample.iAdj[i*2], g_AcSample.iAdj[i*2+1]); //矢量校正
		VectAdj(cplxFftOut, m_cplxFftOut[i], g_AcSample.iAdj[i<<1], g_AcSample.iAdj[i<<1+1]); //矢量校正		
		g_AcSample.iValue[i] = CalcuRMS(m_cplxFftOut[i]);
        
//        WriteItemEx(BN0, PN0, 0xC322+i, (BYTE *)g_AcSample.iValue); //校准后的值入库
        //SetLed(false, 2);
        //DTRACE(0, ("%d\r\n", m_iValue[i]));
		//printf("ch %d = %d\r\n", i, m_iValue[i]);
		//TODO：判断小电流接地
    }
#endif
    
    for (i=0; i<SCN_NUM; i++) 
    {
        if (g_AcSample.iValue[i] < VAILD_V)
            g_AcSample.iValue[i] = 0;        
    }    
    
    //故障检测
	//DoFaultJugement(); 
    
   	//频率跟踪
	FreqSync();

	//计算谐波含有率
	//DoHarmonic();

	//计算功率、功率因素
	//CalcuAngle();
	//CalcuPQCOS();
    
    //CalcU0();
    //CalcI0();

 	//Transfer();

	if (g_AcSample.fAdjParaSet) //校准状态返回false不做保护
		return false;
	else
		return true;
}

void GetZero(short* psZero, WORD wFreqPtr, WORD wPtrInBuf)
{
    DWORD dwAvg = VOLT_REF>>1;      //零点是基准的一半
	if (wPtrInBuf == 0)		
        psZero[0] = g_wAdBuf[g_AcSample.wFreqCn+(SBUF_SIZE-1)*NUM_CHANNELS]-dwAvg;  //减去平均值
	else		
    {
        psZero[0] = g_wAdBuf[g_AcSample.wFreqCn+(wPtrInBuf-1)*NUM_CHANNELS]-dwAvg;
    }					

    psZero[1] = g_wAdBuf[g_AcSample.wFreqCn+wPtrInBuf*NUM_CHANNELS]-dwAvg;
	psZero[2] = wFreqPtr;   //整个跟踪范围的相对位置
}

//返回的是两个用于测算频率的过零点间的采样间隔数，间隔放大了FREQ_UNIT倍
int CalcuFreq(short* psZero1, short* psZero2, WORD wFreqPntCnt) 
{
	if (wFreqPntCnt>NUM_PER_MAX_PN || 
		wFreqPntCnt<NUM_PER_MIN_PN)
		return -1;
		
	int t1, t2;
    if ((psZero1[1] == psZero1[0]) || (psZero2[1] == psZero2[0]))
        return -1;
    
	t1 = (int )psZero1[1] * FREQ_UNIT / (psZero1[1] - psZero1[0]);//细分过零点的部分，提高精度
	t2 = (int )psZero2[1] * FREQ_UNIT / (psZero2[1] - psZero2[0]);
	
	return wFreqPntCnt*FREQ_UNIT + t1 - t2;
}

void ResetFreqSync()
{
	g_AcSample.fPrePos = false;
	g_AcSample.wZeroPntCnt = 0;
	g_AcSample.wFreqPntCnt = 0;
	g_AcSample.wZeroCnt = 0;
        
	g_dwFreqPtr = 0;    
    	
	memset(g_AcSample.sZero1, -1, sizeof(g_AcSample.sZero1));
	memset(g_AcSample.sZero2, -1, sizeof(g_AcSample.sZero2));
	
	//重新确定频率跟踪的通道A->C->B               //CL818K5只有一路交采
	/*if (g_AcSample.iValue[0]>=g_AcSample.iValue[1] && g_AcSample.iValue[0]>=g_AcSample.iValue[2])   //哪一通道有效值最大就跟踪哪一通道
		g_AcSample.wFreqCn = 0;
	else if (g_AcSample.iValue[2] >= g_AcSample.iValue[1])
		g_AcSample.wFreqCn = 2;
	else*/
		g_AcSample.wFreqCn = 1;
		
	g_AcSample.dwFreq = 50000;
		
	g_AcSample.dwFreqPnts = FREQ_CYC_NUM * NUM_PER_CYC * FREQ_UNIT; //频率跟踪FREQ_CYC_NUM个周期(标准每周期NUM_PER_CYC点)点数 * FREQ_UNIT
    
	UpdateFreq(); 
}

//n:t时间内从正到负的过零点个数
//t:时间
//2:一个周波内1个从正到负的过零点
//原理:f = (n/t) 
//t = 采样间隔*采样点数
//描述:利用从正到负的n个过零点算频率
void FreqSync(void)
{    
    WORD i;
    DWORD dwAvg = VOLT_REF>>1;      //零点是基准的一半
    if ((g_bFreqSync != 0) || (g_bFreqStep == 0))  //频率切换过渡期数据是按旧的采样频率采样得来的，不能与下一组用新周期采样得来的数一起跟踪，这个是由DMA缓存造成的
    {
		g_AcSample.fPrePos = false;
		g_AcSample.wZeroPntCnt = 0;
		g_AcSample.wFreqPntCnt = 0;
		g_AcSample.wZeroCnt = 0;
		g_dwFreqPtr = 0;

		memset(g_AcSample.sZero1, -1, sizeof(g_AcSample.sZero1));
		memset(g_AcSample.sZero2, -1, sizeof(g_AcSample.sZero2));

		return;
	}        
    	  
    for (i=0; i<NUM_PER_CYC*CYC_NUM; i++)  //每回频率跟踪到本轮计算的最后一个点
	{
		g_AcSample.wZeroPntCnt++;   //频率跟踪的相邻两个过零点间的点数            一个波内的点数
		g_AcSample.wFreqPntCnt++;   //频率跟踪的参与计算频率两个过零点间的点数    多个波总共的点数
		if (g_AcSample.wZeroPntCnt > NUM_PER_CYC*2)  
		{   //这里的复位判断要求不要太严格,避免随便一个干扰就复位跟踪
			ResetFreqSync();
			break;
		}
		
		if (g_AcSample.fPrePos)
		{			
            if (g_wAdBuf[g_AcSample.wFreqCn+i*NUM_CHANNELS] < dwAvg)//找到新从正到负的过零点
			{
				g_AcSample.fPrePos = false;
				
				if (g_AcSample.sZero1[2] <= 0)   //第一个过零点还没有
				{
					GetZero(g_AcSample.sZero1, g_dwFreqPtr, i);
					g_AcSample.wZeroPntCnt = 1;
					g_AcSample.wFreqPntCnt = 1;
					g_AcSample.wZeroCnt = 1;
				}
				else
				{
					if (g_AcSample.wZeroPntCnt > NUM_PER_CYC_55HZ*2/3)  //合理的过零点间隔
					{
						g_AcSample.wZeroPntCnt = 1;
						g_AcSample.wZeroCnt++;
						if (g_AcSample.wZeroCnt > FREQ_CYC_NUM) //达到计算的周波个数则计算频率,2秒算一次
						{
							GetZero(g_AcSample.sZero2, g_dwFreqPtr, i);
							int iFreq = CalcuFreq(g_AcSample.sZero1, g_AcSample.sZero2, g_AcSample.wFreqPntCnt-1);
							//if (iFreq>0 && g_AcSample.iValue[g_AcSample.wFreqCn]>VAILD_V) 
                            if (iFreq>0 && g_AcSample.iValue[0]>VAILD_V)  //g_AcSample.iValue里存放的是交流值，注意这里固定的，不通用
							{			//计算出有效的频率及有效的幅值才更新频率值
								g_AcSample.dwFreqPnts = iFreq;
								memcpy(g_AcSample.sZero1, g_AcSample.sZero2, sizeof(g_AcSample.sZero1));
								g_AcSample.wZeroPntCnt = 1;
								g_AcSample.wFreqPntCnt = 1;
								g_AcSample.wZeroCnt = 1;
                                g_dwFreqPtr = 0;
								
								UpdateFreq();
							}
							else   //频率跟踪失败
							{
								ResetFreqSync();
								g_AcSample.wFreqRstCnt++;
								WriteItemEx(BN2, PN0, 0x1026, (BYTE *)&g_AcSample.wFreqRstCnt); //频率跟踪复位次数,HEX
								break;
							}
						}
					}
				}
				
				g_AcSample.wZeroPntCnt = 0;
			}  //end of 找到新从正到负的过零点
		}		
        else if (g_wAdBuf[g_AcSample.wFreqCn+i*NUM_CHANNELS] >= dwAvg)
		{
			g_AcSample.fPrePos = true;
		}
		
		g_dwFreqPtr++;		
	}
}

//零序电压在配置里面可能没有对应量，所以另外存储在缓存中
//三相电压的矢量和，只算基波,并且是校准后的基波
/*
void CalcU0()
{		
	float Ureal,Uimg;
	if (g_AcSample.AcPara.bConnectType == CONNECT_3P4W)
	{
		Ureal = m_cplxFftOut[0][1].re + m_cplxFftOut[1][1].re + m_cplxFftOut[2][1].re;//三相电压基波实部和
		Uimg = m_cplxFftOut[0][1].im + m_cplxFftOut[1][1].im + m_cplxFftOut[2][1].im;
		g_AcSample.iU0[0] = (int)sqrt(Ureal*Ureal+Uimg*Uimg);
	}
	else //三相三线没有零序电
	{
		g_AcSample.iU0[0] = 0;
	}
}

//零序电压在配置里面可能没有对应量，所以另外存储在缓存中
void CalcI0()
{				
	float Ireal,Iimg;

	if (g_AcSample.AcPara.bConnectType == CONNECT_3P4W)
	{
		Ireal = m_cplxFftOut[3][1].re + m_cplxFftOut[4][1].re + m_cplxFftOut[5][1].re;
		Iimg = m_cplxFftOut[3][1].im + m_cplxFftOut[4][1].im + m_cplxFftOut[5][1].im;
		g_AcSample.iI0[0] = (int)sqrt(Ireal*Ireal+Iimg*Iimg);
	}
    else
    {
        g_AcSample.iI0[0] = 0;
    }
    //DTRACE(0, ("%s Ia %f Ib %f Ic %f, AngleA %f AngleB %f AngleC %f, I0 %d!\r\n", __FUNCTION__, Ia,Ib,Ic,AngleA,AngleB,AngleC, g_AcSample.iI0[0]));	
}*/


//数据转存到数据库中
/*
void Transfer()
{
	WORD i;
	BYTE* p;	
	BYTE bBuf[SCN_NUM*8];
    	
	//交采基波矢量
    memset(bBuf, 0, sizeof(bBuf));
	p = bBuf;
	for (i=0; i<SCN_NUM; i++)
	{
		memcpy(p, &m_cplxFftOut[i][1].re, 4);
        //memcpy(p, &m_cplxFftOutUnAdj[i][1].re, 4);
		p += 4;
		memcpy(p, &m_cplxFftOut[i][1].im, 4);
        //memcpy(p, &m_cplxFftOutUnAdj[i][1].im, 4);
		p += 4;
        
#if PRINTF_IEVALUE
        if (i==0)
        {   
            int iEValue = CalcuRMS(m_cplxFftOut[i]);  //真有效值
            DTRACE(1, ("%d\r\n", iEValue));            
        }
#endif
	}

	WriteItemDI(&g_AcSample.diAcVect, bBuf);	//各通道基波矢量入库
       
	//YC数据块
	SaveYCData();
}*/

unsigned short CalcuTotalHarmonic(complex_fract32* cplx, int iBaseHarVal, int* piBasePercent)
{
	int iResult = 0;
	int iRet = 0;
	int iNoVal = iBaseHarVal >> 8;
    cplx++;	//不计算直流分量
    
    int iTemp;
    
    int iBaseSqr = (int )cplx->re*cplx->re + (int )cplx->im*cplx->im;
    cplx++;	//不计算基波
        
    for (WORD i=2; i<=g_AcSample.AcPara.wHarmNum; i++)
    {
    	iRet = (int )cplx->re * cplx->re + (int )cplx->im * cplx->im;
    	if (iRet <= iNoVal)
    		iRet = 0;
    	
        iResult += iRet;
	 	cplx++;
    }
    
    iTemp = (int)(10*sqrt((double)(iResult+ iBaseSqr))); //放大10倍,可以保留一位小数
    if (iTemp == 0)
        iTemp = 1*10;
    
	*piBasePercent = 100000ULL* iBaseHarVal / iTemp; //基波有效值占总有效值的比例
    
    if (iBaseHarVal == 0)
        iBaseHarVal = 1;
    
    return (unsigned short )sqrt((double)iResult)*10000/iBaseHarVal;
}


//描述:计算某次谐波的含量,
//参数:@cplx 某次谐波的向量
//     @total 总的有效值
//返回:谐波含量,单位万分一
unsigned short CalcuHarmonic(complex_fract32* cplx, int total)
{
	if (total == 0)
		return 0;
		
    int iResult = (int )cplx->re * cplx->re + (int )cplx->im * cplx->im;
    
	iResult = (int)sqrt((double)iResult)*10000/total;
	return (unsigned short ) (iResult >= 10000 ? 9999 : iResult);
}

#if 0
//备注:谐波含有率格式:依次Ua,Ub,Uc,Ia,Ib,Ic的总,2~HARMONIC_NUM次,格式NN.NN,单位%
void DoHarmonic()
{
	if (!g_AcSample.AcPara.fCalcuHarmonic)	//不计算谐波
		return;

	WORD i;
	WORD wHarPercent;
	DWORD dwNoVolt = g_AcSample.AcPara.dwUn / 10 + 1;	//10%标准电压
	DWORD dwNoCurrent = g_AcSample.AcPara.dwIn * 5 / 100;	//5%标准电流
	
	int iBasePercent;	//基波有效值占总有效值的比例
	DWORD dwBaseVal;	//基波有效值
	WORD m = 0;
	WORD wBase;

   	for (i=0; i<6; i++)	//UA IA UB IB UC IC共6个通道
   	{
   		//基波有效值
   		DWORD dwBaseVal = (int )m_cplxFftOut[i][1].re*m_cplxFftOut[i][1].re + (int )m_cplxFftOut[i][1].im*m_cplxFftOut[i][1].im;
		WORD wBaseHarVal = (unsigned short)sqrt((double)dwBaseVal);
		
   		//算总谐波含有率及有效值
		WORD wTotalHarPecent;

		if ((i%2==0 && g_AcSample.iValue[i]<dwNoVolt) ||
			(i%2!=0 && g_AcSample.iValue[i]<dwNoCurrent) || 
			wBaseHarVal==0)
		{
			wTotalHarPecent = 0;
		}
		else
		{
   			wTotalHarPecent = CalcuTotalHarmonic(m_cplxFftOut[i], wBaseHarVal, &iBasePercent);//总畸变
		}
		m = 0;
		if (i%2 == 0)	//将UaIaUbIbUcIc顺序变成UaUbUcIaIbIc
			wBase = i/2 * HARM_NUM_MAX;
		else 
			wBase = (i/2+3) * HARM_NUM_MAX;
		g_AcSample.tPOH[wBase+m++].iValue = wTotalHarPecent;
		g_AcSample.tPOH[wBase+m-1].wDesBits = 0;
		
		dwBaseVal = g_AcSample.iValue[i] * iBasePercent / 10000; 	//基波有效值
		
		//算各次谐波含有率及有效值
   		for (WORD j=2; j<=g_AcSample.AcPara.wHarmNum; j++)
   		{
   			//算各次谐波含有率
   			if (wTotalHarPecent == 0) //某相没有电压或电流
   				wHarPercent = 0;
   			else
				wHarPercent = CalcuHarmonic(&m_cplxFftOut[i][j], wBaseHarVal);
			
			g_AcSample.tPOH[wBase+m++].iValue = wHarPercent;
			g_AcSample.tPOH[wBase+m-1].wDesBits = 0;
   		}
   	}
		
	if (g_diHarPercent.pbAddr != NULL);
		WriteItemDI(&g_diHarPercent, (BYTE*)g_AcSample.tPOH);
}
#endif

//描述:校正一个通道FFT_NUM/2个谐波分量
//void VectAdj(complex_fract16* incplx, complex_fract32* outcplx, complex_fract32* outUnAjdcplx, int adjreal, int adjimag)
void VectAdj(complex_fract16* incplx, complex_fract32* outcplx, int adjreal, int adjimag)
{
    int i;    
	if (!g_AcSample.fAdjParaSet) //校准时直接将值拷贝给outcplx，
    {
        for(i=0; i<FFT_NUM; i++)
        {
            outcplx->re = (int)incplx->re;
            outcplx->im = (int)incplx->im;
            
            //outUnAjdcplx->re = (int)incplx->re;
		    //outUnAjdcplx->im = (int)incplx->im;
            
            //outUnAjdcplx++;
            incplx++;
	        outcplx++;  
        }
        return;
    }

	complex_fract32  tmp;

	incplx++;//不计算直流分量
	outcplx++;
	//outUnAjdcplx++;

	for(i=1; i<FFT_NUM/2; i++)
	{
        //tmp.re = ((int )incplx->re*adjreal - (int )incplx->im*adjimag) / 8192;
		//tmp.im = ((int )incplx->im*adjreal + (int )incplx->re*adjimag) / 8192;
		tmp.re = ((int )incplx->re*adjreal - (int )incplx->im*adjimag)>>13;
		tmp.im = ((int )incplx->im*adjreal + (int )incplx->re*adjimag)>>13;

		//outUnAjdcplx->re = (int)incplx->re;
		//outUnAjdcplx->im = (int)incplx->im;
		outcplx->re = tmp.re;
		outcplx->im = tmp.im;
		//outUnAjdcplx++;
		incplx++;
		outcplx++;
	}
}

//描述：计算有效值(RMS : Root Mean Square)
int CalcuRMS(complex_fract32* cplxIn)
{
	double sum = 0;
	int i;
	cplxIn++; //不计算直流分量
	for(i=1; i<FFT_NUM/2; i++) //FFT_NUM/2
	{
		 sum += ((double )cplxIn->re*cplxIn->re + (double )cplxIn->im*cplxIn->im);
		 cplxIn++;
	}
	return (int )sqrt((double )sum);
}

//备注：三相三线时的功率  Pa = Uab * Ia *Cos()/Pc = Ucb * Ic * Cos()/ P0 = Pa + Pc
//对于接线,各个地方版本的终端内部有两种不同的接线方式,
// 第一种:
// A相接Ua和Ub
// B相接Ua和Uc
// C相接Uc和Ub
// 第二种:
// A相接Ua和Ub
// B相接Ub和Uc
// C相接Uc和Ua
// 第二种方式下的C相电压相当于第一种方式下的B相电压，相位反向
// 计算Pc时，要使用B相的电压乘以C相电流，角度使用B相电压角度减去180度
// 后与C相电流的差值
/*
int CalcuPQCOS()
{
	//int iResultP = 0;
	//int iResultQ = 0;
	int* iP = g_AcSample.iP;
	int* iQ = g_AcSample.iQ;
	int* iCos = g_AcSample.iCos;
	//complex_fract32* cplxI;
	//complex_fract32* cplxV;
	
	float x,y;
	int iDeltAngle;
	
	for (int j = 0; j < 4; j++,iCos++,iP++,iQ++)
	{
		if (j < 3)
		{		
			iDeltAngle = g_AcSample.iAngle[j]-g_AcSample.iAngle[j+3];
			x = 1000*cos(ANGLE(iDeltAngle));
			y = 1000*sin(ANGLE(iDeltAngle));

//			*iP = int((long)m_iValue[*pbChannel]*(long)m_iValue[*(pbChannel+3)]*cos(ANGLE(m_iAngle[j]-m_iAngle[j+3]))/100000);
//			*iQ = int((long)m_iValue[*pbChannel]*(long)m_iValue[*(pbChannel+3)]*sin(ANGLE(m_iAngle[j]-m_iAngle[j+3]))/100000);
    		*iP = (g_AcSample.iValue[j]*g_AcSample.iValue[j+3]*x)/1000000; //保留两位小数
			*iQ = (g_AcSample.iValue[j]*g_AcSample.iValue[j+3]*y)/1000000;

			if (g_AcSample.iValue[j]<100 &&g_AcSample.iValue[j+3]<100) //没有电压电流的时候相角差也为0
				*iCos = 0;
			else
				*iCos = x;
		}
		else if (j==3)
		{
			if (g_AcSample.AcPara.bConnectType == CONNECT_3P3W)
			{
				*iP = *(iP-3) + *(iP-1);
				*iQ = *(iQ-3) + *(iQ-1);
				*(iP-2) = 0;
				*(iQ-2)	= 0;
			}
			else
			{
				*iP = *(iP-3) + *(iP-2) + *(iP-1);
				*iQ = *(iQ-3) + *(iQ-2) + *(iQ-1);
			}

			*iCos = CalcuCos(*iP, *iQ); //总功率因数
		}
	}	

	#if 0
	//由于各个通道采样不同步带来相位差，用下面的方法来计算功率会因
	//电压电流之间的相位差而得到错误的值，必须对这个相角差进行校正
	// 之后计算才可以	
	for (int j = 0; j < 4; j++,iCos++,iP++,iQ++)
	{
		if (j < 3)
		{			
			cplxV = m_cplxFftOut[j];
			cplxI = m_cplxFftOut[j+3];

			cplxI++;
			cplxV++; //不计算直流分量

			iResultP = 0;
			iResultQ = 0;
			for(WORD k=1; k<FFT_NUM/2; k++)
			{
			   iResultP += (int )cplxV->re * cplxI->re + (int )cplxV->im * cplxI->im;
			   iResultQ += (int )cplxI->re * cplxV->im - (int )cplxI->im * cplxV->re;

			   cplxI++;
			   cplxV++;
			}

			*iP = iResultP/100000;	//电压扩大了100倍，电流扩大了1000倍
				
			//if (iResultQ < 0)
			//	iResultQ = -iResultQ;
			*iQ = iResultQ/100000;	//电压扩大了100倍，电流扩大了1000倍				
		}
		else if (j == 3)//总功率
		{
			if (m_AcPara.bConnectType == CONNECT_3P3W)
			{
				*iP = *(iP-3) + *(iP-1);
				*iQ = *(iQ-3) + *(iQ-1);
				*(iP-2) = 0;
				*(iQ-2)	= 0;
			}
			else
			{
				*iP = *(iP-3) + *(iP-2) + *(iP-1);
				*iQ = *(iQ-3) + *(iQ-2) + *(iQ-1);
			}
		}
		*iCos = CalcuCos(*iP, *iQ);
	}
	//printf("Pa=%d  Pc=%d  P=%d\n", m_iP[0], m_iP[2], m_iP[3]);
	//printf("Qa=%d  Qc=%d  Q=%d\n", m_iQ[0], m_iQ[2], m_iQ[3]);

    #endif
	
	return 1;
}

long CalcuCos(long P,long Q)
{
	if (P==0 && Q==0)   //避免除0
		return 0;
		
    //return (long)((long long )COS_N * ABS(P) / sqrt((double)P*P + (double)Q*Q));
	return (long)((long long )COS_N * P / sqrt((double)P*P + (double)Q*Q));
}

void CalcuS()
{
	for(int i=0;i<4;i++)
	{
		g_AcSample.iS[i] = (int)sqrt((double)g_AcSample.iP[i]*g_AcSample.iP[i] + (double)g_AcSample.iQ[i]*g_AcSample.iQ[i]);
	}
	return;
}

//算相角
void CalcuAngle()
{
	int iAngleUa = 0;
	g_AcSample.iAngleUa = 0;
	int* ipAngel = g_AcSample.iAngle;	
    //int iCnt;
    memset(g_AcSample.iAngle, 0, sizeof(g_AcSample.iAngle));

	for (int j = 0; j < 6; j++)//三相电压、电流
	{		
        if (g_AcSample.iValue[j] < VAILD_V)//真值小于某个值时就不用计算相角
        {            
            *ipAngel++ = 0;
            continue;
        }
		if (j == 0)//Ua
		{
			//角度计算要用未经矢量校正的FFT值
            *ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOut[j][1].im, (float)m_cplxFftOut[j][1].re) / 3.1415926 ;
//			*ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOutUnAdj[j][1].im, (float)m_cplxFftOutUnAdj[j][1].re) / 3.1415926 ;
			g_AcSample.iAngleUa = iAngleUa = *ipAngel;
			*ipAngel = 0;			
		}
		else 
		{
			//角度计算要用未经矢量校正的FFT值
			*ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOut[j][1].im, (float)m_cplxFftOut[j][1].re) / 3.1415926 ;
//			*ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOutUnAdj[j][1].im, (float)m_cplxFftOutUnAdj[j][1].re) / 3.1415926 ;
			*ipAngel = (*ipAngel + 3600 - iAngleUa) % 3600;

			//修正一个AD分时采样各个通道采样不同步带来的角度误差
			//iCnt = j - 0;// bChannelUa
//			*ipAngel = ((*ipAngel*10 +36000 - iCnt*36000/(SCN_NUM*64))/10) % 3600;//先放大10倍再缩小10倍，控制误差,不修正电流有5度误差
		}		
		ipAngel++;
        //DTRACE(0, ("%s j %d, *ipAngel %d\r\n",__FUNCTION__, j, *ipAngel));
	}
}*/

int CorrectVol(int iVal)
{
    if (iVal > 40 && iVal < 100) {
        iVal -= 20;
    }
    return iVal;
}

//描述：将相位计算值组成YC格式入库
/*
void SaveYCData()
{	
	const BYTE *pData = (const BYTE*)g_AcSample.tVal;
    //int i = 0;	
	int* iP = g_AcSample.iP;
	int* iQ = g_AcSample.iQ;
	int* iCos = g_AcSample.iCos;
	int* iAngle = g_AcSample.iAngle;
    memset(g_AcSample.tVal, 0, sizeof(g_AcSample.tVal));
   	
	memset((BYTE*)g_AcSample.tVal, 0, sizeof(g_AcSample.tVal));    
  
	//BYTE bChIa, bChIc, bChProIa, bChProIc,bChI0;
    
	g_AcSample.tVal[AC_VAL_UA].iValue = g_AcSample.iValue[0]; 	//Uab
    g_AcSample.tVal[AC_VAL_UB].iValue = g_AcSample.iValue[1];	//Uac
    g_AcSample.tVal[AC_VAL_UC].iValue = g_AcSample.iValue[2];	//Ucb
    g_AcSample.tVal[AC_VAL_ANG_UA].iValue = *iAngle++;
  	g_AcSample.tVal[AC_VAL_ANG_UB].iValue = *iAngle++;
	g_AcSample.tVal[AC_VAL_ANG_UC].iValue = *iAngle++;

	g_AcSample.tVal[AC_VAL_IA].iValue = g_AcSample.iValue[3];  //IA
	g_AcSample.tVal[AC_VAL_IB].iValue = g_AcSample.iValue[4];  //IB
    g_AcSample.tVal[AC_VAL_IC].iValue = g_AcSample.iValue[5];	//IC

	g_AcSample.tVal[AC_VAL_ANG_IA].iValue = *iAngle++;
	g_AcSample.tVal[AC_VAL_ANG_IB].iValue = *iAngle++;
	g_AcSample.tVal[AC_VAL_ANG_IC].iValue = *iAngle++;
	//iAngle++; //零序电压,没有保存零序相角
	//iAngle++; //零序电流

	//保护电流
	//pbProChannel += 3;
	//bChProIa = *pbProChannel;
	//g_AcSample.tVal[AC_VAL_PRO_IA].iValue = g_AcSample.iValue[*pbProChannel++]; //IAP
	//g_AcSample.tVal[AC_VAL_PRO_IB].iValue = g_AcSample.iValue[*pbProChannel++]; //IBP
	//g_AcSample.tVal[AC_VAL_PRO_IC].iValue = g_AcSample.iValue[*pbProChannel++]; //ICP
    g_AcSample.tVal[AC_VAL_PA].iValue = *iP++; //PA
    if (g_AcSample.AcPara.bConnectType == CONNECT_3P3W) //三相三线
   		iP++;//PB
    else
        g_AcSample.tVal[AC_VAL_PB].iValue = *iP++;
    g_AcSample.tVal[AC_VAL_PC].iValue = *iP++; //PC
	g_AcSample.tVal[AC_VAL_P].iValue = *iP++;  //P
										  
	g_AcSample.tVal[AC_VAL_QA].iValue = *iQ++; //QA
    if (g_AcSample.AcPara.bConnectType == CONNECT_3P3W)
   	    iQ++; //QB
    else
        g_AcSample.tVal[AC_VAL_QB].iValue = *iQ++;
	g_AcSample.tVal[AC_VAL_QC].iValue = *iQ++; //QC
	g_AcSample.tVal[AC_VAL_Q].iValue = *iQ++;  //Q

	g_AcSample.tVal[AC_VAL_COSA].iValue = *iCos++; //QA
    if (g_AcSample.AcPara.bConnectType == CONNECT_3P3W)
   		iCos++;
    else
        g_AcSample.tVal[AC_VAL_COSB].iValue = *iCos++;
	g_AcSample.tVal[AC_VAL_COSC].iValue = *iCos++; //QC
	g_AcSample.tVal[AC_VAL_COS].iValue = *iCos++;  //Q
    
    g_AcSample.tVal[AC_VAL_I0].iValue = g_AcSample.iI0[0];//I0
    g_AcSample.tVal[AC_VAL_U0].iValue = g_AcSample.iU0[0];//U0

	#if 0
	static BYTE bCnt=0;
	if (++bCnt >=20)
	{
		bCnt = 0;
		if (i==1) 
		{
	    	//printf("Ia = %d\n",m_tVal[AC_VAL_IA].iValue);
    		//printf("Ib = %d\n",m_tVal[AC_VAL_IB].iValue);
		    //printf("Ic = %d\n",m_tVal[AC_VAL_IC].iValue);
	    	//printf("U0 = %d\n",m_tVal[AC_VAL_U0].iValue);
		    //printf("I0 = %d\n",m_tVal[AC_VAL_I0].iValue);
           	//printf("Uab = %d\n",m_tVal[AC_VAL_UA].iValue);
		}			
	}
	#endif 

	//频率
	g_AcSample.tVal[AC_VAL_F].iValue = g_AcSample.dwFreq;
	g_AcSample.tVal[AC_VAL_F].wDesBits = 0;
	WriteItemDI(&g_AcSample.diYcBlk[0], (BYTE*)pData);  //终端自身交采数据
}*/
/*
//描述: 判断某相电流是否发生短路
int IsPhaseXShortCircuit(BYTE bPn, int iIx, BYTE bPhase, TSHJPara *pSHJPara)
{
	DWORD *pStartTime;
	DWORD *pRetrayTime;

	pStartTime = &g_dwTimeStart[bPn][bPhase];
	pRetrayTime = &g_dwTimeRetray[bPn][bPhase];
	if (iIx > pSHJPara->dwIh)//故障发生
	{
		if (*pStartTime == 0)
		{
			*pStartTime = GetTick();//开始计时
		}
		else
		{
			if ((GetTick() - *pStartTime) > pSHJPara->wT1Min)	//Ix过流且超过过流时间整定值
			{
				*pRetrayTime = GetTick();
                if (fOccur[bPn][bPhase] == false) {
                    DTRACE(0, ("%s Occur StartTime %d, now %d !\r\n", __FUNCTION__, *pStartTime, GetTick()));
                }
                fOccur[bPn][bPhase] = true;
				return 1;
			}
		}
	}
	else
	{//先发生，才能恢复
		if ((*pStartTime != 0) && (*pRetrayTime == 0))//故障恢复
		{
			*pRetrayTime = GetTick();
            DTRACE(0, ("%s Recover StartTime %d!\r\n", __FUNCTION__, GetTick()));
		}
		if ((GetTick() - *pRetrayTime) > pSHJPara->wT1Min)	//Ix过流且超过过流时间整定值
		{
			*pStartTime = GetTick();
            if (fOccur[bPn][bPhase] == true) {
                DTRACE(0, ("%s Recover StartTime %d, now %d!\r\n", __FUNCTION__, *pRetrayTime, GetTick()));
            }
            fOccur[bPn][bPhase] = false;
			return (-1);
		}
	}
	return 0;
}*/

#ifdef DEBUG
static BYTE Status;
#endif
//函数：相间故障检测，包括单相接地故障、两相相间短路、三相相间短路
//备注：
WORD PhaseFaultDetection(BYTE bPn, int iIa, int iIb, int iIc, TSHJPara *pSHJPara)
{
	WORD wStatus = 0;

	int iStatus = 0;
	iStatus = IsPhaseXShortCircuit(bPn, iIa, PHASE_A, pSHJPara);//相间短路
	if (iStatus > 0)//A相电流超阀值
		wStatus |= 1<<0;
	else if (iStatus < 0)//A相电流超阀值
		wStatus |= 1<<8;

	iStatus = IsPhaseXShortCircuit(bPn, iIb, PHASE_B, pSHJPara);//B相电流超阀值
	if (iStatus > 0)
		wStatus |= 1<<1;
	else if (iStatus < 0)
		wStatus |= 1<<9;

	iStatus = IsPhaseXShortCircuit(bPn, iIc, PHASE_C, pSHJPara);//C相电流超阀值
	if (iStatus > 0)
		wStatus |= 1<<2;
	else if (iStatus < 0)
		wStatus |= 1<<10;

	return wStatus;
}

const BYTE g_bSlickStatus[2][3] = {
	{0x03, 0x05, 0x06},
	{0x01, 0x02, 0x04},
};
/*
//根据计算电流幅值的方法，计算到的异常状态
WORD Det2Status(WORD wPN, WORD wState, bool fZeroI)
{
	BYTE bStatus = 0;
	if (wPN >= PN4)
		return 0;

	if (((wState & 0x0f) == 0x07) && (g_fStatusABC[wPN] == false))
	{
		DTRACE(0, ("%s Point %d, 3 short occur wState %d!\r\n", __FUNCTION__, wPN, wState));
		g_fStatusABC[wPN] = true;
		return 1<<THREE_SHORT;
	}
	else if (((wState>>8) & 0x07) && g_fStatusABC[wPN])
	{
		DTRACE(0, ("%s Point %d, 3 short recover wState %x!\r\n", __FUNCTION__, wPN, wState));
		g_fStatusABC[wPN] = false;
		return 1<<THREE_SHORT_RECOVER;
	}

	for (int i = 0; i < 3; i++)//发生
	{
		bStatus = (BYTE)wState&0x0f;
		if (bStatus == g_bSlickStatus[0][i])
		{
			if (fZeroI && (g_fStatus_groud[wPN][i] == false))//接地，并且没有发生
			{
				DTRACE(0, ("%s Point %d, two short & groud occur!\r\n", __FUNCTION__, wPN));
				g_fStatus_groud[wPN][i] = true;
				return 1<<TWO_SHORT_GROUD;
			}
			else if((bStatus == g_bSlickStatus[0][i]) && (!fZeroI) && (g_fStatus[wPN][i] == false))//没接地，并且没有发生 
			{
				DTRACE(0, ("%s Point %d, two short occur!\r\n", __FUNCTION__, wPN));
				g_fStatus[wPN][i] = true;
				return 1<<TWO_SHORT;
			}
		}
		else if (bStatus == g_bSlickStatus[1][i])
		{
			if (fZeroI && (g_fStatus_one_phase_groud[wPN][i] == false))//接地，并且没有发生
			{
				DTRACE(0, ("%s Point %d, 1 phase groud occur!\r\n", __FUNCTION__, wPN));
				g_fStatus_one_phase_groud[wPN][i] = true;
				return 1<<ONE_GROUD;
			}
		}

		bStatus = (BYTE)(wState>>8)&0x0f;
		if (((bStatus & g_bSlickStatus[0][i]) || (!fZeroI)) && (g_fStatus_groud[wPN][i] == true))//接地恢复或相间短路恢复，并且发生
		{
			DTRACE(0, ("%s Point %d, two short & groud recover!\r\n", __FUNCTION__, wPN));
			g_fStatus_groud[wPN][i] = false;
			return 1<<TWO_SHORT_GROUD_RECOVER;
		}
		else if ((bStatus & g_bSlickStatus[0][i]) && (g_fStatus[wPN][i] == true))//接地恢复，并且发生
		{
			DTRACE(0, ("%s Point %d, two short recover !\r\n", __FUNCTION__, wPN));
			g_fStatus[wPN][i] = false;
			return 1<<TWO_SHORT_RECOVER;
		}
		else if (((bStatus & g_bSlickStatus[1][i]) || (!fZeroI)) && (g_fStatus_one_phase_groud[wPN][i] == true))//接地恢复或相间短路恢复，并且发生
		{
			DTRACE(0, ("%s Point %d, 1 phase groud recover!\r\n", __FUNCTION__, wPN));
			g_fStatus_one_phase_groud[wPN][i] = false;
			return 1<<ONE_GROUD_RECOVER;
		}
	}
	return 0;
}*/
/*
void PhaseOffEvent(WORD wPn, int* piU, int* piI, TSHJPara *pSHJPara)
{
    for (int i = 0; i < 3; i++) 
    {
        if ((*piU++ < pSHJPara->dwUOff) & (*piI++ < pSHJPara->dwIOff)) 
			goto OFF_NOW;
    }
//都没有断相
	if (g_AcSample.dwOffRecoverTime[wPn] == 0) 
	{
		g_AcSample.dwOffStartTime[wPn] = 0;
		g_AcSample.dwOffRecoverTime[wPn] = GetClick();
		g_AcSample.fPowerOff[wPn] = false;
	}
	if ((g_AcSample.fOffRecover[wPn] == false) & (GetClick() - g_AcSample.dwOffRecoverTime[wPn] >= pSHJPara->wOffTime))
	{
		g_AcSample.fOffRecover[wPn] = true;
		SaveSOE(wPn, 0xc027, 0);//系統断相
	}
	return;

OFF_NOW:
	if (g_AcSample.dwOffStartTime[wPn] == 0)
	{
		g_AcSample.dwOffStartTime[wPn] = GetClick();
		g_AcSample.dwOffRecoverTime[wPn] = 0;
		g_AcSample.fOffRecover[wPn] = false;
	}
	if ((g_AcSample.fPowerOff[wPn] == false) & (GetClick() - g_AcSample.dwOffStartTime[wPn] >= pSHJPara->wOffTime))
	{
		g_AcSample.fPowerOff[wPn] = true;
		SaveSOE(wPn, 0xc027, 1);//系統断相
	}
	return;
}*/
//描述：检测两相接地短路
void GroundShortDetec(WORD wPn, int iIa, int iIb, int iIc, int iIn, TSHJPara *pSHJPara)
{
	BYTE bStatus;

	WORD wStatus = PhaseFaultDetection(wPn, iIa, iIb, iIc, pSHJPara);

#ifdef DEBUG
	Status++;
	if (Status == 0)
	{
		DTRACE(0, ("%s Point %d, iIa %d, iIb %d, iIc %d, iIn %d status %x!\r\n", __FUNCTION__, wPn, iIa, iIb, iIc, iIn, wStatus));
	}
#endif

	wStatus = Det2Status(wPn, wStatus, (bool)(iIn > pSHJPara->dwI0));//进行异常的类型监视

	if (((wStatus & (1<<TWO_SHORT))) || ((wStatus & (1<<TWO_SHORT_RECOVER))))//两相接地，发生
	{
		if (wStatus & (1<<TWO_SHORT))
			bStatus = 1;
		else
			bStatus = 0;
		
		SaveSOE(wPn, 0xc022, bStatus);
		return;
	}

	if ((wStatus & (1<<TWO_SHORT_GROUD)) || (wStatus & (1<<TWO_SHORT_GROUD_RECOVER)))
	{
		if (wStatus & (1<<TWO_SHORT_GROUD))
			bStatus = 1;
		else 
			bStatus = 0;

		SaveSOE(wPn, 0xc023, bStatus);
	}

	if ((wStatus & (1<<THREE_SHORT)) || (wStatus & (1<<THREE_SHORT_RECOVER)))
	{
		if (wStatus & (1<<THREE_SHORT))
			bStatus = 1;
		else
			bStatus = 0;

		SaveSOE(wPn, 0xc021, bStatus);
	}

	if ((wStatus & (1<<ONE_GROUD)) || (wStatus & (1<<ONE_GROUD_RECOVER)))
	{
		if (wStatus & (1<<ONE_GROUD))
			bStatus = 1;
		else
			bStatus = 0;

		SaveSOE(wPn, 0xc024, bStatus);
	}
}

#if 0
//描述：零序电流过流、零序电压过压检测
bool ZeroCVOverDetection(WORD wPn, int iIn, int iUn)
{
/*	DWORD dwInmax, dwUnmax;
	BYTE bBuf[12];

	ReadItemEx(BN0, PN0, 0x8952, bBuf);
	g_tSHJPara.dwZeroI;
	dwInmax = BcdToDWORD(bBuf, 4);
	ReadItemEx(BN0, PN0, 0x8953, bBuf);
	dwUnmax = BcdToDWORD(bBuf, 4);
*/
	if (iIn > g_tSHJPara.dwZeroI)//发生
	{
        if (g_InOccClick[wPn] == 0) 
        {
            g_InOccClick[wPn] = GetClick();
            g_InRecClick[wPn] = 0;
        }
        else if ((GetClick() - g_InOccClick[wPn] > g_tSHJPara.wInOverTime) && (g_fInOccur[wPn] == false))
        {
            SaveSOE(wPn, 0xc025, 1);
            g_fInOccur[wPn] = true;
            g_fInRec[wPn] = false;
            DTRACE(0, ("%s Point %d, In over Occur!\r\n", __FUNCTION__, wPn));
        }
	}
    else if (iIn < g_tSHJPara.dwZeroI)//恢复
    {
        if (g_InRecClick[wPn] == 0) 
        {
            g_InOccClick[wPn] = 0;
            g_InRecClick[wPn] = GetClick();
        }
        else if ((GetClick() - g_InRecClick[wPn] > g_tSHJPara.wInOverTime) && (g_fInRec[wPn] == false))
        {
            SaveSOE(wPn, 0xc025, 0);
            g_fInRec[wPn] = true;
            g_fInOccur[wPn] = false;
            DTRACE(0, ("%s Point %d, In over Recover!\r\n", __FUNCTION__, wPn));
        }
    }

	if (iUn > g_tSHJPara.dwZeroU)//零序电压过压发生
	{
        if (g_UnOccClick[wPn] == 0) 
        {
            g_UnOccClick[wPn] = GetClick();
            g_UnRecClick[wPn] = 0;
        }
        else if ((GetClick() - g_UnOccClick[wPn] > g_tSHJPara.wUnOverTime) && (g_fUnOccur[wPn] == false))
        {
            SaveSOE(wPn, 0xc026, 1);
            g_fUnOccur[wPn] = true;
            g_fUnRec[wPn] = false;
            DTRACE(0, ("%s Point %d, Un over Occur!\r\n", __FUNCTION__, wPn));
        }
	}
    else if (iUn < g_tSHJPara.dwZeroU)//零序电压过压恢复
    {
        if (g_UnRecClick[wPn] == 0) 
        {
            g_UnOccClick[wPn] = 0;
            g_UnRecClick[wPn] = GetClick();
        }
        else if ((GetClick() - g_UnRecClick[wPn] > g_tSHJPara.wUnOverTime) && (g_fUnRec[wPn] == false))
        {
            SaveSOE(wPn, 0xc026, 0);
            g_fUnRec[wPn] = true;
            g_fUnOccur[wPn] = false;
            DTRACE(0, ("%s Point %d, Un over Recover!\r\n", __FUNCTION__, wPn));
        }
    }

	return true;
}
#endif


//描述：保存SOE事件库，更新写指针
void SaveSOE(WORD wPn, const WORD wID, BYTE bYxValue)
{/*
    TYXData tYxData;
    BYTE bPtr = 0;
	BYTE bBuf[512];
	BYTE *p = bBuf;
	unsigned long long uMsec = GetCurTimeMs();
	if (!IS_ENVET_OPERATION(wID-0xc021))//ID无效的时候，就不进行事件保存，在检测的时候也要进行判断
		return;

	memset(&tYxData, 0, sizeof(tYxData));
    ReadItemDI(&g_AcSample.ptSOEPtr, bBuf);		//SOE库
	bPtr = bBuf[0];	//SOE指针
	p += sizeof(TYXData)*(bPtr++) + 1;
	if (bPtr >= 32)
        bPtr = 0;
	bBuf[0] = bPtr;

	tYxData.bPn = (BYTE)wPn;	//测量点
	tYxData.wID = wID;			//遥信ID
	Uint64ToBCD(uMsec, tYxData.bTimeStamp, 6);
	tYxData.bValue[0] = bYxValue;	//YX数据

	memcpy(p, &tYxData, sizeof(TYXData));
	WriteItemDI(&g_AcSample.ptSOEPtr, bBuf);
	WriteItemEx(BN0, wPn, wID, tYxData.bValue);*/
}


//描述：各种故障检测，生成遥信事件
void DoFaultJugement()
{/*
	int iCurrentVal[4];//排序时Ia、Ib、Ic、I0	
	
	GetCurrent(iCurrentVal);
	if (g_AcSample.AcPara.wWorkMode == WORK_MODE_GUANGZHOU1)
	{        
    	PhaseOffEvent(g_AcSample.wPn, &g_AcSample.iValue[], iCurrentVal, &g_tSHJPara);		
		ZeroCVOverDetection(g_AcSample.wPn, iCurrentVal[3], 0);
		GroundShortDetec(g_AcSample.wPn, iCurrentVal[0], iCurrentVal[1], \
						 iCurrentVal[2], iCurrentVal[3], &g_tSHJPara);
	}
	*/
}

//使用Ia、Ic和I0做Ib
/*
int CalcIb(BYTE bIaCh, BYTE bIcCh, BYTE bI0Ch, int* piAngleIb)
{
	float fRe, fIm;
	fRe = (float)g_AcSample.iValue[bI0Ch]* cos(ANGLE(g_AcSample.iAngle[6])) - \
		  (float)g_AcSample.iValue[bIaCh]* cos(ANGLE(g_AcSample.iAngle[3])) - \
		  (float)g_AcSample.iValue[bIcCh]* cos(ANGLE(g_AcSample.iAngle[5]));

	fIm = (float)g_AcSample.iValue[bI0Ch]* sin(ANGLE(g_AcSample.iAngle[6])) - \
		  (float)g_AcSample.iValue[bIaCh]* sin(ANGLE(g_AcSample.iAngle[3])) - \
		  (float)g_AcSample.iValue[bIcCh]* sin(ANGLE(g_AcSample.iAngle[5]));

	if (piAngleIb!=NULL) 
	{
		*piAngleIb = 180.0 * 10 * (float)atan2f(fIm, fRe) / 3.1415926 ;
		*piAngleIb = (*piAngleIb + 3600 - g_AcSample.iAngleUa) % 3600;
	}
	
    return (int)(sqrt(fRe*fRe + fIm*fIm));
}

int CalcuI0(int Ia, int Ib, int Ic, int* piAngel)
{
    float fIa = Ia;
    float fIb = Ib;
    float fIc = Ic;
    float AngleA;
    float AngleB;
    float AngleC;
    float fI0 = 0;
    float Ireal,Iimg;

    if (g_AcSample.AcPara.bConnectType == CONNECT_3P4W)
    {
        AngleA = (float)ANGLE(*piAngel);
        piAngel++;
        AngleB = (float)ANGLE(*piAngel);
        piAngel++;
        AngleC = (float)ANGLE(*piAngel);
        piAngel++;

        Ireal = Ia*cos(AngleA) + Ib*cos(AngleB) + Ic*cos(AngleC);
        Iimg = Ia*sin(AngleA) + Ib*sin(AngleB) + Ic*sin(AngleC);
        fI0 = sqrt(Ireal*Ireal+Iimg*Iimg);
    }
    else //三相三线没有零序电流
    {
        fI0 = 0;
    }
    return fI0;
}*/

//TODO:tunning 
void GetCurrent(int* ipVal)
{
#if 0
	int* pVal = ipVal;
	BYTE bCurrenrCh[4];
	BYTE* bpChannel = RAW_IA;
	BYTE* bpProChannel = RAW_IA;
	for (int i = 0; i < 4; i++,bpProChannel++,bpChannel++,pVal++)
	{
		if (m_iValue[*bpChannel] > m_iCurrentVal)
		{	
			*pVal = m_iValue[*bpProChannel];
			bCurrenrCh[i] = *bpProChannel;
		}
		else
		{	
			*pVal = m_iValue[*bpChannel];
			bCurrenrCh[i] = *bpChannel;
		}
	}
	
	if ((m_AcPara.wWorkMode == WORK_MODE_GUANGZHOU1) || (m_AcPara.wWorkMode == WORK_MODE_GUANGZHOU2))
		*(ipVal + 1) = CalcIb(bCurrenrCh[0], bCurrenrCh[2], bCurrenrCh[3], NULL);
	else
		*(ipVal + 3) = m_iI0[0];
#endif
}

