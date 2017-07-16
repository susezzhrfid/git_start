/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件对73360采集到的样点进行计算,算出有效值,功率,功率因数,频率,电能,相角等
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef ACSAMPLE_H
#define ACSAMPLE_H

#include "Typedef.h"
#include "stdlib.h"
//#include "filter.h"
#include "math.h"
#include "sysarch.h"
//#include "Sample.h"
#include "AcFmt.h"
#include "DbConst.h"
#include "LibAcConst.h"
#include "LibDbStruct.h"
#include "filter2.h"

#define CT_NUM      64
#define CT_AVGNUM   15

#define VAILD_V     300   //小于3V的电压都是无效的
#define VAILD_A     50    //小于5ma的电流是无效的

typedef struct {
	WORD wPoint;  		//测量点
	//WORD wWorkMode;		//工作模式:0:深圳模式;1：广州模式;其它：深圳模式
	BYTE bConnectType;  //终端接线方式 1	1:单相;3:三项三线;4:三相四线
	DWORD dwUn;		   	//额定电压,格式NNNNN.N
	DWORD dwIn;		   	//额定电流,格式NNN.NNN
	BYTE bAngleClockwise;	//角度方向,0表示角度按照逆时针方向表示,Ua,Ub,Uc分别为0,240,120
							//		   1表示角度按照顺时针方向表示,Ua,Ub,Uc分别为0,120,240
	bool fCalcuHarmonic;	//是否计算谐波
	WORD wHarmNum;			//谐波计算次数
}TAcPara;	//交采参数,某些字段修改后电能计算程序可即时刷新

#pragma pack(1)        //1字节对齐
typedef struct {
	int iValue;		//值
	WORD wDesBits;	//品质描述串
}TYCData;
#pragma pack()

//#pragma pack(1)
typedef struct {
	BYTE bPn;	//测量点
	WORD wID;	//遥信ID
	BYTE bTimeStamp[6];		//时标
	BYTE bValue[3];	//YX数据
}TYXData;
//#pragma pack()

typedef struct {
	DWORD dwIh;		//故障电流定值
	DWORD dwIno;	//无流定值
	WORD wT1Min;	//过流时间整点值(最小值)
	WORD wT1Max;	//过流时间整点值(最大值)
	WORD wT2;		//重合闸时间整定值
	WORD wT3;		//后加速时间整定值
	DWORD dwI0;		//零序电流突变量整定值
	DWORD dwZeroI;	//零序电流整定值
    DWORD dwZeroU;	//零序电压整定值
    WORD wInOverTime;//零序电流过流报警延迟时间
    WORD wUnOverTime;//零序电压过流报警延迟时间
    DWORD dwUOff;   //断相电压阀值
    DWORD dwIOff;   //断相电流阀值
    WORD wOffTime;  //断相持续时间
}TSHJPara;		//Short-Circuit Judgement parameters

typedef struct {
	WORD wChannelNum;//小电流通道数量      //测量点
	BYTE* bpChannelPtr;//通道指针
	WORD wProChannelNum;//保护（大）电流通道数量
	BYTE* bpProChannelPtr;//通道指针
}TChannelCfg;

#define PHASE_A		0
#define PHASE_B		1
#define PHASE_C		2
#define PHASE_ALL   3

#define PN_ALL      1       //支持的交采测量点


bool AcLoadPara(WORD wPn, TAcPara* pAcPara);
extern	TSHJPara g_tSHJPara;	//Short-Circuit Judgement parameters

typedef struct {
    int iValue[SCN_NUM];  //有效值      电流电压，还有保护电压，电流
    //int iP[32];     //分别是A,B,C和总
    //int iQ[32];     //分别是A,B,C和总
    //int iS[32];	 //分别是A,B,C和总
    //int iCos[32];   //分别是A,B,C和总
    //int iFreq[16];
	//int iAngle[SCN_NUM];    
	//TYCData tVal[AC_VAL_NUM];
    //int iI0[1];//零序电流
    //int iU0[1];//零序电压
	
	WORD wPn;
	//bool fParaChg;
	TAcPara AcPara;
	bool fAdjParaSet;

	//参数配置
	//DWORD dwLastAutoDate;
	//DWORD dwCTRatio;
	//DWORD dwPTRatio;
	//DWORD dwPowerRatio;
	//DWORD dwUn;
	//DWORD dwIn;
	
	//频率跟踪
	DWORD dwFreqPnts;  //频率跟踪FREQ_CYC_NUM个周期(标准每周期NUM_PER_CYC点)点数 * FREQ_UNIT
	WORD wZeroCnt;     //频率跟踪已经跟踪到的过零点数
	WORD wFreqPntCnt;  //频率跟踪的参与计算频率两个过零点间的点数
	WORD wZeroPntCnt;	 //频率跟踪的相邻两个过零点间的点数
	WORD wFreqCn;      //用来计算频率跟踪的通道只能是0,1,2即Ua,Ub,Uc
	bool fPrePos;      //频率跟踪的上一个样点是正数
	short sZero1[3];   //过零点1,分别存放:正、负、位置索引
	short sZero2[3];   //过零点2,分别存放:正、负、位置索引
	WORD  wFreqRstCnt;
	DWORD  dwFreq;		 //信号频率值,比如50HZ表示为50000
	
	//电能及采样计算
	//int iBarrelEp[4];	//有功累计到A,B,C,总有功电能累计的桶
	//int iBarrelEq[4]; //无功累计到A,B,C,总无功电能累计的桶
	//long long  iSigmaEP[4];  	//计算有功电能和功率用到的累加值,为了保留更多的有效位,且不至于大电流电压的时候溢出,所以用64位long long
	//long long  iSigmaEQ[4];     //计算无功电能和功率用到的累加值
	//long long  dwSigmaValue[SCN_NUM];	//计算有效值用到的累加值
	int  iDcValue[SCN_NUM];		//每个通道的直流分量
	//int  iDcSum[SCN_NUM];
	//WORD wSigmaPtr;				//使用SIGMA算法的指针
	//WORD wShiftPtrQ;			    //计算无功时电压相对于电流往前移相90度的指针
	//int  iShiftFracP[3];			//有功相位校正的小数部分
	//int  iShiftFracQ[3];			//无功90度移相及相位校正的小数部分
	//WORD wSigmaPntCnt;			//SIGMA累计的点数
	//int  iFracP[3];				//相位校正角度	
	int  iAdj[SCN_NUM*2];       //每个通道的矢量校正参数,2个值,包括实部和虚部
	TDataItem diAdjPara;

	//WORD wAdjStep;
	
//	WORD m_wHarPercent[HARM_NUM_MAX*SCN_NUM];	//谐波含有率
//	WORD m_wHarVal[HARM_NUM_MAX*SCN_NUM];		//谐波有效值
//	TYCData  tPOH[HARM_NUM_MAX*6]; 			//percentage of harmonics        //需要占520B RAM

    //DWORD dwOffStartTime[SCN_NUM];
    //DWORD dwOffRecoverTime[SCN_NUM];
    //bool fPowerOff[SCN_NUM];//交流采样检测到调电了
    //bool fOffRecover[SCN_NUM];

	//入库数据项
	//TDataItem diAcVect;			//交采基波矢量
	//TDataItem diYcBlk[MAX_PN_NUM];	//YC数据块   //库里只有终端的数据
    
	//TDataItem ptSOEPtr;
    
    //int iCurrentVal;//保护电流阀值
    
    //int iGZIb[PN4];//广州的特殊数据项
    int iAngleUa;
    //TChannelCfg tChannelCfg;
}TAcSample;

bool Calcu();     		//每周波调用计算一次,算出有效值,功率,功率因数,频率,电能
bool AcSampleInit(WORD wPn);    //初始化内部成员
void Transfer();  //数据转换到数据库中
bool TrigerAdj(BYTE* bBuf);
//void TrigerAdj2(void);
void InitAdj();
//bool IsPowerOff();   //交流采样检测到掉电了

	//频率跟踪
void GetZero(short* psZero, WORD wFreqPtr, WORD wPtrInBuf);
int CalcuFreq(short* psZero1, short* psZero2, WORD wFreqPntCnt);
void ResetFreqSync();
void FreqSync(void);

void UpdateFreq();
bool CopyToFftBuf();
int CalcuRMS(complex_fract32* cplxIn);

int CalcuPQCOS();

int CalcuP(complex_fract16* cplxI, complex_fract16* cplxV);
int CalcuQ(complex_fract16* cplxI, complex_fract16* cplxV);

void CalcuS();
long CalcuCos(long P,long Q);
void CalcuAngle();
int  CalcuI0();
void RunMeter();
void PhaseAdjust();
void AdErrCheck(bool fCaluTimeout);
	//void VectAdj(complex_fract16* cplx, int adjreal, int adjimag);
void VectAdj(complex_fract16* incplx, complex_fract32* outcplx, int adjreal, int adjimag);
//void VectAdj(complex_fract16* incplx, complex_fract32* outcplx, complex_fract32* outUnAjdcplx, int adjreal, int adjimag);
	
void LoadPara();

	//谐波含量计算
void DoHarmonic();
unsigned short CalcuTotalHarmonic(complex_fract32* cplx, int iBaseHarVal, int* piBasePercent);
unsigned short CalcuHarmonic(complex_fract32* cplx, int total);
void SaveYCData(void);

	//故障判别
void DoFaultJugement(void);
int IsPhaseXShortCircuit(BYTE bPn, int iIx, BYTE bPhase, TSHJPara *pSHJPara);
WORD PhaseFaultDetection(BYTE bPn, int iIa, int iIb, int iIc, TSHJPara *pSHJPara);
void GroundShortDetec(WORD wPn, int iIa, int iIb, int iIc, int iIn, TSHJPara *pSHJPara);
//void PhaseOffEvent(WORD wPn, int* piU, int* piI, TSHJPara *pSHJPara);
bool ZeroCVOverDetection(WORD wPn, int iIn, int iUn);
WORD Det2Status(WORD wPN, WORD wState, bool fZeroI);
void SaveSOE(WORD wPn, WORD wID, BYTE bYxValue);

void GetCurrent(int* ipVal);
int CalcuI0(int Ia, int Ib, int Ic, int* piAngel);

	//下面部分需要特殊处理的
	
	//int CalcIb(BYTE bIaCh, BYTE bIcCh, BYTE bI0Ch);//使用Ia、Ic和I0做Ib
int CalcIb(BYTE bIaCh, BYTE bIcCh, BYTE bI0Ch, int* piAngleIb);//使用Ia、Ic和I0做Ib
void CalcI0();
void CalcU0();

int CorrectVol(int iVal);

#endif //ACSAMPLE_H
