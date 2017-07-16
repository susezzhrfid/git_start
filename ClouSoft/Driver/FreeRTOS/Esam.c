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
        if (buf[0] == 0x55) //�ӿ��豸�б�״̬�֣��Ƿ�Ϊ0x55        
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
    
	//���������ʼ��������
	SSI1GetData(ESAM_CHIP, p, 4);  //����4���ֽ�
    p += 4;
    wCnt = (buf[3]<<8) | buf[4];    //�������ݵĳ���
    if (count < wCnt+5)
    {
        SPIDisable(ESAM_CHIP);   
        return -1;
    }
    SSI1GetData(ESAM_CHIP, p, wCnt+1); //����һ���ֽ�LRC2
    p += wCnt+1;
    
	SPIDisable(ESAM_CHIP);        

	return p-buf;//���ر����յ����ֽ���
}

int esam_read2(BYTE *buf, int count)
{
    BYTE *p = buf; 
    SPIEnable(ESAM_CHIP);
	SSI1GetData(ESAM_CHIP, p, count);
	SPIDisable(ESAM_CHIP);        

	return count;//���ر����յ����ֽ���
}

//�ӿ��豸����������Len2����Ҫ��100��sʱ�������ٷ���DATA
int esam_write(BYTE * buffer, int count)
{
    SPIEnable(ESAM_CHIP);		//����SSI  
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
    Gpio_Set(&EsamPins[0]); //����  
    Sleep(1000);
    Gpio_Clear(&EsamPins[0]); //����  
    DTRACE(1, ("Esam warm reset\n"));     
}

void esam_init(void)
{
	 configure_esam_pins();
     
     Gpio_Clear(&EsamPins[0]); //���� 
     
     Sleep(100);    //��ʼ��ʱ��������ϣ���ܸ��õس�ʼ��оƬ     
	 
     //DTRACE(1, ("Esam driver init OK\n"));     
}
