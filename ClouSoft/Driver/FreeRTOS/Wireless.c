#include "Wireless.h"
#include "Si4432.h"
#include "Drivers.h"


void RfInit(void)
{
    BYTE bDevType = 0;
//    WL_POWER_M = 0; //OUT
    //WL_POWER = 1;   //打开电源
    WlModeOn();      
    WirelessInit();

    SI4432Init();

    bDevType = SpiReadRegister(0);  //SI_4432_DTC     //TI SI模块可以自识别
    
    if (bDevType == 0x08)   //si
    {
        SI4432Init();
    }
    
    RfSetFreq(g_bFreqGrp[0][0]); //如果是集中器节点，频率组设置大于10时默认取0
}

BYTE RfSendPacket(BYTE *pbTxBuf, BYTE bLen)
{    
    return SendPacketSI(pbTxBuf, bLen);
}

void RfRecvPacket(void)
{    
    RecvPacketSI();
}

void RfSetFreq(BYTE bChannel)
{    
    RfSetFreqSI(bChannel);
}

int GetRSSI(void)
{    
    return GetRSSISI();
}
