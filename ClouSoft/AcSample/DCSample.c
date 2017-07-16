#include "DCSample.h"
#include "Sample.h"
#include "DbAPI.h"
#include "sysdebug.h"
#include "AD.h"

#define INER_ADJ         //内部校准，通计简易计算方式

TDcSample g_DCSmple;

//直流通道号到物理AD通道号的映射

bool DCSmpleInit()
{
	memset(g_DCSmple.wDCVector, 0, sizeof(g_DCSmple.wDCVector));

	memset(g_DCSmple.iDcValue, 0, sizeof(g_DCSmple.iDcValue));
	//memset(g_tDCAdjBuf, 0, sizeof(g_tDCAdjBuf));

	//GetAdjustVector();
	
	//g_DCSmple.iDcChannel = 0;
	return true;
}

void GetADValue(WORD *pwDcData) //有可能这边正在取数据，而交采线程又正在更新数据
{
    /*BYTE i;    
    for (i=0; i<DC_CHANNEL_NUM; i++)
    {        
        pwDcData[i] = g_wAdBuf[i]; //第1道开始是直流    只取了每道的第一个点 todo:如果波动较大可以用多点求均值
    }*/
    pwDcData[0] = g_wAdBuf[0];  //时钟电池电压测量
    //pwDcData[1] = g_wAdBuf[2];  //温度
	
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
    
    //存一份采样原始值到系统库
    WriteItemEx(BN2, PN0, 0xba20, (BYTE *)wDCSampleValue);
    
    for (i=0; i<DC_CHANNEL_NUM; i++)       //数据入库     
    {            
#ifdef INER_ADJ     
        WORD wValue;
        BYTE bBuf[4];
        wValue = (wDCSampleValue[i]*VOLT_REF)/MAX_DIGITAL;     //基准为3.3V
        if (i == 0) //时钟电池电压测量
        {            
            wValue = wValue<<1;             //直实值    x.xxx V
            wValue = (wValue+5)/10;                         //xx.xxV
            DWORDToBCD(wValue, bBuf, 2);
            WriteItemEx(BN2, PN0, 0x1044, bBuf); 
        }
        else if (i == 1) //温度
        {
            /* Using multiplication (*0.37736) instead of division (/2.65). */
		    wValue = ((wValue - 800) * 37736 + 2700000)/1000;  //先放大100000倍，再缩小1000倍，实际放大100倍  xx.xx度
            memset(bBuf, 0, sizeof(bBuf));
            DWORDToBCD(wValue, bBuf, 2);
            WriteItemEx(BN2, PN0, 0x1045, bBuf); 
        }
        //DWORDToBCD(wValue, bBuf, 2);
        //WriteItemEx(BN2, PN0, 0xba21+i, bBuf);  
        
#else
        long lRms;    
        
        lRms = (long)ConvertToLineVal(i, wDCSampleValue[i]);  //校正结果
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

