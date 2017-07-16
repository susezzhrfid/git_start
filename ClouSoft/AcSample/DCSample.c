#include "DCSample.h"
#include "Sample.h"
#include "DbAPI.h"
#include "sysdebug.h"
#include "AD.h"

#define INER_ADJ         //�ڲ�У׼��ͨ�Ƽ��׼��㷽ʽ

TDcSample g_DCSmple;

//ֱ��ͨ���ŵ�����ADͨ���ŵ�ӳ��

bool DCSmpleInit()
{
	memset(g_DCSmple.wDCVector, 0, sizeof(g_DCSmple.wDCVector));

	memset(g_DCSmple.iDcValue, 0, sizeof(g_DCSmple.iDcValue));
	//memset(g_tDCAdjBuf, 0, sizeof(g_tDCAdjBuf));

	//GetAdjustVector();
	
	//g_DCSmple.iDcChannel = 0;
	return true;
}

void GetADValue(WORD *pwDcData) //�п����������ȡ���ݣ��������߳������ڸ�������
{
    /*BYTE i;    
    for (i=0; i<DC_CHANNEL_NUM; i++)
    {        
        pwDcData[i] = g_wAdBuf[i]; //��1����ʼ��ֱ��    ֻȡ��ÿ���ĵ�һ���� todo:��������ϴ�����ö�����ֵ
    }*/
    pwDcData[0] = g_wAdBuf[0];  //ʱ�ӵ�ص�ѹ����
    //pwDcData[1] = g_wAdBuf[2];  //�¶�
	
	return;    
}

void AcDataSave(BYTE bType, DWORD dwData)
{                
    WriteItemEx(BN2, PN0, 0xba21+bType, (BYTE *)&dwData); 
}

void DoDCSample(void)
{    
    BYTE i;      
    WORD wDCSampleValue[DC_CHANNEL_NUM];   
    
    GetADValue(wDCSampleValue);    
    
    //��һ�ݲ���ԭʼֵ��ϵͳ��
    WriteItemEx(BN2, PN0, 0xba20, (BYTE *)wDCSampleValue);
    
    for (i=0; i<DC_CHANNEL_NUM; i++)       //�������     
    {            
#ifdef INER_ADJ     
        WORD wValue;
        BYTE bBuf[4];
        wValue = (wDCSampleValue[i]*VOLT_REF)/MAX_DIGITAL;     //��׼Ϊ3.3V
        if (i == 0) //ʱ�ӵ�ص�ѹ����
        {            
            wValue = wValue<<1;             //ֱʵֵ    x.xxx V
            wValue = (wValue+5)/10;                         //xx.xxV
            DWORDToBCD(wValue, bBuf, 2);
            WriteItemEx(BN2, PN0, 0x1044, bBuf); 
        }
        else if (i == 1) //�¶�
        {
            /* Using multiplication (*0.37736) instead of division (/2.65). */
		    wValue = ((wValue - 800) * 37736 + 2700000)/1000;  //�ȷŴ�100000��������С1000����ʵ�ʷŴ�100��  xx.xx��
            memset(bBuf, 0, sizeof(bBuf));
            DWORDToBCD(wValue, bBuf, 2);
            WriteItemEx(BN2, PN0, 0x1045, bBuf); 
        }
        //DWORDToBCD(wValue, bBuf, 2);
        //WriteItemEx(BN2, PN0, 0xba21+i, bBuf);  
        
#else
        long lRms;    
        
        lRms = (long)ConvertToLineVal(i, wDCSampleValue[i]);  //У�����
        if (lRms >= 0)
        {            
            AcDataSave(i, (DWORD)lRms);
        }
        else
        {
            //DTRACE(DB_TASK, ("#### Convert Err!\n"));
        }
#endif
    } 
}

