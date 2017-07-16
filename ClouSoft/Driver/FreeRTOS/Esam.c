#include "Esam.h"
#include "board.h"
#include "SysDebug.h"
#include "string.h"
#include "Sysarch.h"
#include "Gpio.h"
#include "pmc.h"
#include "SpiBus.h"


#define PIN_ESAM_PWR    {PIO_PB14, PIOB, ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}   

static const Pin EsamPins[] = {PIN_ESAM_PWR};

static void configure_esam_pins(void)
{   
    Gpio_Config(EsamPins, PIO_LISTSIZE( EsamPins ));
}

int esam_read(BYTE *buf, int count)
{    
    WORD wCnt;
    BYTE *p = buf; 
    DWORD dwTick;
    bool fReady = false;
	buf[0] = 0;
    dwTick = GetTick();
    SPIEnable(ESAM_CHIP);	
    while(GetTick()-dwTick < 3000)  //3s
    {
        SSI1GetData(ESAM_CHIP, p, 1);
        if (buf[0] == 0x55) //接口设备判别状态字，是否为0x55        
        {
            p++;
            fReady = true;
            break;
        } 
        Sleep(10);
    }
    if (!fReady)
    {
        SPIDisable(ESAM_CHIP);     
        return -1;
    }
    
	//发送完命令开始接收数据
	SSI1GetData(ESAM_CHIP, p, 4);  //接收4个字节
    p += 4;
    wCnt = (buf[3]<<8) | buf[4];    //接收数据的长度
    if (count < wCnt+5)
    {
        SPIDisable(ESAM_CHIP);   
        return -1;
    }
    SSI1GetData(ESAM_CHIP, p, wCnt+1); //还有一个字节LRC2
    p += wCnt+1;
    
	SPIDisable(ESAM_CHIP);        

	return p-buf;//返回本次收到的字节数
}

int esam_read2(BYTE *buf, int count)
{
    BYTE *p = buf; 
    SPIEnable(ESAM_CHIP);
	SSI1GetData(ESAM_CHIP, p, count);
	SPIDisable(ESAM_CHIP);        

	return count;//返回本次收到的字节数
}

//接口设备发送完数据Len2后，需要加100μs时间间隔，再发送DATA
int esam_write(BYTE * buffer, int count)
{
    SPIEnable(ESAM_CHIP);		//启用SSI  
    if (count < 7) // for test
    {
        SSI1SendData(ESAM_CHIP, buffer, count);   
    }
    else
    {        		  
        SSI1SendData(ESAM_CHIP, buffer, 7);    
        Delay1us(100);
        SSI1SendData(ESAM_CHIP, buffer+7, count-7);    
    }
	SPIDisable(ESAM_CHIP);
        
	return count;
}

void esam_warm_reset(void)
{
    Gpio_Set(&EsamPins[0]); //供电  
    Sleep(1000);
    Gpio_Clear(&EsamPins[0]); //供电  
    DTRACE(1, ("Esam warm reset\n"));     
}

void esam_init(void)
{
	 configure_esam_pins();
     
     Gpio_Clear(&EsamPins[0]); //供电 
     
     Sleep(100);    //初始化时间留长，希望能更好地初始化芯片     
	 
     //DTRACE(1, ("Esam driver init OK\n"));     
}
