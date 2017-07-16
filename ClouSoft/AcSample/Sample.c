/*********************************************************************************************************
 * Copyright (c) 2005,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ��Խ��������Ĺ����������������������ж���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2005��11��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/

#include "AcSample.h"
#include "Sample.h"
#include "AcFmt.h"
#include "DbAPI.h"
#include "sysdebug.h"
#include "FaConst.h"

bool g_fDcSample;
//BYTE g_bDcSampleFlag;

//��������ʼ����������
bool InitAcSample()
{
	//memset(g_sSampleBuf, 0, sizeof(g_sSampleBuf));  	//�޹���������õ�֮ǰ��û�еĵ�ѹ����,���������	
	AcSampleInit(PN0);
	//g_fStartAdCheck = false;    //����ADͨ�����Ҽ��		
	return true;
}

//��������ʼ��ֱ������
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

//����:���ݵ�ǰADֵ��ȡУ׼����
//����:���ҵ���ȫ��ȵĵ�ʱ����0,����ͨ��nAD1��nVal1����
//     û����ȫ��ȵ�ʱ����1,���ڵ�����ͨ��nAD1��nVal1��nAD2��nVal2����
//     ʧ�ܷ��ظ���
//��ע:У׼ʱҪ���С�������,���ϵͳ��У׼����g_tCalibCfg.tCalibTab�ǰ�����
//���е�
int GetCalibPt(BYTE bType, DWORD dwCurAD, DWORD* dwAD1, DWORD* dwVal1, DWORD* dwAD2, DWORD* dwVal2)
{
    BYTE  bLow = 0;
    if (g_tCalibCfg.bCalibPtNum[bType] < 1)  
        return -1;
    BYTE  bHi = g_tCalibCfg.bCalibPtNum[bType] - 1;
    
    if (g_tCalibCfg.bCalibPtNum[bType] == 1)//����У׼ʱ������ԭ��
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
        {//�պ����
            *dwAD1 = g_tCalibCfg.tCalibTab[bType][bLow].wAD;
            *dwVal1 = g_tCalibCfg.tCalibTab[bType][bLow].wVal;
            return 0;
        }
        else if (dwCurAD == g_tCalibCfg.tCalibTab[bType][bLow+1].wAD)
        {//�պ����            
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

//����:��ADֵת����ʵ�ʵ���·�ϵ���ֵ,��У׼��������������
//����:�쳣������ظ���
long ConvertToLineVal(BYTE bType, long lAdVal)
{
    DWORD dwAD1, dwAD2;
    DWORD dwVal1, dwVal2;
    long lVal = 0;
    int iRet = GetCalibPt(bType, (DWORD)lAdVal, &dwAD1, &dwVal1, &dwAD2, &dwVal2);
    
    if (iRet==0)    //ADֵ��У׼��պ���ͬ
        return dwVal1;
    
    //ADֵ��ʵ�ʵ���ֵ�Ĺ�ϵӦ���ǵ�����,������������������,�����Է���һ
    if (dwAD2 == dwAD1)  
        return -1;
    
    if (iRet > 0)
    {
        //����ʽֱ�߷��� (Y-Y1)/(Y2-Y1) = (X-X1)/(X2-X1)        
        lVal = (lAdVal - (long)dwAD1)*(long)(dwVal2 - dwVal1) / (long)(dwAD2 - dwAD1) + (long)dwVal1;
        if (lVal < 0)
            lVal = 0;
        return lVal;
    }
    
    return -1;     
}

//------------����У׼��ʽ�ʺϽ���/ֱ��У׼,������У׼������Чֵ--------------
//����0-OK
int OnCalibrate(BYTE* pbBuf)
{
    BYTE* pbRx = pbBuf;    
    BYTE bStep = *pbRx++;    //У׼����
    
    BYTE bBuf[16];
    BYTE bBuf2[4];    
    BYTE i;
	//WORD wUn;

    //for (i=0; i<4; i++)  //CL818K5�������ݷ��ڵ�5ͨ��
        //pbRx += 4;
    
    for (i=0; i<SCN_NUM+DC_CHANNEL_NUM; i++)  //��ҪУ׼��ͨ����,���ֱ����ҪУ׼��ҲӦ����
    {
        //�����ݿ��ȡ��ǰ�Ĳ���ԭʼֵ
        if (i < SCN_NUM)  //����
        {
            ReadItemEx(BN2, PN0, 0xba11, bBuf);  //δУ׼����Чֵ   
            memcpy(bBuf2, &bBuf[i<<1], 2);
            memcpy(&bBuf2[2], pbRx, 2);           //У��ֵ
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
        else                  //ֱ��
        {
            ReadItemEx(BN2, PN0, 0xba20, bBuf);  //bBufǰ���ֽ���ADԭʼֵ����Чֵ 
            memcpy(bBuf2, &bBuf[(i-SCN_NUM)<<1], 2);
            memcpy(&bBuf2[2], pbRx, 2);
            pbRx += 4;
            WriteItemEx(BN25, PN0, 0x1003+(i<<12)+bStep-1, bBuf2); //todo:
        }
        
        WriteItemEx(BN25, PN0, 0x1002+(i<<12), &bStep);//�����µ�У׼������
    }
    
    LoadCalibCfg();  //����һ����Ϣ
    
    return ERR_APP_OK;
}

//����:��ʼ��ϵͳ����У׼����
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
        if (bCalibPtNum > MAX_CALIB_PT_NUM) //��������ȷ
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

//����Ƿ񽻲�У׼��
bool IsAcCalib(void)
{
    if ((g_tCalibCfg.bCalibPtNum[1]>0) && 
        (g_tCalibCfg.bCalibPtNum[1]<MAX_CALIB_PT_NUM))
        return true;
    
    return false;
}
