/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ���73360�ɼ�����������м���,�����Чֵ,����,��������,Ƶ��,����,��ǵ�
 * ��ǰ�汾��������VER_STR
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 ---------------------------------------------------------------------------------------------------------
* �汾��Ϣ:
 ---2009-4-16:---V1.04----᯼���---
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

#define PRINTF_AD_DATA     0 //AD ԭʼֵ, �����ܲ�     ,��ӡ������
#define PRINTF_AD_ONEPOINT 0 //AD ԭʼֵ��ÿ�ܲ�ֻ��һ��
#define PRINTF_IEVALUE     0 //AD FFT��δУ׼����Чֵ
#define PRINTF_FREQ        0 //Ƶ�ʸ��ٲ�����ź�Ƶ��

#define FFT                0 //1-FFT��0-��ʹ��FFT�㷨,ʹ�þ������㷨

#define CHANNLE_NONE	0//��Чͨ��
#define UA	1//A���ѹ
#define UB	2//B���ѹ
#define UC	3//C���ѹ
#define IA	4//A�����
#define IB	5//B�����
#define IC	6//C�����
#define U0	7//C�����
#define I0	8//�������
#define IAP	9//A�ౣ������
#define IBP	10//A�ౣ������
#define ICP	11//A�ౣ������

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
#define CHANNEL_NULL  16//��Ч��û��ʹ��

#define MAX_CHANNEL_NUM		16

//�¼���ʾ��˳��
#define 	EVENT_FLAG_3_SHORT				0//�����·
#define 	EVENT_FLAG_2_SHORT				1//2���·
#define 	EVENT_FLAG_2_SHORT_GROUND		2//2��ӵض�·
#define 	EVENT_FLAG_1_SHORT_GROUND		3//����ӵض�·
#define 	EVENT_FLAG_I0_OVER				4//�����������
#define 	EVENT_FLAG_U0_OVER				5//�����ѹ����
#define 	EVENT_FLAG_PHASE_OFF			6//���߹���

#define 	TWO_SHORT					0
#define 	TWO_SHORT_RECOVER			1
#define 	TWO_SHORT_GROUD				2
#define 	TWO_SHORT_GROUD_RECOVER		3
#define 	THREE_SHORT					4
#define 	THREE_SHORT_RECOVER			5
#define 	ONE_GROUD					6
#define 	ONE_GROUD_RECOVER			7

#define RAW_UA	0//A���ѹ
#define RAW_UB	1//B���ѹ
#define RAW_UC	2//C���ѹ
#define RAW_IA	3//A�����
#define RAW_IB	4//B�����
#define RAW_IC	5//C�����
#define RAW_U0	6//�����ѹ
#define RAW_I0	7//�������

//static DWORD g_dwEventFlag;//�¼���ʶ
//#define 	IS_ENVET_OPERATION(x)	((g_dwEventFlag & (0x01<<x)) != 0)
//#define 	IS_ENVET_ID_OPERATION(x)	((g_dwEventFlag & (0x01<<(x-0xc021))) != 0)

//static bool g_fStatus[PN_ALL][PHASE_ALL];//����·
//static bool g_fStatus_groud[PN_ALL][PHASE_ALL];//���ӵض�·

//static bool g_fStatusABC[PN_ALL];//�����·
//static bool g_fStatus_one_phase_groud[PN_ALL][PHASE_ALL];//���൥·

//#pragma align   16
//�ڲ���Ա����,�������ﶨ��,������ֶ��������
static complex_fract16 m_cplxFftIn[SCN_NUM][FFT_NUM];  //������ԣ��,���ӿɿ��� 
static complex_fract32 m_cplxFftOut[SCN_NUM][FFT_NUM]; //������ԣ��,���ӿɿ��� +2��������͵�ѹ	
//static complex_fract32 m_cplxFftOutUnAdj[SCN_NUM][FFT_NUM]; //������ԣ��,���ӿɿ��� +2��������͵�ѹ	

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

//static DWORD g_dwTimeStart[PN_ALL][PHASE_ALL] = {0};//����ʱ��
//static DWORD g_dwTimeRetray[PN_ALL][PHASE_ALL] = {0};//�ָ�ʱ��

//��ʼ���ڲ���Ա,��һЩ��������,����߷�ʽ��У��������
//�㽭��͹����Ĳ�ͬ�����ӿڣ����԰�װ�ڱ�������
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
    
	g_AcSample.wFreqCn = 1;   //��ֹ���ϵ�ResetFreqSync()��ѡ
    
    g_AcSample.wFreqRstCnt = 0;
	
	g_AcSample.diAdjPara = GetItemEx(BN25, PN0, 0x8960);	//У������

	//g_AcSample.diAcVect = GetItemEx(BN2, PN0, 0xba10);	//���ɻ���ʸ��

	//g_diHarPercent = GetItemEx(BN0, wPn, 0xB7FF); //��ǰA��B��C�����ѹ������,2~N��г��������//todo

	//for (i=1; i<MAX_PN_NUM; i++) //YC���ݿ�
	//{
		//g_AcSample.diYcBlk[i] = GetItemEx(BN0, i, 0xb6ff); //todo
	//}
    //g_AcSample.diYcBlk[0] = GetItemEx(BN0, PN0, 0xb6ff); //todo

	g_AcSample.fAdjParaSet = true;	//Ĭ�ϴ�FLASH�ж���

    DTRACE(DB_CRITICAL, ("CAcSample::Init: "VER_STR" init ok\n"));

	return true;
}

void LoadPara()
{
	if (GetInfo(INFO_AC_PARA) == false)
		return;	

	WORD wPn = GetAcPn();	//ȡ�ý��ɵĲ������
	if (wPn != g_AcSample.wPn)		//������ŷ����˸ı�
	{
		g_AcSample.wPn = wPn;
		//InitAcValToDb(wPn);	//��ʼ�������������Ŀ��ƽṹ
	}	

	AcLoadPara(g_AcSample.wPn, &g_AcSample.AcPara);
}

//��������ʼ��У������ÿһ������У��֮ǰ���ô˽ӿڳ�ʼ��
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
}   //����������⵽������
*/

void UpdateFreq(void)
{    
    DWORD dwFreq;
    int iAddf;
    dwFreq = g_AcSample.dwFreq;    

    //Ƶ��
	//Ĭ��:m_dwFreqPnts = FREQ_CYC_NUM * NUM_PER_CYC * FREQ_UNIT;		
    if (g_AcSample.dwFreqPnts == 0)
        return;
    
    g_AcSample.dwFreq = ((DDWORD)g_dwAdFreq*FREQ_CYC_NUM*FREQ_UNIT)/g_AcSample.dwFreqPnts;//��λ��HZ/1000,���Ŵ���һǧ��
                                                      //����1000û����Ϊ�˰�Ƶ�ʵ�С��������50.000Hz
#if PRINTF_FREQ
    DTRACE(1, ("%d\r\n", g_AcSample.dwFreq));
#endif		    
    WriteItemEx(BN2, PN0, 0x1054, (BYTE *)&g_AcSample.dwFreq); 
	    
    iAddf = g_AcSample.dwFreq - dwFreq;    
    if (iAddf <= 3 && iAddf >= -3)//����+-0.003HZ//Ƶ��ƫ���һ����Χ�Ÿ��²����������Ϊ����һ�ξͻ���һ������һ�����ǾɵĲ�������ɵõ����ݣ�һ�������µĲ�������ɵõ�����
    {
        return;   //
    }
    //�ж����õ������������������µ�ʱ����뱣������
    EnterCritical();
    g_dwAdFreq = g_AcSample.dwFreq * NUM_PER_CYC; //�µĲ���Ƶ��
    
	g_bFreqStep = 0; //Ƶ�ʸ��ٵĵ�ǰ����:0-���ɼ����������һ����Ƶ��;1-AD�жϲ������µ�Ƶ�ʽ��в���        
    ExitCritical();
}

//#define COMPOSE_SAMPLE(ps, p1, p2, frac) ( *(ps + p1) + ( (*(ps + p1) - *(ps + p2)) * frac >> FREQ_SHIFT) )
#define COMPOSE_SAMPLE(ps, p1, p2, frac) ( *(ps + p1) + (*(ps + p2) - *(ps + p1)) * frac / FREQ_UNIT )

//��������AD�Ĳ����������У����ȳ�ȡFFT_NUM�����㣬������FFT�任
bool CopyToFftBuf()
{    
#if PRINTF_AD_DATA    
    DTRACE(1, ("\r\n"));//for test!
#endif
	for (WORD wCn=0; wCn<SCN_NUM; wCn++)
	{		
		for (WORD i=0; i<FFT_NUM; i++)
		{
			m_cplxFftIn[wCn][i].re = g_wAdBuf[wCn+1+i*NUM_CHANNELS];//g_sSampleBuf[wCn][wAcRdPtr++];//��һ���ǵ�ص�ѹ,�ڶ���ʱ���ɣ����������¶�			            
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
//						//p2��p1ǰ��ĵ�

#define COMPOSE_SAMPLE2(ps, p1, p, p2, frac) (frac<0 ? *(ps + p) + ( (*(ps + p2) - *(ps + p)) * -frac >> FREQ_SHIFT )	: \
														*(ps + p) - ( (*(ps + p) - *(ps + p1)) * frac >> FREQ_SHIFT )	)
						//p�ǵ�ǰ�㣬p1��ǰ��һ�㣬p2����һ��
						//fracΪ����ʹ��ѹ�����ͺ󣬲�������������
						//fracΪ����ʹ��ѹ���γ�ǰ����������������
//���ڽǶ�У�������������ӣ�һ̨�ն��ڽǶ�У��ǰ���������
// ���Ե�		���  
//  0.5L		-0.84
//  0.5C		0.85
//	0.8L		-0.275
//	0.8C		0.293

//������������Կ����ն˵ĵ���������ѹ�ͺ���һ���ĽǶȣ�
//��ô��ѹ����ڵ����ĽǶ�У��ֵfracӦΪ������ʾ��ѹ�����ƣ�����С����Ƕ����

//ÿ�ܲ�����һ��,�����Чֵ,����,��������,Ƶ��,����
//���أ�һ���ܲ�������������ɷ���true.���ڲ�������false
//section("L1_code")
bool Calcu()
{	
	LoadPara();
	WORD i;
	//WORD wSamplePtr = SBUF_SIZE;
#if (FFT != 0)
	complex_fract16 cplxFftOut[FFT_NUM];
#endif

	//�����������ݵ�����
    if (!CopyToFftBuf())
		return false;
    
#if (FFT==0)  //�������㷨
    BYTE j;
    long lRms;
    DWORD dwAdData[SCN_NUM]; 
    BYTE bVolt[4];
    static WORD wAdData[SCN_NUM] = {0};
    static BYTE bCnt = 0;   //��ƽ��ֵ���ܲ�����ͳ��
    memset(dwAdData, 0, sizeof(dwAdData));
    for (i=0; i<SCN_NUM; i++)   
    {
        for (j=0; j<FFT_NUM; j++)//���ֵ
        {            
            dwAdData[i] += (DWORD)m_cplxFftIn[i][j].re;
        }
        dwAdData[i] = dwAdData[i]/FFT_NUM; 
        
        for (j=0; j<FFT_NUM; j++)//ȥ��ֱ���ź�
        {
            if (m_cplxFftIn[i][j].re >= dwAdData[i])//������ָ���
                m_cplxFftIn[i][j].re -= dwAdData[i]; 
            else
                m_cplxFftIn[i][j].re = dwAdData[i]-m_cplxFftIn[i][j].re;
        }
    }        
    memset(dwAdData, 0, sizeof(dwAdData));                         
    for (i=0; i<SCN_NUM; i++) //����32�����������
    {
        for (j=0; j<FFT_NUM; j++)
        {                               
            dwAdData[i] += (DWORD)m_cplxFftIn[i][j].re*m_cplxFftIn[i][j].re;//��ƽ����                
        }
        dwAdData[i] = dwAdData[i]/FFT_NUM;
        dwAdData[i] = (DWORD)(sqrt((float)dwAdData[i])); //�����
        wAdData[i] += (WORD)dwAdData[i];   //����ܲ����
    }  
    
    bCnt++;
    if (bCnt >= 25)
    {         
        for (i=0; i<SCN_NUM; i++)//��ͨ������У׼���
        {        
            wAdData[i] = wAdData[i]/bCnt;
#if PRINTF_IEVALUE        
            if (i==0)
            {   
                DTRACE(1, ("%d\r\n", dwAdData[i])); //δУ׼��ֵ
            }
#endif                
            lRms = (long)ConvertToLineVal(i, wAdData[i]);  //У�����
            if (lRms >= 0)
            {       
                g_AcSample.iValue[i] = lRms;
                DWORDToBCD((DWORD)(lRms+5)/10, bVolt, sizeof(bVolt));  //ֻ����һλС��
                WriteItemEx(BN2, PN0, 0x1031+i, bVolt); //У׼���ֵ���
                //DTRACE(1, ("%d\r\n", lRms)); //У׼��ֵ            
            }
            else
            {
                //DTRACE(DB_TASK, ("#### Convert Err!\n"));
            }  
        }
         
        //ԭʼֵ���
        WriteItemEx(BN2, PN0, 0xba11, (BYTE *)wAdData);
        memset(wAdData, 0, sizeof(wAdData));
        
        bCnt = 0;  
    } 
#else

	//��̬װ��У������,������̬У��
	ReadItemDI(&g_AcSample.diAdjPara, (BYTE* )&g_AcSample.iAdj);

	//FFT�任�������ͨ����Чֵ
    for (i=0; i<SCN_NUM; i++) //32��240us���Լ�����һ��ͨ����64��550us
    {
        //SetLed(true, 2);        //test fft cost time
        //memset(&m_cplxFftOut[i], 0, sizeof(m_cplxFftOut[i]));
		//memset(&m_cplxFftOutUnAdj[i], 0, sizeof(m_cplxFftOutUnAdj[i]));
		
    	//cfft_fr16(m_cplxFftIn[i], NULL, cplxFftOut, w, 1, FFT_NUM, 0, 0);
        cfft_fr16(m_cplxFftIn[i], NULL, cplxFftOut, NULL, 1, FFT_NUM, 0, 0); 
        
		//NOTE:complex_fract16��,������ĵ���ֵ�Ƚϴ�(�籣�����������)�������,
        //VectAjd�����н���У��ʱ�����,���Խ�m_cplxFftOut��complex_fract16�͸�Ϊ
        //��complex_fract32��,ʹ���м���ʱ����cplxFftOut����cfft_fr16�ļ���,��VectAjd
        //�н�cplxFftOutֵ������m_cplxFftOut����������ļ���,����VectAjd����������ֱ��
        //������,�������ĵļ�����ʹ�õ���ֱ������,���ܻ�������
        //VectAdj(cplxFftOut, m_cplxFftOut[i], m_cplxFftOutUnAdj[i], g_AcSample.iAdj[i*2], g_AcSample.iAdj[i*2+1]); //ʸ��У��
		VectAdj(cplxFftOut, m_cplxFftOut[i], g_AcSample.iAdj[i<<1], g_AcSample.iAdj[i<<1+1]); //ʸ��У��		
		g_AcSample.iValue[i] = CalcuRMS(m_cplxFftOut[i]);
        
//        WriteItemEx(BN0, PN0, 0xC322+i, (BYTE *)g_AcSample.iValue); //У׼���ֵ���
        //SetLed(false, 2);
        //DTRACE(0, ("%d\r\n", m_iValue[i]));
		//printf("ch %d = %d\r\n", i, m_iValue[i]);
		//TODO���ж�С�����ӵ�
    }
#endif
    
    for (i=0; i<SCN_NUM; i++) 
    {
        if (g_AcSample.iValue[i] < VAILD_V)
            g_AcSample.iValue[i] = 0;        
    }    
    
    //���ϼ��
	//DoFaultJugement(); 
    
   	//Ƶ�ʸ���
	FreqSync();

	//����г��������
	//DoHarmonic();

	//���㹦�ʡ���������
	//CalcuAngle();
	//CalcuPQCOS();
    
    //CalcU0();
    //CalcI0();

 	//Transfer();

	if (g_AcSample.fAdjParaSet) //У׼״̬����false��������
		return false;
	else
		return true;
}

void GetZero(short* psZero, WORD wFreqPtr, WORD wPtrInBuf)
{
    DWORD dwAvg = VOLT_REF>>1;      //����ǻ�׼��һ��
	if (wPtrInBuf == 0)		
        psZero[0] = g_wAdBuf[g_AcSample.wFreqCn+(SBUF_SIZE-1)*NUM_CHANNELS]-dwAvg;  //��ȥƽ��ֵ
	else		
    {
        psZero[0] = g_wAdBuf[g_AcSample.wFreqCn+(wPtrInBuf-1)*NUM_CHANNELS]-dwAvg;
    }					

    psZero[1] = g_wAdBuf[g_AcSample.wFreqCn+wPtrInBuf*NUM_CHANNELS]-dwAvg;
	psZero[2] = wFreqPtr;   //�������ٷ�Χ�����λ��
}

//���ص����������ڲ���Ƶ�ʵĹ�����Ĳ��������������Ŵ���FREQ_UNIT��
int CalcuFreq(short* psZero1, short* psZero2, WORD wFreqPntCnt) 
{
	if (wFreqPntCnt>NUM_PER_MAX_PN || 
		wFreqPntCnt<NUM_PER_MIN_PN)
		return -1;
		
	int t1, t2;
    if ((psZero1[1] == psZero1[0]) || (psZero2[1] == psZero2[0]))
        return -1;
    
	t1 = (int )psZero1[1] * FREQ_UNIT / (psZero1[1] - psZero1[0]);//ϸ�ֹ����Ĳ��֣���߾���
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
	
	//����ȷ��Ƶ�ʸ��ٵ�ͨ��A->C->B               //CL818K5ֻ��һ·����
	/*if (g_AcSample.iValue[0]>=g_AcSample.iValue[1] && g_AcSample.iValue[0]>=g_AcSample.iValue[2])   //��һͨ����Чֵ���͸�����һͨ��
		g_AcSample.wFreqCn = 0;
	else if (g_AcSample.iValue[2] >= g_AcSample.iValue[1])
		g_AcSample.wFreqCn = 2;
	else*/
		g_AcSample.wFreqCn = 1;
		
	g_AcSample.dwFreq = 50000;
		
	g_AcSample.dwFreqPnts = FREQ_CYC_NUM * NUM_PER_CYC * FREQ_UNIT; //Ƶ�ʸ���FREQ_CYC_NUM������(��׼ÿ����NUM_PER_CYC��)���� * FREQ_UNIT
    
	UpdateFreq(); 
}

//n:tʱ���ڴ��������Ĺ�������
//t:ʱ��
//2:һ���ܲ���1�����������Ĺ����
//ԭ��:f = (n/t) 
//t = �������*��������
//����:���ô���������n���������Ƶ��
void FreqSync(void)
{    
    WORD i;
    DWORD dwAvg = VOLT_REF>>1;      //����ǻ�׼��һ��
    if ((g_bFreqSync != 0) || (g_bFreqStep == 0))  //Ƶ���л������������ǰ��ɵĲ���Ƶ�ʲ��������ģ���������һ���������ڲ�����������һ����٣��������DMA������ɵ�
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
    	  
    for (i=0; i<NUM_PER_CYC*CYC_NUM; i++)  //ÿ��Ƶ�ʸ��ٵ����ּ�������һ����
	{
		g_AcSample.wZeroPntCnt++;   //Ƶ�ʸ��ٵ���������������ĵ���            һ�����ڵĵ���
		g_AcSample.wFreqPntCnt++;   //Ƶ�ʸ��ٵĲ������Ƶ������������ĵ���    ������ܹ��ĵ���
		if (g_AcSample.wZeroPntCnt > NUM_PER_CYC*2)  
		{   //����ĸ�λ�ж�Ҫ��Ҫ̫�ϸ�,�������һ�����ž͸�λ����
			ResetFreqSync();
			break;
		}
		
		if (g_AcSample.fPrePos)
		{			
            if (g_wAdBuf[g_AcSample.wFreqCn+i*NUM_CHANNELS] < dwAvg)//�ҵ��´��������Ĺ����
			{
				g_AcSample.fPrePos = false;
				
				if (g_AcSample.sZero1[2] <= 0)   //��һ������㻹û��
				{
					GetZero(g_AcSample.sZero1, g_dwFreqPtr, i);
					g_AcSample.wZeroPntCnt = 1;
					g_AcSample.wFreqPntCnt = 1;
					g_AcSample.wZeroCnt = 1;
				}
				else
				{
					if (g_AcSample.wZeroPntCnt > NUM_PER_CYC_55HZ*2/3)  //����Ĺ������
					{
						g_AcSample.wZeroPntCnt = 1;
						g_AcSample.wZeroCnt++;
						if (g_AcSample.wZeroCnt > FREQ_CYC_NUM) //�ﵽ������ܲ����������Ƶ��,2����һ��
						{
							GetZero(g_AcSample.sZero2, g_dwFreqPtr, i);
							int iFreq = CalcuFreq(g_AcSample.sZero1, g_AcSample.sZero2, g_AcSample.wFreqPntCnt-1);
							//if (iFreq>0 && g_AcSample.iValue[g_AcSample.wFreqCn]>VAILD_V) 
                            if (iFreq>0 && g_AcSample.iValue[0]>VAILD_V)  //g_AcSample.iValue���ŵ��ǽ���ֵ��ע������̶��ģ���ͨ��
							{			//�������Ч��Ƶ�ʼ���Ч�ķ�ֵ�Ÿ���Ƶ��ֵ
								g_AcSample.dwFreqPnts = iFreq;
								memcpy(g_AcSample.sZero1, g_AcSample.sZero2, sizeof(g_AcSample.sZero1));
								g_AcSample.wZeroPntCnt = 1;
								g_AcSample.wFreqPntCnt = 1;
								g_AcSample.wZeroCnt = 1;
                                g_dwFreqPtr = 0;
								
								UpdateFreq();
							}
							else   //Ƶ�ʸ���ʧ��
							{
								ResetFreqSync();
								g_AcSample.wFreqRstCnt++;
								WriteItemEx(BN2, PN0, 0x1026, (BYTE *)&g_AcSample.wFreqRstCnt); //Ƶ�ʸ��ٸ�λ����,HEX
								break;
							}
						}
					}
				}
				
				g_AcSample.wZeroPntCnt = 0;
			}  //end of �ҵ��´��������Ĺ����
		}		
        else if (g_wAdBuf[g_AcSample.wFreqCn+i*NUM_CHANNELS] >= dwAvg)
		{
			g_AcSample.fPrePos = true;
		}
		
		g_dwFreqPtr++;		
	}
}

//�����ѹ�������������û�ж�Ӧ������������洢�ڻ�����
//�����ѹ��ʸ���ͣ�ֻ�����,������У׼��Ļ���
/*
void CalcU0()
{		
	float Ureal,Uimg;
	if (g_AcSample.AcPara.bConnectType == CONNECT_3P4W)
	{
		Ureal = m_cplxFftOut[0][1].re + m_cplxFftOut[1][1].re + m_cplxFftOut[2][1].re;//�����ѹ����ʵ����
		Uimg = m_cplxFftOut[0][1].im + m_cplxFftOut[1][1].im + m_cplxFftOut[2][1].im;
		g_AcSample.iU0[0] = (int)sqrt(Ureal*Ureal+Uimg*Uimg);
	}
	else //��������û�������
	{
		g_AcSample.iU0[0] = 0;
	}
}

//�����ѹ�������������û�ж�Ӧ������������洢�ڻ�����
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


//����ת�浽���ݿ���
/*
void Transfer()
{
	WORD i;
	BYTE* p;	
	BYTE bBuf[SCN_NUM*8];
    	
	//���ɻ���ʸ��
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
            int iEValue = CalcuRMS(m_cplxFftOut[i]);  //����Чֵ
            DTRACE(1, ("%d\r\n", iEValue));            
        }
#endif
	}

	WriteItemDI(&g_AcSample.diAcVect, bBuf);	//��ͨ������ʸ�����
       
	//YC���ݿ�
	SaveYCData();
}*/

unsigned short CalcuTotalHarmonic(complex_fract32* cplx, int iBaseHarVal, int* piBasePercent)
{
	int iResult = 0;
	int iRet = 0;
	int iNoVal = iBaseHarVal >> 8;
    cplx++;	//������ֱ������
    
    int iTemp;
    
    int iBaseSqr = (int )cplx->re*cplx->re + (int )cplx->im*cplx->im;
    cplx++;	//���������
        
    for (WORD i=2; i<=g_AcSample.AcPara.wHarmNum; i++)
    {
    	iRet = (int )cplx->re * cplx->re + (int )cplx->im * cplx->im;
    	if (iRet <= iNoVal)
    		iRet = 0;
    	
        iResult += iRet;
	 	cplx++;
    }
    
    iTemp = (int)(10*sqrt((double)(iResult+ iBaseSqr))); //�Ŵ�10��,���Ա���һλС��
    if (iTemp == 0)
        iTemp = 1*10;
    
	*piBasePercent = 100000ULL* iBaseHarVal / iTemp; //������Чֵռ����Чֵ�ı���
    
    if (iBaseHarVal == 0)
        iBaseHarVal = 1;
    
    return (unsigned short )sqrt((double)iResult)*10000/iBaseHarVal;
}


//����:����ĳ��г���ĺ���,
//����:@cplx ĳ��г��������
//     @total �ܵ���Чֵ
//����:г������,��λ���һ
unsigned short CalcuHarmonic(complex_fract32* cplx, int total)
{
	if (total == 0)
		return 0;
		
    int iResult = (int )cplx->re * cplx->re + (int )cplx->im * cplx->im;
    
	iResult = (int)sqrt((double)iResult)*10000/total;
	return (unsigned short ) (iResult >= 10000 ? 9999 : iResult);
}

#if 0
//��ע:г�������ʸ�ʽ:����Ua,Ub,Uc,Ia,Ib,Ic����,2~HARMONIC_NUM��,��ʽNN.NN,��λ%
void DoHarmonic()
{
	if (!g_AcSample.AcPara.fCalcuHarmonic)	//������г��
		return;

	WORD i;
	WORD wHarPercent;
	DWORD dwNoVolt = g_AcSample.AcPara.dwUn / 10 + 1;	//10%��׼��ѹ
	DWORD dwNoCurrent = g_AcSample.AcPara.dwIn * 5 / 100;	//5%��׼����
	
	int iBasePercent;	//������Чֵռ����Чֵ�ı���
	DWORD dwBaseVal;	//������Чֵ
	WORD m = 0;
	WORD wBase;

   	for (i=0; i<6; i++)	//UA IA UB IB UC IC��6��ͨ��
   	{
   		//������Чֵ
   		DWORD dwBaseVal = (int )m_cplxFftOut[i][1].re*m_cplxFftOut[i][1].re + (int )m_cplxFftOut[i][1].im*m_cplxFftOut[i][1].im;
		WORD wBaseHarVal = (unsigned short)sqrt((double)dwBaseVal);
		
   		//����г�������ʼ���Чֵ
		WORD wTotalHarPecent;

		if ((i%2==0 && g_AcSample.iValue[i]<dwNoVolt) ||
			(i%2!=0 && g_AcSample.iValue[i]<dwNoCurrent) || 
			wBaseHarVal==0)
		{
			wTotalHarPecent = 0;
		}
		else
		{
   			wTotalHarPecent = CalcuTotalHarmonic(m_cplxFftOut[i], wBaseHarVal, &iBasePercent);//�ܻ���
		}
		m = 0;
		if (i%2 == 0)	//��UaIaUbIbUcIc˳����UaUbUcIaIbIc
			wBase = i/2 * HARM_NUM_MAX;
		else 
			wBase = (i/2+3) * HARM_NUM_MAX;
		g_AcSample.tPOH[wBase+m++].iValue = wTotalHarPecent;
		g_AcSample.tPOH[wBase+m-1].wDesBits = 0;
		
		dwBaseVal = g_AcSample.iValue[i] * iBasePercent / 10000; 	//������Чֵ
		
		//�����г�������ʼ���Чֵ
   		for (WORD j=2; j<=g_AcSample.AcPara.wHarmNum; j++)
   		{
   			//�����г��������
   			if (wTotalHarPecent == 0) //ĳ��û�е�ѹ�����
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

//����:У��һ��ͨ��FFT_NUM/2��г������
//void VectAdj(complex_fract16* incplx, complex_fract32* outcplx, complex_fract32* outUnAjdcplx, int adjreal, int adjimag)
void VectAdj(complex_fract16* incplx, complex_fract32* outcplx, int adjreal, int adjimag)
{
    int i;    
	if (!g_AcSample.fAdjParaSet) //У׼ʱֱ�ӽ�ֵ������outcplx��
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

	incplx++;//������ֱ������
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

//������������Чֵ(RMS : Root Mean Square)
int CalcuRMS(complex_fract32* cplxIn)
{
	double sum = 0;
	int i;
	cplxIn++; //������ֱ������
	for(i=1; i<FFT_NUM/2; i++) //FFT_NUM/2
	{
		 sum += ((double )cplxIn->re*cplxIn->re + (double )cplxIn->im*cplxIn->im);
		 cplxIn++;
	}
	return (int )sqrt((double )sum);
}

//��ע����������ʱ�Ĺ���  Pa = Uab * Ia *Cos()/Pc = Ucb * Ic * Cos()/ P0 = Pa + Pc
//���ڽ���,�����ط��汾���ն��ڲ������ֲ�ͬ�Ľ��߷�ʽ,
// ��һ��:
// A���Ua��Ub
// B���Ua��Uc
// C���Uc��Ub
// �ڶ���:
// A���Ua��Ub
// B���Ub��Uc
// C���Uc��Ua
// �ڶ��ַ�ʽ�µ�C���ѹ�൱�ڵ�һ�ַ�ʽ�µ�B���ѹ����λ����
// ����Pcʱ��Ҫʹ��B��ĵ�ѹ����C��������Ƕ�ʹ��B���ѹ�Ƕȼ�ȥ180��
// ����C������Ĳ�ֵ
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
    		*iP = (g_AcSample.iValue[j]*g_AcSample.iValue[j+3]*x)/1000000; //������λС��
			*iQ = (g_AcSample.iValue[j]*g_AcSample.iValue[j+3]*y)/1000000;

			if (g_AcSample.iValue[j]<100 &&g_AcSample.iValue[j+3]<100) //û�е�ѹ������ʱ����ǲ�ҲΪ0
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

			*iCos = CalcuCos(*iP, *iQ); //�ܹ�������
		}
	}	

	#if 0
	//���ڸ���ͨ��������ͬ��������λ�������ķ��������㹦�ʻ���
	//��ѹ����֮�����λ����õ������ֵ������������ǲ����У��
	// ֮�����ſ���	
	for (int j = 0; j < 4; j++,iCos++,iP++,iQ++)
	{
		if (j < 3)
		{			
			cplxV = m_cplxFftOut[j];
			cplxI = m_cplxFftOut[j+3];

			cplxI++;
			cplxV++; //������ֱ������

			iResultP = 0;
			iResultQ = 0;
			for(WORD k=1; k<FFT_NUM/2; k++)
			{
			   iResultP += (int )cplxV->re * cplxI->re + (int )cplxV->im * cplxI->im;
			   iResultQ += (int )cplxI->re * cplxV->im - (int )cplxI->im * cplxV->re;

			   cplxI++;
			   cplxV++;
			}

			*iP = iResultP/100000;	//��ѹ������100��������������1000��
				
			//if (iResultQ < 0)
			//	iResultQ = -iResultQ;
			*iQ = iResultQ/100000;	//��ѹ������100��������������1000��				
		}
		else if (j == 3)//�ܹ���
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
	if (P==0 && Q==0)   //�����0
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

//�����
void CalcuAngle()
{
	int iAngleUa = 0;
	g_AcSample.iAngleUa = 0;
	int* ipAngel = g_AcSample.iAngle;	
    //int iCnt;
    memset(g_AcSample.iAngle, 0, sizeof(g_AcSample.iAngle));

	for (int j = 0; j < 6; j++)//�����ѹ������
	{		
        if (g_AcSample.iValue[j] < VAILD_V)//��ֵС��ĳ��ֵʱ�Ͳ��ü������
        {            
            *ipAngel++ = 0;
            continue;
        }
		if (j == 0)//Ua
		{
			//�Ƕȼ���Ҫ��δ��ʸ��У����FFTֵ
            *ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOut[j][1].im, (float)m_cplxFftOut[j][1].re) / 3.1415926 ;
//			*ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOutUnAdj[j][1].im, (float)m_cplxFftOutUnAdj[j][1].re) / 3.1415926 ;
			g_AcSample.iAngleUa = iAngleUa = *ipAngel;
			*ipAngel = 0;			
		}
		else 
		{
			//�Ƕȼ���Ҫ��δ��ʸ��У����FFTֵ
			*ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOut[j][1].im, (float)m_cplxFftOut[j][1].re) / 3.1415926 ;
//			*ipAngel = 180.0 * 10 * (float)atan2f((float)m_cplxFftOutUnAdj[j][1].im, (float)m_cplxFftOutUnAdj[j][1].re) / 3.1415926 ;
			*ipAngel = (*ipAngel + 3600 - iAngleUa) % 3600;

			//����һ��AD��ʱ��������ͨ��������ͬ�������ĽǶ����
			//iCnt = j - 0;// bChannelUa
//			*ipAngel = ((*ipAngel*10 +36000 - iCnt*36000/(SCN_NUM*64))/10) % 3600;//�ȷŴ�10������С10�����������,������������5�����
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

//����������λ����ֵ���YC��ʽ���
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
	//iAngle++; //�����ѹ,û�б����������
	//iAngle++; //�������

	//��������
	//pbProChannel += 3;
	//bChProIa = *pbProChannel;
	//g_AcSample.tVal[AC_VAL_PRO_IA].iValue = g_AcSample.iValue[*pbProChannel++]; //IAP
	//g_AcSample.tVal[AC_VAL_PRO_IB].iValue = g_AcSample.iValue[*pbProChannel++]; //IBP
	//g_AcSample.tVal[AC_VAL_PRO_IC].iValue = g_AcSample.iValue[*pbProChannel++]; //ICP
    g_AcSample.tVal[AC_VAL_PA].iValue = *iP++; //PA
    if (g_AcSample.AcPara.bConnectType == CONNECT_3P3W) //��������
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

	//Ƶ��
	g_AcSample.tVal[AC_VAL_F].iValue = g_AcSample.dwFreq;
	g_AcSample.tVal[AC_VAL_F].wDesBits = 0;
	WriteItemDI(&g_AcSample.diYcBlk[0], (BYTE*)pData);  //�ն�����������
}*/
/*
//����: �ж�ĳ������Ƿ�����·
int IsPhaseXShortCircuit(BYTE bPn, int iIx, BYTE bPhase, TSHJPara *pSHJPara)
{
	DWORD *pStartTime;
	DWORD *pRetrayTime;

	pStartTime = &g_dwTimeStart[bPn][bPhase];
	pRetrayTime = &g_dwTimeRetray[bPn][bPhase];
	if (iIx > pSHJPara->dwIh)//���Ϸ���
	{
		if (*pStartTime == 0)
		{
			*pStartTime = GetTick();//��ʼ��ʱ
		}
		else
		{
			if ((GetTick() - *pStartTime) > pSHJPara->wT1Min)	//Ix�����ҳ�������ʱ������ֵ
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
	{//�ȷ��������ָܻ�
		if ((*pStartTime != 0) && (*pRetrayTime == 0))//���ϻָ�
		{
			*pRetrayTime = GetTick();
            DTRACE(0, ("%s Recover StartTime %d!\r\n", __FUNCTION__, GetTick()));
		}
		if ((GetTick() - *pRetrayTime) > pSHJPara->wT1Min)	//Ix�����ҳ�������ʱ������ֵ
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
//�����������ϼ�⣬��������ӵع��ϡ���������·����������·
//��ע��
WORD PhaseFaultDetection(BYTE bPn, int iIa, int iIb, int iIc, TSHJPara *pSHJPara)
{
	WORD wStatus = 0;

	int iStatus = 0;
	iStatus = IsPhaseXShortCircuit(bPn, iIa, PHASE_A, pSHJPara);//����·
	if (iStatus > 0)//A���������ֵ
		wStatus |= 1<<0;
	else if (iStatus < 0)//A���������ֵ
		wStatus |= 1<<8;

	iStatus = IsPhaseXShortCircuit(bPn, iIb, PHASE_B, pSHJPara);//B���������ֵ
	if (iStatus > 0)
		wStatus |= 1<<1;
	else if (iStatus < 0)
		wStatus |= 1<<9;

	iStatus = IsPhaseXShortCircuit(bPn, iIc, PHASE_C, pSHJPara);//C���������ֵ
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
//���ݼ��������ֵ�ķ��������㵽���쳣״̬
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

	for (int i = 0; i < 3; i++)//����
	{
		bStatus = (BYTE)wState&0x0f;
		if (bStatus == g_bSlickStatus[0][i])
		{
			if (fZeroI && (g_fStatus_groud[wPN][i] == false))//�ӵأ�����û�з���
			{
				DTRACE(0, ("%s Point %d, two short & groud occur!\r\n", __FUNCTION__, wPN));
				g_fStatus_groud[wPN][i] = true;
				return 1<<TWO_SHORT_GROUD;
			}
			else if((bStatus == g_bSlickStatus[0][i]) && (!fZeroI) && (g_fStatus[wPN][i] == false))//û�ӵأ�����û�з��� 
			{
				DTRACE(0, ("%s Point %d, two short occur!\r\n", __FUNCTION__, wPN));
				g_fStatus[wPN][i] = true;
				return 1<<TWO_SHORT;
			}
		}
		else if (bStatus == g_bSlickStatus[1][i])
		{
			if (fZeroI && (g_fStatus_one_phase_groud[wPN][i] == false))//�ӵأ�����û�з���
			{
				DTRACE(0, ("%s Point %d, 1 phase groud occur!\r\n", __FUNCTION__, wPN));
				g_fStatus_one_phase_groud[wPN][i] = true;
				return 1<<ONE_GROUD;
			}
		}

		bStatus = (BYTE)(wState>>8)&0x0f;
		if (((bStatus & g_bSlickStatus[0][i]) || (!fZeroI)) && (g_fStatus_groud[wPN][i] == true))//�ӵػָ�������·�ָ������ҷ���
		{
			DTRACE(0, ("%s Point %d, two short & groud recover!\r\n", __FUNCTION__, wPN));
			g_fStatus_groud[wPN][i] = false;
			return 1<<TWO_SHORT_GROUD_RECOVER;
		}
		else if ((bStatus & g_bSlickStatus[0][i]) && (g_fStatus[wPN][i] == true))//�ӵػָ������ҷ���
		{
			DTRACE(0, ("%s Point %d, two short recover !\r\n", __FUNCTION__, wPN));
			g_fStatus[wPN][i] = false;
			return 1<<TWO_SHORT_RECOVER;
		}
		else if (((bStatus & g_bSlickStatus[1][i]) || (!fZeroI)) && (g_fStatus_one_phase_groud[wPN][i] == true))//�ӵػָ�������·�ָ������ҷ���
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
//��û�ж���
	if (g_AcSample.dwOffRecoverTime[wPn] == 0) 
	{
		g_AcSample.dwOffStartTime[wPn] = 0;
		g_AcSample.dwOffRecoverTime[wPn] = GetClick();
		g_AcSample.fPowerOff[wPn] = false;
	}
	if ((g_AcSample.fOffRecover[wPn] == false) & (GetClick() - g_AcSample.dwOffRecoverTime[wPn] >= pSHJPara->wOffTime))
	{
		g_AcSample.fOffRecover[wPn] = true;
		SaveSOE(wPn, 0xc027, 0);//ϵ�y����
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
		SaveSOE(wPn, 0xc027, 1);//ϵ�y����
	}
	return;
}*/
//�������������ӵض�·
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

	wStatus = Det2Status(wPn, wStatus, (bool)(iIn > pSHJPara->dwI0));//�����쳣�����ͼ���

	if (((wStatus & (1<<TWO_SHORT))) || ((wStatus & (1<<TWO_SHORT_RECOVER))))//����ӵأ�����
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
//������������������������ѹ��ѹ���
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
	if (iIn > g_tSHJPara.dwZeroI)//����
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
    else if (iIn < g_tSHJPara.dwZeroI)//�ָ�
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

	if (iUn > g_tSHJPara.dwZeroU)//�����ѹ��ѹ����
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
    else if (iUn < g_tSHJPara.dwZeroU)//�����ѹ��ѹ�ָ�
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


//����������SOE�¼��⣬����дָ��
void SaveSOE(WORD wPn, const WORD wID, BYTE bYxValue)
{/*
    TYXData tYxData;
    BYTE bPtr = 0;
	BYTE bBuf[512];
	BYTE *p = bBuf;
	unsigned long long uMsec = GetCurTimeMs();
	if (!IS_ENVET_OPERATION(wID-0xc021))//ID��Ч��ʱ�򣬾Ͳ������¼����棬�ڼ���ʱ��ҲҪ�����ж�
		return;

	memset(&tYxData, 0, sizeof(tYxData));
    ReadItemDI(&g_AcSample.ptSOEPtr, bBuf);		//SOE��
	bPtr = bBuf[0];	//SOEָ��
	p += sizeof(TYXData)*(bPtr++) + 1;
	if (bPtr >= 32)
        bPtr = 0;
	bBuf[0] = bPtr;

	tYxData.bPn = (BYTE)wPn;	//������
	tYxData.wID = wID;			//ң��ID
	Uint64ToBCD(uMsec, tYxData.bTimeStamp, 6);
	tYxData.bValue[0] = bYxValue;	//YX����

	memcpy(p, &tYxData, sizeof(TYXData));
	WriteItemDI(&g_AcSample.ptSOEPtr, bBuf);
	WriteItemEx(BN0, wPn, wID, tYxData.bValue);*/
}


//���������ֹ��ϼ�⣬����ң���¼�
void DoFaultJugement()
{/*
	int iCurrentVal[4];//����ʱIa��Ib��Ic��I0	
	
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

//ʹ��Ia��Ic��I0��Ib
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
    else //��������û���������
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

