/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ���73360�ɼ�����������м���,�����Чֵ,����,��������,Ƶ��,����,��ǵ�
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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

#define VAILD_V     300   //С��3V�ĵ�ѹ������Ч��
#define VAILD_A     50    //С��5ma�ĵ�������Ч��

typedef struct {
	WORD wPoint;  		//������
	//WORD wWorkMode;		//����ģʽ:0:����ģʽ;1������ģʽ;����������ģʽ
	BYTE bConnectType;  //�ն˽��߷�ʽ 1	1:����;3:��������;4:��������
	DWORD dwUn;		   	//���ѹ,��ʽNNNNN.N
	DWORD dwIn;		   	//�����,��ʽNNN.NNN
	BYTE bAngleClockwise;	//�Ƕȷ���,0��ʾ�ǶȰ�����ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,240,120
							//		   1��ʾ�ǶȰ���˳ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,120,240
	bool fCalcuHarmonic;	//�Ƿ����г��
	WORD wHarmNum;			//г���������
}TAcPara;	//���ɲ���,ĳЩ�ֶ��޸ĺ���ܼ������ɼ�ʱˢ��

#pragma pack(1)        //1�ֽڶ���
typedef struct {
	int iValue;		//ֵ
	WORD wDesBits;	//Ʒ��������
}TYCData;
#pragma pack()

//#pragma pack(1)
typedef struct {
	BYTE bPn;	//������
	WORD wID;	//ң��ID
	BYTE bTimeStamp[6];		//ʱ��
	BYTE bValue[3];	//YX����
}TYXData;
//#pragma pack()

typedef struct {
	DWORD dwIh;		//���ϵ�����ֵ
	DWORD dwIno;	//������ֵ
	WORD wT1Min;	//����ʱ������ֵ(��Сֵ)
	WORD wT1Max;	//����ʱ������ֵ(���ֵ)
	WORD wT2;		//�غ�բʱ������ֵ
	WORD wT3;		//�����ʱ������ֵ
	DWORD dwI0;		//�������ͻ��������ֵ
	DWORD dwZeroI;	//�����������ֵ
    DWORD dwZeroU;	//�����ѹ����ֵ
    WORD wInOverTime;//����������������ӳ�ʱ��
    WORD wUnOverTime;//�����ѹ���������ӳ�ʱ��
    DWORD dwUOff;   //�����ѹ��ֵ
    DWORD dwIOff;   //���������ֵ
    WORD wOffTime;  //�������ʱ��
}TSHJPara;		//Short-Circuit Judgement parameters

typedef struct {
	WORD wChannelNum;//С����ͨ������      //������
	BYTE* bpChannelPtr;//ͨ��ָ��
	WORD wProChannelNum;//�������󣩵���ͨ������
	BYTE* bpProChannelPtr;//ͨ��ָ��
}TChannelCfg;

#define PHASE_A		0
#define PHASE_B		1
#define PHASE_C		2
#define PHASE_ALL   3

#define PN_ALL      1       //֧�ֵĽ��ɲ�����


bool AcLoadPara(WORD wPn, TAcPara* pAcPara);
extern	TSHJPara g_tSHJPara;	//Short-Circuit Judgement parameters

typedef struct {
    int iValue[SCN_NUM];  //��Чֵ      ������ѹ�����б�����ѹ������
    //int iP[32];     //�ֱ���A,B,C����
    //int iQ[32];     //�ֱ���A,B,C����
    //int iS[32];	 //�ֱ���A,B,C����
    //int iCos[32];   //�ֱ���A,B,C����
    //int iFreq[16];
	//int iAngle[SCN_NUM];    
	//TYCData tVal[AC_VAL_NUM];
    //int iI0[1];//�������
    //int iU0[1];//�����ѹ
	
	WORD wPn;
	//bool fParaChg;
	TAcPara AcPara;
	bool fAdjParaSet;

	//��������
	//DWORD dwLastAutoDate;
	//DWORD dwCTRatio;
	//DWORD dwPTRatio;
	//DWORD dwPowerRatio;
	//DWORD dwUn;
	//DWORD dwIn;
	
	//Ƶ�ʸ���
	DWORD dwFreqPnts;  //Ƶ�ʸ���FREQ_CYC_NUM������(��׼ÿ����NUM_PER_CYC��)���� * FREQ_UNIT
	WORD wZeroCnt;     //Ƶ�ʸ����Ѿ����ٵ��Ĺ������
	WORD wFreqPntCnt;  //Ƶ�ʸ��ٵĲ������Ƶ������������ĵ���
	WORD wZeroPntCnt;	 //Ƶ�ʸ��ٵ���������������ĵ���
	WORD wFreqCn;      //��������Ƶ�ʸ��ٵ�ͨ��ֻ����0,1,2��Ua,Ub,Uc
	bool fPrePos;      //Ƶ�ʸ��ٵ���һ������������
	short sZero1[3];   //�����1,�ֱ���:��������λ������
	short sZero2[3];   //�����2,�ֱ���:��������λ������
	WORD  wFreqRstCnt;
	DWORD  dwFreq;		 //�ź�Ƶ��ֵ,����50HZ��ʾΪ50000
	
	//���ܼ���������
	//int iBarrelEp[4];	//�й��ۼƵ�A,B,C,���й������ۼƵ�Ͱ
	//int iBarrelEq[4]; //�޹��ۼƵ�A,B,C,���޹������ۼƵ�Ͱ
	//long long  iSigmaEP[4];  	//�����й����ܺ͹����õ����ۼ�ֵ,Ϊ�˱����������Чλ,�Ҳ����ڴ������ѹ��ʱ�����,������64λlong long
	//long long  iSigmaEQ[4];     //�����޹����ܺ͹����õ����ۼ�ֵ
	//long long  dwSigmaValue[SCN_NUM];	//������Чֵ�õ����ۼ�ֵ
	int  iDcValue[SCN_NUM];		//ÿ��ͨ����ֱ������
	//int  iDcSum[SCN_NUM];
	//WORD wSigmaPtr;				//ʹ��SIGMA�㷨��ָ��
	//WORD wShiftPtrQ;			    //�����޹�ʱ��ѹ����ڵ�����ǰ����90�ȵ�ָ��
	//int  iShiftFracP[3];			//�й���λУ����С������
	//int  iShiftFracQ[3];			//�޹�90�����༰��λУ����С������
	//WORD wSigmaPntCnt;			//SIGMA�ۼƵĵ���
	//int  iFracP[3];				//��λУ���Ƕ�	
	int  iAdj[SCN_NUM*2];       //ÿ��ͨ����ʸ��У������,2��ֵ,����ʵ�����鲿
	TDataItem diAdjPara;

	//WORD wAdjStep;
	
//	WORD m_wHarPercent[HARM_NUM_MAX*SCN_NUM];	//г��������
//	WORD m_wHarVal[HARM_NUM_MAX*SCN_NUM];		//г����Чֵ
//	TYCData  tPOH[HARM_NUM_MAX*6]; 			//percentage of harmonics        //��Ҫռ520B RAM

    //DWORD dwOffStartTime[SCN_NUM];
    //DWORD dwOffRecoverTime[SCN_NUM];
    //bool fPowerOff[SCN_NUM];//����������⵽������
    //bool fOffRecover[SCN_NUM];

	//���������
	//TDataItem diAcVect;			//���ɻ���ʸ��
	//TDataItem diYcBlk[MAX_PN_NUM];	//YC���ݿ�   //����ֻ���ն˵�����
    
	//TDataItem ptSOEPtr;
    
    //int iCurrentVal;//����������ֵ
    
    //int iGZIb[PN4];//���ݵ�����������
    int iAngleUa;
    //TChannelCfg tChannelCfg;
}TAcSample;

bool Calcu();     		//ÿ�ܲ����ü���һ��,�����Чֵ,����,��������,Ƶ��,����
bool AcSampleInit(WORD wPn);    //��ʼ���ڲ���Ա
void Transfer();  //����ת�������ݿ���
bool TrigerAdj(BYTE* bBuf);
//void TrigerAdj2(void);
void InitAdj();
//bool IsPowerOff();   //����������⵽������

	//Ƶ�ʸ���
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

	//г����������
void DoHarmonic();
unsigned short CalcuTotalHarmonic(complex_fract32* cplx, int iBaseHarVal, int* piBasePercent);
unsigned short CalcuHarmonic(complex_fract32* cplx, int total);
void SaveYCData(void);

	//�����б�
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

	//���沿����Ҫ���⴦���
	
	//int CalcIb(BYTE bIaCh, BYTE bIcCh, BYTE bI0Ch);//ʹ��Ia��Ic��I0��Ib
int CalcIb(BYTE bIaCh, BYTE bIcCh, BYTE bI0Ch, int* piAngleIb);//ʹ��Ia��Ic��I0��Ib
void CalcI0();
void CalcU0();

int CorrectVol(int iVal);

#endif //ACSAMPLE_H
