/*********************************************************************************************************
 * Copyright (c) 2013,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Gpio.h
 * ժ    Ҫ�����ļ���Ҫ��IO����
 * ��ǰ�汾��1.0
 * ��    �ߣ�liyan
 * ������ڣ�2013��1��
 *********************************************************************************************************/
#ifndef GPIO_H
#define GPIO_H

#include "pio.h"

#define PIO_LISTSIZE(pPins)    (sizeof(pPins) / sizeof(Pin))

typedef struct _Pin
{
    /*  Bitmask indicating which pin(s) to configure. */
    uint32_t mask;
    /*  Pointer to the PIO controller which has the pin(s). */
    Pio    *pio;
    /*  Peripheral ID of the PIO controller which has the pin(s). */
    uint32_t id;
    /*  Pin type. */
    pio_type_t type;
    /*  Pin attribute. */
    uint32_t attribute;
} Pin ;

void Gpio_Config(const Pin* pPins, uint32_t dwSize );
void Gpio_Clear(const Pin* pPin);
void Gpio_Set(const Pin* pPin);
uint32_t Gpio_Get(const Pin* pPin);
void PIO_PinConfigureIt(const Pin *pPin, void (*handler)(uint32_t, uint32_t) );
void PIO_PinEnableIt( const Pin *pPin );
void PIO_PinDisableIt( const Pin *pPin );
void PIO_PinSetDebounceFilter( const Pin* pPin, uint32_t dwCutOff );

#endif