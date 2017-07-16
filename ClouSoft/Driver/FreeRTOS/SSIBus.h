#ifndef SPIBUS_H
#define SPIBUS_H

#include "Typedef.h"

//FlashƬѡ���
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

//Flash��ȴ�ʱ��
#define EXFLASH_WAIT_MS	500	//sector program/erase need 50ms   setting 5 times



//������ʹ��SPI
//������@bChip ������FlashƬ
//���أ�NONE
//�ܽ᣺
bool SPIEnable(BYTE bChip);

//��������ֹSPI
//������@bChip ������FlashƬ
//���أ�NONE
//�ܽ᣺ 
bool SPIDisable(BYTE bChip);

//��������SSI1����һ���ֽ�
//������NONE
//���أ����ؽ��յ�������
BYTE SSI1GetByte(void);

//��������pdwData������ͨ��SSI1����ȥ
//������@*pulData ׼������ȥ������
//		@nLen �������ݵĳ���
//���أ���ȷ���ͷ��ط��͵ĳ��ȣ����򷵻�0
//�ܽ᣺ 
DWORD SSI1SendData(BYTE *pbData, WORD nLen);

//��������SSI1�������ݻ���
//������@*pulData �������ݻ�����
//		@nLen �������ĳ���
//���أ���ȷ���շ������ݵĳ��ȣ����򷵻�0
//�ܽ᣺ 
DWORD SSI1GetData(BYTE *pbData, WORD nLen);

void SSI1SendByte(BYTE data);

void SpiInit(void);

#endif