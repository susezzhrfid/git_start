#ifndef SPIBUS_H
#define SPIBUS_H

#include "Typedef.h"

//Flash片选相关
#define EXFLASH_FST_CHIP	1
#define EXFLASH_SND_CHIP	2

#define EXFLASH_CS_BASE		GPIO_PORTB_BASE
#define EXFLASH_CS_FST		GPIO_PIN_2
#define EXFLASH_CS_SND		GPIO_PIN_3

#define EXFSH_CS_FST_HIG	GPIO_PIN_2
#define EXFSH_CS_SND_HIG	GPIO_PIN_3

//Flash Speed
#define EXFLASH_RATE_8M	8000000
#define EXFLASH_RATE_5M	5000000
#define EXFLASH_RATE_2M	2000000
#define EXFLASH_RATE_1M	1000000

//Flash最长等待时间
#define EXFLASH_WAIT_MS	500	//sector program/erase need 50ms   setting 5 times



//描述：使能SPI
//参数：@bChip 操作的Flash片
//返回：NONE
//总结：
bool SPIEnable(BYTE bChip);

//描述：禁止SPI
//参数：@bChip 操作的Flash片
//返回：NONE
//总结： 
bool SPIDisable(BYTE bChip);

//描述：从SSI1接收一个字节
//参数：NONE
//返回：返回接收到的数据
BYTE SSI1GetByte(void);

//描述：将pdwData的数据通过SSI1发出去
//参数：@*pulData 准备发出去的数据
//		@nLen 发送数据的长度
//返回：正确发送返回发送的长度，否则返回0
//总结： 
DWORD SSI1SendData(BYTE *pbData, WORD nLen);

//描述：从SSI1接收数据回来
//参数：@*pulData 接收数据缓冲区
//		@nLen 缓冲区的长度
//返回：正确接收返回数据的长度，否则返回0
//总结： 
DWORD SSI1GetData(BYTE *pbData, WORD nLen);

void SSI1SendByte(BYTE data);

void SpiInit(void);

#endif